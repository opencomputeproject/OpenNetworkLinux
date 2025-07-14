// SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0
/*
 * Copyright (C) 2024 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com>
 *
 * This module provides support for accessing the Accton CPLD and
 * retrieving the status of QSFP-DD/SFP port.
 * This includes the:
 *	 Accton as7515_24x FPGA/CPLD2/CPLD3
 *
 * Based on:
 *	pca954x.c from Kumar Gala <galak@kernel.crashing.org>
 * Copyright (C) 2006
 *
 * Based on:
 *	pca954x.c from Ken Harrenstien
 * Copyright (C) 2004 Google, Inc. (Ken Harrenstien)
 *
 * Based on:
 *	i2c-virtual_cb.c from Brian Kuschak <bkuschak@yahoo.com>
 * and
 *	pca9540.c from Jean Delvare <khali@linux-fr.org>.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/version.h>
#include <linux/stat.h>
#include <linux/hwmon-sysfs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#define DRVNAME "as7515_24x_cpld"

#define I2C_RW_RETRY_COUNT			(10)
#define I2C_RW_RETRY_INTERVAL		(60)	/* ms */

/* FPGA */
#define FPGA_BOARD_INFO_REG		(0x00)
#define FPGA_MAJOR_VER_REG		(0x01)
#define FPGA_MINOR_VER_REG		(0x02)

/* CPLD */
#define CPLD_MAJOR_VER_REG		(0x01)
#define CPLD_MINOR_VER_REG		(0x02)

static LIST_HEAD(cpld_client_list);
static struct mutex list_lock;

struct cpld_client_node {
	struct i2c_client *client;
	struct list_head list;
};

enum cpld_type {
	as7515_24x_fpga,
	as7515_24x_cpld
};

struct as7515_24x_cpld_data {
	u8 reg;
	enum cpld_type type;
	struct mutex update_lock;
	struct platform_device *led_pdev;
	struct platform_device *sfp_pdev;
};

static const struct i2c_device_id as7515_24x_cpld_id[] = {
	{"as7515_24x_fpga", as7515_24x_fpga},
	{"as7515_24x_cpld", as7515_24x_cpld},
	{}
};

MODULE_DEVICE_TABLE(i2c, as7515_24x_cpld_id);

enum as7515_24x_cpld_sysfs_attributes {
	BOARD_INFO,
	VERSION,
	ACCESS
};

/* sysfs attributes for hwmon */
static ssize_t show(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_version(struct device *dev, struct device_attribute *da,
			 char *buf);
static ssize_t reg_read(struct device *dev, struct device_attribute *da,
			 char *buf);
static ssize_t reg_write(struct device *dev, struct device_attribute *da,
			 const char *buf, size_t count);
static int __as7515_24x_cpld_read(struct i2c_client *client, u8 reg);
static int __as7515_24x_cpld_write(struct i2c_client *client, u8 reg,
			 u8 value);

/* declare transceiver attributes callback function */
static SENSOR_DEVICE_ATTR(board_info, S_IRUGO, show, NULL, BOARD_INFO);
static SENSOR_DEVICE_ATTR(version, S_IRUGO, show_version, NULL, VERSION);
static SENSOR_DEVICE_ATTR(access, S_IRUGO | S_IWUSR, reg_read, reg_write,
			  ACCESS);

static struct attribute *as7515_24x_fpga_attributes[] = {
	&sensor_dev_attr_board_info.dev_attr.attr,
	&sensor_dev_attr_version.dev_attr.attr,
	&sensor_dev_attr_access.dev_attr.attr,
	NULL
};

static struct attribute *as7515_24x_cpld_attributes[] = {
	&sensor_dev_attr_version.dev_attr.attr,
	&sensor_dev_attr_access.dev_attr.attr,
	NULL
};

static const struct attribute_group as7515_24x_fpga_group = {
	.attrs = as7515_24x_fpga_attributes,
};

static const struct attribute_group as7515_24x_cpld_group = {
	.attrs = as7515_24x_cpld_attributes,
};

static ssize_t show_version(struct device *dev, struct device_attribute *da,
				char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as7515_24x_cpld_data *data = i2c_get_clientdata(client);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	int reg_major = 0, reg_minor = 0;
	int major, minor;

	switch (attr->index) {
	case VERSION:
		switch (data->type) {
		case as7515_24x_fpga:
			reg_major = FPGA_MAJOR_VER_REG;
			reg_minor = FPGA_MINOR_VER_REG;
			break;
		case as7515_24x_cpld:
			reg_major = CPLD_MAJOR_VER_REG;
			reg_minor = CPLD_MINOR_VER_REG;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	major = i2c_smbus_read_byte_data(client, reg_major);
	if (major < 0) {
		dev_dbg(&client->dev, "cpld(0x%02x) reg(0x%02x) err %d\n",
			client->addr, reg_major, major);
		return major;
	}

	minor = i2c_smbus_read_byte_data(client, reg_minor);
	if (minor < 0) {
		dev_dbg(&client->dev, "cpld(0x%02x) reg(0x%02x) err %d\n",
			client->addr, reg_minor, minor);
		return minor;
	}

	return sprintf(buf, "%d.%d\n", major, minor);
}

static ssize_t show(struct device *dev, struct device_attribute *da, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	int val = 0;
	u8 reg = 0;
	u8 mask = 0;
	u8 invert = 0;
	u8 bits_shift = 0;

	switch (attr->index) {
	case BOARD_INFO:
		reg = FPGA_BOARD_INFO_REG;
		mask = 0x03;
		break;
	default:
		break;
	}

	val = i2c_smbus_read_byte_data(client, reg);
	if (val < 0) {
		dev_dbg(&client->dev, "cpld(0x%02x) reg(0x%02x) err %d\n",
			client->addr, reg, val);
		return -EIO;
	}

	val = (val >> bits_shift) & mask;
	return sprintf(buf, "%d\n", invert ? !(val) : (val));
}

static ssize_t reg_read(struct device *dev, struct device_attribute *da,
			char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as7515_24x_cpld_data *data = i2c_get_clientdata(client);
	int reg_val, status = 0;

	mutex_lock(&data->update_lock);
	reg_val = __as7515_24x_cpld_read(client, data->reg);
	if (unlikely(reg_val < 0)) {
		goto exit;
	}
	mutex_unlock(&data->update_lock);

	status = sprintf(buf, "0x%02x\n", reg_val);

 exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t reg_write(struct device *dev, struct device_attribute *da,
			 const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as7515_24x_cpld_data *data = i2c_get_clientdata(client);
	int args, status;
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
		status = __as7515_24x_cpld_write(client, input[0], input[1]);
		if (unlikely(status < 0)) {
			goto exit;
		}
		mutex_unlock(&data->update_lock);
		break;
	case 1:
		/* Read value from register */
		data->reg = input[0];
		break;
	default:
		return -EINVAL;
	}

	return count;

 exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static void as7515_24x_cpld_add_client(struct i2c_client *client)
{
	struct cpld_client_node *node =
		kzalloc(sizeof(struct cpld_client_node), GFP_KERNEL);

	if (!node) {
		dev_dbg(&client->dev,
			"Can't allocate cpld_client_node (0x%x)\n",
			client->addr);
		return;
	}

	node->client = client;

	mutex_lock(&list_lock);
	list_add(&node->list, &cpld_client_list);
	mutex_unlock(&list_lock);
}

static void as7515_24x_cpld_remove_client(struct i2c_client *client)
{
	struct list_head *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int found = 0;

	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list) {
		cpld_node =
			list_entry(list_node, struct cpld_client_node, list);

		if (cpld_node->client == client) {
			found = 1;
			break;
		}
	}

	if (found) {
		list_del(list_node);
		kfree(cpld_node);
	}

	mutex_unlock(&list_lock);
}

/*
 * I2C init/probing/exit functions
 */
static int as7515_24x_cpld_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adap = to_i2c_adapter(client->dev.parent);
	struct as7515_24x_cpld_data *data;
	int ret = -ENODEV;
	const struct attribute_group *group = NULL;

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE))
		goto exit;

	data = kzalloc(sizeof(struct as7515_24x_cpld_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);
	data->type = id->driver_data;

	/* Register sysfs hooks */
	switch (data->type) {
	case as7515_24x_fpga:
		group = &as7515_24x_fpga_group;
		break;
	case as7515_24x_cpld:
		group = &as7515_24x_cpld_group;
		break;
	default:
		break;
	}

	if (group) {
		ret = sysfs_create_group(&client->dev.kobj, group);
		if (ret) {
			goto exit_free;
		}
	}

	as7515_24x_cpld_add_client(client);

	if (data->type == as7515_24x_fpga) {
		data->led_pdev = platform_device_register_simple("as7515_24x_led", -1, NULL, 0);
		if (IS_ERR(data->led_pdev)) {
			ret = PTR_ERR(data->led_pdev);
			goto exit_remove_client;
		}
	}

	if (data->type == as7515_24x_cpld) {
		data->sfp_pdev = platform_device_register_simple("as7515_24x_sfp", -1, NULL, 0);
		if (IS_ERR(data->sfp_pdev)) {
			ret = PTR_ERR(data->sfp_pdev);
			goto exit_remove_client;
		}
	}

	return 0;

exit_remove_client:
	as7515_24x_cpld_remove_client(client);

	if (group) {
		sysfs_remove_group(&client->dev.kobj, group);
	}
 exit_free:
	kfree(data);
 exit:
	return ret;
}

static int as7515_24x_cpld_remove(struct i2c_client *client)
{
	struct as7515_24x_cpld_data *data = i2c_get_clientdata(client);
	const struct attribute_group *group = NULL;

	if (data->led_pdev)
		platform_device_unregister(data->led_pdev);

	if (data->sfp_pdev)
		platform_device_unregister(data->sfp_pdev);

	as7515_24x_cpld_remove_client(client);

	/* Remove sysfs hooks */
	switch (data->type) {
	case as7515_24x_fpga:
		group = &as7515_24x_fpga_group;
		break;
	case as7515_24x_cpld:
		group = &as7515_24x_cpld_group;
		break;
	default:
		break;
	}

	if (group) {
		sysfs_remove_group(&client->dev.kobj, group);
	}

	kfree(data);

	return 0;
}

static int __as7515_24x_cpld_read(struct i2c_client *client, u8 reg)
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

static int __as7515_24x_cpld_write(struct i2c_client *client, u8 reg,
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

int as7515_24x_cpld_read(unsigned short cpld_addr, u8 reg)
{
	struct list_head *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int ret = -EPERM;

	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list) {
		cpld_node = list_entry(list_node, struct cpld_client_node, list);

		if (cpld_node->client->addr == cpld_addr) {
			ret = __as7515_24x_cpld_read(cpld_node->client, reg);
			break;
		}
	}

	mutex_unlock(&list_lock);

	return ret;
}

EXPORT_SYMBOL(as7515_24x_cpld_read);

int as7515_24x_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
{
	struct list_head *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int ret = -EIO;

	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list) {
		cpld_node = list_entry(list_node, struct cpld_client_node, list);

		if (cpld_node->client->addr == cpld_addr) {
			ret = __as7515_24x_cpld_write(cpld_node->client, reg, value);
			break;
		}
	}

	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(as7515_24x_cpld_write);

static struct i2c_driver as7515_24x_cpld_driver = {
	.driver = {
		   .name = DRVNAME,
		   .owner = THIS_MODULE,
		   },
	.probe = as7515_24x_cpld_probe,
	.remove = as7515_24x_cpld_remove,
	.id_table = as7515_24x_cpld_id,
};

static int __init as7515_24x_cpld_init(void)
{
	mutex_init(&list_lock);
	return i2c_add_driver(&as7515_24x_cpld_driver);
}

static void __exit as7515_24x_cpld_exit(void)
{
	i2c_del_driver(&as7515_24x_cpld_driver);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com>");
MODULE_DESCRIPTION("AS7515-24X CPLD driver");
MODULE_LICENSE("GPL");

module_init(as7515_24x_cpld_init);
module_exit(as7515_24x_cpld_exit);
