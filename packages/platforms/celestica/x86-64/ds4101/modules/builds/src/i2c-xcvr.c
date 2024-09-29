// SPDX-License-Identifier: GPL-2.0+
/*
 * i2c-xcvr.c - Celestica XCVR control/status driver
 * based on PCA9535 i/o expander.
 * Copyright (C) 2023 Celestica Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/i2c.h>
#include <linux/acpi.h>
#include <linux/hwmon.h>

/* One PCA9535 manage 4 OSFP ports*/
#define PCA9535_MAN_PORT_NUM 4

#define PCA9535_1_I2C_ADDR 0x20
#define PCA9535_2_I2C_ADDR 0x21
#define PCA9535_3_I2C_ADDR 0x22
#define PCA9535_4_I2C_ADDR 0x23
#define PCA9535_5_I2C_ADDR 0x24
#define PCA9535_6_I2C_ADDR 0x25
#define PCA9535_7_I2C_ADDR 0x26
#define PCA9535_8_I2C_ADDR 0x27
#define PCA9535_NUM 8


#define INPUT_PORT_REG0		0x00
#define INPUT_PORT_REG1		0x01
#define OUTPUT_PORT_REG0	0x02
#define OUTPUT_PORT_REG1	0x03
#define POL_INVER_REG0		0x04
#define POL_INVER_REG1		0x05
#define CONFIG_REG0			0x06
#define CONFIG_REG1			0x07

#define OUTPUT		0x00
#define INPUT		0xff

#define INPUT_PORT_INTIO_OFFSET		0
#define INPUT_PORT_PRSIO_OFFSET		4
#define OUTPUT_PORT_RSTIO_OFFSET	0
#define OUTPUT_PORT_LPWIO_OFFSET	4

/* Private for all SFF devices */
struct xcvr_data {
	struct device *sff_devices[PCA9535_MAN_PORT_NUM];
	struct i2c_client *client;
	struct class *class;
};

/* private data for individual SFF device */
struct sff_dev_data {
	int portid;
	struct i2c_client *client;
};


/* OSFP attributes */
static ssize_t osfp_reset_show(struct device *dev,
								struct device_attribute *attr,
								char *buf)
{
	int value;
	u8 portio = 0;
	struct sff_dev_data *sff_data = dev_get_drvdata(dev);
	struct i2c_client *client = sff_data->client;

	portio = sff_data->portid + OUTPUT_PORT_RSTIO_OFFSET;
	value = i2c_smbus_read_byte_data(client, OUTPUT_PORT_REG0);
	if (value < 0)
		return value;

	return sprintf(buf, "%d\n", (value >> portio) & 0x01);

}
static ssize_t osfp_reset_store(struct device *dev,
								struct device_attribute *attr,
								const char *buf,
								size_t size)
{
	ssize_t status;
	long value;
	u8 data;
	u8 portio = 0;
	struct sff_dev_data *sff_data = dev_get_drvdata(dev);
	struct i2c_client *client = sff_data->client;

	portio = sff_data->portid + OUTPUT_PORT_RSTIO_OFFSET;
	status = kstrtol(buf, 0, &value);
	if (status == 0) {
		// if value is 0, reset signal is low
		data = i2c_smbus_read_byte_data(client, OUTPUT_PORT_REG0);
		if (!value)
			data = data & ~((u8)0x1 << portio);
		else
			data = data | ((u8)0x1 << portio);

		i2c_smbus_write_byte_data(client, OUTPUT_PORT_REG0, data);
		status = size;
	}

	return status;

}

static ssize_t osfp_lpmode_show(struct device *dev,
								struct device_attribute *attr,
								char *buf)
{
	int value;
	u8 portio = 0;
	struct sff_dev_data *sff_data = dev_get_drvdata(dev);
	struct i2c_client *client = sff_data->client;

	portio = sff_data->portid + OUTPUT_PORT_LPWIO_OFFSET;
	value = i2c_smbus_read_byte_data(client, OUTPUT_PORT_REG0);
	if (value < 0)
		return value;

	return sprintf(buf, "%d\n", (value >> portio) & 0x01);

}
static ssize_t osfp_lpmode_store(struct device *dev,
								struct device_attribute *attr,
								const char *buf,
								size_t size)
{
	ssize_t status;
	long value;
	u8 data;
	u8 portio = 0;
	struct sff_dev_data *sff_data = dev_get_drvdata(dev);
	struct i2c_client *client = sff_data->client;

	portio = sff_data->portid + OUTPUT_PORT_LPWIO_OFFSET;

	status = kstrtol(buf, 0, &value);
	if (status == 0) {
		data = i2c_smbus_read_byte_data(client, OUTPUT_PORT_REG0);
		if (!value)
			data = data & ~((u8)0x1 << portio);
		else
			data = data | ((u8)0x1 << portio);

		i2c_smbus_write_byte_data(client, OUTPUT_PORT_REG0, data);
		status = size;
	}

	return status;

}

static ssize_t osfp_modprs_show(struct device *dev,
								struct device_attribute *attr,
								char *buf)
{
	u32 data;
	int len = 0;
	u8 portio = 0;
	struct sff_dev_data *sff_data = dev_get_drvdata(dev);
	struct i2c_client *client = sff_data->client;

	portio = sff_data->portid + INPUT_PORT_PRSIO_OFFSET;

	data = i2c_smbus_read_byte_data(client, INPUT_PORT_REG1);
	len = sprintf(buf, "%x\n", (data >> portio) & 0x01);

	return len;

}

static ssize_t osfp_modirq_show(struct device *dev,
								struct device_attribute *attr,
								char *buf)
{
	u32 data;
	int len = 0;
	u8 portio = 0;
	struct sff_dev_data *sff_data = dev_get_drvdata(dev);
	struct i2c_client *client = sff_data->client;

	portio = sff_data->portid + INPUT_PORT_INTIO_OFFSET;
	data = i2c_smbus_read_byte_data(client, INPUT_PORT_REG1);
	len = sprintf(buf, "%x\n", (data >> portio) & 0x01);

	return len;

}

DEVICE_ATTR_RW(osfp_reset);
DEVICE_ATTR_RW(osfp_lpmode);
DEVICE_ATTR_RO(osfp_modprs);
DEVICE_ATTR_RO(osfp_modirq);

static struct attribute *sff_attrs[] = {
	&dev_attr_osfp_modirq.attr,
	&dev_attr_osfp_modprs.attr,
	&dev_attr_osfp_lpmode.attr,
	&dev_attr_osfp_reset.attr,
	NULL,
};

static struct attribute_group sff_attr_grp = {
	.attrs = sff_attrs,
};
static const struct attribute_group *sff_attr_grps[] = {
	&sff_attr_grp,
	NULL
};


static struct device *sff_dev_init(struct device *dev, int portid)
{
	struct xcvr_data *data = dev_get_drvdata(dev);
	struct device *sff_device;
	struct sff_dev_data *sff_data;
	unsigned char pindex = 0;

	sff_data = kzalloc(sizeof(struct sff_dev_data), GFP_KERNEL);
	if (!sff_data) {
		dev_alert(dev, "Cannot allocate SFF device data @port%d", portid);
		return NULL;
	}

	/* The OSFP port ID start from 1 */
	sff_data->portid = portid;
	sff_data->client = data->client;

	/* PCA9535 addressed are consistent from 0x20 to 0x27,
	 * so we can use the address offset
	 * and interval to calculate the port index
	 */
	pindex = portid + 1
			+ (data->client->addr - PCA9535_1_I2C_ADDR)
			* PCA9535_MAN_PORT_NUM;

	sff_device = device_create_with_groups(data->class,
										   NULL,
										   MKDEV(0, 0),
										   sff_data,
										   sff_attr_grps,
										   "%s%d",
										   "OSFP", pindex);

	if (IS_ERR(sff_device)) {
		dev_alert(dev, "Cannot create SFF device @port%d", pindex);
		kfree(sff_data);
		return NULL;
	}

	dev_info(dev, "Create SFF device @port%d", pindex);
	return sff_device;
}


static int xcvr_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	u8 portid = 0;
	int err = 0;
	struct device *dev;
	struct xcvr_data *data;
	static struct class *class;
	struct sff_dev_data *sff_data;

	dev = &client->dev;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	/* Set io0-7 as output and io8-15 as input */
	i2c_smbus_write_byte_data(client, CONFIG_REG0, OUTPUT);
	i2c_smbus_write_byte_data(client, CONFIG_REG1, INPUT);

	/* Set Initial state */
	i2c_smbus_write_byte_data(client, OUTPUT_PORT_REG0, 0x00);

	data = devm_kzalloc(dev, sizeof(struct xcvr_data),
		GFP_KERNEL);

	if (!data) {
		err = -ENOMEM;
		goto fail_alloc_xcvr_data;
	}

	dev_set_drvdata(dev, data);
	data->client = client;

	if (class == NULL) {
		class = class_create(THIS_MODULE, "SFF");
		if (IS_ERR(data->class)) {
			dev_alert(dev, "Failed to create class\n");
			err = PTR_ERR(data->class);
			goto fail_alloc_xcvr_data;
		}
	}

	data->class = class;

	/* create 32 OSFP SYSFS */
	for (portid = 0; portid < PCA9535_MAN_PORT_NUM; portid++) {
		data->sff_devices[portid] = sff_dev_init(dev, portid);
		if (IS_ERR(data->sff_devices[portid])) {
			dev_alert(dev, "Failed to register device\n");
			err = PTR_ERR(data->sff_devices[portid]);
			goto fail_register_sff_device;
		}
	}
	return 0;

fail_register_sff_device:
	for (portid = 0; portid < PCA9535_MAN_PORT_NUM; portid++) {
		if (data->sff_devices[portid] != NULL) {
			sff_data = dev_get_drvdata(data->sff_devices[portid]);
			device_unregister(data->sff_devices[portid]);
			put_device(data->sff_devices[portid]);
			kfree(sff_data);
		}
	}
	device_destroy(data->class, MKDEV(0, 0));
	class_unregister(data->class);
	class_destroy(data->class);

fail_alloc_xcvr_data:
	return err;
}

static int xcvr_remove(struct i2c_client *client)
{
	u8 portid = 0;
	struct sff_dev_data *sff_data;
	struct device *dev = &client->dev;
	struct xcvr_data *data = dev_get_drvdata(dev);
	static u8 index = 1;

	for (portid = 0; portid < PCA9535_MAN_PORT_NUM; portid++) {
		if (data->sff_devices[portid] != NULL) {
			sff_data = dev_get_drvdata(data->sff_devices[portid]);
			device_unregister(data->sff_devices[portid]);
			put_device(data->sff_devices[portid]);
			kfree(sff_data);
		}
	}
	if (index == PCA9535_NUM) {
		device_destroy(data->class, MKDEV(0, 0));
		class_unregister(data->class);
		class_destroy(data->class);
	}
	index++;

	return 0;
}

static const struct i2c_device_id xcvr_ids[] = {
	{ "xcvr", 0 },
	{ /* END OF List */ }
};
MODULE_DEVICE_TABLE(i2c, xcvr_ids);

struct i2c_driver xcvr_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = "xcvr",
		.owner = THIS_MODULE,
	},
	.probe = xcvr_probe,
	.remove = xcvr_remove,
	.id_table = xcvr_ids,
};

module_i2c_driver(xcvr_driver);

MODULE_AUTHOR("Celestica Inc.");
MODULE_DESCRIPTION("Celestica CPLD XCVR driver");
MODULE_VERSION("0.0.2");
MODULE_LICENSE("GPL");

