/*
 * A hwmon driver for the Accton as7936-22xke fan
 *
 * Copyright (C) 2019  Edgecore Networks Corporation.
 * Jostar Yang <jostar_yang@edge-core.com>
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
#include <linux/kernel.h>

#define DRVNAME "as7936_22xke_fan"

extern int accton_i2c_cpld_read (unsigned short cpld_addr, u8 reg);
static struct fanData *update_device(struct device *dev);
static ssize_t fan_show_value(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
                              const char *buf, size_t count);
static ssize_t show_version(struct device *dev, struct device_attribute *da,
                            char *buf);

/* fan related data, the index should match sysfs_fan_attributes
 */
static const u8 fan_reg[] = {
    0x80,       /* fan 1-8 present status in FAN board*/
    0x81,       /* fan DIR*/
    0x87,       /* fan PWM(for all fan) */
    0x93,       /* front fan 1 speed(rpm) */
    0x92,       /* front fan 2 speed(rpm) */
    0x91,       /* front fan 3 speed(rpm) */
    0x90,       /* front fan 4 speed(rpm) */
    0x94,       /* front fan 5 speed(rpm) */
    0x9B,       /* rear fan 1 speed(rpm) */
    0x9A,       /* rear fan 2 speed(rpm) */
    0x99,       /* rear fan 3 speed(rpm) */
    0x98,       /* rear fan 4 speed(rpm) */
    0x9C,       /* rear fan 5 speed(rpm) */
};

/* Each client has this additional data */
struct fanData {
    struct device   *hwmon_dev;
    struct mutex     update_lock;
    char             valid;           /* != 0 if registers are valid */
    unsigned long    last_updated;    /* In jiffies */
    u8               reg_val[ARRAY_SIZE(fan_reg)]; /* Register value */
};

enum fan_id {
    FAN1_ID,
    FAN2_ID,
    FAN3_ID,
    FAN4_ID,
    FAN5_ID,
};

enum sysfs_fan_attributes {
    FAN_PRESENT_REG,
    FAN_DIR_REG,
    FAN_DUTY_CYCLE,     /* One CPLD reg controls all fans */
    FAN1_FRONT_SPEED_RPM,
    FAN2_FRONT_SPEED_RPM,
    FAN3_FRONT_SPEED_RPM,
    FAN4_FRONT_SPEED_RPM,
    FAN5_FRONT_SPEED_RPM,
    FAN1_REAR_SPEED_RPM,
    FAN2_REAR_SPEED_RPM,
    FAN3_REAR_SPEED_RPM,
    FAN4_REAR_SPEED_RPM,
    FAN5_REAR_SPEED_RPM,  /*Above attr has coresponding register.*/
    FAN1_PRESENT,
    FAN2_PRESENT,
    FAN3_PRESENT,
    FAN4_PRESENT,
    FAN5_PRESENT,
    FAN1_FAULT,
    FAN2_FAULT,
    FAN3_FAULT,
    FAN4_FAULT,
    FAN5_FAULT,
    FAN1_DIR,
    FAN2_DIR,
    FAN3_DIR,
    FAN4_DIR,
    FAN5_DIR,
    CPLD_VERSION,
};

/* Define attributes
 */
#define DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_fault, S_IRUGO, fan_show_value, NULL, FAN##index##_FAULT)
#define DECLARE_FAN_FAULT_ATTR(index)      &sensor_dev_attr_fan##index##_fault.dev_attr.attr

#define DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_duty_cycle_percentage, S_IWUSR | S_IRUGO, fan_show_value, set_duty_cycle, FAN##index##_DUTY_CYCLE)
#define DECLARE_FAN_DUTY_CYCLE_ATTR(index) &sensor_dev_attr_fan##index##_duty_cycle_percentage.dev_attr.attr

#define DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_present, S_IRUGO, fan_show_value, NULL, FAN##index##_PRESENT)
#define DECLARE_FAN_PRESENT_ATTR(index)      &sensor_dev_attr_fan##index##_present.dev_attr.attr

#define DECLARE_FAN_DIR_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_dir, S_IRUGO, fan_show_value, NULL, FAN##index##_DIR)
#define DECLARE_FAN_DIR_ATTR(index)      &sensor_dev_attr_fan##index##_dir.dev_attr.attr

#define DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(index, index2) \
    static SENSOR_DEVICE_ATTR(fan##index##_input, S_IRUGO, fan_show_value, NULL, FAN##index##_FRONT_SPEED_RPM);\
    static SENSOR_DEVICE_ATTR(fan##index2##_input, S_IRUGO, fan_show_value, NULL, FAN##index##_REAR_SPEED_RPM)
#define DECLARE_FAN_SPEED_RPM_ATTR(index, index2)  &sensor_dev_attr_fan##index##_input.dev_attr.attr, \
                                           &sensor_dev_attr_fan##index2##_input.dev_attr.attr

DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(2);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(3);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(4);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(5);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(1, 11);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(2, 12);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(3, 13);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(4, 14);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(5, 15);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(2);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(3);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(4);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(5);
DECLARE_FAN_DIR_SENSOR_DEV_ATTR(1);
DECLARE_FAN_DIR_SENSOR_DEV_ATTR(2);
DECLARE_FAN_DIR_SENSOR_DEV_ATTR(3);
DECLARE_FAN_DIR_SENSOR_DEV_ATTR(4);
DECLARE_FAN_DIR_SENSOR_DEV_ATTR(5);
static SENSOR_DEVICE_ATTR(version, S_IRUGO, show_version, NULL, CPLD_VERSION);

/* 1 fan duty cycle attribute in this platform */
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR();
static struct attribute *attributes[] = {
    /* fan related attributes */
    DECLARE_FAN_FAULT_ATTR(1),
    DECLARE_FAN_FAULT_ATTR(2),
    DECLARE_FAN_FAULT_ATTR(3),
    DECLARE_FAN_FAULT_ATTR(4),
    DECLARE_FAN_FAULT_ATTR(5),
    DECLARE_FAN_SPEED_RPM_ATTR(1, 11),
    DECLARE_FAN_SPEED_RPM_ATTR(2, 12),
    DECLARE_FAN_SPEED_RPM_ATTR(3, 13),
    DECLARE_FAN_SPEED_RPM_ATTR(4, 14),
    DECLARE_FAN_SPEED_RPM_ATTR(5, 15),
    DECLARE_FAN_PRESENT_ATTR(1),
    DECLARE_FAN_PRESENT_ATTR(2),
    DECLARE_FAN_PRESENT_ATTR(3),
    DECLARE_FAN_PRESENT_ATTR(4),
    DECLARE_FAN_PRESENT_ATTR(5),
    DECLARE_FAN_DIR_ATTR(1),
    DECLARE_FAN_DIR_ATTR(2),
    DECLARE_FAN_DIR_ATTR(3),
    DECLARE_FAN_DIR_ATTR(4),
    DECLARE_FAN_DIR_ATTR(5),
    DECLARE_FAN_DUTY_CYCLE_ATTR(),
    &sensor_dev_attr_version.dev_attr.attr,
    NULL
};

#define FAN_MAX_DUTY_CYCLE              100
#define FAN_REG_VAL_TO_SPEED_RPM_STEP    75

static int read_value(struct i2c_client *client, u8 reg)
{
    return i2c_smbus_read_byte_data(client, reg);
}

static int write_value(struct i2c_client *client, u8 reg, u8 value)
{
    return i2c_smbus_write_byte_data(client, reg, value);
}

/* fan utility functions
 */
static u32 reg_val_to_duty_cycle(u8 reg_val)
{
    return min_t(int, (reg_val+1), FAN_MAX_DUTY_CYCLE);
}

static u8 duty_cycle_to_reg_val(u8 duty_cycle)
{
    return min_t(int, duty_cycle, FAN_MAX_DUTY_CYCLE) - 1;
}

static u32 reg_val_to_speed_rpm(u8 reg_val)
{
    return (u32)reg_val * FAN_REG_VAL_TO_SPEED_RPM_STEP;
}

static enum fan_id remap_index(enum fan_id id)
{
    if (FAN5_ID != id) {
       return (FAN4_ID - id);
    }
    return id;
}

static u8 reg_val_to_is_present(u8 reg_val, enum fan_id id)
{
    u8 mask = (1 << remap_index(id));
    return !(reg_val & mask);
}

static u8 reg_val_to_dir(u8 reg_val, enum fan_id id)
{
    u8 mask = (1 << remap_index(id));
    return !(reg_val & mask);
}

static ssize_t show_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct fanData *data = i2c_get_clientdata(client);
    int status = 0;

    mutex_lock(&data->update_lock);
    status = read_value(client, 0x01);
    if (unlikely(status < 0))
    {
        mutex_unlock(&data->update_lock);
        return status;
    }
    mutex_unlock(&data->update_lock);
    return sprintf(buf, "%d\n", status);
}

static u8 is_fan_fault(struct fanData *data, enum fan_id id)
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

static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
                              const char *buf, size_t count)
{
    int error, value;
    struct i2c_client *client = to_i2c_client(dev);

    error = kstrtoint(buf, 10, &value);
    if (error)
        return error;

    if (value < 0 || value > FAN_MAX_DUTY_CYCLE)
        return -EINVAL;

    write_value(client, 0x28, 0); /* Disable fan speed watch dog */
    write_value(client, fan_reg[FAN_DUTY_CYCLE], duty_cycle_to_reg_val(value));
    return count;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
                              char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct fanData *data = update_device(dev);
    ssize_t ret = 0;

    if (data->valid) {
        switch (attr->index) {
        case FAN_DUTY_CYCLE:
        {
            u32 duty_cycle = reg_val_to_duty_cycle(data->reg_val[FAN_DUTY_CYCLE]);
            ret = sprintf(buf, "%u\n", duty_cycle);
            break;
        }
        case FAN1_FRONT_SPEED_RPM:
        case FAN2_FRONT_SPEED_RPM:
        case FAN3_FRONT_SPEED_RPM:
        case FAN4_FRONT_SPEED_RPM:
        case FAN5_FRONT_SPEED_RPM:
        case FAN1_REAR_SPEED_RPM:
        case FAN2_REAR_SPEED_RPM:
        case FAN3_REAR_SPEED_RPM:
        case FAN4_REAR_SPEED_RPM:
        case FAN5_REAR_SPEED_RPM:
        {
            ret = sprintf(buf, "%u\n", reg_val_to_speed_rpm(data->reg_val[attr->index]));
            break;
        }
        case FAN1_PRESENT:
        case FAN2_PRESENT:
        case FAN3_PRESENT:
        case FAN4_PRESENT:
        case FAN5_PRESENT:
        {
            ret = sprintf(buf, "%d\n",
                          reg_val_to_is_present(data->reg_val[FAN_PRESENT_REG],
                                                attr->index - FAN1_PRESENT));
            /*bit0-7*/
            break;
        }
        case FAN1_DIR:
        case FAN2_DIR:
        case FAN3_DIR:
        case FAN4_DIR:
        case FAN5_DIR:
        {
            ret = sprintf(buf, "%d\n",
                          reg_val_to_dir(data->reg_val[FAN_DIR_REG],
                                         attr->index - FAN1_DIR));
            /*bit0-7*/
            break;
            break;
        }
        case FAN1_FAULT:
        case FAN2_FAULT:
        case FAN3_FAULT:
        case FAN4_FAULT:
        case FAN5_FAULT:
        {
            ret = sprintf(buf, "%d\n", is_fan_fault(data, attr->index - FAN1_FAULT));
            break;
        }
        default:
            break;
        }
    }

    return ret;
}

static struct fanData *update_device(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct fanData *data = i2c_get_clientdata(client);
    int status;

    mutex_lock(&data->update_lock);

    if (time_after(jiffies, data->last_updated + HZ + HZ / 2) ||
            !data->valid) {
        int i;

        dev_dbg(&client->dev, "Starting as7936_22xke_fan update\n");
        data->valid = 0;

        /* Update fan data
         */
        for (i = 0; i < ARRAY_SIZE(data->reg_val); i++) {
            status = read_value(client, fan_reg[i]);
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

static int as7936_22xke_fan_probe(struct i2c_client *client,
                                  const struct i2c_device_id *dev_id)
{
    struct fanData *data;
    int status;
    struct device *dev = &client->dev;
    static const struct attribute_group group = {
        .attrs = attributes,
    };

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        return -EIO;
    }

    data = devm_kzalloc(dev, sizeof(struct fanData),
                        GFP_KERNEL);
    if (!data) {
        return -ENOMEM;
    }

    i2c_set_clientdata(client, data);
    data->valid = 0;
    mutex_init(&data->update_lock);
    dev_info(dev, "chip found\n");

    /* Register sysfs hooks */
    status = devm_device_add_group(dev, &group);
    if (status) {
        return status;
    }

    data->hwmon_dev = devm_hwmon_device_register_with_info(dev, DRVNAME,
                      NULL, NULL, NULL);
    if (IS_ERR(data->hwmon_dev)) {
        dev_dbg(dev, "unable to register hwmon device\n");
        return PTR_ERR(data->hwmon_dev);
    }

    dev_info(&client->dev, "%s: fan '%s'\n",
             dev_name(data->hwmon_dev), client->name);

    return 0;

}

/* Addresses to scan */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

static const struct i2c_device_id as7936_22xke_fan_id[] = {
    { DRVNAME, 0 },
    {}
};
MODULE_DEVICE_TABLE(i2c, as7936_22xke_fan_id);

static struct i2c_driver as7936_22xke_fan_driver = {
    .class        = I2C_CLASS_HWMON,
    .driver = {   .name     = DRVNAME,},
    .probe        = as7936_22xke_fan_probe,
    .id_table     = as7936_22xke_fan_id,
    .address_list = normal_i2c,
};

static int __init as7936_22xke_fan_init(void)
{
    return i2c_add_driver(&as7936_22xke_fan_driver);
}

static void __exit as7936_22xke_fan_exit(void)
{
    i2c_del_driver(&as7936_22xke_fan_driver);
}

module_init(as7936_22xke_fan_init);
module_exit(as7936_22xke_fan_exit);

MODULE_AUTHOR("Jostar Yang <jostar_yang@edge-core.com>");
MODULE_DESCRIPTION("as7936_22xke_fan driver");
MODULE_LICENSE("GPL");

