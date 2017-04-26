/*
 * An hwmon driver for delta ag9032v1 qsfp
 *
 * Copyright (C) 2017 Delta Networks, Inc.
 *
 * Aries Lin <aries.lin@deltaww.com> 
 *
 * Based on ad7414.c
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
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>

#define SWPLD_REG 	  0x31
#define SWPLD_SFP_MUX_REG 0x20
#define SFP_PRESENCE_1 	  0x38
#define SFP_PRESENCE_2 	  0x39
#define SFP_PRESENCE_3 	  0x3A
#define SFP_PRESENCE_4 	  0x3B

#define SFP_LP_MODE_1  	  0x34
#define SFP_LP_MODE_2  	  0x35
#define SFP_LP_MODE_3  	  0x36
#define SFP_LP_MODE_4  	  0x37

#define SFP_RESET_1    	  0x3C
#define SFP_RESET_2	  0x3D
#define SFP_RESET_3	  0x3E
#define SFP_RESET_4	  0x3F

/* Check cpld read results */
#define VALIDATED_READ(_buf, _rv, _read, _invert)		\
	do {							\
		_rv = _read;					\
		if (_rv < 0) {					\
			return sprintf(_buf, "READ ERROR\n");	\
		}						\
		if (_invert) {					\
			_rv = ~_rv;				\
		}						\
		_rv &= 0xFF;					\
	} while(0)						\

u8 sfp_port_data = 0x00;

static const u8 cpld_to_port_table[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10,
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20,
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30,
	0x31, 0x32 };

/* Addresses scanned */
static const unsigned short normal_i2c[] = { 0x50, I2C_CLIENT_END };

/* Each client has this additional data */
struct ag9032v1_sfp_data {
	struct device	*hwmon_dev;
	struct mutex	update_lock;
	char		valid;
	unsigned long	last_updated;
	int		port;
	char		eeprom[256];
};

static ssize_t for_eeprom(struct device *dev, struct device_attribute *dev_attr,
								char *buf);
static int ag9032v1_sfp_read_block(struct i2c_client *client, u8 command,
						u8 *data, int data_len);
static struct ag9032v1_sfp_data *ag9032v1_sfp_update_device( \
					 		struct device *dev);
static ssize_t set_w_port_data(struct device *dev, struct device_attribute \
                                *dev_attr, const char *buf, size_t count);
static ssize_t for_r_port_data(struct device *dev, struct device_attribute \
							*dev_attr, char *buf);
static ssize_t for_status(struct device *dev, struct device_attribute \
							*dev_attr, char *buf);
static ssize_t set_w_lp_mode_data(struct device *dev, struct device_attribute \
				*dev_attr, const char *buf, size_t count);
static ssize_t set_w_reset_data(struct device *dev, struct device_attribute \
				*dev_attr, const char *buf, size_t count);
extern int i2c_cpld_write(int bus, unsigned short cpld_addr, u8 reg, u8 value);

extern int i2c_cpld_read(int bus, unsigned short cpld_addr, u8 reg);

enum ag9032v1_sfp_sysfs_attributes {
	SFP_EEPROM,
	SFP_SELECT_PORT,
	SFP_IS_PRESENT,
	SFP_IS_PRESENT_ALL,
	SFP_LP_MODE,
	SFP_RESET
};

static ssize_t set_w_port_data(struct device *dev, struct device_attribute \
				*dev_attr, const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
	long data;
	int error;
	if (attr->index == SFP_SELECT_PORT) {
		error = kstrtol(buf, 16, &data);
		if (error)
			return error; 

		sfp_port_data = data;
	}
	return count;
}

static ssize_t for_r_port_data(struct device *dev, struct device_attribute \
							*dev_attr, char *buf)
{
	return sprintf(buf, "0x%02X\n", sfp_port_data);
}

static ssize_t set_w_lp_mode_data(struct device *dev, struct device_attribute \
				*dev_attr, const char *buf, size_t count)
{
	long data;
	int error;
	u8 port_t = 0;
	int bit_t = 0x00;
	int values = 0x00;

	error = kstrtol(buf, 10, &data);
	if (error)
		return error;
	for (port_t = 0; port_t < 32; port_t++) {
		/* port number range is 0 - 31 */
		if (cpld_to_port_table[port_t] == sfp_port_data)
			break;
	}

	if (port_t < 8) { 			 /* Port 0-7 */
	        values = i2c_cpld_read(6, SWPLD_REG, 
							SFP_LP_MODE_1);
		if (data == 0) {
			bit_t = ~(1 << (7 - (port_t % 8)));
			values = values & bit_t;
		} else if (data == 1) {
			bit_t = 1 << (7 - (port_t % 8));
			values = values | bit_t;
		} else {
			return -EINVAL;
		}
		i2c_cpld_write(6, SWPLD_REG,
						SFP_LP_MODE_1,  values);
	} else if (port_t > 7 && port_t < 16) {  /* Port 7-15 */
	        values = i2c_cpld_read(6, SWPLD_REG, 
						SFP_LP_MODE_2);
		if (data == 0) {
			bit_t = ~(1 << (7 - (port_t % 8)));
			values = values & bit_t;
		} else if (data == 1) {
			bit_t = 1 << (7 - (port_t % 8));
			values = values | bit_t;
		} else {
			return -EINVAL;
		}
		i2c_cpld_write(6, SWPLD_REG,
						SFP_LP_MODE_2,  values);
	} else if (port_t > 15 && port_t < 24) { /* Port 16-23 */
	        values = i2c_cpld_read(6, SWPLD_REG, 
						SFP_LP_MODE_3);
		if (data == 0) {
			bit_t = ~(1 << (7 - (port_t % 8)));
			values = values & bit_t;
		} else if (data == 1) {
			bit_t = 1 << (7 - (port_t % 8));
			values = values | bit_t;
		} else {
			return -EINVAL;
		}
		i2c_cpld_write(6, SWPLD_REG,
						SFP_LP_MODE_3,  values);
	} else if (port_t > 23 && port_t < 32) { /* Port 24-31 */
	        values = i2c_cpld_read(6, SWPLD_REG, 
						SFP_LP_MODE_4);
		if (data == 0) {
			bit_t = ~(1 << (7 - (port_t % 8)));
			values = values & bit_t;
		} else if (data == 1) {
			bit_t = 1 << (7 - (port_t % 8));
			values = values | bit_t;
		} else {
			return -EINVAL;
		}
		i2c_cpld_write(6, SWPLD_REG,
						SFP_LP_MODE_4,  values);
	}
	
	return count;
}
static ssize_t set_w_reset_data(struct device *dev, struct device_attribute \
				*dev_attr, const char *buf, size_t count)
{
	long data;
	int error;
	u8 port_t = 0;
	int bit_t = 0x00;
	int values = 0x00;

	error = kstrtol(buf, 10, &data);
	if (error)
		return error;
	for (port_t = 0; port_t < 32; port_t++) {
		/* port number range is 0 - 31 */
		if (cpld_to_port_table[port_t] == sfp_port_data)
			break;
	}

	if (port_t < 8) { 			 /* Port 0-7 */
	        values = i2c_cpld_read(6, SWPLD_REG, 
							SFP_RESET_1);
		if (data == 0) {
			bit_t = ~(1 << (7 - (port_t % 8)));
			values = values & bit_t;
		} else if (data == 1) {
			bit_t = 1 << (7 - (port_t % 8));
			values = values | bit_t;
		} else {
			return -EINVAL;
		}
		i2c_cpld_write(6, SWPLD_REG,
						SFP_RESET_1,  values);
	} else if (port_t > 7 && port_t < 16) {  /* Port 7-15 */
	        values = i2c_cpld_read(6, SWPLD_REG, 
						SFP_RESET_2);
		if (data == 0) {
			bit_t = ~(1 << (7 - (port_t % 8)));
			values = values & bit_t;
		} else if (data == 1) {
			bit_t = 1 << (7 - (port_t % 8));
			values = values | bit_t;
		} else {
			return -EINVAL;
		}
		i2c_cpld_write(6, SWPLD_REG,
						SFP_RESET_2,  values);
	} else if (port_t > 15 && port_t < 24) { /* Port 16-23 */
	        values = i2c_cpld_read(6, SWPLD_REG, 
						SFP_RESET_3);
		if (data == 0) {
			bit_t = ~(1 << (7 - (port_t % 8)));
			values = values & bit_t;
		} else if (data == 1) {
			bit_t = 1 << (7 - (port_t % 8));
			values = values | bit_t;
		} else {
			return -EINVAL;
		}
		i2c_cpld_write(6, SWPLD_REG,
						SFP_RESET_3,  values);
	} else if (port_t > 23 && port_t < 32) { /* Port 24-31 */
	        values = i2c_cpld_read(6, SWPLD_REG, 
						SFP_RESET_4);
		if (data == 0) {
			bit_t = ~(1 << (7 - (port_t % 8)));
			values = values & bit_t;
		} else if (data == 1) {
			bit_t = 1 << (7 - (port_t % 8));
			values = values | bit_t;
		} else {
			return -EINVAL;
		}
		i2c_cpld_write(6, SWPLD_REG,
						SFP_RESET_4,  values);
	}
	
	return count;
}

static ssize_t for_status(struct device *dev, struct device_attribute \
							*dev_attr, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
	u8 port_t = 0;
	int values[4] = {'\0'};
	int bit_t = 0x00;

	if (attr->index == SFP_IS_PRESENT) {
		for (port_t = 0; port_t < 32; port_t++) {
			/* port number range is 0 - 31 */
			if (cpld_to_port_table[port_t] == sfp_port_data)
				break;
		}

		if (port_t < 8) { 			 /* Port 0-7 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_PRESENCE_1), 0);
		} else if (port_t > 7 && port_t < 16) {  /* Port 7-15 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_PRESENCE_2), 0);
		} else if (port_t > 15 && port_t < 24) { /* Port 16-23 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_PRESENCE_3), 0);
		} else if (port_t > 23 && port_t < 32) { /* Port 24-31 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_PRESENCE_4), 0);
		}
	
		/* SWPLD QSFP module respond */
		bit_t = 1 << (7 - (port_t % 8));
		values[0] = values[0] & bit_t;
		values[0] = values[0]/bit_t;

		/* sfp_is_present value
		 * return 1 is module NOT present
		 * return 0 is module present
		 */
		return sprintf(buf, "%d\n", values[0]); 
	}
	if (attr->index == SFP_IS_PRESENT_ALL) {

		/*
		 * Report the SFP ALL PRESENCE status
		 * This data information form CPLD.
		 */

		/* SFP_PRESENT Ports 1-8 */
		VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG,
 							SFP_PRESENCE_1), 0);
		/* SFP_PRESENT Ports 9-16 */
		VALIDATED_READ(buf, values[1], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_PRESENCE_2), 0);
		/* SFP_PRESENT Ports 17-24 */
		VALIDATED_READ(buf, values[2], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_PRESENCE_3), 0);
		/* SFP_PRESENT Ports 25-32 */
		VALIDATED_READ(buf, values[3], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_PRESENCE_4), 0);
		/* sfp_is_present_all value
		 * return 0 is module NOT present
		 * return 1 is module present
		 */
		return sprintf(buf, "%02X %02X %02X %02X\n",
							values[0], values[1], 
							values[2], values[3]); 
	}
	if (attr->index == SFP_LP_MODE) {
		for (port_t = 0; port_t < 32; port_t++) {
			/* port number range is 0 - 31 */
			if (cpld_to_port_table[port_t] == sfp_port_data)
				break;
		}

		if (port_t < 8) { 			 /* Port 0-7 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_LP_MODE_1), 0);
		} else if (port_t > 7 && port_t < 16) {  /* Port 7-15 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_LP_MODE_2), 0);
		} else if (port_t > 15 && port_t < 24) { /* Port 16-23 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_LP_MODE_3), 0);
		} else if (port_t > 23 && port_t < 32) { /* Port 24-31 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_LP_MODE_4), 0);
		}
	
		/* SWPLD QSFP module respond */
		bit_t = 1 << (7 - (port_t % 8));
		values[0] = values[0] & bit_t;
		values[0] = values[0] / bit_t;

		/* sfp_lp_mode value
		 * return 0 is module NOT in LP mode
		 * return 1 is module in LP mode
		 */
		return sprintf(buf, "%d\n", values[0]);
	}
	if (attr->index == SFP_RESET) {
		for (port_t = 0; port_t < 32; port_t++) {
			/* port number range is 0 - 31 */
			if (cpld_to_port_table[port_t] == sfp_port_data)
				break;
		}

		if (port_t < 8) { 			 /* Port 0-7 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_RESET_1), 0);
		} else if (port_t > 7 && port_t < 16) {  /* Port 7-15 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_RESET_2), 0);
		} else if (port_t > 15 && port_t < 24) { /* Port 16-23 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_RESET_3), 0);
		} else if (port_t > 23 && port_t < 32) { /* Port 24-31 */
			VALIDATED_READ(buf, values[0], 
		      i2c_cpld_read(6, SWPLD_REG, 
							SFP_RESET_4), 0);
		}
	
		/* SWPLD QSFP module respond */
		bit_t = 1 << (7 - (port_t % 8));
		values[0] = values[0] & bit_t;
		values[0] = values[0] / bit_t;

		/* sfp_reset value
		 * return 0 is module NOT in Reset
		 * return 1 is module in Reset
		 */
		return sprintf(buf, "%d\n", values[0]);
	}
	
	return (attr->index);
}

static ssize_t for_eeprom(struct device *dev, struct device_attribute *dev_attr,
								char *buf)
{
	struct ag9032v1_sfp_data *data = ag9032v1_sfp_update_device(dev);
	if (!data->valid) {
		return 0;
	}
	memcpy(buf, data->eeprom, sizeof(data->eeprom));
	return sizeof(data->eeprom);
}

static int ag9032v1_sfp_read_block(struct i2c_client *client, u8 command, \
							u8 *data, int data_len)
{
	int result = i2c_smbus_read_i2c_block_data(client, command, data_len, 
									data);
	if (unlikely(result < 0))
		goto abort;
	if (unlikely(result != data_len)) {
		result = -EIO;
		goto abort;
	}
	result = 0;
abort:
	return result;
}

static struct ag9032v1_sfp_data *ag9032v1_sfp_update_device( \
							struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ag9032v1_sfp_data *data = i2c_get_clientdata(client);
	u8 port_t = 0;
	
	/* Use SWPLD to change sfp port offset */
	for (port_t = 0; port_t < 32; port_t++) {
		/* port number range is 0 - 31 */
		if (cpld_to_port_table[port_t] == sfp_port_data)
			break;
	}
	if (port_t < 8) {                        /* Port 0-7 */
    		i2c_cpld_write(6, SWPLD_REG,
                                	SWPLD_SFP_MUX_REG, sfp_port_data);
	} else if (port_t > 7 && port_t < 16) {   /* Port 7-15 */
		i2c_cpld_write(6, SWPLD_REG,
                                       	SWPLD_SFP_MUX_REG, sfp_port_data);
	} else if (port_t > 15 && port_t < 24) { /* Port 16-23 */
		i2c_cpld_write(6, SWPLD_REG,
                                         SWPLD_SFP_MUX_REG, sfp_port_data);
	} else if (port_t > 23 && port_t < 32) { /* Port 24-31 */
		i2c_cpld_write(6, SWPLD_REG,
					 SWPLD_SFP_MUX_REG, sfp_port_data);
	}
	
	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated) || !data->valid) {
		int status = -1;
		int i = 0;
		data->valid = 0;

		memset(data->eeprom, 0, sizeof(data->eeprom));

		for (i = 0; i < sizeof(data->eeprom)/	\
					I2C_SMBUS_BLOCK_MAX; i++) {
			status = ag9032v1_sfp_read_block(
				client, 
				i*I2C_SMBUS_BLOCK_MAX, 
				data->eeprom + (i*I2C_SMBUS_BLOCK_MAX),
				I2C_SMBUS_BLOCK_MAX
				);
			if (status < 0) {
				printk(KERN_INFO "status = %d\n", status);
				dev_dbg(&client->dev, 
			"unable to read eeprom from port(%d)\n", data->port);

				goto exit;
			}
		}
		data->last_updated = jiffies;
                data->valid = 1;	
	}

exit:
	mutex_unlock(&data->update_lock);
	return data;
}

/* sysfs attributes for hwmon */
static SENSOR_DEVICE_ATTR(sfp_eeprom,         S_IRUGO, for_eeprom, NULL,
							SFP_EEPROM);
static SENSOR_DEVICE_ATTR(sfp_select_port,    S_IWUSR | S_IRUGO, 
		for_r_port_data, set_w_port_data,  	SFP_SELECT_PORT);
static SENSOR_DEVICE_ATTR(sfp_is_present,     S_IRUGO, for_status, NULL,
							SFP_IS_PRESENT);
static SENSOR_DEVICE_ATTR(sfp_is_present_all, S_IRUGO, for_status, NULL,
							SFP_IS_PRESENT_ALL);
static SENSOR_DEVICE_ATTR(sfp_lp_mode, 	      S_IWUSR | S_IRUGO, 
		for_status, set_w_lp_mode_data, 	SFP_LP_MODE);
static SENSOR_DEVICE_ATTR(sfp_reset,	      S_IWUSR | S_IRUGO, 
		for_status, set_w_reset_data,		SFP_RESET);

static struct attribute *ag9032v1_sfp_attributes[] = {
	&sensor_dev_attr_sfp_eeprom.dev_attr.attr,
	&sensor_dev_attr_sfp_select_port.dev_attr.attr,
	&sensor_dev_attr_sfp_is_present.dev_attr.attr,
	&sensor_dev_attr_sfp_is_present_all.dev_attr.attr,
	&sensor_dev_attr_sfp_lp_mode.dev_attr.attr,
	&sensor_dev_attr_sfp_reset.dev_attr.attr,
	NULL
};

static const struct attribute_group ag9032v1_sfp_group = {
	.attrs = ag9032v1_sfp_attributes,
};

static int ag9032v1_sfp_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{
	struct ag9032v1_sfp_data *data;
	int status;
	
	if (!i2c_check_functionality(client->adapter,
						I2C_FUNC_SMBUS_I2C_BLOCK)) {
		status = -EIO;
		goto exit;
	}
	
	data = kzalloc(sizeof(struct ag9032v1_sfp_data), GFP_KERNEL);
	if (!data) {
		status = -ENOMEM;
		goto exit;
	}

	mutex_init(&data->update_lock);
	data->port = id->driver_data;
	i2c_set_clientdata(client, data);

	dev_info(&client->dev, "chip found\n");

	status = sysfs_create_group(&client->dev.kobj, &ag9032v1_sfp_group);
        if (status)
                goto exit_sysfs_create_group;

	data->hwmon_dev = hwmon_device_register(&client->dev);
        if (IS_ERR(data->hwmon_dev)) {
                status = PTR_ERR(data->hwmon_dev);
                goto exit_hwmon_device_register;
        }

	dev_info(&client->dev, "%s: sfp '%s'\n", dev_name(data->hwmon_dev),
							client->name);

	return 0;

exit_hwmon_device_register:
	sysfs_remove_group(&client->dev.kobj, &ag9032v1_sfp_group);
exit_sysfs_create_group:
	kfree(data);
exit:
	return status;
}

static int ag9032v1_sfp_remove(struct i2c_client *client)
{
	struct ag9032v1_sfp_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &ag9032v1_sfp_group);
	kfree(data);
	return 0;
}

enum id_name {
	dni_ag9032v1_sfp
};

static const struct i2c_device_id ag9032v1_sfp_id[] = {
	{ "dni_ag9032v1_sfp", dni_ag9032v1_sfp },
	{}
};
MODULE_DEVICE_TABLE(i2c, ag9032v1_sfp_id);


static struct i2c_driver ag9032v1_sfp_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "dni_ag9032v1_sfp",
	},
	.probe		= ag9032v1_sfp_probe,
	.remove		= ag9032v1_sfp_remove,
	.id_table	= ag9032v1_sfp_id,
	.address_list	= normal_i2c,
};

static int __init ag9032v1_sfp_init(void)
{
        return i2c_add_driver(&ag9032v1_sfp_driver);
}

static void __exit ag9032v1_sfp_exit(void)
{
        i2c_del_driver(&ag9032v1_sfp_driver);
}

MODULE_AUTHOR("Aries Lin <aries.lin@deltaww.com>");
MODULE_DESCRIPTION("AG9032v1 SFP Driver");
MODULE_LICENSE("GPL");

module_init(ag9032v1_sfp_init);
module_exit(ag9032v1_sfp_exit);

