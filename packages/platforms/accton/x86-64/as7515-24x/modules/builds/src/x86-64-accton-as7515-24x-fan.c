// SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0
/*
 * A hwmon driver for the Accton as7515 24x fan
 *
 * Copyright (C) 2024 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com>
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
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#define DRVNAME "as7515_24x_fan"

#define BOARD_INFO_REG_OFFSET 0x00
#define FAN_STATUS_I2C_ADDR 0x40
#define I2C_RW_RETRY_COUNT 10
#define I2C_RW_RETRY_INTERVAL 60

#define FAN_NUM 5
#define MAX_FAN_INPUT 23000
#define FAN_DUTY_CYCLE_REG_MASK 0x3F
#define FAN_MAX_PWM 100

static struct as7515_24x_fan_data *as7515_fan_update_device(struct device *dev);
static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
		char *buf);
static ssize_t fan_set_enable(struct device *dev, struct device_attribute *da,
		const char *buf, size_t count);
static ssize_t fan_set_pwm(struct device *dev, struct device_attribute *da,
		const char *buf, size_t count);
static ssize_t show_wdt(struct device *dev, struct device_attribute *da,
		char *buf);
static ssize_t set_wdt(struct device *dev, struct device_attribute *da,
		const char *buf, size_t count);
static int _reset_wdt(struct as7515_24x_fan_data *data);
static ssize_t reset_wdt(struct device *dev, struct device_attribute *da,
		const char *buf, size_t count);

/* fan related data, the index should match sysfs_fan_attributes
 */
static const u8 fan_reg[] = {
	0x01,	   /* fan board version */
	0x02,	   /* fan board sub version */
	0x10,	   /* fan enable */
	0x21,	   /* fan1 PWM */
	0x31,	   /* fan2 PWM */
	0x41,	   /* fan3 PWM */
	0x51,	   /* fan4 PWM */
	0x61,	   /* fan5 PWM */
	0x20,	   /* fan 1 tach speed */
	0x30,	   /* fan 2 tach speed */
	0x40,	   /* fan 3 tach speed */
	0x50,	   /* fan 4 tach speed */
	0x60,	   /* fan 5 tach speed */
	0x22,	   /* fan 1 present/fault */
	0x32,	   /* fan 2 present/fault */
	0x42,	   /* fan 3 present/fault */
	0x52,	   /* fan 4 present/fault */
	0x62,	   /* fan 5 present/fault */
	0x0D,	   /* fan tech speed setting */
};

/* fan data */
struct as7515_24x_fan_data {
	struct i2c_client *client;
	struct device *hwmon_dev;
	struct mutex update_lock;
	char valid; /* != 0 if registers are valid */
	unsigned long last_updated; /* In jiffies */
	u8 reg_val[ARRAY_SIZE(fan_reg)]; /* Register value */
};

enum fan_id {
	FAN1_ID,
	FAN2_ID,
	FAN3_ID,
	FAN4_ID,
	FAN5_ID
};

enum sysfs_fan_attributes {
	FAN_MAJOR_VERSION_REG,
	FAN_MINOR_VERSION_REG,
	FAN_ENABLE_REG,
	FAN1_PWM,
	FAN2_PWM,
	FAN3_PWM,
	FAN4_PWM,
	FAN5_PWM,
	FAN1_INPUT,
	FAN2_INPUT,
	FAN3_INPUT,
	FAN4_INPUT,
	FAN5_INPUT,
	FAN1_PRESENT,
	FAN2_PRESENT,
	FAN3_PRESENT,
	FAN4_PRESENT,
	FAN5_PRESENT,
	FAN_TECH_SETTING,
	FAN1_FAULT,
	FAN2_FAULT,
	FAN3_FAULT,
	FAN4_FAULT,
	FAN5_FAULT,
	FAN1_ENABLE,
	FAN2_ENABLE,
	FAN3_ENABLE,
	FAN4_ENABLE,
	FAN5_ENABLE,
	FAN1_TARGET,
	FAN2_TARGET,
	FAN3_TARGET,
	FAN4_TARGET,
	FAN5_TARGET,
	FAN_MAX_RPM,
	WDT_CLOCK,
	WDT_COUNTER,
	WDT_ENABLE,
	WDT_RESET,
	FAN_FW_VERSION
};

static const int fan_target_speed[64] = {
	   0,      0,     0,     0,     0,  2324,  3218,  5007, /* 0x00 ~ 0x07 */
	 5364,  6080,  6795,  7331,  8226,  8762,  9298,  9477, /* 0x08 ~ 0x0F */
	10014, 10371, 11087, 11444, 11623, 12160, 12339, 12875, /* 0x10 ~ 0x17 */
	13054, 13412, 13769, 14127, 14484, 14842, 15200, 15379, /* 0x18 ~ 0x1F */
	15915, 16273, 16630, 16988, 17346, 17525, 17882, 18061, /* 0x20 ~ 0x27 */
	18240, 18776, 18955, 19313, 19670, 19670, 19849, 20028, /* 0x28 ~ 0x2F */
	20386, 20565, 20743, 20922, 21101, 21280, 21638, 21816, /* 0x30 ~ 0x37 */
	21995, 22174, 22353, 22532, 22711, 22889, 22889, 23000, /* 0x38 ~ 0x3F */
};

/* sysfs attributes for hwmon
 */
static SENSOR_DEVICE_ATTR(version, S_IRUGO, fan_show_value, NULL,
			FAN_FW_VERSION);
static SENSOR_DEVICE_ATTR(fan_input_max, S_IRUGO, fan_show_value, NULL,
			FAN_MAX_RPM);

/* Fan watchdog */
static SENSOR_DEVICE_ATTR(wdt_clock, S_IRUGO | S_IWUSR, show_wdt, set_wdt,
			WDT_CLOCK);
static SENSOR_DEVICE_ATTR(wdt_counter, S_IRUGO | S_IWUSR, show_wdt, set_wdt,
			WDT_COUNTER);
static SENSOR_DEVICE_ATTR(wdt_enable, S_IRUGO | S_IWUSR, show_wdt, set_wdt,
			WDT_ENABLE);
static SENSOR_DEVICE_ATTR(wdt_reset, S_IWUSR, NULL, reset_wdt, WDT_RESET);

static struct attribute *fan_attributes_common[] = {
	&sensor_dev_attr_version.dev_attr.attr,
	&sensor_dev_attr_fan_input_max.dev_attr.attr,
	&sensor_dev_attr_wdt_clock.dev_attr.attr,
	&sensor_dev_attr_wdt_counter.dev_attr.attr,
	&sensor_dev_attr_wdt_enable.dev_attr.attr,
	&sensor_dev_attr_wdt_reset.dev_attr.attr,
	NULL
};

#define FAN_ATTRS_COMMON() \
static const struct attribute_group fan_attrgroup_common = { \
	.attrs = fan_attributes_common, \
};

#define FAN_ATTRS(fid) \
	static SENSOR_DEVICE_ATTR(fan##fid##_present, S_IRUGO, fan_show_value, \
		NULL, FAN##fid##_PRESENT); \
	static SENSOR_DEVICE_ATTR(fan##fid##_pwm, S_IWUSR | S_IRUGO, \
		fan_show_value, fan_set_pwm, FAN##fid##_PWM); \
	static SENSOR_DEVICE_ATTR(fan##fid##_input, S_IRUGO, fan_show_value, \
		NULL, FAN##fid##_INPUT); \
	static SENSOR_DEVICE_ATTR(fan##fid##_target, S_IRUGO, fan_show_value, \
		NULL, FAN##fid##_TARGET); \
	static SENSOR_DEVICE_ATTR(fan##fid##_fault, S_IRUGO, fan_show_value, \
		NULL, FAN##fid##_FAULT); \
	static SENSOR_DEVICE_ATTR(fan##fid##_enable, S_IWUSR | S_IRUGO, \
		fan_show_value, fan_set_enable, FAN##fid##_ENABLE); \
	static struct attribute *fan_attributes##fid[] = { \
		&sensor_dev_attr_fan##fid##_present.dev_attr.attr, \
		&sensor_dev_attr_fan##fid##_pwm.dev_attr.attr, \
		&sensor_dev_attr_fan##fid##_input.dev_attr.attr, \
		&sensor_dev_attr_fan##fid##_target.dev_attr.attr, \
		&sensor_dev_attr_fan##fid##_fault.dev_attr.attr, \
		&sensor_dev_attr_fan##fid##_enable.dev_attr.attr, \
		NULL \
}

#define FAN_ATTR_GROUP(fid) \
	FAN_ATTRS(fid); \
static const struct attribute_group fan_attrgroup_##fid = { \
	.attrs = fan_attributes##fid \
}

FAN_ATTRS_COMMON();
FAN_ATTR_GROUP(1);
FAN_ATTR_GROUP(2);
FAN_ATTR_GROUP(3);
FAN_ATTR_GROUP(4);
FAN_ATTR_GROUP(5);

static const struct attribute_group *fan_groups[] = {
	&fan_attrgroup_common,
	&fan_attrgroup_1,
	&fan_attrgroup_2,
	&fan_attrgroup_3,
	&fan_attrgroup_4,
	&fan_attrgroup_5,
	NULL
};

//ATTRIBUTE_GROUPS(fan_group);

static int as7515_fan_read_value(struct i2c_client *client, u8 reg)
{
	int status = 0, retry = I2C_RW_RETRY_COUNT;

	while (retry) {
		status = i2c_smbus_read_byte_data(client, reg);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

	return status;
}

static int as7515_fan_write_value(struct i2c_client *client, u8 reg, u8 value)
{
	int status = 0, retry = I2C_RW_RETRY_COUNT;

	while (retry) {
		status = i2c_smbus_write_byte_data(client, reg, value);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

	return status;
}

/* fan utility functions
 */
static u8 reg_val_to_is_present(u8 reg_val)
{
	return !(reg_val & 0x1);
}

static u32 reg_val_to_pwm(u8 reg_val)
{
	reg_val &= FAN_DUTY_CYCLE_REG_MASK;
	return (((u32)reg_val) * 100 / 63);
}

static u8 pwm_to_reg_val(u8 pwm)
{
	if (pwm == 0)
		return 0;
	else if (pwm > FAN_MAX_PWM)
		pwm = FAN_MAX_PWM;

	return ((u32)pwm) * 63 / 100;
}

static u32 reg_val_to_speed_rpm(u8 reg_val, u8 tech_reg_val)
{
	u32 timer[] = { 1048, 2097, 4194, 8389 };
	u8 counter = (tech_reg_val & 0x3F);
	u8 clock   = (tech_reg_val >> 6) & 0x3;

	return (reg_val * 3000000) / (timer[clock] * counter);
}

static u8 reg_val_to_fault(u8 reg_val)
{
	return !!(reg_val & 0x2);
}

static u8 reg_val_to_enable(u8 reg_val, enum fan_id id)
{
	return !!(reg_val & BIT(id));
}

static ssize_t fan_set_pwm(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	int error, value;
	struct as7515_24x_fan_data *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

	error = kstrtoint(buf, 10, &value);
	if (error)
		return error;

	if (value < 0 || value > FAN_MAX_PWM)
		return -EINVAL;

	error = as7515_fan_write_value(data->client,
						fan_reg[attr->index],
						pwm_to_reg_val(value));
	if (unlikely(error < 0)) {
		dev_dbg(dev, "Unable to set fan pwm, error = (%d)\n", error);
		return error;
	}

	data->valid = 0;
	return count;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as7515_24x_fan_data *data = as7515_fan_update_device(dev);
	ssize_t ret = 0;

	mutex_lock(&data->update_lock);

	if (data->valid) {
		switch (attr->index) {
		case FAN_FW_VERSION:
			ret = sprintf(buf, "%d.%d\n",
					data->reg_val[FAN_MAJOR_VERSION_REG],
					data->reg_val[FAN_MINOR_VERSION_REG]);
		break;
		case FAN1_PRESENT ... FAN5_PRESENT:
			ret = sprintf(buf, "%d\n",
				reg_val_to_is_present(data->reg_val[attr->index]));
		break;
		case FAN1_PWM ... FAN5_PWM:
			ret = sprintf(buf, "%u\n",
				reg_val_to_pwm(data->reg_val[attr->index]));
			break;
		case FAN1_INPUT ... FAN5_INPUT:
			ret = sprintf(buf, "%u\n",
				reg_val_to_speed_rpm(data->reg_val[attr->index],
					data->reg_val[FAN_TECH_SETTING]));
			break;
		case FAN1_TARGET ... FAN5_TARGET:
			ret = sprintf(buf, "%u\n",
				fan_target_speed[data->reg_val[FAN1_PWM + attr->index - FAN1_TARGET]]);
			break;
		case FAN1_FAULT ... FAN5_FAULT:
			ret = sprintf(buf, "%d\n",
				reg_val_to_fault(data->reg_val[FAN1_PRESENT + attr->index - FAN1_FAULT]));
			break;
		case FAN1_ENABLE ... FAN5_ENABLE:
			ret = sprintf(buf, "%d\n",
				reg_val_to_enable(data->reg_val[FAN_ENABLE_REG],
					attr->index - FAN1_ENABLE));
			break;
		case FAN_MAX_RPM:
			ret = sprintf(buf, "%d\n", MAX_FAN_INPUT);
			break;
		default:
			break;
		}
	}

	mutex_unlock(&data->update_lock);
	return ret;
}

static ssize_t fan_set_enable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	int status, value;
	struct as7515_24x_fan_data *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

	status = kstrtoint(buf, 10, &value);
	if (status)
		return status;

	if (value < 0 || value > 1)
		return -EINVAL;

	status = as7515_fan_read_value(data->client, fan_reg[FAN_ENABLE_REG]);
	if (unlikely(status < 0)) {
		dev_dbg(dev, "Unable to read the fan reg(0x%x), error = (%d)\n",
			fan_reg[FAN_ENABLE_REG], status);
		return status;
	}

	if (value)
		status |= BIT(attr->index - FAN1_ENABLE);
	else
		status &= ~BIT(attr->index - FAN1_ENABLE);

	as7515_fan_write_value(data->client,
						fan_reg[FAN_ENABLE_REG],
						status);
	data->valid = 0;
	return count;
}

static ssize_t show_wdt(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct as7515_24x_fan_data *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	int status = 0, invert = 0;
	u8 reg = 0, mask = 0;

	switch (attr->index) {
	case WDT_ENABLE:
		invert = 1;
		reg  = 0x0A;
		mask = 0x01;
		break;
	case WDT_CLOCK:
		reg  = 0x0B;
		mask = 0xC0;
		break;
	case WDT_COUNTER:
		reg  = 0x0B;
		mask = 0x3F;
		break;
	default:
		return 0;
	}

	mutex_lock(&data->update_lock);
	status = as7515_fan_read_value(data->client, reg);
	if (unlikely(status < 0))
		goto exit;
	mutex_unlock(&data->update_lock);

	while (!(mask & 0x1)) {
		status >>= 1;
		mask >>= 1;
	}

	return sprintf(buf, "%d\n", invert ? !(status & mask) : (status & mask));
exit:
	mutex_unlock(&data->update_lock);
	return status;
}

#define VALIDATE_WDT_VAL_RETURN(value, mask) \
	do { \
		if (value & ~mask) \
			return -EINVAL; \
	} while (0)

static ssize_t set_wdt(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	struct as7515_24x_fan_data *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	long value;
	int status;
	u8 reg = 0, mask = 0;

	status = kstrtol(buf, 10, &value);
	if (status)
		return status;

	switch (attr->index) {
	case WDT_ENABLE:
		value = !value;
		reg  = 0x0A;
		mask = 0x01;
		value &= mask;
		VALIDATE_WDT_VAL_RETURN(value, mask);
		break;
	case WDT_CLOCK:
		reg  = 0x0B;
		mask = 0xC0;
		value <<= 6;
		VALIDATE_WDT_VAL_RETURN(value, mask);
		break;
	case WDT_COUNTER:
		reg  = 0x0B;
		mask = 0x3F;
		value &= mask;
		VALIDATE_WDT_VAL_RETURN(value, mask);
		break;
	default:
		return 0;
	}

	/* Read current status */
	mutex_lock(&data->update_lock);

	status = as7515_fan_read_value(data->client, reg);
	if (unlikely(status < 0))
		goto exit;

	/* Update wdt status */
	status = (status & ~mask) | (value & mask);
	status = as7515_fan_write_value(data->client, reg, status);
	if (unlikely(status < 0))
		goto exit;

	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static int _reset_wdt(struct as7515_24x_fan_data *data)
{
	int status;

	/* Set value as 1->0 to reset wdt */
	status = as7515_fan_write_value(data->client, 0x0A, 1);
	if (unlikely(status < 0))
		return status;

	msleep(50);
	status = as7515_fan_write_value(data->client, 0x0A, 0);
	if (unlikely(status < 0))
		return status;

	return status;
}

static ssize_t reset_wdt(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	long value;
	int status;
	struct as7515_24x_fan_data *data = dev_get_drvdata(dev);

	status = kstrtol(buf, 10, &value);
	if (status)
		return status;

	if (!value)
		return count;

	/* Read current status */
	mutex_lock(&data->update_lock);

	status = _reset_wdt(data);
	if (unlikely(status < 0)) {
		dev_dbg(dev, "Unable to reset the watchdog timer\n");
		return status;
	}

	mutex_unlock(&data->update_lock);
	return count;
}

static struct as7515_24x_fan_data *as7515_fan_update_device(struct device *dev)
{
	struct as7515_24x_fan_data *data = dev_get_drvdata(dev);
	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ + HZ / 2) ||
		!data->valid) {
		int i;

		dev_dbg(dev, "Starting as7515_fan update\n");
		data->valid = 0;

		/* Update fan data
		 */
		for (i = 0; i < ARRAY_SIZE(data->reg_val); i++) {
			int status = as7515_fan_read_value(data->client, fan_reg[i]);

			if (status < 0) {
				data->valid = 0;
				mutex_unlock(&data->update_lock);
				dev_dbg(dev, "reg %d, err %d\n",
						fan_reg[i], status);
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

static int as7515_24x_fan_probe(struct i2c_client *client,
				   const struct i2c_device_id *dev_id)
{
	struct as7515_24x_fan_data *data;
	int status;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		status = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct as7515_24x_fan_data), GFP_KERNEL);
	if (!data) {
		status = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	data->valid = 0;
	data->client = client;
	mutex_init(&data->update_lock);
	dev_info(&client->dev, "chip found\n");

	data->hwmon_dev =
		hwmon_device_register_with_groups(&client->dev, client->name, data,
						fan_groups);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_free;
	}

	dev_info(&client->dev, "%s: fan '%s'\n",
		 dev_name(data->hwmon_dev), client->name);

	return 0;

 exit_free:
	kfree(data);
 exit:
	return status;
}

static int as7515_24x_fan_remove(struct i2c_client *client)
{
	struct as7515_24x_fan_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	kfree(data);
	return 0;
}

/* Addresses to scan */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

static const struct i2c_device_id as7515_24x_fan_id[] = {
	{"as7515_24x_fan", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, as7515_24x_fan_id);

static struct i2c_driver as7515_24x_fan_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		   .name = DRVNAME,
		   },
	.probe = as7515_24x_fan_probe,
	.remove = as7515_24x_fan_remove,
	.id_table = as7515_24x_fan_id,
	.address_list = normal_i2c,
};

module_i2c_driver(as7515_24x_fan_driver);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as7515_fan driver");
MODULE_LICENSE("GPL");
