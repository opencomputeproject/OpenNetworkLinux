/*
 * An hwmon driver for accton as7716_24sc Transceiver Expansion Card
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
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/delay.h>

#define CPLD_I2C_ADDR				0x60
#define EXPANSION_CARD_PRESENT_REG	0x7A

static ssize_t show_present(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_status(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t set_slot(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count) ;
extern int accton_i2c_cpld_read(unsigned short cpld_addr, u8 reg);

/* Addresses scanned 
 */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

enum as7716_24sc_expansion_card_sysfs_attributes {
    CARD_TYPE,
    CARD_SLOT,
	CARD_PRESENT,
    NUM_OF_ATTR
};

/* Each client has this additional data 
 */
struct as7716_24sc_expansion_card_data {
    struct device      *hwmon_dev;
    struct mutex        update_lock;
    char                valid;           /* !=0 if registers are valid */
    unsigned long       last_updated;    /* In jiffies */
	u8 driver_slot;
	u8 status[NUM_OF_ATTR]; /* 0: card type
	                           1: card slot
	                           2: card presence */
};

static struct as7716_24sc_expansion_card_data *update_device(struct device *dev, int update_all);

/* sysfs attributes for hwmon 
 */
static SENSOR_DEVICE_ATTR(card_present, S_IRUGO, show_present, NULL, CARD_PRESENT);
static SENSOR_DEVICE_ATTR(card_type, S_IRUGO, show_status, NULL, CARD_TYPE);
static SENSOR_DEVICE_ATTR(card_slot, S_IWUSR | S_IRUGO, show_status, set_slot, CARD_SLOT);

static struct attribute *as7716_24sc_expansion_card_attributes[] = {
	&sensor_dev_attr_card_present.dev_attr.attr,
	&sensor_dev_attr_card_type.dev_attr.attr,
    &sensor_dev_attr_card_slot.dev_attr.attr,
    NULL
};

static ssize_t show_present(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct as7716_24sc_expansion_card_data *data = i2c_get_clientdata(client);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    ssize_t status = 0;
	u8 present = 0;

    mutex_lock(&data->update_lock);
    
    data = update_device(dev, 0);
    if (!data->valid) {
        mutex_unlock(&data->update_lock);
        return -EIO;
    }
    present = !(data->status[CARD_PRESENT] & BIT(data->driver_slot));

    mutex_unlock(&data->update_lock);
	return sprintf(buf, "%d\n", (int)present);
}

static ssize_t show_status(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct as7716_24sc_expansion_card_data *data = i2c_get_clientdata(client);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    ssize_t status = 0;
	u8 present = 0;

    mutex_lock(&data->update_lock);

    data = update_device(dev, 1);
    if (!data->valid) {
        status = -EIO;
        goto exit;
    }

	present = !(data->status[CARD_PRESENT] & BIT(data->driver_slot));

	switch (attr->index) {
	case CARD_TYPE:
		status = present ? (data->status[CARD_TYPE] >> 4) : -EIO;
		break;
	case CARD_SLOT:
		status = present ? (data->status[CARD_SLOT] & 0x7) : -EIO;
		break;
	default:
		status = -EINVAL;
        goto exit;
	}

	if (status < 0) {
        goto exit;
	}

    mutex_unlock(&data->update_lock);
    return sprintf(buf, "%d\n", (int)status);

exit:
    mutex_unlock(&data->update_lock);
    return status;
}

static ssize_t set_slot(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count) 
{
    int status, value;
    struct i2c_client *client = to_i2c_client(dev);
    
    status = kstrtoint(buf, 10, &value);
    if (status)
        return status;

    if (value < 0 || value > 7)
        return -EINVAL;

	/* Write the slot id to register 0x02 */
	status = i2c_smbus_write_byte_data(client, 0x02, value);
	if (unlikely(status < 0)) {
		return status;
	}

	return count;
}

static const struct attribute_group as7716_24sc_expansion_card_group = {
    .attrs = as7716_24sc_expansion_card_attributes,
};

static int as7716_24sc_expansion_card_probe(struct i2c_client *client,
            const struct i2c_device_id *dev_id)
{
    struct as7716_24sc_expansion_card_data *data;
    int status;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        status = -EIO;
        goto exit;
    }

    data = kzalloc(sizeof(struct as7716_24sc_expansion_card_data), GFP_KERNEL);
    if (!data) {
        status = -ENOMEM;
        goto exit;
    }

    i2c_set_clientdata(client, data);
    data->valid  = 0;
	data->driver_slot = dev_id->driver_data;
    mutex_init(&data->update_lock);

    dev_info(&client->dev, "chip found\n");

    /* Register sysfs hooks */
    status = sysfs_create_group(&client->dev.kobj, &as7716_24sc_expansion_card_group);
    if (status) {
        goto exit_free;
    }

    data->hwmon_dev = hwmon_device_register_with_info(&client->dev, "as7716_24sc_expansion_card",
                                                      NULL, NULL, NULL);
    if (IS_ERR(data->hwmon_dev)) {
        status = PTR_ERR(data->hwmon_dev);
        goto exit_remove;
    }

    dev_info(&client->dev, "%s: expansion card '%s'\n",
         dev_name(data->hwmon_dev), client->name);
    
    return 0;

exit_remove:
    sysfs_remove_group(&client->dev.kobj, &as7716_24sc_expansion_card_group);
exit_free:
    kfree(data);
exit:
    
    return status;
}

static int as7716_24sc_expansion_card_remove(struct i2c_client *client)
{
    struct as7716_24sc_expansion_card_data *data = i2c_get_clientdata(client);

    hwmon_device_unregister(data->hwmon_dev);
    sysfs_remove_group(&client->dev.kobj, &as7716_24sc_expansion_card_group);
    kfree(data);
    
    return 0;
}

enum expansion_card_index 
{ 
    as7716_24sc_excard1, 
    as7716_24sc_excard2,
    as7716_24sc_excard3,
    as7716_24sc_excard4,
    as7716_24sc_excard5,
    as7716_24sc_excard6,
    as7716_24sc_excard7,
    as7716_24sc_excard8
};

#define I2C_DEV_ID(x) { #x, x}

static const struct i2c_device_id as7716_24sc_expansion_card_id[] = {
I2C_DEV_ID(as7716_24sc_excard1),
I2C_DEV_ID(as7716_24sc_excard2),
I2C_DEV_ID(as7716_24sc_excard3),
I2C_DEV_ID(as7716_24sc_excard4),
I2C_DEV_ID(as7716_24sc_excard5),
I2C_DEV_ID(as7716_24sc_excard6),
I2C_DEV_ID(as7716_24sc_excard7),
I2C_DEV_ID(as7716_24sc_excard8),
{/* LIST END */}
};
MODULE_DEVICE_TABLE(i2c, as7716_24sc_expansion_card_id);

static struct i2c_driver as7716_24sc_expansion_card_driver = {
    .class        = I2C_CLASS_HWMON,
    .driver = {
        .name     = "as7716_24sc_expansion_card",
    },
    .probe        = as7716_24sc_expansion_card_probe,
    .remove       = as7716_24sc_expansion_card_remove,
    .id_table     = as7716_24sc_expansion_card_id,
    .address_list = normal_i2c,
};

static struct as7716_24sc_expansion_card_data *update_device(struct device *dev, int update_all)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct as7716_24sc_expansion_card_data *data = i2c_get_clientdata(client);

    if (time_after(jiffies, data->last_updated + HZ + HZ / 2)
        || !data->valid) {
        int status, i;
		u8  regs[] = {0x00, 0x02}; /* 0x00: type, 0x02: slot */

        data->valid = 0;
        dev_dbg(&client->dev, "Starting as7716_24sc update\n");

        /* Read present status */
        status = accton_i2c_cpld_read(CPLD_I2C_ADDR, EXPANSION_CARD_PRESENT_REG);
        
        if (status < 0) {
            dev_dbg(&client->dev, "cpld reg 0x60 err %d\n", status);
            goto exit;
        }
        else {
            data->status[CARD_PRESENT] = status;
        }

		/* Update card type and slot if card is present */
		if (!(data->status[CARD_PRESENT] & BIT(data->driver_slot))) {
		    for (i = 0; i < ARRAY_SIZE(regs); i++) {
		        status = i2c_smbus_read_byte_data(client, regs[i]);

		        if (status < 0) {
		            goto exit;
		        }
		
		        data->status[i] = status;
		    }	
		}

        data->last_updated = jiffies;
        data->valid = 1;
    }

exit:
    return data;
}

static int __init as7716_24sc_expansion_card_init(void)
{
	return i2c_add_driver(&as7716_24sc_expansion_card_driver);
}

static void __exit as7716_24sc_expansion_card_exit(void)
{
    i2c_del_driver(&as7716_24sc_expansion_card_driver);
}

module_init(as7716_24sc_expansion_card_init);
module_exit(as7716_24sc_expansion_card_exit);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as7716_24sc_expansion_card driver");
MODULE_LICENSE("GPL");

