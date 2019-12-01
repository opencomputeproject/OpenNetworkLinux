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
#include <linux/hwmon-sysfs.h>


#define CPLD_VERSION_REG	0x2
#define I2C_RW_RETRY_COUNT		10
#define I2C_RW_RETRY_INTERVAL	60 /* ms */

static LIST_HEAD(cpld_client_list);
static struct mutex	 list_lock;

struct cpld_client_node {
	struct i2c_client *client;
	struct list_head   list;
};

enum cpld_type 
{ 
    asxvolt16_cpld,
    asxvolt16_fpga
};

struct asxvolt16_cpld_data {
    enum cpld_type   type;
    struct device   *hwmon_dev;
    struct mutex     update_lock;
    char             valid;           /* != 0 if registers are valid */
    unsigned long    last_updated;    /* In jiffies */
    u8  reg_val;
};

#define MAPLE_RESET_ATTR_ID(index)   	  MAPLE_RESET_##index
#define PON_PORT_ACT_LED_ATTR_ID(index)   PON_PORT_ACT_LED_##index

enum asxvolt16_cpld_sysfs_attributes {
    CPLD_VERSION,
    MAPLE_RESET_ATTR_ID(1),
    MAPLE_RESET_ATTR_ID(2),
    MAPLE_RESET_ATTR_ID(3),
    MAPLE_RESET_ATTR_ID(4),
    MAPLE_RESET_ATTR_ID(5),
    MAPLE_RESET_ATTR_ID(6),
    MAPLE_RESET_ATTR_ID(7),
    MAPLE_RESET_ATTR_ID(8),
    PON_PORT_ACT_LED_ATTR_ID(1),
    PON_PORT_ACT_LED_ATTR_ID(2),
    PON_PORT_ACT_LED_ATTR_ID(3),
    PON_PORT_ACT_LED_ATTR_ID(4),
    PON_PORT_ACT_LED_ATTR_ID(5),
    PON_PORT_ACT_LED_ATTR_ID(6),
    PON_PORT_ACT_LED_ATTR_ID(7),
    PON_PORT_ACT_LED_ATTR_ID(8),
    PON_PORT_ACT_LED_ATTR_ID(9),
    PON_PORT_ACT_LED_ATTR_ID(10),
    PON_PORT_ACT_LED_ATTR_ID(11),
    PON_PORT_ACT_LED_ATTR_ID(12),
    PON_PORT_ACT_LED_ATTR_ID(13),
    PON_PORT_ACT_LED_ATTR_ID(14),
    PON_PORT_ACT_LED_ATTR_ID(15),
    PON_PORT_ACT_LED_ATTR_ID(16),
};


/* Addresses scanned for asxvolt16_cpld
 */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

static ssize_t show_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0;
    struct i2c_client *client = to_i2c_client(dev);
	
	val = i2c_smbus_read_byte_data(client, CPLD_VERSION_REG);

    if (val < 0) {
        dev_dbg(&client->dev, "cpld(0x%x) reg(0x%x) err %d\n", client->addr, CPLD_VERSION_REG, val);
    }
	
    return sprintf(buf, "%d\n", val);
}	
static SENSOR_DEVICE_ATTR(version, S_IRUGO, show_version, NULL, CPLD_VERSION);

static ssize_t show_pon_port_act_led(struct device *dev, struct device_attribute *da,
             char *buf);
static ssize_t set_pon_port_act_led(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);

static ssize_t show_reset(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t set_reset(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static int asxvolt16_cpld_read_internal(struct i2c_client *client, u8 reg);
static int asxvolt16_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value);
/* maple reset attributes */

#define DECLARE_MAPLE_RESET_SENSOR_DEVICE_ATTR(index) \
    static SENSOR_DEVICE_ATTR(maple_reset_##index, S_IWUSR|S_IRUGO, show_reset, set_reset, MAPLE_RESET_##index)
#define DECLARE_MAPLE_RESET_ATTR(index)  \
    &sensor_dev_attr_maple_reset_##index.dev_attr.attr

#define DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(index) \
    static SENSOR_DEVICE_ATTR(pon_port_act_led_##index, S_IRUGO | S_IWUSR, show_pon_port_act_led, set_pon_port_act_led, PON_PORT_ACT_LED_##index);
    
#define DECLARE_GPON_PORT_LED_ATTR(index)  \
    &sensor_dev_attr_pon_port_act_led_##index.dev_attr.attr

/* maple reset attributes */
DECLARE_MAPLE_RESET_SENSOR_DEVICE_ATTR(1);
DECLARE_MAPLE_RESET_SENSOR_DEVICE_ATTR(2);
DECLARE_MAPLE_RESET_SENSOR_DEVICE_ATTR(3);
DECLARE_MAPLE_RESET_SENSOR_DEVICE_ATTR(4);
DECLARE_MAPLE_RESET_SENSOR_DEVICE_ATTR(5);
DECLARE_MAPLE_RESET_SENSOR_DEVICE_ATTR(6);
DECLARE_MAPLE_RESET_SENSOR_DEVICE_ATTR(7);
DECLARE_MAPLE_RESET_SENSOR_DEVICE_ATTR(8);

DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(1);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(2);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(3);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(4);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(5);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(6);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(7);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(8);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(9);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(10);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(11);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(12);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(13);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(14);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(15);
DECLARE_GPON_PORT_LED_SENSOR_DEVICE_ATTR(16);


static struct attribute *asxvolt16_cpld_attributes[] = {
    &sensor_dev_attr_version.dev_attr.attr,
    DECLARE_MAPLE_RESET_ATTR(1),
    DECLARE_MAPLE_RESET_ATTR(2),
    DECLARE_MAPLE_RESET_ATTR(3),
    DECLARE_MAPLE_RESET_ATTR(4),
    DECLARE_MAPLE_RESET_ATTR(5),
    DECLARE_MAPLE_RESET_ATTR(6),
    DECLARE_MAPLE_RESET_ATTR(7),
    DECLARE_MAPLE_RESET_ATTR(8),
    DECLARE_GPON_PORT_LED_ATTR(1),
    DECLARE_GPON_PORT_LED_ATTR(2),
    DECLARE_GPON_PORT_LED_ATTR(3),
    DECLARE_GPON_PORT_LED_ATTR(4),
    DECLARE_GPON_PORT_LED_ATTR(5),
    DECLARE_GPON_PORT_LED_ATTR(6),
    DECLARE_GPON_PORT_LED_ATTR(7),
    DECLARE_GPON_PORT_LED_ATTR(8),
    DECLARE_GPON_PORT_LED_ATTR(9),
    DECLARE_GPON_PORT_LED_ATTR(10),
    DECLARE_GPON_PORT_LED_ATTR(11),
    DECLARE_GPON_PORT_LED_ATTR(12),
    DECLARE_GPON_PORT_LED_ATTR(13),
    DECLARE_GPON_PORT_LED_ATTR(14),
    DECLARE_GPON_PORT_LED_ATTR(15),
    DECLARE_GPON_PORT_LED_ATTR(16),
    NULL
};

static struct attribute *asxvolt16_fpga_attributes[] = {
    &sensor_dev_attr_version.dev_attr.attr,
};

static const struct attribute_group asxvolt16_cpld_group = {
	.attrs = asxvolt16_cpld_attributes,
};

static const struct attribute_group asxvolt16_fpga_group = {
	.attrs = asxvolt16_fpga_attributes,
};


static struct asxvolt16_cpld_data *asxvolt16_cpld_update_device(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct asxvolt16_cpld_data *data = i2c_get_clientdata(client);
    int status;
    u8 reg=0x37;
    
    mutex_lock(&data->update_lock);
    if (time_after(jiffies, data->last_updated + HZ + HZ / 2) || 
        !data->valid)
    {
        dev_dbg(&client->dev, "Starting asxvolt16_cpld update\n");
        data->valid = 0;
        /* Update cpld data
         */
        status = asxvolt16_cpld_read_internal(client, reg);
        if (status < 0)
        {
            data->valid = 0;
            mutex_unlock(&data->update_lock);
            dev_dbg(&client->dev, "reg %d, err %d\n", reg, status);
            return data;
        }
        else
        {
            data->reg_val = status;
        }
        data->last_updated = jiffies;
        data->valid = 1;
    }
    mutex_unlock(&data->update_lock);

    return data;
}

static ssize_t show_reset(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    u8 mask = 0, revert = 1;
    struct asxvolt16_cpld_data *data = asxvolt16_cpld_update_device(dev);
    
    if (!data->valid)
    {
        return -EIO;
    }
    if (attr->index >=MAPLE_RESET_1 && attr->index <=MAPLE_RESET_8)
    {
        mask=0x1 << (attr->index - MAPLE_RESET_1);
    }
    else
    {
       return -ENXIO;
    }
    
    return sprintf(buf, "%d\n", revert ? !(data->reg_val & mask) : !!(data->reg_val & mask));
}


static ssize_t set_reset(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct asxvolt16_cpld_data *data = i2c_get_clientdata(client);
    long input;
    int status;
    u8 reg=0x37, mask=0;
        
    status = kstrtol(buf, 10, &input);
    if (status) 
    {
        return status;
    }
    if (input)
     {
        input=1;
    }
    else
    {
        input=0;
    }
	
    if (attr->index >=MAPLE_RESET_1 && attr->index <=MAPLE_RESET_8)
    {
        mask=0x1 << (attr->index - MAPLE_RESET_1);
    }
    else
    {
       return -ENXIO;
    }
	    
    /* Read current status */
    mutex_lock(&data->update_lock);
       
    status = asxvolt16_cpld_read_internal(client, reg);
    if (unlikely(status < 0))
    {
        goto exit;
    }
    /* Update led status */
    if (input)
    {
        status &= ~mask;
    }
    else
    {
        status |= mask;
    }
    status = asxvolt16_cpld_write_internal(client, reg, status);
    if (unlikely(status < 0))
    {
        goto exit;
    }
	
exit:
    mutex_unlock(&data->update_lock);
    return count;
}

static ssize_t set_pon_port_act_led(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct asxvolt16_cpld_data *data = i2c_get_clientdata(client);
    long input;
    int status;
    u8 reg=0x44, mask=0;
        
    status = kstrtol(buf, 10, &input);
    if (status) 
    {
        return status;
    }
    if (input)
     {
        input=1;
    }
    else
    {
        input=0;
    }
	
    if (attr->index >=PON_PORT_ACT_LED_1 && attr->index <=PON_PORT_ACT_LED_8)
    {
        reg=0x44;
        mask=0x1 << (attr->index - PON_PORT_ACT_LED_1);
    }
    else if (attr->index >=PON_PORT_ACT_LED_9 && attr->index <=PON_PORT_ACT_LED_16)
    {
        reg=0x45;
        mask=0x1 << (attr->index - PON_PORT_ACT_LED_9);
    }
    else
    {
       return -ENXIO;
    }
	    
    /* Read current status */
    mutex_lock(&data->update_lock);
       
    status = asxvolt16_cpld_read_internal(client, reg);
    if (unlikely(status < 0))
    {
        goto exit;
    }
    /* Update led status */
    if (input)
    {
        status |= mask;
    }
    else
    {
        status &= ~mask;
    }
    status = asxvolt16_cpld_write_internal(client, reg, status);
    if (unlikely(status < 0))
    {
        goto exit;
    }
	
exit:
    mutex_unlock(&data->update_lock);
    return count;
}

static ssize_t show_pon_port_act_led(struct device *dev, struct device_attribute *da,
             char *buf)
{ 
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct asxvolt16_cpld_data *data = i2c_get_clientdata(client);
    int status = 0;
    u8 mask = 0, revert = 0, reg=0x44;
    
    if (attr->index >=PON_PORT_ACT_LED_1 && attr->index <=PON_PORT_ACT_LED_8)
    {
        reg=0x44;
        mask=0x1 << (attr->index - PON_PORT_ACT_LED_1);
    }
    else if((attr->index >=PON_PORT_ACT_LED_9 && attr->index <=PON_PORT_ACT_LED_16))
    {
        reg=0x45;
        mask=0x1 << (attr->index - PON_PORT_ACT_LED_9);   
    }
    else 
    {
       return -ENXIO;
    }
     /* Read current status */
    mutex_lock(&data->update_lock);
       
    status = asxvolt16_cpld_read_internal(client, reg);
    if (unlikely(status < 0))
    {
        goto exit;
    }
    mutex_unlock(&data->update_lock);
    
    return sprintf(buf, "%d\n", revert ? !(status & mask) : !!(status & mask));
       
exit:
    mutex_unlock(&data->update_lock);
    return status;

       
}

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
    struct asxvolt16_cpld_data *data;
    int ret = -ENODEV;
    const struct attribute_group *group = NULL;
    
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_dbg(&client->dev, "i2c_check_functionality failed (0x%x)\n", client->addr);
        status = -EIO;
        goto exit;
    }
    
    data = kzalloc(sizeof(struct asxvolt16_cpld_data), GFP_KERNEL);
    if (!data) {
        ret = -ENOMEM;
        goto exit;
    }

    i2c_set_clientdata(client, data);
    mutex_init(&data->update_lock);
    data->type = dev_id->driver_data;
       
    /* Register sysfs hooks */
    switch (data->type)
    {             
        case asxvolt16_cpld:
            group = &asxvolt16_cpld_group;
            break;
        case asxvolt16_fpga:
            group = &asxvolt16_fpga_group;
            break;
        default:
            break;
    }

    if (group)
    {
        ret = sysfs_create_group(&client->dev.kobj, group);
        if (ret) {
            goto exit_free;
        }
    }

    dev_info(&client->dev, "chip found\n");
    asxvolt16_cpld_add_client(client);
	
    return 0;

exit_free:
    kfree(data);
exit:
	return ret;
}

static int asxvolt16_cpld_remove(struct i2c_client *client)
{
    struct asxvolt16_cpld_data *data = i2c_get_clientdata(client);
    const struct attribute_group *group = NULL;

	asxvolt16_cpld_remove_client(client);
	
    /* Remove sysfs hooks */
    switch (data->type)
    {
        case asxvolt16_cpld:
            group = &asxvolt16_cpld_group;
            break;
        case asxvolt16_fpga:
            group = &asxvolt16_fpga_group;
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

static int asxvolt16_cpld_read_internal(struct i2c_client *client, u8 reg)
{
    int status = 0, retry = I2C_RW_RETRY_COUNT;

    while (retry)
    {
        status = i2c_smbus_read_byte_data(client, reg);
        if (unlikely(status < 0)) 
        {
            msleep(I2C_RW_RETRY_INTERVAL);
            retry--;
            continue;
        }
        break;
    }

    return status;
}

static int asxvolt16_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value)
{
    int status = 0, retry = I2C_RW_RETRY_COUNT;
    
    while (retry) 
    {
        status = i2c_smbus_write_byte_data(client, reg, value);
        if (unlikely(status < 0))
        {
	    msleep(I2C_RW_RETRY_INTERVAL);
            retry--;
            continue;
        }
        break;
    }

    return status;
}

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

