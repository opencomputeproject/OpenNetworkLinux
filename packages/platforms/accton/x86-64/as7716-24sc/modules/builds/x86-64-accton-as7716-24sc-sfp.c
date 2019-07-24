/*
 * SFP driver for accton as7716_24sc sfp
 *
 * Copyright (C)  Brandon Chuang <brandon_chuang@accton.com.tw>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#define DRIVER_NAME 	"as7716_24sc_sfp" /* Platform dependent */

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
	#define DEBUG_PRINT(fmt, args...)										 \
		printk (KERN_INFO "%s:%s[%d]: " fmt "\r\n", __FILE__, __FUNCTION__, __LINE__, ##args)
#else
	#define DEBUG_PRINT(fmt, args...)
#endif

#define NUM_OF_SFP_PORT			24
#define EEPROM_NAME				"sfp_eeprom"
#define EEPROM_SIZE				256	/*	256 byte eeprom */
#define BIT_INDEX(i)			(1ULL << (i))
#define USE_I2C_BLOCK_READ 		1 /* Platform dependent */
#define I2C_RW_RETRY_COUNT		10
#define I2C_RW_RETRY_INTERVAL	60 /* ms */

#define SFP_EEPROM_A0_I2C_ADDR (0xA0 >> 1)
#define SFP_EEPROM_A2_I2C_ADDR (0xA2 >> 1)

#define SFF8024_PHYSICAL_DEVICE_ID_ADDR		0x0
#define SFF8024_DEVICE_ID_SFP				0x3
#define SFF8024_DEVICE_ID_QSFP				0xC
#define SFF8024_DEVICE_ID_QSFP_PLUS			0xD
#define SFF8024_DEVICE_ID_QSFP28			0x11

#define SFF8472_DIAG_MON_TYPE_ADDR			92
#define SFF8472_DIAG_MON_TYPE_DDM_MASK		0x40
#define SFF8472_10G_ETH_COMPLIANCE_ADDR		0x3
#define SFF8472_10G_BASE_MASK				0xF0

#define SFF8436_RX_LOS_ADDR					3
#define SFF8436_TX_FAULT_ADDR				4
#define SFF8436_TX_DISABLE_ADDR				86

static ssize_t show_port_number(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_present(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t sfp_show_tx_rx_status(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t qsfp_show_tx_rx_status(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t qsfp_set_tx_disable(struct device *dev, struct device_attribute *da, const char *buf, size_t count);;
static ssize_t sfp_eeprom_read(struct i2c_client *, u8, u8 *,int);
static ssize_t sfp_eeprom_write(struct i2c_client *, u8 , const char *,int);
extern int accton_i2c_cpld_read (unsigned short cpld_addr, u8 reg);

enum sfp_sysfs_attributes {
	PRESENT,
	PRESENT_ALL,
	PORT_NUMBER,
	PORT_TYPE,
	DDM_IMPLEMENTED,
	TX_FAULT,
	TX_FAULT1,
	TX_FAULT2,
	TX_FAULT3,
	TX_FAULT4,
	TX_DISABLE,
	TX_DISABLE1,
	TX_DISABLE2,
	TX_DISABLE3,
	TX_DISABLE4,
	RX_LOS,
	RX_LOS1,
	RX_LOS2,
	RX_LOS3,
	RX_LOS4,
	RX_LOS_ALL
};

/* SFP/QSFP common attributes for sysfs */
static SENSOR_DEVICE_ATTR(sfp_port_number, S_IRUGO, show_port_number, NULL, PORT_NUMBER);
static SENSOR_DEVICE_ATTR(sfp_is_present,  S_IRUGO, show_present, NULL, PRESENT);
static SENSOR_DEVICE_ATTR(sfp_is_present_all,  S_IRUGO, show_present, NULL, PRESENT_ALL);
static SENSOR_DEVICE_ATTR(sfp_tx_disable,  S_IWUSR | S_IRUGO, sfp_show_tx_rx_status, qsfp_set_tx_disable, TX_DISABLE);
static SENSOR_DEVICE_ATTR(sfp_tx_fault,	 S_IRUGO, sfp_show_tx_rx_status, NULL, TX_FAULT);

/* QSFP attributes for sysfs */
static SENSOR_DEVICE_ATTR(sfp_rx_los,  S_IRUGO, sfp_show_tx_rx_status, NULL, RX_LOS);
static SENSOR_DEVICE_ATTR(sfp_rx_los1, S_IRUGO, qsfp_show_tx_rx_status, NULL, RX_LOS1);
static SENSOR_DEVICE_ATTR(sfp_rx_los2, S_IRUGO, qsfp_show_tx_rx_status, NULL, RX_LOS2);
static SENSOR_DEVICE_ATTR(sfp_rx_los3, S_IRUGO, qsfp_show_tx_rx_status, NULL, RX_LOS3);
static SENSOR_DEVICE_ATTR(sfp_rx_los4, S_IRUGO, qsfp_show_tx_rx_status, NULL, RX_LOS4);
static SENSOR_DEVICE_ATTR(sfp_tx_disable1, S_IWUSR | S_IRUGO, qsfp_show_tx_rx_status, qsfp_set_tx_disable, TX_DISABLE1);
static SENSOR_DEVICE_ATTR(sfp_tx_disable2, S_IWUSR | S_IRUGO, qsfp_show_tx_rx_status, qsfp_set_tx_disable, TX_DISABLE2);
static SENSOR_DEVICE_ATTR(sfp_tx_disable3, S_IWUSR | S_IRUGO, qsfp_show_tx_rx_status, qsfp_set_tx_disable, TX_DISABLE3);
static SENSOR_DEVICE_ATTR(sfp_tx_disable4, S_IWUSR | S_IRUGO, qsfp_show_tx_rx_status, qsfp_set_tx_disable, TX_DISABLE4);
static SENSOR_DEVICE_ATTR(sfp_tx_fault1, S_IRUGO, qsfp_show_tx_rx_status, NULL, TX_FAULT1);
static SENSOR_DEVICE_ATTR(sfp_tx_fault2, S_IRUGO, qsfp_show_tx_rx_status, NULL, TX_FAULT2);
static SENSOR_DEVICE_ATTR(sfp_tx_fault3, S_IRUGO, qsfp_show_tx_rx_status, NULL, TX_FAULT3);
static SENSOR_DEVICE_ATTR(sfp_tx_fault4, S_IRUGO, qsfp_show_tx_rx_status, NULL, TX_FAULT4);
static struct attribute *qsfp_attributes[] = {
	&sensor_dev_attr_sfp_port_number.dev_attr.attr,
	&sensor_dev_attr_sfp_is_present.dev_attr.attr,
	&sensor_dev_attr_sfp_is_present_all.dev_attr.attr,
	&sensor_dev_attr_sfp_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp_rx_los1.dev_attr.attr,
	&sensor_dev_attr_sfp_rx_los2.dev_attr.attr,
	&sensor_dev_attr_sfp_rx_los3.dev_attr.attr,
	&sensor_dev_attr_sfp_rx_los4.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_disable1.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_disable2.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_disable3.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_disable4.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_fault.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_fault1.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_fault2.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_fault3.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_fault4.dev_attr.attr,
	NULL
};

/* CFP attributes for sysfs */
static struct attribute *cfp_attributes[] = {
	&sensor_dev_attr_sfp_port_number.dev_attr.attr,
	&sensor_dev_attr_sfp_is_present.dev_attr.attr,
	&sensor_dev_attr_sfp_is_present_all.dev_attr.attr,
	NULL
};

/* Platform dependent +++ */
#define CPLD_PORT_TO_FRONT_PORT(port)  (port+1)

enum port_numbers {
as7716_24sc_port1,  as7716_24sc_port2,  as7716_24sc_port3,  as7716_24sc_port4,  
as7716_24sc_port5,  as7716_24sc_port6,  as7716_24sc_port7,  as7716_24sc_port8, 
as7716_24sc_port9,  as7716_24sc_port10, as7716_24sc_port11, as7716_24sc_port12, 
as7716_24sc_port13, as7716_24sc_port14, as7716_24sc_port15, as7716_24sc_port16,
as7716_24sc_port17, as7716_24sc_port18, as7716_24sc_port19, as7716_24sc_port20,
as7716_24sc_port21, as7716_24sc_port22, as7716_24sc_port23, as7716_24sc_port24,
as7716_24sc_port25, as7716_24sc_port26, as7716_24sc_port27, as7716_24sc_port28,
as7716_24sc_port29, as7716_24sc_port30, as7716_24sc_port31, as7716_24sc_port32
};

#define I2C_DEV_ID(x) { #x, x}

static const struct i2c_device_id sfp_device_id[] = {
I2C_DEV_ID(as7716_24sc_port1),
I2C_DEV_ID(as7716_24sc_port2),
I2C_DEV_ID(as7716_24sc_port3),
I2C_DEV_ID(as7716_24sc_port4),
I2C_DEV_ID(as7716_24sc_port5),
I2C_DEV_ID(as7716_24sc_port6),
I2C_DEV_ID(as7716_24sc_port7),
I2C_DEV_ID(as7716_24sc_port8),
I2C_DEV_ID(as7716_24sc_port9),
I2C_DEV_ID(as7716_24sc_port10),
I2C_DEV_ID(as7716_24sc_port11),
I2C_DEV_ID(as7716_24sc_port12),
I2C_DEV_ID(as7716_24sc_port13),
I2C_DEV_ID(as7716_24sc_port14),
I2C_DEV_ID(as7716_24sc_port15),
I2C_DEV_ID(as7716_24sc_port16),
I2C_DEV_ID(as7716_24sc_port17),
I2C_DEV_ID(as7716_24sc_port18),
I2C_DEV_ID(as7716_24sc_port19),
I2C_DEV_ID(as7716_24sc_port20),
I2C_DEV_ID(as7716_24sc_port21),
I2C_DEV_ID(as7716_24sc_port22),
I2C_DEV_ID(as7716_24sc_port23),
I2C_DEV_ID(as7716_24sc_port24),
I2C_DEV_ID(as7716_24sc_port25),
I2C_DEV_ID(as7716_24sc_port26),
I2C_DEV_ID(as7716_24sc_port27),
I2C_DEV_ID(as7716_24sc_port28),
I2C_DEV_ID(as7716_24sc_port29),
I2C_DEV_ID(as7716_24sc_port30),
I2C_DEV_ID(as7716_24sc_port31),
I2C_DEV_ID(as7716_24sc_port32),
{ /* LIST END */ }
};
MODULE_DEVICE_TABLE(i2c, sfp_device_id);
/* Platform dependent --- */

enum driver_type_e {
	DRIVER_TYPE_SFP_MSA,
	DRIVER_TYPE_SFP_DDM,
	DRIVER_TYPE_QSFP,
	DRIVER_TYPE_XFP,
	DRIVER_TYPE_CFP
};

/* Each client has this additional data
 */
struct eeprom_data {
	char				 valid;			/* !=0 if registers are valid */
	unsigned long		 last_updated;	/* In jiffies */
	struct bin_attribute bin;			/* eeprom data */
};

struct sfp_msa_data {
	char			valid;			/* !=0 if registers are valid */
	unsigned long	last_updated;	/* In jiffies */
	u64				status[6];		/* bit0:port0, bit1:port1 and so on */
									/* index 0 => tx_fail
											 1 => tx_disable
											 2 => rx_loss
											 3 => device id
											 4 => 10G Ethernet Compliance Codes
												  to distinguish SFP or SFP+
											 5 => DIAGNOSTIC MONITORING TYPE */
	struct eeprom_data		eeprom;
};

struct cfp_data {
	char			valid;			/* !=0 if registers are valid */
	unsigned long	last_updated;	/* In jiffies */
	struct eeprom_data		eeprom;
};

struct qsfp_data {
	char			valid;			/* !=0 if registers are valid */
	unsigned long	last_updated;	/* In jiffies */
	u8				status[3];		/* bit0:port0, bit1:port1 and so on */
									/* index 0 => tx_fail
											 1 => tx_disable
											 2 => rx_loss */
	u8					device_id;
	struct eeprom_data	eeprom;
};

struct sfp_port_data {
	struct mutex		   update_lock;
	enum driver_type_e	   driver_type;
	int					   port;		/* CPLD port index */
	u64					   present;		/* present status, bit0:port0, bit1:port1 and so on */

	struct sfp_msa_data	  *msa;
	struct cfp_data	  	  *cfp;
	struct qsfp_data	  *qsfp;

	struct i2c_client	  *client;
};

static ssize_t show_port_number(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sfp_port_data *data = i2c_get_clientdata(client);
	return sprintf(buf, "%d\n", CPLD_PORT_TO_FRONT_PORT(data->port));
}

/* Platform dependent +++ */
static struct sfp_port_data *sfp_update_present(struct i2c_client *client)
{
	struct sfp_port_data *data = i2c_get_clientdata(client);
	int i = 0;
	int status = -1;
	u8 regs[] = {0x78, 0x79, 0x7C, 0x7B};
	u8 reg_vals[4]= {0};

	DEBUG_PRINT("Starting sfp present status update");
	mutex_lock(&data->update_lock);

	/* Read present status of port 1~32 */
    data->present = 0;
	
    for (i = 0; i < ARRAY_SIZE(regs); i++) {
        status = accton_i2c_cpld_read(0x60, regs[i]);
        
        if (status < 0) {
            DEBUG_PRINT("cpld(0x60) reg(0x%x) err %d", regs[i], status);
            goto exit;
        }

        //data->present |= (u64)status << (i*8);
        reg_vals[i] = status;
		DEBUG_PRINT("Present status = 0x%lx", reg_vals[i]);
    }

	data->present |= reg_vals[0];           /* Port  1 ~  8 */
	data->present |= (u64)reg_vals[1] << 8; /* Port  9 ~ 16 */

	data->present |= ((u64)reg_vals[2] & 0x1)  << 16; /* Port 17 */
	data->present |= ((u64)reg_vals[2] & 0x2)  << 17; /* Port 19 */
	data->present |= ((u64)reg_vals[2] & 0x4)  << 18; /* Port 21 */
	data->present |= ((u64)reg_vals[2] & 0x8)  << 19; /* Port 23 */
	data->present |= ((u64)reg_vals[2] & 0x10) << 20; /* Port 25 */
	data->present |= ((u64)reg_vals[2] & 0x20) << 21; /* Port 27 */
	data->present |= ((u64)reg_vals[2] & 0x40) << 22; /* Port 29 */
	data->present |= ((u64)reg_vals[2] & 0x80) << 23; /* Port 31 */
	
	data->present |= ((u64)reg_vals[3] & 0x1)  << 17; /* Port 18 */
	data->present |= ((u64)reg_vals[3] & 0x2)  << 18; /* Port 20 */
	data->present |= ((u64)reg_vals[3] & 0x4)  << 19; /* Port 22 */
	data->present |= ((u64)reg_vals[3] & 0x8)  << 20; /* Port 24 */
	data->present |= ((u64)reg_vals[3] & 0x10) << 21; /* Port 26 */
	data->present |= ((u64)reg_vals[3] & 0x20) << 22; /* Port 28 */
	data->present |= ((u64)reg_vals[3] & 0x40) << 23; /* Port 30 */
	data->present |= ((u64)reg_vals[3] & 0x80) << 24; /* Port 32 */
	data->present &= 0xFFFFFFFF;
	DEBUG_PRINT("Present status = 0x%lx", data->present);
exit:
	mutex_unlock(&data->update_lock);
	return (status < 0) ? ERR_PTR(status) : data;
}

/* Platform dependent --- */

static int sfp_is_port_present(struct i2c_client *client, int port)
{
	struct sfp_port_data *data = i2c_get_clientdata(client);

	data = sfp_update_present(client);
	if (IS_ERR(data)) {
		return PTR_ERR(data);
	}

	return (data->present & BIT_INDEX(data->port)) ? 0 : 1; /* Platform dependent */
}

/* Platform dependent +++ */
static ssize_t show_present(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);

	if (PRESENT_ALL == attr->index) {
		int i;
		u8 values[4]  = {0};
		struct sfp_port_data *data = sfp_update_present(client);
		
		if (IS_ERR(data)) {
			return PTR_ERR(data);
		}

		for (i = 0; i < ARRAY_SIZE(values); i++) {
			values[i] = ~(u8)(data->present >> (i * 8));
		}

        /* Return values 1 -> 32 in order */
        return sprintf(buf, "%.2x %.2x %.2x %.2x\n",
                       values[0], values[1], values[2], values[3]);
	}
	else {
		struct sfp_port_data *data = i2c_get_clientdata(client);
		int present = sfp_is_port_present(client, data->port);

		if (IS_ERR_VALUE(present)) {
			return present;
		}

		/* PRESENT */
		return sprintf(buf, "%d\n", present);
	}
}
/* Platform dependent --- */

static struct sfp_port_data *qsfp_update_tx_rx_status(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sfp_port_data *data = i2c_get_clientdata(client);
	int i, status = -1;
	u8 buf = 0;
	u8 reg[] = {SFF8436_TX_FAULT_ADDR, SFF8436_TX_DISABLE_ADDR, SFF8436_RX_LOS_ADDR};

	if (time_before(jiffies, data->qsfp->last_updated + HZ + HZ / 2) && data->qsfp->valid) {
		return data;
	}

	DEBUG_PRINT("Starting sfp tx rx status update");
	mutex_lock(&data->update_lock);
	data->qsfp->valid = 0;
	memset(data->qsfp->status, 0, sizeof(data->qsfp->status));

	/* Notify device to update tx fault/ tx disable/ rx los status */
	for (i = 0; i < ARRAY_SIZE(reg); i++) {
		status = sfp_eeprom_read(client, reg[i], &buf, sizeof(buf));
		if (unlikely(status < 0)) {
			goto exit;
		}
	}
	msleep(200);

	/* Read actual tx fault/ tx disable/ rx los status */
	for (i = 0; i < ARRAY_SIZE(reg); i++) {
		status = sfp_eeprom_read(client, reg[i], &buf, sizeof(buf));
		if (unlikely(status < 0)) {
			goto exit;
		}

		DEBUG_PRINT("qsfp reg(0x%x) status = (0x%x)", reg[i], data->qsfp->status[i]);
		data->qsfp->status[i] = (buf & 0xF);
	}

	data->qsfp->valid = 1;
	data->qsfp->last_updated = jiffies;

exit:
	mutex_unlock(&data->update_lock);
	return (status < 0) ? ERR_PTR(status) : data;
}

static ssize_t qsfp_show_tx_rx_status(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int present;
	u8 val = 0;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct sfp_port_data *data = i2c_get_clientdata(client);

	present = sfp_is_port_present(client, data->port);
	if (IS_ERR_VALUE(present)) {
		return present;
	}

	if (present == 0) {
		/* port is not present */
		return -ENXIO;
	}

	data = qsfp_update_tx_rx_status(dev);
	if (IS_ERR(data)) {
		return PTR_ERR(data);
	}

	switch (attr->index) {
	case TX_FAULT:
		val = !!(data->qsfp->status[2] & 0xF);
		break;
	case TX_FAULT1:
	case TX_FAULT2:
	case TX_FAULT3:
	case TX_FAULT4:
		val = !!(data->qsfp->status[2] & BIT_INDEX(attr->index - TX_FAULT1));
		break;
	case TX_DISABLE:
		val = data->qsfp->status[1] & 0xF;
		break;
	case TX_DISABLE1:
	case TX_DISABLE2:
	case TX_DISABLE3:
	case TX_DISABLE4:
		val = !!(data->qsfp->status[1] & BIT_INDEX(attr->index - TX_DISABLE1));
		break;
	case RX_LOS:
		val = !!(data->qsfp->status[0] & 0xF);
		break;
	case RX_LOS1:
	case RX_LOS2:
	case RX_LOS3:
	case RX_LOS4:
		val = !!(data->qsfp->status[0] & BIT_INDEX(attr->index - RX_LOS1));
		break;
	default:
		break;
	}

	return sprintf(buf, "%d\n", val);
}

static ssize_t qsfp_set_tx_disable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	long disable;
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct sfp_port_data *data = i2c_get_clientdata(client);	

	status = sfp_is_port_present(client, data->port);
	if (IS_ERR_VALUE(status)) {
		return status;
	}

	if (!status) {
		/* port is not present */
		return -ENXIO;
	}

	status = kstrtol(buf, 10, &disable);
	if (status) {
		return status;
	}

	data = qsfp_update_tx_rx_status(dev);
	if (IS_ERR(data)) {
		return PTR_ERR(data);
	}

	mutex_lock(&data->update_lock);

	if (attr->index == TX_DISABLE) {
		if (disable) {
			data->qsfp->status[1] |= 0xF;
		}
		else {
			data->qsfp->status[1] &= ~0xF;
		}
	}
	else {/* TX_DISABLE1 ~ TX_DISABLE4*/
		if (disable) {
			data->qsfp->status[1] |= (1 << (attr->index - TX_DISABLE1));
		}
		else {
			data->qsfp->status[1] &= ~(1 << (attr->index - TX_DISABLE1));
		}
	}

	DEBUG_PRINT("index = (%d), status = (0x%x)", attr->index, data->qsfp->status[1]);
	status = sfp_eeprom_write(data->client, SFF8436_TX_DISABLE_ADDR, &data->qsfp->status[1], sizeof(data->qsfp->status[1]));
	if (unlikely(status < 0)) {
		count = status;
	}

	mutex_unlock(&data->update_lock);
	return count;
}

/* Platform dependent +++ */
static ssize_t sfp_show_tx_rx_status(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sfp_port_data *data = i2c_get_clientdata(client);

	if (data->driver_type == DRIVER_TYPE_QSFP) {
		return qsfp_show_tx_rx_status(dev, da, buf);
	}

	return 0;
}
/* Platform dependent --- */

static ssize_t sfp_eeprom_write(struct i2c_client *client, u8 command, const char *data,
			  int data_len)
{
#if USE_I2C_BLOCK_READ
	int status, retry = I2C_RW_RETRY_COUNT;

	if (data_len > I2C_SMBUS_BLOCK_MAX) {
		data_len = I2C_SMBUS_BLOCK_MAX;
	}

	while (retry) {
		status = i2c_smbus_write_i2c_block_data(client, command, data_len, data);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

	if (unlikely(status < 0)) {
		return status;
	}

	return data_len;
#else
	int status, retry = I2C_RW_RETRY_COUNT;

	while (retry) {
		status = i2c_smbus_write_byte_data(client, command, *data);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

	if (unlikely(status < 0)) {
		return status;
	}

	return 1;
#endif


}

static ssize_t sfp_port_write(struct sfp_port_data *data,
						  const char *buf, loff_t off, size_t count)
{
	ssize_t retval = 0;

	if (unlikely(!count)) {
		return count;
	}

	/*
	 * Write data to chip, protecting against concurrent updates
	 * from this host, but not from other I2C masters.
	 */
	mutex_lock(&data->update_lock);

	while (count) {
		ssize_t status;

		status = sfp_eeprom_write(data->client, off, buf, count);
		if (status <= 0) {
			if (retval == 0) {
				retval = status;
			}
			break;
		}
		buf += status;
		off += status;
		count -= status;
		retval += status;
	}

	mutex_unlock(&data->update_lock);
	return retval;
}


static ssize_t sfp_bin_write(struct file *filp, struct kobject *kobj,
				struct bin_attribute *attr,
				char *buf, loff_t off, size_t count)
{
	int present;
	struct sfp_port_data *data;
	DEBUG_PRINT("%s(%d) offset = (%d), count = (%d)", off, count);
	data = dev_get_drvdata(container_of(kobj, struct device, kobj));

	present = sfp_is_port_present(data->client, data->port);
	if (IS_ERR_VALUE(present)) {
		return present;
	}

	if (present == 0) {
		/* port is not present */
		return -ENODEV;
	}

	return sfp_port_write(data, buf, off, count);
}

static ssize_t sfp_eeprom_read(struct i2c_client *client, u8 command, u8 *data,
			  int data_len)
{
#if USE_I2C_BLOCK_READ
	int status, retry = I2C_RW_RETRY_COUNT;

	if (data_len > I2C_SMBUS_BLOCK_MAX) {
		data_len = I2C_SMBUS_BLOCK_MAX;
	}

	while (retry) {
		status = i2c_smbus_read_i2c_block_data(client, command, data_len, data);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

	if (unlikely(status < 0)) {
		goto abort;
	}
	if (unlikely(status != data_len)) {
		status = -EIO;
		goto abort;
	}

	//result = data_len;

abort:
	return status;
#else
	int status, retry = I2C_RW_RETRY_COUNT;

	while (retry) {
		status = i2c_smbus_read_byte_data(client, command);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

	if (unlikely(status < 0)) {
		dev_dbg(&client->dev, "sfp read byte data failed, command(0x%2x), data(0x%2x)\r\n", command, status);
		goto abort;
	}

	*data  = (u8)status;
	status = 1;

abort:
	return status;
#endif
}

static ssize_t sfp_port_read(struct sfp_port_data *data,
				char *buf, loff_t off, size_t count)
{
	ssize_t retval = 0;

	if (unlikely(!count)) {
		DEBUG_PRINT("Count = 0, return");
		return count;
	}

	/*
	 * Read data from chip, protecting against concurrent updates
	 * from this host, but not from other I2C masters.
	 */
	mutex_lock(&data->update_lock);

	while (count) {
		ssize_t status;

		status = sfp_eeprom_read(data->client, off, buf, count);
		if (status <= 0) {
			if (retval == 0) {
				retval = status;
			}
			break;
		}

		buf += status;
		off += status;
		count -= status;
		retval += status;
	}

	mutex_unlock(&data->update_lock);
	return retval;

}

static ssize_t sfp_bin_read(struct file *filp, struct kobject *kobj,
		struct bin_attribute *attr,
		char *buf, loff_t off, size_t count)
{
	int present;
	struct sfp_port_data *data;
	DEBUG_PRINT("offset = (%d), count = (%d)", off, count);
	
	data = dev_get_drvdata(container_of(kobj, struct device, kobj));
	present = sfp_is_port_present(data->client, data->port);
	if (IS_ERR_VALUE(present)) {
		return present;
	}

	if (present == 0) {
		/* port is not present */
		return -ENODEV;
	}

	return sfp_port_read(data, buf, off, count);
}

static int sfp_sysfs_eeprom_init(struct kobject *kobj, struct bin_attribute *eeprom)
{
	int err;

	sysfs_bin_attr_init(eeprom);
	eeprom->attr.name = EEPROM_NAME;
	eeprom->attr.mode = S_IWUSR | S_IRUGO;
	eeprom->read	  = sfp_bin_read;
	eeprom->write	  = sfp_bin_write;
	eeprom->size	  = EEPROM_SIZE;

	/* Create eeprom file */
	err = sysfs_create_bin_file(kobj, eeprom);
	if (err) {
		return err;
	}

	return 0;
}

static int sfp_sysfs_eeprom_cleanup(struct kobject *kobj, struct bin_attribute *eeprom)
{
	sysfs_remove_bin_file(kobj, eeprom);
	return 0;
}

static int sfp_i2c_check_functionality(struct i2c_client *client)
{
#if USE_I2C_BLOCK_READ
	return i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK);
#else
	return i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA);
#endif
}

static const struct attribute_group cfp_group = {
	.attrs = cfp_attributes,
};

static int cfp_probe(struct i2c_client *client, const struct i2c_device_id *dev_id,
							   struct cfp_data **data)
{
	int status;
	struct cfp_data *cfp;

	if (!sfp_i2c_check_functionality(client)) {
		status = -EIO;
		goto exit;
	}

	cfp = kzalloc(sizeof(struct cfp_data), GFP_KERNEL);
	if (!cfp) {
		status = -ENOMEM;
		goto exit;
	}

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &cfp_group);
	if (status) {
		goto exit_free;
	}

	/* init eeprom */
	status = sfp_sysfs_eeprom_init(&client->dev.kobj, &cfp->eeprom.bin);
	if (status) {
		goto exit_remove;
	}

	*data = cfp;
	dev_info(&client->dev, "cfp '%s'\n", client->name);

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &cfp_group);
exit_free:
	kfree(cfp);
exit:

	return status;
}

static const struct attribute_group qsfp_group = {
	.attrs = qsfp_attributes,
};

static int qsfp_probe(struct i2c_client *client, const struct i2c_device_id *dev_id,
						  struct qsfp_data **data)
{
	int status;
	struct qsfp_data *qsfp;

	if (!sfp_i2c_check_functionality(client)) {
		status = -EIO;
		goto exit;
	}

	qsfp = kzalloc(sizeof(struct qsfp_data), GFP_KERNEL);
	if (!qsfp) {
		status = -ENOMEM;
		goto exit;
	}

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &qsfp_group);
	if (status) {
		goto exit_free;
	}

	/* init eeprom */
	status = sfp_sysfs_eeprom_init(&client->dev.kobj, &qsfp->eeprom.bin);
	if (status) {
		goto exit_remove;
	}

	*data = qsfp;
	dev_info(&client->dev, "qsfp '%s'\n", client->name);

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &qsfp_group);
exit_free:
	kfree(qsfp);
exit:

	return status;
}

/* Platform dependent +++ */
static int sfp_device_probe(struct i2c_client *client,
			const struct i2c_device_id *dev_id)
{
	int ret = 0;
	struct sfp_port_data *data = NULL;

	if (client->addr != SFP_EEPROM_A0_I2C_ADDR) {
		return -ENODEV;
	}

	data = kzalloc(sizeof(struct sfp_port_data), GFP_KERNEL);
	if (!data) {
		return -ENOMEM;
	}

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);
	data->port	 = dev_id->driver_data;
	data->client = client;
	data->driver_type = DRIVER_TYPE_QSFP;
	ret = qsfp_probe(client, dev_id, &data->qsfp);

	if (ret < 0) {
		goto exit_kfree;
	}

	return ret;

exit_kfree:
	kfree(data);
	return ret;
}
/* Platform dependent --- */

static int cfp_remove(struct i2c_client *client, struct cfp_data *data)
{
	sfp_sysfs_eeprom_cleanup(&client->dev.kobj, &data->eeprom.bin);
	sysfs_remove_group(&client->dev.kobj, &cfp_group);
	kfree(data);
	return 0;
}

static int qsfp_remove(struct i2c_client *client, struct qsfp_data *data)
{
	sfp_sysfs_eeprom_cleanup(&client->dev.kobj, &data->eeprom.bin);
	sysfs_remove_group(&client->dev.kobj, &qsfp_group);
	kfree(data);
	return 0;
}

static int sfp_device_remove(struct i2c_client *client)
{
	int ret = 0;
	struct sfp_port_data *data = i2c_get_clientdata(client);

	switch (data->driver_type) {
		case DRIVER_TYPE_CFP:
			ret = cfp_remove(client, data->cfp);
			break;
		case DRIVER_TYPE_QSFP:
			ret = qsfp_remove(client, data->qsfp);
			break;
		default:
			break;
	}

	kfree(data);
	return ret;
}

/* Addresses scanned
 */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

static struct i2c_driver sfp_driver = {
	.driver = {
		.name	  = DRIVER_NAME,
	},
	.probe		  = sfp_device_probe,
	.remove		  = sfp_device_remove,
	.id_table	  = sfp_device_id,
	.address_list = normal_i2c,
};

static int __init sfp_init(void)
{
	return i2c_add_driver(&sfp_driver);
}

static void __exit sfp_exit(void)
{
	i2c_del_driver(&sfp_driver);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("accton as7716_24sc_sfp driver");
MODULE_LICENSE("GPL");

module_init(sfp_init);
module_exit(sfp_exit);

