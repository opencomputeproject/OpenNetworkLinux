/*
 * A hwmon driver for the Accton as5822 54x fan
 *
 * Copyright (C) 2016 Accton Technology Corporation.
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
#include <linux/platform_device.h>

#define DRVNAME "as5822_54x_fan"
#define FAN_STATUS_I2C_ADDR	0x60
#define MAX_FAN_SPEED_RPM	25500

static struct as5822_54x_fan_data *as5822_54x_fan_update_device(struct device *dev);                    
static ssize_t fan_show_value(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);
extern int accton_i2c_cpld_read(unsigned short cpld_addr, u8 reg);
extern int accton_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

/* fan related data, the index should match sysfs_fan_attributes
 */
static const u8 fan_reg[] = {
    0x0C,       /* fan 1-5 present status */
	0x0D,	    /* fan 1-5 direction(0:F2B 1:B2F) */
    0x0E,       /* fan PWM(for all fan) */
    0x0F,       /* front fan 1 speed(rpm) */
    0x10,       /* front fan 2 speed(rpm) */
    0x11,       /* front fan 3 speed(rpm) */
    0x12,       /* front fan 4 speed(rpm) */
    0x13,       /* front fan 5 speed(rpm) */
    0x14,       /* rear fan 1 speed(rpm) */
    0x15,       /* rear fan 2 speed(rpm) */
    0x16,       /* rear fan 3 speed(rpm) */
    0x17,       /* rear fan 4 speed(rpm) */
    0x18,       /* rear fan 5 speed(rpm) */
};

/* fan data */
struct as5822_54x_fan_data {
    struct platform_device *pdev;
    struct device   *hwmon_dev;
    struct mutex     update_lock;
    char             valid;           /* != 0 if registers are valid */
    unsigned long    last_updated;    /* In jiffies */
    u8               reg_val[ARRAY_SIZE(fan_reg)]; /* Register value */
};

static struct as5822_54x_fan_data *fan_data = NULL;

enum fan_id {
    FAN1_ID,
    FAN2_ID,
    FAN3_ID,
    FAN4_ID,
    FAN5_ID
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
    FAN1_REAR_SPEED_RPM,
    FAN2_REAR_SPEED_RPM,
    FAN3_REAR_SPEED_RPM,
    FAN4_REAR_SPEED_RPM,
    FAN5_REAR_SPEED_RPM,
	FAN1_DIRECTION,
	FAN2_DIRECTION,
	FAN3_DIRECTION,
	FAN4_DIRECTION,
	FAN5_DIRECTION,
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
    FAN_MAX_RPM
};

/* Define attributes
 */
#define DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_fault, S_IRUGO, fan_show_value, NULL, FAN##index##_FAULT)
#define DECLARE_FAN_FAULT_ATTR(index)      &sensor_dev_attr_fan##index##_fault.dev_attr.attr

#define DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_direction, S_IRUGO, fan_show_value, NULL, FAN##index##_DIRECTION)
#define DECLARE_FAN_DIRECTION_ATTR(index)  &sensor_dev_attr_fan##index##_direction.dev_attr.attr

#define DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_duty_cycle_percentage, S_IWUSR | S_IRUGO, fan_show_value, set_duty_cycle, FAN##index##_DUTY_CYCLE_PERCENTAGE)
#define DECLARE_FAN_DUTY_CYCLE_ATTR(index) &sensor_dev_attr_fan##index##_duty_cycle_percentage.dev_attr.attr

#define DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_present, S_IRUGO, fan_show_value, NULL, FAN##index##_PRESENT)
#define DECLARE_FAN_PRESENT_ATTR(index)      &sensor_dev_attr_fan##index##_present.dev_attr.attr

#define DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_front_speed_rpm, S_IRUGO, fan_show_value, NULL, FAN##index##_FRONT_SPEED_RPM);\
    static SENSOR_DEVICE_ATTR(fan##index##_rear_speed_rpm, S_IRUGO, fan_show_value, NULL, FAN##index##_REAR_SPEED_RPM)
#define DECLARE_FAN_SPEED_RPM_ATTR(index)  &sensor_dev_attr_fan##index##_front_speed_rpm.dev_attr.attr, \
                                           &sensor_dev_attr_fan##index##_rear_speed_rpm.dev_attr.attr

static SENSOR_DEVICE_ATTR(fan_max_speed_rpm, S_IRUGO, fan_show_value, NULL, FAN_MAX_RPM);
#define DECLARE_FAN_MAX_RPM_ATTR(index)      &sensor_dev_attr_fan_max_speed_rpm.dev_attr.attr

/* 6 fan fault attributes in this platform */
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(2);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(3);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(4);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(5); 
/* 6 fan speed(rpm) attributes in this platform */
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(1);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(2);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(3);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(4);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(5);
/* 6 fan present attributes in this platform */
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(2);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(3);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(4);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(5);
/* 6 fan direction attribute in this platform */
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(1);
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(2);
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(3);
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(4);
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(5);
/* 1 fan duty cycle attribute in this platform */
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR();

static struct attribute *as5822_54x_fan_attributes[] = {
    /* fan related attributes */
    DECLARE_FAN_FAULT_ATTR(1),
    DECLARE_FAN_FAULT_ATTR(2),
    DECLARE_FAN_FAULT_ATTR(3),
    DECLARE_FAN_FAULT_ATTR(4),
    DECLARE_FAN_FAULT_ATTR(5),
    DECLARE_FAN_SPEED_RPM_ATTR(1),
    DECLARE_FAN_SPEED_RPM_ATTR(2),
    DECLARE_FAN_SPEED_RPM_ATTR(3),
    DECLARE_FAN_SPEED_RPM_ATTR(4),
    DECLARE_FAN_SPEED_RPM_ATTR(5),
    DECLARE_FAN_PRESENT_ATTR(1),
    DECLARE_FAN_PRESENT_ATTR(2),
    DECLARE_FAN_PRESENT_ATTR(3),
    DECLARE_FAN_PRESENT_ATTR(4),
    DECLARE_FAN_PRESENT_ATTR(5),
	DECLARE_FAN_DIRECTION_ATTR(1),
	DECLARE_FAN_DIRECTION_ATTR(2),
	DECLARE_FAN_DIRECTION_ATTR(3),
	DECLARE_FAN_DIRECTION_ATTR(4),
	DECLARE_FAN_DIRECTION_ATTR(5),
    DECLARE_FAN_DUTY_CYCLE_ATTR(),
    DECLARE_FAN_MAX_RPM_ATTR(),
    NULL
};

#define FAN_DUTY_CYCLE_REG_MASK         0xF
#define FAN_MAX_DUTY_CYCLE              100
#define FAN_REG_VAL_TO_SPEED_RPM_STEP   100

static int as5822_54x_fan_read_value(u8 reg)
{
    return accton_i2c_cpld_read(FAN_STATUS_I2C_ADDR, reg);
}

static int as5822_54x_fan_write_value(u8 reg, u8 value)
{
    return accton_i2c_cpld_write(FAN_STATUS_I2C_ADDR, reg, value);
}

/* fan utility functions
 */
static u32 reg_val_to_duty_cycle(u8 reg_val) 
{
    reg_val &= FAN_DUTY_CYCLE_REG_MASK;

	if (reg_val == 0) {
		return 0;
	}

    return ((u32)(reg_val+1) * 625)/ 100;
}

static u8 duty_cycle_to_reg_val(u8 duty_cycle) 
{
	if (duty_cycle >= FAN_MAX_DUTY_CYCLE) {
		return 0xF;
	}
		
    return ((u32)duty_cycle * 100 / 625);
}

static u32 reg_val_to_speed_rpm(u8 reg_val)
{
    return (u32)reg_val * FAN_REG_VAL_TO_SPEED_RPM_STEP;
}

static u8 reg_val_to_direction(u8 reg_val, enum fan_id id)
{
	return !!(reg_val & BIT(id));
}

static u8 reg_val_to_is_present(u8 reg_val, enum fan_id id)
{
    return !(reg_val & BIT(id));
}

static u8 is_fan_fault(struct as5822_54x_fan_data *data, enum fan_id id)
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

    error = kstrtoint(buf, 10, &value);
    if (error) {
        return error;
    }
        
    if (value < 0 || value > FAN_MAX_DUTY_CYCLE) {
        return -EINVAL;
    }
	
    as5822_54x_fan_write_value(0x33, 0); /* Disable fan speed watch dog */
    as5822_54x_fan_write_value(fan_reg[FAN_DUTY_CYCLE_PERCENTAGE], duty_cycle_to_reg_val(value));
    return count;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct as5822_54x_fan_data *data = as5822_54x_fan_update_device(dev);
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
            case FAN1_REAR_SPEED_RPM:
            case FAN2_REAR_SPEED_RPM:
            case FAN3_REAR_SPEED_RPM:
            case FAN4_REAR_SPEED_RPM:
            case FAN5_REAR_SPEED_RPM:
                ret = sprintf(buf, "%u\n", reg_val_to_speed_rpm(data->reg_val[attr->index]));
                break;
            case FAN1_PRESENT:
            case FAN2_PRESENT:
            case FAN3_PRESENT:
            case FAN4_PRESENT:
            case FAN5_PRESENT:
                ret = sprintf(buf, "%d\n",
                              reg_val_to_is_present(data->reg_val[FAN_PRESENT_REG],
                              attr->index - FAN1_PRESENT));
                break;
            case FAN1_FAULT:
            case FAN2_FAULT:
            case FAN3_FAULT:
            case FAN4_FAULT:
            case FAN5_FAULT:
                ret = sprintf(buf, "%d\n", is_fan_fault(data, attr->index - FAN1_FAULT));
                break;
			case FAN1_DIRECTION:
			case FAN2_DIRECTION:
			case FAN3_DIRECTION:
			case FAN4_DIRECTION:
			case FAN5_DIRECTION:
				ret = sprintf(buf, "%d\n",
							  reg_val_to_direction(data->reg_val[FAN_DIRECTION_REG],
							  attr->index - FAN1_DIRECTION));
				break;
			case FAN_MAX_RPM:
				ret = sprintf(buf, "%d\n", MAX_FAN_SPEED_RPM);
            default:
                break;
        }        
    }
    
    return ret;
}

static const struct attribute_group as5822_54x_fan_group = {
    .attrs = as5822_54x_fan_attributes,
};

static struct as5822_54x_fan_data *as5822_54x_fan_update_device(struct device *dev)
{
    mutex_lock(&fan_data->update_lock);

    if (time_after(jiffies, fan_data->last_updated + HZ + HZ / 2) || 
        !fan_data->valid) {
        int i;

        dev_dbg(fan_data->hwmon_dev, "Starting as5822_54x_fan update\n");
        fan_data->valid = 0;
        
        /* Update fan data
         */
        for (i = 0; i < ARRAY_SIZE(fan_data->reg_val); i++) {
            int status = as5822_54x_fan_read_value(fan_reg[i]);
            
            if (status < 0) {
                fan_data->valid = 0;
                mutex_unlock(&fan_data->update_lock);
                dev_dbg(fan_data->hwmon_dev, "reg %d, err %d\n", fan_reg[i], status);
                return fan_data;
            }
            else {
                fan_data->reg_val[i] = status;
            }
        }
        
        fan_data->last_updated = jiffies;
        fan_data->valid = 1;
    }
    
    mutex_unlock(&fan_data->update_lock);

    return fan_data;
}

static int as5822_54x_fan_probe(struct platform_device *pdev)
{
    int status = -1;

    /* Register sysfs hooks */
    status = sysfs_create_group(&pdev->dev.kobj, &as5822_54x_fan_group);
    if (status) {
        goto exit;

    }

	fan_data->hwmon_dev = hwmon_device_register(&pdev->dev);
	if (IS_ERR(fan_data->hwmon_dev)) {
		status = PTR_ERR(fan_data->hwmon_dev);
		goto exit_remove;
	}

    dev_info(&pdev->dev, "accton_as5822_54x_fan\n");

    return 0;

exit_remove:
    sysfs_remove_group(&pdev->dev.kobj, &as5822_54x_fan_group);
exit:
    return status;
}

static int as5822_54x_fan_remove(struct platform_device *pdev)
{
    hwmon_device_unregister(fan_data->hwmon_dev);
    sysfs_remove_group(&pdev->dev.kobj, &as5822_54x_fan_group);

    return 0;
}

static struct platform_driver as5822_54x_fan_driver = {
    .probe      = as5822_54x_fan_probe,
    .remove     = as5822_54x_fan_remove,
    .driver     = {
        .name   = DRVNAME,
        .owner  = THIS_MODULE,
    },
};

static int __init as5822_54x_fan_init(void)
{
    int ret;

    ret = platform_driver_register(&as5822_54x_fan_driver);
    if (ret < 0) {
        goto exit;
    }

    fan_data = kzalloc(sizeof(struct as5822_54x_fan_data), GFP_KERNEL);
    if (!fan_data) {
        ret = -ENOMEM;
        platform_driver_unregister(&as5822_54x_fan_driver);
        goto exit;
    }

	mutex_init(&fan_data->update_lock);
    fan_data->valid = 0;

    fan_data->pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
    if (IS_ERR(fan_data->pdev)) {
        ret = PTR_ERR(fan_data->pdev);
        platform_driver_unregister(&as5822_54x_fan_driver);
        kfree(fan_data);
        goto exit;
    }

exit:
    return ret;
}

static void __exit as5822_54x_fan_exit(void)
{
    platform_device_unregister(fan_data->pdev);
    platform_driver_unregister(&as5822_54x_fan_driver);
    kfree(fan_data);
}

module_init(as5822_54x_fan_init);
module_exit(as5822_54x_fan_exit);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as5822_54x_fan driver");
MODULE_LICENSE("GPL");

