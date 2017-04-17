/*
 * SFP driver for accton as5822_54x sfp
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

#define DRIVER_NAME 	"as5822_54x_sfp" /* Platform dependent */

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
	#define DEBUG_PRINT(fmt, args...)                                        \
		printk (KERN_INFO "%s:%s[%d]: " fmt "\r\n", __FILE__, __FUNCTION__, __LINE__, ##args)
#else
	#define DEBUG_PRINT(fmt, args...)
#endif

#define NUM_OF_SFP_PORT 		54
#define EEPROM_NAME 			"sfp_eeprom"
#define EEPROM_SIZE				256	/*  256 byte eeprom */
#define BIT_INDEX(i) 			(1ULL << (i))
#define USE_I2C_BLOCK_READ 		1 /* Platform dependent */
#define I2C_RW_RETRY_COUNT		3
#define I2C_RW_RETRY_INTERVAL	100 /* ms */

#define SFP_EEPROM_A0_I2C_ADDR (0xA0 >> 1)
#define SFP_EEPROM_A2_I2C_ADDR (0xA2 >> 1)

#define SFF8024_PHYSICAL_DEVICE_ID_ADDR		0x0
#define SFF8024_DEVICE_ID_SFP				0x3
#define SFF8024_DEVICE_ID_QSFP				0xC
#define SFF8024_DEVICE_ID_QSFP_PLUS			0xD
#define SFF8024_DEVICE_ID_QSFP28			0x11

#define SFF8472_DIAG_MON_TYPE_ADDR        	92
#define SFF8472_DIAG_MON_TYPE_DDM_MASK		0x40
#define SFF8472_10G_ETH_COMPLIANCE_ADDR		0x3
#define SFF8472_10G_BASE_MASK				0xF0

#define SFF8436_RX_LOS_ADDR					3
#define SFF8436_TX_FAULT_ADDR				4
#define SFF8436_TX_DISABLE_ADDR				86

/* Platform dependent +++ */
enum CPLD_ID{
	SFP_CPLD1,
	SFP_CPLD2,
	NUM_OF_CPLD
};

int cplds[NUM_OF_CPLD] = {0x61, 0x62};

/* Platform dependent --- */

static ssize_t show_port_number(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_port_type(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_present(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t sfp_show_tx_rx_status(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t qsfp_show_tx_rx_status(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t sfp_set_tx_disable(struct device *dev, struct device_attribute *da, const char *buf, size_t count);
static ssize_t qsfp_set_tx_disable(struct device *dev, struct device_attribute *da, const char *buf, size_t count);;
static ssize_t sfp_show_ddm_implemented(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t sfp_eeprom_read(struct i2c_client *, u8, u8 *,int);
static ssize_t sfp_eeprom_write(struct i2c_client *, u8 , const char *,int);
extern int accton_i2c_cpld_read(unsigned short cpld_addr, u8 reg);
extern int accton_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

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
	TX_FAULT_ALL,
	TX_DISABLE,
	TX_DISABLE1,
	TX_DISABLE2,
	TX_DISABLE3,
	TX_DISABLE4,
	TX_DISABLE_ALL,
	RX_LOS,
	RX_LOS1,
	RX_LOS2,
	RX_LOS3,
	RX_LOS4,
	RX_LOS_ALL
};

/* SFP/QSFP common attributes for sysfs */
static SENSOR_DEVICE_ATTR(sfp_port_number, S_IRUGO, show_port_number, NULL, PORT_NUMBER);
static SENSOR_DEVICE_ATTR(sfp_port_type, S_IRUGO, show_port_type, NULL, PORT_TYPE);
static SENSOR_DEVICE_ATTR(sfp_is_present,  S_IRUGO, show_present, NULL, PRESENT);
static SENSOR_DEVICE_ATTR(sfp_is_present_all,  S_IRUGO, show_present, NULL, PRESENT_ALL);
static SENSOR_DEVICE_ATTR(sfp_rx_los,  S_IRUGO, sfp_show_tx_rx_status, NULL, RX_LOS);
static SENSOR_DEVICE_ATTR(sfp_tx_disable,  S_IWUSR | S_IRUGO, sfp_show_tx_rx_status, sfp_set_tx_disable, TX_DISABLE);
static SENSOR_DEVICE_ATTR(sfp_tx_fault,	 S_IRUGO, sfp_show_tx_rx_status, NULL, TX_FAULT);

/* QSFP attributes for sysfs */
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
	&sensor_dev_attr_sfp_port_type.dev_attr.attr,
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

/* SFP msa attributes for sysfs */
static SENSOR_DEVICE_ATTR(sfp_ddm_implemented,	 S_IRUGO, sfp_show_ddm_implemented, NULL, DDM_IMPLEMENTED);
static SENSOR_DEVICE_ATTR(sfp_rx_los_all,  S_IRUGO, sfp_show_tx_rx_status, NULL, RX_LOS_ALL);
static SENSOR_DEVICE_ATTR(sfp_tx_fault_all,  S_IRUGO, sfp_show_tx_rx_status, NULL, TX_FAULT_ALL);
static SENSOR_DEVICE_ATTR(sfp_tx_disable_all,  S_IRUGO, sfp_show_tx_rx_status, NULL, TX_DISABLE_ALL);
static struct attribute *sfp_msa_attributes[] = {
	&sensor_dev_attr_sfp_port_number.dev_attr.attr,
	&sensor_dev_attr_sfp_port_type.dev_attr.attr,
	&sensor_dev_attr_sfp_is_present.dev_attr.attr,
	&sensor_dev_attr_sfp_is_present_all.dev_attr.attr,
	&sensor_dev_attr_sfp_ddm_implemented.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_fault.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_fault_all.dev_attr.attr,
	&sensor_dev_attr_sfp_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp_rx_los_all.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_disable_all.dev_attr.attr,
	NULL
};

/* SFP ddm attributes for sysfs */
static struct attribute *sfp_ddm_attributes[] = {
	NULL
};

/* Platform dependent +++ */
#define CPLD_PORT_TO_FRONT_PORT(port)  (port+1)

enum port_numbers {
as5822_54x_sfp1,  as5822_54x_sfp2,  as5822_54x_sfp3,  as5822_54x_sfp4,  as5822_54x_sfp5,  as5822_54x_sfp6,  as5822_54x_sfp7,  as5822_54x_sfp8, 
as5822_54x_sfp9,  as5822_54x_sfp10, as5822_54x_sfp11, as5822_54x_sfp12, as5822_54x_sfp13, as5822_54x_sfp14, as5822_54x_sfp15, as5822_54x_sfp16,
as5822_54x_sfp17, as5822_54x_sfp18, as5822_54x_sfp19, as5822_54x_sfp20, as5822_54x_sfp21, as5822_54x_sfp22, as5822_54x_sfp23, as5822_54x_sfp24,
as5822_54x_sfp25, as5822_54x_sfp26, as5822_54x_sfp27, as5822_54x_sfp28, as5822_54x_sfp29, as5822_54x_sfp30, as5822_54x_sfp31, as5822_54x_sfp32,
as5822_54x_sfp33, as5822_54x_sfp34, as5822_54x_sfp35, as5822_54x_sfp36, as5822_54x_sfp37, as5822_54x_sfp38, as5822_54x_sfp39, as5822_54x_sfp40,
as5822_54x_sfp41, as5822_54x_sfp42, as5822_54x_sfp43, as5822_54x_sfp44, as5822_54x_sfp45, as5822_54x_sfp46, as5822_54x_sfp47, as5822_54x_sfp48,
as5822_54x_sfp49, as5822_54x_sfp50, as5822_54x_sfp51, as5822_54x_sfp52, as5822_54x_sfp53, as5822_54x_sfp54
};

static const struct i2c_device_id sfp_device_id[] = {
{ "as5822_54x_sfp1",  as5822_54x_sfp1 },  { "as5822_54x_sfp2",  as5822_54x_sfp2 },  { "as5822_54x_sfp3",  as5822_54x_sfp3 },  { "as5822_54x_sfp4",  as5822_54x_sfp4 },
{ "as5822_54x_sfp5",  as5822_54x_sfp5 },  { "as5822_54x_sfp6",  as5822_54x_sfp6 },  { "as5822_54x_sfp7",  as5822_54x_sfp7 },  { "as5822_54x_sfp8",  as5822_54x_sfp8 },
{ "as5822_54x_sfp9",  as5822_54x_sfp9 },  { "as5822_54x_sfp10", as5822_54x_sfp10 }, { "as5822_54x_sfp11", as5822_54x_sfp11 }, { "as5822_54x_sfp12", as5822_54x_sfp12 },
{ "as5822_54x_sfp13", as5822_54x_sfp13 }, { "as5822_54x_sfp14", as5822_54x_sfp14 }, { "as5822_54x_sfp15", as5822_54x_sfp15 }, { "as5822_54x_sfp16", as5822_54x_sfp16 },
{ "as5822_54x_sfp17", as5822_54x_sfp17 }, { "as5822_54x_sfp18", as5822_54x_sfp18 }, { "as5822_54x_sfp19", as5822_54x_sfp19 }, { "as5822_54x_sfp20", as5822_54x_sfp20 },
{ "as5822_54x_sfp21", as5822_54x_sfp21 }, { "as5822_54x_sfp22", as5822_54x_sfp22 }, { "as5822_54x_sfp23", as5822_54x_sfp23 }, { "as5822_54x_sfp24", as5822_54x_sfp24 },
{ "as5822_54x_sfp25", as5822_54x_sfp25 }, { "as5822_54x_sfp26", as5822_54x_sfp26 }, { "as5822_54x_sfp27", as5822_54x_sfp27 }, { "as5822_54x_sfp28", as5822_54x_sfp28 },
{ "as5822_54x_sfp29", as5822_54x_sfp29 }, { "as5822_54x_sfp30", as5822_54x_sfp30 }, { "as5822_54x_sfp31", as5822_54x_sfp31 }, { "as5822_54x_sfp32", as5822_54x_sfp32 },
{ "as5822_54x_sfp33", as5822_54x_sfp33 }, { "as5822_54x_sfp34", as5822_54x_sfp34 }, { "as5822_54x_sfp35", as5822_54x_sfp35 }, { "as5822_54x_sfp36", as5822_54x_sfp36 },
{ "as5822_54x_sfp37", as5822_54x_sfp37 }, { "as5822_54x_sfp38", as5822_54x_sfp38 }, { "as5822_54x_sfp39", as5822_54x_sfp39 }, { "as5822_54x_sfp40", as5822_54x_sfp40 },
{ "as5822_54x_sfp41", as5822_54x_sfp41 }, { "as5822_54x_sfp42", as5822_54x_sfp42 }, { "as5822_54x_sfp43", as5822_54x_sfp43 }, { "as5822_54x_sfp44", as5822_54x_sfp44 },
{ "as5822_54x_sfp45", as5822_54x_sfp45 }, { "as5822_54x_sfp46", as5822_54x_sfp46 }, { "as5822_54x_sfp47", as5822_54x_sfp47 }, { "as5822_54x_sfp48", as5822_54x_sfp48 },
{ "as5822_54x_sfp49", as5822_54x_sfp49 }, { "as5822_54x_sfp50", as5822_54x_sfp50 }, { "as5822_54x_sfp51", as5822_54x_sfp51 }, { "as5822_54x_sfp52", as5822_54x_sfp52 },
{ "as5822_54x_sfp53", as5822_54x_sfp53 }, { "as5822_54x_sfp54", as5822_54x_sfp54 },
{ /* LIST END */ }
};
MODULE_DEVICE_TABLE(i2c, sfp_device_id);
/* Platform dependent --- */

/*
 * list of valid port types
 * note OOM_PORT_TYPE_NOT_PRESENT to indicate no
 * module is present in this port
 */
typedef enum oom_driver_port_type_e {
	OOM_DRIVER_PORT_TYPE_INVALID,
	OOM_DRIVER_PORT_TYPE_NOT_PRESENT,
	OOM_DRIVER_PORT_TYPE_SFP,
	OOM_DRIVER_PORT_TYPE_SFP_PLUS,
	OOM_DRIVER_PORT_TYPE_QSFP,
	OOM_DRIVER_PORT_TYPE_QSFP_PLUS,
	OOM_DRIVER_PORT_TYPE_QSFP28
} oom_driver_port_type_t;

enum driver_type_e {
	DRIVER_TYPE_SFP_MSA,
	DRIVER_TYPE_SFP_DDM,
	DRIVER_TYPE_QSFP
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
	struct eeprom_data				eeprom;
};

struct sfp_ddm_data {
	struct eeprom_data				eeprom;
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
	oom_driver_port_type_t port_type;
	u64					   present;		/* present status, bit0:port0, bit1:port1 and so on */

	struct sfp_msa_data	  *msa;
	struct sfp_ddm_data	  *ddm;
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
	int i = 0, j = 0, status = -1;
	u8 reg;
	struct sfp_port_data *data = i2c_get_clientdata(client);

	DEBUG_PRINT("Starting sfp present status update");
	mutex_lock(&data->update_lock);
	data->present = 0;

	/* Read present status of port 1~48(SFP port) */
	for (i = 0; i < ARRAY_SIZE(cplds); i++) {
		for (j = 0; j < 4; j++) {
			reg    = 0x3+j;
			status = accton_i2c_cpld_read(cplds[i], reg);

			if (unlikely(status < 0)) {
				dev_dbg(&client->dev, "cpld(0x%x) reg(0x%x) err %d\n", cplds[i], reg, status);
				goto exit;
			}

			if (i == SFP_CPLD1) { /* SFP_CPLD1 */
				data->present |= (u64)status << j*8;
			}
			else if (i == SFP_CPLD2) { /* SFP_CPLD2 */
				switch (j) {
				case 0:
				case 1:
					data->present |= (u64)status << (j*8 + 29);
					break;
				case 2:
					data->present |= ((u64)status & 0x7) << 45;
					break;
				case 3:
					data->present |= ((u64)status & 0x3F) << 48;
					break;
				default:
					break;
				};
			}
		}
	}

	DEBUG_PRINT("Present status = 0x%lx", data->present);
exit:
	mutex_unlock(&data->update_lock);
	return (status < 0) ? ERR_PTR(status) : data;
}

struct i2c_info {
	enum CPLD_ID cid;
	int reg;
};

static struct sfp_port_data* sfp_update_tx_rx_status(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sfp_port_data *data = i2c_get_clientdata(client);
	int i = 0, j = 0;
	int status = -1;
	struct i2c_info tx_fault[]   = {{SFP_CPLD1, 0x7}, {SFP_CPLD1, 0x8},
								    {SFP_CPLD1, 0x9}, {SFP_CPLD1, 0xa},
								    {SFP_CPLD2, 0x7}, {SFP_CPLD2, 0x8},
								    {SFP_CPLD2, 0x9}};
	struct i2c_info tx_disable[] = {{SFP_CPLD1, 0xb}, {SFP_CPLD1, 0xc},
								    {SFP_CPLD1, 0xd}, {SFP_CPLD1, 0xe},
								    {SFP_CPLD2, 0xa}, {SFP_CPLD2, 0xb},
								    {SFP_CPLD2, 0xc}};
	struct i2c_info rx_los[] 	 = {{SFP_CPLD1, 0xf}, {SFP_CPLD1, 0x10},
									{SFP_CPLD1, 0x11},{SFP_CPLD1, 0x12},
									{SFP_CPLD2, 0xd}, {SFP_CPLD2, 0xe},
									{SFP_CPLD2, 0xf}};
	struct i2c_info *tx_rx_data[] = {tx_fault, tx_disable, rx_los};
	
	if (time_before(jiffies, data->msa->last_updated + HZ + HZ / 2) && data->msa->valid) {
		return data;
	}

	DEBUG_PRINT("Starting as5822_54x sfp tx rx status update");
	mutex_lock(&data->update_lock);
	data->msa->valid = 0;
	memset(data->msa->status, 0, sizeof(data->msa->status));

	for (i = 0; i < ARRAY_SIZE(tx_rx_data); i++) {
		for (j = 0; j < ARRAY_SIZE(tx_fault); j++) {
			int cid = tx_rx_data[i][j].cid;
			int reg = tx_rx_data[i][j].reg;

			status = accton_i2c_cpld_read(cplds[cid], reg);

			if (unlikely(status < 0)) {
				dev_dbg(&client->dev, "cpld(0x%x) reg(0x%x) err %d\n", cplds[cid], reg, status);
				goto exit;
			}

			switch (j) {
			case 0 ... 2:
				data->msa->status[i] |= (u64)status << j*8;
				break;
			case 3:
				data->msa->status[i] |= ((u64)status & 0x1F) << j*8;
				break;
			case 4 ... 5:
				data->msa->status[i] |= (u64)status << ((j-4)*8 + 29);
				break;
			case 6:
				data->msa->status[i] |= ((u64)status & 0x7) << 45;
				break;
			default:
				break;
			};
		}
	}

	data->msa->valid = 1;
	data->msa->last_updated = jiffies;

exit:
	mutex_unlock(&data->update_lock);
	return (status < 0) ? ERR_PTR(status) : data;
}

static ssize_t sfp_set_tx_disable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sfp_port_data *data = i2c_get_clientdata(client);
	unsigned short cpld_id = 0;
	u8 cpld_reg = 0, cpld_val = 0, cpld_bit = 0;
	long disable;
	int error;
	u8 tx_disable_regs[] = {0xb, 0xc, 0xd, 0xe, 0xa, 0xb, 0xc};

	if (data->driver_type == DRIVER_TYPE_QSFP) {
		return qsfp_set_tx_disable(dev, da, buf, count);
	}

	error = kstrtol(buf, 10, &disable);
	if (error) {
		return error;
	}

	mutex_lock(&data->update_lock);

	if(data->port < as5822_54x_sfp29) {
		cpld_id = SFP_CPLD1;
		cpld_reg  = tx_disable_regs[data->port / 8];
		cpld_bit  = 1 << (data->port % 8);
	}
	else { /* port 30 ~ 48 */
		cpld_id = SFP_CPLD2;
		cpld_reg  = tx_disable_regs[(data->port - as5822_54x_sfp29) / 8 + 4];
		cpld_bit  = 1 << ((data->port - as5822_54x_sfp30) % 8);
	}

	/* Read current status */
	cpld_val = accton_i2c_cpld_read(cplds[cpld_id], cpld_reg);

	/* Update tx_disable status */
	if (disable) {
		data->msa->status[1] |= BIT_INDEX(data->port);
		cpld_val |= cpld_bit;
	}
	else {
		data->msa->status[1] &= ~BIT_INDEX(data->port);
		cpld_val &= ~cpld_bit;
	}

	DEBUG_PRINT("Write to CPLD(0x%x), REG(0x%x), VAL(0x%x)", cplds[cpld_id], cpld_reg, cpld_val);
	accton_i2c_cpld_write(cplds[cpld_id], cpld_reg, cpld_val);
	mutex_unlock(&data->update_lock);
	return count;
}

static int sfp_is_port_present(struct i2c_client *client, int port)
{
	struct sfp_port_data *data = i2c_get_clientdata(client);

	data = sfp_update_present(client);
	if (IS_ERR(data)) {
		return PTR_ERR(data);
	}

	return !(data->present & BIT_INDEX(data->port)); /* Platform dependent */
}

static ssize_t show_present(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);

	if (PRESENT_ALL == attr->index) {
		int i;
		u8 values[7]  = {0};
		struct sfp_port_data *data = sfp_update_present(client);
		
		if (IS_ERR(data)) {
			return PTR_ERR(data);
		}

		for (i = 0; i < ARRAY_SIZE(values); i++) {
			values[i] = ~(u8)(data->present >> (i * 8));
		}

        /* Return values 1 -> 54 in order */
        return sprintf(buf, "%.2x %.2x %.2x %.2x %.2x %.2x %.2x\n",
                       values[0], values[1], values[2],
                       values[3], values[4], values[5],
                       values[6] & 0x3F);
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

static struct sfp_port_data *sfp_update_port_type(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sfp_port_data *data = i2c_get_clientdata(client);
	u8 buf = 0;
	int status;

	mutex_lock(&data->update_lock);

	switch (data->driver_type) {
		case DRIVER_TYPE_SFP_MSA:
		{
			status = sfp_eeprom_read(client, SFF8024_PHYSICAL_DEVICE_ID_ADDR, &buf, sizeof(buf));
			if (unlikely(status < 0)) {
				data->port_type = OOM_DRIVER_PORT_TYPE_INVALID;
				break;
			}

			if (buf != SFF8024_DEVICE_ID_SFP) {
				data->port_type = OOM_DRIVER_PORT_TYPE_INVALID;
				break;
			}

			status = sfp_eeprom_read(client, SFF8472_10G_ETH_COMPLIANCE_ADDR, &buf, sizeof(buf));
			if (unlikely(status < 0)) {
				data->port_type = OOM_DRIVER_PORT_TYPE_INVALID;
				break;
			}

			DEBUG_PRINT("sfp port type (0x3) data = (0x%x)", buf);
			data->port_type = buf & SFF8472_10G_BASE_MASK ? OOM_DRIVER_PORT_TYPE_SFP_PLUS : OOM_DRIVER_PORT_TYPE_SFP;
			break;
		}
		case DRIVER_TYPE_QSFP:
		{
			status = sfp_eeprom_read(client, SFF8024_PHYSICAL_DEVICE_ID_ADDR, &buf, sizeof(buf));
			if (unlikely(status < 0)) {
				data->port_type = OOM_DRIVER_PORT_TYPE_INVALID;
				break;
			}

			DEBUG_PRINT("qsfp port type (0x0) buf = (0x%x)", buf);
			switch (buf) {
			case SFF8024_DEVICE_ID_QSFP:
				data->port_type = OOM_DRIVER_PORT_TYPE_QSFP;
				break;
			case SFF8024_DEVICE_ID_QSFP_PLUS:
				data->port_type = OOM_DRIVER_PORT_TYPE_QSFP_PLUS;
				break;
			case SFF8024_DEVICE_ID_QSFP28:
				data->port_type = OOM_DRIVER_PORT_TYPE_QSFP_PLUS;
				break;
			default:
				data->port_type = OOM_DRIVER_PORT_TYPE_INVALID;
				break;
			}

			break;
		}
		default:
			break;
	}

	mutex_unlock(&data->update_lock);
	return data;
}

static ssize_t show_port_type(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sfp_port_data *data = i2c_get_clientdata(client);
	int present = sfp_is_port_present(client, data->port);

	if (IS_ERR_VALUE(present)) {
		return present;
	}

	if (!present) {
		return sprintf(buf, "%d\n", OOM_DRIVER_PORT_TYPE_NOT_PRESENT);
	}

	sfp_update_port_type(dev);
	return sprintf(buf, "%d\n", data->port_type);
}

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
		data->qsfp->status[1] = disable & 0xF;
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

static ssize_t sfp_show_ddm_implemented(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int status;
	char ddm;
	struct i2c_client *client = to_i2c_client(dev);
	struct sfp_port_data *data = i2c_get_clientdata(client);

	status = sfp_is_port_present(client, data->port);
	if (IS_ERR_VALUE(status)) {
		return status;
	}

	if (status == 0) {
		/* port is not present */
		return -ENODEV;
	}

	status = sfp_eeprom_read(client, SFF8472_DIAG_MON_TYPE_ADDR, &ddm, sizeof(ddm));
	if (unlikely(status < 0)) {
		return status;
	}

	return sprintf(buf, "%d\n", !!(ddm & SFF8472_DIAG_MON_TYPE_DDM_MASK));
}

/* Platform dependent +++ */
static ssize_t sfp_show_tx_rx_status(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u8 val = 0, index = 0;
	struct i2c_client *client = to_i2c_client(dev);
	struct sfp_port_data *data = i2c_get_clientdata(client);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

	if (data->driver_type == DRIVER_TYPE_QSFP) {
		return qsfp_show_tx_rx_status(dev, da, buf);
	}

	data = sfp_update_tx_rx_status(dev);
	if (IS_ERR(data)) {
		return PTR_ERR(data);
	}

	if(attr->index == RX_LOS_ALL || attr->index == TX_FAULT_ALL ||
	   attr->index == TX_DISABLE_ALL) {
		int i = 0, index = 0;
		u8 values[6] = {0};

		switch (attr->index) {
		case TX_FAULT_ALL:
			index = 0;
			break;
		case TX_DISABLE_ALL:
			index = 1;
			break;
		case RX_LOS_ALL:
			index = 2;
			break;
		default:
			break;
		};

		for (i = 0; i < ARRAY_SIZE(values); i++) {
			values[i] = (u8)(data->msa->status[index] >> (i * 8));
		}
		
		/** Return values 1 -> 48 in order */
		return sprintf(buf, "%.2x %.2x %.2x %.2x %.2x %.2x\n",
						values[0], values[1], values[2],
						values[3], values[4], values[5]);	
	}
	
	switch (attr->index) {
	case TX_FAULT:
		index = 0;
		break;
	case TX_DISABLE:
		index = 1;
		break;
	case RX_LOS:
		index = 2;
		break;
	default:
		return 0;
	}

	val = !!(data->msa->status[index] & BIT_INDEX(data->port));
	return sprintf(buf, "%d\n", val);
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

static const struct attribute_group sfp_msa_group = {
	.attrs = sfp_msa_attributes,
};

static int sfp_i2c_check_functionality(struct i2c_client *client)
{
#if USE_I2C_BLOCK_READ
	return i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK);
#else
	return i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA);
#endif
}

static int sfp_msa_probe(struct i2c_client *client, const struct i2c_device_id *dev_id,
							   struct sfp_msa_data **data)
{
	int status;
	struct sfp_msa_data *msa;

	if (!sfp_i2c_check_functionality(client)) {
		status = -EIO;
		goto exit;
	}

	msa = kzalloc(sizeof(struct sfp_msa_data), GFP_KERNEL);
	if (!msa) {
		status = -ENOMEM;
		goto exit;
	}

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &sfp_msa_group);
	if (status) {
		goto exit_free;
	}

	/* init eeprom */
	status = sfp_sysfs_eeprom_init(&client->dev.kobj, &msa->eeprom.bin);
	if (status) {
		goto exit_remove;
	}

	*data = msa;
	dev_info(&client->dev, "sfp msa '%s'\n", client->name);

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &sfp_msa_group);
exit_free:
	kfree(msa);
exit:

	return status;
}

static const struct attribute_group sfp_ddm_group = {
	.attrs = sfp_ddm_attributes,
};

static int sfp_ddm_probe(struct i2c_client *client, const struct i2c_device_id *dev_id,
							   struct sfp_ddm_data **data)
{
	int status;
	struct sfp_ddm_data *ddm;

	if (!sfp_i2c_check_functionality(client)) {
		status = -EIO;
		goto exit;
	}

	ddm = kzalloc(sizeof(struct sfp_ddm_data), GFP_KERNEL);
	if (!ddm) {
		status = -ENOMEM;
		goto exit;
	}

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &sfp_ddm_group);
	if (status) {
		goto exit_free;
	}

	/* init eeprom */
	status = sfp_sysfs_eeprom_init(&client->dev.kobj, &ddm->eeprom.bin);
	if (status) {
		goto exit_remove;
	}

	*data = ddm;
	dev_info(&client->dev, "sfp ddm '%s'\n", client->name);

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &sfp_ddm_group);
exit_free:
	kfree(ddm);
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

static int sfp_device_probe(struct i2c_client *client,
			const struct i2c_device_id *dev_id)
{
	struct sfp_port_data *data = NULL;

	data = kzalloc(sizeof(struct sfp_port_data), GFP_KERNEL);
	if (!data) {
		return -ENOMEM;
	}

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);
	data->port 	 = dev_id->driver_data;
	data->client = client;

	if (dev_id->driver_data >= as5822_54x_sfp1 && dev_id->driver_data <= as5822_54x_sfp48) {
		if (client->addr == SFP_EEPROM_A0_I2C_ADDR) {
			data->driver_type = DRIVER_TYPE_SFP_MSA;
			return sfp_msa_probe(client, dev_id, &data->msa);
		}
		else if (client->addr == SFP_EEPROM_A2_I2C_ADDR) {
			data->driver_type = DRIVER_TYPE_SFP_DDM;
			return sfp_ddm_probe(client, dev_id, &data->ddm);
		}
	}
	else { /* as5822_54x_sfp49 ~ as5822_54x_sfp54 */
		if (client->addr == SFP_EEPROM_A0_I2C_ADDR) {
			data->driver_type = DRIVER_TYPE_QSFP;
			return qsfp_probe(client, dev_id, &data->qsfp);
		}
	}
	
	return -ENODEV;
}

static int sfp_msa_remove(struct i2c_client *client, struct sfp_msa_data *data)
{
	sfp_sysfs_eeprom_cleanup(&client->dev.kobj, &data->eeprom.bin);
	sysfs_remove_group(&client->dev.kobj, &sfp_msa_group);	
	kfree(data);
	return 0;
}

static int sfp_ddm_remove(struct i2c_client *client, struct sfp_ddm_data *data)
{
	sfp_sysfs_eeprom_cleanup(&client->dev.kobj, &data->eeprom.bin);
	sysfs_remove_group(&client->dev.kobj, &sfp_ddm_group);
	kfree(data);
	return 0;
}

static int qfp_remove(struct i2c_client *client, struct qsfp_data *data)
{
	sfp_sysfs_eeprom_cleanup(&client->dev.kobj, &data->eeprom.bin);
	sysfs_remove_group(&client->dev.kobj, &qsfp_group);
	kfree(data);
	return 0;
}

static int sfp_device_remove(struct i2c_client *client)
{
	struct sfp_port_data *data = i2c_get_clientdata(client);

	switch (data->driver_type) {
		case DRIVER_TYPE_SFP_MSA:
			return sfp_msa_remove(client, data->msa);
		case DRIVER_TYPE_SFP_DDM:
			return sfp_ddm_remove(client, data->ddm);
		case DRIVER_TYPE_QSFP:
			return qfp_remove(client, data->qsfp);
	}

	return 0;
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
MODULE_DESCRIPTION("accton as5822_54x_sfp driver");
MODULE_LICENSE("GPL");

late_initcall(sfp_init);
module_exit(sfp_exit);


