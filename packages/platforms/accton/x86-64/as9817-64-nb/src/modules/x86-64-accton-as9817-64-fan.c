// SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0
/*
 * A hwmon driver for the Accton as9817_64 fan
 *
 * Copyright (C) 2024 Accton Technology Corporation.
 * Roger Ho <roger530_ho@accton.com>
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
#include <linux/delay.h>
#include <asm/uaccess.h>

#define DRVNAME "as9817_64_fan"

#define I2C_RW_RETRY_COUNT			 (10)
#define I2C_RW_RETRY_INTERVAL		  (60)	/* ms */

static struct as9817_64_fan_data *as9817_64_fan_update_device(struct device
							      *dev);
static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
			      char *buf);
static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
			      const char *buf, size_t count);
static ssize_t set_fan_led(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count);
static ssize_t reg_read(struct device *dev, struct device_attribute *da,
			char *buf);
static ssize_t reg_write(struct device *dev, struct device_attribute *da,
			 const char *buf, size_t count);

/* fan related data, the index should match sysfs_fan_attributes
 */
static const u8 fan_reg[] = {
	0x00,			/* fan pcb information */
	0x01,			/* fan cpld major version */
	0x02,			/* fan cpld minor version */
	0x06,			/* fan 0-3 led */
	0x07,			/* fan 0-3 present status */
	0x16,			/* front fan 0 pwm */
	0x17,			/* rear fan 0 pwm */
	0x14,			/* front fan 1 pwm */
	0x15,			/* rear fan 1 pwm */
	0x12,			/* front fan 2 pwm */
	0x13,			/* rear fan 2 pwm */
	0x10,			/* front fan 3 pwm */
	0x11,			/* rear fan 3 pwm */
	0x26,			/* front fan 0 speed(rpm) */
	0x27,			/* rear fan 0 speed(rpm) */
	0x24,			/* front fan 1 speed(rpm) */
	0x25,			/* rear fan 1 speed(rpm) */
	0x22,			/* front fan 2 speed(rpm) */
	0x23,			/* rear fan 2 speed(rpm) */
	0x20,			/* front fan 3 speed(rpm) */
	0x21,			/* rear fan 3 speed(rpm) */
};

/* Each client has this additional data */
struct as9817_64_fan_data {
	struct i2c_client *client;
	struct device *hwmon_dev;
	struct mutex update_lock;
	char valid;		/* != 0 if registers are valid */
	unsigned long last_updated;	/* In jiffies */
	u8 reg_val[ARRAY_SIZE(fan_reg)];	/* Register value */
	u8 reg_addr;
};

enum fan_id {
	FAN1_ID,
	FAN2_ID,
	FAN3_ID,
	FAN4_ID,
};

enum sysfs_fan_attributes {
	FAN_PCB_REG,
	FAN_MAJOR_VERSION_REG,
	FAN_MINOR_VERSION_REG,
	FAN_LED_REG,
	FAN_PRESENT_REG,
	FAN1_FRONT_PWM_REG,
	FAN1_REAR_PWM_REG,
	FAN2_FRONT_PWM_REG,
	FAN2_REAR_PWM_REG,
	FAN3_FRONT_PWM_REG,
	FAN3_REAR_PWM_REG,
	FAN4_FRONT_PWM_REG,
	FAN4_REAR_PWM_REG,
	FAN1_FRONT_SPEED_RPM_REG,
	FAN1_REAR_SPEED_RPM_REG,
	FAN2_FRONT_SPEED_RPM_REG,
	FAN2_REAR_SPEED_RPM_REG,
	FAN3_REAR_SPEED_RPM_REG,
	FAN3_FRONT_SPEED_RPM_REG,
	FAN4_FRONT_SPEED_RPM_REG,
	FAN4_REAR_SPEED_RPM_REG,

	FAN1_FRONT_SPEED_RPM,
	FAN1_REAR_SPEED_RPM,
	FAN2_FRONT_SPEED_RPM,
	FAN2_REAR_SPEED_RPM,
	FAN3_REAR_SPEED_RPM,
	FAN3_FRONT_SPEED_RPM,
	FAN4_FRONT_SPEED_RPM,
	FAN4_REAR_SPEED_RPM,
	FAN1_FRONT_TARGET_SPEED_RPM,
	FAN2_FRONT_TARGET_SPEED_RPM,
	FAN3_FRONT_TARGET_SPEED_RPM,
	FAN4_FRONT_TARGET_SPEED_RPM,
	FAN1_REAR_TARGET_SPEED_RPM,
	FAN2_REAR_TARGET_SPEED_RPM,
	FAN3_REAR_TARGET_SPEED_RPM,
	FAN4_REAR_TARGET_SPEED_RPM,
	FAN1_FRONT_TOLERANCE,
	FAN1_REAR_TOLERANCE,
	FAN2_FRONT_TOLERANCE,
	FAN2_REAR_TOLERANCE,
	FAN3_REAR_TOLERANCE,
	FAN3_FRONT_TOLERANCE,
	FAN4_FRONT_TOLERANCE,
	FAN4_REAR_TOLERANCE,
	FAN1_DIRECTION,
	FAN2_DIRECTION,
	FAN3_DIRECTION,
	FAN4_DIRECTION,
	FAN1_PRESENT,
	FAN2_PRESENT,
	FAN3_PRESENT,
	FAN4_PRESENT,
	FAN1_FRONT_FAULT,
	FAN1_REAR_FAULT,
	FAN2_FRONT_FAULT,
	FAN2_REAR_FAULT,
	FAN3_FRONT_FAULT,
	FAN3_REAR_FAULT,
	FAN4_FRONT_FAULT,
	FAN4_REAR_FAULT,
	FAN1_PWM,
	FAN2_PWM,
	FAN3_PWM,
	FAN4_PWM,
	FAN1_LED,
	FAN2_LED,
	FAN3_LED,
	FAN4_LED,
	FAN_FW_VERSION,
	FAN_PCB_VERSION,
	FAN_ACCESS
};

enum fan_led_light_mode {
	FAN_LED_MODE_OFF,
	FAN_LED_MODE_RED = 10,
	FAN_LED_MODE_GREEN = 16,
	FAN_LED_MODE_UNKNOWN = 99
};

enum fan_direction {
	FAN_DIR_F2B,
	FAN_DIR_B2F
};

static const int front_fan_target_speed_f2b[] = {
	4275, 4350, 4275, 4275, 4500, 5175, 6000, 6900,
	7725, 8625, 9375, 10200, 10875, 12075, 12300, 12150
};

static const int rear_fan_target_speed_f2b[] = {
	4725, 4725, 4800, 4725, 4950, 5925, 6825, 7575,
	8625, 9675, 10425, 11475, 12300, 12975, 14175, 14925
};

/* Define attributes
 */
#define DECLARE_FAN_SENSOR_DEVICE_ATTR(index, index2) \
	static SENSOR_DEVICE_ATTR(fan##index##_present, S_IRUGO, fan_show_value, NULL, \
								FAN##index##_PRESENT); \
	static SENSOR_DEVICE_ATTR(fan##index##_pwm, S_IWUSR | S_IRUGO, fan_show_value, \
								set_duty_cycle, FAN##index##_PWM); \
	static SENSOR_DEVICE_ATTR(fan##index##_input, S_IRUGO, fan_show_value, NULL, \
								FAN##index##_FRONT_SPEED_RPM); \
	static SENSOR_DEVICE_ATTR(fan##index2##_input, S_IRUGO, fan_show_value, NULL, \
								FAN##index##_REAR_SPEED_RPM); \
	static SENSOR_DEVICE_ATTR(fan##index##_direction, S_IRUGO, fan_show_value, NULL, \
								FAN##index##_DIRECTION); \
	static SENSOR_DEVICE_ATTR(fan##index##_fault, S_IRUGO, fan_show_value, NULL, \
								FAN##index##_FRONT_FAULT); \
	static SENSOR_DEVICE_ATTR(fan##index2##_fault, S_IRUGO, fan_show_value, NULL, \
								FAN##index##_REAR_FAULT); \
	static SENSOR_DEVICE_ATTR(fan##index##_target, S_IRUGO, fan_show_value, \
								NULL, FAN##index##_FRONT_TARGET_SPEED_RPM); \
	static SENSOR_DEVICE_ATTR(fan##index2##_target, S_IRUGO, fan_show_value, \
								NULL, FAN##index##_REAR_TARGET_SPEED_RPM); \
	static SENSOR_DEVICE_ATTR(fan##index##_tolerance, S_IRUGO, fan_show_value,\
								NULL, FAN##index##_FRONT_TOLERANCE); \
	static SENSOR_DEVICE_ATTR(fan##index2##_tolerance, S_IRUGO, fan_show_value,\
								NULL, FAN##index##_REAR_TOLERANCE); \
	static SENSOR_DEVICE_ATTR(fan##index##_led, S_IWUSR | S_IRUGO, fan_show_value,\
								set_fan_led, FAN##index##_LED); \

#define DECLARE_FAN_ATTR(index, index2) \
	&sensor_dev_attr_fan##index##_present.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_pwm.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_input.dev_attr.attr, \
	&sensor_dev_attr_fan##index2##_input.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_direction.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_fault.dev_attr.attr, \
    &sensor_dev_attr_fan##index2##_fault.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_target.dev_attr.attr, \
	&sensor_dev_attr_fan##index2##_target.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_tolerance.dev_attr.attr, \
	&sensor_dev_attr_fan##index2##_tolerance.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_led.dev_attr.attr

#define DECLARE_FAN_FW_VERSION_SENSOR_DEV_ATTR() \
	static SENSOR_DEVICE_ATTR(version, S_IRUGO, fan_show_value, NULL, FAN_FW_VERSION)

#define DECLARE_FAN_FW_VERSION_ATTR()	&sensor_dev_attr_version.dev_attr.attr

#define DECLARE_FAN_PCB_VERSION_SENSOR_DEV_ATTR() \
	static SENSOR_DEVICE_ATTR(pcb_version, S_IRUGO, fan_show_value, NULL, FAN_PCB_VERSION)

#define DECLARE_FAN_PCB_VERSION_ATTR()	&sensor_dev_attr_pcb_version.dev_attr.attr

#define DECLARE_FAN_ACCESS_SENSOR_DEV_ATTR() \
	static SENSOR_DEVICE_ATTR(access, S_IWUSR | S_IRUGO, reg_read, reg_write, FAN_ACCESS)

#define DECLARE_FAN_ACCESS_ATTR()		&sensor_dev_attr_access.dev_attr.attr

DECLARE_FAN_SENSOR_DEVICE_ATTR(1, 5);
DECLARE_FAN_SENSOR_DEVICE_ATTR(2, 6);
DECLARE_FAN_SENSOR_DEVICE_ATTR(3, 7);
DECLARE_FAN_SENSOR_DEVICE_ATTR(4, 8);
DECLARE_FAN_FW_VERSION_SENSOR_DEV_ATTR();
DECLARE_FAN_PCB_VERSION_SENSOR_DEV_ATTR();
DECLARE_FAN_ACCESS_SENSOR_DEV_ATTR();

static struct attribute *as9817_64_fan_attributes[] = {
	/* fan related attributes */
	DECLARE_FAN_ATTR(1, 5),
	DECLARE_FAN_ATTR(2, 6),
	DECLARE_FAN_ATTR(3, 7),
	DECLARE_FAN_ATTR(4, 8),
	DECLARE_FAN_FW_VERSION_ATTR(),
	DECLARE_FAN_PCB_VERSION_ATTR(),
	DECLARE_FAN_ACCESS_ATTR(),
	NULL
};

#define FAN_DUTY_CYCLE_REG_MASK		 0xF
#define FAN_MAX_DUTY_CYCLE			  100
#define FAN_REG_VAL_TO_SPEED_RPM_STEP   150
#define FAN_TOLERANCE				   10

static int as9817_64_fan_read_value(struct i2c_client *client, u8 reg)
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

static int as9817_64_fan_write_value(struct i2c_client *client, u8 reg,
				     u8 value)
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

static u32 reg_val_to_speed_rpm(u8 reg_val)
{
	return (u32) reg_val *FAN_REG_VAL_TO_SPEED_RPM_STEP;
}

static u8 reg_val_to_direction(enum fan_id id)
{
	return FAN_DIR_F2B;
}

static u8 reg_val_to_is_present(u8 reg_val, enum fan_id id)
{
	u8 mask = BIT(7 - id);
	return !(reg_val & mask);
}

static u8 reg_val_to_color(u8 reg_val, enum fan_id id)
{
	u8 green_mask = (1 << (7 - (id * 2)));
	u8 red_mask = (1 << (6 - (id * 2)));

	if (!(reg_val & green_mask))
		return FAN_LED_MODE_GREEN;
	else if (!(reg_val & red_mask))
		return FAN_LED_MODE_RED;
	else
		return FAN_LED_MODE_OFF;
}

static u8 is_fan_fault(struct as9817_64_fan_data *data, int id)
{
	u8 ret = 1;

	/* Check if the front or rear fan is present or not. */
	if (!reg_val_to_is_present(data->reg_val[FAN_PRESENT_REG], (id >> 1))) {
		ret = 0; /* None present */
	}
	else {
		/* Check if the speed of front or rear fan is ZERO */
		if (reg_val_to_speed_rpm(data->reg_val[FAN1_FRONT_SPEED_RPM_REG + id]))
			ret = 0;
	}

	return ret;
}

static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
			      const char *buf, size_t count)
{
	int error, value;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as9817_64_fan_data *data = dev_get_drvdata(dev);
	u8 reg_val, idx;

	error = kstrtoint(buf, 10, &value);
	if (error) {
		return error;
	}

	if (value < 0 || value > FAN_MAX_DUTY_CYCLE) {
		return -EINVAL;
	}

	switch (attr->index) {
	case FAN1_PWM ... FAN4_PWM:
		reg_val = (value * 100) / 666;
		idx = ((attr->index - FAN1_PWM) * 2);

		mutex_lock(&data->update_lock);
		/* Front FAN */
		as9817_64_fan_write_value(data->client,
					  fan_reg[FAN1_FRONT_PWM_REG + idx],
					  reg_val);
		/* Rear FAN */
		as9817_64_fan_write_value(data->client,
					  fan_reg[FAN1_REAR_PWM_REG + idx],
					  reg_val);
		/* force update register */
		data->valid = 0;
		mutex_unlock(&data->update_lock);
		break;
	default:
		break;
	}

	return count;
}

static ssize_t set_fan_led(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count)
{
	int error, value;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as9817_64_fan_data *data = dev_get_drvdata(dev);
	u8 green_mask, red_mask;
	u8 reg_val;

	error = kstrtoint(buf, 10, &value);
	if (error) {
		return error;
	}

	if (value < 0) {
		return -EINVAL;
	}
	else if (value != FAN_LED_MODE_GREEN &&
			   value != FAN_LED_MODE_RED &&
			   value != FAN_LED_MODE_OFF) {
		return -EINVAL;
	}

	switch (attr->index) {
	case FAN1_LED ... FAN4_LED:
		green_mask = (1 << (7 - ((attr->index - FAN1_LED) * 2)));
		red_mask = (1 << (6 - ((attr->index - FAN1_LED) * 2)));
		reg_val = as9817_64_fan_read_value(data->client, fan_reg[FAN_LED_REG]);
		switch (value) {
		case FAN_LED_MODE_RED:
			reg_val |= green_mask;
			reg_val &= ~(red_mask);
			break;
		case FAN_LED_MODE_GREEN:
			reg_val |= red_mask;
			reg_val &= ~(green_mask);
			break;
		case FAN_LED_MODE_OFF:
			reg_val |= (green_mask);
			reg_val |= (red_mask);
			break;
		default:
			break;
		}
		mutex_lock(&data->update_lock);
		as9817_64_fan_write_value(data->client, fan_reg[FAN_LED_REG],
					  reg_val);
		/* force update register */
		data->valid = 0;
		mutex_unlock(&data->update_lock);
		break;
	default:
		break;
	}

	return count;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
			      char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as9817_64_fan_data *data = as9817_64_fan_update_device(dev);
	ssize_t ret = 0;
	u8 idx, reg_val;

	if (!data->valid) {
		return ret;
	}

	switch (attr->index) {
	case FAN_PCB_VERSION:
		ret = sprintf(buf, "0x%02x\n", data->reg_val[FAN_PCB_REG]);
		break;
	case FAN_FW_VERSION:
		ret = sprintf(buf, "%d.%d\n",
					data->reg_val[FAN_MAJOR_VERSION_REG],
					data->reg_val[FAN_MINOR_VERSION_REG]);
		break;
	case FAN1_PWM ... FAN4_PWM:
		/* FAN0_FRONT_PWM, FAN1_FRONT_PWM, FAN2_FRONT_PWM, FAN3_FRONT_PWM */
		reg_val = data->reg_val[FAN1_FRONT_PWM_REG +
					((attr->index - FAN1_PWM) * 2)] & 0x0F;
		ret = sprintf(buf, "%u\n", (reg_val * 667) / 100);
		break;
	case FAN1_FRONT_SPEED_RPM ... FAN4_REAR_SPEED_RPM:
		idx = FAN1_FRONT_SPEED_RPM_REG + (attr->index - FAN1_FRONT_SPEED_RPM);
		ret = sprintf(buf, "%u\n",
				reg_val_to_speed_rpm(data->reg_val[idx]));
		break;
	case FAN1_PRESENT ... FAN4_PRESENT:
		ret = sprintf(buf, "%d\n",
			      reg_val_to_is_present(data->reg_val[FAN_PRESENT_REG],
						    attr->index - FAN1_PRESENT));
		break;
	case FAN1_FRONT_FAULT ... FAN4_REAR_FAULT:
		ret = sprintf(buf, "%d\n",
				is_fan_fault(data, attr->index - FAN1_FRONT_FAULT));
		break;
	case FAN1_DIRECTION ... FAN4_DIRECTION:
		ret = sprintf(buf, "%d\n",
			      reg_val_to_direction(attr->index -
						   FAN1_DIRECTION));
		break;
	case FAN1_FRONT_TARGET_SPEED_RPM ... FAN4_FRONT_TARGET_SPEED_RPM:
		idx = FAN1_FRONT_PWM_REG +
		    ((attr->index - FAN1_FRONT_TARGET_SPEED_RPM) * 2);
		ret = sprintf(buf, "%d\n",
				front_fan_target_speed_f2b[data->reg_val[idx]]);
		break;
	case FAN1_REAR_TARGET_SPEED_RPM ... FAN4_REAR_TARGET_SPEED_RPM:
		idx = FAN1_REAR_PWM_REG +
		    ((attr->index - FAN1_REAR_TARGET_SPEED_RPM) * 2);
		ret = sprintf(buf, "%d\n",
				rear_fan_target_speed_f2b[data->reg_val[idx]]);
		break;
	case FAN1_FRONT_TOLERANCE ... FAN4_REAR_TOLERANCE:
		ret = sprintf(buf, "%d\n", FAN_TOLERANCE);
		break;
	case FAN1_LED ... FAN4_LED:
		ret = sprintf(buf, "%d\n",
			      reg_val_to_color(data->reg_val[FAN_LED_REG],
					       attr->index - FAN1_LED));
		break;
	default:
		break;
	}

	return ret;
}

static ssize_t reg_read(struct device *dev, struct device_attribute *da,
			char *buf)
{
	struct as9817_64_fan_data *data = dev_get_drvdata(dev);
	ssize_t ret = -EINVAL;
	u8 reg_val;

	reg_val = as9817_64_fan_read_value(data->client, data->reg_addr);
	ret = sprintf(buf, "0x%02x\n", reg_val);

	return ret;
}

static ssize_t reg_write(struct device *dev, struct device_attribute *da,
			 const char *buf, size_t count)
{
	struct as9817_64_fan_data *data = dev_get_drvdata(dev);
	int args;
	char *opt, tmp[32] = { 0 };
	char *tmp_p;
	size_t copy_size;
	u8 input[2] = { 0 };

	copy_size = (count < sizeof(tmp)) ? count : sizeof(tmp) - 1;
#ifdef __STDC_LIB_EXT1__
	memcpy_s(tmp, copy_size, buf, copy_size);
#else
	memcpy(tmp, buf, copy_size);
#endif
	tmp[copy_size] = '\0';

	args = 0;
	tmp_p = tmp;
	while (args < 2 && (opt = strsep(&tmp_p, " ")) != NULL) {
		if (kstrtou8(opt, 16, &input[args]) == 0) {
			args++;
		}
	}

	switch (args) {
	case 2:
		/* Write value to register */
		mutex_lock(&data->update_lock);
		as9817_64_fan_write_value(data->client, input[0], input[1]);
		data->valid = 0;
		mutex_unlock(&data->update_lock);
		break;
	case 1:
		/* Read value from register */
		data->reg_addr = input[0];
		break;
	default:
		return -EINVAL;
	}

	return count;
}

static const struct attribute_group as9817_64_fan_group = {
	.attrs = as9817_64_fan_attributes,
};

__ATTRIBUTE_GROUPS(as9817_64_fan);

static struct as9817_64_fan_data *as9817_64_fan_update_device(struct device
							      *dev)
{
	struct as9817_64_fan_data *data = dev_get_drvdata(dev);

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ + HZ / 2) ||
	    !data->valid) {
		int i;

		dev_dbg(dev, "Starting as9817_64_fan update\n");
		data->valid = 0;

		/* Update fan data */
		for (i = 0; i < ARRAY_SIZE(data->reg_val); i++) {
			int status = as9817_64_fan_read_value(data->client, fan_reg[i]);
			if (status < 0) {
				data->valid = 0;
				mutex_unlock(&data->update_lock);
				dev_dbg(dev, "reg %d, err %d\n", fan_reg[i],
					status);
				return data;
			} else {
				data->reg_val[i] = status;
			}
		}

		data->last_updated = jiffies;
		data->valid = 1;
	}

	mutex_unlock(&data->update_lock);
	return data;
}

static int as9817_64_fan_probe(struct i2c_client *client,
			       const struct i2c_device_id *dev_id)
{
	struct as9817_64_fan_data *data;
	int status;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		status = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct as9817_64_fan_data), GFP_KERNEL);
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
					      as9817_64_fan_groups);
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

static int as9817_64_fan_remove(struct i2c_client *client)
{
	struct as9817_64_fan_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	kfree(data);

	return 0;
}

/* Addresses to scan */
static const unsigned short normal_i2c[] = { 0x33, I2C_CLIENT_END };

static const struct i2c_device_id as9817_64_fan_id[] = {
	{"as9817_64_fan", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, as9817_64_fan_id);

static struct i2c_driver as9817_64_fan_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		   .name = DRVNAME,
		   },
	.probe = as9817_64_fan_probe,
	.remove = as9817_64_fan_remove,
	.id_table = as9817_64_fan_id,
	.address_list = normal_i2c,
};

module_i2c_driver(as9817_64_fan_driver);

MODULE_AUTHOR("Roger Ho <roger530_ho@accton.com>");
MODULE_DESCRIPTION("AS9817-64-NB FAN Driver");
MODULE_LICENSE("GPL");
