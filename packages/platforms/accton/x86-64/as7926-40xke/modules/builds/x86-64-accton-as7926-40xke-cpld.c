/*
 * A hwmon driver for the as7926_40xke_cpld
 *
 * Copyright (C) 2019  Edgecore Networks Corporation.
 * Jostar Yang <jostar_yang@edge-core.com>
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

static LIST_HEAD(cpld_client_list);
static struct mutex	 list_lock;

struct cpld_client_node {
	struct i2c_client *client;
	struct list_head   list;
};

#define I2C_RW_RETRY_COUNT				10
#define I2C_RW_RETRY_INTERVAL			60 /* ms */

static ssize_t show_status(struct device *dev, struct device_attribute *da,
             char *buf);

static ssize_t set_tx_disable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);

struct as7926_40xke_cpld_data {
    struct device      *hwmon_dev;
    struct mutex        update_lock;
    u8  index;          /* CPLD index */
};

/* Addresses scanned for as7926_40xke_cpld
 */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

#define TRANSCEIVER_PRESENT_ATTR_ID(index)   MODULE_PRESENT_##index
#define TRANSCEIVER_TXDISABLE_ATTR_ID(index)   	MODULE_TXDISABLE_##index
#define TRANSCEIVER_RXLOS_ATTR_ID(index)   		MODULE_RXLOS_##index

enum as7926_40xke_cpld1_sysfs_attributes {
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
	TRANSCEIVER_PRESENT_ATTR_ID(35),
	TRANSCEIVER_PRESENT_ATTR_ID(36),
	TRANSCEIVER_PRESENT_ATTR_ID(37),
	TRANSCEIVER_PRESENT_ATTR_ID(38),
	TRANSCEIVER_PRESENT_ATTR_ID(39),
	TRANSCEIVER_PRESENT_ATTR_ID(40),
	TRANSCEIVER_PRESENT_ATTR_ID(41),
	TRANSCEIVER_PRESENT_ATTR_ID(42),
	TRANSCEIVER_TXDISABLE_ATTR_ID(41),
	TRANSCEIVER_TXDISABLE_ATTR_ID(42),
	TRANSCEIVER_RXLOS_ATTR_ID(41),
	TRANSCEIVER_RXLOS_ATTR_ID(42),
	TRANSCEIVER_PRESENT_ATTR_ID(43),
	TRANSCEIVER_PRESENT_ATTR_ID(44),
	TRANSCEIVER_PRESENT_ATTR_ID(45),
	TRANSCEIVER_PRESENT_ATTR_ID(46),
	TRANSCEIVER_PRESENT_ATTR_ID(47),
	TRANSCEIVER_PRESENT_ATTR_ID(48),
	TRANSCEIVER_PRESENT_ATTR_ID(49),
	TRANSCEIVER_PRESENT_ATTR_ID(50),
	TRANSCEIVER_PRESENT_ATTR_ID(51),
	TRANSCEIVER_PRESENT_ATTR_ID(52),
	TRANSCEIVER_PRESENT_ATTR_ID(53),
	TRANSCEIVER_PRESENT_ATTR_ID(54),
	TRANSCEIVER_PRESENT_ATTR_ID(55),
	CPLD_VERSION,
	ACCESS,
};

/* sysfs attributes for hwmon 
 */

/* qsfp transceiver attributes */
#define DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, show_status, NULL, MODULE_PRESENT_##index)
#define DECLARE_TRANSCEIVER_ATTR(index)  &sensor_dev_attr_module_present_##index.dev_attr.attr

/* sfp transceiver attributes */
#define DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(module_tx_disable_##index, S_IRUGO | S_IWUSR, show_status, set_tx_disable, MODULE_TXDISABLE_##index); \
	static SENSOR_DEVICE_ATTR(module_rx_los_##index, S_IRUGO, show_status, NULL, MODULE_RXLOS_##index);

#define DECLARE_SFP_TRANSCEIVER_ATTR(index)  \
	&sensor_dev_attr_module_tx_disable_##index.dev_attr.attr, \
	&sensor_dev_attr_module_rx_los_##index.dev_attr.attr

static SENSOR_DEVICE_ATTR(version, S_IRUGO, show_version, NULL, CPLD_VERSION);
static SENSOR_DEVICE_ATTR(access, S_IWUSR, NULL, access, ACCESS);

/* transceiver attributes */
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(1);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(2);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(3);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(4);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(5);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(6);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(7);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(8);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(9);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(10);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(11);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(12);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(13);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(14);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(15);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(16);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(17);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(18);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(19);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(20);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(21);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(22);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(23);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(24);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(25);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(26);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(27);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(28);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(29);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(30);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(31);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(32);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(33);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(34);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(35);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(36);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(37);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(38);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(39);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(40);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(41);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(42);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(41);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(42);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(43);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(44);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(45);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(46);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(47);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(48);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(49);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(50);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(51);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(52);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(53);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(54);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(55);

static struct attribute *as7926_40xke_cpld2_attributes[] = {
	/* transceiver attributes */
	DECLARE_TRANSCEIVER_ATTR(1),
	DECLARE_TRANSCEIVER_ATTR(2),
	DECLARE_TRANSCEIVER_ATTR(3),
	DECLARE_TRANSCEIVER_ATTR(4),
	DECLARE_TRANSCEIVER_ATTR(5),
	DECLARE_TRANSCEIVER_ATTR(6),
	DECLARE_TRANSCEIVER_ATTR(7),
	DECLARE_TRANSCEIVER_ATTR(8),
	DECLARE_TRANSCEIVER_ATTR(9),
	DECLARE_TRANSCEIVER_ATTR(10),
	DECLARE_TRANSCEIVER_ATTR(11),
	DECLARE_TRANSCEIVER_ATTR(12),
	DECLARE_TRANSCEIVER_ATTR(13),
	DECLARE_TRANSCEIVER_ATTR(14),
	DECLARE_TRANSCEIVER_ATTR(15),
	DECLARE_TRANSCEIVER_ATTR(16),
	DECLARE_TRANSCEIVER_ATTR(17),
	DECLARE_TRANSCEIVER_ATTR(18),
	DECLARE_TRANSCEIVER_ATTR(19),
	DECLARE_TRANSCEIVER_ATTR(20),
	DECLARE_TRANSCEIVER_ATTR(41),
	DECLARE_TRANSCEIVER_ATTR(42),
	DECLARE_SFP_TRANSCEIVER_ATTR(41),
    DECLARE_SFP_TRANSCEIVER_ATTR(42),
    &sensor_dev_attr_version.dev_attr.attr,
    &sensor_dev_attr_access.dev_attr.attr,	
	NULL
};

static struct attribute *as7926_40xke_cpld3_attributes[] = {
    DECLARE_TRANSCEIVER_ATTR(21),
	DECLARE_TRANSCEIVER_ATTR(22),
	DECLARE_TRANSCEIVER_ATTR(23),
	DECLARE_TRANSCEIVER_ATTR(24),
	DECLARE_TRANSCEIVER_ATTR(25),
	DECLARE_TRANSCEIVER_ATTR(26),
	DECLARE_TRANSCEIVER_ATTR(27),
	DECLARE_TRANSCEIVER_ATTR(28),
	DECLARE_TRANSCEIVER_ATTR(29),
	DECLARE_TRANSCEIVER_ATTR(30),
	DECLARE_TRANSCEIVER_ATTR(31),
	DECLARE_TRANSCEIVER_ATTR(32),
	DECLARE_TRANSCEIVER_ATTR(33),
	DECLARE_TRANSCEIVER_ATTR(34),
	DECLARE_TRANSCEIVER_ATTR(35),
	DECLARE_TRANSCEIVER_ATTR(36),
	DECLARE_TRANSCEIVER_ATTR(37),
	DECLARE_TRANSCEIVER_ATTR(38),
	DECLARE_TRANSCEIVER_ATTR(39),
	DECLARE_TRANSCEIVER_ATTR(40),
	&sensor_dev_attr_version.dev_attr.attr,
    &sensor_dev_attr_access.dev_attr.attr,	
    NULL
};

static struct attribute *as7926_40xke_cpld4_attributes[] = {
    DECLARE_TRANSCEIVER_ATTR(43),
	DECLARE_TRANSCEIVER_ATTR(44),
	DECLARE_TRANSCEIVER_ATTR(45),
	DECLARE_TRANSCEIVER_ATTR(46),
	DECLARE_TRANSCEIVER_ATTR(47),
	DECLARE_TRANSCEIVER_ATTR(48),
	DECLARE_TRANSCEIVER_ATTR(49),
	DECLARE_TRANSCEIVER_ATTR(50),
	DECLARE_TRANSCEIVER_ATTR(51),
	DECLARE_TRANSCEIVER_ATTR(52),
	DECLARE_TRANSCEIVER_ATTR(53),
	DECLARE_TRANSCEIVER_ATTR(54),
	DECLARE_TRANSCEIVER_ATTR(55),
	&sensor_dev_attr_version.dev_attr.attr,
    &sensor_dev_attr_access.dev_attr.attr,	
    NULL
};

static const struct attribute_group as7926_40xke_cpld2_group = {
	.attrs = as7926_40xke_cpld2_attributes,
};

static const struct attribute_group as7926_40xke_cpld3_group = {
	.attrs = as7926_40xke_cpld3_attributes,
};


static const struct attribute_group as7926_40xke_cpld4_group = {
	.attrs = as7926_40xke_cpld4_attributes,
};

int as7926_40xke_cpld_read(int bus_num, unsigned short cpld_addr, u8 reg)
{
	struct list_head   *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int ret = -EPERM;
	
	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list)
	{
		cpld_node = list_entry(list_node, struct cpld_client_node, list);
		
		if (cpld_node->client->addr == cpld_addr
            && cpld_node->client->adapter->nr == bus_num) {
			ret = i2c_smbus_read_byte_data(cpld_node->client, reg);
			break;
		}
	}
	
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(as7926_40xke_cpld_read);

int as7926_40xke_cpld_write(int bus_num, unsigned short cpld_addr, u8 reg, u8 value)
{
	struct list_head   *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int ret = -EIO;
	
	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list)
	{
		cpld_node = list_entry(list_node, struct cpld_client_node, list);
		
		if (cpld_node->client->addr == cpld_addr
            && cpld_node->client->adapter->nr == bus_num) {
			ret = i2c_smbus_write_byte_data(cpld_node->client, reg, value);
			break;
		}
	}
	
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(as7926_40xke_cpld_write);

static ssize_t show_status(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct as7926_40xke_cpld_data *data = i2c_get_clientdata(client);
	int status = 0;
	u8 reg = 0, mask = 0, revert = 1;
    int qsfp_dd_index=0;

	switch (attr->index) {
	case MODULE_PRESENT_1 ... MODULE_PRESENT_8:
		reg  = 0x10;
		mask = 0x1 << (attr->index - MODULE_PRESENT_1);
		break;
	case MODULE_PRESENT_9 ... MODULE_PRESENT_16:
		reg  = 0x11;
		mask = 0x1 << (attr->index - MODULE_PRESENT_9);
		break;
	case MODULE_PRESENT_17 ... MODULE_PRESENT_20:
		reg  = 0x12;
		mask = 0x1 << (attr->index - MODULE_PRESENT_17);
		break;
	case MODULE_PRESENT_21 ... MODULE_PRESENT_28:
		reg  = 0x10;
		mask = 0x1 << (attr->index - MODULE_PRESENT_21);
		break;
	case MODULE_PRESENT_29 ... MODULE_PRESENT_36:
		reg  = 0x11;
		mask = 0x1 << (attr->index - MODULE_PRESENT_29);
		break;
	case MODULE_PRESENT_37 ... MODULE_PRESENT_40:
		reg  = 0x12;
		mask = 0x1 << (attr->index - MODULE_PRESENT_37);
		break;
	case MODULE_PRESENT_41 ... MODULE_PRESENT_42:
		reg  = 0x13;
		mask = 0x1 << (attr->index - MODULE_PRESENT_41);
		break;
        case MODULE_TXDISABLE_41 ... MODULE_TXDISABLE_42:
                reg  = 0xB;
		mask = 0x1 << (attr->index - MODULE_TXDISABLE_41);
	    break;
	case MODULE_RXLOS_41 ... MODULE_RXLOS_42:
        reg  = 0x23;
		mask = 0x1 << (attr->index - MODULE_RXLOS_41);
		revert=0;
		break;
        case MODULE_PRESENT_43 ... MODULE_PRESENT_50: /*QSFP-DD*/
            reg  = 0x50;
            mask = 0x1 << (attr->index - MODULE_PRESENT_43);
            break;
        case MODULE_PRESENT_51 ... MODULE_PRESENT_55: /*QSFP-DD*/
            reg  = 0x51;
            mask = 0x1 << (attr->index - MODULE_PRESENT_51);
            break;
 	default:
            return -ENXIO;
        }

    mutex_lock(&data->update_lock);
    switch(data->index) {
        /* Port 1-20, 41-42 present statuus: read from i2c bus number '12'
            and CPLD slave address 0x62 */
        case 1: status = as7926_40xke_cpld_read(12,0x62, reg);
                break;
        /* Port 21-40 present statuus: read from i2c bus number '13'
            and CPLD slave address 0x63 */
        case 2: status = as7926_40xke_cpld_read(13,0x63, reg);
                break;
        /* Port 43-55 plug-unplug read from i2c bus number '76'
            and CPLD slave address 0x64 */
        case 3: status = as7926_40xke_cpld_read(76,0x64, reg);
                break;
        default: status = -ENXIO;
                break;
    }
        
	if (unlikely(status < 0)) {
		goto exit;
	}
	
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n", revert? !(status & mask): !!(status & mask));

exit:
	mutex_unlock(&data->update_lock);
	return status;
}
static ssize_t set_tx_disable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct as7926_40xke_cpld_data *data = i2c_get_clientdata(client);
	long disable;
	int status, val;
    u8 reg = 0, mask = 0;
     
	status = kstrtol(buf, 10, &disable);
	if (status) {
		return status;
	}
    reg  = 0xB;    
	switch (attr->index) {
	case MODULE_TXDISABLE_41:	   
		mask = 0x1;
		break;
	case MODULE_TXDISABLE_42:
		mask = 0x2;
		break;
	default:
		return 0;
	}
    /* Read current status */
    mutex_lock(&data->update_lock);
	val = as7926_40xke_cpld_read(12, 0x62, reg);
	if (unlikely(status < 0)) {
		goto exit;
	}

	/* Update tx_disable status */
	if (disable) {
		val &= ~mask;
	}
	else {
		
		val |= mask;
	}

    status = as7926_40xke_cpld_write(12, 0x62, reg, val);
	if (unlikely(status < 0)) {
		goto exit;
	}
    
    mutex_unlock(&data->update_lock);
    return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}
static void as7926_40xke_cpld_add_client(struct i2c_client *client)
{
	struct cpld_client_node *node = kzalloc(sizeof(struct cpld_client_node), GFP_KERNEL);
	
	if (!node) {
		dev_dbg(&client->dev, "Can't allocate cpld_client_node (0x%x)\n", client->addr);
		return;
	}
	
	node->client = client;
	
	mutex_lock(&list_lock);
	list_add(&node->list, &cpld_client_list);
	mutex_unlock(&list_lock);
}

static void as7926_40xke_cpld_remove_client(struct i2c_client *client)
{
	struct list_head		*list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int found = 0;
	
	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list)
	{
		cpld_node = list_entry(list_node, struct cpld_client_node, list);
		
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

static ssize_t access(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	int status;
	u32 reg, val;
    struct i2c_client *client = to_i2c_client(dev);
    struct as7926_40xke_cpld_data *data = i2c_get_clientdata(client);
    
	if (sscanf(buf, "0x%x 0x%x", &reg, &val) != 2) {
		return -EINVAL;
	}
	if (reg > 0xFF || val > 0xFF) {
		return -EINVAL;
	}
	mutex_lock(&data->update_lock);
	switch(data->index) {
        case 1: status = as7926_40xke_cpld_write(12,0x62, reg, val);
            break;
        case 2: status = as7926_40xke_cpld_write(13,0x63, reg, val);
            break;
        case 3: status = as7926_40xke_cpld_write(76,0x64, reg, val);
            break;
        default: status = -ENXIO;
                break;
    }
	
	if (unlikely(status < 0)) {
		goto exit;
	}
	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t show_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct as7926_40xke_cpld_data *data = i2c_get_clientdata(client);
	int status = 0;
	
	mutex_lock(&data->update_lock);
    switch(data->index) {
        case 1: status = as7926_40xke_cpld_read(12,0x62, 0x1);
            break;
        case 2: status = as7926_40xke_cpld_read(13,0x63, 0x1);
            break;
        case 3: status = as7926_40xke_cpld_read(76,0x64, 0x1);
            break;
        default: status = -1;
                break;
    }
        
	if (unlikely(status < 0))
	{
	    mutex_unlock(&data->update_lock);
		goto exit;
	}
	
	mutex_unlock(&data->update_lock);	
	
    return sprintf(buf, "%d\n", val);
exit:
    return status;    
}


static int as7926_40xke_cpld_probe(struct i2c_client *client,
            const struct i2c_device_id *dev_id)
{
    int status;
	struct as7926_40xke_cpld_data *data = NULL;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_dbg(&client->dev, "i2c_check_functionality failed (0x%x)\n", client->addr);
        status = -EIO;
        goto exit;
    }

    data = kzalloc(sizeof(struct as7926_40xke_cpld_data), GFP_KERNEL);
    if (!data) {
        status = -ENOMEM;
        goto exit;
    }

    i2c_set_clientdata(client, data);
    data->index = dev_id->driver_data;
    mutex_init(&data->update_lock);
    dev_info(&client->dev, "chip found\n");
    
	/* Register sysfs hooks */
    switch(data->index) {
        case 1: status = sysfs_create_group(&client->dev.kobj, &as7926_40xke_cpld2_group);
                break;
        case 2: status = sysfs_create_group(&client->dev.kobj, &as7926_40xke_cpld3_group);
                break;
        case 3: status = sysfs_create_group(&client->dev.kobj, &as7926_40xke_cpld4_group);
                break;
        default:status = 1;
                break;
    }
	if (status) {
		goto exit_free;
	}

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

    as7926_40xke_cpld_add_client(client);
    
	dev_info(&client->dev, "%s: cpld '%s'\n",
		 dev_name(data->hwmon_dev), client->name);

    return 0;

exit_remove:
    sysfs_remove_group(&client->dev.kobj, &as7926_40xke_cpld2_group);
    sysfs_remove_group(&client->dev.kobj, &as7926_40xke_cpld3_group);
    sysfs_remove_group(&client->dev.kobj, &as7926_40xke_cpld4_group);
exit_free:
    kfree(data);
exit:
    
    return status;
}

static int as7926_40xke_cpld_remove(struct i2c_client *client)
{
    struct as7926_40xke_cpld_data *data = i2c_get_clientdata(client);

    hwmon_device_unregister(data->hwmon_dev);
    sysfs_remove_group(&client->dev.kobj, &as7926_40xke_cpld2_group);
    sysfs_remove_group(&client->dev.kobj, &as7926_40xke_cpld3_group);
    sysfs_remove_group(&client->dev.kobj, &as7926_40xke_cpld4_group);
    kfree(data);
    as7926_40xke_cpld_remove_client(client);

    return 0;
}

static const struct i2c_device_id as7926_40xke_cpld_id[] = {
    { "as7926_40xke_cpld2", 1 },
    { "as7926_40xke_cpld3", 2 },
    { "as7926_40xke_cpld4", 3 },
    {}
};

MODULE_DEVICE_TABLE(i2c, as7926_40xke_cpld_id);

static struct i2c_driver as7926_40xke_cpld_driver = {
    .class        = I2C_CLASS_HWMON,
    .driver = {
        .name     = "as7926_40xke_cpld1",
    },
    .probe        = as7926_40xke_cpld_probe,
    .remove       = as7926_40xke_cpld_remove,
    .id_table     = as7926_40xke_cpld_id,
    .address_list = normal_i2c,
};

static int __init as7926_40xke_cpld_init(void)
{
	mutex_init(&list_lock);
	return i2c_add_driver(&as7926_40xke_cpld_driver);
}

static void __exit as7926_40xke_cpld_exit(void)
{
	i2c_del_driver(&as7926_40xke_cpld_driver);
}

module_init(as7926_40xke_cpld_init);
module_exit(as7926_40xke_cpld_exit);

MODULE_AUTHOR("Jostar Yang <jostar_yang@edge-core.com>");
MODULE_DESCRIPTION("as7926_40xke_cpld driver");
MODULE_LICENSE("GPL");

