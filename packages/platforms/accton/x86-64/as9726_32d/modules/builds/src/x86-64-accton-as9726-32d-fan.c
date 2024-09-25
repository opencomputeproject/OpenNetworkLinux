/*
 * A hwmon driver for the Accton as9726 32d fan
 *
 * Copyright (C) 2014 Accton Technology Corporation.
 * Alex Lai <alex_lai@edge-core.com>
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

#define DRVNAME "as9726_32d_fan"

static struct as9726_32d_fan_data *as9726_32d_fan_update_device(
						struct device *dev);
static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
				char *buf);
static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
			      const char *buf, size_t count);

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
struct as9726_32d_fan_data {
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
#define DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_fault, S_IRUGO, \
				fan_show_value, NULL, FAN##index##_FAULT)
#define DECLARE_FAN_FAULT_ATTR(index) \
	&sensor_dev_attr_fan##index##_fault.dev_attr.attr

#define DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_direction, S_IRUGO, \
				fan_show_value, NULL, FAN##index##_DIRECTION)
#define DECLARE_FAN_DIRECTION_ATTR(index)  \
	&sensor_dev_attr_fan##index##_direction.dev_attr.attr

#define DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_duty_cycle_percentage, \
				S_IWUSR | S_IRUGO, fan_show_value, \
				set_duty_cycle, \
				FAN##index##_DUTY_CYCLE_PERCENTAGE)
#define DECLARE_FAN_DUTY_CYCLE_ATTR(index) \
	&sensor_dev_attr_fan##index##_duty_cycle_percentage.dev_attr.attr

#define DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_present, S_IRUGO, \
				fan_show_value, NULL, FAN##index##_PRESENT)
#define DECLARE_FAN_PRESENT_ATTR(index) \
	&sensor_dev_attr_fan##index##_present.dev_attr.attr

#define DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_front_speed_rpm, S_IRUGO, \
				fan_show_value, NULL, \
				FAN##index##_FRONT_SPEED_RPM);\
	static SENSOR_DEVICE_ATTR(fan##index##_rear_speed_rpm, S_IRUGO, \
				fan_show_value, NULL, \
				FAN##index##_REAR_SPEED_RPM)
#define DECLARE_FAN_SPEED_RPM_ATTR(index)  \
	&sensor_dev_attr_fan##index##_front_speed_rpm.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_rear_speed_rpm.dev_attr.attr

/* 6 fan fault attributes in this platform */
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(2);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(3);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(4);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(5);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(6);
/* 6 fan speed(rpm) attributes in this platform */
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(1);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(2);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(3);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(4);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(5);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(6);
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
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR();

static struct attribute *as9726_32d_fan_attributes[] = {
	/* fan related attributes */
	DECLARE_FAN_FAULT_ATTR(1),
	DECLARE_FAN_FAULT_ATTR(2),
	DECLARE_FAN_FAULT_ATTR(3),
	DECLARE_FAN_FAULT_ATTR(4),
	DECLARE_FAN_FAULT_ATTR(5),
	DECLARE_FAN_FAULT_ATTR(6),
	DECLARE_FAN_SPEED_RPM_ATTR(1),
	DECLARE_FAN_SPEED_RPM_ATTR(2),
	DECLARE_FAN_SPEED_RPM_ATTR(3),
	DECLARE_FAN_SPEED_RPM_ATTR(4),
	DECLARE_FAN_SPEED_RPM_ATTR(5),
	DECLARE_FAN_SPEED_RPM_ATTR(6),
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
	DECLARE_FAN_DUTY_CYCLE_ATTR(),
	NULL
};

#define FAN_DUTY_CYCLE_REG_MASK         0xF
#define FAN_MAX_DUTY_CYCLE              100
#define FAN_REG_VAL_TO_SPEED_RPM_STEP   200

static int as9726_32d_fan_read_value(struct i2c_client *client, u8 reg)
{
	return i2c_smbus_read_byte_data(client, reg);
}

static int as9726_32d_fan_write_value(struct i2c_client *client, u8 reg,
					u8 value)
{
	return i2c_smbus_write_byte_data(client, reg, value);
}

/* fan utility functions
 */
static u32 reg_val_to_duty_cycle(u8 reg_val)
{
	reg_val &= FAN_DUTY_CYCLE_REG_MASK;
	return ((u32)(reg_val+1) * 625 + 75) / 100;
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

static u8 is_fan_fault(struct as9726_32d_fan_data *data, enum fan_id id)
{
	u8 ret = 1;
	int front_fan_index = FAN1_FRONT_SPEED_RPM + id;
	int rear_fan_index  = FAN1_REAR_SPEED_RPM  + id;

	/* Check if the speed of front or rear fan is ZERO,
	 */
	if (reg_val_to_speed_rpm(data->reg_val[front_fan_index]) &&
		reg_val_to_speed_rpm(data->reg_val[rear_fan_index]))
		ret = 0;

	return ret;
}

static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
			      const char *buf, size_t count)
{
	int error, value;
	struct i2c_client *client = to_i2c_client(dev);
	struct as9726_32d_fan_data *data = i2c_get_clientdata(client);

	error = kstrtoint(buf, 10, &value);
	
	if (error)
		return error;

	if (value < 0 || value > FAN_MAX_DUTY_CYCLE)
		return -EINVAL;

	mutex_lock(&data->update_lock);

	/* Disable the watchdog timer
	 */
	error = as9726_32d_fan_write_value(client, 0x33, 0);
	if (error != 0) {
		dev_dbg(&client->dev, "Unable to disable the watchdog timer\n"
			);
		mutex_unlock(&data->update_lock);
		return error;
	}

	as9726_32d_fan_write_value(client, 
				fan_reg[FAN_DUTY_CYCLE_PERCENTAGE],
				duty_cycle_to_reg_val(value));
	data->valid = 0;

	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
			      char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as9726_32d_fan_data *data = i2c_get_clientdata(client);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	ssize_t ret = 0;

	mutex_lock(&data->update_lock);
	data = as9726_32d_fan_update_device(dev);

	if (data->valid) {
		switch (attr->index) {
		case FAN_DUTY_CYCLE_PERCENTAGE: {
			u32 duty_cycle = 
				reg_val_to_duty_cycle(
				data->reg_val[FAN_DUTY_CYCLE_PERCENTAGE]);
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
			ret = sprintf(buf, "%u\n",
				reg_val_to_speed_rpm(
					data->reg_val[attr->index]));
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
			ret = sprintf(buf, "%d\n", 
				is_fan_fault(data, attr->index - FAN1_FAULT));
			break;
		case FAN1_DIRECTION:
		case FAN2_DIRECTION:
		case FAN3_DIRECTION:
		case FAN4_DIRECTION:
		case FAN5_DIRECTION:
		case FAN6_DIRECTION:
			ret = sprintf(buf, "%d\n",
				reg_val_to_direction(
					data->reg_val[FAN_DIRECTION_REG],
					attr->index - FAN1_DIRECTION));
			break;
		default:
			break;
		}
	}

	mutex_unlock(&data->update_lock);

	return ret;
}

static const struct attribute_group as9726_32d_fan_group = {
	.attrs = as9726_32d_fan_attributes,
};

static struct as9726_32d_fan_data *as9726_32d_fan_update_device(struct 
								device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as9726_32d_fan_data *data = i2c_get_clientdata(client);

	if (time_after(jiffies, data->last_updated + HZ + HZ / 2) ||
		!data->valid) {
		int i;

		dev_dbg(&client->dev, "Starting as9726_32d_fan update\n");
		data->valid = 0;

		/* Update fan data
		 */
		for (i = 0; i < ARRAY_SIZE(data->reg_val); i++) {
			int status = as9726_32d_fan_read_value(client, 
								fan_reg[i]);
			if (status < 0) {
				data->valid = 0;
				dev_dbg(&client->dev, "reg %d, err %d\n", 
					fan_reg[i], status);
				return data;
			} else {
				data->reg_val[i] = status;
			}
		}

		data->last_updated = jiffies;
		data->valid = 1;
	}

	return data;
}

static int as9726_32d_fan_probe(struct i2c_client *client,
				const struct i2c_device_id *dev_id)
{
	struct as9726_32d_fan_data *data;
	int status;

	if (!i2c_check_functionality(client->adapter, 
		I2C_FUNC_SMBUS_BYTE_DATA)) {
		status = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct as9726_32d_fan_data), GFP_KERNEL);
	if (!data) {
		status = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	data->valid = 0;
	mutex_init(&data->update_lock);

	dev_info(&client->dev, "chip found\n");

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &as9726_32d_fan_group);
	if (status)
		goto exit_free;

	data->hwmon_dev = hwmon_device_register_with_info(&client->dev,
							"as9726_32d_fan",
							NULL, NULL, NULL);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	dev_info(&client->dev, "%s: fan '%s'\n",
		 dev_name(data->hwmon_dev), client->name);

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &as9726_32d_fan_group);
exit_free:
	kfree(data);
exit:

	return status;
}

static int as9726_32d_fan_remove(struct i2c_client *client)
{
	struct as9726_32d_fan_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &as9726_32d_fan_group);

	return 0;
}

/* Addresses to scan */
static const unsigned short normal_i2c[] = { 0x66, I2C_CLIENT_END };

static const struct i2c_device_id as9726_32d_fan_id[] = {
	{ "as9726_32d_fan", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, as9726_32d_fan_id);

static struct i2c_driver as9726_32d_fan_driver = {
	.class    = I2C_CLASS_HWMON,
	.driver = {
		.name     = DRVNAME,
	},
	.probe        = as9726_32d_fan_probe,
	.remove       = as9726_32d_fan_remove,
	.id_table     = as9726_32d_fan_id,
	.address_list = normal_i2c,
};

module_i2c_driver(as9726_32d_fan_driver);

MODULE_AUTHOR("Alex Lai <alex_lai@edge-core.com>");
MODULE_DESCRIPTION("as9726_32d_fan driver");
MODULE_LICENSE("GPL");
