/*
 * A hwmon driver for the as7535_32xb_fpga
 *
 * Copyright (C) 2019  Edgecore Networks Corporation.
 * Jostar Yang <brandon_chuang@edge-core.com>
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
#include <linux/list.h>

#define DRVNAME "as7535_28xb_fpga"

static LIST_HEAD(fpga_client_list);
static struct mutex	 list_lock;

struct fpga_client_node {
	struct i2c_client *client;
	struct list_head list;
};

enum fpga_type {
	as7535_28xb_fpga
};

#define I2C_RW_RETRY_COUNT 10
#define I2C_RW_RETRY_INTERVAL 60 /* ms */

static ssize_t show_status(struct device *dev, struct device_attribute *da,
			 char *buf);
static ssize_t set_control(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t access(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);

struct as7535_28xb_fpga_data {
	struct mutex update_lock;
	u8 index; /* FPGA index */
};

/* Addresses scanned for as7535_28xb_fpga
 */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

#define TRANSCEIVER_LPMODE_ATTR_ID(index) MODULE_LPMODE_##index

enum as7535_28xb_fpga_sysfs_attributes {
	/* transceiver attributes */
	TRANSCEIVER_LPMODE_ATTR_ID(2),
	TRANSCEIVER_LPMODE_ATTR_ID(4),
	TRANSCEIVER_LPMODE_ATTR_ID(1),
	TRANSCEIVER_LPMODE_ATTR_ID(3),
	ACCESS,
};

/* sysfs attributes for hwmon
 */

/* qsfp transceiver attributes */
#define DECLARE_QSFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(module_lpmode_##index, S_IRUGO | S_IWUSR, \
								show_status, set_control, MODULE_LPMODE_##index)
#define DECLARE_QSFP_TRANSCEIVER_ATTR(index) \
	&sensor_dev_attr_module_lpmode_##index.dev_attr.attr

static SENSOR_DEVICE_ATTR(access, S_IWUSR, NULL, access, ACCESS);

/* transceiver attributes */
DECLARE_QSFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(1);
DECLARE_QSFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(2);
DECLARE_QSFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(3);
DECLARE_QSFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(4);

static struct attribute *as7535_28xb_fpga_attributes[] = {
	/* transceiver attributes */
	DECLARE_QSFP_TRANSCEIVER_ATTR(1),
	DECLARE_QSFP_TRANSCEIVER_ATTR(2),
	DECLARE_QSFP_TRANSCEIVER_ATTR(3),
	DECLARE_QSFP_TRANSCEIVER_ATTR(4),
	&sensor_dev_attr_access.dev_attr.attr,
	NULL
};

static const struct attribute_group as7535_28xb_fpga_group = {
	.attrs = as7535_28xb_fpga_attributes,
};

static const struct attribute_group* fpga_groups[] = {
	&as7535_28xb_fpga_group
};

int as7535_28xb_fpga_read(int bus_num, unsigned short fpga_addr, u8 reg)
{
	struct list_head *list_node = NULL;
	struct fpga_client_node *fpga_node = NULL;
	int ret = -EPERM;

	mutex_lock(&list_lock);

	list_for_each(list_node, &fpga_client_list)
	{
		fpga_node = list_entry(list_node, struct fpga_client_node, list);

		if (fpga_node->client->addr == fpga_addr
			&& fpga_node->client->adapter->nr == bus_num) {
			ret = i2c_smbus_read_byte_data(fpga_node->client, reg);
			break;
		}
	}

	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(as7535_28xb_fpga_read);

int as7535_28xb_fpga_write(int bus_num, unsigned short fpga_addr, u8 reg, u8 value)
{
	struct list_head *list_node = NULL;
	struct fpga_client_node *fpga_node = NULL;
	int ret = -EIO;

	mutex_lock(&list_lock);

	list_for_each(list_node, &fpga_client_list)
	{
		fpga_node = list_entry(list_node, struct fpga_client_node, list);

		if (fpga_node->client->addr == fpga_addr
			&& fpga_node->client->adapter->nr == bus_num) {
			ret = i2c_smbus_write_byte_data(fpga_node->client, reg, value);
			break;
		}
	}

	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(as7535_28xb_fpga_write);

static ssize_t show_status(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct as7535_28xb_fpga_data *data = i2c_get_clientdata(client);
	int status = 0;
	u8 reg = 0, mask = 0, invert = 0;

	switch (attr->index) {
	case MODULE_LPMODE_2 ... MODULE_LPMODE_3:
		reg = 0x67;
		mask = 0x1 << (attr->index - MODULE_LPMODE_2);
		break;
 	default:
		return -ENXIO;
	}

	mutex_lock(&data->update_lock);

	status = as7535_28xb_fpga_read(11, 0x60, reg);
	if (unlikely(status < 0))
		goto exit;

	mutex_unlock(&data->update_lock);

	if (status & mask)
		return sprintf(buf, "1 \n");
	else
		return sprintf(buf, "0 \n");

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t set_control(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct as7535_28xb_fpga_data *data = i2c_get_clientdata(client);
	long lpmode;
	int status;
	u8 reg = 0, mask = 0;

	status = kstrtol(buf, 10, &lpmode);
	if (status)
		return status;

	switch (attr->index) {
	case MODULE_LPMODE_2 ... MODULE_LPMODE_3:
		reg = 0x67;
		mask = 0x1 << (attr->index - MODULE_LPMODE_2);
		break;
	default:
		return -ENXIO;
	}

	mutex_lock(&data->update_lock);
	/* Read current status */
	status = as7535_28xb_fpga_read(11, 0x60, reg);
	if (unlikely(status < 0))
		goto exit;

	/* Update status */
	if (lpmode)
		status |= mask;
	else
		status &= ~mask;

	status = as7535_28xb_fpga_write(11, 0x60, reg, status);
	if (unlikely(status < 0))
		goto exit;

	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static void as7535_28xb_fpga_add_client(struct i2c_client *client)
{
	struct fpga_client_node *node = kzalloc(sizeof(struct fpga_client_node),
											GFP_KERNEL);

	if (!node) {
		dev_dbg(&client->dev, "Can't allocate fpga_client_node (0x%x)\n",
								client->addr);
		return;
	}

	node->client = client;

	mutex_lock(&list_lock);
	list_add(&node->list, &fpga_client_list);
	mutex_unlock(&list_lock);
}

static void as7535_28xb_fpga_remove_client(struct i2c_client *client)
{
	struct list_head *list_node = NULL;
	struct fpga_client_node *fpga_node = NULL;
	int found = 0;

	mutex_lock(&list_lock);

	list_for_each(list_node, &fpga_client_list)
	{
		fpga_node = list_entry(list_node, struct fpga_client_node, list);

		if (fpga_node->client == client) {
			found = 1;
			break;
		}
	}

	if (found) {
		list_del(list_node);
		kfree(fpga_node);
	}

	mutex_unlock(&list_lock);
}

static ssize_t access(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	int status;
	u32 reg, val;
	struct i2c_client *client = to_i2c_client(dev);
	struct as7535_28xb_fpga_data *data = i2c_get_clientdata(client);

	if (sscanf(buf, "0x%x 0x%x", &reg, &val) != 2)
		return -EINVAL;

	if (reg > 0xFF || val > 0xFF)
		return -EINVAL;

	mutex_lock(&data->update_lock);

	status = as7535_28xb_fpga_write(11, 0x60, reg, val);
	if (unlikely(status < 0))
		goto exit;

	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static int as7535_28xb_fpga_probe(struct i2c_client *client,
			const struct i2c_device_id *dev_id)
{
	int status;
	struct as7535_28xb_fpga_data *data = NULL;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_dbg(&client->dev, "i2c_check_functionality failed (0x%x)\n",
								client->addr);
		status = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct as7535_28xb_fpga_data), GFP_KERNEL);
	if (!data) {
		status = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	data->index = dev_id->driver_data;
	mutex_init(&data->update_lock);
	dev_info(&client->dev, "chip found\n");

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, fpga_groups[data->index]);
	if (status)
		goto exit_free;

	as7535_28xb_fpga_add_client(client);

	dev_info(&client->dev, "%s: fpga '%s'\n",
							dev_name(&client->dev), client->name);

	return 0;

exit_free:
	kfree(data);
exit:

	return status;
}

static int as7535_28xb_fpga_remove(struct i2c_client *client)
{
	struct as7535_28xb_fpga_data *data = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, fpga_groups[data->index]);
	kfree(data);
	as7535_28xb_fpga_remove_client(client);

	return 0;
}

static const struct i2c_device_id as7535_28xb_fpga_id[] = {
	{ "as7535_28xb_fpga", as7535_28xb_fpga },
	{}
};
MODULE_DEVICE_TABLE(i2c, as7535_28xb_fpga_id);

static struct i2c_driver as7535_28xb_fpga_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = DRVNAME,
	},
	.probe = as7535_28xb_fpga_probe,
	.remove = as7535_28xb_fpga_remove,
	.id_table = as7535_28xb_fpga_id,
	.address_list = normal_i2c,
};

static int __init as7535_28xb_fpga_init(void)
{
	mutex_init(&list_lock);
	return i2c_add_driver(&as7535_28xb_fpga_driver);
}

static void __exit as7535_28xb_fpga_exit(void)
{
	i2c_del_driver(&as7535_28xb_fpga_driver);
}

module_init(as7535_28xb_fpga_init);
module_exit(as7535_28xb_fpga_exit);

MODULE_AUTHOR("Willy Liu <willy_liu@edge-core.com>");
MODULE_DESCRIPTION("as7535_28xb_fpga driver");
MODULE_LICENSE("GPL");
