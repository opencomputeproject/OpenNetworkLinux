#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/i2c.h>

#include "pmbus.h"
#include "switch_avs_driver.h"
#include "sysfs_ipmi.h"

#define DRVNAME "drv_avs_driver"
#define SWITCH_AVS_DRIVER_VERSION "0.0.1"

unsigned int loglevel = 0;
static struct platform_device *drv_avs_device;

struct mutex     update_lock;

static unsigned int avs_i2c_bus[TOTAL_FMEA_AVS_NUM] = {302, 303}; //AVS I2C bus number for TD4 platform
static unsigned short avs_i2c_addr[TOTAL_FMEA_AVS_NUM] = {0x20, 0x70};


typedef struct avs_fmea_dump_reg_s {
    int page;
    int reg;
    PMBUS_PROTOCAL_TYPE_E type;
}AVS_FMEA_DUMP_REG_T;

static AVS_FMEA_DUMP_REG_T mp2882_fmea_dump_reg_list[] = {
    /* Page 0 */
    {0, 0x20, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x21, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x79, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x7A, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x7B, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x7C, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x7D, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x7E, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x88, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x8B, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x8C, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x8D, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x96, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x9A, PMBUS_PROTOCAL_TYPE_BLOCK},
    {0, 0x9D, PMBUS_PROTOCAL_TYPE_BLOCK},
    {0, 0xD0, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xD1, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xD2, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xD3, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xD4, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xD5, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xD6, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xD7, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xD8, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xD9, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xDA, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xDB, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xDC, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xDD, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xDE, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xDF, PMBUS_PROTOCAL_TYPE_WORD},
    /* Page 1 */
    {1, 0x20, PMBUS_PROTOCAL_TYPE_BYTE},
    {1, 0x21, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0x79, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0x7A, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0x7B, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0x7C, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0x7D, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0x7E, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0x88, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0x8B, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0x8C, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0x8D, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0x96, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xD0, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xD1, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xD2, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xD3, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xDA, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xDB, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xDC, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xDD, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xDE, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xDF, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xE0, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xE1, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xE2, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xE3, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xE4, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xE5, PMBUS_PROTOCAL_TYPE_WORD},
    {1, 0xE6, PMBUS_PROTOCAL_TYPE_WORD},
};

static AVS_FMEA_DUMP_REG_T mp2975_fmea_dump_reg_list[] = {
    /* Page 0 */
    {0, 0x20, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x21, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x79, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x7A, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x7B, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x7C, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x7D, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x7E, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x80, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x82, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x83, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x84, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x88, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x8B, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x8C, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x8D, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x90, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x91, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x96, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x9A, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x9D, PMBUS_PROTOCAL_TYPE_BLOCK},
    {0, 0xA4, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0xA5, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0xF4, PMBUS_PROTOCAL_TYPE_BYTE}
};

static AVS_FMEA_DUMP_REG_T ph86b03_fmea_dump_reg_list[] = {
    /* Page 0 */
    {0, 0x20, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x21, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x79, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x7A, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x7B, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x7C, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x7D, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x7E, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x80, PMBUS_PROTOCAL_TYPE_BYTE},
    {0, 0x88, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x8B, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x8C, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x8D, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x96, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0x9A, PMBUS_PROTOCAL_TYPE_BLOCK},
    {0, 0x9D, PMBUS_PROTOCAL_TYPE_BLOCK},
    {0, 0xD7, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xD8, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xDE, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xEB, PMBUS_PROTOCAL_TYPE_BLOCK},
    {0, 0xEF, PMBUS_PROTOCAL_TYPE_WORD},
    {0, 0xF4, PMBUS_PROTOCAL_TYPE_BLOCK},
    {0, 0xF9, PMBUS_PROTOCAL_TYPE_BLOCK}
};

void dump_fmea_reg(unsigned int index, AVS_FMEA_DUMP_REG_T *dump_reg_list, unsigned int size, char *buf)
{
    int i, block_index;
    int retval=0;
    int page, last_page = -1;
    int reg;
    PMBUS_PROTOCAL_TYPE_E type;
    unsigned char tmp[64] = {0};
    unsigned char val[I2C_SMBUS_BLOCK_MAX] = {0};

    for(i = 0; i < size; i++)
    {
        page = dump_reg_list[i].page;
        reg = dump_reg_list[i].reg;
        type = dump_reg_list[i].type;

        if(page != last_page)
        {
#ifdef C11_ANNEX_K
            if(sprintf_s(tmp, 64, "page%d", page) < 0)
#else
            if(sprintf(tmp, "page%d", page) < 0)
#endif
            {
                AVS_DEBUG("avs%d sprintf failed\n", index);
            }
#ifdef C11_ANNEX_K
            if(strcat_s(buf, 1024, tmp) != 0)
#else
            if(strcat(buf, tmp) != 0)
#endif
            {
                AVS_DEBUG("avs%d strcat page failed\n", index);
            }
            last_page = page;
        }
#ifdef C11_ANNEX_K
        memset_s(val, I2C_SMBUS_BLOCK_MAX, 0, sizeof(val));
#else
        memset(val, 0, sizeof(val));
#endif
        retval = pmbus_core_read_reg(avs_i2c_bus[index], avs_i2c_addr[index], page, reg, type, val);
        if(retval < 0)
        {
            // continue to dump other registers
            AVS_DEBUG("Read avs%d page %d reg 0x%x type %d failed\n", index, page, reg, type);
            continue;
        }
#ifdef C11_ANNEX_K
        memset_s(tmp, 64, 0, sizeof(tmp));
#else
        memset(tmp, 0, sizeof(tmp));
#endif
        if(type == PMBUS_PROTOCAL_TYPE_BYTE)
        {
#ifdef C11_ANNEX_K
            if(sprintf_s(tmp, 64, " 0x%x: 0x%02x", reg, val[0]) < 0)
#else
            if(sprintf(tmp, " 0x%x: 0x%02x", reg, val[0]) < 0)
#endif
            {
                AVS_DEBUG("avs%d sprintf failed\n", index);
            }
#ifdef C11_ANNEX_K
            if(strcat_s(buf, 1024, tmp) != 0)
#else
            if(strcat(buf, tmp) != 0)
#endif
            {
                AVS_DEBUG("avs%d strcat_s failed\n", index);
                continue;
            }
        }
        else if(type == PMBUS_PROTOCAL_TYPE_WORD)
        {
#ifdef C11_ANNEX_K
            if(sprintf_s(tmp, 64, " 0x%x: 0x%02x%02x", reg, val[1], val[0]) < 0)
#else
            if(sprintf(tmp, " 0x%x: 0x%02x%02x", reg, val[1], val[0]) < 0)
#endif
            {
                AVS_DEBUG("avs%d sprintf failed\n", index);
            }
#ifdef C11_ANNEX_K
            if(strcat_s(buf, 1024, tmp) != 0)
#else
            if(strcat(buf, tmp) != 0)
#endif
            {
                AVS_DEBUG("avs%d strcat failed\n", index);
                continue;
            }
        }
        else if(type == PMBUS_PROTOCAL_TYPE_BLOCK)
        {
#ifdef C11_ANNEX_K
            if(sprintf_s(tmp, 64, " 0x%x: 0x", reg) < 0)
#else
            if(sprintf(tmp, " 0x%x: 0x", reg) < 0)
#endif
            {
                AVS_DEBUG("avs%d sprintf failed\n", index);
            }
#ifdef C11_ANNEX_K
            if(strcat_s(buf, 1024, tmp) != 0)
#else
            if(strcat(buf, tmp) != 0)
#endif
            {
                AVS_DEBUG("avs%d strcat failed\n", index);
                continue;
            }

            for(block_index = 0; block_index < retval ; block_index++)
            {
#ifdef C11_ANNEX_K
                memset_s(tmp, 64, 0, sizeof(tmp));
#else
                memset(tmp, 0, sizeof(tmp));
#endif
#ifdef C11_ANNEX_K
                if(sprintf_s(tmp, 64, "%02x", val[block_index]) < 0)
#else
                if(sprintf(tmp, "%02x", val[block_index]) < 0)
#endif
                {
                    AVS_DEBUG("avs%d sprintf block failed\n", index);
                }
#ifdef C11_ANNEX_K
                if(strcat_s(buf, 1024, tmp) != 0)
#else
                if(strcat(buf, tmp) != 0)
#endif
                {
                    AVS_DEBUG("avs%d strcat block failed\n", index);
                    break;
                }
            }
        }
    }

    return;
}
EXPORT_SYMBOL_GPL(dump_fmea_reg);

ssize_t drv_fmea_get_work_status(unsigned int index, char *buf, char *plt)
{
    int page, num_page, num_page_mfr_specific = 0;
    int check_status_bmp, check_status_mfr_specific_bmp = 0, check_temperature;
    int mfr_id = 0;
    int retval=0;
    int val;
    long long_value;
    char tmp_buf[1024] = {0};
    AVS_FMEA_DUMP_REG_T *dump_reg_list = NULL;
    unsigned int dump_size = 0;

    if(index >= TOTAL_FMEA_AVS_NUM)
    {
        AVS_DEBUG("Invalid avs index %d.\n", index);
        return -EPERM;
    }

    mutex_lock(&update_lock);

    /* Get MFR_ID (0x99) in word */
    retval = pmbus_core_read_mfr_id(avs_i2c_bus[index], avs_i2c_addr[index], &mfr_id);
    if(retval < 0)
    {
        AVS_DEBUG("Get avs%d mfr_id failed.\n", index);
        mutex_unlock(&update_lock);
        return -EPERM;
    }

    switch(mfr_id)
    {
        case PMBUS_AVS_MFR_ID_MP2882:
            num_page = 2;
            check_status_bmp = 0xF73D;  /* Ignore bit1, bit6, bit7, bit11 for MP2882 STATUS */
            check_temperature = 112000; /* 112 celsius*/
            dump_reg_list = mp2882_fmea_dump_reg_list;
            dump_size = (sizeof(mp2882_fmea_dump_reg_list)/sizeof(AVS_FMEA_DUMP_REG_T));
            break;
        case PMBUS_AVS_MFR_ID_MP2975:
            num_page = 1;
            check_status_bmp = 0xD33D;  /* Ignore bit1, bit6, bit7, bit10, bit11, bit13 for MP2975 STATUS */
            check_temperature = 112000; /* 112 celsius*/
            dump_reg_list = mp2975_fmea_dump_reg_list;
            dump_size = (sizeof(mp2975_fmea_dump_reg_list)/sizeof(AVS_FMEA_DUMP_REG_T));
            break;
        case PMBUS_AVS_MFR_ID_PH86B03:
            num_page = 1;
            num_page_mfr_specific = 1;
            check_status_bmp = 0xF7BE; /* Ignore bit0, bit6, bit11 for PH86B03 STATUS */
            check_status_mfr_specific_bmp = 0xDF; /* Ignore bit5 for PH86B03 STATUS_MFR_SPECIFIC */
            check_temperature = 112000; /* 112 celsius*/
            dump_reg_list = ph86b03_fmea_dump_reg_list;
            dump_size = (sizeof(ph86b03_fmea_dump_reg_list)/sizeof(AVS_FMEA_DUMP_REG_T));
            break;
        default:
            AVS_DEBUG("Unsupported avs%d mfr_id:0x%04X failed.\n", index, mfr_id);
            mutex_unlock(&update_lock);
            return -EPERM;
    }

    for(page = 0; page < num_page; page++)
    {
        /* Check PMBUS_STATUS_WORD (0x79) */
        retval = pmbus_core_read_reg(avs_i2c_bus[index], avs_i2c_addr[index], page, PMBUS_STATUS_WORD, PMBUS_PROTOCAL_TYPE_WORD, &val);
        if(retval < 0)
        {
            AVS_DEBUG("Read avs%d mfr_id:0x%04X page %d PMBUS_STATUS_WORD failed\n", index, mfr_id, page);
            mutex_unlock(&update_lock);
            return retval;
        }

        if((val & check_status_bmp) != 0)
        {
            AVS_DEBUG("%s %d, retval&0x%04X:0x%x", __func__, __LINE__, check_status_bmp, (val & check_status_bmp));
            goto work_abnormal;
        }

        /* Check PMBUS_READ_TEMPERATURE_1 (0x8D) */
        retval = pmbus_core_read_attrs_by_reg(avs_i2c_bus[index], avs_i2c_addr[index], 0, PMBUS_READ_TEMPERATURE_1, &long_value);
        if(retval < 0)
        {
            AVS_DEBUG("Read avs%d mfr_id:0x%04X page %d PMBUS_READ_TEMPERATURE_1 failed\n", index, mfr_id, page);
            mutex_unlock(&update_lock);
            return retval;
        }

        if((int)long_value > check_temperature)
        {
            AVS_DEBUG("%s %d, retval:%d > %d\n", __func__, __LINE__, (int)long_value, check_temperature);
            goto work_abnormal;
        }
    }

    for(page = 0; page < num_page_mfr_specific; page++)
    {
        /* Check custom register */
        switch(mfr_id)
        {
            case PMBUS_AVS_MFR_ID_PH86B03:
                /* Check STATUS_MFR_SPECIFIC (0x80) */
                retval = pmbus_core_read_reg(avs_i2c_bus[index], avs_i2c_addr[index], page, 0x80, PMBUS_PROTOCAL_TYPE_BYTE, &val);
                if(retval < 0)
                {
                    AVS_DEBUG("Read avs%d mfr_id:0x%04X page %d STATUS_MFR_SPECIFIC failed\n", index, mfr_id, page);
                    mutex_unlock(&update_lock);
                    return retval;
                }

                if((val & check_status_mfr_specific_bmp) != 0)
                {
                    AVS_DEBUG("%s %d, retval&0x%04X:0x%x", __func__, __LINE__, check_status_mfr_specific_bmp, (val & check_status_mfr_specific_bmp));
                    goto work_abnormal;
                }
                break;
            case PMBUS_AVS_MFR_ID_MP2882:
            case PMBUS_AVS_MFR_ID_MP2975:
                /* do nothing */
                break;
            default:
                /* do nothing */
                break;
        }
    }

    mutex_unlock(&update_lock);
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", "0 0x00000000");
#else
    return sprintf(buf, "%s\n", "0 0x00000000");
#endif
 
work_abnormal:
    dump_fmea_reg(index, dump_reg_list, dump_size, tmp_buf);
    pmbus_core_clear_fault(avs_i2c_bus[index], avs_i2c_addr[index]);
    mutex_unlock(&update_lock);
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "1 %s\n", tmp_buf);
#else
    return sprintf(buf, "1 %s\n", tmp_buf);
#endif

}
EXPORT_SYMBOL_GPL(drv_fmea_get_work_status);

ssize_t drv_fmea_get_current_status(unsigned int index, char *buf, char *plt)
{
    int retval=0;
    int mfr_id = 0;
    int multiplier = 1000;
    int vout, iout, pout;
    int iout_threshold = 0;
    long val;

    if(index >= TOTAL_FMEA_AVS_NUM)
    {
        AVS_DEBUG("Invalid avs index %d.\n", index);
        return -EPERM;
    }

    mutex_lock(&update_lock);

    /* Get MFR_ID (0x99) in word */
    retval = pmbus_core_read_mfr_id(avs_i2c_bus[index], avs_i2c_addr[index], &mfr_id);
    if(retval < 0)
    {
        AVS_DEBUG("Get avs%d mfr_id failed.\n", index);
        mutex_unlock(&update_lock);
        return -EPERM;
    }

    switch(mfr_id)
    {
        case PMBUS_AVS_MFR_ID_MP2882: /* MP2882 */
            iout_threshold = 589000; /* (491A * 120%) */
            break;
        case PMBUS_AVS_MFR_ID_MP2975: /* MP2975 */
        case PMBUS_AVS_MFR_ID_PH86B03: /* PH86B03 */
            iout_threshold = 63000; /* ((41W/0.77V) * 120%) */
            break;
        default:
            AVS_DEBUG("Unsupported avs%d mfr_id:0x%04X failed.\n", index, mfr_id);
            mutex_unlock(&update_lock);
            return -EPERM;
    }

    retval = pmbus_core_read_attrs_by_reg(avs_i2c_bus[index], avs_i2c_addr[index], 0, PMBUS_READ_IOUT, &val);
    if(retval < 0)
    {
        AVS_DEBUG("Read avs%d (mfr_id:0x%04X) page 0 Iout failed\n", index, mfr_id);
        mutex_unlock(&update_lock);
        return retval;
    }
    mutex_unlock(&update_lock);

    iout = (int)val;

    if(iout > iout_threshold)
    {
        mutex_lock(&update_lock);
        retval = pmbus_core_read_attrs_by_reg(avs_i2c_bus[index], avs_i2c_addr[index], 0, PMBUS_READ_VOUT, &val);
        if(retval < 0)
        {
            AVS_DEBUG("Read avs%d (mfr_id:0x%04X) page 0 Vout failed\n", index, mfr_id);
            mutex_unlock(&update_lock);
            return retval;
        }
        mutex_unlock(&update_lock);

        vout = (int)val;

        mutex_lock(&update_lock);
        retval = pmbus_core_read_attrs_by_reg(avs_i2c_bus[index], avs_i2c_addr[index], 0, PMBUS_READ_POUT, &val);
        if(retval < 0)
        {
            AVS_DEBUG("Read avs%d (mfr_id:0x%04X) page 0 Pout failed\n", index, mfr_id);
            mutex_unlock(&update_lock);
            return retval;
        }
        mutex_unlock(&update_lock);

        pout = (int)val;
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "1 %d.%03d(V) %s%d.%03d(A) %s%d.%03d(W)\n", (vout / multiplier), (vout % multiplier)
                                                                      , ((iout < 0) ? "-":""), abs(iout / multiplier), abs(iout % multiplier)
                                                                      , ((pout < 0) ? "-":""), abs(pout / (multiplier * multiplier)), ((abs(pout % (multiplier * multiplier))) / multiplier));
#else
	    return sprintf(buf, "1 %d.%03d(V) %s%d.%03d(A) %s%d.%03d(W)\n", (vout / multiplier), (vout % multiplier)
																      , ((iout < 0) ? "-":""), abs(iout / multiplier), abs(iout % multiplier)
																      , ((pout < 0) ? "-":""), abs(pout / (multiplier * multiplier)), ((abs(pout % (multiplier * multiplier))) / multiplier));
#endif
    }
    else
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%s\n", "0 0x00000000");
#else
	    return sprintf(buf, "%s\n", "0 0x00000000");
#endif
}
EXPORT_SYMBOL_GPL(drv_fmea_get_current_status);

ssize_t drv_fmea_get_pmbus_status(unsigned int index, char *buf, char *plt)
{
    int retval=0;
    int mfr_id = 0;
    bool match = false;

    if(index >= TOTAL_FMEA_AVS_NUM)
    {
        AVS_DEBUG("Invalid avs index %d.\n", index);
        return -EPERM;
    }

    mutex_lock(&update_lock);

    /* Get MFR_ID (0x99) in word */
    retval = pmbus_core_read_mfr_id(avs_i2c_bus[index], avs_i2c_addr[index], &mfr_id);
    if(retval < 0)
    {
        AVS_DEBUG("Get avs%d mfr_id failed.\n", index);
        mutex_unlock(&update_lock);
        return -EPERM;
    }

    switch(mfr_id)
    {
        case PMBUS_AVS_MFR_ID_MP2882: /* MP2882 */
        case PMBUS_AVS_MFR_ID_MP2975: /* MP2975 */
        case PMBUS_AVS_MFR_ID_PH86B03: /* PH86B03 */
            match = true;
            break;
        default:
            AVS_DEBUG("Unsupported avs%d mfr_id:0x%04X failed.\n", index, mfr_id);
            break;
    }

    mutex_unlock(&update_lock);

    if(match)
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%s\n", "0 0x00000000");
#else
	    return sprintf(buf, "%s\n", "0 0x00000000");
#endif

    else
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "1 0x%04X\n", mfr_id);
#else
	    return sprintf(buf, "1 0x%04X\n", mfr_id);
#endif

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

// For S3IP sonic
static struct avs_drivers_t pfunc = {
    .fmea_get_work_status = drv_fmea_get_work_status,
    .fmea_get_current_status = drv_fmea_get_current_status,
    .fmea_get_pmbus_status = drv_fmea_get_pmbus_status,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
};

static struct avs_drivers_t pfunc_bmc = {
    .fmea_get_work_status = drv_fmea_get_work_status_from_bmc,
    .fmea_get_current_status = drv_fmea_get_current_status_from_bmc,
    .fmea_get_pmbus_status = drv_fmea_get_pmbus_status_from_bmc,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
};

static int drv_avs_probe(struct platform_device *pdev)
{
    if(ipmi_bmc_is_ok())
    {
        s3ip_avs_drivers_register(&pfunc_bmc);
    }
    else
    {
        s3ip_avs_drivers_register(&pfunc);
    }

    return 0;
}

static int drv_avs_remove(struct platform_device *pdev)
{
    s3ip_avs_drivers_unregister();

    return 0;
}

static struct platform_driver drv_avs_driver = {
    .driver = {
        .name   = DRVNAME,
        .owner = THIS_MODULE,
    },
    .probe      = drv_avs_probe,
    .remove     = drv_avs_remove,
};

static int __init drv_avs_init(void)
{
    int err=0;
    int retval=0;

    drv_avs_device = platform_device_alloc(DRVNAME, 0);
    if(!drv_avs_device)
    {
        AVS_DEBUG("%s(#%d): platform_device_alloc fail\n", __func__, __LINE__);
        return -ENOMEM;
    }

    retval = platform_device_add(drv_avs_device);
    if(retval)
    {
        AVS_DEBUG("%s(#%d): platform_device_add failed\n", __func__, __LINE__);
        err = -retval;
        goto dev_add_failed;
    }

    retval = platform_driver_register(&drv_avs_driver);
    if(retval)
    {
        AVS_DEBUG("%s(#%d): platform_driver_register failed(%d)\n", __func__, __LINE__, err);
        err = -retval;
        goto dev_reg_failed;
    }

    mutex_init(&update_lock);

    return 0;

dev_reg_failed:
    platform_device_unregister(drv_avs_device);
    return err;

dev_add_failed:
    platform_device_put(drv_avs_device);
    return err;
}

static void __exit drv_avs_exit(void)
{
    platform_driver_unregister(&drv_avs_driver);
    platform_device_unregister(drv_avs_device);

    mutex_destroy(&update_lock);

    return;
}

MODULE_DESCRIPTION("S3IP AVS Driver");
MODULE_VERSION(SWITCH_AVS_DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_init(drv_avs_init);
module_exit(drv_avs_exit);
