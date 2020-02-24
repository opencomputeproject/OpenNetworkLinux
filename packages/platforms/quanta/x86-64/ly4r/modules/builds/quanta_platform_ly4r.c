/*
 *  Quanta LY4R platform driver
 *
 *
 *  Copyright (C) 2014 Quanta Computer inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/input.h>
#include <linux/input/sparse-keymap.h>
#include <linux/input-polldev.h>
#include <linux/rfkill.h>
#include <linux/slab.h>
#include <linux/i2c/pca954x.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0))
#include <linux/i2c/pca953x.h>
#else
#include <linux/platform_data/pca953x.h>
#endif

#define MUX_INFO(bus, deselect) \
	{.adap_id = bus, .deselect_on_exit = deselect}

static struct pca954x_platform_mode pca9548sfp1_modes[] = {
	MUX_INFO(0x20, 1),
	MUX_INFO(0x21, 1),
	MUX_INFO(0x22, 1),
	MUX_INFO(0x23, 1),
	MUX_INFO(0x24, 1),
};

static struct pca954x_platform_data pca9548sfp1_data = {
	.modes 		= pca9548sfp1_modes,
	.num_modes 	= 5,
};

static struct pca953x_platform_data pca9698_data = {
	.gpio_base = 0x10,
};

static struct i2c_board_info ly4r_i2c_devices[] = {
	{
		I2C_BOARD_INFO("pca9698", 0x20),
		.platform_data = &pca9698_data,
	},
	{
		I2C_BOARD_INFO("24c02", 0x54),
	},
	{
		I2C_BOARD_INFO("pca9548", 0x73),
		.platform_data = &pca9548sfp1_data,
	},
	{
		I2C_BOARD_INFO("24c02", 0x50),
	},
};

static struct platform_driver ly4r_platform_driver = {
	.driver = {
		.name = "qci-ly4r",
		.owner = THIS_MODULE,
	},
};

static struct platform_device *ly4r_device;

static int __init ly4r_platform_init(void)
{
	struct i2c_client *client;
	struct i2c_adapter *adapter;
	int ret;

	ret = platform_driver_register(&ly4r_platform_driver);
	if (ret < 0)
		return ret;

	/* Register platform stuff */
	ly4r_device = platform_device_alloc("qci-ly4r", -1);
	if (!ly4r_device) {
		ret = -ENOMEM;
		goto fail_platform_driver;
	}

	ret = platform_device_add(ly4r_device);
	if (ret)
		goto fail_platform_device;

	adapter = i2c_get_adapter(1);
	client = i2c_new_device(adapter, &ly4r_i2c_devices[2]);		// pca9548sfp_1
	client = i2c_new_device(adapter, &ly4r_i2c_devices[0]);		// pca9698
	client = i2c_new_device(adapter, &ly4r_i2c_devices[1]);		// EEprom
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x20);
	client = i2c_new_device(adapter, &ly4r_i2c_devices[3]);		// sfp_1 EEprom
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x21);
	client = i2c_new_device(adapter, &ly4r_i2c_devices[3]);		// sfp_2 EEprom
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x22);
	client = i2c_new_device(adapter, &ly4r_i2c_devices[3]);		// sfp_3 EEprom
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x23);
	client = i2c_new_device(adapter, &ly4r_i2c_devices[3]);		// sfp_4 EEprom
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x24);
	client = i2c_new_device(adapter, &ly4r_i2c_devices[3]);		// msfp EEprom
	i2c_put_adapter(adapter);

#if 0
	if (!ly4r_device_present) {
		ret = -ENODEV;
		goto fail_no_device;
	}
#endif

	return 0;

fail_platform_device:
	platform_device_put(ly4r_device);

fail_platform_driver:
	platform_driver_unregister(&ly4r_platform_driver);
	return ret;
}

static void __exit ly4r_platform_exit(void)
{
	platform_device_unregister(ly4r_device);
	platform_driver_unregister(&ly4r_platform_driver);
}

module_init(ly4r_platform_init);
module_exit(ly4r_platform_exit);


MODULE_AUTHOR("Jonathan Tsai (jonathan.tsai@quantatw.com)");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Quanta LY4R Platform Driver");
MODULE_LICENSE("GPL");
