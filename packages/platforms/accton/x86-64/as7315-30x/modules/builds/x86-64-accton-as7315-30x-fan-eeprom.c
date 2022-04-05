/*
 * An hwmon driver for accton as7315_30x Power Module
 *
 * Copyright (C) 2014 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com.tw>
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

#define FAN_DIR_LEN 6
#define FAN_DIR_REG_OFFSET 0x11

static ssize_t show_string(struct device *dev, struct device_attribute *da,
				char *buf);

/* Addresses scanned
 */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

/* Each client has this additional data
 */
struct as7315_30x_fan_eeprom_data {
	struct device *hwmon_dev;
	struct mutex update_lock;
	char valid; /* !=0 if registers are valid */
	unsigned long last_updated;	/* In jiffies */
	char fan_dir[FAN_DIR_LEN+1];
};

static struct as7315_30x_fan_eeprom_data *as7315_30x_fan_eeprom_update_device(
													struct device *dev);

enum as7315_30x_fan_eeprom_sysfs_attributes {
	FAN_DIRECTION
};

/* sysfs attributes for hwmon
 */
static SENSOR_DEVICE_ATTR(fan_dir, S_IRUGO, show_string, NULL, FAN_DIRECTION);

static struct attribute *as7315_30x_fan_eeprom_attributes[] = {
	&sensor_dev_attr_fan_dir.dev_attr.attr,
	NULL
};

static ssize_t show_string(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as7315_30x_fan_eeprom_data *data = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&data->update_lock);

	data = as7315_30x_fan_eeprom_update_device(dev);
	if (!data->valid) {
		mutex_unlock(&data->update_lock);
		return 0;
	}

	ret = sprintf(buf, "%s\n", data->fan_dir);

	mutex_unlock(&data->update_lock);
	return ret;
}


static int as7315_30x_fan_eeprom_read_byte(struct i2c_client *client,
										u8 command, u8 *data)
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
		dev_dbg(&client->dev,
				"psu read data failed, command(0x%2x), data(0x%2x)\r\n",
				command, status);
		goto abort;
	}

	*data  = (u8)status;

abort:
	return status;
}

static int as7315_30x_fan_eeprom_read_bytes(struct i2c_client *client,
										u8 command, u8 *data, int data_len)
{
	int ret = 0;

	while (data_len) {
		ssize_t status;

		status = as7315_30x_fan_eeprom_read_byte(client, command, data);
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

static struct as7315_30x_fan_eeprom_data *as7315_30x_fan_eeprom_update_device(
													struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as7315_30x_fan_eeprom_data *data = i2c_get_clientdata(client);

	if (time_after(jiffies, data->last_updated + HZ + HZ / 2)
		|| !data->valid) {
		int status;

		memset(data->fan_dir, 0, sizeof(data->fan_dir));

		/* Read model name */
		status = as7315_30x_fan_eeprom_read_bytes(client, FAN_DIR_REG_OFFSET,
						data->fan_dir, ARRAY_SIZE(data->fan_dir)-1);
		if (status < 0) {
			data->fan_dir[0] = '\0';
			dev_dbg(&client->dev,"unable to read fan direction from (0x%x)\n",
						client->addr);
			goto exit;
		}

		data->last_updated = jiffies;
		data->valid = 1;
	}

exit:
	return data;
}

static const struct attribute_group as7315_30x_fan_eeprom_group = {
	.attrs = as7315_30x_fan_eeprom_attributes,
};

static int as7315_30x_fan_eeprom_probe(struct i2c_client *client,
			const struct i2c_device_id *dev_id)
{
	struct as7315_30x_fan_eeprom_data *data;
	int status;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		status = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct as7315_30x_fan_eeprom_data), GFP_KERNEL);
	if (!data) {
		status = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	data->valid = 0;
	mutex_init(&data->update_lock);

	dev_info(&client->dev, "chip found\n");

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj,&as7315_30x_fan_eeprom_group);
	if (status)
		goto exit_free;

	data->hwmon_dev = hwmon_device_register_with_info(&client->dev,
									"as7315_30x_fan_eeprom", NULL, NULL, NULL);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	dev_info(&client->dev, "%s: fan '%s'\n",
		dev_name(data->hwmon_dev), client->name);

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &as7315_30x_fan_eeprom_group);
exit_free:
	kfree(data);
exit:

	return status;
}

static int as7315_30x_fan_eeprom_remove(struct i2c_client *client)
{
	struct as7315_30x_fan_eeprom_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &as7315_30x_fan_eeprom_group);
	kfree(data);

	return 0;
}

enum fan_index
{
	as7315_fan_eeprom
};

static const struct i2c_device_id as7315_30x_fan_eeprom_id[] = {
	{ "as7315_fan_eeprom", as7315_fan_eeprom },
	{}
};
MODULE_DEVICE_TABLE(i2c, as7315_30x_fan_eeprom_id);

static struct i2c_driver as7315_30x_fan_eeprom_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = "as7315_fan_eeprom",
	},
	.probe = as7315_30x_fan_eeprom_probe,
	.remove = as7315_30x_fan_eeprom_remove,
	.id_table = as7315_30x_fan_eeprom_id,
	.address_list = normal_i2c,
};

module_i2c_driver(as7315_30x_fan_eeprom_driver);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as7315_30x_fan_eeprom driver");
MODULE_LICENSE("GPL");
