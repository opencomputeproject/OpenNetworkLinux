/*
 * An hwmon driver for agema ag5648 qsfp
 *
 * Copyright (C) 2017 Delta Networks, Inc.
 *
 * DNI <DNIsales@delta.com.tw> 
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

#define I2C_BUS_2 2
#define MASTER_CPLD 0x35
#define SLAVE_CPLD 0x39

#define SFP_PRESENCE_1 0x08
#define SFP_PRESENCE_2 0x09
#define SFP_PRESENCE_3 0x0a
#define SFP_PRESENCE_4 0x0b
#define SFP_PRESENCE_5 0x0c
#define SFP_PRESENCE_6 0x08
#define SFP_PRESENCE_7 0x09
#define QSFP_PRESENCE_1 0x12
#define QSFP_LP_MODE_1 0x11
#define QSFP_RESET_1 0x13

#define DEFAULT_DISABLE 0x00
#define QSFP_SFP_SEL_I2C_MUX 0x18


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


long sfp_port_data = 0;

static const u8 cpld_to_port_table[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 
  0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36 };

/* Addresses scanned */
static const unsigned short normal_i2c[] = { 0x50, I2C_CLIENT_END };

/* Each client has this additional data */
struct ag5648_sfp_data 
{
  struct device	*hwmon_dev;
  struct mutex update_lock;
  char valid;
  unsigned long last_updated;
  int port;
  char eeprom[256];
};

static ssize_t for_eeprom(struct device *dev, struct device_attribute *dev_attr,char *buf);
static int ag5648_sfp_read_block(struct i2c_client *client, u8 command,u8 *data, int data_len);
static struct ag5648_sfp_data *ag5648_sfp_update_device(struct device *dev);
static ssize_t for_status(struct device *dev, struct device_attribute *dev_attr, char *buf);
static ssize_t set_w_port_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count);
static ssize_t for_r_port_data(struct device *dev, struct device_attribute *dev_attr, char *buf);
static ssize_t set_w_lp_mode_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count);
static ssize_t set_w_reset_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count);

extern int i2c_cpld_read(int bus, unsigned short cpld_addr, u8 reg);
extern int i2c_cpld_write(int bus, unsigned short cpld_addr, u8 reg, u8 value);

enum ag5648_sfp_sysfs_attributes 
{
  SFP_EEPROM,
  SFP_SELECT_PORT,
  SFP_IS_PRESENT,
  SFP_IS_PRESENT_ALL,
  SFP_LP_MODE,
  SFP_RESET
};

static ssize_t set_w_port_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
  long data;
  int error;
  long port_t = 0;
  u8 reg_t = 0x00;

  error = kstrtol(buf, 10, &data);
  if(error)
    return error;

  port_t = data;

  if(port_t > 0 && port_t < 9) 
  {		 /* SFP Port 1-8 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
	} 
  else if (port_t > 8 && port_t < 17) 
  {  /* SFP Port 9-16 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
  } 
  else if (port_t > 16 && port_t < 25) 
  { /* SFP Port 17-24 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
  } 
  else if (port_t > 24 && port_t < 33) 
  { /* SFP Port 25-32 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
  }
  else if (port_t > 32 && port_t < 37) 
  { /* SFP Port 33-36 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
  } 
  else if (port_t > 36 && port_t < 45) 
  { /* SFP Port 37-44 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
  } 
  else if (port_t > 44 && port_t < 49) 
  { /* SFP Port 45-48 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
  } 
  else if (port_t > 48 && port_t < 55) 
  { /* QSFP Port 49-54 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
  } 
  else 
  {

		/* Disable QSFP SFP channel */
    if (i2c_cpld_write(I2C_BUS_2, MASTER_CPLD, QSFP_SFP_SEL_I2C_MUX, DEFAULT_DISABLE) < 0) 
    {
      return -EIO;
    }

    goto exit;
  }

	/* Disable QSFP SFP channel */
  if (i2c_cpld_write(I2C_BUS_2, MASTER_CPLD, QSFP_SFP_SEL_I2C_MUX, DEFAULT_DISABLE) < 0) 
  {
    return -EIO;
  }

	/* Select SFP or QSFP port channel */
  if (i2c_cpld_write(I2C_BUS_2, MASTER_CPLD, reg_t, cpld_to_port_table[port_t]) < 0) 
  {
    return -EIO;
  }

exit:
  sfp_port_data = data;	
  return count;
}

static ssize_t for_r_port_data(struct device *dev, struct device_attribute *dev_attr, char *buf)
{

	if (sfp_port_data == DEFAULT_DISABLE) 
  {
		/* Disable QSFP SFP channel */
    if (i2c_cpld_write(I2C_BUS_2, MASTER_CPLD, QSFP_SFP_SEL_I2C_MUX, DEFAULT_DISABLE) < 0) 
    {
      return -EIO;
    }
  }
 
  return sprintf(buf, "%d\n", sfp_port_data);
}

static ssize_t set_w_lp_mode_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
  long data;
  int error;
  long port_t = 0;
  int bit_t = 0x00;
  int values = 0x00;

  error = kstrtol(buf, 10, &data);
  if (error)
    return error;

  port_t = sfp_port_data;
	
  if (port_t > 48 && port_t < 55) 
  { /* QSFP Port 48-53 */
    values = i2c_cpld_read(I2C_BUS_2, MASTER_CPLD, QSFP_LP_MODE_1);
    if (values < 0)
      return -EIO;

		/* Indicate the module is in LP mode or not
		 * 0 = Disable
		 * 1 = Enable
		 */
    if (data == 0) 
    {
      bit_t = ~(1 << ((port_t - 1) % 8));
      values = values & bit_t;
    }
    else if (data == 1) 
    {
      bit_t = 1 << ((port_t - 1) % 8);
      values = values | bit_t;
    } 
    else  
    {
      return -EINVAL;
    }

    if (i2c_cpld_write(I2C_BUS_2, MASTER_CPLD, QSFP_LP_MODE_1, 
								values) < 0) 
    {
      return -EIO;
    }
  }

  return count;
}

static ssize_t set_w_reset_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
  long data;
  int error;
  long port_t = 0;
  int bit_t = 0x00;
  int values = 0x00;

  error = kstrtol(buf, 10, &data);
  if (error)
    return error;

  port_t = sfp_port_data;
	
  if (port_t > 48 && port_t < 55) 
  { /* QSFP Port 48-53 */
    values = i2c_cpld_read(I2C_BUS_2, MASTER_CPLD, QSFP_RESET_1);
    if (values < 0)
      return -EIO;

		/* Indicate the module Reset or not
		 * 0 = Reset
		 * 1 = Normal
		 */
    if (data == 0) 
    {
      bit_t = ~(1 << ((port_t - 1) % 8));
      values = values & bit_t;
    } 
    else if (data == 1) 
    {
      bit_t = 1 << ((port_t - 1) % 8);
      values = values | bit_t;
    } 
    else 
    {
      return -EINVAL;
    }

    if (i2c_cpld_write(I2C_BUS_2, MASTER_CPLD, QSFP_RESET_1, 
								values) < 0) 
    {
      return -EIO;
    }
  }

  return count;
}

static ssize_t for_status(struct device *dev, struct device_attribute *dev_attr, char *buf)
{
  struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
  long port_t = 0;
  u8 reg_t = 0x00;
  u8 cpld_addr_t = 0x00;
  int values[8] = {'\0'};
  int bit_t = 0x00;
	
  switch (attr->index) 
  {
  case SFP_IS_PRESENT:
    port_t = sfp_port_data;

    if (port_t > 0 && port_t < 9) 
    {		 /* SFP Port 1-8 */
      cpld_addr_t = SLAVE_CPLD;
      reg_t = SFP_PRESENCE_1;
		} 
    else if (port_t > 8 && port_t < 17) 
    {  /* SFP Port 9-16 */
      cpld_addr_t = SLAVE_CPLD;
      reg_t =	SFP_PRESENCE_2;
    } 
    else if (port_t > 16 && port_t < 25) 
    { /* SFP Port 17-24 */
      cpld_addr_t = SLAVE_CPLD;
      reg_t = SFP_PRESENCE_3;
		} 
    else if (port_t > 24 && port_t < 33) 
    { /* SFP Port 25-32 */
      cpld_addr_t = SLAVE_CPLD;
      reg_t = SFP_PRESENCE_4;
		} 
    else if (port_t > 32 && port_t < 37) 
    { /* SFP Port 33-36 */
      cpld_addr_t = SLAVE_CPLD;
      reg_t = SFP_PRESENCE_5;
		} 
    else if (port_t > 36 && port_t < 45) 
    { /* SFP Port 37-44 */
      cpld_addr_t = MASTER_CPLD;
      reg_t = SFP_PRESENCE_6;
    } 
    else if (port_t > 44 && port_t < 49) 
    { /* SFP Port 45-48 */
      cpld_addr_t = MASTER_CPLD;
      reg_t = SFP_PRESENCE_7;
		} 
    else if (port_t > 48 && port_t < 55) 
    { /* QSFP Port 49-54 */
      cpld_addr_t = MASTER_CPLD;
      reg_t = QSFP_PRESENCE_1;
    } 
    else 
    {
      values[0] = 1; /* return 1, module NOT present */
      return sprintf(buf, "%d\n", values[0]);
    }

    VALIDATED_READ(buf, values[0], i2c_cpld_read(I2C_BUS_2,
		      				cpld_addr_t, reg_t), 0);

	        /* SWPLD QSFP module respond */
    bit_t = 1 << ((port_t - 1) % 8);
    values[0] = values[0] & bit_t;
    values[0] = values[0] / bit_t;

    /* sfp_is_present value
		 * return 0 is module present
     * return 1 is module NOT present*/
    return sprintf(buf, "%d\n", values[0]);

  case SFP_IS_PRESENT_ALL:
		/*
		 * Report the SFP ALL PRESENCE status
		 * This data information form CPLD.
		 */

		/* SFP_PRESENT Ports 1-8 */
    VALIDATED_READ(buf, values[0],i2c_cpld_read(I2C_BUS_2, SLAVE_CPLD, SFP_PRESENCE_1), 0);
		/* SFP_PRESENT Ports 9-16 */
		VALIDATED_READ(buf, values[1],i2c_cpld_read(I2C_BUS_2, SLAVE_CPLD, SFP_PRESENCE_2), 0);
		/* SFP_PRESENT Ports 17-24 */
		VALIDATED_READ(buf, values[2],i2c_cpld_read(I2C_BUS_2, SLAVE_CPLD, SFP_PRESENCE_3), 0); 
		/* SFP_PRESENT Ports 25-32 */
		VALIDATED_READ(buf, values[3],i2c_cpld_read(I2C_BUS_2, SLAVE_CPLD, SFP_PRESENCE_4), 0);
		/* SFP_PRESENT Ports 33-40 */
		VALIDATED_READ(buf, values[4],i2c_cpld_read(I2C_BUS_2, SLAVE_CPLD, SFP_PRESENCE_5), 0);
		/* SFP_PRESENT Ports 41-48 */
		VALIDATED_READ(buf, values[5],i2c_cpld_read(I2C_BUS_2, MASTER_CPLD, SFP_PRESENCE_6), 0);
		/* SFP_PRESENT Ports 41-48 */
		VALIDATED_READ(buf, values[6],i2c_cpld_read(I2C_BUS_2, MASTER_CPLD, SFP_PRESENCE_7), 0);
		/* QSFP_PRESENT Ports 49-54 */
		VALIDATED_READ(buf, values[7],i2c_cpld_read(I2C_BUS_2, MASTER_CPLD, QSFP_PRESENCE_1), 0);


		/* sfp_is_present_all value
		 * return 0 is module present
		 * return 1 is module NOT present
		 */
    return sprintf(buf, "%02X %02X %02X %02X %02X %02X %02X %02X\n",values[0], values[1], values[2],values[3], values[4], values[5],values[6], values[7]); 

  case SFP_LP_MODE:
    port_t = sfp_port_data;
    if (port_t > 48 && port_t < 55) 
    { /* QSFP Port 49-54 */
      VALIDATED_READ(buf, values[0], i2c_cpld_read(I2C_BUS_2,MASTER_CPLD, QSFP_LP_MODE_1), 0);
		} 
    else 
    {
			/* In AG5648 only QSFP support control LP MODE */
      values[0] = 0;
      return sprintf(buf, "%d\n", values[0]);
    }

		/* SWPLD QSFP module respond */
    bit_t = 1 << ((port_t - 1) % 8);
    values[0] = values[0] & bit_t;
    values[0] = values[0] / bit_t;

		/* sfp_lp_mode value
		 * return 0 is module NOT in LP mode
		 * return 1 is module in LP mode
		 */
    return sprintf(buf, "%d\n", values[0]);

  case SFP_RESET:
    port_t = sfp_port_data;
    if (port_t > 48 && port_t < 55) 
    { /* QSFP Port 49-54 */
      VALIDATED_READ(buf, values[0], i2c_cpld_read(I2C_BUS_2,MASTER_CPLD, QSFP_RESET_1), 0);
		} 
    else 
    {
			/* In AG5648 only QSFP support control RESET MODE */
      values[0] = 1;
      return sprintf(buf, "%d\n", values[0]);
		}

		/* SWPLD QSFP module respond */
    bit_t = 1 << ((port_t - 1) % 8);
    values[0] = values[0] & bit_t;
    values[0] = values[0] / bit_t;

		/* sfp_reset value
		 * return 0 is module Reset
		 * return 1 is module Normal*/
    return sprintf(buf, "%d\n", values[0]);	

  default:
    return (attr->index);
	}
}

static ssize_t for_eeprom(struct device *dev, struct device_attribute *dev_attr,char *buf)
{
  struct ag5648_sfp_data *data = ag5648_sfp_update_device(dev);
  if (!data->valid) 
  {
    return 0;
  }
  memcpy(buf, data->eeprom, sizeof(data->eeprom));
  return sizeof(data->eeprom);
}

static int ag5648_sfp_read_block(struct i2c_client *client, u8 command, u8 *data, int data_len)
{
  int result = i2c_smbus_read_i2c_block_data(client, command, data_len,data);
  if (unlikely(result < 0))
    goto abort;
  if (unlikely(result != data_len)) 
  {
    result = -EIO;
    goto abort;
  }
  result = 0;
  abort:
    return result;
}

static struct ag5648_sfp_data *ag5648_sfp_update_device(struct device *dev)
{
  struct i2c_client *client = to_i2c_client(dev);
  struct ag5648_sfp_data *data = i2c_get_clientdata(client);
  long port_t = 0;
	u8 reg_t = 0x00;

  port_t = sfp_port_data;

	memset(data->eeprom, 0, sizeof(data->eeprom));
	
  if (port_t > 0 && port_t < 9) 
  {		 /* SFP Port 1-8 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
	} 
  else if (port_t > 8 && port_t < 17) 
  {  /* SFP Port 9-16 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
	} 
  else if (port_t > 16 && port_t < 25) 
  { /* SFP Port 17-24 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
	} 
  else if (port_t > 24 && port_t < 33) 
  { /* SFP Port 25-32 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
	} 
  else if (port_t > 32 && port_t < 37) 
  { /* SFP Port 33-36 */
    reg_t = QSFP_SFP_SEL_I2C_MUX;
	} 
  else if (port_t > 36 && port_t < 45) 
  { /* SFP Port 37-44 */
		reg_t = QSFP_SFP_SEL_I2C_MUX;
	} 
  else if (port_t > 44 && port_t < 49) 
  { /* SFP Port 45-48 */
		reg_t = QSFP_SFP_SEL_I2C_MUX;
	} 
  else if (port_t > 48 && port_t < 55) 
  { /* QSFP Port 49-54 */
		reg_t = QSFP_SFP_SEL_I2C_MUX;
	} 
  else 
  {
		memset(data->eeprom, 0, sizeof(data->eeprom));

		/* Disable QSFP SFP channel */
    if (i2c_cpld_write(I2C_BUS_2, MASTER_CPLD, QSFP_SFP_SEL_I2C_MUX,DEFAULT_DISABLE) < 0) 
    {
      goto exit;
    }

		goto exit;
	}

	/* Disable QSFP SFP channel */
	if (i2c_cpld_write(I2C_BUS_2, MASTER_CPLD, QSFP_SFP_SEL_I2C_MUX,DEFAULT_DISABLE) < 0) {
    memset(data->eeprom, 0, sizeof(data->eeprom));
		goto exit;
	}

	/* Select SFP or QSFP port channel */
  if (i2c_cpld_write(I2C_BUS_2, MASTER_CPLD, reg_t,cpld_to_port_table[port_t]) < 0) 
  {
		memset(data->eeprom, 0, sizeof(data->eeprom));
		goto exit;
	}
    mutex_lock(&data->update_lock);

    if (time_after(jiffies, data->last_updated) || !data->valid) 
    {
		  int status = -1;
		  int i = 0;
		  data->valid = 0;
      memset(data->eeprom, 0, sizeof(data->eeprom));

      for (i = 0; i < sizeof(data->eeprom)/I2C_SMBUS_BLOCK_MAX; i++) 
      {
        status = ag5648_sfp_read_block(client,i * I2C_SMBUS_BLOCK_MAX,data->eeprom + (i * I2C_SMBUS_BLOCK_MAX),I2C_SMBUS_BLOCK_MAX);
        if (status < 0) 
        {
          printk(KERN_INFO "status = %d\n", status);
          dev_dbg(&client->dev,"unable to read eeprom from port(%d)\n", data->port);

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
static SENSOR_DEVICE_ATTR(sfp_eeprom,S_IRUGO, for_eeprom, NULL,SFP_EEPROM);
static SENSOR_DEVICE_ATTR(sfp_select_port, S_IWUSR | S_IRUGO, for_r_port_data, set_w_port_data, SFP_SELECT_PORT);
static SENSOR_DEVICE_ATTR(sfp_is_present, S_IRUGO, for_status, NULL, SFP_IS_PRESENT);
static SENSOR_DEVICE_ATTR(sfp_is_present_all, S_IRUGO, for_status, NULL, SFP_IS_PRESENT_ALL);
static SENSOR_DEVICE_ATTR(sfp_lp_mode,S_IWUSR | S_IRUGO, for_status, set_w_lp_mode_data, SFP_LP_MODE);
static SENSOR_DEVICE_ATTR(sfp_reset, S_IWUSR | S_IRUGO, for_status, set_w_reset_data, SFP_RESET);

static struct attribute *ag5648_sfp_attributes[] = {
	&sensor_dev_attr_sfp_eeprom.dev_attr.attr,
	&sensor_dev_attr_sfp_select_port.dev_attr.attr,
	&sensor_dev_attr_sfp_is_present.dev_attr.attr,
	&sensor_dev_attr_sfp_is_present_all.dev_attr.attr,
	&sensor_dev_attr_sfp_lp_mode.dev_attr.attr,
	&sensor_dev_attr_sfp_reset.dev_attr.attr,
	NULL
};

static const struct attribute_group ag5648_sfp_group = {
	.attrs = ag5648_sfp_attributes,
};

static int ag5648_sfp_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ag5648_sfp_data *data;
	int status;
	
	if (!i2c_check_functionality(client->adapter,I2C_FUNC_SMBUS_I2C_BLOCK)) 
  {
		status = -EIO;
		goto exit;
	}
	
	data = kzalloc(sizeof(struct ag5648_sfp_data), GFP_KERNEL);
	if (!data) {
		status = -ENOMEM;
		goto exit;
	}

	mutex_init(&data->update_lock);
	data->port = id->driver_data;
	i2c_set_clientdata(client, data);

	dev_info(&client->dev, "chip found\n");

	status = sysfs_create_group(&client->dev.kobj, &ag5648_sfp_group);
  if (status)
    goto exit_sysfs_create_group;

	data->hwmon_dev = hwmon_device_register(&client->dev);
  if (IS_ERR(data->hwmon_dev)) 
  {
    status = PTR_ERR(data->hwmon_dev);
    goto exit_hwmon_device_register;
  }

	dev_info(&client->dev, "%s: sfp '%s'\n", dev_name(data->hwmon_dev),client->name);

	return 0;

  exit_hwmon_device_register:
	  sysfs_remove_group(&client->dev.kobj, &ag5648_sfp_group);
  exit_sysfs_create_group:
	  kfree(data);
  exit:
	  return status;
}

static int ag5648_sfp_remove(struct i2c_client *client)
{
	struct ag5648_sfp_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &ag5648_sfp_group);
	kfree(data);
	return 0;
}

enum id_name 
{
	dni_ag5648_sfp
};

static const struct i2c_device_id ag5648_sfp_id[] = {
	{ "dni_ag5648_sfp", dni_ag5648_sfp },
	{}
};
MODULE_DEVICE_TABLE(i2c, ag5648_sfp_id);


static struct i2c_driver ag5648_sfp_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "dni_ag5648_sfp",
	},
	.probe		= ag5648_sfp_probe,
	.remove		= ag5648_sfp_remove,
	.id_table	= ag5648_sfp_id,
	.address_list	= normal_i2c,
};

static int __init ag5648_sfp_init(void)
{
        return i2c_add_driver(&ag5648_sfp_driver);
}

static void __exit ag5648_sfp_exit(void)
{
        i2c_del_driver(&ag5648_sfp_driver);
}

MODULE_AUTHOR("Aries Lin <aries.lin@deltaww.com>");
MODULE_DESCRIPTION("ag5648 SFP Driver");
MODULE_LICENSE("GPL");

module_init(ag5648_sfp_init);
module_exit(ag5648_sfp_exit);
