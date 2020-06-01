/*
 * An hwmon driver for accton as7936_22xke Power Module
 *
 * Copyright (C) 2019 Edgecore Networks Corporation.
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
#include <linux/dmi.h>


#define DEV_NAME            "as7936_22xke_psu"
#define CPLD_I2C_BUS        11
#define CPLD_I2C_ADDR		(0x60)

static ssize_t show_status(struct device *dev, struct device_attribute *da, char *buf);

extern int as7936_22xke_cpld_read(int bus_num, unsigned short cpld_addr, u8 reg);

/* Addresses scanned
 */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

/* Each client has this additional data
 */
struct psu_data {
    struct mutex        update_lock;
    char                valid;           /* !=0 if registers are valid */
    unsigned long       last_updated;    /* In jiffies */
    u8  index;           /* PSU index */
    u8  psu_present;     /* Status(present) register read from CPLD */
    u8  psu_power_good;  /* Status(power_good) register read from CPLD */
};

static struct psu_data *update_device(struct device *dev);

enum as7936_22xke_psu_sysfs_attributes {
    PSU_PRESENT,
    PSU_POWER_GOOD
};

/* sysfs attributes for hwmon
 */
static SENSOR_DEVICE_ATTR(psu_present,    S_IRUGO, show_status, NULL, PSU_PRESENT);
static SENSOR_DEVICE_ATTR(psu_power_good, S_IRUGO, show_status, NULL, PSU_POWER_GOOD);

static struct attribute *as7936_22xke_psu_attributes[] = {
    &sensor_dev_attr_psu_present.dev_attr.attr,
    &sensor_dev_attr_psu_power_good.dev_attr.attr,
    NULL
};

static ssize_t show_status(struct device *dev, struct device_attribute *da,
                           char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct psu_data *data = update_device(dev);
    u8 status = 0;

    if (!data->valid) {
        return -EIO;
    }

    if (attr->index == PSU_PRESENT) {
        status = !((data->psu_present >> data->index) & 0x1);
    }
    else { /* PSU_POWER_GOOD */
        status = ((data->psu_power_good >> data->index) & 0x1);
    }

    return sprintf(buf, "%d\n", status);
}



static int as7936_22xke_psu_probe(struct i2c_client *client,
                                  const struct i2c_device_id *dev_id)
{
    struct psu_data *data;
    int status;
    struct device *dev = &client->dev;
    static const struct attribute_group group = {
        .attrs = as7936_22xke_psu_attributes,
    };

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK)) {
        return -EIO;
    }

    data = devm_kzalloc(dev, sizeof(struct psu_data),
                        GFP_KERNEL);
    if (!data) {
        return -ENOMEM;
    }

    i2c_set_clientdata(client, data);
    data->valid = 0;
    data->index = dev_id->driver_data;
    mutex_init(&data->update_lock);
    dev_info(dev, "chip found\n");

    /* Register sysfs hooks */
    status = devm_device_add_group(dev, &group);
    if (status) {
        return status;
    }
    dev_info(dev, "%s: psu '%s'\n",
             dev_name(dev), client->name);

    return 0;
}

enum psu_index
{
    as7936_22xke_psu1 = 0,
    as7936_22xke_psu2
};

static const struct i2c_device_id as7936_22xke_psu_id[] = {
    { "as7936_22xke_psu1", as7936_22xke_psu1 },
    { "as7936_22xke_psu2", as7936_22xke_psu2 },
    {}
};
MODULE_DEVICE_TABLE(i2c, as7936_22xke_psu_id);

static struct i2c_driver as7936_22xke_psu_driver = {
    .class        = I2C_CLASS_HWMON,
    .driver = {
        .name     = DEV_NAME,
    },
    .probe        = as7936_22xke_psu_probe,
    .id_table     = as7936_22xke_psu_id,
    .address_list = normal_i2c,
};

static struct psu_data *update_device(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct psu_data *data = i2c_get_clientdata(client);
    enum {
        prst_addr = 0x51,
        pgood_addr = 0x52,
    };

    mutex_lock(&data->update_lock);
    if (time_after(jiffies, data->last_updated + HZ + HZ / 2)
            || !data->valid) {
        int psu_present = 0;
        int power_good = 0;

        data->valid = 0;
        dev_dbg(&client->dev, "Starting %s update\n", DEV_NAME);

        /* Read psu present */
        psu_present = as7936_22xke_cpld_read(CPLD_I2C_BUS, CPLD_I2C_ADDR, prst_addr);

        if (psu_present < 0) {
            dev_dbg(&client->dev, "cpld reg 0x%x err %d\n", prst_addr, psu_present);
            goto exit;
        }
        else {
            data->psu_present = psu_present;
        }

        /* Read psu power good */
        power_good = as7936_22xke_cpld_read(CPLD_I2C_BUS, CPLD_I2C_ADDR, pgood_addr);

        if (power_good < 0) {
            dev_dbg(&client->dev, "cpld reg 0x%x err %d\n", pgood_addr, power_good);
            goto exit;
        }
        else {
            data->psu_power_good = power_good;
        }

        data->last_updated = jiffies;
        data->valid = 1;
    }

exit:
    mutex_unlock(&data->update_lock);

    return data;
}

static int __init as7936_22xke_psu_init(void)
{
    return i2c_add_driver(&as7936_22xke_psu_driver);
}

static void __exit as7936_22xke_psu_exit(void)
{
    i2c_del_driver(&as7936_22xke_psu_driver);
}

module_init(as7936_22xke_psu_init);
module_exit(as7936_22xke_psu_exit);

MODULE_AUTHOR("Jostar Yang <jostar_yang@edge-core.com>");
MODULE_DESCRIPTION("as7936_22xke_psu driver");
MODULE_LICENSE("GPL");

