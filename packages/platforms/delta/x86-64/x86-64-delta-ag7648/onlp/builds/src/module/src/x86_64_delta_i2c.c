/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc
 *           Copyright 2017 Delta Networks, Inc
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************/
 
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "x86_64_delta_ag7648_log.h"
#include "x86_64_delta_i2c.h"
#include "x86_64_delta_i2c_dev.h"

struct i2c_device_info i2c_device_list[]={
	{"RTC",0X0,0X69},
	{"TMP1_CLOSE_TO_CPU",0X2,0X4d},
	{"TMP1_CLOSE_TO_MAC",0X3,0X4c},
 	{"TMP2_CLOSE_TO_SFP_PLUS",0X3,0X4d},
 	{"TMP3_CLOSE_TO_QSFP",0X3,0X4E},
	{"SYSCPLD",0X2,0X31},
	{"MASTERCPLD",0X2,0X32},
	{"SLAVECPLD",0X2,0X33},
 	{"FAN1EEPROM",0X3,0X51},
 	{"FAN2EEPROM",0X3,0X52},
 	{"FAN3EEPROM",0X3,0X53},
 	{"FANCTRL1",0X3,0X2A},
 	{"FANCTRL2",0X3,0X29},
	{"CURT_MONTOR",0X1,0X40},
	{"ID_EEPROM",0X2,0X53},
    {"SFP1",0XA,0X50},
    {"SFP2",0XB,0X50},
    {"SFP3",0XC,0X50},
    {"SFP4",0XD,0X50},
    {"SFP5",0XE,0X50},
    {"SFP6",0XF,0X50},
    {"SFP7",0X10,0X50},
    {"SFP8",0X11,0X50},
    {"SFP9",0X12,0X50},
    {"SFP10",0X13,0X50},
    {"SFP11",0X14,0X50},
    {"SFP12",0X15,0X50},
    {"SFP13",0X16,0X50},
    {"SFP14",0X17,0X50},
    {"SFP15",0X18,0X50},
    {"SFP16",0X19,0X50},
    {"SFP17",0X1A,0X50},
    {"SFP18",0X1B,0X50},
    {"SFP19",0X1C,0X50},
    {"SFP20",0X1D,0X50},
    {"SFP21",0X1E,0X50},
    {"SFP22",0X1F,0X50},
    {"SFP23",0X20,0X50},
    {"SFP24",0X21,0X50},
    {"SFP25",0X22,0X50},
    {"SFP26",0X23,0X50},
    {"SFP27",0X24,0X50},
    {"SFP28",0X25,0X50},
    {"SFP29",0X26,0X50},
    {"SFP30",0X27,0X50},
    {"SFP31",0X28,0X50},
    {"SFP32",0X29,0X50},
    {"SFP33",0X2A,0X50},
    {"SFP34",0X2B,0X50},
    {"SFP35",0X2C,0X50},
    {"SFP36",0X2D,0X50},
    {"SFP37",0X2E,0X50},
    {"SFP38",0X2F,0X50},
    {"SFP39",0X30,0X50},
    {"SFP40",0X31,0X50},
    {"SFP41",0X32,0X50},
    {"SFP42",0X33,0X50},
    {"SFP43",0X34,0X50},
    {"SFP44",0X35,0X50},
    {"SFP45",0X36,0X50},
    {"SFP46",0X37,0X50},
    {"SFP47",0X38,0X50},
    {"SFP48",0X39,0X50},
    {"QSFP49",0X3A,0X50},
    {"QSFP50",0X3B,0X50},
    {"QSFP51",0X3C,0X50},
    {"QSFP52",0X3D,0X50},
    {"QSFP53",0X3E,0X50},
    {"QSFP54",0X3F,0X50},
//	-------------------------
    {"PSU1_PMBUS",0X6,0X58},
    {"PSU2_PMBUS",0X6,0X59},
	{"PSU1_EEPROM",0X6,0X50},
	{"PSU2_EEPROM",0X6,0X51},

    {NULL,  -1,-1},
};

#define I2C_DATA_B	1
#define I2C_DATA_W	2
#define I2C_DATA_C	3
#define I2C_DATA_QUICK  4


static pthread_mutex_t i2c_mutex = PTHREAD_MUTEX_INITIALIZER;

void I2C_PROTECT (void)
{
	pthread_mutex_lock (&i2c_mutex);
}

void I2C_UNPROTECT (void)
{
	pthread_mutex_unlock (&i2c_mutex);
}

static int open_i2cbus_dev(int i2cbus, char *filename,  int quiet)
{
	int file;

	sprintf(filename, "/dev/i2c-%d", i2cbus);
	file = open(filename, O_RDWR);

	if (file < 0 && !quiet) {
		if (errno == ENOENT) {
			fprintf(stderr, "Error: Could not open file "
				"`/dev/i2c-%d: %s'\n",i2cbus, strerror(ENOENT));
		} else {
			fprintf(stderr, "Error: Could not open file "
				"`%s': %s\n", filename, strerror(errno));
			if (errno == EACCES)
				fprintf(stderr, "Run as root?\n");
		}
	}

	return file;
}

static int set_slave_addr(int file, int address, int force)
{
	/* hack */
	force = 1; /* force always, it will break th i2c driver's behave */


	/* With force, let the user read from/write to the registers
	   even when a driver is also running */
	if (ioctl(file, force ? I2C_SLAVE_FORCE : I2C_SLAVE, address) < 0) {
		if (errno != EBUSY) {
		fprintf(stderr,
			"Error: Could not set address to 0x%02x: %s\n",
			address, strerror(errno));
		return -errno;
		}
	}

	return 0;
}

static int i2c_dev_read (int i2cbus, int dev, int reg, int mode)
{
	int file;
	char filename[20];
	int force = 0;
	int res = 0;

	file = open_i2cbus_dev(i2cbus, filename, 0);
	if (file < 0) return -1;

	if (set_slave_addr(file, dev, force))return -1;
	
	switch (mode) {
	case I2C_DATA_W:
		res = i2c_smbus_read_word_data(file, reg);
		break;
	case I2C_DATA_C:
		if (reg >= 0) {
			res = i2c_smbus_write_byte(file, reg);
			if (res < 0) break;
		}
		res = i2c_smbus_read_byte(file);
		break;
    default:
        res = i2c_smbus_read_byte_data(file, reg);
        break;
	}
	close(file);
	
	return res;
}

static int i2c_dev_write (int i2cbus, int dev, int reg, int value, int mode)
{
	int file;
	char filename[20];
	int force = 0;
	int res = 0;

	file = open_i2cbus_dev(i2cbus, filename, 0);
	if (file < 0) return -1;

	if (set_slave_addr(file, dev, force)) return -1;
	
	switch (mode) {
	case I2C_DATA_W:
		res = i2c_smbus_write_word_data(file, reg, value);
		break;
	case I2C_DATA_C:
		res = i2c_smbus_write_byte (file, reg);
		break;
    case I2C_DATA_QUICK:
        res = i2c_smbus_write_quick(file, I2C_SMBUS_WRITE);
        break;
	default:
		res = i2c_smbus_write_byte_data(file, reg, value);
		break;
	
	}
	close(file);
	usleep (5000);

	return res;
}

static int i2c_block_read (int i2cbus, int dev, int reg, char *buff, int buff_len)
{
	int file;
	char filename[20];
	int force = 0;
	int res = 0;
	
	file = open_i2cbus_dev(i2cbus, filename, 0);
	
	if (file < 0) return -1;
	
	if (set_slave_addr(file, dev, force)) return -1;
	
	res = i2c_smbus_read_i2c_block_data (file, reg, buff_len, (__u8 *)buff);
	
	close(file);

	return res;
}

static int i2c_block_write (int i2cbus, int dev, int reg, char *buff, int buff_len)
{
	int file;
	char filename[20];
	int force = 0;
	int res = 0;

	file = open_i2cbus_dev(i2cbus, filename, 0);
	
	if (file < 0) return -1;

	if (set_slave_addr(file, dev, force)) return -1;

	res = i2c_smbus_write_i2c_block_data (file, reg, buff_len, (__u8 *)buff);
	
	close(file);

	return res;
}

i2c_device_info_t *i2c_dev_find_by_name (char *name)
{
	i2c_device_info_t *i2c_dev = i2c_device_list;

	if (name == NULL) return NULL;

	while (i2c_dev->name) {
		if (strcmp (name, i2c_dev->name) == 0) break;
		++ i2c_dev;
	}
	if (i2c_dev->name == NULL) return NULL;

	return i2c_dev;
}

int i2c_devname_read_byte  (char *name, int reg)
{	
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);

    
	if(i2c_dev==NULL) return -1;

	I2C_PROTECT();
	
	ret=i2c_dev_read (i2c_dev->i2cbus, i2c_dev->addr, reg,  I2C_DATA_B);	

	I2C_UNPROTECT();

	return ret;
}

int i2c_devname_write_byte (char *name, int reg, int value)
{
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);
	
	 if(i2c_dev==NULL) return -1;

	I2C_PROTECT();
	
	ret=i2c_dev_write (i2c_dev->i2cbus, i2c_dev->addr, reg, value, I2C_DATA_B);

	I2C_UNPROTECT();

	return ret;
}

int i2c_devname_read_word  (char *name, int reg)
{	
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);

	if(i2c_dev==NULL) return -1;

	I2C_PROTECT();
	
	ret=i2c_dev_read (i2c_dev->i2cbus, i2c_dev->addr, reg,  I2C_DATA_W);	

	I2C_UNPROTECT();

	return ret;
}

int i2c_devname_write_word (char *name, int reg, int value)
{
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);
	
	if(i2c_dev==NULL) return -1;

	I2C_PROTECT();
	
	ret=i2c_dev_write (i2c_dev->i2cbus, i2c_dev->addr, reg, value, I2C_DATA_W);

	I2C_UNPROTECT();

	return ret;
}

int i2c_devname_read_block (char *name, int reg, char *buff, int buff_size)
{	
	int ret = -1;
	
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);
		
	if(i2c_dev==NULL) return -1;
	
	I2C_PROTECT();
	
	ret = i2c_block_read (i2c_dev->i2cbus, i2c_dev->addr, reg, buff, buff_size);

	I2C_UNPROTECT();

	return ret;

}

int i2c_devname_write_block (char *name, int reg, char *buff, int buff_size)
{	
	int ret = -1;
	
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);
		
	if(i2c_dev==NULL) return -1;
	
	I2C_PROTECT();

	ret = i2c_block_write (i2c_dev->i2cbus, i2c_dev->addr, reg, buff, buff_size);
	
	I2C_UNPROTECT();

	return ret;

}
