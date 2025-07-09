#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <uapi/mtd/mtd-abi.h>
#include <linux/fs.h>
#include <linux/mtd/mtd.h>

#include "switch_mb_driver.h"
#include "switch_at24.h"

#define DRVNAME "drv_mb_driver"

#define SWITCH_MB_DRIVER_VERSION "0.0.1"

#define SYS_EEPROM_SIZE             256
#define SYS_EEPROM_MAX_SIZE         4096

#define TLV_INFO_ID_STRING          "TlvInfo"
#define TLV_INFO_ID_STRING_LENGTH   8
#define TLV_INFO_VERSION            0x01
#define TLV_TOTAL_LEN_MAX           (SYS_EEPROM_SIZE - sizeof(tlvinfo_header_t))
#define TLV_VALUE_MAX_LEN           255
#define TLV_DECODE_VALUE_MAX_LEN    ((5 * TLV_VALUE_MAX_LEN) + 1)

#define ODM_BSP_VERSION "0.0.1"

/**
 *  The TLV Types.
 *
 *  Keep these in sync with tlv_code_list in cmd_sys_eeprom.c
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

unsigned int loglevel = 0;

static struct platform_device *drv_mb_device;

struct mutex     update_lock;

static uint8_t eeprom[SYS_EEPROM_SIZE];

uint32_t *global_crc32_table;

/* Set to 1 if we've read EEPROM into memory */
static int has_been_read = 0;
/* Set to 1 if the EEPROM contents were valid when read from hardware */
static int hw_eeprom_valid = 1;

/*
 * Tlvinf header: Layout of the header for the TlvInfo format
 *
 * See the end of this file for details of this eeprom format
 */
struct tlvinfo_header_s {
    char    signature[TLV_INFO_ID_STRING_LENGTH];   /* 0x00 - 0x07 EEPROM Tag "TlvInfo" */
    uint8_t      version;  /* 0x08        Structure version */
    uint16_t     totallen; /* 0x09 - 0x0A Length of all data which follows */
} __packed;
typedef struct tlvinfo_header_s tlvinfo_header_t;

/*
 * TlvInfo TLV: Layout of a TLV field
 */
struct tlvinfo_tlv_s {
    uint8_t  type;
    uint8_t  length;
    uint8_t  value[0];
} __packed;
typedef struct tlvinfo_tlv_s tlvinfo_tlv_t;

/*
 * read_sys_eeprom - read the hwinfo from TLV EEPROM
 */
int read_sys_eeprom(void *eeprom_data, loff_t offset, int len)
{
    int ret;
    MAINBOARD_DEBUG("Read EEPROM...\n");

    /* erase the buffer before reading */
#ifdef C11_ANNEX_K
    memset_s(eeprom_data, SYS_EEPROM_SIZE, 0x0, len);
#else
    memset(eeprom_data, 0x0, len);
#endif
    ret = at24_read_fan_eeprom(0, eeprom_data, offset, len);
    if(ret < 0)
    {
        MAINBOARD_DEBUG("Read EEPROM failed: %d\n", ret);
        return -1;
    }

    return ret;
}

uint32_t* crc32_filltable(uint32_t *crc_table, int endian)
{
    uint32_t polynomial = endian ? 0x04c11db7 : 0xedb88320;
    uint32_t c;
    unsigned i, j;

    if (!crc_table)
    {
        crc_table = kzalloc(256 * sizeof(uint32_t), GFP_KERNEL);
    }

    if (!crc_table)
    {
        return NULL;
    }

    for (i = 0; i < 256; i++) {
        c = endian ? (i << 24) : i;
        for (j = 8; j; j--) {
            if (endian)
            {
                c = (c&0x80000000) ? ((c << 1) ^ polynomial) : (c << 1);
            }
            else
            {
                c = (c&1) ? ((c >> 1) ^ polynomial) : (c >> 1);
            }
        }
        *crc_table++ = c;
    }

    return crc_table - 256;
}

uint32_t crc32_block_endian0(uint32_t val, const void *buf, unsigned len, uint32_t *crc_table)
{
    const void *end = (uint8_t*)buf + len;

    while (buf != end) {
        val = crc_table[(uint8_t)val ^ *(uint8_t*)buf] ^ (val >> 8);
        buf = (uint8_t*)buf + 1;
    }
    return val;
}

unsigned long crc32 (unsigned long crc, const unsigned char *buf, unsigned len)
{
    if (!global_crc32_table) {
        global_crc32_table = crc32_filltable(NULL, 0);
        if(!global_crc32_table)
            return -ENOMEM;
    }
    return crc32_block_endian0( crc ^ 0xffffffffL, buf, len, global_crc32_table) ^ 0xffffffffL;
}

/*
 *  is_valid_tlv
 *
 *  Perform basic sanity checks on a TLV field. The TLV is pointed to
 *  by the parameter provided.
 *      1. The type code is not reserved (0x00 or 0xFF)
 */
static bool is_valid_tlv(tlvinfo_tlv_t *tlv)
{
    return((tlv->type != 0x00) && (tlv->type != 0xFF));
}

/*
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
static bool is_valid_tlvinfo_header(tlvinfo_header_t *hdr)
{
    int max_size = TLV_TOTAL_LEN_MAX;

    return((strcmp(hdr->signature, TLV_INFO_ID_STRING) == 0) &&
       (hdr->version == TLV_INFO_VERSION) &&
       (be16_to_cpu(hdr->totallen) <= max_size) );
}

/*
 *  is_checksum_valid
 *
 *  Validate the checksum in the provided TlvInfo EEPROM data. First,
 *  verify that the TlvInfo header is valid, then make sure the last
 *  TLV is a CRC-32 TLV. Then calculate the CRC over the EEPROM data
 *  and compare it to the value stored in the EEPROM CRC-32 TLV.
 */
static bool is_checksum_valid(uint8_t *eeprom)
{
    tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t    * eeprom_crc;
    unsigned int       calc_crc;
    unsigned int       stored_crc;

    // Is the eeprom header valid?
    if (!is_valid_tlvinfo_header(eeprom_hdr)) {
        return(FALSE);
    }

    // Is the last TLV a CRC?
    eeprom_crc = (tlvinfo_tlv_t *) &eeprom[sizeof(tlvinfo_header_t) +
                       be16_to_cpu(eeprom_hdr->totallen) -
                       (sizeof(tlvinfo_tlv_t) + 4)];
    if ((eeprom_crc->type != TLV_CODE_CRC_32) || (eeprom_crc->length != 4)) {
        return(FALSE);
    }

    // Calculate the checksum
    calc_crc = crc32(0, (void *)eeprom, sizeof(tlvinfo_header_t) +
             be16_to_cpu(eeprom_hdr->totallen) - 4);
    if(calc_crc < 0)
    {
        return FALSE;
    }
    stored_crc = ((eeprom_crc->value[0] << 24) | (eeprom_crc->value[1] << 16) |
          (eeprom_crc->value[2] <<  8) | eeprom_crc->value[3]);
    return(calc_crc == stored_crc);
}

/*
 *  decode_tlv_value
 *
 *  Decode a single TLV value into a string.

 *  The validity of EEPROM contents and the TLV field have been verified
 *  prior to calling this function.
 */
#define DECODE_NAME_MAX     20

int decode_tlv_value(tlvinfo_tlv_t * tlv, char* value)
{
    int i;
    int ret = 0;

    switch (tlv->type)
    {
        case TLV_CODE_PRODUCT_NAME:
        case TLV_CODE_PART_NUMBER:
        case TLV_CODE_SERIAL_NUMBER:
        case TLV_CODE_MANUF_DATE:
        case TLV_CODE_LABEL_REVISION:
        case TLV_CODE_PLATFORM_NAME:
        case TLV_CODE_ONIE_VERSION:
        case TLV_CODE_MANUF_NAME:
        case TLV_CODE_MANUF_COUNTRY:
        case TLV_CODE_VENDOR_NAME:
        case TLV_CODE_DIAG_VERSION:
        case TLV_CODE_SERVICE_TAG:
#ifdef C11_ANNEX_K
            if(memcpy_s(value, TLV_VALUE_MAX_LEN, tlv->value, tlv->length) != 0)
            {
                ret = -1;
            }
#else
            memcpy(value, tlv->value, tlv->length);
#endif
            value[tlv->length] = 0;
            break;
        case TLV_CODE_MAC_BASE:
#ifdef C11_ANNEX_K
            ret = sprintf_s(value, TLV_VALUE_MAX_LEN, "%02X:%02X:%02X:%02X:%02X:%02X",
                tlv->value[0], tlv->value[1], tlv->value[2],
                tlv->value[3], tlv->value[4], tlv->value[5]);
#else
            ret = sprintf(value, "%02X:%02X:%02X:%02X:%02X:%02X",
                tlv->value[0], tlv->value[1], tlv->value[2],
                tlv->value[3], tlv->value[4], tlv->value[5]);
#endif
            break;
        case TLV_CODE_DEVICE_VERSION:
#ifdef C11_ANNEX_K
            ret = sprintf_s(value, TLV_VALUE_MAX_LEN, "%u", tlv->value[0]);
#else
            ret = sprintf(value, "%u", tlv->value[0]);
#endif
            break;
        case TLV_CODE_MAC_SIZE:
#ifdef C11_ANNEX_K
            ret = sprintf_s(value, TLV_VALUE_MAX_LEN, "%u", (tlv->value[0] << 8) | tlv->value[1]);
#else
            ret = sprintf(value, "%u", (tlv->value[0] << 8) | tlv->value[1]);
#endif
            break;
        case TLV_CODE_VENDOR_EXT:
            value[0] = 0;
            for (i = 0; (i < (TLV_DECODE_VALUE_MAX_LEN/5)) && (i < tlv->length);i++)
            {
#ifdef C11_ANNEX_K
                ret = sprintf_s(value, TLV_VALUE_MAX_LEN, "%s 0x%02X", value, tlv->value[i]);
#else
			ret = sprintf(value, "%s 0x%02X", value, tlv->value[i]);
#endif
            }
            break;
        case TLV_CODE_CRC_32:
#ifdef C11_ANNEX_K
            ret = sprintf_s(value, TLV_VALUE_MAX_LEN, "0x%02X%02X%02X%02X",
                tlv->value[0], tlv->value[1], tlv->value[2],
                tlv->value[3]);
#else
            ret = sprintf(value, "0x%02X%02X%02X%02X",
                tlv->value[0], tlv->value[1], tlv->value[2],
                tlv->value[3]);
#endif
            break;
        default:
            value[0] = 0;
            for (i = 0; (i < (TLV_DECODE_VALUE_MAX_LEN/5)) && (i < tlv->length);i++)
            {
#ifdef C11_ANNEX_K
                ret = sprintf_s(value, TLV_VALUE_MAX_LEN, "%s 0x%02X", value, tlv->value[i]);
#else
			    ret = sprintf(value, "%s 0x%02X", value, tlv->value[i]);
#endif
            }
            break;
    }
    return ret;
}
EXPORT_SYMBOL_GPL(decode_tlv_value);

/*
 *  tlvinfo_find_tlv
 *
 *  This function finds the TLV with the supplied code in the EERPOM.
 *  An offset from the beginning of the EEPROM is returned in the
 *  eeprom_index parameter if the TLV is found.
 */
bool tlvinfo_find_tlv(uint8_t *eeprom, uint8_t tcode,
                 int *eeprom_index)
{
    tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t    * eeprom_tlv;
    int eeprom_end;

    // Search through the TLVs, looking for the first one which matches the
    // supplied type code.
    *eeprom_index = sizeof(tlvinfo_header_t);
    eeprom_end = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
    while (*eeprom_index < eeprom_end)
    {
        eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[*eeprom_index];
        if (!is_valid_tlv(eeprom_tlv))
        {
            return(FALSE);
        }
        if (eeprom_tlv->type == tcode)
        {
            return(TRUE);
        }
        *eeprom_index += sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
    }
    return(FALSE);
}

/*
 *  tlvinfo_decode_tlv
 *
 *  This function finds the TLV with the supplied code in the EERPOM
 *  and decodes the value into the buffer provided.
 */
bool tlvinfo_decode_tlv(uint8_t *eeprom, uint8_t tcode, char* value)
{
    int eeprom_index;
    tlvinfo_tlv_t * eeprom_tlv;

    // Find the TLV and then decode it
    if (tlvinfo_find_tlv(eeprom, tcode, &eeprom_index)) {
        eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
        if (decode_tlv_value(eeprom_tlv, value) < 0)
        {
            return FALSE;
        }
        return TRUE;
    }

    return FALSE;
}

/*
 *  update_crc
 *
 *  This function updates the CRC-32 TLV. If there is no CRC-32 TLV, then
 *  one is added. This function should be called after each update to the
 *  EEPROM structure, to make sure the CRC is always correct.
 */
static int update_crc(uint8_t *eeprom)
{
    tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t    * eeprom_crc;
    int                calc_crc;
    int                eeprom_index;

    // Discover the CRC TLV
    if (!tlvinfo_find_tlv(eeprom, TLV_CODE_CRC_32, &eeprom_index))
    {
        if ((be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + 4) > TLV_TOTAL_LEN_MAX)
        {
            return -1;
        }
        eeprom_index = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
        eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + 4);
    }
    eeprom_crc = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
    eeprom_crc->type = TLV_CODE_CRC_32;
    eeprom_crc->length = 4;

    // Calculate the checksum
    calc_crc = crc32(0, (void *)eeprom, sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen) - 4);
    if(calc_crc < 0)
    {
        return calc_crc;
    }

    eeprom_crc->value[0] = (calc_crc >> 24) & 0xFF;
    eeprom_crc->value[1] = (calc_crc >> 16) & 0xFF;
    eeprom_crc->value[2] = (calc_crc >>  8) & 0xFF;
    eeprom_crc->value[3] = (calc_crc >>  0) & 0xFF;
    return 0;
}

/*
 *  read_eeprom
 *
 *  Read the EEPROM into memory, if it hasn't already been read.
 */
int read_eeprom(uint8_t *eeprom)
{
    int ret;
    tlvinfo_header_t *eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t *eeprom_tlv = (tlvinfo_tlv_t *)&eeprom[sizeof(tlvinfo_header_t)];

    if (has_been_read)
    {
        return 0;
    }
#ifdef C11_ANNEX_K
    memset_s(eeprom, SYS_EEPROM_SIZE, 0xff, SYS_EEPROM_SIZE);
#else
    memset(eeprom, 0xff, SYS_EEPROM_SIZE);
#endif
    /* Read the header */
    ret = read_sys_eeprom((void *)eeprom_hdr, 0, sizeof(tlvinfo_header_t));
    /* If the header was successfully read, read the TLVs */
    if ((ret == 0) && is_valid_tlvinfo_header(eeprom_hdr)) 
    {
        ret = read_sys_eeprom((void *)eeprom_tlv, sizeof(tlvinfo_header_t), be16_to_cpu(eeprom_hdr->totallen));
    }

    if ( !is_valid_tlvinfo_header(eeprom_hdr) || !is_checksum_valid(eeprom) )
    {
#ifdef C11_ANNEX_K
        strcpy_s(eeprom_hdr->signature, TLV_INFO_ID_STRING_LENGTH, TLV_INFO_ID_STRING);
#else
        strcpy(eeprom_hdr->signature, TLV_INFO_ID_STRING);
#endif
        eeprom_hdr->version = TLV_INFO_VERSION;
        eeprom_hdr->totallen = cpu_to_be16(0);
        ret = update_crc(eeprom);
        if(ret < 0)
        {
            return ret;
        }
        /* Note that the contents of the hardware is not valid */
        hw_eeprom_valid = 0;
    }

    has_been_read = 1;

    return ret;
}

static ssize_t mb_read(uint8_t tcode, char *buf)
{
    char tlv_value[TLV_DECODE_VALUE_MAX_LEN];
    int len = 0;

    mutex_lock(&update_lock);

    if (read_eeprom(eeprom))
    {
        mutex_unlock(&update_lock);
        return -1;
    }

    if (!(tlvinfo_decode_tlv(eeprom, tcode, tlv_value)))
    {
        MAINBOARD_DEBUG("ERROR: TLV code not present in EEPROM: 0x%02x\n", tcode);
        mutex_unlock(&update_lock);
        return -1;
    }
#ifdef C11_ANNEX_K
    len = sprintf_s(buf, TLV_DECODE_VALUE_MAX_LEN, "%s\n", tlv_value);
#else
    len = sprintf(buf, "%s\n", tlv_value);
#endif

    mutex_unlock(&update_lock);

    return len;
}

ssize_t drv_get_odm_bsp_version(char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, TLV_VALUE_MAX_LEN, "%s\n", ODM_BSP_VERSION);
#else
    return sprintf(buf, "%s\n", ODM_BSP_VERSION);
#endif
}

ssize_t drv_get_mgmt_mac(char *buf)
{
    return mb_read(TLV_CODE_MAC_BASE, buf);
}

ssize_t drv_get_date(char *buf)
{
    return mb_read(TLV_CODE_MANUF_DATE, buf);
}

ssize_t drv_get_hw_version(char *buf)
{
    return mb_read(TLV_CODE_DEVICE_VERSION, buf);
}

ssize_t drv_get_name(char *buf)
{
	
    return mb_read(TLV_CODE_PLATFORM_NAME, buf);
}

ssize_t drv_get_sn(char *buf)
{
    return mb_read(TLV_CODE_SERIAL_NUMBER, buf);
}

ssize_t drv_get_vendor(char *buf)
{
    return mb_read(TLV_CODE_VENDOR_NAME, buf);
}


ssize_t drv_get_chassis_sn(char *buf)
{
    return 0;
}

ssize_t drv_get_syseeprom(char *buf)
{
    if (read_eeprom(eeprom))
    {
        return -1 ;
    }
#ifdef C11_ANNEX_K
    if(memcpy_s(buf, SYS_EEPROM_SIZE, eeprom, SYS_EEPROM_SIZE) != 0)
    {
        return -EINVAL;
    }
#else
    memcpy(buf, eeprom, SYS_EEPROM_SIZE);
#endif

    return SYS_EEPROM_SIZE;
}

void drv_get_loglevel(long *lev)
{
    *lev = (long)loglevel;

    return;
}

void drv_set_loglevel(long lev)
{
    loglevel = (unsigned int)lev;

    return;
}

ssize_t drv_debug_help(char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE,
        "sysname              debug cmd\n"
        "eeprom               hexdump /sys/bus/i2c/devices/0-0057/eeprom\n"
        "mgmt_mac             decode-syseeprom\n"
        "date                 decode-syseeprom\n"
        "hw_version           decode-syseeprom\n"
        "name                 decode-syseeprom\n"
        "sn                   decode-syseeprom\n"
        "vendor               decode-syseeprom\n");
#else
    return sprintf(buf,
        "sysname              debug cmd\n"
        "eeprom               hexdump /sys/bus/i2c/devices/0-0057/eeprom\n"
        "mgmt_mac             decode-syseeprom\n"
        "date                 decode-syseeprom\n"
        "hw_version           decode-syseeprom\n"
        "name                 decode-syseeprom\n"
        "sn                   decode-syseeprom\n"
        "vendor               decode-syseeprom\n");
#endif
}

ssize_t drv_debug(const char *buf, int count)
{
    return 0;
}

// For s3ip
static struct mainboard_drivers_t pfunc = {
    .get_mgmt_mac = drv_get_mgmt_mac,
    .get_date = drv_get_date,
    .get_hw_version = drv_get_hw_version,
    .get_name = drv_get_name,
    .get_sn = drv_get_sn,
    .get_vendor = drv_get_vendor,
    .get_odm_bsp_version = drv_get_odm_bsp_version,
    .get_syseeprom = drv_get_syseeprom,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
    .debug_help = drv_debug_help,
};

static int drv_mb_probe(struct platform_device *pdev)
{
    s3ip_mainboard_drivers_register(&pfunc);

    return 0;
}

static int drv_mb_remove(struct platform_device *pdev)
{
    s3ip_mainboard_drivers_unregister();

    return 0;
}

static struct platform_driver drv_mb_driver = {
    .driver = {
        .name   = DRVNAME,
        .owner = THIS_MODULE,
    },
    .probe      = drv_mb_probe,
    .remove     = drv_mb_remove,
};

static int __init drv_mb_init(void)
{
    int err=0;
    int retval=0;

    drv_mb_device = platform_device_alloc(DRVNAME, 0);
    if(!drv_mb_device)
    {
        MAINBOARD_DEBUG("%s(#%d): platform_device_alloc fail\n", __func__, __LINE__);
        return -ENOMEM;
    }

    retval = platform_device_add(drv_mb_device);
    if(retval)
    {
        MAINBOARD_DEBUG("%s(#%d): platform_device_add failed\n", __func__, __LINE__);
        err = -retval;
        goto dev_add_failed;
    }

    retval = platform_driver_register(&drv_mb_driver);
    if(retval)
    {
        MAINBOARD_DEBUG("%s(#%d): platform_driver_register failed(%d)\n", __func__, __LINE__, err);
        err = -retval;
        goto dev_reg_failed;
    }

    mutex_init(&update_lock);

    return 0;
    
dev_reg_failed:
    platform_device_unregister(drv_mb_device);
    return err;

dev_add_failed:
    platform_device_put(drv_mb_device);
    return err;
}

static void __exit drv_mb_exit(void)
{
    platform_driver_unregister(&drv_mb_driver);
    platform_device_unregister(drv_mb_device);
    mutex_destroy(&update_lock);

    return;
}

MODULE_DESCRIPTION("S3IP Main Board Driver");
MODULE_VERSION(SWITCH_MB_DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_init(drv_mb_init);
module_exit(drv_mb_exit);
