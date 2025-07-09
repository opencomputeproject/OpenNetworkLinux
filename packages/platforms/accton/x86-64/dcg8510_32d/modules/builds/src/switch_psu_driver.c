#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>

#include "switch_cpld_driver.h"
#include "switch_psu_driver.h"
#include "sysfs_ipmi.h"

#define DRVNAME "drv_psu_driver"
#define SWITCH_PSU_DRIVER_VERSION "0.0.1"

unsigned int loglevel = 0;
static struct platform_device *drv_psu_device;
unsigned int scan_period = 5;

//TBD: the following information need to be confirmed with HW
static int psu_num_temp_sensors[PSU_TOTAL_NUM] = {
    PSU_TOTAL_TEMP_NUM,
    PSU_TOTAL_TEMP_NUM
};

static char *psu_temp_alias[PSU_TOTAL_NUM][PSU_TOTAL_TEMP_NUM] = {
    {"PSU1_8D_TEMP1", "PSU1_8E_TEMP2"},
    {"PSU2_8D_TEMP1", "PSU2_8E_TEMP2"}
};

static char *psu_temp_type[PSU_TOTAL_NUM][PSU_TOTAL_TEMP_NUM] = {
    {"psu1_temp1", "psu1_temp2"},
    {"psu2_temp1", "psu2_temp2"}
};

static int psu_temp_max[PSU_TOTAL_NUM][PSU_TOTAL_TEMP_NUM] = {
    {65000, 65000},
    {65000, 65000}
};

static int psu_temp_max_hyst[PSU_TOTAL_NUM][PSU_TOTAL_TEMP_NUM] = {
    {0, 0},
    {0, 0}
};

static int psu_temp_min[PSU_TOTAL_NUM][PSU_TOTAL_TEMP_NUM] = {
    {-20000, -20000},
    {-20000, -20000}
};

static int psu_alarm_threshold_curr[PSU_TOTAL_NUM] = {
    15000,
    15000
};

static int psu_alarm_threshold_vol[PSU_TOTAL_NUM] = {
    264000,
    264000
};
static int psu_max_output_power[PSU_TOTAL_NUM] = {
    1500000000,
    1500000000
};

static unsigned int psu_present_reg_mask_table[PSU_TOTAL_NUM] = {
    PSU_ONLINE_PSU1_MASK,
    PSU_ONLINE_PSU2_MASK,
};

static unsigned int psu_pok_reg_mask_table[PSU_TOTAL_NUM] = {
    PSU_STATUS_PSU1_ALARM,
    PSU_STATUS_PSU2_ALARM,
};

static unsigned int psu_in_pok_reg_mask_table[PSU_TOTAL_NUM] = {
    PSU_STATUS_PSU1_IPOK,
    PSU_STATUS_PSU2_IPOK,
};

static unsigned int psu_out_pok_reg_mask_table[PSU_TOTAL_NUM] = {
    PSU_STATUS_PSU1_OPOK,
    PSU_STATUS_PSU2_OPOK,
};

int drv_get_mfr_info(unsigned int psu_index, u8 pmbus_command, char *buf)
{
    int retval = 0;
/*
    retval = psu2312_mfr_read(psu_index, pmbus_command, buf);
    if(retval < 0)
    {
        PSU_DEBUG("Get psu%d mfr_info failed.\n", psu_index);
        return retval;
    }
*/
    return retval;
}

ssize_t drv_get_psu_temp_input(unsigned int psu_index, unsigned int psu_temp_index, long *temp_input)
{
    int retval = 0;
/*
    retval = psu2312_mfr_read(psu_index, pmbus_command, buf);
    if(retval < 0)
    {
        PSU_DEBUG("Get psu%d mfr_info failed.\n", psu_index);
        return retval;
    }
*/
    return retval;
}


ssize_t drv_get_psu_temp_input_from_bmc(unsigned int psu_index, unsigned int psu_temp_index, long *temp_input)
{
    int retval=0;

    if(psu_index == PSU1_INDEX)
    {
        retval = drv_get_sensor_temp_input_from_bmc(psu_temp_index + TMP75_4B_TEMP, temp_input);//PSU1_TEMP1 = 10;PSU1_TEMP2 = 11
        if(retval < 0)
            {
                PSU_DEBUG("Get psu%d temp%d input failed.\n", psu_index, psu_temp_index);
                return retval;
            }
    }

    if(psu_index == PSU2_INDEX)
    {
        retval = drv_get_sensor_temp_input_from_bmc(psu_temp_index + TMP431_4C_TEMP2, temp_input);//PSU2_TEMP1 = 12;PSU2_TEMP2 = 13
        if(retval < 0)
            {
                PSU_DEBUG("Get psu%d temp%d input failed.\n", psu_index, psu_temp_index);
                return retval;
            }
    }
    return retval;
}

ssize_t drv_get_num_temp_sensors(unsigned int psu_index, char *buf)
{
    // psu index start from 1
    if((1 <= psu_index && psu_index <= PSU_TOTAL_NUM))
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", psu_num_temp_sensors[psu_index-1]);
#else
	    return sprintf(buf, "%d\n", psu_num_temp_sensors[psu_index-1]);
#endif
    }
    else
    {
        return -EINVAL;
    }
}

ssize_t drv_get_psu_temp_alias(unsigned int psu_index, unsigned int psu_temp_index, char *buf)
{
    // psu index start from 1, but psu temp index start from 0
    if((1 <= psu_index && psu_index <= PSU_TOTAL_NUM) && (0 <= psu_temp_index && psu_temp_index <= PSU_TOTAL_TEMP_NUM))
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%s\n", psu_temp_alias[psu_index-1][psu_temp_index-1]);
#else
	    return sprintf(buf, "%s\n", psu_temp_alias[psu_index-1][psu_temp_index-1]);
#endif
    }
    else
    {
        return -EINVAL;
    }
}

ssize_t drv_get_psu_temp_type(unsigned int psu_index, unsigned int psu_temp_index, char *buf)
{
    // psu index start from 1, but psu temp index start from 0
    if((1 <= psu_index && psu_index <= PSU_TOTAL_NUM) && (0 <= psu_temp_index && psu_temp_index <= PSU_TOTAL_TEMP_NUM))
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%s\n", psu_temp_type[psu_index-1][psu_temp_index-1]);
#else
	    return sprintf(buf, "%s\n", psu_temp_type[psu_index-1][psu_temp_index-1]);
#endif
    }
    else
    {
        return -EINVAL;
    }
}

ssize_t drv_get_psu_temp_max(unsigned int psu_index, unsigned int psu_temp_index, char *buf)
{
    // psu index start from 1, but psu temp index start from 0
    if((1 <= psu_index && psu_index <= PSU_TOTAL_NUM) && (0 <= psu_temp_index && psu_temp_index <= PSU_TOTAL_TEMP_NUM))
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", psu_temp_max[psu_index-1][psu_temp_index-1]);
#else
	    return sprintf(buf, "%d\n", psu_temp_max[psu_index-1][psu_temp_index-1]);
#endif
    }
    else
    {
        return -EINVAL;
    }
}

ssize_t drv_set_psu_temp_max(unsigned int psu_index, unsigned int psu_temp_index, int val)
{
    // psu index start from 1, but psu temp index start from 0
    if((1 <= psu_index && psu_index <= PSU_TOTAL_NUM) && (0 <= psu_temp_index && psu_temp_index <= PSU_TOTAL_TEMP_NUM))
    {
        psu_temp_max[psu_index-1][psu_temp_index-1] = val;
    }
    else
    {
        return -EINVAL;
    }

    return 0;
}

ssize_t drv_get_psu_temp_max_hyst(unsigned int psu_index, unsigned int psu_temp_index, char *buf)
{
    // psu index start from 1, but psu temp index start from 0
    if((1 <= psu_index && psu_index <= PSU_TOTAL_NUM) && (0 <= psu_temp_index && psu_temp_index <= PSU_TOTAL_TEMP_NUM))
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", psu_temp_max_hyst[psu_index-1][psu_temp_index-1]);
#else
	    return sprintf(buf, "%d\n", psu_temp_max_hyst[psu_index-1][psu_temp_index-1]);
#endif
    }
    else
    {
        return -EINVAL;
    }
}

ssize_t drv_get_psu_temp_min(unsigned int psu_index, unsigned int psu_temp_index, char *buf)
{
    // psu index start from 1, but psu temp index start from 0
    if((1 <= psu_index && psu_index <= PSU_TOTAL_NUM) && (0 <= psu_temp_index && psu_temp_index <= PSU_TOTAL_TEMP_NUM))
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", psu_temp_min[psu_index-1][psu_temp_index-1]);
#else
	    return sprintf(buf, "%d\n", psu_temp_min[psu_index-1][psu_temp_index-1]);
#endif
    }
    else
    {
        return -EINVAL;
    }
}

ssize_t drv_set_psu_temp_min(unsigned int psu_index, unsigned int psu_temp_index, int val)
{
    // psu index start from 1, but psu temp index start from 0
    if((1 <= psu_index && psu_index <= PSU_TOTAL_NUM) && (0 <= psu_temp_index && psu_temp_index <= PSU_TOTAL_TEMP_NUM))
    {
        psu_temp_min[psu_index-1][psu_temp_index-1] = val;
    }
    else
    {
        return -EINVAL;
    }

    return 0;
}

ssize_t drv_get_psu_present(unsigned int psu_index, unsigned int *present)
{
    int ret;
    unsigned char buf;

    if((psu_index >= 0) && (psu_index < PSU_TOTAL_NUM))
    {
        ret = switch_cpld_reg_read(SYS_CPLD, PSU_ONLINE_OFFSET, &buf);
        if(ret < 0)
            return ret;

        /* present bit in reg: 0: not present, 1: present */
        *present = ((buf & psu_present_reg_mask_table[psu_index]) == psu_present_reg_mask_table[psu_index]) ? 0 : 1;
    }
    else
    {
        PSU_DEBUG("Invalid psu index.\n");
        return -EINVAL;
    }

    return 0;
}

ssize_t drv_get_psu_pok_alarm(unsigned int psu_index, unsigned int *alarm)
{
    int ret;
    unsigned char buf;
    
    if((psu_index >= 0) && (psu_index < PSU_TOTAL_NUM))
    {
        ret = switch_cpld_reg_read(SYS_CPLD, PSU_ONLINE_OFFSET , &buf);
        if(ret < 0)
            return ret;

        /* pok bit in reg: 0: ok, 1: not ok */
        *alarm = ((buf & psu_pok_reg_mask_table[psu_index]) == psu_pok_reg_mask_table[psu_index]) ? 0 : 1;
    }
    else
    {
        PSU_DEBUG("Invalid psu index.\n");
        return -EINVAL;
    }

    return 0;
}

ssize_t drv_get_psu_in_pok(unsigned int psu_index, unsigned int *status)
{
    int ret;
    unsigned char buf;

    if((psu_index >= 0) && (psu_index < PSU_TOTAL_NUM))
    {
        ret = switch_cpld_reg_read(SYS_CPLD, PSU_ONLINE_OFFSET, &buf);
        if(ret < 0)
            return ret;

        /* pok bit in reg: 0: not ok, 1: ok */
        *status = ((buf & psu_in_pok_reg_mask_table[psu_index]) == psu_in_pok_reg_mask_table[psu_index]) ? 1 : 0;
    }
    else
    {
        PSU_DEBUG("Invalid psu index.\n");
        return -EINVAL;
    }

    return 0;
}

ssize_t drv_get_psu_out_pok(unsigned int psu_index, unsigned int *status)
{
    int ret;
    unsigned char buf;

    if((psu_index >= 0) && (psu_index < PSU_TOTAL_NUM))
    {
        ret = switch_cpld_reg_read(SYS_CPLD, PSU_ONLINE_OFFSET, &buf);
        if(ret < 0)
            return ret;

        /* pok bit in reg: 0: not ok, 1: ok */
        *status = ((buf & psu_out_pok_reg_mask_table[psu_index]) == psu_out_pok_reg_mask_table[psu_index]) ? 1 : 0;
    }
    else
    {
        PSU_DEBUG("Invalid psu index.\n");
        return -EINVAL;
    }

    return 0;
}

bool drv_get_psu_alarm_threshold_curr(unsigned int psu_index, long *val)
{
    *val = psu_alarm_threshold_curr[psu_index-1];
    return true;
}

bool drv_get_psu_alarm_threshold_vol(unsigned int psu_index, long *val)
{
    *val = psu_alarm_threshold_vol[psu_index-1];
    return true;
}

bool drv_get_psu_max_output_power(unsigned int psu_index, long *val)
{
    *val = psu_max_output_power[psu_index-1];
    return true;
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
        "For psu1, use i2cget/i2cset -f -y 110 0x59 to debug.\n"
        "For psu2, use i2cget/i2cset -f -y 111 0x5a to debug.\n"
        "sysname                debug cmd\n"
        "psu1 model_name        i2cget -f -y 110 0x59 0x9a i 14\n"
        "psu2 model_name        i2cget -f -y 111 0x5a 0x9a i 14\n"
        "psu1 serial_number     i2cget -f -y 110 0x59 0x9e i 21\n"
        "psu2 serial_number     i2cget -f -y 111 0x5a 0x9e i 21\n"
        "psu1 date              i2cget -f -y 110 0x59 0x9d i 11\n"
        "psu2 date              i2cget -f -y 111 0x5a 0x9d i 11\n"
        "psu1 vendor            i2cget -f -y 110 0x59 0x99 i 7\n"
        "psu2 vendor            i2cget -f -y 111 0x5a 0x99 i 7\n"
        "psu1 hardware_version  i2cget -f -y 110 0x59 0x9b i 2\n"
        "psu2 hardware_version  i2cget -f -y 111 0x5a 0x9b i 2\n"
        "psu1 alarm             i2cget -f -y 110 0x59 0x79 w\n"
        "psu2 alarm             i2cget -f -y 111 0x5a 0x79 w\n"
        "psu1 max_output_power  i2cget -f -y 110 0x59 0x31 w\n"
        "psu2 max_output_power  i2cget -f -y 111 0x5a 0x31 w\n"
        "psu1 in_curr           i2cget -f -y 110 0x59 0x89 w\n"
        "psu2 in_curr           i2cget -f -y 111 0x5a 0x89 w\n"
        "psu1 in_vol            i2cget -f -y 110 0x59 0x88 w\n"
        "psu2 in_vol            i2cget -f -y 111 0x5a 0x88 w\n"
        "psu1 in_power          i2cget -f -y 110 0x59 0x97 w\n"
        "psu2 in_power          i2cget -f -y 111 0x5a 0x97 w\n"
        "psu1 out_curr          i2cget -f -y 110 0x59 0x8c w\n"
        "psu2 out_curr          i2cget -f -y 111 0x5a 0x8c w\n"
        "psu1 out_vol           i2cget -f -y 110 0x59 0x8b w\n"
        "psu2 out_vol           i2cget -f -y 111 0x5a 0x8b w\n"
        "psu1 out_power         i2cget -f -y 110 0x59 0x96 w\n"
        "psu2 out_power         i2cget -f -y 111 0x5a 0x96 w\n"
        "psu1 status            i2cget -f -y 110 0x59 0x7c\n"
        "psu2 status            i2cget -f -y 111 0x5a 0x7c\n"
        "psu1 fan_speed         i2cget -f -y 110 0x59 0x90 w\n"
        "psu2 fan_speed         i2cget -f -y 111 0x5a 0x90 w\n"
        "psu1 temp1/temp_input  i2cget -f -y 110 0x59 0x8d w\n"
        "psu2 temp1/temp_input  i2cget -f -y 111 0x5a 0x8d w\n");
#else
    return sprintf(buf,
        "For psu1, use i2cget/i2cset -f -y 110 0x59 to debug.\n"
        "For psu2, use i2cget/i2cset -f -y 111 0x5a to debug.\n"
        "sysname                debug cmd\n"
        "psu1 model_name        i2cget -f -y 110 0x59 0x9a i 14\n"
        "psu2 model_name        i2cget -f -y 111 0x5a 0x9a i 14\n"
        "psu1 serial_number     i2cget -f -y 110 0x59 0x9e i 21\n"
        "psu2 serial_number     i2cget -f -y 111 0x5a 0x9e i 21\n"
        "psu1 date              i2cget -f -y 110 0x59 0x9d i 11\n"
        "psu2 date              i2cget -f -y 111 0x5a 0x9d i 11\n"
        "psu1 vendor            i2cget -f -y 110 0x59 0x99 i 7\n"
        "psu2 vendor            i2cget -f -y 111 0x5a 0x99 i 7\n"
        "psu1 hardware_version  i2cget -f -y 110 0x59 0x9b i 2\n"
        "psu2 hardware_version  i2cget -f -y 111 0x5a 0x9b i 2\n"
        "psu1 alarm             i2cget -f -y 110 0x59 0x79 w\n"
        "psu2 alarm             i2cget -f -y 111 0x5a 0x79 w\n"
        "psu1 max_output_power  i2cget -f -y 110 0x59 0x31 w\n"
        "psu2 max_output_power  i2cget -f -y 111 0x5a 0x31 w\n"
        "psu1 in_curr           i2cget -f -y 110 0x59 0x89 w\n"
        "psu2 in_curr           i2cget -f -y 111 0x5a 0x89 w\n"
        "psu1 in_vol            i2cget -f -y 110 0x59 0x88 w\n"
        "psu2 in_vol            i2cget -f -y 111 0x5a 0x88 w\n"
        "psu1 in_power          i2cget -f -y 110 0x59 0x97 w\n"
        "psu2 in_power          i2cget -f -y 111 0x5a 0x97 w\n"
        "psu1 out_curr          i2cget -f -y 110 0x59 0x8c w\n"
        "psu2 out_curr          i2cget -f -y 111 0x5a 0x8c w\n"
        "psu1 out_vol           i2cget -f -y 110 0x59 0x8b w\n"
        "psu2 out_vol           i2cget -f -y 111 0x5a 0x8b w\n"
        "psu1 out_power         i2cget -f -y 110 0x59 0x96 w\n"
        "psu2 out_power         i2cget -f -y 111 0x5a 0x96 w\n"
        "psu1 status            i2cget -f -y 110 0x59 0x7c\n"
        "psu2 status            i2cget -f -y 111 0x5a 0x7c\n"
        "psu1 fan_speed         i2cget -f -y 110 0x59 0x90 w\n"
        "psu2 fan_speed         i2cget -f -y 111 0x5a 0x90 w\n"
        "psu1 temp1/temp_input  i2cget -f -y 110 0x59 0x8d w\n"
        "psu2 temp1/temp_input  i2cget -f -y 111 0x5a 0x8d w\n");
#endif

}

ssize_t drv_debug_help_hisonic(char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE,
        "For psu1, use i2cget/i2cset -f -y 110 0x59 to debug.\n"
        "For psu2, use i2cget/i2cset -f -y 111 0x5a to debug.\n"
        "sysname                debug cmd\n"
        "1 status               i2cget -f -y 110 0x59 0x7c\n"
        "1 currents             i2cget -f -y 110 0x59 0x8c w\n"
        "1 power                i2cget -f -y 110 0x59 0x96 w\n"
        "1 voltage              i2cget -f -y 110 0x59 0x8b w\n"
        "2 status               i2cget -f -y 111 0x5a 0x7c\n"
        "2 currents             i2cget -f -y 111 0x5a 0x8c w\n"
        "2 power                i2cget -f -y 111 0x5a 0x96 w\n"
        "2 voltage              i2cget -f -y 110 0x5a 0x8b w\n");
#else
    return sprintf(buf,
        "For psu1, use i2cget/i2cset -f -y 110 0x59 to debug.\n"
        "For psu2, use i2cget/i2cset -f -y 111 0x5a to debug.\n"
        "sysname                debug cmd\n"
        "1 status               i2cget -f -y 110 0x59 0x7c\n"
        "1 currents             i2cget -f -y 110 0x59 0x8c w\n"
        "1 power                i2cget -f -y 110 0x59 0x96 w\n"
        "1 voltage              i2cget -f -y 110 0x59 0x8b w\n"
        "2 status               i2cget -f -y 111 0x5a 0x7c\n"
        "2 currents             i2cget -f -y 111 0x5a 0x8c w\n"
        "2 power                i2cget -f -y 111 0x5a 0x96 w\n"
        "2 voltage              i2cget -f -y 110 0x5a 0x8b w\n");
#endif

}

ssize_t drv_debug(const char *buf, int count)
{
    return 0;
}

ssize_t drv_get_scan_period(char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "present soft scan time is %d x 100ms.\n", scan_period);
#else
    return sprintf(buf, "present soft scan time is %d x 100ms.\n", scan_period);
#endif
}

void drv_set_scan_period(unsigned int period)	
{
    scan_period = period;
    
    return;
}

// For s3ip
static struct psu_drivers_t pfunc = {
    .get_scan_period = drv_get_scan_period,
    .set_scan_period = drv_set_scan_period,
    .get_mfr_info = drv_get_mfr_info,
    .get_temp_input = drv_get_psu_temp_input,
    .get_num_temp_sensors = drv_get_num_temp_sensors,
    .get_psu_temp_alias = drv_get_psu_temp_alias,
    .get_psu_temp_type = drv_get_psu_temp_type,
    .get_psu_temp_max = drv_get_psu_temp_max,
    .set_psu_temp_max = drv_set_psu_temp_max,
    .get_psu_temp_max_hyst = drv_get_psu_temp_max_hyst,
    .get_psu_temp_min = drv_get_psu_temp_min,
    .set_psu_temp_min = drv_set_psu_temp_min,
    .get_psu_pok_alarm = drv_get_psu_pok_alarm,
    .get_psu_in_pok = drv_get_psu_in_pok,
    .get_psu_out_pok = drv_get_psu_out_pok,
    .get_present = drv_get_psu_present,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
    .debug_help = drv_debug_help,
    .debug_help_hisonic = drv_debug_help_hisonic,
};

static struct psu_drivers_t pfunc_bmc= {
    .get_scan_period = drv_get_scan_period,
    .set_scan_period = drv_set_scan_period,
    .get_mfr_info = drv_get_mfr_info_from_bmc,
    .get_alarm_threshold_curr = drv_get_psu_alarm_threshold_curr,
    .get_alarm_threshold_vol = drv_get_psu_alarm_threshold_vol,
    .get_max_output_power = drv_get_psu_max_output_power,
    .get_temp_input = drv_get_psu_temp_input_from_bmc,
    .get_num_temp_sensors = drv_get_num_temp_sensors,
    .get_psu_temp_alias = drv_get_psu_temp_alias,
    .get_psu_temp_type = drv_get_psu_temp_type,
    .get_psu_temp_max = drv_get_psu_temp_max,
    .set_psu_temp_max = drv_set_psu_temp_max,
    .get_psu_temp_max_hyst = drv_get_psu_temp_max_hyst,
    .get_psu_temp_min = drv_get_psu_temp_min,
    .set_psu_temp_min = drv_set_psu_temp_min,
    .get_psu_pok_alarm = drv_get_psu_pok_alarm,
    .get_psu_in_pok = drv_get_psu_in_pok,
    .get_psu_out_pok = drv_get_psu_out_pok,
    .get_present = drv_get_psu_present,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
    .debug_help = drv_debug_help,
    .debug_help_hisonic = drv_debug_help_hisonic,
};

static int drv_psu_probe(struct platform_device *pdev)
{   
    if(ipmi_bmc_is_ok())
    {
        s3ip_psu_drivers_register(&pfunc_bmc);
    }
    else
    {
        s3ip_psu_drivers_register(&pfunc);
    }
    return 0;
}

static int drv_psu_remove(struct platform_device *pdev)
{
    s3ip_psu_drivers_unregister();
    
    return 0;
}

static struct platform_driver drv_psu_driver = {
    .driver = {
        .name   = DRVNAME,
        .owner = THIS_MODULE,
    },
    .probe      = drv_psu_probe,
    .remove     = drv_psu_remove,
};

static int __init drv_psu_init(void)
{
    int err=0;
    int retval=0;
    
    drv_psu_device = platform_device_alloc(DRVNAME, 0);
    if(!drv_psu_device)
    {
        PSU_ERR("(#%d): platform_device_alloc fail\n", __LINE__);
        return -ENOMEM;
    }
    
    retval = platform_device_add(drv_psu_device);
    if(retval)
    {
        PSU_ERR("(#%d): platform_device_add failed\n", __LINE__);
        err = -retval;
        goto dev_add_failed;
    }

    retval = platform_driver_register(&drv_psu_driver);
    if(retval)
    {
        PSU_ERR("(#%d): platform_driver_register failed(%d)\n", __LINE__, err);
        err = -retval;
        goto dev_reg_failed;
    }

    return 0;
    
dev_reg_failed:
    platform_device_unregister(drv_psu_device);
    return err;
    
dev_add_failed:
    platform_device_put(drv_psu_device);
    return err;
}

static void __exit drv_psu_exit(void)
{
    platform_driver_unregister(&drv_psu_driver);
    platform_device_unregister(drv_psu_device);

    return;
}

MODULE_DESCRIPTION("S3IP PSU Driver");
MODULE_VERSION(SWITCH_PSU_DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_init(drv_psu_init);
module_exit(drv_psu_exit);
