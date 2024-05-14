/*
 * Hardware monitoring driver for Delta dps1300
 *
 * (C) Copyright 2019, Celestica Inc.
 * Author: Pradchaya Phucharoen <pphuchar@celestica.com>
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/debugfs.h>
#include <linux/of_device.h>
#include <linux/pmbus.h>
#include "pmbus.h"


static int dps1300_read_word_data(struct i2c_client *client, int page, int reg)
{
	/*
	 * This masks commands which are not supported.
	 * PSU advertises that all features are supported,
	 * in reality that unfortunately is not true.
	 * So enable only those that the datasheet confirms.
	 */
	switch (reg) {
	case PMBUS_FAN_COMMAND_1:
	case PMBUS_IOUT_OC_WARN_LIMIT:
	case PMBUS_STATUS_WORD:
	case PMBUS_READ_VIN:
	case PMBUS_READ_IIN:
	case PMBUS_READ_VOUT:
	case PMBUS_READ_IOUT:
	case PMBUS_READ_TEMPERATURE_1:
	case PMBUS_READ_TEMPERATURE_2:
	case PMBUS_READ_TEMPERATURE_3:
	case PMBUS_READ_FAN_SPEED_1:
	case PMBUS_READ_POUT:
	case PMBUS_READ_PIN:
	//case PMBUS_MFR_VOUT_MIN:
	//case PMBUS_MFR_VOUT_MAX:
	//case PMBUS_MFR_IOUT_MAX:
	//case PMBUS_MFR_POUT_MAX:
		return pmbus_read_word_data(client, page, reg);
	default:
		return -ENXIO;
	}
}

/*
 * Form FSP-550 datasheet the supported sensors format defined as:
 * VOUT: linear mode with one decimal place.
 * 
 * Other sensors:
 * IOUT: direct mode with one decimal place.
 * IOUT Limits: linear mode, which difference from IOUT.
 * VIN, IIN, POWER, TEMP, & FAN: linear mode.
 */

static struct pmbus_driver_info dps1300_info = {

	.pages = 1,
    .func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
                 | PMBUS_HAVE_IIN | PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT
                 | PMBUS_HAVE_PIN | PMBUS_HAVE_POUT
                 | PMBUS_HAVE_FAN12 | PMBUS_HAVE_STATUS_FAN12
                 | PMBUS_HAVE_TEMP | PMBUS_HAVE_TEMP2 | PMBUS_HAVE_TEMP3 | PMBUS_HAVE_STATUS_TEMP
                 | PMBUS_HAVE_STATUS_INPUT,
    .format = {
        [PSC_VOLTAGE_OUT] = linear,
        [PSC_CURRENT_OUT] = linear,
    },
	.m = {
        [PSC_VOLTAGE_OUT] = 10,
        [PSC_CURRENT_OUT] = 10,
    },
    .b = {
        [PSC_VOLTAGE_OUT] = 0,
        [PSC_CURRENT_OUT] = 0,
    },
    .R = {
        [PSC_VOLTAGE_OUT] = 0,
        [PSC_CURRENT_OUT] = 0,
    },
    .read_word_data = &dps1300_read_word_data,
};

static int pmbus_probe(struct i2c_client *client,
               const struct i2c_device_id *id)
{	

	int ret = -1;
	ret = pmbus_do_probe(client, id, &dps1300_info);
	printk(KERN_WARNING "pmbus_do_probe ret:%d.\n", ret);
	//dps1300_init_debugfs(data, client);
	return ret;
}
/* user driver datat to pass the grpup */
static const struct i2c_device_id dps1300_id[] = {
    {"platform_psu", 0},
    {}
};

MODULE_DEVICE_TABLE(i2c, dps1300_id);

/* This is the driver that will be inserted */
static struct i2c_driver dps1300_driver = {
    .driver = {
           .name = "platform_psu",
           },
    .probe = pmbus_probe,
    .remove = pmbus_do_remove,
    .id_table = dps1300_id,
};

module_i2c_driver(dps1300_driver);

MODULE_AUTHOR("Pradchaya Phucharoen");
MODULE_DESCRIPTION("PMBus driver for Delta dps1300");
MODULE_LICENSE("GPL");
