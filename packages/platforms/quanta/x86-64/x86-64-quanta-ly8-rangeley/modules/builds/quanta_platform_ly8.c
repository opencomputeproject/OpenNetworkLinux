/*
 *  Quanta Switch platform driver
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_data/pca953x.h>
#include <linux/i2c/pca954x.h>
#include <linux/gpio.h>

#include <linux/i2c-mux-gpio.h>
#include <linux/platform_device.h>
#include <linux/dmi.h>

#define DRIVER_NAME			"quanta-platform-ly8"

#define MAX_I2C_CLIENTS	512
#define I2C_GPIO_BASE	0x80
#define XSTR(x) STR(X)
#define STR(x) #x

enum i2c_types {
	i2c_type_spd,
	i2c_type_rtc,
	i2c_type_pca9546,
	i2c_type_pca9548,
	i2c_type_pca9554,
	i2c_type_pca9555,
	i2c_type_pca9698,
	i2c_type_24c02,
	i2c_type_emerson700,
	i2c_type_quanta_ly8_hwmon,
	i2c_type_optoe1_QSFP,
	i2c_type_optoe2_SFP,
};

char *i2c_type_names[] = {
	"spd",
	"ds1339",
	"pca9546",
	"pca9548",
	"pca9554",
	"pca9555",
	"pca9698",
	"24c02",
	"emerson700",
	"quanta_ly8_hwmon",
	"optoe1",
	"optoe2",
};

struct i2c_init_data {
	int parent_bus;
	int type;
	int addr;
	int busno;
	int gpio_base;
	char name[I2C_NAME_SIZE];
};

static struct i2c_init_data quanta_ly8_i2c_init_data[] = {
	{ .parent_bus = (0x00 + 0), .type = i2c_type_pca9546, .addr = 0x71, .busno = 0x02, .name = "PCA9546(CPU)\0" },
	{ .parent_bus = (0x02 + 0), .type = i2c_type_pca9555, .addr = 0x20, .gpio_base = 0x40, .name = "PCA9555_1(CPU)\0" },

	{ .parent_bus = (0x00 + 0), .type = i2c_type_quanta_ly8_hwmon,   .addr = 0x4e, .name = "PSoc\0" },
	{ .parent_bus = (0x00 + 0), .type = i2c_type_spd,     .addr = 0x52, .name = "SPD\0" },
	{ .parent_bus = (0x00 + 0), .type = i2c_type_rtc,     .addr = 0x68, .name = "RTC\0" },
	{ .parent_bus = (0x00 + 0), .type = i2c_type_pca9548, .addr = 0x77, .busno = 0x10, .name = "PCA9548_1\0" },

	{ .parent_bus = (0x10 + 1), .type = i2c_type_pca9548, .addr = 0x73, .busno = 0x20, .name = "PCA9548(SFP_1-8)\0" },
	{ .parent_bus = (0x10 + 1), .type = i2c_type_pca9548, .addr = 0x74, .busno = 0x28, .name = "PCA9548(SFP_9-16)\0" },
	{ .parent_bus = (0x10 + 1), .type = i2c_type_pca9548, .addr = 0x76, .busno = 0x30, .name = "PCA9548(SFP_17-24)\0" },
	{ .parent_bus = (0x10 + 2), .type = i2c_type_pca9548, .addr = 0x73, .busno = 0x38, .name = "PCA9548(SFP_25-32)\0" },
	{ .parent_bus = (0x10 + 2), .type = i2c_type_pca9548, .addr = 0x74, .busno = 0x40, .name = "PCA9548(SFP_33-40)\0" },
	{ .parent_bus = (0x10 + 2), .type = i2c_type_pca9548, .addr = 0x75, .busno = 0x48, .name = "PCA9548(SFP_41-48)\0" },
	{ .parent_bus = (0x10 + 0), .type = i2c_type_pca9554, .addr = 0x25, .name = "PCA9554(PCA9698INT)\0" },
	{ .parent_bus = (0x10 + 0), .type = i2c_type_pca9555, .addr = 0x24, .name = "PCA9555_3(FAN)\0" },
	{ .parent_bus = (0x10 + 0), .type = i2c_type_pca9555, .addr = 0x23, .name = "PCA9555_4(QSFP_EN)\0" },
	{ .parent_bus = (0x20 + 0), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_1_EEPROM\0" },
	{ .parent_bus = (0x20 + 1), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_2_EEPROM\0" },
	{ .parent_bus = (0x20 + 2), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_3_EEPROM\0" },
	{ .parent_bus = (0x20 + 3), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_4_EEPROM\0" },
	{ .parent_bus = (0x20 + 4), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_5_EEPROM\0" },
	{ .parent_bus = (0x20 + 5), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_6_EEPROM\0" },
	{ .parent_bus = (0x20 + 6), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_7_EEPROM\0" },
	{ .parent_bus = (0x20 + 7), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_8_EEPROM\0" },
	{ .parent_bus = (0x28 + 0), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_9_EEPROM\0" },
	{ .parent_bus = (0x28 + 1), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_10_EEPROM\0" },
	{ .parent_bus = (0x28 + 2), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_11_EEPROM\0" },
	{ .parent_bus = (0x28 + 3), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_12_EEPROM\0" },
	{ .parent_bus = (0x28 + 4), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_13_EEPROM\0" },
	{ .parent_bus = (0x28 + 5), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_14_EEPROM\0" },
	{ .parent_bus = (0x28 + 6), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_15_EEPROM\0" },
	{ .parent_bus = (0x28 + 7), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_16_EEPROM\0" },
	{ .parent_bus = (0x30 + 0), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_17_EEPROM\0" },
	{ .parent_bus = (0x30 + 1), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_18_EEPROM\0" },
	{ .parent_bus = (0x30 + 2), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_19_EEPROM\0" },
	{ .parent_bus = (0x30 + 3), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_20_EEPROM\0" },
	{ .parent_bus = (0x30 + 4), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_21_EEPROM\0" },
	{ .parent_bus = (0x30 + 5), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_22_EEPROM\0" },
	{ .parent_bus = (0x30 + 6), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_23_EEPROM\0" },
	{ .parent_bus = (0x30 + 7), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_24_EEPROM\0" },
	{ .parent_bus = (0x38 + 0), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_25_EEPROM\0" },
	{ .parent_bus = (0x38 + 1), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_26_EEPROM\0" },
	{ .parent_bus = (0x38 + 2), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_27_EEPROM\0" },
	{ .parent_bus = (0x38 + 3), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_28_EEPROM\0" },
	{ .parent_bus = (0x38 + 4), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_29_EEPROM\0" },
	{ .parent_bus = (0x38 + 5), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_30_EEPROM\0" },
	{ .parent_bus = (0x38 + 6), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_31_EEPROM\0" },
	{ .parent_bus = (0x38 + 7), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_32_EEPROM\0" },
	{ .parent_bus = (0x40 + 0), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_33_EEPROM\0" },
	{ .parent_bus = (0x40 + 1), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_34_EEPROM\0" },
	{ .parent_bus = (0x40 + 2), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_35_EEPROM\0" },
	{ .parent_bus = (0x40 + 3), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_36_EEPROM\0" },
	{ .parent_bus = (0x40 + 4), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_37_EEPROM\0" },
	{ .parent_bus = (0x40 + 5), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_38_EEPROM\0" },
	{ .parent_bus = (0x40 + 6), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_39_EEPROM\0" },
	{ .parent_bus = (0x40 + 7), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_40_EEPROM\0" },
	{ .parent_bus = (0x48 + 0), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_41_EEPROM\0" },
	{ .parent_bus = (0x48 + 1), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_42_EEPROM\0" },
	{ .parent_bus = (0x48 + 2), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_43_EEPROM\0" },
	{ .parent_bus = (0x48 + 3), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_44_EEPROM\0" },
	{ .parent_bus = (0x48 + 4), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_45_EEPROM\0" },
	{ .parent_bus = (0x48 + 5), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_46_EEPROM\0" },
	{ .parent_bus = (0x48 + 6), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_47_EEPROM\0" },
	{ .parent_bus = (0x48 + 7), .type = i2c_type_optoe2_SFP,   .addr = 0x50, .name = "SFP_48_EEPROM\0" },

	{ .parent_bus = (0x10 + 3), .type = i2c_type_pca9698, .addr = 0x23, .name = "PCA9698(SFP_1-8)\0" },
	{ .parent_bus = (0x10 + 3), .type = i2c_type_pca9698, .addr = 0x21, .name = "PCA9698(SFP_9-16)\0" },
	{ .parent_bus = (0x10 + 3), .type = i2c_type_pca9698, .addr = 0x22, .name = "PCA9698(SFP_17-24)\0" },
	{ .parent_bus = (0x10 + 4), .type = i2c_type_pca9698, .addr = 0x23, .name = "PCA9698(SFP_25-32)\0" },
	{ .parent_bus = (0x10 + 4), .type = i2c_type_pca9698, .addr = 0x24, .name = "PCA9698(SFP_33-40)\0" },
	{ .parent_bus = (0x10 + 4), .type = i2c_type_pca9698, .addr = 0x25, .name = "PCA9698(SFP_41-48)\0" },

	{ .parent_bus = (0x10 + 5), .type = i2c_type_pca9548, .addr = 0x76, .busno = 0x50, .name = "PCA9548_8\0" },
	{ .parent_bus = (0x50 + 0), .type = i2c_type_optoe1_QSFP,   .addr = 0x50, .name = "QSFP_1_EEPROM\0" },
	{ .parent_bus = (0x50 + 1), .type = i2c_type_optoe1_QSFP,   .addr = 0x50, .name = "QSFP_2_EEPROM\0" },
	{ .parent_bus = (0x50 + 2), .type = i2c_type_optoe1_QSFP,   .addr = 0x50, .name = "QSFP_3_EEPROM\0" },
	{ .parent_bus = (0x50 + 3), .type = i2c_type_optoe1_QSFP,   .addr = 0x50, .name = "QSFP_4_EEPROM\0" },

	{ .parent_bus = (0x10 + 5), .type = i2c_type_pca9555, .addr = 0x24, .name = "PCA9555_1(LED)\0" },
	{ .parent_bus = (0x10 + 6), .type = i2c_type_pca9555, .addr = 0x23, .name = "PCA9555_2(QSFP)\0" },

	/* QSFP+ DB */
	{ .parent_bus = (0x10 + 7), .type = i2c_type_pca9555, .addr = 0x23, .name = "PCA9555(QDB)\0" },
	{ .parent_bus = (0x10 + 7), .type = i2c_type_pca9546, .addr = 0x76, .busno = 0x58, .name = "PCA9546(QDB)\0" },
	{ .parent_bus = (0x58 + 0), .type = i2c_type_optoe1_QSFP,   .addr = 0x50, .name = "QDB_QSFP_1_EEPROM\0" },
	{ .parent_bus = (0x58 + 1), .type = i2c_type_optoe1_QSFP,   .addr = 0x50, .name = "QDB_QSFP_2_EEPROM\0" },

	{ .parent_bus = (0x00 + 0), .type = i2c_type_pca9546, .addr = 0x72, .busno = 0x18, .name = "PCA9546\0" },
	{ .parent_bus = (0x18 + 0), .type = i2c_type_emerson700,   .addr = 0x6f, .name = "PSU_1\0" }, /* RPSU 1 */
	{ .parent_bus = (0x18 + 1), .type = i2c_type_emerson700,   .addr = 0x69, .name = "PSU_2\0" }, /* RPSU 2 */
	{ .parent_bus = (0x18 + 2), .type = i2c_type_pca9555, .addr = 0x26, .name = "PCA9555_5(PSU)\0" },
	{ .parent_bus = (0x18 + 3), .type = i2c_type_24c02,   .addr = 0x54, .name = "Board_EEPROM\0" },
};

static inline struct pca954x_platform_data *pca954x_platform_data_get(int type, int busno) {
	static struct pca954x_platform_mode platform_modes[8];
	static struct pca954x_platform_data platform_data;
	int num_modes, i;

	switch(type) {
		case i2c_type_pca9546:
			num_modes = 4;
			break;

		case i2c_type_pca9548:
			num_modes = 8;
			break;

		default:
			return (struct pca954x_platform_data *) NULL;
			break;
	}

	for(i=0;i<num_modes;i++) {
		platform_modes[i] = (struct pca954x_platform_mode) {
			.adap_id = (busno + i),
			.deselect_on_exit = 1,
		};
	}

	platform_data = (struct pca954x_platform_data) {
		.modes = platform_modes,
		.num_modes = num_modes,
	};

	return &platform_data;
}

static int base_gpio_num = I2C_GPIO_BASE;
static inline struct pca953x_platform_data *pca953x_platform_data_get(int type, int gpio_base) {
	static struct pca953x_platform_data platform_data;
	int num_gpios, num_gpio;

	switch(type) {
		case i2c_type_pca9554:
		num_gpios = 0x8;
		break;

		case i2c_type_pca9555:
		num_gpios = 0x10;
		break;

		case i2c_type_pca9698:
		num_gpios = 0x28;
		break;

		default:
		return (struct pca953x_platform_data *) NULL;
		break;
	}

	if(gpio_base == 0) {
		num_gpio = base_gpio_num;
		base_gpio_num += num_gpios;
	}
	else {
		num_gpio = gpio_base;
	}

	platform_data = (struct pca953x_platform_data) {
		.gpio_base = num_gpio,
	};

	return &platform_data;
}

static inline struct i2c_board_info *i2c_board_info_get(struct i2c_init_data data) {
	struct pca954x_platform_data *mux_platform_data;
	struct pca953x_platform_data *gpio_platform_data;
	static struct i2c_board_info board_info;

	switch(data.type) {
		case i2c_type_pca9546:
		case i2c_type_pca9548:
			mux_platform_data = pca954x_platform_data_get(data.type, data.busno);
			if(mux_platform_data == NULL) {
				return (struct i2c_board_info *) NULL;
			}

			board_info = (struct i2c_board_info) {
				.platform_data = mux_platform_data,
			};
			break;

		case i2c_type_pca9554:
		case i2c_type_pca9555:
		case i2c_type_pca9698:
			gpio_platform_data = pca953x_platform_data_get(data.type, data.gpio_base);
			if(gpio_platform_data == NULL) {
				return (struct i2c_board_info *) NULL;
			}

			board_info = (struct i2c_board_info) {
				.platform_data = gpio_platform_data,
			};
			break;
		case i2c_type_optoe1_QSFP:
		case i2c_type_optoe2_SFP:
		case i2c_type_rtc:
		case i2c_type_spd:
		case i2c_type_24c02:
		case i2c_type_emerson700:
		case i2c_type_quanta_ly8_hwmon:
			board_info = (struct i2c_board_info) {
				.platform_data = (void *) NULL,
			};
			break;

		default:
			return (struct i2c_board_info *) NULL;
			break;
	}

	board_info.addr = data.addr;
	strcpy(board_info.type, i2c_type_names[data.type]);

	return &board_info;
}

static struct platform_driver quanta_platfom_ly8_platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static struct i2c_init_data *init_data;
static int init_data_size;
static struct i2c_client *registered_i2c_clients[MAX_I2C_CLIENTS];

static int __init quanta_platfom_ly8_init(void)
{
	char const *vendor, *product;
	struct i2c_adapter *adapter;
	struct i2c_board_info *board_info;
	int i;
	int ret = 0;

	vendor  = dmi_get_system_info(DMI_SYS_VENDOR);
	product = dmi_get_system_info(DMI_PRODUCT_NAME);

	init_data = quanta_ly8_i2c_init_data;
	init_data_size = ARRAY_SIZE(quanta_ly8_i2c_init_data);

	ret = platform_driver_register(&quanta_platfom_ly8_platform_driver);
	if (ret < 0)
		return ret;

    /**
     * Register I2C devices on new buses
     */
	for(i = 0; i < init_data_size; i++) {
		adapter = i2c_get_adapter(init_data[i].parent_bus);
		board_info = i2c_board_info_get(init_data[i]);
		pr_info("register i2c_new_device\n\t%s for bus 0x%x:0x%x. ",
			   init_data[i].name, init_data[i].parent_bus, init_data[i].addr);
		if((registered_i2c_clients[i] = i2c_new_device(adapter, board_info)) == NULL) {
			pr_err("%s: i2c_new_device for bus 0x%x:0x%x failed.",
				   __FUNCTION__, init_data[i].parent_bus, init_data[i].addr);
		}
	}

	return 0;
}

static void __exit quanta_platfom_ly8_cleanup(void)
{
	int i;
    /**
     * Unregister I2C devices
     */
	for(i = 0; i < init_data_size; i++) {
		if(registered_i2c_clients[i] != NULL) {
			i2c_unregister_device(registered_i2c_clients[init_data_size-(i+1)]);
		}
	}
	platform_driver_unregister(&quanta_platfom_ly8_platform_driver);
}

module_init(quanta_platfom_ly8_init);
module_exit(quanta_platfom_ly8_cleanup);

MODULE_AUTHOR("Jonathan Tsai (jonathan.tsai@quantatw.com)");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Quanta Platform LY8");
MODULE_LICENSE("GPL");
