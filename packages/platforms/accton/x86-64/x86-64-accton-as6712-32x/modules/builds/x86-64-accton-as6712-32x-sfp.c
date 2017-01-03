/*
 * An hwmon driver for accton as6712_32x sfp
 *
 * Copyright (C) 2014 Accton Technology Corporation.
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

#define BIT_INDEX(i) (1ULL << (i))

/* Addresses scanned 
 */
static const unsigned short normal_i2c[] = { 0x50, I2C_CLIENT_END };

/* Each client has this additional data 
 */
struct as6712_32x_sfp_data {
    struct device      *hwmon_dev;
    struct mutex        update_lock;
    char                valid;           /* !=0 if registers are valid */
    unsigned long       last_updated;    /* In jiffies */
    int                 port;            /* Front port index */
    char                eeprom[256];     /* eeprom data */
    u64                 is_present;      /* present status */
};

static struct as6712_32x_sfp_data *as6712_32x_sfp_update_device(struct device *dev, int update_eeprom);             
static ssize_t show_present(struct device *dev, struct device_attribute *da,char *buf);
static ssize_t show_eeprom(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_port_number(struct device *dev, struct device_attribute *da,
             char *buf);
static int as6712_32x_sfp_read_byte(struct i2c_client *client, u8 command, u8 *data);
extern int as6712_32x_i2c_cpld_read(unsigned short cpld_addr, u8 reg);
extern int as6712_32x_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);
//extern int accton_i2c_cpld_mux_get_index(int adap_index);

enum as6712_32x_sfp_sysfs_attributes {
    SFP_IS_PRESENT,
    SFP_EEPROM,
    SFP_PORT_NUMBER,
    SFP_IS_PRESENT_ALL
};

/* sysfs attributes for hwmon 
 */
static SENSOR_DEVICE_ATTR(sfp_is_present,  S_IRUGO, show_present, NULL, SFP_IS_PRESENT);
static SENSOR_DEVICE_ATTR(sfp_is_present_all,  S_IRUGO, show_present, NULL, SFP_IS_PRESENT_ALL);
static SENSOR_DEVICE_ATTR(sfp_eeprom,      S_IRUGO, show_eeprom, NULL, SFP_EEPROM);
static SENSOR_DEVICE_ATTR(sfp_port_number,  S_IRUGO, show_port_number, NULL, SFP_PORT_NUMBER);

static struct attribute *as6712_32x_sfp_attributes[] = {
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
    struct as6712_32x_sfp_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%d\n", data->port+1);
}

/* Error-check the CPLD read results. */
#define VALIDATED_READ(_buf, _rv, _read_expr, _invert)  \
do {                                                \
    _rv = (_read_expr);                             \
    if(_rv < 0) {                                   \
        return sprintf(_buf, "READ ERROR\n");       \
    }                                               \
    if(_invert) {                                   \
        _rv = ~_rv;                                 \
    }                                               \
    _rv &= 0xFF;                                    \
} while(0)

static ssize_t show_present(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

    if(attr->index == SFP_IS_PRESENT_ALL) {
        int values[4];
        /*
         * Report the SFP_PRESENCE status for all ports.
         */

        /* SFP_PRESENT Ports 1-8 */
        VALIDATED_READ(buf, values[0], as6712_32x_i2c_cpld_read(0x62, 0xA), 1);
        /* SFP_PRESENT Ports 9-16 */
        VALIDATED_READ(buf, values[1], as6712_32x_i2c_cpld_read(0x62, 0xB), 1);
        /* SFP_PRESENT Ports 17-24 */
        VALIDATED_READ(buf, values[2], as6712_32x_i2c_cpld_read(0x64, 0xA), 1);
        /* SFP_PRESENT Ports 25-32 */
        VALIDATED_READ(buf, values[3], as6712_32x_i2c_cpld_read(0x64, 0xB), 1);

        /* Return values 1 -> 32 in order */
        return sprintf(buf, "%.2x %.2x %.2x %.2x\n",
                       values[0], values[1], values[2], values[3]);
    }
    else { /* SFP_IS_PRESENT */
        u8 val;
        struct as6712_32x_sfp_data *data = as6712_32x_sfp_update_device(dev, 0);

        if (!data->valid) {
            return -EIO;
        }

        val = (data->is_present & BIT_INDEX(data->port)) ? 0 : 1;
        return sprintf(buf, "%d", val);
    }    
}
    
static ssize_t show_eeprom(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct as6712_32x_sfp_data *data = as6712_32x_sfp_update_device(dev, 1);

    if (!data->valid) {
        return 0;
    }

    if ((data->is_present & BIT_INDEX(data->port)) != 0) {
        return 0;
    }

    memcpy(buf, data->eeprom, sizeof(data->eeprom));

    return sizeof(data->eeprom);
}

static const struct attribute_group as6712_32x_sfp_group = {
    .attrs = as6712_32x_sfp_attributes,
};

static int as6712_32x_sfp_probe(struct i2c_client *client,
            const struct i2c_device_id *dev_id)
{
    struct as6712_32x_sfp_data *data;
    int status;

    extern int platform_accton_as6712_32x(void);
    if(!platform_accton_as6712_32x()) { 
      return -ENODEV;
    }

    if (!i2c_check_functionality(client->adapter, /*I2C_FUNC_SMBUS_BYTE_DATA | */I2C_FUNC_SMBUS_WORD_DATA)) {
        status = -EIO;
        goto exit;
    }

    data = kzalloc(sizeof(struct as6712_32x_sfp_data), GFP_KERNEL);
    if (!data) {
        status = -ENOMEM;
        goto exit;
    }

    mutex_init(&data->update_lock);
    data->port = dev_id->driver_data;
    i2c_set_clientdata(client, data);

    dev_info(&client->dev, "chip found\n");

    /* Register sysfs hooks */
    status = sysfs_create_group(&client->dev.kobj, &as6712_32x_sfp_group);
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
    sysfs_remove_group(&client->dev.kobj, &as6712_32x_sfp_group);
exit_free:
    kfree(data);
exit:
    
    return status;
}

static int as6712_32x_sfp_remove(struct i2c_client *client)
{
    struct as6712_32x_sfp_data *data = i2c_get_clientdata(client);

    hwmon_device_unregister(data->hwmon_dev);
    sysfs_remove_group(&client->dev.kobj, &as6712_32x_sfp_group);
    kfree(data);
    
    return 0;
}

enum port_numbers {
as6712_32x_sfp1,  as6712_32x_sfp2,  as6712_32x_sfp3, as6712_32x_sfp4,
as6712_32x_sfp5,  as6712_32x_sfp6,  as6712_32x_sfp7, as6712_32x_sfp8, 
as6712_32x_sfp9,  as6712_32x_sfp10, as6712_32x_sfp11,as6712_32x_sfp12,
as6712_32x_sfp13, as6712_32x_sfp14, as6712_32x_sfp15,as6712_32x_sfp16,
as6712_32x_sfp17, as6712_32x_sfp18, as6712_32x_sfp19,as6712_32x_sfp20,
as6712_32x_sfp21, as6712_32x_sfp22, as6712_32x_sfp23,as6712_32x_sfp24,
as6712_32x_sfp25, as6712_32x_sfp26, as6712_32x_sfp27,as6712_32x_sfp28,
as6712_32x_sfp29, as6712_32x_sfp30, as6712_32x_sfp31,as6712_32x_sfp32
};

static const struct i2c_device_id as6712_32x_sfp_id[] = {
{ "as6712_32x_sfp1",  as6712_32x_sfp1 },  { "as6712_32x_sfp2",  as6712_32x_sfp2 },
{ "as6712_32x_sfp3",  as6712_32x_sfp3 },  { "as6712_32x_sfp4",  as6712_32x_sfp4 },
{ "as6712_32x_sfp5",  as6712_32x_sfp5 },  { "as6712_32x_sfp6",  as6712_32x_sfp6 },
{ "as6712_32x_sfp7",  as6712_32x_sfp7 },  { "as6712_32x_sfp8",  as6712_32x_sfp8 },
{ "as6712_32x_sfp9",  as6712_32x_sfp9 },  { "as6712_32x_sfp10", as6712_32x_sfp10 },
{ "as6712_32x_sfp11", as6712_32x_sfp11 }, { "as6712_32x_sfp12", as6712_32x_sfp12 },
{ "as6712_32x_sfp13", as6712_32x_sfp13 }, { "as6712_32x_sfp14", as6712_32x_sfp14 },
{ "as6712_32x_sfp15", as6712_32x_sfp15 }, { "as6712_32x_sfp16", as6712_32x_sfp16 },
{ "as6712_32x_sfp17", as6712_32x_sfp17 }, { "as6712_32x_sfp18", as6712_32x_sfp18 },
{ "as6712_32x_sfp19", as6712_32x_sfp19 }, { "as6712_32x_sfp20", as6712_32x_sfp20 },
{ "as6712_32x_sfp21", as6712_32x_sfp21 }, { "as6712_32x_sfp22", as6712_32x_sfp22 },
{ "as6712_32x_sfp23", as6712_32x_sfp23 }, { "as6712_32x_sfp24", as6712_32x_sfp24 },
{ "as6712_32x_sfp25", as6712_32x_sfp25 }, { "as6712_32x_sfp26", as6712_32x_sfp26 },
{ "as6712_32x_sfp27", as6712_32x_sfp27 }, { "as6712_32x_sfp28", as6712_32x_sfp28 },
{ "as6712_32x_sfp29", as6712_32x_sfp29 }, { "as6712_32x_sfp30", as6712_32x_sfp30 },
{ "as6712_32x_sfp31", as6712_32x_sfp31 }, { "as6712_32x_sfp32", as6712_32x_sfp32 },
{}
};
MODULE_DEVICE_TABLE(i2c, as6712_32x_sfp_id);


static struct i2c_driver as6712_32x_sfp_driver = {
    .class        = I2C_CLASS_HWMON,
    .driver = {
        .name     = "as6712_32x_sfp",
    },
    .probe        = as6712_32x_sfp_probe,
    .remove       = as6712_32x_sfp_remove,
    .id_table     = as6712_32x_sfp_id,
    .address_list = normal_i2c,
};

#if 0
static int as6712_32x_sfp_read_byte(struct i2c_client *client, u8 command, u8 *data)
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
#endif

static int as6712_32x_sfp_read_word(struct i2c_client *client, u8 command, u16 *data)
{
    int result = i2c_smbus_read_word_data(client, command);

    if (unlikely(result < 0)) {
        dev_dbg(&client->dev, "sfp read byte data failed, command(0x%2x), data(0x%2x)\r\n", command, result);
        goto abort;
    }

    *data = (u16)result;
    result = 0;

abort:
    return result;
}

#define ALWAYS_UPDATE 1

static struct as6712_32x_sfp_data *as6712_32x_sfp_update_device(struct device *dev, int update_eeprom)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct as6712_32x_sfp_data *data = i2c_get_clientdata(client);
    
    mutex_lock(&data->update_lock);

    if (ALWAYS_UPDATE || time_after(jiffies, data->last_updated + HZ + HZ / 2)
        || !data->valid) {
        int status = -1;
        int i = 0, j = 0;

        data->valid = 0;
        
        /* Read present status of port 1~32 */
        data->is_present = 0;
		
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 2; j++) {
                status = as6712_32x_i2c_cpld_read(0x62+i*2, 0xA+j);
                
                if (status < 0) {
                    dev_dbg(&client->dev, "cpld(0x%x) reg(0x%x) err %d\n", 0x62+i*2, 0xA+j, status);
                    goto exit;
                }
                
                data->is_present |= (u64)status << ((i*16) + (j*8));
            }
        }

        if (update_eeprom) {
            /* Read eeprom data based on port number */
            memset(data->eeprom, 0, sizeof(data->eeprom));

            /* Check if the port is present */
            if ((data->is_present & BIT_INDEX(data->port)) == 0) {
                /* read eeprom */
                u16 eeprom_data;          
                for (i = 0; i < (sizeof(data->eeprom) / 2); i++) {
                    status = as6712_32x_sfp_read_word(client, i*2, &eeprom_data);
                    
                    if (status < 0) {
                        dev_dbg(&client->dev, "unable to read eeprom from port(%d)\n", data->port);
                        goto exit;
                    }

                    data->eeprom[i*2]   = eeprom_data & 0xff;
                    data->eeprom[i*2 + 1] = eeprom_data >> 8;
                }
            }
        }
        
        data->last_updated = jiffies;
        data->valid = 1;
    }

exit:
    mutex_unlock(&data->update_lock);

    return data;
}

module_i2c_driver(as6712_32x_sfp_driver);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("accton as6712_32x_sfp driver");
MODULE_LICENSE("GPL");

