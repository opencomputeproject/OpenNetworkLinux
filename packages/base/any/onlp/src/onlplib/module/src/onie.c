/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 * ONIE System Board Information decoding.
 *
 * See:
 * https://github.com/onie/onie/blob/master/docs/design-spec/hw_requirements.rst
 *
 * This code derived from the feature-sys-eeprom-tlv.patch in the ONIE
 * distribution.
 *
 ***********************************************************/

#include <onlplib/onie.h>
#include <onlplib/crc32.h>

#include <AIM/aim_memory.h>
#include <AIM/aim_string.h>
#include <AIM/aim_printf.h>

#include <arpa/inet.h>

#include "onlplib_log.h"

#include <IOF/iof.h>

/**
 * Header Field Constants
 */
#define TLV_INFO_ID_STRING      "TlvInfo"
#define TLV_INFO_VERSION        0x01
#define TLV_INFO_MAX_LEN        2048
#define TLV_TOTAL_LEN_MAX       (TLV_INFO_MAX_LEN - sizeof(tlvinfo_header_t))

/**
 * Validate checksum
 */
static int checksum_validate__(const uint8_t *data);


/**
 * ONIE TLV EEPROM Header
 */
typedef struct __attribute__ ((__packed__)) tlvinfo_header_s {
    char        signature[8];       /* 0x00 - 0x07 EEPROM Tag "TlvInfo" */
    uint8_t     version;            /* 0x08        Structure version    */
    uint16_t    totallen;           /* 0x09 - 0x0A Length of all data which follows */
} tlvinfo_header_t;

/**
 * ONIE TLV Entry
 */
typedef struct __attribute__ ((__packed__)) tlvinfo_tlv_s {
    uint8_t  type;
    uint8_t  length;
    uint8_t  value[0];
} tlvinfo_tlv_t;


/**
 *  The TLV Types.
 */
#define TLV_CODE_PRODUCT_NAME   0x21
#define TLV_CODE_PART_NUMBER    0x22
#define TLV_CODE_SERIAL_NUMBER  0x23
#define TLV_CODE_MAC_BASE       0x24
#define TLV_CODE_MANUF_DATE     0x25
#define TLV_CODE_DEVICE_VERSION 0x26
#define TLV_CODE_LABEL_REVISION 0x27
#define TLV_CODE_PLATFORM_NAME  0x28
#define TLV_CODE_ONIE_VERSION   0x29
#define TLV_CODE_MAC_SIZE       0x2A
#define TLV_CODE_MANUF_NAME     0x2B
#define TLV_CODE_MANUF_COUNTRY  0x2C
#define TLV_CODE_VENDOR_NAME    0x2D
#define TLV_CODE_DIAG_VERSION   0x2E
#define TLV_CODE_SERVICE_TAG    0x2F
#define TLV_CODE_VENDOR_EXT     0xFD
#define TLV_CODE_CRC_32         0xFE


static void
decode_tlv__(onlp_onie_info_t* info, tlvinfo_tlv_t * tlv)
{
    switch (tlv->type)
        {
            /* String TLVs */
#define CASE_TLV_STRING(_info, _member, _code, _tlv)                    \
            case TLV_CODE_##_code :                                     \
                {                                                       \
                    if(_info -> _member) {                              \
                        aim_free((void*) _info -> _member);             \
                    }                                                   \
                    _info -> _member = aim_zmalloc(_tlv->length + 1);   \
                    memcpy((void*) _info -> _member, _tlv->value, _tlv->length); \
                    break; \
                }

            CASE_TLV_STRING(info, product_name, PRODUCT_NAME, tlv);
            CASE_TLV_STRING(info, part_number, PART_NUMBER, tlv);
            CASE_TLV_STRING(info, serial_number, SERIAL_NUMBER, tlv);
            CASE_TLV_STRING(info, manufacture_date, MANUF_DATE, tlv);
            CASE_TLV_STRING(info, label_revision, LABEL_REVISION, tlv);
            CASE_TLV_STRING(info, platform_name, PLATFORM_NAME, tlv);
            CASE_TLV_STRING(info, onie_version, ONIE_VERSION, tlv);
            CASE_TLV_STRING(info, manufacturer, MANUF_NAME, tlv);
            CASE_TLV_STRING(info, country_code, MANUF_COUNTRY, tlv);
            CASE_TLV_STRING(info, vendor, VENDOR_NAME, tlv);
            CASE_TLV_STRING(info, service_tag, SERVICE_TAG, tlv);
            CASE_TLV_STRING(info, diag_version, DIAG_VERSION, tlv);

        case TLV_CODE_MAC_BASE:
            memcpy(info->mac, tlv->value, 6);
            break;

        case TLV_CODE_DEVICE_VERSION:
            info->device_version = tlv->value[0];
            break;

        case TLV_CODE_MAC_SIZE:
            info->mac_range = (tlv->value[0] << 8) | tlv->value[1];
            break;

        case TLV_CODE_VENDOR_EXT:
            {
                onlp_onie_vx_t* vx = aim_zmalloc(sizeof(*vx));
                vx->size = tlv->length;
                memcpy(vx->data, tlv->value, tlv->length);
                list_push(&info->vx_list, &vx->links);
                break;
            }

        case TLV_CODE_CRC_32:
            info->crc =
                (tlv->value[0] << 24) |
                (tlv->value[1] << 16) |
                (tlv->value[2] <<  8) |
                (tlv->value[3]);
            break;

        default:
            AIM_LOG_WARN("ONIE data contains an unrecognized TLV code: 0x%.2x (ignored)", tlv->type);
            break;
        }
}

/**
 *  is_valid_tlvinfo_header
 *
 *  Perform sanity checks on the first 11 bytes of the TlvInfo EEPROM
 *  data pointed to by the parameter:
 *      1. First 8 bytes contain null-terminated ASCII string "TlvInfo"
 *      2. Version byte is 1
 *      3. Total length bytes contain value which is less than or equal
 *         to the allowed maximum (2048-11)
 *
 */
static inline int is_valid_tlvinfo_header__(tlvinfo_header_t *hdr)
{
    return( (strcmp(hdr->signature, TLV_INFO_ID_STRING) == 0) &&
            (hdr->version == TLV_INFO_VERSION) &&
            (ntohs(hdr->totallen) <= TLV_TOTAL_LEN_MAX) );
}


/**
 *  is_valid_tlv
 *
 *  Perform basic sanity checks on a TLV field. The TLV is pointed to
 *  by the parameter provided.
 *      1. The type code is not reserved (0x00 or 0xFF)
 */
static inline int is_valid_tlv__(tlvinfo_tlv_t *tlv)
{
       return( (tlv->type != 0x00) &&
               (tlv->type != 0xFF) );

}


int
onlp_onie_decode(onlp_onie_info_t* rv, const uint8_t* data, int size)
{
    int tlv_end;
    int curr_tlv;
    tlvinfo_header_t* data_hdr = (tlvinfo_header_t *) data;
    tlvinfo_tlv_t* data_tlv;

    if(rv == NULL || data == NULL || (size && size < sizeof(*data_hdr))) {
        return -1;
    }

    memset(rv, 0, sizeof(*rv));
    list_init(&rv->vx_list);

    if ( !is_valid_tlvinfo_header__(data_hdr) ) {
        AIM_LOG_ERROR("ONIE data is not in TlvInfo format.");
        return -1;
    }

    rv->_hdr_id_string = aim_strdup(data_hdr->signature);
    rv->_hdr_version = data_hdr->version;
    rv->_hdr_length = ntohs(data_hdr->totallen);

    /* We only parse TLV Header Version 1 */
    if(rv->_hdr_version != 1) {
        AIM_LOG_ERROR("ONIE data header version %d id string %s is not supported.", rv->_hdr_version, rv->_hdr_id_string);
        return -1;
    }

    /* Validate CRC checksum before attempting to parse */
    if(checksum_validate__(data) != 0) {
        /* Error already logged */
        return -1;
    }


    curr_tlv = sizeof(tlvinfo_header_t);
    tlv_end  = sizeof(tlvinfo_header_t) + ntohs(data_hdr->totallen);
    while (curr_tlv < tlv_end) {
        data_tlv = (tlvinfo_tlv_t *) &data[curr_tlv];
        if (!is_valid_tlv__(data_tlv)) {
            AIM_LOG_ERROR("ONIE data invalid TLV field starting at offset %d\n", curr_tlv);
            return -1;
        }
        decode_tlv__(rv, data_tlv);
        curr_tlv += sizeof(tlvinfo_tlv_t) + data_tlv->length;
    }

    return 0;
}

int
onlp_onie_decode_file(onlp_onie_info_t* onie, const char* file)
{
    char* data;
    FILE* fp  = fopen(file, "rb");
    int rv = -1;

    if(fp) {
        fseek(fp, 0L, SEEK_END);
        int size = ftell(fp);
        rewind(fp);
        data = aim_malloc(size);

        rv = fread(data, 1, size, fp);
        fclose(fp);

        if(rv == size) {
            rv = onlp_onie_decode(onie, (uint8_t*)data, size);
        }

        aim_free(data);
    }
    return rv;
}

/**
 *  Validate the checksum in the provided TlvInfo EEPROM data. First,
 *  verify that the TlvInfo header is valid, then make sure the last
 *  TLV is a CRC-32 TLV. Then calculate the CRC over the EEPROM data
 *  and compare it to the value stored in the EEPROM CRC-32 TLV.
 */
static int
checksum_validate__(const uint8_t *data)
{
    tlvinfo_header_t* data_hdr = (tlvinfo_header_t *) data;
    tlvinfo_tlv_t* data_crc;
    unsigned int calc_crc;
    unsigned int stored_crc;

    // Is the eeprom header valid?
    if (!is_valid_tlvinfo_header__(data_hdr)) {
        return 0;
    }

    // Is the last TLV a CRC?
    data_crc = (tlvinfo_tlv_t *) &data[sizeof(tlvinfo_header_t) +
                                       ntohs(data_hdr->totallen) - (sizeof(tlvinfo_tlv_t) + 4)];

    if ((data_crc->type != TLV_CODE_CRC_32) || (data_crc->length != 4)) {
        AIM_LOG_ERROR("ONIE CRC TLV is invalid.");
        return 0;
    }

    // Calculate the checksum
    calc_crc = onlp_crc32(0, (void *)data,
                          sizeof(tlvinfo_header_t) + ntohs(data_hdr->totallen) - 4);
    stored_crc = (data_crc->value[0] << 24) |
        (data_crc->value[1] << 16) |
        (data_crc->value[2] <<  8) |
        data_crc->value[3];

    if(calc_crc != stored_crc)  {
        AIM_LOG_ERROR("ONIE data crc error: expected 0x%.8x calculated 0x%.8x",
                      stored_crc, calc_crc);
        return -1;
    }
    return 0;
}

void
onlp_onie_info_free(onlp_onie_info_t* info)
{
    if(info) {
        aim_free(info->product_name);
        aim_free(info->part_number);
        aim_free(info->serial_number);
        aim_free(info->manufacture_date);
        aim_free(info->label_revision);
        aim_free(info->platform_name);
        aim_free(info->onie_version);
        aim_free(info->manufacturer);
        aim_free(info->country_code);
        aim_free(info->vendor);
        aim_free(info->diag_version);
        aim_free(info->service_tag);
        aim_free(info->_hdr_id_string);

        list_links_t *cur, *next;
        LIST_FOREACH_SAFE(&info->vx_list, cur, next) {
            onlp_onie_vx_t* vx = container_of(cur, links, onlp_onie_vx_t);
            aim_free(vx);
        }
    }
}

void
onlp_onie_show(onlp_onie_info_t* info, aim_pvs_t* pvs)
{
    iof_t iof;
    iof_init(&iof, pvs);
    if(info->product_name) {
        iof_iprintf(&iof, "Product Name: %s", info->product_name);
    }
    if(info->part_number) {
        iof_iprintf(&iof, "Part Number: %s", info->part_number);
    }
    if(info->serial_number) {
        iof_iprintf(&iof, "Serial Number: %s", info->serial_number);
    }
    if(info->mac) {
        iof_iprintf(&iof, "MAC: %{mac}", info->mac);
    }
    if(info->mac_range) {
        iof_iprintf(&iof, "MAC Range: %d", info->mac_range);
    }
    if(info->manufacturer) {
        iof_iprintf(&iof, "Manufacturer: %s", info->manufacturer);
    }
    if(info->manufacture_date) {
        iof_iprintf(&iof, "Manufacture Date: %s", info->manufacture_date);
    }
    if(info->vendor) {
        iof_iprintf(&iof, "Vendor: %s", info->vendor);
    }
    if(info->platform_name) {
        iof_iprintf(&iof, "Platform Name: %s", info->platform_name);
    }
    if(info->device_version) {
        iof_iprintf(&iof, "Device Version: %u", info->device_version);
    }
    if(info->label_revision) {
        iof_iprintf(&iof, "Label Revision: %s", info->label_revision);
    }
    if(info->country_code) {
        iof_iprintf(&iof, "Country Code: %s", info->country_code);
    }
    if(info->diag_version) {
        iof_iprintf(&iof, "Diag Version: %s", info->diag_version);
    }
    if(info->service_tag) {
        iof_iprintf(&iof, "Service Tag: %s", info->service_tag);
    }
    if(info->onie_version) {
        iof_iprintf(&iof, "ONIE Version: %s", info->onie_version);
    }
}

#include <cjson/cJSON.h>
#include <cjson_util/cjson_util.h>

void
onlp_onie_show_json(onlp_onie_info_t* info, aim_pvs_t* pvs)
{
    cJSON* cj = cJSON_CreateObject();

#define _S(_name, _member)                                              \
    do {                                                                \
        if(info-> _member) {                                            \
            cJSON_AddStringToObject(cj, #_name, info-> _member);        \
        } else {                                                        \
            cJSON_AddNullToObject(cj, #_name);                          \
        }                                                               \
    } while(0)

#define _N(_name, _member)                                      \
    do {                                                        \
        cJSON_AddNumberToObject(cj, #_name, info-> _member);    \
    } while(0)

    _S(Product Name, product_name);
    _S(Part Number, part_number);
    _S(Serial Number, serial_number);
    {
        char* mac = aim_dfstrdup("%{mac}", info->mac);
        cJSON_AddStringToObject(cj, "MAC", mac);
        aim_free(mac);
    }
    _S(Manufacturer, manufacturer);
    _S(Manufacture Date,manufacture_date);
    _S(Vendor,vendor);
    _S(Platform Name,platform_name);
    _S(Label Revision,label_revision);
    _S(Country Code,country_code);
    _S(Diag Version,diag_version);
    _S(Service Tag,service_tag);
    _S(ONIE Version,onie_version);
    _N(Device Version, device_version);
    {
        char* crc = aim_fstrdup("0x%x", info->crc);
        cJSON_AddStringToObject(cj, "CRC", crc);
        aim_free(crc);
    }
    char* out = cJSON_Print(cj);
    aim_printf(pvs, "%s\n", out);
    free(out);
    cJSON_Delete(cj);
}

static char*
lookup_entry__(cJSON* cj, const char* name, int code)
{
    char* str = NULL;
    int rv = cjson_util_lookup_string(cj, &str, "0x%x", code);
    if(rv < 0) {
        rv = cjson_util_lookup_string(cj, &str, name);
    }
    if(rv < 0) {
        return NULL;
    }
    else {
        return aim_strdup(str);
    }
}

int
onlp_onie_read_json(onlp_onie_info_t* info, const char* fname)
{
    cJSON* cj;

    memset(info, 0, sizeof(*info));

    list_init(&info->vx_list);

    int rv = cjson_util_parse_file(fname, &cj);
    if(rv < 0) {
        AIM_LOG_ERROR("Could not parse ONIE JSON file '%s' rv=%{aim_error}",
                      fname, rv);
        return rv;
    }

#define ONIE_TLV_ENTRY_str(_member, _name, _code)                   \
    do {                                                            \
        info->_member = lookup_entry__(cj, #_name, _code);          \
    } while(0)

#define ONIE_TLV_ENTRY_mac(_member, _name, _code)             \
    do {                                                      \
        char* str = lookup_entry__(cj, #_name, _code);        \
        int mac[6] = {0};                                     \
        if(str) {                                             \
            int i;                                            \
            sscanf(str, "%x:%x:%x:%x:%x:%x",                  \
                   mac+0, mac+1, mac+2, mac+3, mac+4, mac+5); \
            for(i = 0; i < 6; i++) info->mac[i] = mac[i];     \
            aim_free(str);                                    \
        }                                                     \
    } while(0)

#define ONIE_TLV_ENTRY_byte(_member, _name, _code)      \
    do {                                                \
        char* v = lookup_entry__(cj, #_name, _code);    \
        if(v) {                                         \
            info->_member = atoi(v);                    \
            aim_free(v);                                \
        }                                               \
    } while(0)

#define ONIE_TLV_ENTRY_int16(_member, _name, _code)     \
    do {                                                \
        char* v = lookup_entry__(cj, #_name, _code);    \
        if(v) {                                         \
            info->_member = atoi(v);                    \
            aim_free(v);                                \
        }                                               \
    } while(0)

#define ONIE_TLV_ENTRY(_member, _name, _code, _type)    \
    ONIE_TLV_ENTRY_##_type(_member, _name, _code);

    #include <onlplib/onlplib.x>


    cJSON_Delete(cj);
    return 0;
}
