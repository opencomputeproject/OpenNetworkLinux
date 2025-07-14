// SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0
/*
 * An hwmon driver for accton as7515 Power Module
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
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>

#define DRVNAME "as7515_24x_sfp"

#define SFP_STATUS_I2C_ADDR 0x61
#define PORT_NUM (4 + 20) /* 4 OSFP + 20 SFP */

static ssize_t show_present_all(struct device *dev, struct device_attribute *da,
			 char *buf);
static ssize_t show_rxlos_all(struct device *dev, struct device_attribute *da,
			 char *buf);
static ssize_t show_module(struct device *dev, struct device_attribute *da,
			 char *buf);
static ssize_t set_control(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t set_module_reset_all(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);

extern int as7515_24x_cpld_read(unsigned short cpld_addr, u8 reg);
extern int as7515_24x_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

/* Platform specific data
 */
struct as7515_sfp_data {
	struct mutex update_lock;
};

#define TRANSCEIVER_ATTR_ID_QSFP(index) \
	MODULE_PRESENT_##index =	(index-1), \
	MODULE_LPMODE_##index = 	(index-1) + (PORT_NUM * 1), \
	MODULE_RESET_##index =		(index-1) + (PORT_NUM * 2)

#define TRANSCEIVER_ATTR_ID_SFP(index) \
	MODULE_PRESENT_##index		= (index-1), \
	MODULE_TX_DISABLE_##index	= (index-1) + (PORT_NUM * 3), \
	MODULE_TX_FAULT_##index		= (index-1) + (PORT_NUM * 4), \
	MODULE_RX_LOS_##index		= (index-1) + (PORT_NUM * 5)

/*
 * MODULE_PRESENT_1     ... MODULE_PRESENT_24    =>   0 ...  23
 * MODULE_LPMODE_1      ... MODULE_LPMODE_4      =>  24 ...  27
 * MODULE_RESET_1       ... MODULE_RESET_4       =>  48 ...  51
 * MODULE_TX_DISABLE_5  ... MODULE_TX_DISABLE_24 =>  76 ...  95
 * MODULE_TX_FAULT_5    ... MODULE_TX_FAULT_24   => 100 ... 119
 * MODULE_RX_LOS_5      ... MODULE_RX_LOS_24     => 124 ... 143
 */

enum as7515_sfp_sysfs_attributes {
	MODULE_PRESENT_ALL,
	MODULE_RXLOS_ALL,
	MODULE_RESET_ALL,
	/* transceiver attributes */
	TRANSCEIVER_ATTR_ID_QSFP(1),
	TRANSCEIVER_ATTR_ID_QSFP(2),
	TRANSCEIVER_ATTR_ID_QSFP(3),
	TRANSCEIVER_ATTR_ID_QSFP(4),
	TRANSCEIVER_ATTR_ID_SFP(5),
	TRANSCEIVER_ATTR_ID_SFP(6),
	TRANSCEIVER_ATTR_ID_SFP(7),
	TRANSCEIVER_ATTR_ID_SFP(8),
	TRANSCEIVER_ATTR_ID_SFP(9),
	TRANSCEIVER_ATTR_ID_SFP(10),
	TRANSCEIVER_ATTR_ID_SFP(11),
	TRANSCEIVER_ATTR_ID_SFP(12),
	TRANSCEIVER_ATTR_ID_SFP(13),
	TRANSCEIVER_ATTR_ID_SFP(14),
	TRANSCEIVER_ATTR_ID_SFP(15),
	TRANSCEIVER_ATTR_ID_SFP(16),
	TRANSCEIVER_ATTR_ID_SFP(17),
	TRANSCEIVER_ATTR_ID_SFP(18),
	TRANSCEIVER_ATTR_ID_SFP(19),
	TRANSCEIVER_ATTR_ID_SFP(20),
	TRANSCEIVER_ATTR_ID_SFP(21),
	TRANSCEIVER_ATTR_ID_SFP(22),
	TRANSCEIVER_ATTR_ID_SFP(23),
	TRANSCEIVER_ATTR_ID_SFP(24)
};

/* sysfs attributes for hwmon
 */
#define MODULE_ATTRS_COMMON() \
	static SENSOR_DEVICE_ATTR(module_present_all, S_IRUGO, \
		show_present_all, NULL, MODULE_PRESENT_ALL); \
	static SENSOR_DEVICE_ATTR(module_rx_los_all, S_IRUGO, \
		show_rxlos_all, NULL, MODULE_RXLOS_ALL); \
	static SENSOR_DEVICE_ATTR(module_reset_all, S_IWUSR, \
		NULL, set_module_reset_all, MODULE_RESET_ALL); \
	static struct attribute *module_attributes_common[] = { \
		&sensor_dev_attr_module_present_all.dev_attr.attr, \
		&sensor_dev_attr_module_rx_los_all.dev_attr.attr, \
		&sensor_dev_attr_module_reset_all.dev_attr.attr, \
		NULL \
	}

#define MODULE_ATTRS_QSFP(index) \
	static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, \
		show_module, NULL, MODULE_PRESENT_##index); \
	static SENSOR_DEVICE_ATTR(module_reset_##index, S_IRUGO | S_IWUSR,\
		show_module, set_control, MODULE_RESET_##index); \
	static SENSOR_DEVICE_ATTR(module_lpmode_##index, S_IRUGO | S_IWUSR, \
		show_module, set_control, MODULE_LPMODE_##index); \
	static struct attribute *module_attributes##index[] = { \
		&sensor_dev_attr_module_present_##index.dev_attr.attr, \
		&sensor_dev_attr_module_reset_##index.dev_attr.attr, \
		&sensor_dev_attr_module_lpmode_##index.dev_attr.attr, \
		NULL \
	}

#define MODULE_ATTRS_SFP(index) \
	static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, \
		show_module, NULL, MODULE_PRESENT_##index); \
	static SENSOR_DEVICE_ATTR(module_tx_disable_##index, S_IRUGO | S_IWUSR,\
		show_module, set_control, MODULE_TX_DISABLE_##index); \
	static SENSOR_DEVICE_ATTR(module_rx_los_##index, S_IRUGO, \
		show_module, NULL, MODULE_RX_LOS_##index); \
	static SENSOR_DEVICE_ATTR(module_tx_fault_##index, S_IRUGO, \
		show_module, NULL, MODULE_TX_FAULT_##index); \
	static struct attribute *module_attributes##index[] = { \
		&sensor_dev_attr_module_present_##index.dev_attr.attr, \
		&sensor_dev_attr_module_tx_disable_##index.dev_attr.attr, \
		&sensor_dev_attr_module_rx_los_##index.dev_attr.attr, \
		&sensor_dev_attr_module_tx_fault_##index.dev_attr.attr, \
		NULL \
	}

MODULE_ATTRS_COMMON();
MODULE_ATTRS_QSFP(1);
MODULE_ATTRS_QSFP(2);
MODULE_ATTRS_QSFP(3);
MODULE_ATTRS_QSFP(4);
MODULE_ATTRS_SFP(5);
MODULE_ATTRS_SFP(6);
MODULE_ATTRS_SFP(7);
MODULE_ATTRS_SFP(8);
MODULE_ATTRS_SFP(9);
MODULE_ATTRS_SFP(10);
MODULE_ATTRS_SFP(11);
MODULE_ATTRS_SFP(12);
MODULE_ATTRS_SFP(13);
MODULE_ATTRS_SFP(14);
MODULE_ATTRS_SFP(15);
MODULE_ATTRS_SFP(16);
MODULE_ATTRS_SFP(17);
MODULE_ATTRS_SFP(18);
MODULE_ATTRS_SFP(19);
MODULE_ATTRS_SFP(20);
MODULE_ATTRS_SFP(21);
MODULE_ATTRS_SFP(22);
MODULE_ATTRS_SFP(23);
MODULE_ATTRS_SFP(24);

#define MODULE_ATTR_GROUP_COMMON() { .attrs = module_attributes_common }
#define MODULE_ATTR_GROUP_QSFP(index) { .attrs = module_attributes##index }
#define MODULE_ATTR_GROUP_SFP(index) { .attrs = module_attributes##index }

static struct attribute_group sfp_group[] = {
	MODULE_ATTR_GROUP_COMMON(),
	MODULE_ATTR_GROUP_QSFP(1),
	MODULE_ATTR_GROUP_QSFP(2),
	MODULE_ATTR_GROUP_QSFP(3),
	MODULE_ATTR_GROUP_QSFP(4),
	MODULE_ATTR_GROUP_SFP(5),
	MODULE_ATTR_GROUP_SFP(6),
	MODULE_ATTR_GROUP_SFP(7),
	MODULE_ATTR_GROUP_SFP(8),
	MODULE_ATTR_GROUP_SFP(9),
	MODULE_ATTR_GROUP_SFP(10),
	MODULE_ATTR_GROUP_SFP(11),
	MODULE_ATTR_GROUP_SFP(12),
	MODULE_ATTR_GROUP_SFP(13),
	MODULE_ATTR_GROUP_SFP(14),
	MODULE_ATTR_GROUP_SFP(15),
	MODULE_ATTR_GROUP_SFP(16),
	MODULE_ATTR_GROUP_SFP(17),
	MODULE_ATTR_GROUP_SFP(18),
	MODULE_ATTR_GROUP_SFP(19),
	MODULE_ATTR_GROUP_SFP(20),
	MODULE_ATTR_GROUP_SFP(21),
	MODULE_ATTR_GROUP_SFP(22),
	MODULE_ATTR_GROUP_SFP(23),
	MODULE_ATTR_GROUP_SFP(24)
};

struct attribute_mapping {
	u16 attr_base;
	u16 reg;
	u8 invert;
};

// Define an array of attribute mappings
static const struct attribute_mapping attribute_mappings[] = {
	[ MODULE_PRESENT_1 ... MODULE_PRESENT_4 ] = { MODULE_PRESENT_1, 0x10, 1 },
	[ MODULE_PRESENT_5 ... MODULE_PRESENT_12 ] = { MODULE_PRESENT_5, 0x11, 1 },
	[ MODULE_PRESENT_13 ... MODULE_PRESENT_20 ] = { MODULE_PRESENT_13, 0x12, 1 },
	[ MODULE_PRESENT_21 ... MODULE_PRESENT_24 ] = { MODULE_PRESENT_21, 0x13, 1 },
	[ MODULE_LPMODE_1 ... MODULE_LPMODE_4 ] = { MODULE_LPMODE_1, 0x09, 0 },
	[ MODULE_RESET_1 ... MODULE_RESET_4 ] = { MODULE_RESET_1, 0x08, 1 },
	[ MODULE_TX_DISABLE_5 ... MODULE_TX_DISABLE_12 ] = { MODULE_TX_DISABLE_5, 0x0A, 0 },
	[ MODULE_TX_DISABLE_13 ... MODULE_TX_DISABLE_20 ] = { MODULE_TX_DISABLE_13, 0x0B, 0 },
	[ MODULE_TX_DISABLE_21 ... MODULE_TX_DISABLE_24 ] = { MODULE_TX_DISABLE_21, 0x0C, 0 },
	[ MODULE_TX_FAULT_5 ... MODULE_TX_FAULT_12 ] = { MODULE_TX_FAULT_5, 0x15, 0 },
	[ MODULE_TX_FAULT_13 ... MODULE_TX_FAULT_20 ] = { MODULE_TX_FAULT_13, 0x16, 0 },
	[ MODULE_TX_FAULT_21 ... MODULE_TX_FAULT_24 ] = { MODULE_TX_FAULT_21, 0x17, 0 },
	[ MODULE_RX_LOS_5 ... MODULE_RX_LOS_12 ] = { MODULE_RX_LOS_5, 0x21, 0 },
	[ MODULE_RX_LOS_13 ... MODULE_RX_LOS_20 ] = { MODULE_RX_LOS_13, 0x22, 0 },
	[ MODULE_RX_LOS_21 ... MODULE_RX_LOS_24 ] = { MODULE_RX_LOS_21, 0x23, 0 }
};

static int as7515_sfp_read_value(u8 reg)
{
	return as7515_24x_cpld_read(SFP_STATUS_I2C_ADDR, reg);
}

static int as7515_sfp_write_value(u8 reg, u8 value)
{
	return as7515_24x_cpld_write(SFP_STATUS_I2C_ADDR, reg, value);
}

static ssize_t show_present_all(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int i, status;
	u8 regs[] = { 0x10, 0x11, 0x12, 0x13 };
	u8 values[ARRAY_SIZE(regs)] = { 0 };
	struct as7515_sfp_data *data = dev_get_drvdata(dev);

	mutex_lock(&data->update_lock);

	for (i = 0; i < ARRAY_SIZE(regs); i++) {
		status = as7515_sfp_read_value(regs[i]);
		if (status < 0)
			goto exit;

		values[i] = ~(u8)status;
	}

	mutex_unlock(&data->update_lock);

	/* Return values in order */
	return sprintf(buf, "%.2x %.2x %.2x\n",
	(unsigned int)((values[0] & 0x0F) | ((values[1] & 0xF) << 4)),
	(unsigned int)(((values[1] & 0xF0) >> 4) | ((values[2] & 0xF) << 4)),
	(unsigned int)(((values[2] & 0xF0) >> 4) | ((values[3] & 0xF) << 4)));

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t show_rxlos_all(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int i, status;
	u8 regs[] = { 0x21, 0x22, 0x23 };
	u8 values[ARRAY_SIZE(regs)] = { 0 };
	struct as7515_sfp_data *data = dev_get_drvdata(dev);

	mutex_lock(&data->update_lock);

	for (i = 0; i < ARRAY_SIZE(regs); i++) {
		status = as7515_sfp_read_value(regs[i]);
		if (status < 0)
			goto exit;

		values[i] = (u8)status;
	}

	mutex_unlock(&data->update_lock);

	/* Return values in order */
	return sprintf(buf, "%.2x %.2x %.2x\n",
	(unsigned int)((values[0] & 0x0F) << 4),
	(unsigned int)(((values[0] & 0xF0) >> 4) | ((values[1] & 0xF) << 4)),
	(unsigned int)(((values[1] & 0xF0) >> 4) | ((values[2] & 0xF) << 4)));

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t set_module_reset_all(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	struct as7515_sfp_data *data = dev_get_drvdata(dev);
	long value;
	int status;
	u8 reg = 0x08;

	status = kstrtol(buf, 10, &value);
	if (status)
		return status;

	/* Read current status */
	mutex_lock(&data->update_lock);

	/* Update status */
	if (value)
		value = 0;
	else
		value = 0x0F;

	/* Set value to CPLD */
	status = as7515_sfp_write_value(reg, value);
	if (unlikely(status < 0))
		goto exit;

	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t show_module(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct as7515_sfp_data *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	int status = 0;
	u8 reg = 0, mask = 0, invert = 0;

	switch (attr->index) {
	case MODULE_PRESENT_1 ... MODULE_RX_LOS_24:
		invert = attribute_mappings[attr->index].invert;
		reg = attribute_mappings[attr->index].reg;
		mask = BIT(attr->index - attribute_mappings[attr->index].attr_base);
		break;
	default:
		return 0;
	}

	mutex_lock(&data->update_lock);
	status = as7515_sfp_read_value(reg);
	if (unlikely(status < 0))
		goto exit;
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n", invert ? !(status & mask) : !!(status & mask));

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t set_control(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	struct as7515_sfp_data *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	long value;
	int status;
	u8 reg = 0, mask = 0, invert = 0;

	status = kstrtol(buf, 10, &value);
	if (status)
		return status;

	switch (attr->index) {
	case MODULE_LPMODE_1 ... MODULE_LPMODE_4:
	case MODULE_RESET_1 ... MODULE_RESET_4:
	case MODULE_TX_DISABLE_5 ... MODULE_TX_DISABLE_24:
		invert = attribute_mappings[attr->index].invert;
		reg = attribute_mappings[attr->index].reg;
		mask = BIT(attr->index - attribute_mappings[attr->index].attr_base);
		break;
	default:
		return 0;
	}

	/* Read current status */
	mutex_lock(&data->update_lock);
	status = as7515_sfp_read_value(reg);
	if (unlikely(status < 0))
		goto exit;

	/* Update status */
	value = invert ? !value : !!value;

	if (value)
		value = (status | mask);
	else
		value = (status & ~mask);

	/* Set value to CPLD */
	status = as7515_sfp_write_value(reg, value);
	if (unlikely(status < 0))
		goto exit;

	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static int as7515_sfp_probe(struct platform_device *pdev)
{
	int status = -1;
	int i = 0;
	struct as7515_sfp_data *data;

	data = kzalloc(sizeof(struct as7515_sfp_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	mutex_init(&data->update_lock);
	platform_set_drvdata(pdev, data);

	/* Register sysfs hooks */
	for (i = 0; i < ARRAY_SIZE(sfp_group); i++) {
		/* Register sysfs hooks */
		status = sysfs_create_group(&pdev->dev.kobj, &sfp_group[i]);
		if (status)
			goto exit_sysfs_group;
	}

	dev_info(&pdev->dev, "device created\n");
	return 0;

exit_sysfs_group:
	for (--i; i >= 0; i--) {
		sysfs_remove_group(&pdev->dev.kobj, &sfp_group[i]);
	}

	kfree(data);
	return status;
}


static int as7515_sfp_remove(struct platform_device *pdev)
{
	struct as7515_sfp_data *data = platform_get_drvdata(pdev);
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(sfp_group); i++) {
		sysfs_remove_group(&pdev->dev.kobj, &sfp_group[i]);
	}

	kfree(data);
	return 0;
}

static const struct platform_device_id as7515_sfp_id[] = {
	{ DRVNAME, 0 },
	{}
};
MODULE_DEVICE_TABLE(platform, as7515_sfp_id);

static struct platform_driver as7515_sfp_driver = {
	.probe	  = as7515_sfp_probe,
	.remove	 = as7515_sfp_remove,
	.id_table	= as7515_sfp_id,
	.driver	 = {
		.name   = DRVNAME,
		.owner  = THIS_MODULE,
	},
};

module_platform_driver(as7515_sfp_driver);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as7515_sfp driver");
MODULE_LICENSE("GPL");
