
/*
 *
 * Copyright (C) 2017 Delta Networks, Inc.
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#if 0
#include "x86-64-delta-ag8032-i2c-mux-cpld.h"
#else
struct i2c_mux_cpld_platform_data
{
	u8 cpld_bus;
	u8 cpld_addr;
	u8 cpld_reg;

	u8 parent_bus;

	u8 base_nr;

	const u8 *values;
	int n_values;
	bool idle_in_use;
	u8  idle;

	void *ctrl_adap;
};
#endif

static const u8 subbus_mux_values[] = {
	0xe0, 0xe1, 0xe2, 0xe3
};
static struct i2c_mux_cpld_platform_data subbus_mux = {
	.cpld_bus = 0,
	.cpld_addr= 0x2e,
	.cpld_reg = 0x15,

	.parent_bus = 1,

	.values = subbus_mux_values,
	.n_values = 4,
};
static const u8 qsfp_mux_values_1_8[] = {
	 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f
};
static struct i2c_mux_cpld_platform_data qsfp_mux_1_8 = {
	.cpld_bus = 3,
	.cpld_addr= 0x30,
	.cpld_reg = 0x05,

	.parent_bus = 3,

	.values = qsfp_mux_values_1_8,
	.n_values = 8,
	.idle_in_use = 1,
	.idle = 0xff,
};
static const u8 qsfp_mux_values_9_16[] = {
	 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f
};
static struct i2c_mux_cpld_platform_data qsfp_mux_9_16 = {
	.cpld_bus = 3,
	.cpld_addr= 0x30,
	.cpld_reg = 0x04,

	.parent_bus = 3,

	.values = qsfp_mux_values_9_16,
	.n_values = 8,
	.idle_in_use = 1,
	.idle = 0xff,

};

static const u8 qsfp_mux_values_17_24[] = {
	 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f
};
static struct i2c_mux_cpld_platform_data qsfp_mux_17_24 = {
	.cpld_bus = 3,
	.cpld_addr= 0x30,
	.cpld_reg = 0x03,

	.parent_bus = 3,

	.values = qsfp_mux_values_17_24,
	.n_values = 8,
	.idle_in_use = 1,
	.idle = 0xff,

};

static const u8 qsfp_mux_values_25_32[] = {
	 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f
};
static struct i2c_mux_cpld_platform_data qsfp_mux_25_32 = {
	.cpld_bus = 3,
	.cpld_addr= 0x30,
	.cpld_reg = 0x02,

	.parent_bus = 3,

	.values = qsfp_mux_values_25_32,
	.n_values = 8,
	.idle_in_use = 1,
	.idle = 0xff,
};


static int add_ag8032_platform_cpld_mux_devices(void)
{
	struct platform_device *pdev;

	pdev = platform_device_register_simple("i2c-mux-cpld", 1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);
	platform_device_add_data (pdev, &subbus_mux, sizeof(subbus_mux));
	
	pdev = platform_device_register_simple("i2c-mux-cpld", 2, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);
	platform_device_add_data (pdev, &qsfp_mux_1_8, sizeof(qsfp_mux_1_8));

	pdev = platform_device_register_simple("i2c-mux-cpld", 3, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);
	platform_device_add_data (pdev, &qsfp_mux_9_16, sizeof(qsfp_mux_9_16));

	pdev = platform_device_register_simple("i2c-mux-cpld", 4, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);
	platform_device_add_data (pdev, &qsfp_mux_17_24, sizeof(qsfp_mux_17_24));

	pdev = platform_device_register_simple("i2c-mux-cpld", 5, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);
	platform_device_add_data (pdev, &qsfp_mux_25_32, sizeof(qsfp_mux_25_32));

	return 0;
}

static int __init ag8032_platform_init(void)
{
	add_ag8032_platform_cpld_mux_devices ();

        return 0;
}

static void __exit ag8032_platform_exit(void)
{
}

MODULE_AUTHOR("Dave Hu <dave.hu@deltaww.com>");
MODULE_DESCRIPTION("Delta AG8032");
MODULE_LICENSE("GPL");

module_init(ag8032_platform_init);
module_exit(ag8032_platform_exit);


