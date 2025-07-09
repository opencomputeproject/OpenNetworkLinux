#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "switch_fan_driver.h"
#include "switch_cpld_driver.h"
#include "switch_at24.h"
#include "sysfs_ipmi.h"

#define DRVNAME "drv_fan_driver"
#define SWITCH_FAN_DRIVER_VERSION "0.0.1"

unsigned int loglevel = 0;
static struct platform_device *drv_fan_device;
unsigned int scan_period = 5;

static unsigned int fan_fmea_status[MAX_FAN_NUM] = {0};
static uint8_t fan_eeprom[MAX_FAN_NUM][FAN_EEPROM_SIZE];

/* Set to 1 if we've read EEPROM into memory */
static int has_been_read[MAX_FAN_NUM] = {0};

struct mutex     update_lock;

const char *delim = "\r\n";
const char *sn_str = "BarCode=";
const char *part_num_str = "Item=";
const char *model_str = "Model=";
const char *vendor_str = "VendorName=";
const char *boardtype_str = "BoardType=";
// there are two kind of fans, use the same threshold to check
/*
        29500*pwm-6400  32000*pwm+6400
100%    23100           38400
90%     20150           35200
80%     17200           32000
70%     14250           28800
60%     11300           25600
50%     8350            22400
40%     5400            19200
*/
//static int max_speed1 = 29500;
//static int max_speed2 = 32000;

static int fan_speed_max[MAX_FAN_NUM][MAX_MOTOR_NUM] = {
    {36000, 33000},
    {36000, 33000},
    {36000, 33000},
    {36000, 33000},
    {36000, 33000},
    {36000, 33000}
};

static int fan_speed_min[MAX_FAN_NUM][MAX_MOTOR_NUM] = {
    {5500, 5000},
    {5500, 5000},
    {5500, 5000},
    {5500, 5000},
    {5500, 5000},
    {5500, 5000}
};

static int fan_speed_tolerance[MAX_FAN_NUM][MAX_MOTOR_NUM] = {
    {3000, 2750},
    {3000, 2750},
    {3000, 2750},
    {3000, 2750},
    {3000, 2750},
    {3000, 2750}
};

static int i2c_bus_fan_eeprom[MAX_FAN_NUM] = {106, 107, 108, 109, 110};

static unsigned int fan_pwm_reg_offset_table[MAX_FAN_NUM] = {
    0x08, //FAN_PWM_CTL1_OFFSET
    0x09, //FAN_PWM_CTL2_OFFSET
    0x0a, //FAN_PWM_CTL3_OFFSET
    0x0b, //FAN_PWM_CTL4_OFFSET
    0x0c, //FAN_PWM_CTL5_OFFSET
    0x0d, //FAN_PWM_CTL6_OFFSET
};

int read_fan_eeprom(unsigned int fan_index, uint8_t *eeprom)
{
    int ret;

    if(has_been_read[fan_index-1])
    {
        /* read eeprom once the plug event has been detected */
        if(!drv_get_plug_his(fan_index))
        {
            return 0;
        }
    }

    FAN_DEBUG("Read EEPROM...\n");

    mutex_lock(&update_lock);

    /* erase the buffer before reading */
#ifdef C11_ANNEX_K
    memset_s(eeprom, FAN_EEPROM_SIZE, 0x0, FAN_EEPROM_SIZE);
#else
    memset(eeprom, 0x0, FAN_EEPROM_SIZE);
#endif
    ret = at24_read_fan_eeprom(i2c_bus_fan_eeprom[fan_index-1], eeprom, 0, FAN_EEPROM_SIZE);
    if(ret < 0)
    {
        FAN_ERR("Read EEPROM failed: %d\n", ret);
        mutex_unlock(&update_lock);
        return -1;
    }

    has_been_read[fan_index-1] = 1;

    mutex_unlock(&update_lock);

    return ret;
}

unsigned int drv_get_number(void)
{
    return MAX_FAN_NUM;
}

bool drv_get_fan_powergood(unsigned short *bitmap)
{
    return true;
}

bool drv_get_plug_his(unsigned int fan_index)
{
    return false;
}

bool drv_get_alarm(unsigned int fan_index, int *alarm)
{
    return true;
}

ssize_t drv_get_model_name(unsigned int fan_index, char *buf)
{
    int ret;
    char *token, *end; 
    char temp[FAN_EEPROM_SIZE] = {0};

    ret = read_fan_eeprom(fan_index, fan_eeprom[fan_index-1]);
    if(ret < 0)
    {
        return -1;
    }
#ifdef C11_ANNEX_K
    if(memcpy_s(temp, FAN_EEPROM_SIZE, fan_eeprom[fan_index-1], FAN_EEPROM_SIZE) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(temp, fan_eeprom[fan_index-1], FAN_EEPROM_SIZE);
#endif

    end = token = temp;
    while(token != NULL) 
    {
        strsep(&end, delim);
        if(strstr(token, boardtype_str))
        {
            break;
        }
        token = end;
    }
    if(token)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, FAN_EEPROM_SIZE, "%s\n", token+strlen(boardtype_str));
#else
	return sprintf(buf, "%s\n", token+strlen(boardtype_str));
#endif
    }

    end = token = temp;
    while(token != NULL)
    {
        strsep(&end, delim);
        if(strstr(token, model_str))
        {
            break;
        }
        token = end;
    }
    if(token)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, FAN_EEPROM_SIZE, "%s\n", token+strlen(model_str));
#else
	    return sprintf(buf, "%s\n", token+strlen(model_str));
#endif

    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, FAN_EEPROM_SIZE, "NA\n");
#else
    return sprintf(buf, "NA\n");
#endif

}

ssize_t drv_get_sn(unsigned int fan_index, char *buf)
{
    int ret;
    char *token, *end;
    char temp[FAN_EEPROM_SIZE];

    ret = read_fan_eeprom(fan_index, fan_eeprom[fan_index-1]);
    if(ret < 0)
        return -1;
#ifdef C11_ANNEX_K
    if(memcpy_s(temp, FAN_EEPROM_SIZE, fan_eeprom[fan_index-1], FAN_EEPROM_SIZE) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(temp, fan_eeprom[fan_index-1], FAN_EEPROM_SIZE);
#endif

    end = token = temp;

    while(token != NULL) 
    {
        strsep(&end, delim);

        if(strstr(token, sn_str))
            break;

        token = end;
    }

    if(token)
#ifdef C11_ANNEX_K
        return sprintf_s(buf, FAN_EEPROM_SIZE, "%s\n", token+strlen(sn_str));
#else
	    return sprintf(buf, "%s\n", token+strlen(sn_str));
#endif

    else
#ifdef C11_ANNEX_K
        return sprintf_s(buf, FAN_EEPROM_SIZE, "NA\n");
#else
	    return sprintf(buf, "NA\n");
#endif

}

ssize_t drv_get_vendor(unsigned int fan_index, char *buf)
{
    int ret;
    char *token, *end; 
    char temp[FAN_EEPROM_SIZE];

    ret = read_fan_eeprom(fan_index, fan_eeprom[fan_index-1]);
    if(ret < 0)
        return -1;
#ifdef C11_ANNEX_K
    if(memcpy_s(temp, FAN_EEPROM_SIZE, fan_eeprom[fan_index-1], FAN_EEPROM_SIZE) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(temp, fan_eeprom[fan_index-1], FAN_EEPROM_SIZE);
#endif

    end = token = temp;

    while(token != NULL) 
    {
        strsep(&end, delim);

        if(strstr(token, vendor_str))
            break;

        token = end;
    }

    if(token)
#ifdef C11_ANNEX_K
        return sprintf_s(buf, FAN_EEPROM_SIZE, "%s\n", token+strlen(vendor_str));
#else
	    return sprintf(buf, "%s\n", token+strlen(vendor_str));
#endif

    else
#ifdef C11_ANNEX_K
        return sprintf_s(buf, FAN_EEPROM_SIZE, "NA\n");
#else
	    return sprintf(buf, "NA\n");
#endif

}

ssize_t drv_get_part_number(unsigned int fan_index, char *buf)
{
    int ret;
    char *token, *end; 
    char temp[FAN_EEPROM_SIZE];

    ret = read_fan_eeprom(fan_index, fan_eeprom[fan_index-1]);
    if(ret < 0)
        return -1;
#ifdef C11_ANNEX_K
    if(memcpy_s(temp, FAN_EEPROM_SIZE, fan_eeprom[fan_index-1], FAN_EEPROM_SIZE) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(temp, fan_eeprom[fan_index-1], FAN_EEPROM_SIZE);
#endif

    end = token = temp;

    while(token != NULL) 
    {
        strsep(&end, delim);

        if(strstr(token, part_num_str))
            break;

        token = end;
    }

    if(token)
#ifdef C11_ANNEX_K
        return sprintf_s(buf, FAN_EEPROM_SIZE, "%s\n", token+strlen(part_num_str));
#else
	    return sprintf(buf, "%s\n", token+strlen(part_num_str));
#endif

    else
#ifdef C11_ANNEX_K
        return sprintf_s(buf, FAN_EEPROM_SIZE, "NA\n");
#else
	    return sprintf(buf, "NA\n");
#endif

}

ssize_t drv_get_hw_version(unsigned int fan_index, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "NA\n");
#else
    return sprintf(buf, "NA\n");
#endif

}

ssize_t drv_get_speed(unsigned int fan_index, unsigned int motor_index, unsigned int *speed)
{
    int retval=0;

    retval = drv_get_speed_from_bmc(fan_index, motor_index, speed);

    if(retval < 0)
    {
        FAN_ERR("Get fan%d motor%d speed failed.\n", fan_index, motor_index);
        return retval;
    }

    return retval;
}

ssize_t drv_get_speed_target(unsigned int fan_index, unsigned int motor_index, unsigned int *speed_target)
{
    int pwm=0;
    unsigned int retval=0;
    retval = drv_get_pwm_from_bmc(fan_index,&pwm);
    if ((retval < 0) || (pwm < 30) || (pwm > 100))
    {
        FAN_ERR("Get fan%d pwm failed.\n", fan_index);
        return -1;
    }

    if(motor_index == 1)
        *speed_target = 264*pwm + 3570;
    else if(motor_index == 2)
        *speed_target = 243*pwm + 3220;
    return 0;
}

ssize_t drv_get_speed_max(unsigned int fan_index, unsigned int motor_index, unsigned int *speed)
{
    if((1 <= fan_index && fan_index <= MAX_FAN_NUM) && (1 <= motor_index && motor_index <= MAX_MOTOR_NUM))
    {
        *speed = fan_speed_max[fan_index-1][motor_index-1];
    }
    else
    {
        return -EINVAL;
    }
    return 0;
}

ssize_t drv_get_speed_min(unsigned int fan_index, unsigned int motor_index, unsigned int *speed)
{
    if((1 <= fan_index && fan_index <= MAX_FAN_NUM) && (1 <= motor_index && motor_index <= MAX_MOTOR_NUM))
    {
        *speed = fan_speed_min[fan_index-1][motor_index-1];
    }
    else
    {
        return -EINVAL;
    }
    return 0;
}


ssize_t drv_get_speed_tolerance(unsigned int fan_index, unsigned int motor_index, unsigned int *speed_tolerance)
{
    if((1 <= fan_index && fan_index <= MAX_FAN_NUM) && (1 <= motor_index && motor_index <= MAX_MOTOR_NUM))
    {
        *speed_tolerance = fan_speed_tolerance[fan_index-1][motor_index-1];
    }
    else
    {
        return -EINVAL;
    }
    return 0;
}


ssize_t drv_get_pwm(unsigned int fan_index, int *pwm)
{
    int ret;
    unsigned char buf;
    unsigned int offset;

    offset = fan_pwm_reg_offset_table[fan_index-1];

    ret = switch_cpld_reg_read(FAN_CPLD, offset, &buf);
    if(ret < 0)
        return ret;

    /* add 1 to roundup the value for pwm consistency */
    *pwm = ((buf+1) * 100) / 255;

    return 0;
}


ssize_t drv_set_pwm(unsigned int fan_index, int pwm)
{
    unsigned int offset;
    int ret = 0;

    offset = fan_pwm_reg_offset_table[fan_index-1];

    if((pwm>100) || (pwm<0))
    {
        FAN_DEBUG("[%s] Invalid pwm value.\n", __func__);
        return false;
    }

    if(pwm < 30)
        pwm = 30;

    ret = switch_cpld_reg_write(FAN_CPLD, offset, pwm);

    return ret;
}

ssize_t drv_get_alarm_threshold(char *buf)
{
    return 0;
}

ssize_t drv_get_wind(unsigned int fan_index, unsigned int *wind)
{
    *wind = FAN_DIRECT_F2B;
    return 0;
}

ssize_t drv_get_led_status(unsigned int fan_index, unsigned int *val)
{
    return 0;
}

ssize_t drv_set_led_status(unsigned int fan_index, unsigned int led)
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

unsigned int drv_get_status(unsigned int fan_index)
{
    unsigned int speed;
    unsigned int speed_target;
    unsigned int speed_tolerance=0;
    unsigned int retval;
    unsigned int motor_index;
    bool has_failed = false;

    for(motor_index = 1; motor_index <= MAX_MOTOR_NUM; motor_index++)
    {
        retval = drv_get_speed(fan_index, motor_index-1, &speed);
        if(retval < 0)
        {
            FAN_ERR("Get fan%d motor%d speed failed.\n", fan_index, motor_index);
            return -1;
        }

        retval = drv_get_speed_target(fan_index, motor_index, &speed_target);
        if(retval < 0)
        {
            FAN_ERR("Get fan%d motor%d speed target failed.\n", fan_index, motor_index);
            return -1;
        }

        retval = drv_get_speed_tolerance(fan_index, motor_index, &speed_tolerance);
        if(retval < 0)
        {
            FAN_DEBUG("Get fan%d motor%d speed tolerance failed.\n", fan_index, motor_index);
            return -1;
        }
        FAN_DEBUG("Fan%d motor%d speed %d is out of valid range? (%d-%d).", fan_index, motor_index, speed, (speed_target - speed_tolerance), (speed_target + speed_tolerance));
        if((speed > (speed_target + speed_tolerance)) || (speed < (speed_target - speed_tolerance)))
        {
            has_failed = true;
            FAN_DEBUG("Fan%d motor%d speed %d is out of valid range (%d-%d).", fan_index, motor_index, speed, (speed_target - speed_tolerance), (speed_target + speed_tolerance));
        }
    }

    if(has_failed)
    {
        return 2; // not ok
    }
    else
    {
        return 1; // ok
    }
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
        "Relative registers: cs:0 reg: 0x1200\n"
        "Use the following command to debug:\n"
        "lpc_cpld read <cs> <reg>\n"
        "lpc_cpld write <cs> <reg> <data>\n"
        "To get the fan current speed: (output value*30 = current speed)\n"
        "sysname              debug cmd\n"
        "fan1 motor0          lpc_cpld read 0 0x1210\n"
        "fan2 motor0          lpc_cpld read 0 0x1212\n"
        "fan3 motor0          lpc_cpld read 0 0x1214\n"
        "fan4 motor0          lpc_cpld read 0 0x1216\n"
        "fan5 motor0          lpc_cpld read 0 0x1218\n"
        "To get the fan current pwm: (output value*2 = current pwm)\n"
        "fan1                 lpc_cpld read 0 0x1208\n"
        "fan2                 lpc_cpld read 0 0x1209\n"
        "fan3                 lpc_cpld read 0 0x120a\n"
        "fan4                 lpc_cpld read 0 0x120b\n"
        "fan5                 lpc_cpld read 0 0x120c\n");
#else
    return sprintf(buf,
        "Relative registers: cs:0 reg: 0x1200\n"
        "Use the following command to debug:\n"
        "lpc_cpld read <cs> <reg>\n"
        "lpc_cpld write <cs> <reg> <data>\n"
        "To get the fan current speed: (output value*30 = current speed)\n"
        "sysname              debug cmd\n"
        "fan1 motor0          lpc_cpld read 0 0x1210\n"
        "fan2 motor0          lpc_cpld read 0 0x1212\n"
        "fan3 motor0          lpc_cpld read 0 0x1214\n"
        "fan4 motor0          lpc_cpld read 0 0x1216\n"
        "fan5 motor0          lpc_cpld read 0 0x1218\n"
        "To get the fan current pwm: (output value*2 = current pwm)\n"
        "fan1                 lpc_cpld read 0 0x1208\n"
        "fan2                 lpc_cpld read 0 0x1209\n"
        "fan3                 lpc_cpld read 0 0x120a\n"
        "fan4                 lpc_cpld read 0 0x120b\n"
        "fan5                 lpc_cpld read 0 0x120c\n");
#endif

}

ssize_t drv_debug(const char *buf, int count)
{
    return 0;
}

int drv_get_fmea_status(unsigned int fan_index)
{
    return fan_fmea_status[fan_index-1];
}

void drv_set_fmea_status(unsigned int fan_index, int status)
{
    fan_fmea_status[fan_index-1] = status;
}

// For s3ip
static struct fan_drivers_t pfunc = {
    .get_scan_period = drv_get_scan_period,
    .set_scan_period = drv_set_scan_period,
    .get_model_name = drv_get_model_name,
    .get_sn = drv_get_sn,
    .get_vendor = drv_get_vendor,
    .get_part_number = drv_get_part_number,
    .get_hw_version = drv_get_hw_version,
    .get_speed = drv_get_speed,
    .get_speed_target = drv_get_speed_target,
    .get_speed_max = drv_get_speed_max,
    .get_speed_min = drv_get_speed_min,
    .get_speed_tolerance = drv_get_speed_tolerance,
    .get_pwm = drv_get_pwm,
    .set_pwm = drv_set_pwm,
    .get_wind = drv_get_wind,
    .get_led_status = drv_get_led_status,
    .set_led_status = drv_set_led_status,
    .get_status = drv_get_status,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
    .debug_help = drv_debug_help,
    .get_fmea_status = drv_get_fmea_status,
    .set_fmea_status = drv_set_fmea_status,
};

static struct fan_drivers_t pfunc_bmc = {
    .get_scan_period = drv_get_scan_period,
    .set_scan_period = drv_set_scan_period,
    .get_model_name = drv_get_model_name_from_bmc,
    .get_sn = drv_get_sn_from_bmc,
    .get_vendor = drv_get_vendor_from_bmc,
    .get_part_number = drv_get_part_number_from_bmc,
    .get_hw_version = drv_get_hw_version_from_bmc,
    .get_speed = drv_get_speed,
    .get_speed_target = drv_get_speed_target,
    .get_speed_max = drv_get_speed_max,
    .get_speed_min = drv_get_speed_min,
    .get_speed_tolerance = drv_get_speed_tolerance,
    .get_pwm = drv_get_pwm_from_bmc,
    .set_pwm = drv_set_pwm_from_bmc,
    .get_wind = drv_get_wind,
    .get_led_status = drv_get_led_status_from_bmc,
    .set_led_status = drv_set_led_status_from_bmc,
    .get_status = drv_get_status,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
    .debug_help = drv_debug_help,
    .get_fmea_status = drv_get_fmea_status,
    .set_fmea_status = drv_set_fmea_status,
};

static int drv_fan_probe(struct platform_device *pdev)
{
    if(ipmi_bmc_is_ok())
    {
        s3ip_fan_drivers_register(&pfunc_bmc);
    }
    else
    {
        s3ip_fan_drivers_register(&pfunc);
    }

    return 0;
}

static int drv_fan_remove(struct platform_device *pdev)
{
    s3ip_fan_drivers_unregister();

    return 0;
}

static struct platform_driver drv_fan_driver = {
    .driver = {
        .name   = DRVNAME,
        .owner = THIS_MODULE,
    },
    .probe      = drv_fan_probe,
    .remove     = drv_fan_remove,
};

static int __init drv_fan_init(void)
{
    int err=0;
    int retval;

    drv_fan_device = platform_device_alloc(DRVNAME, 0);
    if(!drv_fan_device)
    {
        FAN_DEBUG("%s(#%d): platform_device_alloc fail\n", __func__, __LINE__);
        return -ENOMEM;
    }

    retval = platform_device_add(drv_fan_device);
    if(retval)
    {
        FAN_DEBUG("%s(#%d): platform_device_add failed\n", __func__, __LINE__);
        err = -retval;
        goto dev_add_failed;
    }

    retval = platform_driver_register(&drv_fan_driver);
    if(retval)
    {
        FAN_DEBUG("%s(#%d): platform_driver_register failed(%d)\n", __func__, __LINE__, err);
        err = -retval;
        goto dev_reg_failed;
    }

    mutex_init(&update_lock);
    return 0;

dev_reg_failed:
    platform_device_unregister(drv_fan_device);
    return err;

dev_add_failed:
    platform_device_put(drv_fan_device);
    return err;
}

static void __exit drv_fan_exit(void)
{
    platform_driver_unregister(&drv_fan_driver);
    platform_device_unregister(drv_fan_device);

    mutex_destroy(&update_lock);
    return;
}

MODULE_DESCRIPTION("S3IP Fan Driver");
MODULE_VERSION(SWITCH_FAN_DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_init(drv_fan_init);
module_exit(drv_fan_exit);
