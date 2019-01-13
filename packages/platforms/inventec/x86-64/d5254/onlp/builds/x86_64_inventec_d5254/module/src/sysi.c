#if 0
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

#include "x86_64_inventec_d5254_int.h"
#include "x86_64_inventec_d5254_log.h"

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


typedef struct sysi_onie_vpd_s {
    uint8_t type;
    char file[ONLP_CONFIG_INFO_STR_MAX];
} sysi_onie_vpd_t;





typedef struct sysi_platform_info_s {
    char type[SYSI_PLATFORM_INFO_TYPE_STR_MAX];
    char file[ONLP_CONFIG_INFO_STR_MAX];
    int (*parsing_func_ptr)(char* file_str, char* version);
} sysi_platform_info_t;

static int _sysi_cpld_version_parsing(char* file_str, char* version);
static int _sysi_psoc_version_parsing(char* file_str, char* version);
static int _sysi_onie_product_name_get(char** product_name);
static int _sysi_onie_part_number_get(char** part_number);
static int _sysi_onie_serial_number_get(char** serial_number);
static int _sysi_onie_mac_base_get(uint8_t mac_base[6]);
static int _sysi_onie_manuf_date_get(char** manuf_date);
static int _sysi_onie_device_version_get(uint8_t* device_version);
static int _sysi_onie_label_revision_get(char** label_revision);
static int _sysi_onie_platform_name_get(char** platform_name);
static int _sysi_onie_onie_version_get(char** onie_version);
static int _sysi_onie_mac_size_get(uint16_t* mac_size);
static int _sysi_onie_manuf_name_get(char** manuf_name);
static int _sysi_onie_manuf_country_get(char** manuf_country);
static int _sysi_onie_vendor_name_get(char** vendor_name);
static int _sysi_onie_diag_version_get(char** diag_version);
static int _sysi_onie_service_tag_get(char** service_tag);
static int _sysi_onie_vendor_ext_get(list_head_t *vendor_ext);
static int _sysi_onie_crc_32_get(uint32_t* crc_32);

static int _sysi_onie_info_total_len_get(onlp_onie_info_t *onie, uint16_t *total_len);



static sysi_onie_vpd_t __tlv_vpd_info[SYSI_ONIE_TYPE_SUPPORT_NUM] = {
    {
        TLV_CODE_PRODUCT_NAME,
        "/sys/class/eeprom/vpd/product_name"
    },
    {
        TLV_CODE_PART_NUMBER,
        "/sys/class/eeprom/vpd/pn"
    },
    {
        TLV_CODE_SERIAL_NUMBER,
        "/sys/class/eeprom/vpd/sn"
    },
    {
        TLV_CODE_MAC_BASE,
        "/sys/class/eeprom/vpd/base_mac_addr"
    },
    {
        TLV_CODE_MANUF_DATE,
        "/sys/class/eeprom/vpd/man_date"
    },
    {
        TLV_CODE_DEVICE_VERSION,
        "/sys/class/eeprom/vpd/dev_ver"
    },
    {
        TLV_CODE_LABEL_REVISION,
        "/sys/class/eeprom/vpd/label_rev"
    },
    {
        TLV_CODE_PLATFORM_NAME,
        "/sys/class/eeprom/vpd/plat_name"
    },
    {
        TLV_CODE_ONIE_VERSION,
        "/sys/class/eeprom/vpd/ldr_ver"
    },
    {
        TLV_CODE_MAC_SIZE,
        "/sys/class/eeprom/vpd/mac_addr"
    },
    {
        TLV_CODE_MANUF_NAME,
        "/sys/class/eeprom/vpd/manufacturer"
    },
    {
        TLV_CODE_MANUF_COUNTRY,
        "/sys/class/eeprom/vpd/country_code"
    },
    {
        TLV_CODE_VENDOR_NAME,
        "/sys/class/eeprom/vpd/vendor_name"
    },
    {
        TLV_CODE_DIAG_VERSION,
        "/sys/class/eeprom/vpd/diag_ver"
    },
    {
        TLV_CODE_SERVICE_TAG,
        "/sys/class/eeprom/vpd/service_tag"
    },
    {
        TLV_CODE_VENDOR_EXT,
        "/sys/class/eeprom/vpd/vendor_ext"
    },
    {
        TLV_CODE_CRC_32,
        "/sys/class/eeprom/vpd/crc32"
    }
};


static sysi_platform_info_t __platform_info[SYSI_PLATFORM_INFO_NUM] = {
    {"cpld", "/sys/class/hwmon/hwmon2/device/info",_sysi_cpld_version_parsing},
    {"psoc", "/sys/class/hwmon/hwmon1/device/version",_sysi_psoc_version_parsing}
};


static onlp_oid_t __oid_info[] = {
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_PHY),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_CORE0),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_CORE1),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_CORE2),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_CORE3),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_1_ON_MAIN_BROAD),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_2_ON_MAIN_BROAD),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_3_ON_MAIN_BROAD),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_4_ON_MAIN_BROAD),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_5_ON_MAIN_BROAD),
    ONLP_FAN_ID_CREATE(ONLP_FAN_1),
    ONLP_FAN_ID_CREATE(ONLP_FAN_2),
    ONLP_FAN_ID_CREATE(ONLP_FAN_3),
    ONLP_FAN_ID_CREATE(ONLP_FAN_4),
    ONLP_FAN_ID_CREATE(ONLP_FAN_5),
    ONLP_PSU_ID_CREATE(ONLP_PSU_1),
    ONLP_PSU_ID_CREATE(ONLP_PSU_2),
    ONLP_LED_ID_CREATE(ONLP_LED_MGMT_GREEN),
    ONLP_LED_ID_CREATE(ONLP_LED_MGMT_RED),
    0 /*end*/
};




static int _sysi_cpld_version_parsing(char* file_str, char* version)
{
    int rv = ONLP_STATUS_OK;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX*4];
    char *temp;

    rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX*4, &len, file_str);

    temp = strstr(buf, "The CPLD version is ");
    if(temp) {
        temp += strlen("The CPLD version is ");
        snprintf(version,ONLP_CONFIG_INFO_STR_MAX, temp);
        /*remove '\n'*/
        version[strlen(version)-1] = 0;
    } else {
        rv = ONLP_STATUS_E_MISSING;
    }
    return rv;
}

static int _sysi_psoc_version_parsing(char* file_str, char* version)
{
    int rv = ONLP_STATUS_OK;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX];
    char *temp;

    rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, file_str);
    temp = strstr(buf, "ver: ");
    if(temp) {
        temp += strlen("ver: ");
        snprintf(version,ONLP_CONFIG_INFO_STR_MAX, temp);
        /*remove '\n'*/
        version[strlen(version)-1] = 0;
    } else {
        rv = ONLP_STATUS_E_MISSING;
    }
    return rv;
}

static int _sysi_onie_product_name_get(char** product_name)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_PRODUCT_NAME == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *product_name = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *product_name = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_part_number_get(char** part_number)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_PART_NUMBER == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *part_number = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *part_number = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_serial_number_get(char** serial_number)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_SERIAL_NUMBER == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *serial_number = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *serial_number = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_mac_base_get(uint8_t mac_base[6])
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_MAC_BASE == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        if(6 != sscanf( buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx\n",
                        &mac_base[0], &mac_base[1], &mac_base[2],
                        &mac_base[3], &mac_base[4], &mac_base[5])) {
            /*parsing fail*/
            memset(mac_base, 0, 6);
        }
    } else {
        memset(mac_base, 0, 6);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_manuf_date_get(char** manuf_date)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_MANUF_DATE == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *manuf_date = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *manuf_date = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_device_version_get(uint8_t* device_version)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_DEVICE_VERSION == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        *device_version = (uint8_t)strtoul(buf, NULL, 0);
    } else {
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_label_revision_get(char** label_revision)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_LABEL_REVISION == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *label_revision = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *label_revision = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_platform_name_get(char** platform_name)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_PLATFORM_NAME == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *platform_name = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *platform_name = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_onie_version_get(char** onie_version)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_ONIE_VERSION == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *onie_version = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *onie_version = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_mac_size_get(uint16_t* mac_size)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_MAC_SIZE == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        *mac_size = (uint16_t)strtoul(buf, NULL, 0);
    }
    /*return OK no matter what*/
    return ONLP_STATUS_OK;
}


static int _sysi_onie_manuf_name_get(char** manuf_name)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_MANUF_NAME == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *manuf_name = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *manuf_name = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_manuf_country_get(char** manuf_country)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_MANUF_COUNTRY == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *manuf_country = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *manuf_country = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_vendor_name_get(char** vendor_name)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_VENDOR_NAME == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *vendor_name = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *vendor_name = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_diag_version_get(char** diag_version)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_DIAG_VERSION == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *diag_version = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *diag_version = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_service_tag_get(char** service_tag)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_SERVICE_TAG == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*remove \n in file output*/
        buf[strlen(buf)-1] = 0;
        *service_tag = aim_fstrdup("%s",buf);
    } else {
        /*alloc a empty array for it*/
        *service_tag = aim_zmalloc(1);
        rv = ONLP_STATUS_OK;
    }

    return rv;
}


static int _sysi_onie_vendor_ext_get(list_head_t* vendor_ext)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    list_init(vendor_ext);

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_VENDOR_EXT == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        /*TODO*/
    }
    /*return OK no matter what*/
    return ONLP_STATUS_OK;
}


static int _sysi_onie_crc_32_get(uint32_t* crc_32)
{
    int rv = ONLP_STATUS_OK;
    int i;
    int len;
    char* path;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0; i<SYSI_ONIE_TYPE_SUPPORT_NUM; i++ ) {
        if( TLV_CODE_CRC_32 == __tlv_vpd_info[i].type) {
            path = __tlv_vpd_info[i].file;
            break;
        }
    }
    if( SYSI_ONIE_TYPE_SUPPORT_NUM == i) {
        /* Cannot find support type */
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    if( ONLP_STATUS_OK == rv) {
        /*get info*/
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, path);
    }
    if( ONLP_STATUS_OK == rv) {
        *crc_32 = (uint32_t)strtoul (buf, NULL, 0);
    }
    /*return OK no matter what*/
    return ONLP_STATUS_OK;
}

static int _sysi_onie_info_total_len_get(onlp_onie_info_t *onie, uint16_t *total_len)
{
    uint16_t len = 0;

    /*product_name*/
    if(strlen(onie->product_name)!= 0) {
        len += 2;
        len += strlen(onie->product_name);
    }
    /*part_number*/
    if(strlen(onie->part_number)!= 0) {
        len += 2;
        len += strlen(onie->part_number);
    }
    /*serial_number*/
    if(strlen(onie->serial_number)!= 0) {
        len += 2;
        len += strlen(onie->serial_number);
    }

    /*mac*/
    len += 2;
    len += 6;

    /*manufacture_date*/
    if(strlen(onie->manufacture_date)!= 0) {
        len += 2;
        len += 19;
    }

    /*device_version*/
    len += 2;
    len += 1;

    /*label_revision*/
    if(strlen(onie->label_revision)!= 0) {
        len += 2;
        len += strlen(onie->label_revision);
    }

    /*platform_name*/
    if(strlen(onie->platform_name)!= 0) {
        len += 2;
        len += strlen(onie->platform_name);
    }

    /*onie_version*/
    if(strlen(onie->onie_version)!= 0) {
        len += 2;
        len += strlen(onie->onie_version);
    }

    /*mac_range*/
    len += 2;
    len += 2;

    /*manufacturer*/
    if(strlen(onie->manufacturer)!= 0) {
        len += 2;
        len += strlen(onie->manufacturer);
    }

    /*country_code*/
    if(strlen(onie->country_code)!= 0) {
        len += 2;
        len += 2;
    }

    /*vendor*/
    if(strlen(onie->vendor)!= 0) {
        len += 2;
        len += strlen(onie->vendor);
    }

    /*diag_version*/
    if(strlen(onie->diag_version)!= 0) {
        len += 2;
        len += strlen(onie->diag_version);
    }

    /*service_tag*/
    if(strlen(onie->service_tag)!= 0) {
        len += 2;
        len += strlen(onie->service_tag);
    }

    /*crc*/
    len += 2;
    len += 4;

    /*vx_list*/
    /*TODO*/

    *total_len = len;
    return ONLP_STATUS_OK;
}



const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-inventec-d5254-r0";
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
    uint16_t total_len;

    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_product_name_get(&onie->product_name);
    }

    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_part_number_get(&onie->part_number);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_serial_number_get(&onie->serial_number);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_mac_base_get(onie->mac);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_manuf_date_get(&onie->manufacture_date);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_device_version_get(&onie->device_version);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_label_revision_get(&onie->label_revision);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_platform_name_get(&onie->platform_name);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_onie_version_get(&onie->onie_version);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_mac_size_get(&onie->mac_range);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_manuf_name_get(&onie->manufacturer);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_manuf_country_get(&onie->country_code);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_vendor_name_get(&onie->vendor);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_diag_version_get(&onie->diag_version);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_service_tag_get(&onie->service_tag);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_vendor_ext_get(&onie->vx_list);
    }
    if(ONLP_STATUS_OK == rv) {
        rv = _sysi_onie_crc_32_get(&onie->crc);
    }

    _sysi_onie_info_total_len_get(onie, &total_len);

    onie->_hdr_id_string = aim_fstrdup("TlvInfo");
    onie->_hdr_version = 0x1;
    onie->_hdr_length = total_len;
    return rv;
}


int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int i;
    int rv = ONLP_STATUS_OK;
    char cpld_str[ONLP_CONFIG_INFO_STR_MAX]= {0};
    char other_str[ONLP_CONFIG_INFO_STR_MAX]= {0};
    char version[ONLP_CONFIG_INFO_STR_MAX];

    for(i=0 ; i<SYSI_PLATFORM_INFO_NUM; i++) {
        memset(version, 0, ONLP_CONFIG_INFO_STR_MAX);
        /*get version info*/
        rv = __platform_info[i].parsing_func_ptr(
                 __platform_info[i].file, version);
        if( 0 == strncmp(__platform_info[i].type,
                         "cpld", strlen(__platform_info[i].type))) {
            snprintf(cpld_str, ONLP_CONFIG_INFO_STR_MAX, "%s%s ",cpld_str,version);
        } else {
            snprintf(other_str, ONLP_CONFIG_INFO_STR_MAX, "%s%s.%s "
                     ,other_str,__platform_info[i].type,version);
        }
    }

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

    for(i=0; i<max; i++) {
        if(__oid_info[i]==0) {
            break;
        }
        *e++ = __oid_info[i];
    }
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_fans(void)
{
    /*Ensure switch manager is working*/
    PLATFORM_PSOC_DIAG_LOCK;
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    /*Ensure switch manager is working*/
    PLATFORM_PSOC_DIAG_LOCK;
    return ONLP_STATUS_OK;
}

#endif
