/*
 * An hwmon driver for accton as5812_54t sfp
 *
 * Copyright (C) 2015 Accton Technology Corporation.
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

#define QSFP_PORT_START_INDEX 49
#define BIT_INDEX(i) (1ULL << (i))

/* Addresses scanned 
 */
static const unsigned short normal_i2c[] = { 0x50, I2C_CLIENT_END };

/* Each client has this additional data 
 */
struct as5812_54t_sfp_data {
    struct device      *hwmon_dev;
    struct mutex        update_lock;
    char                valid;           /* !=0 if registers are valid */
    unsigned long       last_updated;    /* In jiffies */
    int                 port;            /* Front port index */
    char                eeprom[256];     /* eeprom data */
    u8                  status;       /* bit0:port49, bit1:port50 and so on */
};

static struct as5812_54t_sfp_data *as5812_54t_sfp_update_device(struct device *dev, int update_eeprom);
static ssize_t show_port_number(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_status(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_eeprom(struct device *dev, struct device_attribute *da, char *buf);
extern int accton_i2c_cpld_read(unsigned short cpld_addr, u8 reg);
extern int accton_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

enum as5812_54t_sfp_sysfs_attributes {
    SFP_IS_PRESENT,
    SFP_PORT_NUMBER,
    SFP_EEPROM,
    SFP_IS_PRESENT_ALL,
};

/* sysfs attributes for hwmon 
 */
static SENSOR_DEVICE_ATTR(sfp_is_present,  S_IRUGO, show_status, NULL, SFP_IS_PRESENT);
static SENSOR_DEVICE_ATTR(sfp_port_number, S_IRUGO, show_port_number, NULL, SFP_PORT_NUMBER);
static SENSOR_DEVICE_ATTR(sfp_eeprom,      S_IRUGO, show_eeprom, NULL, SFP_EEPROM);
static SENSOR_DEVICE_ATTR(sfp_is_present_all, S_IRUGO, show_status,NULL, SFP_IS_PRESENT_ALL);

static struct attribute *as5812_54t_sfp_attributes[] = {
    &sensor_dev_attr_sfp_is_present.dev_attr.attr,
    &sensor_dev_attr_sfp_eeprom.dev_attr.attr,
    &sensor_dev_attr_sfp_port_number.dev_attr.attr,
    &sensor_dev_attr_sfp_is_present_all.dev_attr.attr,
    NULL
};

static ssize_t show_port_number(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct as5812_54t_sfp_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%d\n",data->port);
}

static ssize_t show_status(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct as5812_54t_sfp_data *data = as5812_54t_sfp_update_device(dev, 0);
	
    if (attr->index == SFP_IS_PRESENT) {
        u8 val;
    
        val = (data->status & BIT_INDEX(data->port - QSFP_PORT_START_INDEX)) ? 0 : 1;
        return sprintf(buf, "%d", val);
    }
    else { /* SFP_IS_PRESENT_ALL */
        return sprintf(buf, "%.2x\n", ~data->status);
    }
}

static ssize_t show_eeprom(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct as5812_54t_sfp_data *data = as5812_54t_sfp_update_device(dev, 1);

    if (!data->valid) {
        return 0;
    }

    if ((data->status & BIT_INDEX(data->port - QSFP_PORT_START_INDEX)) != 0) {
        return 0;
    }

    memcpy(buf, data->eeprom, sizeof(data->eeprom));

    return sizeof(data->eeprom);
}

static const struct attribute_group as5812_54t_sfp_group = {
    .attrs = as5812_54t_sfp_attributes,
};

static int as5812_54t_sfp_probe(struct i2c_client *client,
            const struct i2c_device_id *dev_id)
{
    struct as5812_54t_sfp_data *data;
    int status;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        status = -EIO;
        goto exit;
    }

    data = kzalloc(sizeof(struct as5812_54t_sfp_data), GFP_KERNEL);
    if (!data) {
        status = -ENOMEM;
        goto exit;
    }

    mutex_init(&data->update_lock);
    data->port = dev_id->driver_data;
    i2c_set_clientdata(client, data);

    dev_info(&client->dev, "chip found\n");

    /* Register sysfs hooks */
    status = sysfs_create_group(&client->dev.kobj, &as5812_54t_sfp_group);
    if (status) {
        goto exit_free;
    }

    data->hwmon_dev = hwmon_device_register(&client->dev);
    if (IS_ERR(data->hwmon_dev)) {
        status = PTR_ERR(data->hwmon_dev);
        goto exit_remove;
    }

    dev_info(&client->dev, "%s: sfp '%s'\n",
         dev_name(data->hwmon_dev), client->name);
    
    return 0;

exit_remove:
    sysfs_remove_group(&client->dev.kobj, &as5812_54t_sfp_group);
exit_free:
    kfree(data);
exit:
    
    return status;
}

static int as5812_54t_sfp_remove(struct i2c_client *client)
{
    struct as5812_54t_sfp_data *data = i2c_get_clientdata(client);

    hwmon_device_unregister(data->hwmon_dev);
    sysfs_remove_group(&client->dev.kobj, &as5812_54t_sfp_group);
    kfree(data);
    
    return 0;
}

enum port_numbers {
as5812_54t_qsfp49 = 49,
as5812_54t_qsfp50,
as5812_54t_qsfp51,
as5812_54t_qsfp52,
as5812_54t_qsfp53,
as5812_54t_qsfp54
};

static const struct i2c_device_id as5812_54t_sfp_id[] = {
{ "as5812_54t_qsfp49", as5812_54t_qsfp49 }, { "as5812_54t_qsfp50", as5812_54t_qsfp50 },
{ "as5812_54t_qsfp51", as5812_54t_qsfp51 }, { "as5812_54t_qsfp52", as5812_54t_qsfp52 },
{ "as5812_54t_qsfp53", as5812_54t_qsfp53 }, { "as5812_54t_qsfp54", as5812_54t_qsfp54 },
{}
};
MODULE_DEVICE_TABLE(i2c, as5812_54t_sfp_id);

static struct i2c_driver as5812_54t_sfp_driver = {
    .class        = I2C_CLASS_HWMON,
    .driver = {
        .name     = "as5812_54t_sfp",
    },
    .probe        = as5812_54t_sfp_probe,
    .remove       = as5812_54t_sfp_remove,
    .id_table     = as5812_54t_sfp_id,
    .address_list = normal_i2c,
};

static int as5812_54t_sfp_read_byte(struct i2c_client *client, u8 command, u8 *data)
{
    int result = i2c_smbus_read_byte_data(client, command);

    if (unlikely(result < 0)) {
        dev_dbg(&client->dev, "sfp read byte data failed, command(0x%2x), data(0x%2x)\r\n", command, result);
        goto abort;
    }

    *data = (u8)result;
    result = 0;
    
abort:
    return result;
}

static struct as5812_54t_sfp_data *as5812_54t_sfp_update_device(struct device *dev, int update_eeprom)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct as5812_54t_sfp_data *data = i2c_get_clientdata(client);

    mutex_lock(&data->update_lock);

    if (time_after(jiffies, data->last_updated + HZ + HZ / 2)
        || !data->valid || update_eeprom) {
        int status = -1;
        int i = 0;

        data->valid = 0;
        //dev_dbg(&client->dev, "Starting as5812_54t sfp status update\n");        
        data->status = 0xFF;

        /*
         * Bring QSFPs out of reset,
         * This is a temporary fix until the QSFP+_MOD_RST register
         * can be exposed through the driver.
         */
        accton_i2c_cpld_write(0x60, 0x23, 0x3F);

        /* Read present status of port 49-54(QSFP port) */
        status = accton_i2c_cpld_read(0x60, 0x22);

        if (status < 0) {
            dev_dbg(&client->dev, "cpld(0x60) reg(0x22) err %d\n", status);
        }
        else {
            data->status = status & 0x3F; /* (u32)status */
        }

        if (update_eeprom) {
            /* Read eeprom data based on port number */
            memset(data->eeprom, 0, sizeof(data->eeprom));

            /* Check if the port is present */
            if ((data->status & BIT_INDEX(data->port - QSFP_PORT_START_INDEX)) == 0) {
                /* read eeprom */
                for (i = 0; i < sizeof(data->eeprom); i++) {
                    status = as5812_54t_sfp_read_byte(client, i, data->eeprom + i);

                    if (status < 0) {
                        dev_dbg(&client->dev, "unable to read eeprom from port(%d)\n",
                                              data->port);
                        goto exit;
                    }
                }
            }
        }

        data->valid = 1;
        data->last_updated = jiffies;
    }

exit:
    mutex_unlock(&data->update_lock);

    return data;
}

static int __init as5812_54t_sfp_init(void)
{
	extern int platform_accton_as5812_54t(void);
	if (!platform_accton_as5812_54t()) {
		return -ENODEV;
	}

    return i2c_add_driver(&as5812_54t_sfp_driver);
}

static void __exit as5812_54t_sfp_exit(void)
{
    i2c_del_driver(&as5812_54t_sfp_driver);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("accton as5812_54t_sfp driver");
MODULE_LICENSE("GPL");

module_init(as5812_54t_sfp_init);
module_exit(as5812_54t_sfp_exit);

