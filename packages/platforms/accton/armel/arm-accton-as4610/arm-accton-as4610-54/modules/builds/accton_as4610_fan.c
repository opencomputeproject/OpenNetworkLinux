/*
 * A hwmon driver for the Accton as4610 fan
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
#include <linux/platform_device.h>
#include "accton_i2c_cpld.h"

#define DRVNAME "as4610_fan"


static struct as4610_fan_data *as4610_fan_update_device(struct device *dev);
static ssize_t fan_show_value(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);

/* fan related data, the index should match sysfs_fan_attributes
 */
static const u8 fan_reg[] = {
	0x2B, /* fan PWM(for all fan) */
	0x2D, /* fan 1 speed(rpm) */
	0x2C, /* fan 2 speed(rpm) */
	0x11, /* fan1-2 operating status */
};

static struct as4610_fan_data *fan_data = NULL;

/* Each client has this additional data */
struct as4610_fan_data {
	struct platform_device *pdev;
	struct device	*hwmon_dev;
	struct mutex	 update_lock;
	char			 valid;			  /* != 0 if registers are valid */
	unsigned long	 last_updated;	  /* In jiffies */
	u8				 reg_val[ARRAY_SIZE(fan_reg)]; /* Register value */
};

enum fan_id {
	FAN1_ID,
	FAN2_ID
};

enum sysfs_fan_attributes {
	FAN_DUTY_CYCLE_PERCENTAGE, /* Only one CPLD register to control duty cycle for all fans */
	FAN1_SPEED_RPM,
	FAN2_SPEED_RPM,
	FAN_FAULT,
	FAN1_FAULT,
	FAN2_FAULT
};

/* Define attributes
 */
#define DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_fault, S_IRUGO, fan_show_value, NULL, FAN##index##_FAULT)
#define DECLARE_FAN_FAULT_ATTR(index)	   &sensor_dev_attr_fan##index##_fault.dev_attr.attr

#define DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_duty_cycle_percentage, S_IWUSR | S_IRUGO, fan_show_value, set_duty_cycle, FAN##index##_DUTY_CYCLE_PERCENTAGE)
#define DECLARE_FAN_DUTY_CYCLE_ATTR(index) &sensor_dev_attr_fan##index##_duty_cycle_percentage.dev_attr.attr

#define DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_speed_rpm, S_IRUGO, fan_show_value, NULL, FAN##index##_SPEED_RPM)
#define DECLARE_FAN_SPEED_RPM_ATTR(index)  &sensor_dev_attr_fan##index##_speed_rpm.dev_attr.attr

/* fan fault attributes in this platform */
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(2);
/* fan speed(rpm) attributes in this platform */
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(1);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(2);
/* 1 fan duty cycle attribute in this platform */
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR();

static struct attribute *as4610_fan_attributes[] = {
	/* fan related attributes */
	DECLARE_FAN_FAULT_ATTR(1),
	DECLARE_FAN_FAULT_ATTR(2),
	DECLARE_FAN_SPEED_RPM_ATTR(1),
	DECLARE_FAN_SPEED_RPM_ATTR(2),
	DECLARE_FAN_DUTY_CYCLE_ATTR(),
	NULL
};

#define FAN_DUTY_CYCLE_REG_MASK			0xF
#define FAN_MAX_DUTY_CYCLE				100
#define FAN_REG_VAL_TO_SPEED_RPM_STEP	100

static int as4610_fan_read_value(u8 reg)
{
	return accton_i2c_cpld_read(AS4610_CPLD_SLAVE_ADDR, reg);
}

static int as4610_fan_write_value(u8 reg, u8 value)
{
	return accton_i2c_cpld_write(AS4610_CPLD_SLAVE_ADDR, reg, value);
}

/* fan utility functions
 */
static u32 reg_val_to_duty_cycle(u8 reg_val)
{
	reg_val &= FAN_DUTY_CYCLE_REG_MASK;
	return (u32)((reg_val * 125 + 5)/10);
}

static u8 duty_cycle_to_reg_val(u8 duty_cycle)
{
	return ((u32)duty_cycle * 10 / 125);
}

static u32 reg_val_to_speed_rpm(u8 reg_val)
{
	/* Count Frequency is 1.515KHz= 0.66ms
	 * Count Period = 400 cycle = 400*0.66ms = 264ms
	 * R.P.M value = read value x3.79*60/2
	 * 3.79 = 1000ms/264ms
	 * 60 = 1min =60s
	 * 2 = 1 rotation of fan has two pulses.
	 */
	return (u32)reg_val * 379 * 60 / 2 / 100;
}

static u8 is_fan_fault(struct as4610_fan_data *data, enum fan_id id)
{
	u8 mask = (id == FAN1_ID) ? 0x20 : 0x10;

	return !(data->reg_val[FAN_FAULT] & mask);
}

static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	int error, value;

	error = kstrtoint(buf, 10, &value);
	if (error)
		return error;

	if (value < 0 || value > FAN_MAX_DUTY_CYCLE)
		return -EINVAL;

	as4610_fan_write_value(fan_reg[FAN_DUTY_CYCLE_PERCENTAGE], duty_cycle_to_reg_val(value));
	return count;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as4610_fan_data *data = as4610_fan_update_device(dev);
	ssize_t ret = 0;

	if (data->valid) {
		switch (attr->index) {
			case FAN_DUTY_CYCLE_PERCENTAGE:
			{
				u32 duty_cycle = reg_val_to_duty_cycle(data->reg_val[attr->index]);
				ret = sprintf(buf, "%u\n", duty_cycle);
				break;
			}
			case FAN1_SPEED_RPM:
			case FAN2_SPEED_RPM:
				ret = sprintf(buf, "%u\n", reg_val_to_speed_rpm(data->reg_val[attr->index]));
				break;
			case FAN1_FAULT:
			case FAN2_FAULT:
				ret = sprintf(buf, "%d\n", is_fan_fault(data, attr->index - FAN1_FAULT));
				break;
			default:
				break;
		}
	}

	return ret;
}

static const struct attribute_group as4610_fan_group = {
	.attrs = as4610_fan_attributes,
};

static struct as4610_fan_data *as4610_fan_update_device(struct device *dev)
{
	mutex_lock(&fan_data->update_lock);

	if (time_after(jiffies, fan_data->last_updated + HZ + HZ / 2) ||
		!fan_data->valid) {
		int i;

		dev_dbg(fan_data->hwmon_dev, "Starting as4610_fan update\n");
		fan_data->valid = 0;

		/* Update fan data
		 */
		for (i = 0; i < ARRAY_SIZE(fan_data->reg_val); i++) {
			int status = as4610_fan_read_value(fan_reg[i]);

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

static int as4610_fan_probe(struct platform_device *pdev)
{
	int status = -1;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &as4610_fan_group);
	if (status) {
		goto exit;

	}

	fan_data->hwmon_dev = hwmon_device_register(&pdev->dev);
	if (IS_ERR(fan_data->hwmon_dev)) {
		status = PTR_ERR(fan_data->hwmon_dev);
		goto exit_remove;
	}

	dev_info(&pdev->dev, "accton_as4610_fan\n");

	return 0;

exit_remove:
	sysfs_remove_group(&pdev->dev.kobj, &as4610_fan_group);
exit:
	return status;
}

static int as4610_fan_remove(struct platform_device *pdev)
{
	hwmon_device_unregister(fan_data->hwmon_dev);
	sysfs_remove_group(&pdev->dev.kobj, &as4610_fan_group);

	return 0;
}

static const struct i2c_device_id as4610_fan_id[] = {
	{ "as4610_fan", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, as4610_fan_id);

static struct platform_driver as4610_fan_driver = {
	.probe		= as4610_fan_probe,
	.remove		= as4610_fan_remove,
	.driver		= {
		.name	= DRVNAME,
		.owner	= THIS_MODULE,
	},
};

static int __init as4610_fan_init(void)
{
	int ret;

	if (as4610_number_of_system_fan() == 0) {
		return -ENODEV;
	}

	ret = platform_driver_register(&as4610_fan_driver);
	if (ret < 0) {
		goto exit;
	}

	fan_data = kzalloc(sizeof(struct as4610_fan_data), GFP_KERNEL);
	if (!fan_data) {
		ret = -ENOMEM;
		platform_driver_unregister(&as4610_fan_driver);
		goto exit;
	}

	mutex_init(&fan_data->update_lock);
	fan_data->valid = 0;

	fan_data->pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
	if (IS_ERR(fan_data->pdev)) {
		ret = PTR_ERR(fan_data->pdev);
		platform_driver_unregister(&as4610_fan_driver);
		kfree(fan_data);
		goto exit;
	}

exit:
	return ret;
}

static void __exit as4610_fan_exit(void)
{
	if (!fan_data) {
		return;
	}

	platform_device_unregister(fan_data->pdev);
	platform_driver_unregister(&as4610_fan_driver);
	kfree(fan_data);
}

late_initcall(as4610_fan_init);
module_exit(as4610_fan_exit);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as4610_fan driver");
MODULE_LICENSE("GPL");
