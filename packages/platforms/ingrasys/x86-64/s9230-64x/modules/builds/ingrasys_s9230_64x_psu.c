/*
 * S9230-64x PSU driver
 *
 * Copyright (C) 2017 Ingrasys, Inc.
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
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/dmi.h>

#ifndef _S9230_64X_PLATFORM_H
#define _S9230_64X_PLATFORM_H

#include <linux/i2c.h>

// remove debug before release
#define DEBUG

enum bus_order {
    I2C_BUS_MAIN,
    MUX_9548_0_CH0,
    MUX_9548_0_CH1,
    MUX_9548_0_CH2,
    MUX_9548_0_CH3,
    MUX_9548_0_CH4,
    MUX_9548_0_CH5,
    MUX_9548_0_CH6,
    MUX_9548_0_CH7,
    MUX_9548_1_CH0,
    MUX_9548_1_CH1,
    MUX_9548_1_CH2,
    MUX_9548_1_CH3,
    MUX_9548_1_CH4,
    MUX_9548_1_CH5,
    MUX_9548_1_CH6,
    MUX_9548_1_CH7,
    MUX_9546_0_CH0,
    MUX_9546_0_CH1,
    MUX_9546_0_CH2,
    MUX_9546_0_CH3,
    MUX_9546_1_CH0,
    MUX_9546_1_CH1,
    MUX_9546_1_CH2,
    MUX_9546_1_CH3,
    MUX_9548_2_CH0,
    MUX_9548_2_CH1,
    MUX_9548_2_CH2,
    MUX_9548_2_CH3,
    MUX_9548_2_CH4,
    MUX_9548_2_CH5,
    MUX_9548_2_CH6,
    MUX_9548_2_CH7,
    MUX_9548_3_CH0,
    MUX_9548_3_CH1,
    MUX_9548_3_CH2,
    MUX_9548_3_CH3,
    MUX_9548_3_CH4,
    MUX_9548_3_CH5,
    MUX_9548_3_CH6,
    MUX_9548_3_CH7,
    MUX_9548_4_CH0,
    MUX_9548_4_CH1,
    MUX_9548_4_CH2,
    MUX_9548_4_CH3,
    MUX_9548_4_CH4,
    MUX_9548_4_CH5,
    MUX_9548_4_CH6,
    MUX_9548_4_CH7,
    MUX_9548_5_CH0,
    MUX_9548_5_CH1,
    MUX_9548_5_CH2,
    MUX_9548_5_CH3,
    MUX_9548_5_CH4,
    MUX_9548_5_CH5,
    MUX_9548_5_CH6,
    MUX_9548_5_CH7,
    MUX_9548_6_CH0,
    MUX_9548_6_CH1,
    MUX_9548_6_CH2,
    MUX_9548_6_CH3,
    MUX_9548_6_CH4,
    MUX_9548_6_CH5,
    MUX_9548_6_CH6,
    MUX_9548_6_CH7,
    MUX_9548_7_CH0,
    MUX_9548_7_CH1,
    MUX_9548_7_CH2,
    MUX_9548_7_CH3,
    MUX_9548_7_CH4,
    MUX_9548_7_CH5,
    MUX_9548_7_CH6,
    MUX_9548_7_CH7,
    MUX_9548_8_CH0,
    MUX_9548_8_CH1,
    MUX_9548_8_CH2,
    MUX_9548_8_CH3,
    MUX_9548_8_CH4,
    MUX_9548_8_CH5,
    MUX_9548_8_CH6,
    MUX_9548_8_CH7,
    MUX_9548_9_CH0,
    MUX_9548_9_CH1,
    MUX_9548_9_CH2,
    MUX_9548_9_CH3,
    MUX_9548_9_CH4,
    MUX_9548_9_CH5,
    MUX_9548_9_CH6,
    MUX_9548_9_CH7,
    MUX_9548_10_CH0,
    MUX_9548_10_CH1,
    MUX_9548_10_CH2,
    MUX_9548_10_CH3,
    MUX_9548_10_CH4,
    MUX_9548_10_CH5,
    MUX_9548_10_CH6,
    MUX_9548_10_CH7,
};

#define I2C_ADDR_MUX_9555_0      (0x20)
#define I2C_ADDR_MUX_9555_1      (0x24)
#define I2C_ADDR_MUX_9555_2      (0x25)
#define I2C_ADDR_MUX_9555_3      (0x26)
#define I2C_ADDR_MUX_9539_0      (0x76)
#define I2C_ADDR_MUX_9539_1      (0x76)
#define I2C_BUS_FAN_STATUS       (I2C_BUS_MAIN)
#define I2C_BUS_SYS_LED          (MUX_9548_1_CH1)

#define NUM_OF_I2C_MUX              (11)
#define NUM_OF_CPLD                     (5)
#define NUM_OF_QSFP_PORT          (64)
#define NUM_OF_SFP_PORT            (2)
#define QSFP_EEPROM_I2C_ADDR      (0x50)

enum gpio_reg {
    REG_PORT0_IN,
    REG_PORT1_IN,
    REG_PORT0_OUT,
    REG_PORT1_OUT,
    REG_PORT0_POL,
    REG_PORT1_POL,
    REG_PORT0_DIR,
    REG_PORT1_DIR,
};

struct ing_i2c_board_info {
    int ch;
    int size;
    struct i2c_board_info *board_info;
};

struct i2c_init_data {
    __u16 ch;
    __u16 addr;
    __u8  reg;
    __u8  value; 
};

#endif

static ssize_t show_psu_eeprom(struct device *dev, 
                               struct device_attribute *da, 
                               char *buf);
static struct s9230_psu_data *s9230_psu_update_status(struct device *dev);
static struct s9230_psu_data *s9230_psu_update_eeprom(struct device *dev);
static int s9230_psu_read_block(struct i2c_client *client, 
                                u8 command, 
                                u8 *data,
                                int data_len);


#define DRIVER_NAME "psu"

// Addresses scanned 
static const unsigned short normal_i2c[] = { 0x50, I2C_CLIENT_END };

/* PSU EEPROM SIZE */
#define EEPROM_SZ 256
#define READ_EEPROM 1
#define NREAD_EEPROM 0

static struct i2c_client pca9555_client;

/* pca9555 gpio pin mapping */
#define PSU2_PWROK      0
#define PSU2_PRSNT_L    1
#define PSU2_PWRON_L    2
#define PSU1_PWROK      3
#define PSU1_PRSNT_L    4
#define PSU1_PWRON_L    5
#define TMP_75_INT_L    6

/* Driver Private Data */
struct s9230_psu_data {
    struct mutex    lock;
    char            valid;           /* !=0 if registers are valid */
    unsigned long   last_updated;    /* In jiffies */
    u8  index;                       /* PSU index */
    s32 status;                      /* IO expander value */
    char eeprom[EEPROM_SZ];          /* psu eeprom data */
    char psuABS;                     /* PSU absent */
    char psuPG;                      /* PSU power good */
};

enum psu_index 
{ 
    s9230_psu1, 
    s9230_psu2
};

/*
 * display power good attribute 
 */
static ssize_t 
show_psu_pg(struct device *dev, 
            struct device_attribute *devattr, 
            char *buf)
{
    struct s9230_psu_data *data = s9230_psu_update_status(dev);
    unsigned int value;

    mutex_lock(&data->lock);
    value = data->psuPG;        
    mutex_unlock(&data->lock);

    return sprintf(buf, "%d\n", value);
}

/*
 * display power absent attribute 
 */
static ssize_t 
show_psu_abs(struct device *dev, 
             struct device_attribute *devattr, 
             char *buf)
{
    struct s9230_psu_data *data = s9230_psu_update_status(dev);
    unsigned int value;

    mutex_lock(&data->lock);
    value = data->psuABS;       
    mutex_unlock(&data->lock);

    return sprintf(buf, "%d\n", value);
}


/* 
 * sysfs attributes for psu 
 */
static DEVICE_ATTR(psu_pg, S_IRUGO, show_psu_pg, NULL);
static DEVICE_ATTR(psu_abs, S_IRUGO, show_psu_abs, NULL);
static DEVICE_ATTR(psu_eeprom, S_IRUGO, show_psu_eeprom, NULL);

static struct attribute *s9230_psu_attributes[] = {
    &dev_attr_psu_pg.attr,
    &dev_attr_psu_abs.attr,
    &dev_attr_psu_eeprom.attr,
    NULL
};

/* 
 * display psu eeprom content
 */
static ssize_t 
show_psu_eeprom(struct device *dev, 
                struct device_attribute *da,
                char *buf)
{
    struct s9230_psu_data *data = s9230_psu_update_eeprom(dev);
    
    memcpy(buf, (char *)data->eeprom, EEPROM_SZ);
    return EEPROM_SZ;
}

static const struct attribute_group s9230_psu_group = {
    .attrs = s9230_psu_attributes,
};

/* 
 * check gpio expander is accessible
 */
static int 
pca9555_detect(struct i2c_client *client)
{
    if (i2c_smbus_read_byte_data(client, REG_PORT0_DIR) < 0) {
        return -ENODEV;
    }

    return 0;
}

/* 
 * client address init
 */
static void 
i2c_devices_client_address_init(struct i2c_client *client)
{
    pca9555_client = *client;
    pca9555_client.addr = 0x25;
}

static int 
s9230_psu_probe(struct i2c_client *client,
                const struct i2c_device_id *dev_id)
{
    struct s9230_psu_data *data;
    int status, err;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK)) {
        status = -EIO;
        goto exit;
    }

    data = kzalloc(sizeof(struct s9230_psu_data), GFP_KERNEL);
    if (!data) {
        status = -ENOMEM;
        goto exit;
    }
    memset(data, 0, sizeof(struct s9230_psu_data));
    i2c_set_clientdata(client, data);
    data->valid = 0;
    data->index = dev_id->driver_data;
    mutex_init(&data->lock);

    i2c_devices_client_address_init(client);

    err = pca9555_detect(&pca9555_client);
    if (err) {
        return err; 
    }

    dev_info(&client->dev, "chip found\n");

    /* Register sysfs hooks */
    status = sysfs_create_group(&client->dev.kobj, &s9230_psu_group);
    if (status) {
        goto exit_free;
    }
    
    return 0;

exit_remove:
    sysfs_remove_group(&client->dev.kobj, &s9230_psu_group);
exit_free:
    kfree(data);
exit:
    
    return status;
}

static int 
s9230_psu_remove(struct i2c_client *client)
{
    struct s9230_psu_data *data = i2c_get_clientdata(client);

    sysfs_remove_group(&client->dev.kobj, &s9230_psu_group);
    kfree(data);
    
    return 0;
}


/* 
 * psu eeprom read utility
 */
static int 
s9230_psu_read_block(struct i2c_client *client, 
                     u8 command, 
                     u8 *data,
                     int data_len)
{
    int i=0, ret=0;
    int blk_max = 32; //max block read size

    /* read eeprom, 32 * 8 = 256 bytes */
    for (i=0; i < EEPROM_SZ/blk_max; i++) {
        ret = i2c_smbus_read_i2c_block_data(client, (i*blk_max), blk_max, 
                                            data + (i*blk_max));
        if (ret < 0) {
            return ret;
        }
    }
    return ret;
}

/* 
 * update eeprom content
 */
static struct s9230_psu_data 
*s9230_psu_update_eeprom(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct s9230_psu_data *data = i2c_get_clientdata(client);
    s32 status = 0;
    int psu_pwrok = 0;
    int psu_prsnt_l = 0;
    
    mutex_lock(&data->lock);

    if (time_after(jiffies, data->last_updated + 300 * HZ)
        || !data->valid) {

        /* Read psu status */
        
        status = i2c_smbus_read_word_data(&(pca9555_client), REG_PORT0_IN);
        data->status = status;

        /*read psu status from io expander*/

        if (data->index == s9230_psu1) {
            psu_pwrok = PSU1_PWROK;
            psu_prsnt_l = PSU1_PRSNT_L;
        } else {
            psu_pwrok = PSU2_PWROK;
            psu_prsnt_l = PSU2_PRSNT_L;
        }
        data->psuPG = (status >> psu_pwrok) & 0x1;
        data->psuABS = (status >> psu_prsnt_l) & 0x1; 
        
        /* Read eeprom */
        if (!data->psuABS) {
            //clear local eeprom data
            memset(data->eeprom, 0, EEPROM_SZ);

            //read eeprom
            status = s9230_psu_read_block(client, 0, data->eeprom, 
                                               ARRAY_SIZE(data->eeprom));

            if (status < 0) {
                memset(data->eeprom, 0, EEPROM_SZ);
                dev_err(&client->dev, "Read eeprom failed, status=(%d)\n", status);
            } else {
                data->valid = 1;
            }
        } else {
            memset(data->eeprom, 0, EEPROM_SZ);
        }
        data->last_updated = jiffies;
    }

    mutex_unlock(&data->lock);

    return data;
}

/* 
 * update psu status
 */
static struct s9230_psu_data 
*s9230_psu_update_status(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct s9230_psu_data *data = i2c_get_clientdata(client);
    s32 status = 0;
    int psu_pwrok = 0;
    int psu_prsnt_l = 0;
    
    mutex_lock(&data->lock);

    /* Read psu status */
        
    status = i2c_smbus_read_word_data(&(pca9555_client), REG_PORT0_IN);
    data->status = status;

    /*read psu status from io expander*/

    if (data->index == s9230_psu1) {
        psu_pwrok = PSU1_PWROK;
        psu_prsnt_l = PSU1_PRSNT_L;
    } else {
        psu_pwrok = PSU2_PWROK;
        psu_prsnt_l = PSU2_PRSNT_L;
    }
    data->psuPG = (status >> psu_pwrok) & 0x1;
    data->psuABS = (status >> psu_prsnt_l) & 0x1; 

    mutex_unlock(&data->lock);

    return data;
}

static const struct i2c_device_id s9230_psu_id[] = {
    { "psu1", s9230_psu1 },
    { "psu2", s9230_psu2 },
    {}
};

MODULE_DEVICE_TABLE(i2c, s9230_psu_id);

static struct i2c_driver s9230_psu_driver = {
    .driver = {
        .name     = DRIVER_NAME,
    },
    .probe        = s9230_psu_probe,
    .remove       = s9230_psu_remove,
    .id_table     = s9230_psu_id,
    .address_list = normal_i2c,
};

static int __init s9230_psu_init(void)
{
    return i2c_add_driver(&s9230_psu_driver);
}

static void __exit s9230_psu_exit(void)
{
    i2c_del_driver(&s9230_psu_driver);
}

module_init(s9230_psu_init);
module_exit(s9230_psu_exit);

MODULE_AUTHOR("Jason Tsai <feng.lee.usa@ingrasys.com>");
MODULE_DESCRIPTION("S9230-64X psu driver");
MODULE_LICENSE("GPL");
