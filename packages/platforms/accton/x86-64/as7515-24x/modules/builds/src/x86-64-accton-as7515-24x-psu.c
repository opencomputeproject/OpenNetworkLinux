// SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0
/*
 * An hwmon driver for accton as7515_24x Power Module
 *
 * Copyright (C) 2024 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com>
 *
 * Based on ad7414.c
 * Copyright 2006 Stefan Roese <sr at denx.de>, DENX Software Engineering
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
#include <linux/delay.h>
#include <linux/dmi.h>
#include <linux/string.h>

#define DRVNAME "as7515_24x_psu"

#define PSU_REG_PRESENT				0x51
#define PSU_REG_POWERGOOD			0x52

#define MODEL_NAME_LEN				12
#define MODEL_NAME_REG_OFFSET		0x0A

#define SERIAL_NUM_LEN				8
#define SERIAL_NUM_REG_OFFSET		0x18

#define IS_POWER_GOOD(id, value)	(!!(value & BIT(id)))
#define IS_PRESENT(id, value)		(!(value & BIT(id)))

static ssize_t show_status(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_string(struct device *dev, struct device_attribute *da, char *buf);
extern int as7515_24x_fpga_read(u8 reg);

/* Addresses scanned
 */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

/* Each client has this additional data
 */
struct as7515_24x_psu_data {
	struct device *hwmon_dev;
	struct mutex update_lock;
	char valid;		   /* !=0 if registers are valid */
	unsigned long last_updated;	/* In jiffies */
	u8  index;  /* PSU index */
	u8  status[2]; /* Status(present/power_good) register read from CPLD */
	char model_name[MODEL_NAME_LEN+1]; /* Model name, read from eeprom */
	char serial[SERIAL_NUM_LEN+1]; /* Serial number, read from eeprom*/
};

static struct as7515_24x_psu_data *as7515_24x_psu_update_device(struct device *dev);

enum as7515_24x_psu_sysfs_attributes {
	PSU_PRESENT,
	PSU_POWER_GOOD,
	PSU_MODEL_NAME,
	PSU_SERIAL_NUMBER
};

/* sysfs attributes for hwmon
 */
static SENSOR_DEVICE_ATTR(psu_present,	S_IRUGO, show_status,	NULL, PSU_PRESENT);
static SENSOR_DEVICE_ATTR(psu_model_name, S_IRUGO, show_string,	NULL, PSU_MODEL_NAME);
static SENSOR_DEVICE_ATTR(psu_power_good, S_IRUGO, show_status,	NULL, PSU_POWER_GOOD);
static SENSOR_DEVICE_ATTR(psu_serial_numer, S_IRUGO, show_string,	NULL, PSU_SERIAL_NUMBER);

static struct attribute *as7515_24x_psu_attributes[] = {
	&sensor_dev_attr_psu_present.dev_attr.attr,
	&sensor_dev_attr_psu_model_name.dev_attr.attr,
	&sensor_dev_attr_psu_power_good.dev_attr.attr,
	&sensor_dev_attr_psu_serial_numer.dev_attr.attr,
	NULL
};

static ssize_t show_status(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as7515_24x_psu_data *data = i2c_get_clientdata(client);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	u8 status = 0;

	mutex_lock(&data->update_lock);

	data = as7515_24x_psu_update_device(dev);
	if (!data->valid) {
		mutex_unlock(&data->update_lock);
		return sprintf(buf, "0\n");
	}

	if (attr->index == PSU_PRESENT) {
		status = IS_PRESENT(data->index, data->status[PSU_PRESENT]);
	}
	else { /* PSU_POWER_GOOD */
		status = IS_POWER_GOOD(data->index, data->status[PSU_POWER_GOOD]);
	}

	mutex_unlock(&data->update_lock);
	return sprintf(buf, "%d\n", status);
}

static ssize_t show_string(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as7515_24x_psu_data *data = i2c_get_clientdata(client);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	char *ptr = NULL;
	int ret = 0;

	mutex_lock(&data->update_lock);

	data = as7515_24x_psu_update_device(dev);
	if (!data->valid) {
		ret = -EIO;
		goto exit;
	}

	switch (attr->index) {
	case PSU_MODEL_NAME:
		ptr = data->model_name;
		break;
	case PSU_SERIAL_NUMBER:
		ptr = data->serial;
		break;
	default:
		ret = -EINVAL;
		goto exit;
	}

	ret = sprintf(buf, "%s\n", ptr);

exit:
	mutex_unlock(&data->update_lock);
	return ret;
}

static const struct attribute_group as7515_24x_psu_group = {
	.attrs = as7515_24x_psu_attributes,
};

static int as7515_24x_psu_probe(struct i2c_client *client,
			const struct i2c_device_id *dev_id)
{
	struct as7515_24x_psu_data *data;
	int status;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE)) {
		status = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct as7515_24x_psu_data), GFP_KERNEL);
	if (!data) {
		status = -ENOMEM;
		goto exit;
	}

	data->valid = 0;
	data->index = dev_id->driver_data;
	mutex_init(&data->update_lock);
	i2c_set_clientdata(client, data);

	dev_info(&client->dev, "chip found\n");

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &as7515_24x_psu_group);
	if (status) {
		goto exit_free;
	}

	data->hwmon_dev = hwmon_device_register_with_info(&client->dev,
											DRVNAME, NULL, NULL, NULL);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	dev_info(&client->dev, "%s: psu '%s'\n",
		 dev_name(data->hwmon_dev), client->name);

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &as7515_24x_psu_group);
exit_free:
	kfree(data);
exit:
	return status;
}

static int as7515_24x_psu_remove(struct i2c_client *client)
{
	struct as7515_24x_psu_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &as7515_24x_psu_group);
	kfree(data);

	return 0;
}

enum psu_index
{
	as7515_24x_psu1,
	as7515_24x_psu2
};

static const struct i2c_device_id as7515_24x_psu_id[] = {
	{ "as7515_24x_psu1", as7515_24x_psu1 },
	{ "as7515_24x_psu2", as7515_24x_psu2 },
	{}
};
MODULE_DEVICE_TABLE(i2c, as7515_24x_psu_id);

static struct i2c_driver as7515_24x_psu_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	 = DRVNAME,
	},
	.probe		= as7515_24x_psu_probe,
	.remove	   = as7515_24x_psu_remove,
	.id_table	 = as7515_24x_psu_id,
	.address_list = normal_i2c,
};

static int as7515_24x_psu_read_byte(struct i2c_client *client, u8 command, u8 *data)
{
	int status = 0;
	int retry_count = 5;

	while (retry_count) {
		status = i2c_smbus_read_byte_data(client, command);
		if (unlikely(status < 0)) {
			msleep(10);
			retry_count--;
			continue;
		}

		break;
	}

	if (unlikely(status < 0)) {
		dev_dbg(&client->dev, "sfp read byte data failed, command(0x%2x), data(0x%2x)\r\n", command, status);
		goto abort;
	}

	*data  = (u8)status;

abort:
	return status;
}

static int as7515_24x_psu_read_bytes(struct i2c_client *client, u8 command, u8 *data,
			  int data_len)
{
	int ret = 0;

	while (data_len) {
		ssize_t status;

		status = as7515_24x_psu_read_byte(client, command, data);
		if (status <= 0) {
			ret = status;
			break;
		}

		data += 1;
		command  += 1;
		data_len -= 1;
	}

	return ret;
}

static struct as7515_24x_psu_data *as7515_24x_psu_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as7515_24x_psu_data *data = i2c_get_clientdata(client);

	if (time_after(jiffies, data->last_updated + HZ + HZ / 2)
		|| !data->valid) {
		int i, status;
		u8 status_regs[] = { PSU_REG_PRESENT, PSU_REG_POWERGOOD };

		dev_dbg(&client->dev, "Starting as7515_24x update\n");
		data->valid = 0;

		/* Read psu status */
		for (i = 0; i < ARRAY_SIZE(status_regs); i++) {
			status = as7515_24x_fpga_read(status_regs[i]);

			if (status < 0) {
				dev_dbg(&client->dev, "fpga reg err %d\n", status);
				goto exit;
			}
			else {
				data->status[i] = status;
			}
		}

		memset(data->model_name, 0, sizeof(data->model_name));
		memset(data->serial, 0, sizeof(data->serial));

		if (IS_POWER_GOOD(data->index, data->status[PSU_POWER_GOOD])) {
			/* Read model name */
			status = as7515_24x_psu_read_bytes(client, MODEL_NAME_REG_OFFSET, data->model_name,
											   ARRAY_SIZE(data->model_name)-1);
			if (status < 0) {
				data->model_name[0] = '\0';
				dev_dbg(&client->dev, "unable to read model name from (0x%x)\n", client->addr);
				goto exit;
			}

			/* Read serial number */
			status = as7515_24x_psu_read_bytes(client, SERIAL_NUM_REG_OFFSET, data->serial,
											   ARRAY_SIZE(data->serial)-1);
			if (status < 0) {
				data->serial[0] = '\0';
				dev_dbg(&client->dev, "unable to read serial number from (0x%x)\n", client->addr);
				goto exit;
			}
			else {
				data->serial[ARRAY_SIZE(data->serial)-1] = '\0';
			}
		}

		data->last_updated = jiffies;
		data->valid = 1;
	}

exit:
	return data;
}

module_i2c_driver(as7515_24x_psu_driver);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as7515_24x_psu driver");
MODULE_LICENSE("GPL");
