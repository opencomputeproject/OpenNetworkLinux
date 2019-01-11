/*
 * A hwmon driver for the Accton as7326 56x fan
 *
 * Copyright (C) 2014 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/dmi.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define DRVNAME "as7326_56x_fan"

#define NUM_THERMAL_SENSORS     (3)     /* Get sum of this number of sensors.*/
#define THERMAL_SENSORS_DRIVER     "lm75"
#define THERMAL_SENSORS_ADDRS   {0x48, 0x49, 0x4a}

#define		IN
#define		OUT

static struct as7326_56x_fan_data *as7326_56x_fan_update_device(struct device *dev);
static ssize_t fan_show_value(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
                              const char *buf, size_t count);
static ssize_t get_enable(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t set_enable(struct device *dev, struct device_attribute *da,
                          const char *buf, size_t count);
static ssize_t get_sys_temp(struct device *dev, struct device_attribute *da, char *buf);
extern int accton_i2c_cpld_read(unsigned short cpld_addr, u8 reg);
extern int accton_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

/* fan related data, the index should match sysfs_fan_attributes
 */
static const u8 fan_reg[] = {
    0x0F,       /* fan 1-6 present status */
    0x10,	    /* fan 1-6 direction(0:F2B 1:B2F) */
    0x11,       /* fan PWM(for all fan) */
    0x12,       /* front fan 1 speed(rpm) */
    0x13,       /* front fan 2 speed(rpm) */
    0x14,       /* front fan 3 speed(rpm) */
    0x15,       /* front fan 4 speed(rpm) */
    0x16,       /* front fan 5 speed(rpm) */
    0x17,       /* front fan 6 speed(rpm) */
    0x22,       /* rear fan 1 speed(rpm) */
    0x23,       /* rear fan 2 speed(rpm) */
    0x24,       /* rear fan 3 speed(rpm) */
    0x25,       /* rear fan 4 speed(rpm) */
    0x26,       /* rear fan 5 speed(rpm) */
    0x27,       /* rear fan 6 speed(rpm) */
};

/* Each client has this additional data */
struct as7326_56x_fan_data {
    struct device   *hwmon_dev;
    struct mutex     update_lock;
    char             valid;           /* != 0 if registers are valid */
    unsigned long    last_updated;    /* In jiffies */
    u8               reg_val[ARRAY_SIZE(fan_reg)]; /* Register value */
    u8               enable;
    int              system_temp;    /*In unit of mini-Celsius*/
    int              sensors_found;
};

enum fan_id {
    FAN1_ID,
    FAN2_ID,
    FAN3_ID,
    FAN4_ID,
    FAN5_ID,
    FAN6_ID
};

enum sysfs_fan_attributes {
    FAN_PRESENT_REG,
    FAN_DIRECTION_REG,
    FAN_DUTY_CYCLE_PERCENTAGE, /* Only one CPLD register to control duty cycle for all fans */
    FAN1_FRONT_SPEED_RPM,
    FAN2_FRONT_SPEED_RPM,
    FAN3_FRONT_SPEED_RPM,
    FAN4_FRONT_SPEED_RPM,
    FAN5_FRONT_SPEED_RPM,
    FAN6_FRONT_SPEED_RPM,
    FAN1_REAR_SPEED_RPM,
    FAN2_REAR_SPEED_RPM,
    FAN3_REAR_SPEED_RPM,
    FAN4_REAR_SPEED_RPM,
    FAN5_REAR_SPEED_RPM,
    FAN6_REAR_SPEED_RPM,
    FAN1_DIRECTION,
    FAN2_DIRECTION,
    FAN3_DIRECTION,
    FAN4_DIRECTION,
    FAN5_DIRECTION,
    FAN6_DIRECTION,
    FAN1_PRESENT,
    FAN2_PRESENT,
    FAN3_PRESENT,
    FAN4_PRESENT,
    FAN5_PRESENT,
    FAN6_PRESENT,
    FAN1_FAULT,
    FAN2_FAULT,
    FAN3_FAULT,
    FAN4_FAULT,
    FAN5_FAULT,
    FAN6_FAULT
};

/* Define attributes
 */
#define DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(index, index2) \
    static SENSOR_DEVICE_ATTR(fan##index##_fault, S_IRUGO, fan_show_value, NULL, FAN##index##_FAULT);\
    static SENSOR_DEVICE_ATTR(fan##index2##_fault, S_IRUGO, fan_show_value, NULL, FAN##index##_FAULT)
#define DECLARE_FAN_FAULT_ATTR(index, index2)      &sensor_dev_attr_fan##index##_fault.dev_attr.attr, \
                                           &sensor_dev_attr_fan##index2##_fault.dev_attr.attr

#define DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_direction, S_IRUGO, fan_show_value, NULL, FAN##index##_DIRECTION)
#define DECLARE_FAN_DIRECTION_ATTR(index)  &sensor_dev_attr_fan##index##_direction.dev_attr.attr

#define DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan_duty_cycle_percentage, S_IWUSR | S_IRUGO, fan_show_value, set_duty_cycle, FAN_DUTY_CYCLE_PERCENTAGE);\
    static SENSOR_DEVICE_ATTR(pwm##index, S_IWUSR | S_IRUGO, fan_show_value, set_duty_cycle, FAN_DUTY_CYCLE_PERCENTAGE);\
    static SENSOR_DEVICE_ATTR(pwm##index##_enable, S_IWUSR | S_IRUGO, get_enable, set_enable, FAN_DUTY_CYCLE_PERCENTAGE)

#define DECLARE_FAN_DUTY_CYCLE_ATTR(index) &sensor_dev_attr_fan_duty_cycle_percentage.dev_attr.attr, \
                                           &sensor_dev_attr_pwm##index.dev_attr.attr, \
                                           &sensor_dev_attr_pwm##index##_enable.dev_attr.attr

#define DECLARE_FAN_SYSTEM_TEMP_SENSOR_DEV_ATTR() \
    static SENSOR_DEVICE_ATTR(sys_temp, S_IRUGO, get_sys_temp, NULL, FAN_DUTY_CYCLE_PERCENTAGE)

#define DECLARE_FAN_SYSTEM_TEMP_ATTR()  &sensor_dev_attr_sys_temp.dev_attr.attr


#define DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_present, S_IRUGO, fan_show_value, NULL, FAN##index##_PRESENT)
#define DECLARE_FAN_PRESENT_ATTR(index)      &sensor_dev_attr_fan##index##_present.dev_attr.attr

#define DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(index, index2) \
    static SENSOR_DEVICE_ATTR(fan##index##_front_speed_rpm, S_IRUGO, fan_show_value, NULL, FAN##index##_FRONT_SPEED_RPM);\
    static SENSOR_DEVICE_ATTR(fan##index##_rear_speed_rpm, S_IRUGO, fan_show_value, NULL, FAN##index##_REAR_SPEED_RPM);\
    static SENSOR_DEVICE_ATTR(fan##index##_input, S_IRUGO, fan_show_value, NULL, FAN##index##_FRONT_SPEED_RPM);\
    static SENSOR_DEVICE_ATTR(fan##index2##_input, S_IRUGO, fan_show_value, NULL, FAN##index##_REAR_SPEED_RPM)
#define DECLARE_FAN_SPEED_RPM_ATTR(index, index2)  &sensor_dev_attr_fan##index##_front_speed_rpm.dev_attr.attr, \
                                           &sensor_dev_attr_fan##index##_rear_speed_rpm.dev_attr.attr, \
                                           &sensor_dev_attr_fan##index##_input.dev_attr.attr, \
                                           &sensor_dev_attr_fan##index2##_input.dev_attr.attr

/* 6 fan fault attributes in this platform */
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(1,11);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(2,12);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(3,13);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(4,14);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(5,15);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(6,16);
/* 6 fan speed(rpm) attributes in this platform */
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(1,11);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(2,12);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(3,13);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(4,14);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(5,15);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(6,16);
/* 6 fan present attributes in this platform */
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(2);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(3);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(4);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(5);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(6);
/* 6 fan direction attribute in this platform */
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(1);
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(2);
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(3);
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(4);
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(5);
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(6);
/* 1 fan duty cycle attribute in this platform */
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(1);
/* System temperature for fancontrol */
DECLARE_FAN_SYSTEM_TEMP_SENSOR_DEV_ATTR();

static struct attribute *as7326_56x_fan_attributes[] = {
    /* fan related attributes */
    DECLARE_FAN_FAULT_ATTR(1,11),
    DECLARE_FAN_FAULT_ATTR(2,12),
    DECLARE_FAN_FAULT_ATTR(3,13),
    DECLARE_FAN_FAULT_ATTR(4,14),
    DECLARE_FAN_FAULT_ATTR(5,15),
    DECLARE_FAN_FAULT_ATTR(6,16),
    DECLARE_FAN_SPEED_RPM_ATTR(1,11),
    DECLARE_FAN_SPEED_RPM_ATTR(2,12),
    DECLARE_FAN_SPEED_RPM_ATTR(3,13),
    DECLARE_FAN_SPEED_RPM_ATTR(4,14),
    DECLARE_FAN_SPEED_RPM_ATTR(5,15),
    DECLARE_FAN_SPEED_RPM_ATTR(6,16),
    DECLARE_FAN_PRESENT_ATTR(1),
    DECLARE_FAN_PRESENT_ATTR(2),
    DECLARE_FAN_PRESENT_ATTR(3),
    DECLARE_FAN_PRESENT_ATTR(4),
    DECLARE_FAN_PRESENT_ATTR(5),
    DECLARE_FAN_PRESENT_ATTR(6),
    DECLARE_FAN_DIRECTION_ATTR(1),
    DECLARE_FAN_DIRECTION_ATTR(2),
    DECLARE_FAN_DIRECTION_ATTR(3),
    DECLARE_FAN_DIRECTION_ATTR(4),
    DECLARE_FAN_DIRECTION_ATTR(5),
    DECLARE_FAN_DIRECTION_ATTR(6),
    DECLARE_FAN_DUTY_CYCLE_ATTR(1),
    DECLARE_FAN_SYSTEM_TEMP_ATTR(),
    NULL
};

#define FAN_DUTY_CYCLE_REG_MASK         0xF
#define FAN_MAX_DUTY_CYCLE              100
#define FAN_REG_VAL_TO_SPEED_RPM_STEP   100

static int as7326_56x_fan_read_value(struct i2c_client *client, u8 reg)
{
    return i2c_smbus_read_byte_data(client, reg);
}

static int as7326_56x_fan_write_value(struct i2c_client *client, u8 reg, u8 value)
{
    return i2c_smbus_write_byte_data(client, reg, value);
}

/* fan utility functions
 */
static u32 reg_val_to_duty_cycle(u8 reg_val)
{
    reg_val &= FAN_DUTY_CYCLE_REG_MASK;
    return ((u32)(reg_val+1) * 625 + 75)/ 100;
}

static u8 duty_cycle_to_reg_val(u8 duty_cycle)
{
    return ((u32)duty_cycle * 100 / 625) - 1;
}

static u32 reg_val_to_speed_rpm(u8 reg_val)
{
    return (u32)reg_val * FAN_REG_VAL_TO_SPEED_RPM_STEP;
}

static u8 reg_val_to_direction(u8 reg_val, enum fan_id id)
{
	u8 mask = (1 << id);

	reg_val &= mask;

	return reg_val ? 1 : 0;
}
static u8 reg_val_to_is_present(u8 reg_val, enum fan_id id)
{
    u8 mask = (1 << id);

    reg_val &= mask;

    return reg_val ? 0 : 1;
}

static u8 is_fan_fault(struct as7326_56x_fan_data *data, enum fan_id id)
{
    u8 ret = 1;
    int front_fan_index = FAN1_FRONT_SPEED_RPM + id;
    int rear_fan_index  = FAN1_REAR_SPEED_RPM  + id;

    /* Check if the speed of front or rear fan is ZERO,
     */
    if (reg_val_to_speed_rpm(data->reg_val[front_fan_index]) &&
            reg_val_to_speed_rpm(data->reg_val[rear_fan_index]))  {
        ret = 0;
    }

    return ret;
}

static ssize_t set_enable(struct device *dev, struct device_attribute *da,
                          const char *buf, size_t count)
{
    struct as7326_56x_fan_data *data = as7326_56x_fan_update_device(dev);
    int error, value;

    error = kstrtoint(buf, 10, &value);
    if (error)
        return error;

    if (value < 0 || value > 1)
        return -EINVAL;

    data->enable = value;
    if (value == 0)
    {
        return set_duty_cycle(dev, da, buf, FAN_MAX_DUTY_CYCLE);
    }
    return count;
}


static ssize_t get_enable(struct device *dev, struct device_attribute *da,
                          char *buf)
{
    struct as7326_56x_fan_data *data = as7326_56x_fan_update_device(dev);

    return sprintf(buf, "%u\n", data->enable);
}
static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
                              const char *buf, size_t count)
{
    int error, value;
    struct i2c_client *client = to_i2c_client(dev);

    error = kstrtoint(buf, 10, &value);
    if (error)
        return error;

    if (value < 0)
        return -EINVAL;

    value = (value > FAN_MAX_DUTY_CYCLE)? FAN_MAX_DUTY_CYCLE : value;

    as7326_56x_fan_write_value(client, 0x33, 0); /* Disable fan speed watch dog */
    as7326_56x_fan_write_value(client, fan_reg[FAN_DUTY_CYCLE_PERCENTAGE], duty_cycle_to_reg_val(value));
    return count;
}

/* Due to this struct is declared at lm75.c, it cannot be include
 * under Sonic environment. I duplicate it from lm75.c.
 */
struct lm75_data {
    struct i2c_client       *client;
    struct device           *hwmon_dev;
    struct thermal_zone_device      *tz;
    struct mutex            update_lock;
    u8                      orig_conf;
    u8                      resolution;     /* In bits, between 9 and 12 */
    u8                      resolution_limits;
    char                    valid;          /* !=0 if registers are valid */
    unsigned long           last_updated;   /* In jiffies */
    unsigned long           sample_time;    /* In jiffies */
    s16                     temp[3];        /* Register values,
                                                   0 = input
                                                   1 = max
                                                   2 = hyst */
};

/*Copied from lm75.c*/
static inline long lm75_reg_to_mc(s16 temp, u8 resolution)
{
    return ((temp >> (16 - resolution)) * 1000) >> (resolution - 8);
}

/*Get hwmon_dev from i2c_client, set hwmon_dev = NULL is failed.*/
static struct device * get_hwmon_dev(
    struct i2c_client *client)
{
    struct lm75_data *data = NULL;

    data = i2c_get_clientdata(client);
    if(data)
    {
        if( data->valid == 1  && data->hwmon_dev)
        {
            return data->hwmon_dev;
        }

    }
    return NULL;
}

/* To find hwmon index by opening hwmon under that i2c address.
 */
static int find_hwmon_index_by_FileOpen(
    int bus_nr,
    unsigned short addr,
    OUT int *index)
{
#define MAX_HWMON_DEVICE        (10)    /* Find hwmon device in 0~10*/
    struct file *sfd;
    char client_name[96];
    int  i=0;

    do {
        snprintf(client_name, sizeof(client_name),
                 "/sys/bus/i2c/devices/%d-%04x/hwmon/hwmon%d/temp1_input",
                 bus_nr, addr, i);

        sfd = filp_open(client_name, O_RDONLY, 0);
        i++;
    } while( IS_ERR(sfd) && i < MAX_HWMON_DEVICE);

    if (IS_ERR(sfd)) {
        pr_err("Failed to open file(%s)#%d\r\n", client_name, __LINE__);
        return -ENOENT;
    }
    filp_close(sfd, 0);
    *index = i - 1;
    return 0;

#undef MAX_HWMON_DEVICE
}

static int get_temp_file_path(
    int bus_nr, unsigned short addr,
    struct device *hwmon_dev
    ,char *path, int max_len)
{

    if(hwmon_dev && strlen(dev_name(hwmon_dev)))
    {
        snprintf(path, max_len,
                 "/sys/bus/i2c/devices/%d-%04x/hwmon/%s/temp1_input",
                 bus_nr, addr, dev_name(hwmon_dev));
    }
    else
    {
        int  i=0;
        if(find_hwmon_index_by_FileOpen( bus_nr, addr, &i))
        {
            return  -EIO;
        }
        snprintf(path, max_len,
                 "/sys/bus/i2c/devices/%d-%04x/hwmon/hwmon%d/temp1_input",
                 bus_nr, addr, i);
    }
    return 0;
}

/*File read the dev file at user space.*/
static int read_devfile_temp1_input(
    struct device *dev,
    int bus_nr,
    unsigned short addr,
    struct device *hwmon_dev,
    int *miniCelsius)
{
    struct file *sfd;
    char buffer[96];
    char devfile[96];
    int     rc, status;
    int     rdlen, value;
    mm_segment_t old_fs;

    rc = 0;
    get_temp_file_path(bus_nr, addr, hwmon_dev, devfile, sizeof(devfile));
    sfd = filp_open(devfile, O_RDONLY, 0);
    if (IS_ERR(sfd)) {
        pr_err("Failed to open file(%s)#%d\r\n", devfile, __LINE__);
        return -ENOENT;
    }
    dev_dbg(dev, "Found device:%s\n",devfile);

    if(!(sfd->f_op) || !(sfd->f_op->read) ) {
        pr_err("file %s cann't readable ?\n",devfile);
        return -ENOENT;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    rdlen = sfd->f_op->read(sfd, buffer, sizeof(buffer), &sfd->f_pos);
    if (rdlen == 0) {
        pr_err( "File(%s) empty!\n", devfile);
        rc = -EIO;
        goto exit;
    }
    status = sscanf(buffer, "%d", &value);
    if (status != 1) {
        rc = -EIO;
        goto exit;
    }
    *miniCelsius = value;
    dev_dbg(dev,"found sensors: %d @i2c %d-%04x\n", value, bus_nr, addr);

exit:
    set_fs(old_fs);
    filp_close(sfd, 0);
    return rc;
}

static u8 is_lm75_data_due(struct i2c_client *client)
{
    struct lm75_data *data = NULL;

    data = i2c_get_clientdata(client);
    if (time_after(jiffies, data->last_updated + data->sample_time))
    {
        return 1;
    }
    return 0;
}
static int get_lm75_temp(struct i2c_client *client, int *miniCelsius)
{
    struct lm75_data *data = NULL;

    data = i2c_get_clientdata(client);
    *miniCelsius = lm75_reg_to_mc(data->temp[0], data->resolution);

    return 0;
}

static bool lm75_addr_mached(unsigned short addr)
{
    int i;
    unsigned short addrs[] = THERMAL_SENSORS_ADDRS;
    
    for (i = 0; i < ARRAY_SIZE(addrs); i++)
    {
        if( addr == addrs[i])
            return 1;
    }
    return 0;
}

static int _find_lm75_device(struct device *dev, void *data)
{
    struct device_driver *driver;
    struct as7326_56x_fan_data *prv = data;
    char *driver_name = THERMAL_SENSORS_DRIVER;

    driver = dev->driver;
    if (driver && driver->name &&
            strcmp(driver->name, driver_name) == 0)
    {
        struct i2c_client *client;
        client = to_i2c_client(dev);
        if (client)
        {
            /*cannot use "struct i2c_adapter *adap = to_i2c_adapter(dev);"*/
            struct i2c_adapter *adap = client->adapter;
            int miniCelsius = 0;

            if (! lm75_addr_mached(client->addr))
            {
                return 0;
            }

            if (!adap) {
                return -ENXIO;
            }

            /* If the data is not updated, read them from devfile
               to drive them updateing data from chip.*/
            if (is_lm75_data_due(client))
            {
                struct device *hwmon_dev;

                hwmon_dev = get_hwmon_dev(client);
                if(0 == read_devfile_temp1_input(dev, adap->nr,
                                                 client->addr, hwmon_dev, &miniCelsius))
                {
                    prv->system_temp += miniCelsius;
                    prv->sensors_found++;
                }

            }
            else
            {
                get_lm75_temp(client, &miniCelsius);
                prv->system_temp += miniCelsius;
                prv->sensors_found++;

            }
        }
    }
    return 0;
}

/*Find all lm75 devices and return sum of temperatures.*/
static ssize_t get_sys_temp(struct device *dev, struct device_attribute *da,
                            char *buf)
{
    ssize_t ret = 0;
    struct as7326_56x_fan_data *data = as7326_56x_fan_update_device(dev);

    data->system_temp=0;
    data->sensors_found=0;
    i2c_for_each_dev(data, _find_lm75_device);
    if (NUM_THERMAL_SENSORS != data->sensors_found)
    {
        dev_dbg(dev,"only %d of %d temps are found\n",
                data->sensors_found, NUM_THERMAL_SENSORS);
        data->system_temp = INT_MAX;
    }
    ret = sprintf(buf, "%d\n",data->system_temp);
    return ret;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
                              char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct as7326_56x_fan_data *data = as7326_56x_fan_update_device(dev);
    ssize_t ret = 0;

    if (data->valid) {
        switch (attr->index) {
        case FAN_DUTY_CYCLE_PERCENTAGE:
        {
            u32 duty_cycle = reg_val_to_duty_cycle(data->reg_val[FAN_DUTY_CYCLE_PERCENTAGE]);
            ret = sprintf(buf, "%u\n", duty_cycle);
            break;
        }
        case FAN1_FRONT_SPEED_RPM:
        case FAN2_FRONT_SPEED_RPM:
        case FAN3_FRONT_SPEED_RPM:
        case FAN4_FRONT_SPEED_RPM:
        case FAN5_FRONT_SPEED_RPM:
        case FAN6_FRONT_SPEED_RPM:
        case FAN1_REAR_SPEED_RPM:
        case FAN2_REAR_SPEED_RPM:
        case FAN3_REAR_SPEED_RPM:
        case FAN4_REAR_SPEED_RPM:
        case FAN5_REAR_SPEED_RPM:
        case FAN6_REAR_SPEED_RPM:
            ret = sprintf(buf, "%u\n", reg_val_to_speed_rpm(data->reg_val[attr->index]));
            break;
        case FAN1_PRESENT:
        case FAN2_PRESENT:
        case FAN3_PRESENT:
        case FAN4_PRESENT:
        case FAN5_PRESENT:
        case FAN6_PRESENT:
            ret = sprintf(buf, "%d\n",
                          reg_val_to_is_present(data->reg_val[FAN_PRESENT_REG],
                                                attr->index - FAN1_PRESENT));
            break;
        case FAN1_FAULT:
        case FAN2_FAULT:
        case FAN3_FAULT:
        case FAN4_FAULT:
        case FAN5_FAULT:
        case FAN6_FAULT:
            ret = sprintf(buf, "%d\n", is_fan_fault(data, attr->index - FAN1_FAULT));
            break;
        case FAN1_DIRECTION:
        case FAN2_DIRECTION:
        case FAN3_DIRECTION:
        case FAN4_DIRECTION:
        case FAN5_DIRECTION:
        case FAN6_DIRECTION:
            ret = sprintf(buf, "%d\n",
                          reg_val_to_direction(data->reg_val[FAN_DIRECTION_REG],
                                               attr->index - FAN1_DIRECTION));
            break;
        default:
            break;
        }
    }

    return ret;
}

static const struct attribute_group as7326_56x_fan_group = {
    .attrs = as7326_56x_fan_attributes,
};

static struct as7326_56x_fan_data *as7326_56x_fan_update_device(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct as7326_56x_fan_data *data = i2c_get_clientdata(client);

    mutex_lock(&data->update_lock);

    if (time_after(jiffies, data->last_updated + HZ + HZ / 2) ||
            !data->valid) {
        int i;

        dev_dbg(&client->dev, "Starting as7326_56x_fan update\n");
        data->valid = 0;

        /* Update fan data
         */
        for (i = 0; i < ARRAY_SIZE(data->reg_val); i++) {
            int status = as7326_56x_fan_read_value(client, fan_reg[i]);

            if (status < 0) {
                data->valid = 0;
                mutex_unlock(&data->update_lock);
                dev_dbg(&client->dev, "reg %d, err %d\n", fan_reg[i], status);
                return data;
            }
            else {
                data->reg_val[i] = status;
            }
        }

        data->last_updated = jiffies;
        data->valid = 1;
    }

    mutex_unlock(&data->update_lock);

    return data;
}

static int as7326_56x_fan_probe(struct i2c_client *client,
                                const struct i2c_device_id *dev_id)
{
    struct as7326_56x_fan_data *data;
    int status;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        status = -EIO;
        goto exit;
    }

    data = kzalloc(sizeof(struct as7326_56x_fan_data), GFP_KERNEL);
    if (!data) {
        status = -ENOMEM;
        goto exit;
    }

    i2c_set_clientdata(client, data);
    data->valid = 0;
    data->enable = 0;
    mutex_init(&data->update_lock);

    dev_info(&client->dev, "chip found\n");

    /* Register sysfs hooks */
    status = sysfs_create_group(&client->dev.kobj, &as7326_56x_fan_group);
    if (status) {
        goto exit_free;
    }

    data->hwmon_dev = hwmon_device_register(&client->dev);
    if (IS_ERR(data->hwmon_dev)) {
        status = PTR_ERR(data->hwmon_dev);
        goto exit_remove;
    }

    dev_info(&client->dev, "%s: fan '%s'\n",
             dev_name(data->hwmon_dev), client->name);

    return 0;

exit_remove:
    sysfs_remove_group(&client->dev.kobj, &as7326_56x_fan_group);
exit_free:
    kfree(data);
exit:

    return status;
}

static int as7326_56x_fan_remove(struct i2c_client *client)
{
    struct as7326_56x_fan_data *data = i2c_get_clientdata(client);
    hwmon_device_unregister(data->hwmon_dev);
    sysfs_remove_group(&client->dev.kobj, &as7326_56x_fan_group);

    return 0;
}

/* Addresses to scan */
static const unsigned short normal_i2c[] = { 0x66, I2C_CLIENT_END };

static const struct i2c_device_id as7326_56x_fan_id[] = {
    { "as7326_56x_fan", 0 },
    {}
};
MODULE_DEVICE_TABLE(i2c, as7326_56x_fan_id);

static struct i2c_driver as7326_56x_fan_driver = {
    .class        = I2C_CLASS_HWMON,
    .driver = {
        .name     = DRVNAME,
    },
    .probe        = as7326_56x_fan_probe,
    .remove       = as7326_56x_fan_remove,
    .id_table     = as7326_56x_fan_id,
    .address_list = normal_i2c,
};

static int __init as7326_56x_fan_init(void)
{
    return i2c_add_driver(&as7326_56x_fan_driver);
}

static void __exit as7326_56x_fan_exit(void)
{
    i2c_del_driver(&as7326_56x_fan_driver);
}

module_init(as7326_56x_fan_init);
module_exit(as7326_56x_fan_exit);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as7326_56x_fan driver");
MODULE_LICENSE("GPL");

