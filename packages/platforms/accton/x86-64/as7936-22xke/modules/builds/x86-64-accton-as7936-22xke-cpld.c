/*
 * A hwmon driver for the as7936_22xke_cpld
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
#include <linux/printk.h>

#define DRV_NAME    "as7936_22xke_cpld"

static LIST_HEAD(cpld_client_list);
static struct mutex	 list_lock;

struct cpld_client_node {
    struct i2c_client *client;
    struct list_head   list;
};

#define I2C_RW_RETRY_COUNT				10
#define I2C_RW_RETRY_INTERVAL			60 /* ms */

static ssize_t show_temp(struct device *dev, struct device_attribute *da,
                         char *buf);
static ssize_t show_status(struct device *dev, struct device_attribute *da,
                           char *buf);
static ssize_t set_tx_disable(struct device *dev, struct device_attribute *da,
                              const char *buf, size_t count);
static ssize_t set_port_reset(struct device *dev, struct device_attribute *da,
                              const char *buf, size_t count);
static ssize_t access(struct device *dev, struct device_attribute *da,
                      const char *buf, size_t count);
static ssize_t show_version(struct device *dev, struct device_attribute *da,
                            char *buf);

struct cpld_data {
    struct mutex        update_lock;
    u8  index;          /* CPLD index */
};

/* Addresses scanned for as7936_22xke_cpld
 */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

#define TRANSCEIVER_PRESENT_ATTR_ID(index)      MODULE_PRESENT_##index
#define TRANSCEIVER_RESET_ATTR_ID(index)        MODULE_RESET_##index
#define TRANSCEIVER_TXDISABLE_ATTR_ID(index)   	MODULE_TXDISABLE_##index
#define TRANSCEIVER_RXLOS_ATTR_ID(index)   		MODULE_RXLOS_##index
#define TEMP_ATTR_ID(index)   		            TEMP_INPUT_##index

typedef enum {
    E_CPLD_ID1 = 0,
    E_CPLD_ID2,
    E_CPLD_ID3,
    E_CPLD_IDMAX
} e_cpld_id;


struct cpld_i2c_t {
    int bus_num;
    unsigned short cpld_addr;
} const cpld_i2cLoc[E_CPLD_IDMAX] = {
    {11, 0x60},
    {12, 0x61},
    {13, 0x62},
};

enum cpld_sysfs_attributes {
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
    TRANSCEIVER_PRESENT_ATTR_ID(29),
    TRANSCEIVER_PRESENT_ATTR_ID(30),
    TRANSCEIVER_TXDISABLE_ATTR_ID(29),
    TRANSCEIVER_TXDISABLE_ATTR_ID(30),
    TRANSCEIVER_RXLOS_ATTR_ID(29),
    TRANSCEIVER_RXLOS_ATTR_ID(30),
    TEMP_ATTR_ID (1),
    TEMP_ATTR_ID (2),
    TEMP_ATTR_ID (3),
    TEMP_ATTR_ID (4),
    TEMP_ATTR_ID (5),
    TEMP_ATTR_ID (6),
    CPLD_VERSION,
    ACCESS,
};

/* sysfs attributes for hwmon
 */

/* qsfp transceiver attributes */
#define DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, show_status, NULL, MODULE_PRESENT_##index); \
	static SENSOR_DEVICE_ATTR(module_reset_##index, S_IRUGO| S_IWUSR, show_status, set_port_reset, MODULE_RESET_##index)
#define DECLARE_TRANSCEIVER_ATTR(index) \
    &sensor_dev_attr_module_present_##index.dev_attr.attr, \
    &sensor_dev_attr_module_reset_##index.dev_attr.attr

/* sfp transceiver attributes */
#define DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, show_status, NULL, MODULE_PRESENT_##index); \
	static SENSOR_DEVICE_ATTR(module_tx_disable_##index, S_IRUGO | S_IWUSR, show_status, set_tx_disable, MODULE_TXDISABLE_##index); \
	static SENSOR_DEVICE_ATTR(module_rx_los_##index, S_IRUGO, show_status, NULL, MODULE_RXLOS_##index);

#define DECLARE_SFP_TRANSCEIVER_ATTR(index)  \
    &sensor_dev_attr_module_present_##index.dev_attr.attr, \
	&sensor_dev_attr_module_tx_disable_##index.dev_attr.attr, \
	&sensor_dev_attr_module_rx_los_##index.dev_attr.attr


#define DECLARE_TEMP_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(temp##index##_input, S_IRUGO, show_temp, NULL, TEMP_INPUT_##index)
#define DECLARE_TEMP_ATTR(index)  \
    &sensor_dev_attr_temp##index##_input.dev_attr.attr

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
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(29);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(30);
DECLARE_TEMP_DEVICE_ATTR(1);
DECLARE_TEMP_DEVICE_ATTR(2);
DECLARE_TEMP_DEVICE_ATTR(3);
DECLARE_TEMP_DEVICE_ATTR(4);
DECLARE_TEMP_DEVICE_ATTR(5);
DECLARE_TEMP_DEVICE_ATTR(6);

static struct attribute *as7936_22xke_cpld1_attributes[] = {
    &sensor_dev_attr_version.dev_attr.attr,
    &sensor_dev_attr_access.dev_attr.attr,
    DECLARE_TEMP_ATTR(1),
    DECLARE_TEMP_ATTR(2),
    DECLARE_TEMP_ATTR(3),
    DECLARE_TEMP_ATTR(4),
    DECLARE_TEMP_ATTR(5),
    DECLARE_TEMP_ATTR(6),
    NULL
};

static struct attribute *as7936_22xke_cpld2_attributes[] = {
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
    DECLARE_SFP_TRANSCEIVER_ATTR(29),
    DECLARE_SFP_TRANSCEIVER_ATTR(30),
    &sensor_dev_attr_version.dev_attr.attr,
    &sensor_dev_attr_access.dev_attr.attr,
    NULL
};

static struct attribute *as7936_22xke_cpld3_attributes[] = {
    DECLARE_TRANSCEIVER_ATTR(17),
    DECLARE_TRANSCEIVER_ATTR(18),
    DECLARE_TRANSCEIVER_ATTR(19),
    DECLARE_TRANSCEIVER_ATTR(20),
    DECLARE_TRANSCEIVER_ATTR(21),
    DECLARE_TRANSCEIVER_ATTR(22),
    DECLARE_TRANSCEIVER_ATTR(23),
    DECLARE_TRANSCEIVER_ATTR(24),
    DECLARE_TRANSCEIVER_ATTR(25),
    DECLARE_TRANSCEIVER_ATTR(26),
    DECLARE_TRANSCEIVER_ATTR(27),
    DECLARE_TRANSCEIVER_ATTR(28),
    &sensor_dev_attr_version.dev_attr.attr,
    &sensor_dev_attr_access.dev_attr.attr,
    NULL
};

int as7936_22xke_cpld_read(int bus_num, unsigned short cpld_addr, u8 reg)
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
EXPORT_SYMBOL(as7936_22xke_cpld_read);

static inline int cpld_readb(e_cpld_id id, u8 reg) {
    return as7936_22xke_cpld_read(cpld_i2cLoc[id].bus_num,
                                  cpld_i2cLoc[id].cpld_addr,
                                  reg);
}

int as7936_22xke_cpld_write(int bus_num, unsigned short cpld_addr, u8 reg, u8 value)
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
EXPORT_SYMBOL(as7936_22xke_cpld_write);

static inline int cpld_writeb(e_cpld_id id, u8 reg, u8 value) {
    return as7936_22xke_cpld_write(cpld_i2cLoc[id].bus_num,
                                   cpld_i2cLoc[id].cpld_addr,
                                   reg, value);
}

static ssize_t show_temp(struct device *dev, struct device_attribute *da,
                         char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    u8 reg;
    long value;

    reg = 0x30 + (attr->index - TEMP_INPUT_1);
    value = i2c_smbus_read_byte_data(client, reg);
    if (value < 0) {
        dev_dbg(dev, "Can't read reg:%02x\n", reg);
        return value;
    }

    return sprintf(buf, "%ld\n", value*1000);
}


static ssize_t show_status(struct device *dev, struct device_attribute *da,
                           char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct cpld_data *data = i2c_get_clientdata(client);
    int pidx;
    int status = 0;
    u8 reg = 0, mask = 0, revert = 1;

    switch (attr->index) {
    case MODULE_PRESENT_1 ... MODULE_PRESENT_28:
        pidx = (attr->index - MODULE_PRESENT_1) % 16;
        reg  = 0x10 + (pidx / 8);
        mask = 0x1 << (pidx % 8);
        break;

    case MODULE_RESET_1 ... MODULE_RESET_28:
        pidx = (attr->index - MODULE_RESET_1) % 16;
        reg  = 0x8 + (pidx / 8);
        mask = 0x1 << (pidx % 8);
        break;

    case MODULE_PRESENT_29 ... MODULE_PRESENT_30:
        reg  = 0x13;
        mask = 0x1 << (attr->index - MODULE_PRESENT_29);
        break;
    case MODULE_TXDISABLE_29 ... MODULE_TXDISABLE_30:
        reg  = 0xB;
        mask = 0x1 << (attr->index - MODULE_TXDISABLE_29);
        revert=0;
        break;
    case MODULE_RXLOS_29 ... MODULE_RXLOS_30:
        reg  = 0x23;
        mask = 0x1 << (attr->index - MODULE_RXLOS_29);
        revert=0;
        break;
    default:
        return -ENXIO;
    }

    mutex_lock(&data->update_lock);
    switch(data->index) {
    case E_CPLD_ID2 ... E_CPLD_ID3:
        status = cpld_readb((data->index - E_CPLD_ID1), reg);
        break;
    default:
        status = -ENXIO;
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

static ssize_t set_port_reset(struct device *dev, struct device_attribute *da,
                              const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct cpld_data *data = i2c_get_clientdata(client);
    long reset;
    int status, val, pidx, cpldi;
    u8 reg = 0x23, mask = 0;

    status = kstrtol(buf, 10, &reset);
    if (status) {
        return status;
    }

    switch (attr->index) {
    case MODULE_RESET_1 ... MODULE_RESET_28:
        pidx = (attr->index - MODULE_RESET_1) % 16;
        reg  = 0x8 + (pidx / 8);
        mask = 0x1 << (pidx % 8);
        break;
    default:
        return 0;
    }

    /* Read current status */
    mutex_lock(&data->update_lock);
    cpldi = data->index - E_CPLD_ID1;
    val = cpld_readb(cpldi, reg);
    if (unlikely(status < 0)) {
        goto exit;
    }

    /* low-active */
    if (reset) {
        val &= ~mask;
    }
    else {
        val |= mask;
    }

    status = cpld_writeb(cpldi, reg, val);
    if (unlikely(status < 0)) {
        goto exit;
    }

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
    struct cpld_data *data = i2c_get_clientdata(client);
    long disable;
    int status, val;
    u8 reg = 0x0b, mask = 0;

    status = kstrtol(buf, 10, &disable);
    if (status) {
        return status;
    }
    switch (attr->index) {
    case MODULE_TXDISABLE_29:
        mask = 0x1;
        break;
    case MODULE_TXDISABLE_30:
        mask = 0x2;
        break;
    default:
        return 0;
    }
    /* Read current status */
    mutex_lock(&data->update_lock);
    val = cpld_readb(E_CPLD_ID2, reg);
    if (unlikely(status < 0)) {
        goto exit;
    }

    /* Update tx_disable status */
    if (disable) {
        val |= mask;
    }
    else {
        val &= ~mask;
    }

    status = cpld_writeb(E_CPLD_ID2, reg, val);
    if (unlikely(status < 0)) {
        goto exit;
    }

    mutex_unlock(&data->update_lock);
    return count;

exit:
    mutex_unlock(&data->update_lock);
    return status;
}

static void as7936_22xke_cpld_add_client(struct i2c_client *client)
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

static void as7936_22xke_cpld_remove_client(struct i2c_client *client)
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
    struct cpld_data *data = i2c_get_clientdata(client);

    if (sscanf(buf, "0x%x 0x%x", &reg, &val) != 2) {
        return -EINVAL;
    }
    if (reg > 0xFF || val > 0xFF) {
        return -EINVAL;
    }


    mutex_lock(&data->update_lock);
    switch(data->index) {
    case E_CPLD_ID1 ... (E_CPLD_IDMAX-1):

        status = cpld_writeb((data->index - E_CPLD_ID1), reg, val);
        break;
    default:
        status = -ENXIO;
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
    struct i2c_client *client = to_i2c_client(dev);
    struct cpld_data *data = i2c_get_clientdata(client);
    int status = 0;

    mutex_lock(&data->update_lock);
    switch(data->index) {
    case 0 ... 2:
        status = cpld_readb((data->index - E_CPLD_ID1), 0x1);
        break;
    default:
        status = -1;
        break;
    }

    if (unlikely(status < 0))
    {
        mutex_unlock(&data->update_lock);
        goto exit;
    }

    mutex_unlock(&data->update_lock);

    return sprintf(buf, "%d\n", status);
exit:
    return status;
}


static int as7936_22xke_cpld_probe(struct i2c_client *client,
                                   const struct i2c_device_id *dev_id)
{
    struct device *hwmon_dev;
    int status;
    struct device *dev = &client->dev;
    struct cpld_data *data = NULL;
    static const struct attribute_group groups[] = {
        {.attrs = as7936_22xke_cpld1_attributes,},
        {.attrs = as7936_22xke_cpld2_attributes,},
        {.attrs = as7936_22xke_cpld3_attributes,},
    };

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_dbg(dev, "i2c_check_functionality failed (0x%x)\n", client->addr);
        return -EIO;
    }

    data = devm_kzalloc(dev, sizeof(struct cpld_data),
                        GFP_KERNEL);
    if (!data) {
        return  -ENOMEM;
    }

    i2c_set_clientdata(client, data);
    data->index = dev_id->driver_data;
    mutex_init(&data->update_lock);
    dev_info(dev, "chip found\n");

    /* Register sysfs hooks */
    switch(data->index) {
    case E_CPLD_ID1 ... E_CPLD_ID3:
        status = devm_device_add_group(dev, &groups[data->index]);
        break;
    default:
        return -EINVAL;
    }
    if (status) {
        return status;
    }

    hwmon_dev = devm_hwmon_device_register_with_info(dev,
                                 client->name, NULL, NULL, NULL);
    if (IS_ERR(hwmon_dev)) {
        dev_dbg(dev, "unable to register hwmon device\n");
        return PTR_ERR(hwmon_dev);
    }

    as7936_22xke_cpld_add_client(client);
    dev_info(dev, "%s: cpld '%s'\n",
             dev_name(dev), client->name);

    return 0;
}

static int as7936_22xke_cpld_remove(struct i2c_client *client)
{
    as7936_22xke_cpld_remove_client(client);
    return 0;
}

static const struct i2c_device_id as7936_22xke_cpld_id[] = {
    { "as7936_22xke_cpld1", E_CPLD_ID1},
    { "as7936_22xke_cpld2", E_CPLD_ID2},
    { "as7936_22xke_cpld3", E_CPLD_ID3},
    {}
};

MODULE_DEVICE_TABLE(i2c, as7936_22xke_cpld_id);

static struct i2c_driver as7936_22xke_cpld_driver = {
    .class        = I2C_CLASS_HWMON,
    .driver = {
        .name     = DRV_NAME,
    },
    .probe        = as7936_22xke_cpld_probe,
    .remove       = as7936_22xke_cpld_remove,
    .id_table     = as7936_22xke_cpld_id,
    .address_list = normal_i2c,
};

static int __init as7936_22xke_cpld_init(void)
{
    mutex_init(&list_lock);
    return i2c_add_driver(&as7936_22xke_cpld_driver);
}

static void __exit as7936_22xke_cpld_exit(void)
{
    i2c_del_driver(&as7936_22xke_cpld_driver);
}

module_init(as7936_22xke_cpld_init);
module_exit(as7936_22xke_cpld_exit);

MODULE_AUTHOR("Jostar Yang <jostar_yang@edge-core.com>");
MODULE_DESCRIPTION("as7936_22xke_cpld driver");
MODULE_LICENSE("GPL");

