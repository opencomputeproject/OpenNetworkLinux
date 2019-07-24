/*
 * A hwmon driver for the asxvolt16_cpld
 *
 * Copyright (C) 2017 Accton Technology Corporation.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
#include <linux/delay.h>

#define CPLD_VERSION_REG	0x2
#define I2C_RW_RETRY_COUNT		10
#define I2C_RW_RETRY_INTERVAL	60 /* ms */

static LIST_HEAD(cpld_client_list);
static struct mutex	 list_lock;

struct cpld_client_node {
	struct i2c_client *client;
	struct list_head   list;
};

/* Addresses scanned for asxvolt16_cpld
 */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

static ssize_t show_cpld_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0;
    struct i2c_client *client = to_i2c_client(dev);
	
	val = i2c_smbus_read_byte_data(client, CPLD_VERSION_REG);

    if (val < 0) {
        dev_dbg(&client->dev, "cpld(0x%x) reg(0x%x) err %d\n", client->addr, CPLD_VERSION_REG, val);
    }
	
    return sprintf(buf, "%d\n", val);
}	

static struct device_attribute ver = __ATTR(version, 0600, show_cpld_version, NULL);

static void asxvolt16_cpld_add_client(struct i2c_client *client)
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

static void asxvolt16_cpld_remove_client(struct i2c_client *client)
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

static int asxvolt16_cpld_probe(struct i2c_client *client,
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

	dev_info(&client->dev, "chip found\n");
	asxvolt16_cpld_add_client(client);
	
	return 0;

exit:
	return status;
}

static int asxvolt16_cpld_remove(struct i2c_client *client)
{
	sysfs_remove_file(&client->dev.kobj, &ver.attr);
	asxvolt16_cpld_remove_client(client);
	
	return 0;
}

enum cpld_chips 
{ 
    asxvolt16_cpld,
	asxvolt16_fpga
};

static const struct i2c_device_id asxvolt16_cpld_id[] = {
	{ "asxvolt16_cpld", asxvolt16_cpld },
	{ "asxvolt16_fpga", asxvolt16_fpga },
	{}
};
MODULE_DEVICE_TABLE(i2c, asxvolt16_cpld_id);

static struct i2c_driver asxvolt16_cpld_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name = "asxvolt16_cpld",
	},
	.probe		= asxvolt16_cpld_probe,
	.remove	   	= asxvolt16_cpld_remove,
	.id_table	= asxvolt16_cpld_id,
	.address_list = normal_i2c,
};

int asxvolt16_cpld_read(unsigned short cpld_addr, u8 reg)
{
	struct list_head   *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int ret = -EIO;
	
	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list)
	{
		cpld_node = list_entry(list_node, struct cpld_client_node, list);
		
		if (cpld_node->client->addr == cpld_addr) {
			int retry = I2C_RW_RETRY_COUNT;
			
			while (retry) {
			ret = i2c_smbus_read_byte_data(cpld_node->client, reg);
				if (unlikely(ret < 0)) {
					msleep(I2C_RW_RETRY_INTERVAL);
					retry--;
					continue;
				}
				break;
			}
			
			break;
		}
	}
	
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(asxvolt16_cpld_read);

int asxvolt16_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
{
	struct list_head   *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int ret = -EIO;
	
	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list)
	{
		cpld_node = list_entry(list_node, struct cpld_client_node, list);
		
		if (cpld_node->client->addr == cpld_addr) {
			int retry = I2C_RW_RETRY_COUNT;
			
			while (retry) {
			ret = i2c_smbus_write_byte_data(cpld_node->client, reg, value);
				if (unlikely(ret < 0)) {
					msleep(I2C_RW_RETRY_INTERVAL);
					retry--;
					continue;
				}
				break;
			}

			break;
		}
	}
	
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(asxvolt16_cpld_write);

static int __init asxvolt16_cpld_init(void)
{
	mutex_init(&list_lock);
	return i2c_add_driver(&asxvolt16_cpld_driver);
}

static void __exit asxvolt16_cpld_exit(void)
{
	i2c_del_driver(&asxvolt16_cpld_driver);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("accton_asxvolt16 driver");
MODULE_LICENSE("GPL");

module_init(asxvolt16_cpld_init);
module_exit(asxvolt16_cpld_exit);

