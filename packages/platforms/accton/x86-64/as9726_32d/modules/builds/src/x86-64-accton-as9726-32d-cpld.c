/*
 * Copyright (C) Alex Lai <alex_lai@edge-core.com>
 *
 * This module supports the accton cpld that hold the channel select
 * mechanism for other i2c slave devices, such as SFP.
 * This includes the:
 *	 Accton as9726_32d CPLD1/CPLD2/CPLD3
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

#define I2C_RW_RETRY_COUNT			10
#define I2C_RW_RETRY_INTERVAL			60 /* ms */

static LIST_HEAD(cpld_client_list);
static struct mutex     list_lock;

struct cpld_client_node {
	struct i2c_client *client;
	struct list_head   list;
};

enum cpld_type {
	as9726_32d_fpga,
	as9726_32d_cpld1,
	as9726_32d_cpld2,
	as9726_32d_cpld_cpu
};

struct as9726_32d_cpld_data {
	enum cpld_type   type;
	struct device   *hwmon_dev;
	struct mutex     update_lock;
};

static const struct i2c_device_id as9726_32d_cpld_id[] = {
	{ "as9726_32d_fpga", as9726_32d_fpga },
	{ "as9726_32d_cpld1", as9726_32d_cpld1 },
	{ "as9726_32d_cpld2", as9726_32d_cpld2 },
	{ "as9726_32d_cpld_cpu", as9726_32d_cpld_cpu },
	{ }
};
MODULE_DEVICE_TABLE(i2c, as9726_32d_cpld_id);

#define TRANSCEIVER_PRESENT_ATTR_ID(index)   	MODULE_PRESENT_##index
#define TRANSCEIVER_TXDISABLE_ATTR_ID(index)   	MODULE_TXDISABLE_##index
#define TRANSCEIVER_RXLOS_ATTR_ID(index)   	MODULE_RXLOS_##index
#define TRANSCEIVER_TXFAULT_ATTR_ID(index)   	MODULE_TXFAULT_##index
#define TRANSCEIVER_RESET_ATTR_ID(index)        MODULE_RESET_##index
#define TRANSCEIVER_LPMODE_ATTR_ID(index)       MODULE_LPMODE_##index

enum as9726_32d_cpld_sysfs_attributes {
	CPLD_VERSION,
	ACCESS,
	MODULE_PRESENT_ALL,
	MODULE_RXLOS_ALL,
	/* transceiver attributes */
	TRANSCEIVER_PRESENT_ATTR_ID(1),
	TRANSCEIVER_PRESENT_ATTR_ID(2),
	TRANSCEIVER_PRESENT_ATTR_ID(3),
	TRANSCEIVER_PRESENT_ATTR_ID(4),
	TRANSCEIVER_PRESENT_ATTR_ID(5),
	TRANSCEIVER_PRESENT_ATTR_ID(6),
	TRANSCEIVER_PRESENT_ATTR_ID(7),
	TRANSCEIVER_PRESENT_ATTR_ID(8),
	TRANSCEIVER_PRESENT_ATTR_ID(9),
	TRANSCEIVER_PRESENT_ATTR_ID(10),
	TRANSCEIVER_PRESENT_ATTR_ID(11),
	TRANSCEIVER_PRESENT_ATTR_ID(12),
	TRANSCEIVER_PRESENT_ATTR_ID(13),
	TRANSCEIVER_PRESENT_ATTR_ID(14),
	TRANSCEIVER_PRESENT_ATTR_ID(15),
	TRANSCEIVER_PRESENT_ATTR_ID(16),
	TRANSCEIVER_PRESENT_ATTR_ID(17),
	TRANSCEIVER_PRESENT_ATTR_ID(18),
	TRANSCEIVER_PRESENT_ATTR_ID(19),
	TRANSCEIVER_PRESENT_ATTR_ID(20),
	TRANSCEIVER_PRESENT_ATTR_ID(21),
	TRANSCEIVER_PRESENT_ATTR_ID(22),
	TRANSCEIVER_PRESENT_ATTR_ID(23),
	TRANSCEIVER_PRESENT_ATTR_ID(24),
	TRANSCEIVER_PRESENT_ATTR_ID(25),
	TRANSCEIVER_PRESENT_ATTR_ID(26),
	TRANSCEIVER_PRESENT_ATTR_ID(27),
	TRANSCEIVER_PRESENT_ATTR_ID(28),
	TRANSCEIVER_PRESENT_ATTR_ID(29),
	TRANSCEIVER_PRESENT_ATTR_ID(30),
	TRANSCEIVER_PRESENT_ATTR_ID(31),
	TRANSCEIVER_PRESENT_ATTR_ID(32),
	TRANSCEIVER_PRESENT_ATTR_ID(33),
	TRANSCEIVER_PRESENT_ATTR_ID(34),
	TRANSCEIVER_TXDISABLE_ATTR_ID(33),
	TRANSCEIVER_TXDISABLE_ATTR_ID(34),
	TRANSCEIVER_RXLOS_ATTR_ID(33),
	TRANSCEIVER_RXLOS_ATTR_ID(34),
	TRANSCEIVER_TXFAULT_ATTR_ID(33),
	TRANSCEIVER_TXFAULT_ATTR_ID(34),
	TRANSCEIVER_RESET_ATTR_ID(1),
	TRANSCEIVER_RESET_ATTR_ID(2),
	TRANSCEIVER_RESET_ATTR_ID(3),
	TRANSCEIVER_RESET_ATTR_ID(4),
	TRANSCEIVER_RESET_ATTR_ID(5),
	TRANSCEIVER_RESET_ATTR_ID(6),
	TRANSCEIVER_RESET_ATTR_ID(7),
	TRANSCEIVER_RESET_ATTR_ID(8),
	TRANSCEIVER_RESET_ATTR_ID(9),
	TRANSCEIVER_RESET_ATTR_ID(10),
	TRANSCEIVER_RESET_ATTR_ID(11),
	TRANSCEIVER_RESET_ATTR_ID(12),
	TRANSCEIVER_RESET_ATTR_ID(13),
	TRANSCEIVER_RESET_ATTR_ID(14),
	TRANSCEIVER_RESET_ATTR_ID(15),
	TRANSCEIVER_RESET_ATTR_ID(16),
	TRANSCEIVER_RESET_ATTR_ID(17),
	TRANSCEIVER_RESET_ATTR_ID(18),
	TRANSCEIVER_RESET_ATTR_ID(19),
	TRANSCEIVER_RESET_ATTR_ID(20),
	TRANSCEIVER_RESET_ATTR_ID(21),
	TRANSCEIVER_RESET_ATTR_ID(22),
	TRANSCEIVER_RESET_ATTR_ID(23),
	TRANSCEIVER_RESET_ATTR_ID(24),
	TRANSCEIVER_RESET_ATTR_ID(25),
	TRANSCEIVER_RESET_ATTR_ID(26),
	TRANSCEIVER_RESET_ATTR_ID(27),
	TRANSCEIVER_RESET_ATTR_ID(28),
	TRANSCEIVER_RESET_ATTR_ID(29),
	TRANSCEIVER_RESET_ATTR_ID(30),
	TRANSCEIVER_RESET_ATTR_ID(31),
	TRANSCEIVER_RESET_ATTR_ID(32),
	TRANSCEIVER_LPMODE_ATTR_ID(1),
	TRANSCEIVER_LPMODE_ATTR_ID(2),
	TRANSCEIVER_LPMODE_ATTR_ID(3),
	TRANSCEIVER_LPMODE_ATTR_ID(4),
	TRANSCEIVER_LPMODE_ATTR_ID(5),
	TRANSCEIVER_LPMODE_ATTR_ID(6),
	TRANSCEIVER_LPMODE_ATTR_ID(7),
	TRANSCEIVER_LPMODE_ATTR_ID(8),
	TRANSCEIVER_LPMODE_ATTR_ID(9),
	TRANSCEIVER_LPMODE_ATTR_ID(10),
	TRANSCEIVER_LPMODE_ATTR_ID(11),
	TRANSCEIVER_LPMODE_ATTR_ID(12),
	TRANSCEIVER_LPMODE_ATTR_ID(13),
	TRANSCEIVER_LPMODE_ATTR_ID(14),
	TRANSCEIVER_LPMODE_ATTR_ID(15),
	TRANSCEIVER_LPMODE_ATTR_ID(16),
	TRANSCEIVER_LPMODE_ATTR_ID(17),
	TRANSCEIVER_LPMODE_ATTR_ID(18),
	TRANSCEIVER_LPMODE_ATTR_ID(19),
	TRANSCEIVER_LPMODE_ATTR_ID(20),
	TRANSCEIVER_LPMODE_ATTR_ID(21),
	TRANSCEIVER_LPMODE_ATTR_ID(22),
	TRANSCEIVER_LPMODE_ATTR_ID(23),
	TRANSCEIVER_LPMODE_ATTR_ID(24),
	TRANSCEIVER_LPMODE_ATTR_ID(25),
	TRANSCEIVER_LPMODE_ATTR_ID(26),
	TRANSCEIVER_LPMODE_ATTR_ID(27),
	TRANSCEIVER_LPMODE_ATTR_ID(28),
	TRANSCEIVER_LPMODE_ATTR_ID(29),
	TRANSCEIVER_LPMODE_ATTR_ID(30),
	TRANSCEIVER_LPMODE_ATTR_ID(31),
	TRANSCEIVER_LPMODE_ATTR_ID(32),
};

/* sysfs attributes for hwmon 
 */
static ssize_t show_status(struct device *dev, struct device_attribute *da,
			char *buf);
static ssize_t show_present_all(struct device *dev, 
				struct device_attribute *da, char *buf);
static ssize_t show_rxlos_all(struct device *dev, struct device_attribute *da,
                              char *buf);
static ssize_t set_tx_disable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t set_port_reset(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t set_lp_mode(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t access(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t show_version(struct device *dev, struct device_attribute *da,
			char *buf);
static int as9726_32d_cpld_read_internal(struct i2c_client *client, u8 reg);
static int as9726_32d_cpld_write_internal(struct i2c_client *client, u8 reg,
					u8 value);

/* transceiver attributes */
#define DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, \
				show_status, NULL, MODULE_PRESENT_##index); \
	static SENSOR_DEVICE_ATTR(module_reset_##index, S_IRUGO | S_IWUSR, \
				show_status, set_port_reset, \
				MODULE_RESET_##index); \
	static SENSOR_DEVICE_ATTR(module_lpmode_##index, S_IRUGO | S_IWUSR, \
				show_status, set_lp_mode, \
				MODULE_LPMODE_##index);
#define DECLARE_TRANSCEIVER_PRESENT_ATTR(index) \
	&sensor_dev_attr_module_present_##index.dev_attr.attr, \
	&sensor_dev_attr_module_reset_##index.dev_attr.attr, \
	&sensor_dev_attr_module_lpmode_##index.dev_attr.attr

#define DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, \
				show_status, NULL, MODULE_PRESENT_##index); \
	static SENSOR_DEVICE_ATTR(module_tx_disable_##index, \
				S_IRUGO | S_IWUSR, show_status, \
				set_tx_disable, MODULE_TXDISABLE_##index); \
	static SENSOR_DEVICE_ATTR(module_rx_los_##index, S_IRUGO, \
				show_status, NULL, MODULE_RXLOS_##index);  \
	static SENSOR_DEVICE_ATTR(module_tx_fault_##index, S_IRUGO, \
				show_status, NULL, MODULE_TXFAULT_##index); 
	
#define DECLARE_SFP_TRANSCEIVER_ATTR(index)  \
	&sensor_dev_attr_module_present_##index.dev_attr.attr, \
	&sensor_dev_attr_module_tx_disable_##index.dev_attr.attr, \
	&sensor_dev_attr_module_rx_los_##index.dev_attr.attr,     \
	&sensor_dev_attr_module_tx_fault_##index.dev_attr.attr
	

static SENSOR_DEVICE_ATTR(version, S_IRUGO, show_version, NULL, CPLD_VERSION);
static SENSOR_DEVICE_ATTR(access, S_IWUSR, NULL, access, ACCESS);

static SENSOR_DEVICE_ATTR(module_present_all, S_IRUGO, show_present_all, \
			  NULL, MODULE_PRESENT_ALL);
static SENSOR_DEVICE_ATTR(module_rx_los_all, S_IRUGO, show_rxlos_all, NULL, \
                          MODULE_RXLOS_ALL);

/* transceiver attributes */
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(1);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(2);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(3);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(4);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(5);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(6);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(7);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(8);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(9);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(10);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(11);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(12);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(13);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(14);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(15);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(16);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(17);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(18);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(19);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(20);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(21);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(22);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(23);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(24);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(25);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(26);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(27);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(28);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(29);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(30);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(31);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(32);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(33);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(34);


static struct attribute *as9726_32d_fpga_attributes[] = {
	&sensor_dev_attr_version.dev_attr.attr,
	&sensor_dev_attr_access.dev_attr.attr,
	NULL
};

static const struct attribute_group as9726_32d_fpga_group = {
	.attrs = as9726_32d_fpga_attributes,
};

static struct attribute *as9726_32d_cpld1_attributes[] = {
	&sensor_dev_attr_version.dev_attr.attr,
	&sensor_dev_attr_access.dev_attr.attr,
	&sensor_dev_attr_module_present_all.dev_attr.attr,
	DECLARE_TRANSCEIVER_PRESENT_ATTR(1),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(2),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(3),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(4),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(5),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(6),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(7),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(8),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(9),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(10),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(11),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(12),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(13),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(14),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(15),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(16),
	NULL
};

static const struct attribute_group as9726_32d_cpld1_group = {
	.attrs = as9726_32d_cpld1_attributes,
};

static struct attribute *as9726_32d_cpld2_attributes[] = {
	&sensor_dev_attr_version.dev_attr.attr,
	&sensor_dev_attr_access.dev_attr.attr,
	&sensor_dev_attr_module_present_all.dev_attr.attr,
	&sensor_dev_attr_module_rx_los_all.dev_attr.attr,	
	DECLARE_TRANSCEIVER_PRESENT_ATTR(17),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(18),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(19),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(20),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(21),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(22),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(23),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(24),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(25),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(26),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(27),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(28),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(29),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(30),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(31),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(32),
	DECLARE_SFP_TRANSCEIVER_ATTR(33),
	DECLARE_SFP_TRANSCEIVER_ATTR(34),
	NULL
};

static const struct attribute_group as9726_32d_cpld2_group = {
	.attrs = as9726_32d_cpld2_attributes,
};

static ssize_t show_present_all(struct device *dev, 
				struct device_attribute *da, char *buf)
{
	int i, status;
	
	struct i2c_client *client = to_i2c_client(dev);
	struct as9726_32d_cpld_data *data = i2c_get_clientdata(client);

	
	if (data->type == as9726_32d_cpld1) {
		u8 regs[] = {0x12, 0x13};
		u8 values[2] = {0};

		mutex_lock(&data->update_lock);
		for (i = 0; i < ARRAY_SIZE(regs); i++) {
			status = as9726_32d_cpld_read_internal(client, regs[i]);
	
			if (status < 0)
				goto exit;

			values[i] = ~(u8)status;
		}
		mutex_unlock(&data->update_lock);

		return sprintf(buf, "%.2x %.2x\n", values[0], values[1]);
	} else if (data->type == as9726_32d_cpld2) {
		u8 regs[] = {0x12, 0x13, 0x20};
		u8 values[3] = {0};

		mutex_lock(&data->update_lock);
		for (i = 0; i < ARRAY_SIZE(regs); i++) {
			status = as9726_32d_cpld_read_internal(client, regs[i]);
	
			if (status < 0)
				goto exit;

			values[i] = ~(u8)status;
		}
		mutex_unlock(&data->update_lock);

		values[2] &= 0x3;
		return sprintf(buf, "%.2x %.2x %.2x\n", values[0], values[1],
				values[2]);
	}

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t show_rxlos_all(struct device *dev, struct device_attribute *da,
			      char *buf)
{
	int i, status;
	u8 value = 0;
	u8 reg = 0x26;
	struct i2c_client *client = to_i2c_client(dev);
	struct as9726_32d_cpld_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);

	status = as9726_32d_cpld_read_internal(client, reg);
	
	if (status < 0)
		goto exit;

	value = ~(u8)status;
	value &= 0x03;
	mutex_unlock(&data->update_lock);

	/* Return values 1 -> 34 in order */
	return sprintf(buf, "00 00 00 00 %.2x\n", value);

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t show_status(struct device *dev, struct device_attribute *da,
	     char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct as9726_32d_cpld_data *data = i2c_get_clientdata(client);
	int status = 0;
	u8 reg = 0, mask = 0, revert = 0;

	switch (attr->index) {
	case MODULE_PRESENT_1 ... MODULE_PRESENT_8:
		reg = 0x12;
		mask = 0x1 << (attr->index - MODULE_PRESENT_1);
		break;
	case MODULE_PRESENT_9 ... MODULE_PRESENT_16:
		reg = 0x13;
		mask = 0x1 << (attr->index - MODULE_PRESENT_9);
		break;
	case MODULE_PRESENT_17 ... MODULE_PRESENT_24:
		reg = 0x12;
		mask = 0x1 << (attr->index - MODULE_PRESENT_17);
		break;
	case MODULE_PRESENT_25 ... MODULE_PRESENT_32:
		reg = 0x13;
		mask = 0x1 << (attr->index - MODULE_PRESENT_25);
		break;
	case MODULE_PRESENT_33:
		reg = 0x20;
		mask = 0x1;
		break;
	case MODULE_PRESENT_34:
		reg = 0x20;
		mask = 0x2;
		break;
	case MODULE_LPMODE_1 ... MODULE_LPMODE_8:
		reg = 0x60;
		mask = 0x1 << (attr->index - MODULE_LPMODE_1);
		break;
	case MODULE_LPMODE_9 ... MODULE_LPMODE_16:
		reg = 0x61;
		mask = 0x1 << (attr->index - MODULE_LPMODE_9);
		break;
	case MODULE_LPMODE_17 ... MODULE_LPMODE_24:
		reg = 0x60;
		mask = 0x1 << (attr->index - MODULE_LPMODE_17);
		break;
	case MODULE_LPMODE_25 ... MODULE_LPMODE_32:
		reg = 0x61;
		mask = 0x1 << (attr->index - MODULE_LPMODE_25);
		break;
	case MODULE_RXLOS_33:
		reg = 0x26;
		mask = 0x1;
		break;
	case MODULE_RXLOS_34:
		reg = 0x26;
		mask = 0x2;
		break;    
	case MODULE_TXDISABLE_33:
		reg = 0x21;
		mask = 0x1;
		break;
	case MODULE_TXDISABLE_34:
		reg = 0x21;
		mask = 0x2;
		break;
	case MODULE_TXFAULT_33:
		reg = 0x27;
		mask = 0x1;
	case MODULE_TXFAULT_34:
		reg = 0x27;
		mask = 0x2;
	case MODULE_RESET_1 ... MODULE_RESET_8:
		reg = 0x14;
		mask = 0x1 << (attr->index - MODULE_RESET_1);
		break;
	case MODULE_RESET_9 ... MODULE_RESET_16:
		reg = 0x15;
		mask = 0x1 << (attr->index - MODULE_RESET_9);
		break;
	case MODULE_RESET_17 ... MODULE_RESET_24:
		reg = 0x14;
		mask = 0x1 << (attr->index - MODULE_RESET_17);
		break;
	case MODULE_RESET_25 ... MODULE_RESET_32:
		reg = 0x15;
		mask = 0x1 << (attr->index - MODULE_RESET_25);
		break;
	default:
		return 0;
	}

	if (attr->index >= MODULE_PRESENT_1 && 
	    attr->index <= MODULE_PRESENT_34)
		revert = 1;

	if (attr->index >= MODULE_RESET_1 && attr->index <= MODULE_RESET_32)
		revert = 1;

	if (attr->index >= MODULE_RXLOS_33 && attr->index <= MODULE_RXLOS_34)
		revert = 1;

	if (attr->index >= MODULE_TXFAULT_33 && attr->index <= MODULE_TXFAULT_34)
		revert = 1;

	mutex_lock(&data->update_lock);
	status = as9726_32d_cpld_read_internal(client, reg);
	if (unlikely(status < 0))
		goto exit;

	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n",
			revert ? !(status & mask) : !!(status & mask));

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t set_lp_mode(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct as9726_32d_cpld_data *data = i2c_get_clientdata(client);

	long value;
	int status;
    	u8 reg = 0, mask = 0;

	status = kstrtol(buf, 10, &value);
	if (status) {
		return status;
	}

	switch (attr->index) {
	case MODULE_LPMODE_1 ... MODULE_LPMODE_8:
		reg  = 0x60;
		mask = 0x1 << (attr->index - MODULE_LPMODE_1);
		break;
	case MODULE_LPMODE_9 ... MODULE_LPMODE_16:
		reg = 0x61;
		mask = 0x1 << (attr->index - MODULE_LPMODE_9);
		break;
	case MODULE_LPMODE_17 ... MODULE_LPMODE_24:
		reg = 0x60;
		mask = 0x1 << (attr->index - MODULE_LPMODE_17);
		break;
	case MODULE_LPMODE_25 ... MODULE_LPMODE_32:
		reg = 0x61;
		mask = 0x1 << (attr->index - MODULE_LPMODE_25);
		break;
	default:
		return 0;
	}

	/* Read current status */
	mutex_lock(&data->update_lock);
	status = as9726_32d_cpld_read_internal(client, reg);
	if (unlikely(status < 0))
		goto exit;

	if (value)
		status |= mask;
	else
		status &= ~mask;

	status = as9726_32d_cpld_write_internal(client, reg, status);
	if (unlikely(status < 0))
		goto exit;
    
	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t set_tx_disable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct as9726_32d_cpld_data *data = i2c_get_clientdata(client);
	long disable;
	int status;
	u8 reg = 0, mask = 0;

	status = kstrtol(buf, 10, &disable);
	if (status)
		return status;

	switch (attr->index) {
	case MODULE_TXDISABLE_33:
		reg = 0x21;
		mask = 0x1;
		break;
	case MODULE_TXDISABLE_34:
		reg = 0x21;
		mask = 0x2;
		break;
	default:
		return 0;
	}

	/* Read current status */
	mutex_lock(&data->update_lock);
	status = as9726_32d_cpld_read_internal(client, reg);
	if (unlikely(status < 0))
		goto exit;

	/* Update tx_disable status */
	if (disable) {
		status |= mask;
	} else {
		status &= ~mask;
	}

	status = as9726_32d_cpld_write_internal(client, reg, status);
	if (unlikely(status < 0))
		goto exit;

	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t set_port_reset(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct as9726_32d_cpld_data *data = i2c_get_clientdata(client);
	long reset;
	int status;
	u8 reg = 0, mask = 0;

	status = kstrtol(buf, 10, &reset);
	if (status)
		return status;

	switch (attr->index) {
	case MODULE_RESET_1 ... MODULE_RESET_8:
		reg = 0x14;
		mask = 0x1 << (attr->index - MODULE_RESET_1);
		break;
	case MODULE_RESET_9 ... MODULE_RESET_16:
		reg = 0x15;
		mask = 0x1 << (attr->index - MODULE_RESET_9);
		break;
	case MODULE_RESET_17 ... MODULE_RESET_24:
		reg = 0x14;
		mask = 0x1 << (attr->index - MODULE_RESET_17);
		break;
	case MODULE_RESET_25 ... MODULE_RESET_32:
		reg = 0x15;
		mask = 0x1 << (attr->index - MODULE_RESET_25);
		break;
	default:
		return -ENXIO;
	}

	/* Read current status */
	mutex_lock(&data->update_lock);
	status = as9726_32d_cpld_read_internal(client, reg);
	if (unlikely(status < 0))
		goto exit;

	/* Update tx_disable status */
	if (reset) {
		status &= ~mask;
	} else {
		status |= mask;
	}

	status = as9726_32d_cpld_write_internal(client, reg, status);
	if (unlikely(status < 0))
		goto exit;

	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}


static ssize_t access(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	int status;
	u32 addr, val;
	struct i2c_client *client = to_i2c_client(dev);
	struct as9726_32d_cpld_data *data = i2c_get_clientdata(client);

	if (sscanf(buf, "0x%x 0x%x", &addr, &val) != 2)
		return -EINVAL;

	if (addr > 0xFF || val > 0xFF)
		return -EINVAL;

	mutex_lock(&data->update_lock);
	status = as9726_32d_cpld_write_internal(client, addr, val);
	if (unlikely(status < 0))
		goto exit;

	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static void as9726_32d_cpld_add_client(struct i2c_client *client)
{
	struct cpld_client_node *node = kzalloc(sizeof(struct cpld_client_node)
					, GFP_KERNEL);

	if (!node) {
		dev_dbg(&client->dev, "Can't allocate cpld_client_node (0x%x)\n",
			client->addr);
		return;
	}

	node->client = client;

	mutex_lock(&list_lock);
	list_add(&node->list, &cpld_client_list);
	mutex_unlock(&list_lock);
}

static void as9726_32d_cpld_remove_client(struct i2c_client *client)
{
	struct list_head    *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int found = 0;

	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list)
	{
		cpld_node = list_entry(list_node, struct cpld_client_node,
					list);

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

static ssize_t show_version(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	int val = 0;
	struct i2c_client *client = to_i2c_client(dev);
	
	val = i2c_smbus_read_byte_data(client, 0x1);

	if (val < 0)
		dev_dbg(&client->dev, "cpld(0x%x) reg(0x1) err %d\n",
			client->addr, val);
	
	return sprintf(buf, "%d\n", val);
}

/*
 * I2C init/probing/exit functions
 */
static int as9726_32d_cpld_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adap = to_i2c_adapter(client->dev.parent);
	struct as9726_32d_cpld_data *data;
	int ret = -ENODEV;
	int status;	
	const struct attribute_group *group = NULL;

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE))
		goto exit;

	data = kzalloc(sizeof(struct as9726_32d_cpld_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);
	data->type = id->driver_data;

	/* Register sysfs hooks */
	switch (data->type) {
	case as9726_32d_fpga:
		group = &as9726_32d_fpga_group;
		break;
	case as9726_32d_cpld1:
		group = &as9726_32d_cpld1_group;
		break;
	case as9726_32d_cpld2:
		group = &as9726_32d_cpld2_group;
		break; 
	case as9726_32d_cpld_cpu:
	/* Disable CPLD reset to avoid DUT will be reset.
	 */
	status=as9726_32d_cpld_write_internal(client, 0x3, 0x0); 
	if (status < 0)
		dev_dbg(&client->dev, "cpu_cpld reg 0x65 err %d\n", status);

	default:
		break;
	}

	if (group) {
		ret = sysfs_create_group(&client->dev.kobj, group);
		if (ret)
			goto exit_free;
	}

	as9726_32d_cpld_add_client(client);
	return 0;

exit_free:
	kfree(data);
exit:
	return ret;
}

static int as9726_32d_cpld_remove(struct i2c_client *client)
{
	struct as9726_32d_cpld_data *data = i2c_get_clientdata(client);
	const struct attribute_group *group = NULL;

	as9726_32d_cpld_remove_client(client);

	/* Remove sysfs hooks */
	switch (data->type) {
	case as9726_32d_fpga:
		group = &as9726_32d_fpga_group;
		break;
	case as9726_32d_cpld1:
		group = &as9726_32d_cpld1_group;
		break;
	case as9726_32d_cpld2:
		group = &as9726_32d_cpld2_group;
		break;
	default:
		break;
	}

	if (group)
		sysfs_remove_group(&client->dev.kobj, group);

	kfree(data);

	return 0;
}

static int as9726_32d_cpld_read_internal(struct i2c_client *client, u8 reg)
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

static int as9726_32d_cpld_write_internal(struct i2c_client *client, u8 reg,
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

int as9726_32d_cpld_read(unsigned short cpld_addr, u8 reg)
{
	struct list_head   *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int ret = -EPERM;

	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list)
	{
		cpld_node = list_entry(list_node, struct cpld_client_node,
					list);

		if (cpld_node->client->addr == cpld_addr) {
			ret = as9726_32d_cpld_read_internal(cpld_node->client,
								reg);
			break;
		}
	}

	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(as9726_32d_cpld_read);

int as9726_32d_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
{
	struct list_head   *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int ret = -EIO;

	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list)
	{
		cpld_node = list_entry(list_node, struct cpld_client_node,
					list);
		if (cpld_node->client->addr == cpld_addr) {
			ret = as9726_32d_cpld_write_internal(cpld_node->client,
								reg, value);
			break;
		}
	}

	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(as9726_32d_cpld_write);

static struct i2c_driver as9726_32d_cpld_driver = {
	.driver		= {
		.name	= "as9726_32d_cpld",
		.owner	= THIS_MODULE,
	},
	.probe		= as9726_32d_cpld_probe,
	.remove		= as9726_32d_cpld_remove,
	.id_table	= as9726_32d_cpld_id,
};

static int __init as9726_32d_cpld_init(void)
{
	mutex_init(&list_lock);
	return i2c_add_driver(&as9726_32d_cpld_driver);
}

static void __exit as9726_32d_cpld_exit(void)
{
	i2c_del_driver(&as9726_32d_cpld_driver);
}

MODULE_AUTHOR("Alex Lai <alex_lai@edge-core.com>");
MODULE_DESCRIPTION("Accton I2C CPLD driver");
MODULE_LICENSE("GPL");

module_init(as9726_32d_cpld_init);
module_exit(as9726_32d_cpld_exit);
