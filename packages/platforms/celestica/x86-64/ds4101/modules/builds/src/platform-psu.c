// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Hardware monitoring driver for Delta TDPS-2000LB A
 *
 * (C) Copyright 2023, Celestica Inc.
 * Author: Nicholas Wu <nicwu@celestica.com>
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
#include <linux/pmbus.h>
#include "pmbus.h"

/*
 * Form TDPS-2000 datasheet the supported sensors format defined as:
 * all sensors: linear mode.
 */
static struct pmbus_driver_info tdps2000_info = {
	.pages = 1,
	.func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		| PMBUS_HAVE_IIN | PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT
		| PMBUS_HAVE_PIN | PMBUS_HAVE_POUT
		| PMBUS_HAVE_FAN12 | PMBUS_HAVE_STATUS_FAN12
		| PMBUS_HAVE_TEMP | PMBUS_HAVE_TEMP2 | PMBUS_HAVE_TEMP3 | PMBUS_HAVE_STATUS_TEMP
		| PMBUS_HAVE_STATUS_INPUT,
};

static int pmbus_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret = -1;

	ret = pmbus_do_probe(client, id, &tdps2000_info);

	return ret;
}
/* user driver datat to pass the grpup */
static const struct i2c_device_id tdps2000_id[] = {
	{"tdps2000", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, tdps2000_id);

/* This is the driver that will be inserted */
static struct i2c_driver tdps2000_driver = {
	.driver = {
			.name = "TDPS-2000LB A",
		},
	.probe = pmbus_probe,
	.remove = pmbus_do_remove,
	.id_table = tdps2000_id,
};

module_i2c_driver(tdps2000_driver);

MODULE_AUTHOR("Nicholas Wu <nicwu@celestica.com>");
MODULE_DESCRIPTION("PMBus driver for Delta TDPS-2000LB A");
MODULE_LICENSE("GPL");
