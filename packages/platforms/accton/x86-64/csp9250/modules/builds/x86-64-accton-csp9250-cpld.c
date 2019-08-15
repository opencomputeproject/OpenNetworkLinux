/*
 * A hwmon driver for the accton_i2c_cpld
 *
 * Copyright (C) 2014 Accton Technology Corporation.
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
#include <linux/sysfs.h>
#include <linux/hwmon-sysfs.h>

static struct dmi_system_id csp9250_dmi_table[] = {
	{
		.ident = "Accton csp9250",
		.matches = {
            DMI_MATCH(DMI_BOARD_VENDOR, "Accton"),
			DMI_MATCH(DMI_PRODUCT_NAME, "csp9250"),
		},
	}
};

int platform_accton_csp9250(void)
{
    return dmi_check_system(csp9250_dmi_table);
}
EXPORT_SYMBOL(platform_accton_csp9250);


static LIST_HEAD(cpld_client_list);
static struct mutex	 list_lock;

struct cpld_client_node {
	struct i2c_client *client;
	struct list_head   list;
};

/* Addresses scanned for accton_i2c_cpld
 */
static const unsigned short normal_i2c[] = { 0x31, 0x35, 0x60, 0x62, 0x64, I2C_CLIENT_END };


//#define DRIVER_CLASS_SWC_NAME           "swc"
//static ssize_t accton_i2c_drv_version_get(struct device *dev, struct device_attribute *attr, char *buf);
//static SENSOR_DEVICE_ATTR(version, S_IRUGO, accton_i2c_drv_version_get, NULL, 0);
//static struct attribute *accton_attributes[] = {
//    &sensor_dev_attr_version.dev_attr.attr,
//    NULL,
//};
//static struct attribute_group accton_attributes_group = {
//    .attrs = accton_attributes,
//};

//struct accton_device {
//	struct device *dev;
	//struct kobject *kobject_version;
//	int id;
//	const char *name;
	
//};
//static struct class sysfs_class_swc = {
//    .name = DRIVER_CLASS_SWC_NAME,
 //   .owner = THIS_MODULE,
//};
//static struct accton_device accton_data;

//#define DRIVER_VERSION					"0.0.0.2"
/*
static ssize_t accton_i2c_drv_version_get(struct device *dev, struct device_attribute *attr, char *buf)
{    
    int status = 0;
    
    status = snprintf(buf, PAGE_SIZE - 1, "\r%s\n", DRIVER_VERSION);

    return status;
}    

*/

static void accton_i2c_cpld_add_client(struct i2c_client *client)
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

static int accton_i2c_cpld_probe(struct i2c_client *client,
			const struct i2c_device_id *dev_id)
{
	int status;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_dbg(&client->dev, "i2c_check_functionality failed (0x%x)\n", client->addr);
		status = -EIO;
		goto exit;
	}

	dev_info(&client->dev, "chip found\n");
	accton_i2c_cpld_add_client(client);
	
	return 0;

exit:
	return status;
}

static int accton_i2c_cpld_remove(struct i2c_client *client)
{
	accton_i2c_cpld_remove_client(client);
	
	return 0;
}

static const struct i2c_device_id accton_i2c_cpld_id[] = {
	{ "accton_i2c_cpld", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, accton_i2c_cpld_id);

static struct i2c_driver accton_i2c_cpld_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name = "accton_i2c_cpld",
	},
	.probe		= accton_i2c_cpld_probe,
	.remove	   	= accton_i2c_cpld_remove,
	.id_table	= accton_i2c_cpld_id,
	.address_list = normal_i2c,
};

int csp9250_i2c_cpld_read(unsigned short cpld_addr, u8 reg)
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
EXPORT_SYMBOL(csp9250_i2c_cpld_read);

int csp9250_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
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
EXPORT_SYMBOL(csp9250_i2c_cpld_write);

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
