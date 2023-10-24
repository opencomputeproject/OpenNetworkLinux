/*
 * Copyright (C)  Brandon Chuang <brandon_chuang@accton.com.tw>
 *
 * This module supports the accton cpld that hold the channel select
 * mechanism for other i2c slave devices, such as SFP.
 * This includes the:
 *	 Accton as4610_54 CPLD
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

#define I2C_RW_RETRY_COUNT				10
#define I2C_RW_RETRY_INTERVAL			60 /* ms */
#define AS4610_CPLD_SLAVE_ADDR 			0x30
#define AS4610_CPLD_PID_OFFSET 			0x01	 /* Product ID offset */

static LIST_HEAD(cpld_client_list);
static struct mutex     list_lock;

struct cpld_client_node {
    struct i2c_client *client;
    struct list_head   list;
};

enum cpld_type {
    as4610_54_cpld
};

enum as4610_product_id_e {
	PID_AS4610_30T,
	PID_AS4610_30P,
	PID_AS4610_54T,
	PID_AS4610_54P,
	PID_RESERVED,
	PID_AS4610_54T_B,
	PID_UNKNOWN
};

struct as4610_54_cpld_data {
    enum cpld_type   type;
    struct device   *hwmon_dev;
    struct platform_device *fan_pdev;
    struct platform_device *led_pdev;
    struct mutex     update_lock;
};

static const struct i2c_device_id as4610_54_cpld_id[] = {
    { "as4610_54_cpld", as4610_54_cpld },
    { }
};
MODULE_DEVICE_TABLE(i2c, as4610_54_cpld_id);

#define TRANSCEIVER_PRESENT_ATTR_ID(index)   	MODULE_PRESENT_##index
#define TRANSCEIVER_TXDISABLE_ATTR_ID(index)   	MODULE_TXDISABLE_##index
#define TRANSCEIVER_RXLOS_ATTR_ID(index)   		MODULE_RXLOS_##index
#define TRANSCEIVER_TXFAULT_ATTR_ID(index)   	MODULE_TXFAULT_##index

enum as4610_54_cpld1_sysfs_attributes {
	CPLD_VERSION,
    PRODUCT_ID,
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
	TRANSCEIVER_TXDISABLE_ATTR_ID(1),
	TRANSCEIVER_TXDISABLE_ATTR_ID(2),
	TRANSCEIVER_TXDISABLE_ATTR_ID(3),
	TRANSCEIVER_TXDISABLE_ATTR_ID(4),
	TRANSCEIVER_RXLOS_ATTR_ID(1),
	TRANSCEIVER_RXLOS_ATTR_ID(2),
	TRANSCEIVER_RXLOS_ATTR_ID(3),
	TRANSCEIVER_RXLOS_ATTR_ID(4),
	TRANSCEIVER_TXFAULT_ATTR_ID(1),
	TRANSCEIVER_TXFAULT_ATTR_ID(2),
	TRANSCEIVER_TXFAULT_ATTR_ID(3),
	TRANSCEIVER_TXFAULT_ATTR_ID(4),
};

/* sysfs attributes for hwmon 
 */
static ssize_t show_status(struct device *dev, struct device_attribute *da,
             char *buf);
static ssize_t show_present_all(struct device *dev, struct device_attribute *da,
             char *buf);
static ssize_t show_rxlos_all(struct device *dev, struct device_attribute *da,
             char *buf);
static ssize_t access(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t show_version(struct device *dev, struct device_attribute *da,
             char *buf);
static ssize_t show_product_id(struct device *dev, struct device_attribute *attr,
             char *buf);
static int as4610_54_cpld_read_internal(struct i2c_client *client, u8 reg);
static int as4610_54_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value);

/* transceiver attributes */
#define DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, show_status, NULL, MODULE_PRESENT_##index)
#define DECLARE_TRANSCEIVER_PRESENT_ATTR(index)  &sensor_dev_attr_module_present_##index.dev_attr.attr

#define DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(module_rx_los_##index, S_IRUGO, show_status, NULL, MODULE_RXLOS_##index); \
	static SENSOR_DEVICE_ATTR(module_tx_fault_##index, S_IRUGO, show_status, NULL, MODULE_TXFAULT_##index)
#define DECLARE_SFP_TRANSCEIVER_ATTR(index)  \
	&sensor_dev_attr_module_rx_los_##index.dev_attr.attr, \
	&sensor_dev_attr_module_tx_fault_##index.dev_attr.attr

static SENSOR_DEVICE_ATTR(version, S_IRUGO, show_version, NULL, CPLD_VERSION);
static SENSOR_DEVICE_ATTR(access, S_IWUSR, NULL, access, ACCESS);
static SENSOR_DEVICE_ATTR(product_id, S_IRUGO, show_product_id, NULL, PRODUCT_ID);
/* transceiver attributes */
static SENSOR_DEVICE_ATTR(module_present_all, S_IRUGO, show_present_all, NULL, MODULE_PRESENT_ALL);
static SENSOR_DEVICE_ATTR(module_rx_los_all, S_IRUGO, show_rxlos_all, NULL, MODULE_RXLOS_ALL);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(1);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(2);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(3);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(4);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(5);
DECLARE_TRANSCEIVER_PRESENT_SENSOR_DEVICE_ATTR(6);

DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(1);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(2);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(3);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(4);

static struct attribute *as4610_54_cpld_attributes[] = {
    &sensor_dev_attr_version.dev_attr.attr,
    &sensor_dev_attr_access.dev_attr.attr,
	&sensor_dev_attr_product_id.dev_attr.attr,
	/* transceiver attributes */
	&sensor_dev_attr_module_present_all.dev_attr.attr,
	&sensor_dev_attr_module_rx_los_all.dev_attr.attr,
	DECLARE_TRANSCEIVER_PRESENT_ATTR(1),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(2),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(3),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(4),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(5),
	DECLARE_TRANSCEIVER_PRESENT_ATTR(6),
	DECLARE_SFP_TRANSCEIVER_ATTR(1),
	DECLARE_SFP_TRANSCEIVER_ATTR(2),
	DECLARE_SFP_TRANSCEIVER_ATTR(3),
	DECLARE_SFP_TRANSCEIVER_ATTR(4),
	NULL
};

static const struct attribute_group as4610_54_cpld_group = {
	.attrs = as4610_54_cpld_attributes,
};

static ssize_t show_present_all(struct device *dev, struct device_attribute *da,
             char *buf)
{
	int i, status, present = 0;
	u8 regs[] = {0x2, 0x3, 0x21};
	struct i2c_client *client = to_i2c_client(dev);
	struct as4610_54_cpld_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);

    for (i = 0; i < ARRAY_SIZE(regs); i++) {
        status = as4610_54_cpld_read_internal(client, regs[i]);
        
        if (status < 0) {
            goto exit;
        }

		switch (i) {
		case 0:
			present |= (status & BIT(6)) >> 6; /* port 25/49 */
			present |= (status & BIT(2)) >> 1; /* port 26/50 */
			break;
		case 1:
			present |= (status & BIT(6)) >> 4; /* port 27/51 */
			present |= (status & BIT(2)) << 1; /* port 28/52 */
			break;
		case 2:
			present |= (status & BIT(0)) << 4; /* port 29/53 */
			present |= (status & BIT(4)) << 1; /* port 30/54 */
			break;
		default:
			break;
		}
    }

	mutex_unlock(&data->update_lock);

    return sprintf(buf, "%.2x\n", present);

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t show_rxlos_all(struct device *dev, struct device_attribute *da,
             char *buf)
{
	int i, status, rx_los = 0;
	u8 regs[] = {0x2, 0x3};
	struct i2c_client *client = to_i2c_client(dev);
	struct as4610_54_cpld_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);

    for (i = 0; i < ARRAY_SIZE(regs); i++) {
        status = as4610_54_cpld_read_internal(client, regs[i]);
        
        if (status < 0) {
            goto exit;
        }

		switch (i) {
		case 0:
			rx_los |= (status & BIT(4)) >> 4; /* port 25/49 rx_los */
			rx_los |= (status & BIT(0)) << 1; /* port 26/50 rx_los */
			break;
		case 1:
			rx_los |= (status & BIT(4)) >> 2; /* port 27/51 rx_los */
			rx_los |= (status & BIT(0)) << 3; /* port 28/52 rx_los */
			break;
		default:
			break;
		}
    }

	mutex_unlock(&data->update_lock);

    /* Return values 25/49 -> 28/52 in order */
    return sprintf(buf, "%.2x\n", rx_los);

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t show_status(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct as4610_54_cpld_data *data = i2c_get_clientdata(client);
	int status = 0;
	u8 reg = 0, mask = 0;

	switch (attr->index) {
    case MODULE_PRESENT_1:
        reg  = 0x2;
        mask = 0x40;
        break;
    case MODULE_PRESENT_2:
        reg  = 0x2;
        mask = 0x4;
        break;
    case MODULE_PRESENT_3:
        reg  = 0x3;
        mask = 0x40;
        break;
    case MODULE_PRESENT_4:
        reg  = 0x3;
        mask = 0x4;
        break;
    case MODULE_PRESENT_5:
        reg  = 0x21;
        mask = 0x1;
        break;
    case MODULE_PRESENT_6:
        reg  = 0x21;
        mask = 0x10;
        break;
    case MODULE_TXFAULT_1:
        reg  = 0x2;
        mask = 0x20;
        break;
    case MODULE_TXFAULT_2:
        reg  = 0x2;
        mask = 0x2;
        break;
    case MODULE_TXFAULT_3:
        reg  = 0x3;
        mask = 0x20;
        break;
    case MODULE_TXFAULT_4:
        reg  = 0x3;
        mask = 0x2;
        break;
    case MODULE_RXLOS_1:
        reg  = 0x2;
        mask = 0x10;
        break;
    case MODULE_RXLOS_2:
        reg  = 0x2;
        mask = 0x1;
        break;
    case MODULE_RXLOS_3:
        reg  = 0x3;
        mask = 0x10;
        break;
    case MODULE_RXLOS_4:
        reg  = 0x3;
        mask = 0x1;
        break;
	default:
		return 0;
	}

    mutex_lock(&data->update_lock);
	status = as4610_54_cpld_read_internal(client, reg);
	if (unlikely(status < 0)) {
		goto exit;
	}
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n", !!(status & mask));

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
    struct as4610_54_cpld_data *data = i2c_get_clientdata(client);

	if (sscanf(buf, "0x%x 0x%x", &addr, &val) != 2) {
		return -EINVAL;
	}

	if (addr > 0xFF || val > 0xFF) {
		return -EINVAL;
	}

	mutex_lock(&data->update_lock);
	status = as4610_54_cpld_write_internal(client, addr, val);
	if (unlikely(status < 0)) {
		goto exit;
	}
	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static void as4610_54_cpld_add_client(struct i2c_client *client)
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

static void as4610_54_cpld_remove_client(struct i2c_client *client)
{
    struct list_head    *list_node = NULL;
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

static ssize_t show_product_id(struct device *dev, struct device_attribute *attr, char *buf)
{
	int val = 0;
	struct i2c_client *client = to_i2c_client(dev);

	val = i2c_smbus_read_byte_data(client, 0x1);
    if (val < 0) {
        dev_dbg(&client->dev, "cpld(0x%x) reg(0x1) err %d\n", client->addr, val);
    }

	return sprintf(buf, "%d\n", (val & 0xF));
}

static ssize_t show_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0;
    struct i2c_client *client = to_i2c_client(dev);
	
	val = i2c_smbus_read_byte_data(client, 0xB);

    if (val < 0) {
        dev_dbg(&client->dev, "cpld(0x%x) reg(0xB) err %d\n", client->addr, val);
    }
	
    return sprintf(buf, "%d", val);
}

int as4610_product_id(void);

/* I2C init/probing/exit functions */
static int as4610_54_cpld_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adap = to_i2c_adapter(client->dev.parent);
	struct as4610_54_cpld_data *data;
	int ret = -ENODEV;
	int pid;

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE))
		goto exit;

	data = kzalloc(sizeof(struct as4610_54_cpld_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
    mutex_init(&data->update_lock);
	data->type = id->driver_data;
    /* Bring QSFPs out of reset */
    as4610_54_cpld_write_internal(client, 0x2A, 0);

    ret = sysfs_create_group(&client->dev.kobj, &as4610_54_cpld_group);
    if (ret) {
        goto exit_free;
    }

    as4610_54_cpld_add_client(client);

    pid = as4610_product_id();

    switch (pid) {
    case PID_AS4610_30P:
    case PID_AS4610_54P:
    case PID_AS4610_54T_B:
	    data->fan_pdev = platform_device_register_simple("as4610_fan", -1, NULL, 0);
	    if (IS_ERR(data->fan_pdev)) {
		    ret = PTR_ERR(data->fan_pdev);
		    goto exit_unregister;
	    }
	    break;
    default:
	    /* no fan */
	    break;
    }

    if (pid != PID_UNKNOWN) {
	data->led_pdev = platform_device_register_simple("as4610_led", -1, NULL, 0);
	    if (IS_ERR(data->led_pdev)) {
		    ret = PTR_ERR(data->led_pdev);
		    goto exit_unregister_fan;
	    }
    }

    return 0;

exit_unregister_fan:
    platform_device_unregister(data->fan_pdev);

exit_unregister:
    as4610_54_cpld_remove_client(client);
    /* Remove sysfs hooks */
    sysfs_remove_group(&client->dev.kobj, &as4610_54_cpld_group);
exit_free:
    kfree(data);
exit:
	return ret;
}

static int as4610_54_cpld_remove(struct i2c_client *client)
{
    struct as4610_54_cpld_data *data = i2c_get_clientdata(client);

    if (data->led_pdev)
	    platform_device_unregister(data->led_pdev);

    if (data->fan_pdev)
	    platform_device_unregister(data->fan_pdev);

    as4610_54_cpld_remove_client(client);

    /* Remove sysfs hooks */
    sysfs_remove_group(&client->dev.kobj, &as4610_54_cpld_group);
    kfree(data);

    return 0;
}

static int as4610_54_cpld_read_internal(struct i2c_client *client, u8 reg)
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

static int as4610_54_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value)
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

int as4610_54_cpld_read(unsigned short cpld_addr, u8 reg)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EPERM;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client->addr == cpld_addr) {
            ret = as4610_54_cpld_read_internal(cpld_node->client, reg);
    		break;
        }
    }

	mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(as4610_54_cpld_read);

int as4610_54_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EIO;

	mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client->addr == cpld_addr) {
            ret = as4610_54_cpld_write_internal(cpld_node->client, reg, value);
            break;
        }
    }

	mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(as4610_54_cpld_write);

int as4610_product_id(void)
{
	int pid = as4610_54_cpld_read(AS4610_CPLD_SLAVE_ADDR, AS4610_CPLD_PID_OFFSET);
	pid &= 0xF;

	if (pid < PID_AS4610_30T || pid > PID_AS4610_54T_B || pid == PID_RESERVED) {
		return PID_UNKNOWN;
	}

	return pid;
}
EXPORT_SYMBOL(as4610_product_id);

int as4610_is_poe_system(void)
{
	int pid = as4610_product_id();
	return (pid == PID_AS4610_30P || pid == PID_AS4610_54P);
}
EXPORT_SYMBOL(as4610_is_poe_system);

static struct i2c_driver as4610_54_cpld_driver = {
	.driver		= {
		.name	= "as4610_54_cpld",
		.owner	= THIS_MODULE,
	},
	.probe		= as4610_54_cpld_probe,
	.remove		= as4610_54_cpld_remove,
	.id_table	= as4610_54_cpld_id,
};

static int __init as4610_54_cpld_init(void)
{
    mutex_init(&list_lock);
    return i2c_add_driver(&as4610_54_cpld_driver);
}

static void __exit as4610_54_cpld_exit(void)
{
    i2c_del_driver(&as4610_54_cpld_driver);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("Accton I2C CPLD driver");
MODULE_LICENSE("GPL");

module_init(as4610_54_cpld_init);
module_exit(as4610_54_cpld_exit);

