/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
 *
 *
 ***********************************************************/
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlplib/onie.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_inventec_d10064_int.h"
#include "x86_64_inventec_d10064_log.h"

#include "platform_lib.h"

#define SYSI_ONIE_TYPE_SUPPORT_NUM        17
#define SYSI_PLATFORM_INFO_TYPE_STR_MAX   10
#define SYSI_PLATFORM_INFO_NUM             2

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

static uint8_t __tlv_code_list[SYSI_ONIE_TYPE_SUPPORT_NUM] = {
    TLV_CODE_PRODUCT_NAME,
    TLV_CODE_PART_NUMBER,
    TLV_CODE_SERIAL_NUMBER,
    TLV_CODE_MAC_BASE,
    TLV_CODE_MANUF_DATE,
    TLV_CODE_DEVICE_VERSION,
    TLV_CODE_LABEL_REVISION,
    TLV_CODE_PLATFORM_NAME,
    TLV_CODE_ONIE_VERSION,
    TLV_CODE_MAC_SIZE,
    TLV_CODE_MANUF_NAME,
    TLV_CODE_MANUF_COUNTRY,
    TLV_CODE_VENDOR_NAME,
    TLV_CODE_DIAG_VERSION,
    TLV_CODE_SERVICE_TAG,
    TLV_CODE_VENDOR_EXT,
    TLV_CODE_CRC_32
};


static int _sysi_version_parsing(char* file_str, char* str_buf, char* version);
static void _case_tlv_code_string(onlp_onie_info_t* info, char** member, char* path);
static int _parse_tlv(onlp_onie_info_t* info, uint8_t type);

static int _sysi_version_parsing(char* file_str, char* str_buf, char* version)
{
    int rv = ONLP_STATUS_OK;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX*4];
    char *temp;

    rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX*4, &len, file_str);
    if( rv != ONLP_STATUS_OK ) { return rv; }

    temp = strstr(buf, str_buf);
    if(temp) {
        temp += strlen(str_buf);
        snprintf(version,ONLP_CONFIG_INFO_STR_MAX, temp);
        /*remove '\n'*/
        version[strlen(version)-1] = '\0';
    } else {
        rv = ONLP_STATUS_E_MISSING;
    }
    return rv;
}

static void _case_tlv_code_string(onlp_onie_info_t* info, char** member, char* path)
{
    int rv = ONLP_STATUS_OK;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX];
    rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    if( rv == ONLP_STATUS_OK ) {
        info->_hdr_length += 2;
        buf[strlen(buf)-1] = '\0';
        *member = aim_fstrdup("%s",buf);
        info->_hdr_length += strlen(*member);
    } else {
        *member = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }
    return;
}
 

static int _parse_tlv(onlp_onie_info_t* info, uint8_t type)
{
    int rv = ONLP_STATUS_OK;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX];
    switch(type)
       {
           case TLV_CODE_PRODUCT_NAME:
               _case_tlv_code_string(info, &(info->product_name), INV_SYS_PREFIX"product_name");
               break;
           case TLV_CODE_PART_NUMBER:
               _case_tlv_code_string(info, &(info->part_number), INV_SYS_PREFIX"pn");
               break;
           case TLV_CODE_SERIAL_NUMBER:
               _case_tlv_code_string(info, &(info->serial_number), INV_SYS_PREFIX"sn");
               break;
           case TLV_CODE_MANUF_DATE:
               _case_tlv_code_string(info, &(info->manufacture_date), INV_SYS_PREFIX"man_date");
               break;
           case TLV_CODE_LABEL_REVISION:
               _case_tlv_code_string(info, &(info->label_revision), INV_SYS_PREFIX"label_rev");
               break;
           case TLV_CODE_PLATFORM_NAME:
               _case_tlv_code_string(info, &(info->platform_name), INV_SYS_PREFIX"plat_name");
               break;
           case TLV_CODE_ONIE_VERSION:
               _case_tlv_code_string(info, &(info->onie_version), INV_SYS_PREFIX"ldr_ver");
               break;
           case TLV_CODE_MANUF_NAME:
               _case_tlv_code_string(info, &(info->manufacturer), INV_SYS_PREFIX"manufacturer");
               break;
           case TLV_CODE_MANUF_COUNTRY:
               _case_tlv_code_string(info, &(info->country_code), INV_SYS_PREFIX"country_code");
               break;
           case TLV_CODE_VENDOR_NAME:
               _case_tlv_code_string(info, &(info->vendor), INV_SYS_PREFIX"vendor_name");
               break;
           case TLV_CODE_SERVICE_TAG:
               _case_tlv_code_string(info, &(info->service_tag), INV_SYS_PREFIX"service_tag");
               break;
           case TLV_CODE_DIAG_VERSION:
               _case_tlv_code_string(info, &(info->diag_version), INV_SYS_PREFIX"diag_ver");
               break;

           case TLV_CODE_MAC_BASE:
               rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, INV_SYS_PREFIX"base_mac_addr" );
               if( rv == ONLP_STATUS_OK ) {
                   if(sscanf( buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx\n",                               
                        &info->mac[0], &info->mac[1], &info->mac[2],
                        &info->mac[3], &info->mac[4], &info->mac[5]) == 6) {
                        info->_hdr_length += 2;
                        info->_hdr_length += 6;
                   } else {
                       /*parsing fail*/
                       memset(info->mac, 0, 6);
                   }
               } else {
                   memset(info->mac, 0, 6);
                   rv = ONLP_STATUS_OK;
               }
               break;

           case TLV_CODE_DEVICE_VERSION:
               rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, INV_SYS_PREFIX"dev_ver");
               if( rv == ONLP_STATUS_OK ) {
                   info->_hdr_length += 2;
                   info->device_version= (uint8_t)strtoul(buf, NULL, 0);
                   info->_hdr_length += 1;
               }else{
                   info->device_version = 0;
                   rv = ONLP_STATUS_OK;
               }
               break;
           case TLV_CODE_MAC_SIZE:
               rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, INV_SYS_PREFIX"mac_addr");
               if( rv == ONLP_STATUS_OK ) {
                   info->_hdr_length += 2;
                   info->mac_range = (uint16_t)strtoul(buf, NULL, 0);
                   info->_hdr_length += 2;
               }else{
                   info->mac_range = 0;
                   rv = ONLP_STATUS_OK;
               }
               break;

           case TLV_CODE_VENDOR_EXT:
               list_init(&info->vx_list);
               rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, INV_SYS_PREFIX"vendor_ext");
               if( rv == ONLP_STATUS_OK ) {
                   /*TODO*/
               }
               rv = ONLP_STATUS_OK;
               break;

           case TLV_CODE_CRC_32:
               rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, INV_SYS_PREFIX"crc32");
               if( rv == ONLP_STATUS_OK ) {
                   info->_hdr_length += 2;
                   info->crc = (uint32_t)strtoul(buf, NULL, 0);
                   info->_hdr_length += 4;
               }else{
                   info->crc = 0;
                   rv = ONLP_STATUS_OK;
               }
               break;

           default:
               break;
       }
    return rv;
}


const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-inventec-d10064-r0";
}


/*
 * This function is called to return the physical base address
 * of the ONIE boot rom.
 *
 * The ONLP framework will mmap() and parse the ONIE TLV structure
 * from the given data.
 *
 * If you platform does not support a mappable address for the ONIE
 * eeprom then you should not provide this function at all.
 *
 * For the purposes of this example we will provide it but
 * return UNSUPPORTED (which is all the default implementation does).
 *
 */
int
onlp_sysi_onie_data_phys_addr_get(void** pa)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}


/*
 * If you cannot provide a base address you must provide the ONLP
 * framework the raw ONIE data through whatever means necessary.
 *
 * This function will be called as a backup in the event that
 * onlp_sysi_onie_data_phys_addr_get() fails.
 */
int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
#if 0
    int rv;
    int i;

    /*
     * This represents the example ONIE data.
     */
    static uint8_t onie_data[] = {
        'T', 'l', 'v','I','n','f','o', 0,
        0x1, 0x0, 0x0,
        0x21, 0x8, 'O', 'N', 'L', 'P', 'I', 'E', 0, 0,
        0x22, 0x3, 'O', 'N', 'L',
        0xFE, 0x4, 0x4b, 0x1b, 0x1d, 0xde,
    };


    memcpy(*data, onie_data, ONLPLIB_CONFIG_I2C_BLOCK_SIZE);
    return 0;
#endif
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * IF the ONLP frame calles onlp_sysi_onie_data_get(),
 * if will call this function to free the data when it
 * is finished with it.
 *
 * This function is optional, and depends on the data
 * you return in onlp_sysi_onie_data_get().
 */
void
onlp_sysi_onie_data_free(uint8_t* data)
{
    /*
     * We returned a static array in onlp_sysi_onie_data_get()
     * so no free operation is required.
     */
    if(data) {
        aim_free(data);
    }
}


int
onlp_sysi_onie_info_get (onlp_onie_info_t *onie)
{
    int rv = ONLP_STATUS_OK;
    int i;
    onie->_hdr_length = 0;
    for(i = 0; i < SYSI_ONIE_TYPE_SUPPORT_NUM; i++)
    {
       if( rv != ONLP_STATUS_OK ) { return rv; }
       rv = _parse_tlv(onie, (__tlv_code_list[i]));

    } 

    onie->_hdr_id_string = aim_fstrdup("TlvInfo");
    onie->_hdr_version = 0x1;
    return rv;
}


int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int rv = ONLP_STATUS_OK;
    char cpld_str[ONLP_CONFIG_INFO_STR_MAX]= {0};
    char other_str[ONLP_CONFIG_INFO_STR_MAX]= {0};
    char version[ONLP_CONFIG_INFO_STR_MAX];

    rv = _sysi_version_parsing(INV_SYSLED_PREFIX"info", "The CPLD version is ", version);
    if( rv != ONLP_STATUS_OK ) { return rv; }
    snprintf(cpld_str, ONLP_CONFIG_INFO_STR_MAX, "%s%s ", cpld_str, version);
    rv = _sysi_version_parsing(INV_HWMON_PREFIX"version", "ver: ", version);
    if( rv != ONLP_STATUS_OK ) { return rv; }
    snprintf(other_str, ONLP_CONFIG_INFO_STR_MAX, "%s%s.%s "
                     ,other_str, "psoc", version);

    /*cpld version*/
    if(strlen(cpld_str) > 0) {
        pi->cpld_versions = aim_fstrdup("%s",cpld_str);
    }

    /*other version*/
    if(strlen(other_str) > 0) {
        pi->other_versions = aim_fstrdup("%s",other_str);
    }
    return rv;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    if(pi->cpld_versions) {
        aim_free(pi->cpld_versions);
    }
    if(pi->other_versions) {
        aim_free(pi->other_versions);
    }
    return;
}


int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    for(i=1;i<=THERMAL_ON_CHASSIS; i++){
        *e++ = ONLP_THERMAL_ID_CREATE(i); 
    }
    for(i=1; i<=FAN_ON_CHASSIS; i++){
        *e++ = ONLP_FAN_ID_CREATE(i); 
    }
    for(i=1; i<=PSU_ON_CHASSIS; i++){
        *e++ = ONLP_PSU_ID_CREATE(i); 
    }
    for(i=1; i<=LED_ON_CHASSIS; i++){
        *e++ = ONLP_LED_ID_CREATE(i); 
    }
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_fans(void)
{
    /*Ensure switch manager is working*/
    PLATFORM_HWMON_DIAG_LOCK;
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    /*Ensure switch manager is working*/
    PLATFORM_HWMON_DIAG_LOCK;
    return ONLP_STATUS_OK;
}

