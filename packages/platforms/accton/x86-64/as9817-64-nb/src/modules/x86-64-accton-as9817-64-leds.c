// SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0
/*
 * A LED driver for the accton_as9716_32d_led
 *
 * Copyright (C) 2014 Accton Technology Corporation.
 * Roger Ho <roger530_ho@edge-core>
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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include <linux/dmi.h>

extern int as9817_64_cpld_read(u8 cpld_addr, u8 reg);
extern int as9817_64_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

extern void led_classdev_unregister(struct led_classdev *led_cdev);
extern void led_classdev_resume(struct led_classdev *led_cdev);
extern void led_classdev_suspend(struct led_classdev *led_cdev);

#define DRVNAME "as9817_64_led"
struct platform_device *pdev = NULL;

/* 
 * LED related data 
 */
#define LED_CNTRLER_I2C_ADDRESS		(0x60)

#define SYS_LED_BLINKING_REG1		(0x92)
#define SYS_LED_BLINKING_REG2		(0x93)
#define SYS_LED_BLINKING_REG3		(0x94)

#define LED_TYPE_LOC_RED_BLINK		(0x01)
#define LED_TYPE_LOC_GREEN_BLINK	(0x80)
#define LED_TYPE_LOC_BLUE_BLINK		(0x40)

#define LED_TYPE_FAN_RED_BLINK		(0x08)
#define LED_TYPE_FAN_GREEN_BLINK	(0x04)
#define LED_TYPE_FAN_BLUE_BLINK		(0x02)

#define LED_TYPE_DIAG_RED_BLINK		(0x40)
#define LED_TYPE_DIAG_GREEN_BLINK	(0x20)
#define LED_TYPE_DIAG_BLUE_BLINK	(0x10)

#define LED_TYPE_ALARM_RED_BLINK	(0x02)
#define LED_TYPE_ALARM_GREEN_BLINK	(0x01)
#define LED_TYPE_ALARM_BLUE_BLINK	(0x80)

#define LED_TYPE_PSU1_RED_BLINK		(0x04)
#define LED_TYPE_PSU1_GREEN_BLINK	(0x02)
#define LED_TYPE_PSU1_BLUE_BLINK	(0x01)

#define LED_TYPE_PSU2_RED_BLINK		(0x20)
#define LED_TYPE_PSU2_GREEN_BLINK	(0x10)
#define LED_TYPE_PSU2_BLUE_BLINK	(0x08)

enum led_type {
	LED_TYPE_DIAG,
	LED_TYPE_LOC,
	LED_TYPE_FAN,
	LED_TYPE_ALARM,
	LED_TYPE_PSU1,
	LED_TYPE_PSU2
};

enum led_light_mode {
	LED_MODE_OFF,
	LED_MODE_RED = 10,
	LED_MODE_RED_BLINKING = 11,
	LED_MODE_ORANGE = 12,
	LED_MODE_ORANGE_BLINKING = 13,
	LED_MODE_YELLOW = 14,
	LED_MODE_YELLOW_BLINKING = 15,
	LED_MODE_GREEN = 16,
	LED_MODE_GREEN_BLINKING = 17,
	LED_MODE_BLUE = 18,
	LED_MODE_BLUE_BLINKING = 19,
	LED_MODE_PURPLE = 20,
	LED_MODE_PURPLE_BLINKING = 21,
	LED_MODE_AUTO = 22,
	LED_MODE_AUTO_BLINKING = 23,
	LED_MODE_WHITE = 24,
	LED_MODE_WHITE_BLINKING = 25,
	LED_MODE_CYAN = 26,
	LED_MODE_CYAN_BLINKING = 27,
	LED_MODE_UNKNOWN = 99
};

enum RGB_COLOR {
	RGB_RED = 0,
	RGB_GREEN,
	RGB_BLUE,
	RGB_COUNT
};

struct color_reg {
	u8 reg_addr;
	u8 reg_val;
};

struct led_reg {
	enum led_type type;
	enum led_light_mode mode;
	struct color_reg rgb[RGB_COUNT];
};

/**
 * Defines control settings for different LED types and modes.
 *
 * Each entry specifies a register address and value for a 
 * specific LED behavior, including color.
 */
static const struct led_reg led_reg_map[] = {
	/*  Type			 Mode */
	{LED_TYPE_DIAG, LED_MODE_RED,
	    {{0x8E, 0xFF}, {0x8D, 0x00}, {0x8C, 0x00}}}, /* R G B	  */
	{LED_TYPE_DIAG, LED_MODE_GREEN,
	    {{0x8E, 0x00}, {0x8D, 0xFF}, {0x8C, 0x00}}},
	{LED_TYPE_DIAG, LED_MODE_OFF,
	    {{0x8E, 0x00}, {0x8D, 0x00}, {0x8C, 0x00}}},

	{LED_TYPE_LOC, LED_MODE_BLUE_BLINKING,
	    {{0x88, 0x00}, {0x87, 0x00}, {0x86, 0xFF}}},
	{LED_TYPE_LOC, LED_MODE_OFF,
	    {{0x88, 0x00}, {0x87, 0x00}, {0x86, 0x00}}},

	{LED_TYPE_FAN, LED_MODE_RED,
	    {{0x8B, 0xFF}, {0x8A, 0x00}, {0x89, 0x00}}},
	{LED_TYPE_FAN, LED_MODE_GREEN,
	    {{0x8B, 0x00}, {0x8A, 0xFF}, {0x89, 0x00}}},
	{LED_TYPE_FAN, LED_MODE_OFF,
	    {{0x8B, 0x00}, {0x8A, 0x00}, {0x89, 0x00}}},

	{LED_TYPE_ALARM, LED_MODE_RED,
	    {{0x91, 0xFF}, {0x90, 0x00}, {0x8F, 0x00}}},
	{LED_TYPE_ALARM, LED_MODE_OFF,
	    {{0x91, 0x00}, {0x90, 0x00}, {0x8F, 0x00}}},

	{LED_TYPE_PSU1, LED_MODE_RED,
	    {{0x82, 0xFF}, {0x81, 0x00}, {0x80, 0x00}}},
	{LED_TYPE_PSU1, LED_MODE_GREEN,
	    {{0x82, 0x00}, {0x81, 0xFF}, {0x80, 0x00}}},
	{LED_TYPE_PSU1, LED_MODE_OFF,
	    {{0x82, 0x00}, {0x81, 0x00}, {0x80, 0x00}}},

	{LED_TYPE_PSU2, LED_MODE_RED,
	    {{0x85, 0xFF}, {0x84, 0x00}, {0x83, 0x00}}},
	{LED_TYPE_PSU2, LED_MODE_GREEN,
	    {{0x85, 0x00}, {0x84, 0xFF}, {0x83, 0x00}}},
	{LED_TYPE_PSU2, LED_MODE_OFF,
	    {{0x85, 0x00}, {0x84, 0x00}, {0x83, 0x00}}}
};

struct blinking_status {
	u8 en;
	u8 reg_addr;
	u8 mask;
};
struct led_blinking_type_info {
	enum led_type type;
	enum led_light_mode mode;
	struct blinking_status blinking[RGB_COUNT];
};

/**
 * Defines LED blinking configurations.
 *
 * Maps LED types and blinking modes to register addresses and 
 * mask values for controlling blinking behavior.
 */
static const struct led_blinking_type_info led_blinking_type_table[] = {
	{			/*  Type      ,        Mode  */
	 LED_TYPE_DIAG, LED_MODE_OFF, {
				       {0, SYS_LED_BLINKING_REG2, LED_TYPE_DIAG_RED_BLINK},	  /* R */
				       {0, SYS_LED_BLINKING_REG2, LED_TYPE_DIAG_GREEN_BLINK}, /* G */
				       {0, SYS_LED_BLINKING_REG2, LED_TYPE_DIAG_BLUE_BLINK}	  /* B */
				       }
	 },
	{
	 LED_TYPE_LOC, LED_MODE_BLUE_BLINKING, {
						{0, SYS_LED_BLINKING_REG2, LED_TYPE_LOC_RED_BLINK},
						{0, SYS_LED_BLINKING_REG1, LED_TYPE_LOC_GREEN_BLINK},
						{1, SYS_LED_BLINKING_REG1, LED_TYPE_LOC_BLUE_BLINK}
						}
	 },
	{
	 LED_TYPE_LOC, LED_MODE_OFF, {
				      {0, SYS_LED_BLINKING_REG2, LED_TYPE_LOC_RED_BLINK},
				      {0, SYS_LED_BLINKING_REG1, LED_TYPE_LOC_GREEN_BLINK},
				      {0, SYS_LED_BLINKING_REG1, LED_TYPE_LOC_BLUE_BLINK}
				      }
	 },
	{
	 LED_TYPE_FAN, LED_MODE_OFF, {
				      {0, SYS_LED_BLINKING_REG2, LED_TYPE_FAN_RED_BLINK},
				      {0, SYS_LED_BLINKING_REG2, LED_TYPE_FAN_GREEN_BLINK},
				      {0, SYS_LED_BLINKING_REG2, LED_TYPE_FAN_BLUE_BLINK}
				      }
	 },
	{
	 LED_TYPE_ALARM, LED_MODE_OFF, {
					{0, SYS_LED_BLINKING_REG3, LED_TYPE_ALARM_RED_BLINK},
					{0, SYS_LED_BLINKING_REG3, LED_TYPE_ALARM_GREEN_BLINK},
					{0, SYS_LED_BLINKING_REG2, LED_TYPE_ALARM_BLUE_BLINK}
				      }
	 },
	{
	 LED_TYPE_PSU1, LED_MODE_OFF, {
					{0, SYS_LED_BLINKING_REG1, LED_TYPE_PSU1_RED_BLINK},
					{0, SYS_LED_BLINKING_REG1, LED_TYPE_PSU1_GREEN_BLINK},
					{0, SYS_LED_BLINKING_REG1, LED_TYPE_PSU1_BLUE_BLINK}
				      }
	 },
	{
	 LED_TYPE_PSU2, LED_MODE_OFF, {
					{0, SYS_LED_BLINKING_REG1, LED_TYPE_PSU2_RED_BLINK},
					{0, SYS_LED_BLINKING_REG1, LED_TYPE_PSU2_GREEN_BLINK},
					{0, SYS_LED_BLINKING_REG1, LED_TYPE_PSU2_BLUE_BLINK}
				      }
	 },
};

static int as9817_64_led_read_value(u8 reg)
{
	return as9817_64_cpld_read(LED_CNTRLER_I2C_ADDRESS, reg);
}

static int as9817_64_led_write_value(u8 reg, u8 value)
{
	return as9817_64_cpld_write(LED_CNTRLER_I2C_ADDRESS, reg, value);
}

/**
 * Determines if the specified LED type and mode are set to blink.
 *
 * Iterates through a predefined table to check if the LED's current 
 * setting matches the requested blinking mode by reading the LED 
 * controller register. 
 * If a match is found, the LED is considered blinking in that mode.
 *
 * @param type The LED type to check.
 * @param mode The LED mode to verify for blinking.
 * @return The blinking mode if active, otherwise LED_MODE_UNKNOWN.
 */
static enum led_light_mode as9817_64_led_blinking(enum led_type type,
						  enum led_light_mode mode)
{
	const struct led_blinking_type_info *info = NULL;
	u8 val;
	int i, j;
	int result = LED_MODE_UNKNOWN;

	for (i = 0; i < ARRAY_SIZE(led_blinking_type_table); i++) {
		info = &led_blinking_type_table[i];
		if (info->type != type) {
			continue;
		}
		if (info->mode != mode) {
			continue;
		}

		for (j = 0; j < ARRAY_SIZE(info->blinking); j++) {
			if (!info->blinking[j].en) {
				continue;
			}

			val = as9817_64_led_read_value(info->blinking[j].
						     reg_addr);

			if ((val & info->blinking[j].mask) != 0) {
				result = info->mode;
			}
		}
	}

	return result;
}

/*
 * Sets the brightness (state) of a specific LED.
 * This function is a generalized interface for setting 
 * the brightness or state (e.g., blinking) of a given LED.
 *
 * @type: The specific LED type to be controlled.
 * @led_light_mode: The desired state or brightness level to set for the LED.
 */
static void as9817_64_led_set(enum led_type type, enum led_light_mode mode)
{
	int i, j;
	u8 val;

	for (i = 0; i < ARRAY_SIZE(led_reg_map); i++) {
		if (led_reg_map[i].type != type) {
			continue;
		}
		if (led_reg_map[i].mode != mode) {
			continue;
		}

		for (j = 0; j < ARRAY_SIZE(led_reg_map[i].rgb); j++) {
			as9817_64_led_write_value(led_reg_map[i].rgb[j]. reg_addr,
							led_reg_map[i].rgb[j].reg_val);
		}
	}

	for (i = 0; i < ARRAY_SIZE(led_blinking_type_table); i++) {
		if (led_blinking_type_table[i].type != type) {
			continue;
		}
		if (led_blinking_type_table[i].mode != mode) {
			continue;
		}

		for (j = 0; j < ARRAY_SIZE(led_blinking_type_table[i].blinking); j++) {
			val = as9817_64_led_read_value(led_blinking_type_table[i].blinking[j].reg_addr);
			if (led_blinking_type_table[i].blinking[j].en) {
				val |= led_blinking_type_table[i].blinking[j].mask;
			} else {
				val &= ~(led_blinking_type_table[i].blinking[j].mask);
			}
			as9817_64_led_write_value(led_blinking_type_table[i]. blinking[j].reg_addr, val);
		}
	}
}

/*
 * Gets the current state of a specific LED.
 * This function queries the current state or brightness of 
 * a given LED by reading the relevant device registers.
 *
 * @type: The specific LED type to be queried.
 * 
 * Return: The current state or brightness of the LED.
 */
static enum led_brightness as9817_64_led_get(enum led_type type)
{
	u8 val, match;
	int i, j;
	enum led_light_mode res = LED_MODE_OFF;

	for (i = 0; i < ARRAY_SIZE(led_reg_map); i++) {
		if (led_reg_map[i].type != type) {
			continue;
		}

		match = 0;
		for (j = 0; j < ARRAY_SIZE(led_reg_map[i].rgb); j++) {
			val = as9817_64_led_read_value(led_reg_map[i].rgb[j].reg_addr);
			if (val == led_reg_map[i].rgb[j].reg_val) {
				match++;
			}
		}

		if (match == RGB_COUNT) {
			enum led_light_mode tmp = LED_MODE_UNKNOWN;

			res = led_reg_map[i].mode;

			tmp = as9817_64_led_blinking(led_reg_map[i].type, res);
			res = (tmp != LED_MODE_UNKNOWN) ? tmp : res;
			break;
		}
	}

	return res;
}

static void as9817_64_led_diag_set(struct led_classdev *led_cdev,
				   enum led_brightness led_light_mode)
{
	as9817_64_led_set(LED_TYPE_DIAG, led_light_mode);
}

static enum led_brightness as9817_64_led_diag_get(struct led_classdev *cdev)
{
	return as9817_64_led_get(LED_TYPE_DIAG);
}

static void as9817_64_led_loc_set(struct led_classdev *led_cdev,
				  enum led_brightness led_light_mode)
{
	as9817_64_led_set(LED_TYPE_LOC, led_light_mode);
}

static enum led_brightness as9817_64_led_loc_get(struct led_classdev *cdev)
{
	return as9817_64_led_get(LED_TYPE_LOC);
}

static void as9817_64_led_fan_set(struct led_classdev *led_cdev,
				  enum led_brightness led_light_mode)
{
	as9817_64_led_set(LED_TYPE_FAN, led_light_mode);
}

static enum led_brightness as9817_64_led_fan_get(struct led_classdev *cdev)
{
	return as9817_64_led_get(LED_TYPE_FAN);
}

static void as9817_64_led_alarm_set(struct led_classdev *led_cdev,
				    enum led_brightness led_light_mode)
{
	as9817_64_led_set(LED_TYPE_ALARM, led_light_mode);
}

static enum led_brightness as9817_64_led_alarm_get(struct led_classdev *cdev)
{
	return as9817_64_led_get(LED_TYPE_ALARM);
}

static void as9817_64_led_psu1_set(struct led_classdev *led_cdev,
				   enum led_brightness led_light_mode)
{
	as9817_64_led_set(LED_TYPE_PSU1, led_light_mode);
}

static enum led_brightness as9817_64_led_psu1_get(struct led_classdev *cdev)
{
	return as9817_64_led_get(LED_TYPE_PSU1);
}

static void as9817_64_led_psu2_set(struct led_classdev *led_cdev,
				   enum led_brightness led_light_mode)
{
	as9817_64_led_set(LED_TYPE_PSU2, led_light_mode);
}

static enum led_brightness as9817_64_led_psu2_get(struct led_classdev *cdev)
{
	return as9817_64_led_get(LED_TYPE_PSU2);
}

static struct led_classdev as9817_64_leds[] = {
	[LED_TYPE_DIAG] = {
			   .name = "as9817_64_led::diag",
			   .default_trigger = "unused",
			   .brightness_set = as9817_64_led_diag_set,
			   .brightness_get = as9817_64_led_diag_get,
			   .flags = LED_CORE_SUSPENDRESUME,
			   .max_brightness = LED_MODE_GREEN,
			   },
	[LED_TYPE_LOC] = {
			  .name = "as9817_64_led::loc",
			  .default_trigger = "unused",
			  .brightness_set = as9817_64_led_loc_set,
			  .brightness_get = as9817_64_led_loc_get,
			  .flags = LED_CORE_SUSPENDRESUME,
			  .max_brightness = LED_MODE_BLUE_BLINKING,
			  },
	[LED_TYPE_FAN] = {
			  .name = "as9817_64_led::fan",
			  .default_trigger = "unused",
			  .brightness_set = as9817_64_led_fan_set,
			  .brightness_get = as9817_64_led_fan_get,
			  .flags = LED_CORE_SUSPENDRESUME,
			  .max_brightness = LED_MODE_GREEN,
			  },
	[LED_TYPE_ALARM] = {
			    .name = "as9817_64_led::alarm",
			    .default_trigger = "unused",
			    .brightness_set = as9817_64_led_alarm_set,
			    .brightness_get = as9817_64_led_alarm_get,
			    .flags = LED_CORE_SUSPENDRESUME,
			    .max_brightness = LED_MODE_RED,
			    },
	[LED_TYPE_PSU1] = {
			   .name = "as9817_64_led::psu1",
			   .default_trigger = "unused",
			   .brightness_set = as9817_64_led_psu1_set,
			   .brightness_get = as9817_64_led_psu1_get,
			   .flags = LED_CORE_SUSPENDRESUME,
			   .max_brightness = LED_MODE_GREEN,
			   },
	[LED_TYPE_PSU2] = {
			   .name = "as9817_64_led::psu2",
			   .default_trigger = "unused",
			   .brightness_set = as9817_64_led_psu2_set,
			   .brightness_get = as9817_64_led_psu2_get,
			   .flags = LED_CORE_SUSPENDRESUME,
			   .max_brightness = LED_MODE_GREEN,
			   },
};

static int as9817_64_led_suspend(struct platform_device *dev,
				 pm_message_t state)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(as9817_64_leds); i++) {
		led_classdev_suspend(&as9817_64_leds[i]);
	}

	return 0;
}

static int as9817_64_led_resume(struct platform_device *dev)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(as9817_64_leds); i++) {
		led_classdev_resume(&as9817_64_leds[i]);
	}

	return 0;
}

static int as9817_64_led_probe(struct platform_device *pdev)
{
	int ret, i;

	for (i = 0; i < ARRAY_SIZE(as9817_64_leds); i++) {
		ret = led_classdev_register(&pdev->dev, &as9817_64_leds[i]);

		if (ret < 0) {
			break;
		}
	}

	/* Check if all LEDs were successfully registered */
	if (i != ARRAY_SIZE(as9817_64_leds)) {
		int j;

		/* only unregister the LEDs that were successfully registered */
		for (j = 0; j < i; j++) {
			led_classdev_unregister(&as9817_64_leds[j]);
		}
	}

	return ret;
}

static int as9817_64_led_remove(struct platform_device *pdev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(as9817_64_leds); i++) {
		led_classdev_unregister(&as9817_64_leds[i]);
	}

	return 0;
}

static struct platform_driver as9817_64_led_driver = {
	.probe = as9817_64_led_probe,
	.remove = as9817_64_led_remove,
	.suspend = as9817_64_led_suspend,
	.resume = as9817_64_led_resume,
	.driver = {
		   .name = DRVNAME,
		   .owner = THIS_MODULE,
		   },
};

static int __init as9817_64_led_init(void)
{
	int ret;

	ret = platform_driver_register(&as9817_64_led_driver);
	if (ret < 0) {
		goto exit;
	}

	pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		ret = PTR_ERR(pdev);
		platform_driver_unregister(&as9817_64_led_driver);
		goto exit;
	}

 exit:
	return ret;
}

static void __exit as9817_64_led_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&as9817_64_led_driver);
}

module_init(as9817_64_led_init);
module_exit(as9817_64_led_exit);

MODULE_AUTHOR("Roger Ho <roger530_ho@edge-core.com>");
MODULE_DESCRIPTION("as9817_64_led driver");
MODULE_LICENSE("GPL");
