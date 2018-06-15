/*
 *  Quanta IX8 platform driver
 *
 *
 *  Copyright (C) 2017 Quanta Computer inc.
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
	MUX_INFO(0x25, 1),
	MUX_INFO(0x26, 1),
	MUX_INFO(0x27, 1),
};

static struct pca954x_platform_data pca9548sfp1_data = {
	.modes 		= pca9548sfp1_modes,
	.num_modes 	= 8,
};

static struct pca954x_platform_mode pca9548sfp2_modes[] = {
	MUX_INFO(0x28, 1),
	MUX_INFO(0x29, 1),
	MUX_INFO(0x2a, 1),
	MUX_INFO(0x2b, 1),
	MUX_INFO(0x2c, 1),
	MUX_INFO(0x2d, 1),
	MUX_INFO(0x2e, 1),
	MUX_INFO(0x2f, 1),
};

static struct pca954x_platform_data pca9548sfp2_data = {
	.modes 		= pca9548sfp2_modes,
	.num_modes 	= 8,
};
static struct pca954x_platform_mode pca9548sfp3_modes[] = {
	MUX_INFO(0x30, 1),
	MUX_INFO(0x31, 1),
	MUX_INFO(0x32, 1),
	MUX_INFO(0x33, 1),
	MUX_INFO(0x34, 1),
	MUX_INFO(0x35, 1),
	MUX_INFO(0x36, 1),
	MUX_INFO(0x37, 1),
};

static struct pca954x_platform_data pca9548sfp3_data = {
	.modes 		= pca9548sfp3_modes,
	.num_modes 	= 8,
};

static struct pca954x_platform_mode pca9548sfp4_modes[] = {
	MUX_INFO(0x38, 1),
	MUX_INFO(0x39, 1),
	MUX_INFO(0x3a, 1),
	MUX_INFO(0x3b, 1),
	MUX_INFO(0x3c, 1),
	MUX_INFO(0x3d, 1),
	MUX_INFO(0x3e, 1),
	MUX_INFO(0x3f, 1),
};

static struct pca954x_platform_data pca9548sfp4_data = {
	.modes 		= pca9548sfp4_modes,
	.num_modes 	= 8,
};

static struct pca954x_platform_mode pca9548sfp5_modes[] = {
	MUX_INFO(0x40, 1),
	MUX_INFO(0x41, 1),
	MUX_INFO(0x42, 1),
	MUX_INFO(0x43, 1),
	MUX_INFO(0x44, 1),
	MUX_INFO(0x45, 1),
	MUX_INFO(0x46, 1),
	MUX_INFO(0x47, 1),
};

static struct pca954x_platform_data pca9548sfp5_data = {
	.modes 		= pca9548sfp5_modes,
	.num_modes 	= 8,
};

static struct pca954x_platform_mode pca9548sfp6_modes[] = {
	MUX_INFO(0x48, 1),
	MUX_INFO(0x49, 1),
	MUX_INFO(0x4a, 1),
	MUX_INFO(0x4b, 1),
	MUX_INFO(0x4c, 1),
	MUX_INFO(0x4d, 1),
	MUX_INFO(0x4e, 1),
	MUX_INFO(0x4f, 1),
};

static struct pca954x_platform_data pca9548sfp6_data = {
	.modes 		= pca9548sfp6_modes,
	.num_modes 	= 8,
};

//ZQSFP
static struct pca954x_platform_mode pca9548sfp7_modes[] = {
	MUX_INFO(0x50, 1),
	MUX_INFO(0x51, 1),
	MUX_INFO(0x52, 1),
	MUX_INFO(0x53, 1),
	MUX_INFO(0x54, 1),
	MUX_INFO(0x55, 1),
	MUX_INFO(0x56, 1),
	MUX_INFO(0x57, 1),
};

static struct pca954x_platform_data pca9548sfp7_data = {
	.modes 		= pca9548sfp7_modes,
	.num_modes 	= 8,
};

// end port

static struct pca954x_platform_mode pca9546_modes[] = {
	MUX_INFO(0x10, 1),
	MUX_INFO(0x11, 1),
	MUX_INFO(0x12, 1),
	MUX_INFO(0x13, 1),
};

static struct pca954x_platform_data pca9546_data = {
	.modes 		= pca9546_modes,
	.num_modes 	= 4,
};

static struct pca954x_platform_mode pca9548_modes[] = {
	MUX_INFO(0x14, 1),
	MUX_INFO(0x15, 1),
	MUX_INFO(0x16, 1),
	MUX_INFO(0x17, 1),
	MUX_INFO(0x18, 1),
	MUX_INFO(0x19, 1),
	MUX_INFO(0x1a, 1),
	MUX_INFO(0x1b, 1),
};

static struct pca954x_platform_data pca9548_data = {
	.modes 		= pca9548_modes,
	.num_modes 	= 8,
};

/* CPU Board i2c device */
static struct pca954x_platform_mode pca9546_cpu_modes[] = {
	MUX_INFO(0x02, 1),
	MUX_INFO(0x03, 1),
	MUX_INFO(0x04, 1),
	MUX_INFO(0x05, 1),
};

static struct pca954x_platform_data pca9546_cpu_data = {
	.modes 		= pca9546_cpu_modes,
	.num_modes 	= 4,
};
//MB Board Data
static struct pca953x_platform_data pca9555_1_data = {
	.gpio_base = 0x10,
};
//QSFP28 49-56 IO Expander
static struct pca953x_platform_data pca9698_2_data = {
	.gpio_base = 0x20,
};
//CPU Board pca9555
static struct pca953x_platform_data pca9555_CPU_data = {
	.gpio_base = 0x48,
};
static struct i2c_board_info ix8_i2c_devices[] = {
	{
		I2C_BOARD_INFO("pca9546", 0x72),		// 0
		.platform_data = &pca9546_data,
	},
	{
		I2C_BOARD_INFO("pca9548", 0x77),		// 1
		.platform_data = &pca9548_data,
	},
	{
		I2C_BOARD_INFO("24c02", 0x54),			// 2 eeprom
	},
	{
		I2C_BOARD_INFO("pca9548", 0x73),		// 3 0x77 ch0
		.platform_data = &pca9548sfp1_data,
	},
	{
		I2C_BOARD_INFO("pca9548", 0x73),		// 4 0x77 ch1
		.platform_data = &pca9548sfp2_data,
	},
	{
		I2C_BOARD_INFO("pca9548", 0x73),		// 5 0x77 ch2
		.platform_data = &pca9548sfp3_data,
	},
	{
		I2C_BOARD_INFO("pca9548", 0x73),		// 6 0x77 ch3
		.platform_data = &pca9548sfp4_data,
	},
	{
		I2C_BOARD_INFO("pca9548", 0x73),		// 7 0x77 ch4
		.platform_data = &pca9548sfp5_data,
	},
	{
		I2C_BOARD_INFO("pca9548", 0x73),		// 8 0x77 ch5
		.platform_data = &pca9548sfp6_data,
	},
	{
		I2C_BOARD_INFO("pca9548", 0x73),		// 9 0x77 ch6
		.platform_data = &pca9548sfp7_data,
	},
	{
		I2C_BOARD_INFO("CPLD-SFP28", 0x38),		// 10 0x72 ch0 CPLD1_:SFP28 1~16
	},
	{
		I2C_BOARD_INFO("CPLD-SFP28", 0x38),		// 11 0x72 ch1 CPLD2_:SFP28 17~32
	},
	{
		I2C_BOARD_INFO("CPLD-SFP28", 0x38),		// 12 0x72 ch2 CPLD_3:SFP28 33~48
	},
	{
		I2C_BOARD_INFO("pca9555", 0x23),		// 13 0x72 ch3 MB Board Data
		.platform_data = &pca9555_1_data,
	},
	{
		I2C_BOARD_INFO("pca9698", 0x21),		// 14 0x72 ch3 QSFP:49~52
		.platform_data = &pca9698_2_data,
	},
	{
		I2C_BOARD_INFO("24c02", 0x50),          // 15 0x50 SFP28, QSFP EEPROM
	},
	{
		I2C_BOARD_INFO("pca9546", 0x71),		// 16
		.platform_data = &pca9546_cpu_data,
	},
	{
		I2C_BOARD_INFO("pca9555", 0x20),		// 17 0x71 ch0 CPU Board Data
		.platform_data = &pca9555_CPU_data,
	},
	{
		I2C_BOARD_INFO("CPLDLED_IX8", 0x3a),	// 18 0x72 ch0 CPLD4 LED function of SFP28 & QSFP28 (Port27~56)
	},
	{
		I2C_BOARD_INFO("CPLDLED_IX8", 0x39),	// 19 0x72 ch0 CPLD6 LED function of SFP28 & QSFP28 (Port1~26)
	},
};

static struct platform_driver ix8_platform_driver = {
	.driver = {
		.name = "qci-ix8",
		.owner = THIS_MODULE,
	},
};

static struct platform_device *ix8_device;

static int __init ix8_platform_init(void)
{
	struct i2c_client *client;
	struct i2c_adapter *adapter;
	int ret, i;

	ret = platform_driver_register(&ix8_platform_driver);
	if (ret < 0)
		return ret;

	/* Register platform stuff */
	ix8_device = platform_device_alloc("qci-ix8", -1);
	if (!ix8_device) {
		ret = -ENOMEM;
		goto fail_platform_driver;
	}

	ret = platform_device_add(ix8_device);
	if (ret)
		goto fail_platform_device;

	adapter = i2c_get_adapter(0);
	client = i2c_new_device(adapter, &ix8_i2c_devices[0]);		// pca9546
	client = i2c_new_device(adapter, &ix8_i2c_devices[1]);		// pca9548
	client = i2c_new_device(adapter, &ix8_i2c_devices[16]);		// pca9546cpu
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x02);
	client = i2c_new_device(adapter, &ix8_i2c_devices[17]);		// CPU Board Data
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x10);
	client = i2c_new_device(adapter, &ix8_i2c_devices[10]);		// CPLD_1
	client = i2c_new_device(adapter, &ix8_i2c_devices[18]);		// CPLD_4
	client = i2c_new_device(adapter, &ix8_i2c_devices[19]);		// CPLD_6
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x11);
	client = i2c_new_device(adapter, &ix8_i2c_devices[11]);		// CPLD_2
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x12);
	client = i2c_new_device(adapter, &ix8_i2c_devices[12]);		// CPLD_3
	client = i2c_new_device(adapter, &ix8_i2c_devices[2]);		// MB_BOARDINFO_EEPROM
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x13);
	client = i2c_new_device(adapter, &ix8_i2c_devices[13]);		// MB Board Data
	client = i2c_new_device(adapter, &ix8_i2c_devices[14]);		// QSFP:49~52
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x14);
	client = i2c_new_device(adapter, &ix8_i2c_devices[3]);		// pca9548_1 SFP
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x15);
	client = i2c_new_device(adapter, &ix8_i2c_devices[4]);		// pca9548_2 SFP
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x16);
	client = i2c_new_device(adapter, &ix8_i2c_devices[5]);		// pca9548_3 SFP
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x17);
	client = i2c_new_device(adapter, &ix8_i2c_devices[6]);		// pca9548_4 SFP
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x18);
	client = i2c_new_device(adapter, &ix8_i2c_devices[7]);		// pca9548_5 SFP
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x19);
	client = i2c_new_device(adapter, &ix8_i2c_devices[8]);		// pca9548_6 SFP
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter(0x1a);
	client = i2c_new_device(adapter, &ix8_i2c_devices[9]);		// pca9548_7 QSFP
	i2c_put_adapter(adapter);

	for(i = 32; i < 88; i ++){									// SFP28 1~48 & QSFP 49~56 EEPROM
		adapter = i2c_get_adapter(i);
		client = i2c_new_device(adapter, &ix8_i2c_devices[15]);
		i2c_put_adapter(adapter);
	}

	return 0;

fail_platform_device:
	platform_device_put(ix8_device);

fail_platform_driver:
	platform_driver_unregister(&ix8_platform_driver);
	return ret;
}

static void __exit ix8_platform_exit(void)
{
	platform_device_unregister(ix8_device);
	platform_driver_unregister(&ix8_platform_driver);
}

module_init(ix8_platform_init);
module_exit(ix8_platform_exit);


MODULE_AUTHOR("Jonathan Tsai <jonathan.tsai@quantatw.com>");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Quanta IX8 Platform Driver");
MODULE_LICENSE("GPL");
