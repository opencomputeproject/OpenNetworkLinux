/*
 * An hwmon driver for alphanetworks scg60d0-484t Delta DPS-550AB-47 Power Module
 *
 * Copyright (C) 2021 Alphanetworks Technology Corporation.
 * Fillmore Chen <fillmore_chen@alphanetworks.com>
 *
 * Based on:
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
#include <linux/version.h>

/* PMBus Protocol. */
#define PSU_REG_VOUT_MODE            		0x20
#define PSU_REG_READ_VIN                 	0x88
#define PSU_REG_READ_IIN                 	0x89
#define PSU_REG_READ_VOUT                	0x8B
#define PSU_REG_READ_IOUT                	0x8C
#define PSU_REG_READ_TEMPERATURE_1       	0x8D
#define PSU_REG_READ_FAN_SPEED_1         	0x90
#define PSU_REG_READ_POUT                	0x96
#define PSU_REG_READ_PIN                 	0x97
#define PSU_REG_MFR_ID                   	0x99
#define PSU_REG_MFR_MODEL                	0x9A
#define PSU_REG_MFR_SERIAL               	0x9E
#define PSU_REG_MFR_POUT_MAX             	0xA7

#define I2C_RW_RETRY_COUNT					10
#define I2C_RW_RETRY_INTERVAL				60 /* ms */

static unsigned int debug = 0;
module_param(debug, uint, S_IRUGO);
MODULE_PARM_DESC(debug, "Set DEBUG mode. Default is disabled.");


#define DEBUG_PRINT(fmt, args...)                                        \
    if (debug == 1)                                                      \
		printk (KERN_INFO "[%s,%d]: " fmt "\r\n", __FUNCTION__, __LINE__, ##args)

/* Addresses scanned
 */
static const unsigned short normal_i2c[] = {0x58, 0x59, I2C_CLIENT_END};

/* Each client has this additional data
 */
struct scg60d0_484t_psu_data
{
    struct device *hwmon_dev;
    struct mutex update_lock;
    char valid;                 /* !=0 if registers are valid */
    unsigned long last_updated; /* In jiffies */
    u8 vout_mode;               /* Register value */
    u16 v_in;                   /* Register value */
    u16 v_out;                  /* Register value */
    u16 i_in;                   /* Register value */
    u16 i_out;                  /* Register value */
    u16 p_in;                   /* Register value */
    u16 p_out;                  /* Register value */
    u16 temp1_input;            /* Register value */
    u16 fan_speed;              /* Register value */
    u8 mfr_id[10];              /* Register value */
    u8 mfr_model[20];           /* Register value */
	u8 mfr_serial[26];          /* Register value */
    u16 mfr_pout_max;           /* Register value */
};

static ssize_t show_vout_by_mode(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_linear(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_ascii(struct device *dev, struct device_attribute *da, char *buf);
static struct scg60d0_484t_psu_data *scg60d0_484t_psu_update_device(struct device *dev);

enum scg60d0_484t_psu_sysfs_attributes
{
    PSU_V_IN,
    PSU_V_OUT,
    PSU_I_IN,
    PSU_I_OUT,
    PSU_P_IN,
    PSU_P_OUT_UW,
    PSU_P_OUT,
    PSU_TEMP1_INPUT,
    PSU_FAN1_SPEED,
	PSU_MFR_ID,
	PSU_MODEL_NAME,
	PSU_SERIAL_NUM,
	PSU_MFR_POUT_MAX
};

/* sysfs attributes for hwmon
 */
static SENSOR_DEVICE_ATTR(psu_v_in, S_IRUGO, show_linear, NULL, PSU_V_IN);
static SENSOR_DEVICE_ATTR(psu_v_out, S_IRUGO, show_vout_by_mode, NULL, PSU_V_OUT);
static SENSOR_DEVICE_ATTR(psu_i_in, S_IRUGO, show_linear, NULL, PSU_I_IN);
static SENSOR_DEVICE_ATTR(psu_i_out, S_IRUGO, show_linear, NULL, PSU_I_OUT);
static SENSOR_DEVICE_ATTR(psu_p_in, S_IRUGO, show_linear, NULL, PSU_P_IN);
static SENSOR_DEVICE_ATTR(psu_p_out, S_IRUGO, show_linear, NULL, PSU_P_OUT);
static SENSOR_DEVICE_ATTR(psu_temp1_input, S_IRUGO, show_linear, NULL, PSU_TEMP1_INPUT);
static SENSOR_DEVICE_ATTR(psu_fan1_speed_rpm, S_IRUGO, show_linear, NULL, PSU_FAN1_SPEED);
static SENSOR_DEVICE_ATTR(psu_mfr_id, S_IRUGO, show_ascii, NULL, PSU_MFR_ID);
static SENSOR_DEVICE_ATTR(psu_mfr_model, S_IRUGO, show_ascii, NULL, PSU_MODEL_NAME);
static SENSOR_DEVICE_ATTR(psu_mfr_serial, S_IRUGO, show_ascii, NULL, PSU_SERIAL_NUM);
static SENSOR_DEVICE_ATTR(psu_mfr_pout_max, S_IRUGO, show_linear, NULL, PSU_MFR_POUT_MAX);

/*Duplicate nodes for lm-sensors.*/
static SENSOR_DEVICE_ATTR(in3_input, S_IRUGO, show_vout_by_mode, NULL, PSU_V_OUT);
static SENSOR_DEVICE_ATTR(curr2_input, S_IRUGO, show_linear, NULL, PSU_I_OUT);
static SENSOR_DEVICE_ATTR(power2_input, S_IRUGO, show_linear, NULL, PSU_P_OUT_UW);
static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, show_linear, NULL, PSU_TEMP1_INPUT);
static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, show_linear, NULL, PSU_FAN1_SPEED);

static struct attribute *scg60d0_484t_psu_attributes[] = {
    &sensor_dev_attr_psu_v_in.dev_attr.attr,
    &sensor_dev_attr_psu_v_out.dev_attr.attr,
    &sensor_dev_attr_psu_i_in.dev_attr.attr,
    &sensor_dev_attr_psu_i_out.dev_attr.attr,
    &sensor_dev_attr_psu_p_in.dev_attr.attr,
    &sensor_dev_attr_psu_p_out.dev_attr.attr,
    &sensor_dev_attr_psu_temp1_input.dev_attr.attr,
    &sensor_dev_attr_psu_fan1_speed_rpm.dev_attr.attr,
    &sensor_dev_attr_psu_mfr_id.dev_attr.attr,
    &sensor_dev_attr_psu_mfr_model.dev_attr.attr,
    &sensor_dev_attr_psu_mfr_serial.dev_attr.attr,
    &sensor_dev_attr_psu_mfr_pout_max.dev_attr.attr,
    /*Duplicate nodes for lm-sensors.*/
    &sensor_dev_attr_curr2_input.dev_attr.attr,
    &sensor_dev_attr_in3_input.dev_attr.attr,
    &sensor_dev_attr_power2_input.dev_attr.attr,
    &sensor_dev_attr_temp1_input.dev_attr.attr,
    &sensor_dev_attr_fan1_input.dev_attr.attr,
    NULL};

static int two_complement_to_int(u16 data, u8 valid_bit, int mask)
{
    u16 valid_data = data & mask;
    bool is_negative = valid_data >> (valid_bit - 1);

    return is_negative ? (-(((~valid_data) & mask) + 1)) : valid_data;
}

static ssize_t show_linear(struct device *dev, struct device_attribute *da,
                           char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct scg60d0_484t_psu_data *data = scg60d0_484t_psu_update_device(dev);

    u16 value = 0;
    int exponent, mantissa;
    int multiplier = 1000;

    switch (attr->index)
    {
    case PSU_V_IN:
        value = data->v_in;
        break;
    case PSU_I_IN:
        value = data->i_in;
        break;
    case PSU_I_OUT:
        value = data->i_out;
        break;
    case PSU_P_IN:
        value = data->p_in;
        break;
    case PSU_P_OUT:
        value = data->p_out;
        break;
    case PSU_P_OUT_UW:
        value = data->p_out;
        multiplier = 1000000;
        break;
    case PSU_TEMP1_INPUT:
        value = data->temp1_input;
        break;
    case PSU_FAN1_SPEED:
        value = data->fan_speed;
        multiplier = 1;
        break;
    case PSU_MFR_POUT_MAX:
        value = data->mfr_pout_max;
        break;
    }

    exponent = two_complement_to_int(value >> 11, 5, 0x1f);
    mantissa = two_complement_to_int(value & 0x7ff, 11, 0x7ff);
    return (exponent >= 0) ? sprintf(buf, "%d\n", (mantissa << exponent) * multiplier) : sprintf(buf, "%d\n", (mantissa * multiplier) / (1 << -exponent));
}

static ssize_t show_ascii(struct device *dev, struct device_attribute *da,
                          char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct scg60d0_484t_psu_data *data = scg60d0_484t_psu_update_device(dev);
    u8 *ptr = NULL;

    switch (attr->index)
    {
    case PSU_MFR_ID: /* psu_mfr_id */
        ptr = data->mfr_id;
        break;
    case PSU_MODEL_NAME: /* psu_mfr_model */
        ptr = data->mfr_model;
        break;
	case PSU_SERIAL_NUM: /* psu_mfr_id */
        ptr = data->mfr_serial;
        break;
    default:
        return 0;
    }

    return sprintf(buf, "%s\n", ptr);
}

static ssize_t show_vout_by_mode(struct device *dev, struct device_attribute *da,
                                 char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct scg60d0_484t_psu_data *data = scg60d0_484t_psu_update_device(dev);
    int exponent, mantissa;
    int multiplier = 1000;

    if (!data->valid)
    {
        return 0;
    }

    exponent = two_complement_to_int(data->vout_mode, 5, 0x1f);
    switch (attr->index)
    {
    case PSU_V_OUT:
        mantissa = data->v_out;
        break;
    default:
        return 0;
    }

    return (exponent > 0) ? sprintf(buf, "%d\n", (mantissa << exponent) * multiplier) : sprintf(buf, "%d\n", (mantissa * multiplier) / (1 << -exponent));
}

static const struct attribute_group scg60d0_484t_psu_group = {
    .attrs = scg60d0_484t_psu_attributes,
};

static int scg60d0_484t_psu_probe(struct i2c_client *client,
                         const struct i2c_device_id *dev_id)
{
    struct scg60d0_484t_psu_data *data;
    int status;

    if (!i2c_check_functionality(client->adapter,
                                 I2C_FUNC_SMBUS_BYTE_DATA |
                                 I2C_FUNC_SMBUS_WORD_DATA |
                                 I2C_FUNC_SMBUS_I2C_BLOCK))
    {
        status = -EIO;
        goto exit;
    }

    data = kzalloc(sizeof(struct scg60d0_484t_psu_data), GFP_KERNEL);
    if (!data)
    {
        status = -ENOMEM;
        goto exit;
    }

    i2c_set_clientdata(client, data);
    mutex_init(&data->update_lock);
    dev_info(&client->dev, "chip found\n");

    /* Register sysfs hooks */
    status = sysfs_create_group(&client->dev.kobj, &scg60d0_484t_psu_group);
    if (status)
    {
        goto exit_free;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
    data->hwmon_dev = hwmon_device_register_with_info(&client->dev, "scg60d0_psu",
                                                      NULL, NULL, NULL);
#else
    data->hwmon_dev = hwmon_device_register(&client->dev);
#endif

    if (IS_ERR(data->hwmon_dev))
    {
        status = PTR_ERR(data->hwmon_dev);
        goto exit_remove;
    }

    dev_info(&client->dev, "%s: psu '%s'\n",
             dev_name(data->hwmon_dev), client->name);

    return 0;

exit_remove:
    sysfs_remove_group(&client->dev.kobj, &scg60d0_484t_psu_group);
exit_free:
    kfree(data);
exit:

    return status;
}

static int scg60d0_484t_psu_remove(struct i2c_client *client)
{
    struct scg60d0_484t_psu_data *data = i2c_get_clientdata(client);

    hwmon_device_unregister(data->hwmon_dev);
    sysfs_remove_group(&client->dev.kobj, &scg60d0_484t_psu_group);
    kfree(data);

    return 0;
}

static const struct i2c_device_id scg60d0_484t_psu_id[] = {
    {"scg60d0_psu", 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, scg60d0_484t_psu_id);

static struct i2c_driver scg60d0_484t_psu_driver = {
    .class = I2C_CLASS_HWMON,
    .driver = {
        .name = "scg60d0_psu",
    },
    .probe = scg60d0_484t_psu_probe,
    .remove = scg60d0_484t_psu_remove,
    .id_table = scg60d0_484t_psu_id,
    .address_list = normal_i2c,
};

static int scg60d0_484t_psu_read_byte(struct i2c_client *client, u8 reg)
{
    return i2c_smbus_read_byte_data(client, reg);
}

static int scg60d0_484t_psu_read_word(struct i2c_client *client, u8 reg)
{
    return i2c_smbus_read_word_data(client, reg);
}

static int scg60d0_484t_psu_read_block(struct i2c_client *client, u8 command, u8 *data, int data_len)
{
	int status = 0, retry = I2C_RW_RETRY_COUNT;

	while (retry) {
		status = i2c_smbus_read_i2c_block_data(client, command, data_len, data);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

	if (debug)
	{
		DEBUG_PRINT("scg60d0_484t_psu_read_block: command:0x%x, data_len:0x%x, status:0x%x", command, data_len, status);
		if (status > 0)
		{
		    int i;
			DEBUG_PRINT("DATA:");
	        for (i =0 ; i < data_len ; i++ )
	        {
				printk (KERN_INFO "data[%d]: 0x%x ", i, data[i]);
			}
			printk (KERN_INFO "\r\n ");
		}
	}

    return status;
}

static int scg60d0_484t_psu_read_block_data(struct i2c_client *client, u8 command, u8 *data, int data_length)
{
    int status = -EIO;
    int length;
    u8 buffer[128] = {0}, *ptr = buffer;

    DEBUG_PRINT("scg60d0_484t_psu_read_block_data: command:0x%x, data_length:0x%x", command, data_length);

    status = scg60d0_484t_psu_read_byte(client, command);
    if (status < 0)
    {
        dev_dbg(&client->dev, "Unable to get data from offset 0x%02X\r\n", command);
        status = -EIO;
        goto EXIT_READ_BLOCK_DATA;
    }
  
    status = (status & 0xFF) + 1;
    if ( status > 128)
    {
        dev_dbg(&client->dev, "Unable to get big data from offset 0x%02X\r\n", command);
        status = -EINVAL;
        goto EXIT_READ_BLOCK_DATA;
    }
    
    length = status;
    status = scg60d0_484t_psu_read_block(client, command, buffer, length);
    if (unlikely(status < 0))
        goto EXIT_READ_BLOCK_DATA;
    if (unlikely(status != length)) {
        status = -EIO;
        goto EXIT_READ_BLOCK_DATA;
    }

    /* The first byte is the count byte of string. */   
    ptr++;
    status--;

    length=status>(data_length-1)?(data_length-1):status;
    memcpy(data, ptr, length);

	if (debug)
	{
		DEBUG_PRINT("scg60d0_484t_psu_read_block_data: command:0x%x, data_length:0x%x, status:0x%x, length:0x%x", command, data_length, status, length);
		if (status > 0)
		{
		    int i;
			DEBUG_PRINT("BLOCK_DATA:");
	        for (i = 0 ; i < data_length ; i++ )
	        {
				printk (KERN_INFO "data[%d]: 0x%x ", i, data[i]);
			}
			printk (KERN_INFO "\r\n ");
		}
	}
    data[length] = 0;

EXIT_READ_BLOCK_DATA:
    
    return status;
}

struct reg_data_byte
{
    u8 reg;
    u8 *value;
};

struct reg_data_word
{
    u8 reg;
    u16 *value;
};

static struct scg60d0_484t_psu_data *scg60d0_484t_psu_update_device(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct scg60d0_484t_psu_data *data = i2c_get_clientdata(client);

    mutex_lock(&data->update_lock);

    if (time_after(jiffies, data->last_updated + HZ + HZ / 2) || !data->valid)
    {
        int i, status;
        struct reg_data_byte regs_byte[] = {{PSU_REG_VOUT_MODE, &data->vout_mode}};
        struct reg_data_word regs_word[] = {{PSU_REG_READ_VIN, &data->v_in},
                                            {PSU_REG_READ_VOUT, &data->v_out},
                                            {PSU_REG_READ_IIN, &data->i_in},
                                            {PSU_REG_READ_IOUT, &data->i_out},
                                            {PSU_REG_READ_PIN, &data->p_in},
                                            {PSU_REG_READ_POUT, &data->p_out},
                                            {PSU_REG_READ_TEMPERATURE_1, &data->temp1_input},
                                            {PSU_REG_READ_FAN_SPEED_1, &data->fan_speed},
                                            {PSU_REG_MFR_POUT_MAX, &data->mfr_pout_max}};

        dev_dbg(&client->dev, "Starting scg60d0_484t_psu update\n");

        /* Read byte data */
        for (i = 0; i < ARRAY_SIZE(regs_byte); i++)
        {
            status = scg60d0_484t_psu_read_byte(client, regs_byte[i].reg);

            if (status < 0)
            {
                dev_dbg(&client->dev, "reg %d, err %d\n",
                        regs_byte[i].reg, status);
                *(regs_byte[i].value) = 0;
            }
            else
            {
                *(regs_byte[i].value) = status;
            }
        }

        /* Read word data */
        for (i = 0; i < ARRAY_SIZE(regs_word); i++)
        {
            status = scg60d0_484t_psu_read_word(client, regs_word[i].reg);

            if (status < 0)
            {
                dev_dbg(&client->dev, "reg %d, err %d\n",
                        regs_word[i].reg, status);
                *(regs_word[i].value) = 0;
            }
            else
            {
                *(regs_word[i].value) = status;
            }
        }

        /* Read mfr_id */
		status = scg60d0_484t_psu_read_block_data(client, PSU_REG_MFR_ID, data->mfr_id,
										 ARRAY_SIZE(data->mfr_id));
		if (status < 0) {
			dev_dbg(&client->dev, "reg %d, err %d\n", PSU_REG_MFR_ID, status);
			goto exit;
		}
		/* Read mfr_model */		
		status = scg60d0_484t_psu_read_block_data(client, PSU_REG_MFR_MODEL, data->mfr_model,
										 ARRAY_SIZE(data->mfr_model));
		if (status < 0) {
			dev_dbg(&client->dev, "reg %d, err %d\n", PSU_REG_MFR_MODEL, status);
			goto exit;
		}
		/* Read mfr_serial */
		status = scg60d0_484t_psu_read_block_data(client, PSU_REG_MFR_SERIAL, data->mfr_serial,
										 ARRAY_SIZE(data->mfr_serial));
		if (status < 0) {
			dev_dbg(&client->dev, "reg %d, err %d\n", PSU_REG_MFR_SERIAL, status);
			goto exit;
		}

        data->last_updated = jiffies;
        data->valid = 1;
    }

exit:
    mutex_unlock(&data->update_lock);

    return data;
}

module_i2c_driver(scg60d0_484t_psu_driver);

MODULE_AUTHOR("Alpha-SID6");
MODULE_DESCRIPTION("Alphanetworks scg60d0-484t PSU driver");
MODULE_LICENSE("GPL");
