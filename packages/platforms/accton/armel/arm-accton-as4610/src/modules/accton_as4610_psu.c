/*
 * An hwmon driver for accton as4610 Power Module
 *
 * Copyright (C) 2016 Accton Technology Corporation.
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
#include <linux/delay.h>
#include <linux/dmi.h>

static ssize_t show_status(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_model_name(struct device *dev, struct device_attribute *da, char *buf);
static int as4610_psu_read_data(struct i2c_client *client, u8 command, u8 *data,int data_len);
extern int accton_i2c_cpld_read(unsigned short cpld_addr, u8 reg);

/* Addresses scanned
 */
static const unsigned short normal_i2c[] = { 0x50, 0x53, I2C_CLIENT_END };

/* Each client has this additional data
 */
struct as4610_psu_data {
	struct device	   *hwmon_dev;
	struct mutex		update_lock;
	char				valid;			 /* !=0 if registers are valid */
	unsigned long		last_updated;	 /* In jiffies */
	u8	index;			 /* PSU index */
	u8	status;			 /* Status(present/power_good) register read from CPLD */
	char model_name[9]; /* Model name, read from eeprom */
};

static struct as4610_psu_data *as4610_psu_update_device(struct device *dev);

enum as4610_psu_sysfs_attributes {
	PSU_PRESENT,
	PSU_MODEL_NAME,
	PSU_POWER_GOOD
};

/* sysfs attributes for hwmon
 */
static SENSOR_DEVICE_ATTR(psu_present,	  S_IRUGO, show_status,	   NULL, PSU_PRESENT);
static SENSOR_DEVICE_ATTR(psu_model_name, S_IRUGO, show_model_name,NULL, PSU_MODEL_NAME);
static SENSOR_DEVICE_ATTR(psu_power_good, S_IRUGO, show_status,	   NULL, PSU_POWER_GOOD);

static struct attribute *as4610_psu_attributes[] = {
	&sensor_dev_attr_psu_present.dev_attr.attr,
	&sensor_dev_attr_psu_model_name.dev_attr.attr,
	&sensor_dev_attr_psu_power_good.dev_attr.attr,
	NULL
};

static ssize_t show_status(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as4610_psu_data *data = as4610_psu_update_device(dev);
	u8 status = 0;

	if (attr->index == PSU_PRESENT) {
		status = (data->status >> (data->index*2) & 0x1);
	}
	else { /* PSU_POWER_GOOD */
		status = (data->status >> (data->index*2 + 1) & 0x1);
	}

	return sprintf(buf, "%d\n", status);
}

static ssize_t show_model_name(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct as4610_psu_data *data = as4610_psu_update_device(dev);

	return sprintf(buf, "%s\n", data->model_name);
}

static const struct attribute_group as4610_psu_group = {
	.attrs = as4610_psu_attributes,
};

static int as4610_psu_probe(struct i2c_client *client,
			const struct i2c_device_id *dev_id)
{
	struct as4610_psu_data *data;
	int status;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		status = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct as4610_psu_data), GFP_KERNEL);
	if (!data) {
		status = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	data->valid = 0;
	data->index = dev_id->driver_data;
	mutex_init(&data->update_lock);

	dev_info(&client->dev, "chip found\n");

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &as4610_psu_group);
	if (status) {
		goto exit_free;
	}

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	dev_info(&client->dev, "%s: psu '%s'\n",
		 dev_name(data->hwmon_dev), client->name);

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &as4610_psu_group);
exit_free:
	kfree(data);
exit:

	return status;
}

static int as4610_psu_remove(struct i2c_client *client)
{
	struct as4610_psu_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &as4610_psu_group);
	kfree(data);

	return 0;
}

enum psu_index
{
	as4610_psu1,
	as4610_psu2
};

static const struct i2c_device_id as4610_psu_id[] = {
	{ "as4610_psu1", as4610_psu1 },
	{ "as4610_psu2", as4610_psu2 },
	{}
};
MODULE_DEVICE_TABLE(i2c, as4610_psu_id);

static struct i2c_driver as4610_psu_driver = {
	.class		  = I2C_CLASS_HWMON,
	.driver = {
		.name	  = "as4610_psu",
	},
	.probe		  = as4610_psu_probe,
	.remove		  = as4610_psu_remove,
	.id_table	  = as4610_psu_id,
	.address_list = normal_i2c,
};

static int as4610_psu_read_data(struct i2c_client *client, u8 command, u8 *data,
			  int count)
{
	int status = 0;

	while (count) {
		status = i2c_smbus_read_byte_data(client, command);
		if (unlikely(status < 0)) {
			break;
		}

		*data	 = (u8)status;
		data	+= 1;
		command += 1;
		count	-= 1;
	}

	return status;
}

static struct as4610_psu_data *as4610_psu_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as4610_psu_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ + HZ / 2)
		|| !data->valid) {
		int status;
		int present = 0;

		data->valid	 = 0;
		data->status = 0;
		dev_dbg(&client->dev, "Starting as4610 update\n");

		/* Read psu status */
		status = accton_i2c_cpld_read(0x30, 0x11);

		if (status < 0) {
			dev_dbg(&client->dev, "cpld reg 0x30 err %d\n", status);
			goto exit;
		}
		else {
			data->status = status;
		}

		/* Read model name */
		memset(data->model_name, 0, sizeof(data->model_name));
		present = (data->status >> (data->index*2) & 0x1);

		if (present) {
			int len = ARRAY_SIZE(data->model_name)-1;

			status = as4610_psu_read_data(client, 0x20, data->model_name,
											   ARRAY_SIZE(data->model_name)-1);

			if (status < 0) {
				data->model_name[0] = '\0';
				dev_dbg(&client->dev, "unable to read model name from (0x%x)\n", client->addr);
				goto exit;
			}
			else {
				data->model_name[ARRAY_SIZE(data->model_name)-1] = '\0';
			}
		}

		data->last_updated = jiffies;
		data->valid = 1;
	}

exit:
	mutex_unlock(&data->update_lock);

	return data;
}

static int __init as4610_psu_init(void)
{
	return i2c_add_driver(&as4610_psu_driver);
}

static void __exit as4610_psu_exit(void)
{
	i2c_del_driver(&as4610_psu_driver);
}

module_init(as4610_psu_init);
module_exit(as4610_psu_exit);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as4610_psu driver");
MODULE_LICENSE("GPL");

