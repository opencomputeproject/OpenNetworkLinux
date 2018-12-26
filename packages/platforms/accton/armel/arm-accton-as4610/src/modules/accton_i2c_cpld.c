/*
 * A hwmon driver for the accton_i2c_cpld
 *
 * Copyright (C) 2013 Accton Technology Corporation.
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
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/dmi.h>
#include "accton_i2c_cpld.h"

static LIST_HEAD(cpld_client_list);
static struct mutex	 list_lock;

enum cpld_device_id {
    as4610_30_cpld,
    as4610_54_cpld
};

struct cpld_data {
    enum cpld_device_id id;
};

struct cpld_client_node {
    struct i2c_client *client;
    struct list_head   list;
};

/* Addresses scanned for accton_i2c_cpld
 */
static const unsigned short normal_i2c[] = { 0x31, 0x35, 0x60, 0x61, 0x62, I2C_CLIENT_END };

static u8 cpld_product_id_offset(enum cpld_device_id id)
{
    switch (id) {
    case as4610_30_cpld:
    case as4610_54_cpld:
        return 0x1;
    }

    return 0;
}

static int cpld_has_product_id(const struct i2c_device_id *dev_id)
{
    return (dev_id->driver_data == as4610_30_cpld) ||
        (dev_id->driver_data == as4610_54_cpld);
}

static ssize_t show_cpld_product_id(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct cpld_data *data = i2c_get_clientdata(client);

    val = i2c_smbus_read_byte_data(client, cpld_product_id_offset(data->id));
    if (val < 0) {
        dev_dbg(&client->dev, "cpld(0x%x) reg(0x%x) err %d\n", client->addr, cpld_product_id_offset(data->id), val);
    }

    return sprintf(buf, "%d\n", (val & 0xF));
}

static u8 cpld_version_offset(enum cpld_device_id id)
{
    switch (id) {
    case as4610_30_cpld:
    case as4610_54_cpld:
        return 0xB;
    }

    return 0;
}

static ssize_t show_cpld_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct cpld_data *data = i2c_get_clientdata(client);

    val = i2c_smbus_read_byte_data(client, cpld_version_offset(data->id));
    if (val < 0) {
        dev_dbg(&client->dev, "cpld(0x%x) reg(0xB) err %d\n", client->addr, val);
    }

    return sprintf(buf, "%d\n", val);
}

static void accton_i2c_cpld_add_client(struct i2c_client *client, enum cpld_device_id id)
{
    struct cpld_client_node *node = kzalloc(sizeof(struct cpld_client_node), GFP_KERNEL);
    struct cpld_data *data = kzalloc(sizeof(struct cpld_data), GFP_KERNEL);

    if (!node) {
        dev_dbg(&client->dev, "Can't allocate cpld_client_node (0x%x)\n", client->addr);
        return;
    }

    if (!data) {
        dev_dbg(&client->dev, "Can't allocate cpld_client_data (0x%x)\n", client->addr);
        return;
    }

    data->id = id;
    i2c_set_clientdata(client, data);
    node->client = client;

    mutex_lock(&list_lock);
    list_add(&node->list, &cpld_client_list);
    mutex_unlock(&list_lock);
}

static void accton_i2c_cpld_remove_client(struct i2c_client *client)
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

static struct device_attribute ver = __ATTR(version, 0600, show_cpld_version, NULL);
static struct device_attribute pid = __ATTR(product_id, 0600, show_cpld_product_id, NULL);

static int accton_i2c_cpld_probe(struct i2c_client *client,
                                 const struct i2c_device_id *dev_id)
{
    int status;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_dbg(&client->dev, "i2c_check_functionality failed (0x%x)\n", client->addr);
        status = -EIO;
        goto exit;
    }

    status = sysfs_create_file(&client->dev.kobj, &ver.attr);
    if (status) {
        goto exit;
    }

    if (cpld_has_product_id(dev_id)) {
        status = sysfs_create_file(&client->dev.kobj, &pid.attr);
        if (status) {
            goto exit;
        }
    }

    dev_info(&client->dev, "chip found\n");
    accton_i2c_cpld_add_client(client, (enum cpld_device_id)dev_id->driver_data);

    return 0;

 exit:
    return status;
}

static int accton_i2c_cpld_remove(struct i2c_client *client)
{
    sysfs_remove_file(&client->dev.kobj, &ver.attr);
    accton_i2c_cpld_remove_client(client);

    return 0;
}

static const struct i2c_device_id accton_i2c_cpld_id[] = {
    { "as4610_30_cpld", as4610_30_cpld },
    { "as4610_54_cpld", as4610_54_cpld },
    { /* LIST END */}
};
MODULE_DEVICE_TABLE(i2c, accton_i2c_cpld_id);

static struct i2c_driver accton_i2c_cpld_driver = {
    .class		= I2C_CLASS_HWMON,
    .driver = {
        .name = "accton_i2c_cpld",
    },
    .probe		= accton_i2c_cpld_probe,
    .remove		= accton_i2c_cpld_remove,
    .id_table	= accton_i2c_cpld_id,
    .address_list = normal_i2c,
};

int accton_i2c_cpld_read(unsigned short cpld_addr, u8 reg)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EPERM;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
	{
            cpld_node = list_entry(list_node, struct cpld_client_node, list);

            if (cpld_node->client->addr == cpld_addr) {
                ret = i2c_smbus_read_byte_data(cpld_node->client, reg);
                break;
            }
	}

    mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(accton_i2c_cpld_read);

int accton_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EIO;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
        {
            cpld_node = list_entry(list_node, struct cpld_client_node, list);

            if (cpld_node->client->addr == cpld_addr) {
                ret = i2c_smbus_write_byte_data(cpld_node->client, reg, value);
                break;
            }
        }

    mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(accton_i2c_cpld_write);

static int __init accton_i2c_cpld_init(void)
{
    mutex_init(&list_lock);
    return i2c_add_driver(&accton_i2c_cpld_driver);
}

static void __exit accton_i2c_cpld_exit(void)
{
    i2c_del_driver(&accton_i2c_cpld_driver);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("accton_i2c_cpld driver");
MODULE_LICENSE("GPL");

module_init(accton_i2c_cpld_init);
module_exit(accton_i2c_cpld_exit);
