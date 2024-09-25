/*
 * A LED driver for the accton_as4610_led
 *
 * Copyright (C) 2016 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com.tw>
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

/*#define DEBUG*/

#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include <linux/bitops.h>

extern int as4610_54_cpld_read (unsigned short cpld_addr, u8 reg);
extern int as4610_54_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);
extern int as4610_product_id(void);

#define DRVNAME "as4610_led"

struct as4610_led_data {
	struct platform_device *pdev;
	struct mutex	 update_lock;
	char			 valid;			  /* != 0 if registers are valid */
	unsigned long	 last_updated;	  /* In jiffies */
	int				 led_map;
	u8				 reg_val[5];	 /* Register value, 0 = (0x1A) Blinking function
														1 = (0x30) 7-seg 2
														2 = (0x31) 7-seg 1
														3 = (0x32) SYS/PRI/PSU1-2 LED
														4 = (0x33) STK1-2/Fan/PoE/Alarm LED */
};

static struct as4610_led_data  *ledctl = NULL;

/* LED related data
 */
#define LED_7SEG_REG_MASK		 0x0F
#define LED_7SEG_POINT_REG_MASK	 0x10

#define LED_NORMAL_MASK			 0x03
#define LED_NORMAL_GREEN_VALUE	 0x02
#define LED_NORMAL_AMBER_VALUE	 0x01
#define LED_NORMAL_OFF_VALUE	 0x00

#define LED_TYPE_SYS_REG_MASK	 0xC0
#define LED_MODE_SYS_BLINK_MASK  0x80

#define LED_TYPE_PRI_REG_MASK    0x30
#define LED_MODE_PRI_BLINK_MASK  0x40

#define LED_TYPE_PSU1_REG_MASK	  0x0C
#define LED_MODE_PSU1_BLINK_MASK 0x20

#define LED_TYPE_PSU2_REG_MASK   0x03
#define LED_MODE_PSU2_BLINK_MASK 0x10

#define LED_TYPE_STK1_REG_MASK   0xC0
#define LED_MODE_STK1_BLINK_MASK 0x08

#define LED_TYPE_STK2_REG_MASK   0x30
#define LED_MODE_STK2_BLINK_MASK 0x04

#define LED_TYPE_FAN_REG_MASK    0x0C
#define LED_MODE_FAN_BLINK_MASK  0x02

#define LED_TYPE_POE_ALARM_REG_MASK   0x03
#define LED_MODE_POE_ALARM_BLINK_MASK 0x01

static const u8 led_reg[] = {
	0x1A, /* Blinking function */
	0x30, /* 7-seg 1 */
	0x31, /* 7-seg 2 */
	0x32, /* SYS/PRI/PSU1-2 LED */
	0x33, /* STK1-2/Fan/PoE/Alarm LED */
};

enum as4610_product_id_e {
	PID_AS4610_30T,
	PID_AS4610_30P,
	PID_AS4610_54T,
	PID_AS4610_54P,
	PID_RESERVED,
	PID_AS4610_54T_B,
	PID_UNKNOWN
};

enum led_type {
	LED_TYPE_SYS,
	LED_TYPE_PRI,
	LED_TYPE_PSU1,
	LED_TYPE_PSU2,
	LED_TYPE_STK1,
	LED_TYPE_STK2,
	LED_TYPE_7SEG_TENS,
	LED_TYPE_7SEG_TENS_POINT,
	LED_TYPE_7SEG_DIGITS,
	LED_TYPE_7SEG_DIGITS_POINT,
	LED_TYPE_FAN,
	LED_TYPE_POE,
	LED_TYPE_ALARM,
	NUM_OF_LED
};

#define AS4610_COMMON_LED_MAP	(BIT(LED_TYPE_SYS) | BIT(LED_TYPE_PRI) | BIT(LED_TYPE_PSU1) | \
							 	 BIT(LED_TYPE_PSU2)| BIT(LED_TYPE_STK1)| BIT(LED_TYPE_STK2))
#define AS4610_NPOE_LED_MAP		(AS4610_COMMON_LED_MAP | BIT(LED_TYPE_7SEG_TENS) | \
							 	 BIT(LED_TYPE_7SEG_TENS_POINT) | BIT(LED_TYPE_7SEG_DIGITS) | \
							 	 BIT(LED_TYPE_7SEG_DIGITS_POINT))
#define AS4610_POE_LED_MAP 		(AS4610_NPOE_LED_MAP | BIT(LED_TYPE_FAN) | BIT(LED_TYPE_POE))
#define AS4610_54T_B_LED_MAP 	(AS4610_COMMON_LED_MAP | BIT(LED_TYPE_FAN) | BIT(LED_TYPE_ALARM))

static int as4610_ledmaps[] = {
	[PID_AS4610_30T] 	= AS4610_NPOE_LED_MAP,
	[PID_AS4610_30P] 	= AS4610_POE_LED_MAP,
	[PID_AS4610_54T] 	= AS4610_NPOE_LED_MAP,
	[PID_AS4610_54P] 	= AS4610_POE_LED_MAP,
	[PID_AS4610_54T_B] 	= AS4610_54T_B_LED_MAP,
	[PID_RESERVED]	 	= 0,
};

enum led_light_mode {
	LED_MODE_OFF = 0,
	LED_MODE_GREEN,
	LED_MODE_GREEN_BLINK,
	LED_MODE_AMBER,
	LED_MODE_AMBER_BLINK,
	LED_MODE_RED,
	LED_MODE_RED_BLINK,
	LED_MODE_BLUE,
	LED_MODE_BLUE_BLINK,
	LED_MODE_AUTO,
	LED_MODE_AUTO_BLINKING,
	LED_MODE_UNKNOWN,
	LED_MODE_SEVEN_SEGMENT_MAX = 9,
};

static int as4610_led_read_value(u8 reg)
{
	return as4610_54_cpld_read(0x30, reg);
}

static int as4610_led_write_value(u8 reg, u8 value)
{
	return as4610_54_cpld_write(0x30, reg, value);
}

static void as4610_led_update(void)
{
	mutex_lock(&ledctl->update_lock);

	if (time_after(jiffies, ledctl->last_updated + HZ + HZ / 2)
		|| !ledctl->valid) {
		int i;

		dev_dbg(&ledctl->pdev->dev, "Starting as4610_led update\n");

		/* Update LED data
		 */
		for (i = 0; i < ARRAY_SIZE(ledctl->reg_val); i++) {
			int status = as4610_led_read_value(led_reg[i]);

			if (status < 0) {
				ledctl->valid = 0;
				dev_dbg(&ledctl->pdev->dev, "reg %d, err %d\n", led_reg[i], status);
				goto exit;
			}
			else
			{
				ledctl->reg_val[i] = status;
			}
		}

		ledctl->last_updated = jiffies;
		ledctl->valid = 1;
	}

exit:
	mutex_unlock(&ledctl->update_lock);
}

static enum led_brightness seven_segment_get(struct led_classdev *cdev, u8 reg_id)
{
	as4610_led_update();
	return (ledctl->reg_val[reg_id] & LED_7SEG_REG_MASK);
}

static void seven_segment_set(struct led_classdev *cdev, enum led_brightness mode, u8 reg_id)
{
	if (mode > cdev->max_brightness) {
		return;
	}

	ledctl->reg_val[reg_id] &= 0xF0;
	ledctl->reg_val[reg_id] |= mode;
	as4610_led_write_value(led_reg[reg_id], ledctl->reg_val[reg_id]);
}

static enum led_brightness seven_segment_digits_get(struct led_classdev *cdev)
{
	return seven_segment_get(cdev, 1);
}

static void seven_segment_digits_set(struct led_classdev *cdev, enum led_brightness mode)
{
	seven_segment_set(cdev, mode, 1);
}

static enum led_brightness seven_segment_tens_get(struct led_classdev *cdev)
{
	return seven_segment_get(cdev, 2);
}

static void seven_segment_tens_set(struct led_classdev *cdev, enum led_brightness mode)
{
	seven_segment_set(cdev, mode, 2);
}

static enum led_brightness seven_segment_point_get(struct led_classdev *cdev, u8 reg_id)
{
	as4610_led_update();
	return (ledctl->reg_val[reg_id] & LED_7SEG_POINT_REG_MASK) ? LED_MODE_GREEN : LED_MODE_OFF;
}

static void seven_segment_point_set(struct led_classdev *cdev,
										  enum led_brightness mode, u8 reg_id)
{
	/* Validate brightness */
	if ((int)mode < LED_MODE_OFF || mode > cdev->max_brightness) {
		return;
	}

	if ((int)mode == (int)LED_MODE_OFF) {
		ledctl->reg_val[reg_id] &= ~LED_7SEG_POINT_REG_MASK;
	}
	else { /* LED_MODE_GREEN */
		ledctl->reg_val[reg_id] |= LED_7SEG_POINT_REG_MASK;
	}

	as4610_led_write_value(led_reg[reg_id], ledctl->reg_val[reg_id]);
}

static enum led_brightness seven_segment_tens_point_get(struct led_classdev *cdev)
{
	return seven_segment_point_get(cdev, 2);
}

static void seven_segment_tens_point_set(struct led_classdev *cdev,
										  enum led_brightness mode)
{
	seven_segment_point_set(cdev, mode, 2);
}

static enum led_brightness seven_segment_digits_point_get(struct led_classdev *cdev)
{
	return seven_segment_point_get(cdev, 1);
}

static void seven_segment_digits_point_set(struct led_classdev *cdev,
										  enum led_brightness mode)
{
	seven_segment_point_set(cdev, mode, 1);
}

static u8 led_is_blinking_mode(enum led_brightness mode)
{
	return ((int)mode == (int)LED_MODE_GREEN_BLINK ||
			(int)mode == (int)LED_MODE_AMBER_BLINK ||
			(int)mode == (int)LED_MODE_AUTO_BLINKING);
}

static enum led_brightness as4610_led_auto_get(u8 blink_mask)
{
	as4610_led_update();
	return (ledctl->reg_val[0] & blink_mask) ? LED_MODE_AUTO_BLINKING : LED_MODE_AUTO;
}

static void as4610_led_auto_set(struct led_classdev *cdev,
							 enum led_brightness mode, u8 blink_mask)
{
	/* Validate brightness */
	if ((int)mode < (int)LED_MODE_AUTO || mode > cdev->max_brightness) {
		return;
	}

	/* Set blinking */
	if (led_is_blinking_mode(mode)) {
		ledctl->reg_val[0] |= blink_mask;
	}
	else {
		ledctl->reg_val[0] &= ~blink_mask;
	}
	as4610_led_write_value(led_reg[0], ledctl->reg_val[0]);
}

static enum led_brightness as4610_led_psu1_get(struct led_classdev *cdev)
{
   return as4610_led_auto_get(LED_MODE_PSU1_BLINK_MASK);
}

static void as4610_led_psu1_set(struct led_classdev *cdev,
										  enum led_brightness mode)
{
	as4610_led_auto_set(cdev, mode, LED_MODE_PSU1_BLINK_MASK);
}

static enum led_brightness as4610_led_psu2_get(struct led_classdev *cdev)
{
	return as4610_led_auto_get(LED_MODE_PSU2_BLINK_MASK);
}

static void as4610_led_psu2_set(struct led_classdev *led_cdev,
										  enum led_brightness mode)
{
	as4610_led_auto_set(led_cdev, mode, LED_MODE_PSU2_BLINK_MASK);
}

static enum led_brightness as4610_led_fan_get(struct led_classdev *cdev)
{
	return as4610_led_auto_get(LED_MODE_FAN_BLINK_MASK);
}

static void as4610_led_fan_set(struct led_classdev *cdev,
										  enum led_brightness mode)
{
	as4610_led_auto_set(cdev, mode, LED_MODE_FAN_BLINK_MASK);
}

static u8 led_normal_light_mode_to_reg_val(enum led_brightness mode)
{
	if (led_is_blinking_mode(mode)) {
		mode -= 1; /* convert blinking mode to non-blinking mode */
	}

	if ((int)mode == (int)LED_MODE_GREEN) {
		return LED_NORMAL_GREEN_VALUE;
	}
	else if ((int)mode == (int)LED_MODE_AMBER) {
		return LED_NORMAL_AMBER_VALUE;
	}

	return LED_NORMAL_OFF_VALUE;
}

static enum led_brightness led_normal_reg_val_to_light_mode(u8 reg_val)
{
	reg_val &= LED_NORMAL_MASK;

	if (reg_val & LED_NORMAL_GREEN_VALUE) {
		return LED_MODE_GREEN;
	}
	else if (reg_val & LED_NORMAL_AMBER_VALUE) {
		return LED_MODE_AMBER;
	}

	return LED_MODE_OFF;
}

static void as4610_led_normal_set(struct led_classdev *cdev,
	enum led_brightness mode, u8 blink_mask, u8 reg_id, u8 reg_mask, u8 shift)
{
	/* Validate brightness */
	if (mode > cdev->max_brightness) {
		return;
	}

	/* Set blinking */
	if (led_is_blinking_mode(mode)) {
		ledctl->reg_val[0] |= blink_mask;
	}
	else {
		ledctl->reg_val[0] &= ~blink_mask;
	}
	as4610_led_write_value(led_reg[0], ledctl->reg_val[0]);

	/* Set color */
	ledctl->reg_val[reg_id] &= ~reg_mask;
	ledctl->reg_val[reg_id] |= (led_normal_light_mode_to_reg_val(mode) << shift);
	as4610_led_write_value(led_reg[reg_id], ledctl->reg_val[reg_id]);
}

static enum led_brightness as4610_led_normal_get(u8 reg_id, u8 blink_mask, u8 shift)
{
	u8 blinking = 0;
	enum led_brightness mode;

	as4610_led_update();

	mode = led_normal_reg_val_to_light_mode(ledctl->reg_val[reg_id] >> shift);
	if ((int)mode == (int)LED_MODE_OFF) {
		return mode;
	}

	/* Checking blinking */
	if (ledctl->reg_val[0] & blink_mask) {
		blinking = 1;
	}

	return blinking ? (mode+1) : mode;
}

static void as4610_led_sys_set(struct led_classdev *cdev,
										  enum led_brightness mode)
{
	as4610_led_normal_set(cdev, mode, LED_MODE_SYS_BLINK_MASK,
							  3, LED_TYPE_SYS_REG_MASK, 6);
}

static enum led_brightness as4610_led_sys_get(struct led_classdev *cdev)
{
	return as4610_led_normal_get(3, LED_MODE_SYS_BLINK_MASK, 6);
}

static void as4610_led_pri_set(struct led_classdev *cdev,
										  enum led_brightness mode)
{
	as4610_led_normal_set(cdev, mode, LED_MODE_PRI_BLINK_MASK,
							  3, LED_TYPE_PRI_REG_MASK, 4);
}

static enum led_brightness as4610_led_pri_get(struct led_classdev *cdev)
{
	return as4610_led_normal_get(3, LED_MODE_PRI_BLINK_MASK, 4);
}

static void as4610_led_poe_alarm_set(struct led_classdev *cdev,
										  enum led_brightness mode)
{
	as4610_led_normal_set(cdev, mode, LED_MODE_POE_ALARM_BLINK_MASK,
							  4, LED_TYPE_POE_ALARM_REG_MASK, 0);
}

static enum led_brightness as4610_led_poe_alarm_get(struct led_classdev *cdev)
{
	return as4610_led_normal_get(4, LED_MODE_POE_ALARM_BLINK_MASK, 0);
}

static void as4610_led_stk1_set(struct led_classdev *cdev,
										  enum led_brightness mode)
{
	as4610_led_normal_set(cdev, mode, LED_MODE_STK1_BLINK_MASK,
							  4, LED_TYPE_STK1_REG_MASK, 6);
}

static enum led_brightness as4610_led_stk1_get(struct led_classdev *cdev)
{
	return as4610_led_normal_get(4, LED_MODE_STK1_BLINK_MASK, 6);
}

static void as4610_led_stk2_set(struct led_classdev *cdev,
										  enum led_brightness mode)
{
	as4610_led_normal_set(cdev, mode, LED_MODE_STK2_BLINK_MASK,
							  4, LED_TYPE_STK2_REG_MASK, 4);
}

static enum led_brightness as4610_led_stk2_get(struct led_classdev *cdev)
{
	return as4610_led_normal_get(4, LED_MODE_STK2_BLINK_MASK, 4);
}

static struct led_classdev as4610_leds[] = {
	[LED_TYPE_SYS] = {
		.name			 = "as4610::sys",
		.default_trigger = "unused",
		.brightness_set	 = as4610_led_sys_set,
		.brightness_get	 = as4610_led_sys_get,
		.max_brightness	 = LED_MODE_AMBER_BLINK,
	},
	[LED_TYPE_PRI] = {
		.name			 = "as4610::pri",
		.default_trigger = "unused",
		.brightness_set	 = as4610_led_pri_set,
		.brightness_get	 = as4610_led_pri_get,
		.max_brightness	 = LED_MODE_AMBER_BLINK,
	},
	[LED_TYPE_PSU1] = {
		.name			 = "as4610::psu1",
		.default_trigger = "unused",
		.brightness_set	 = as4610_led_psu1_set,
		.brightness_get	 = as4610_led_psu1_get,
		.max_brightness	 = LED_MODE_AUTO_BLINKING,
	},
	[LED_TYPE_PSU2] = {
		.name			 = "as4610::psu2",
		.default_trigger = "unused",
		.brightness_set	 = as4610_led_psu2_set,
		.brightness_get	 = as4610_led_psu2_get,
		.max_brightness	 = LED_MODE_AUTO_BLINKING,
	},
	[LED_TYPE_STK1] = {
		.name			 = "as4610::stk1",
		.default_trigger = "unused",
		.brightness_set	 = as4610_led_stk1_set,
		.brightness_get	 = as4610_led_stk1_get,
		.max_brightness	 = LED_MODE_AMBER_BLINK,
	},
	[LED_TYPE_STK2] = {
		.name			 = "as4610::stk2",
		.default_trigger = "unused",
		.brightness_set	 = as4610_led_stk2_set,
		.brightness_get	 = as4610_led_stk2_get,
		.max_brightness	 = LED_MODE_AMBER_BLINK,
	},
	[LED_TYPE_7SEG_TENS] = {
		.name			 = "as4610::7seg_tens",
		.default_trigger = "unused",
		.brightness_set	 = seven_segment_tens_set,
		.brightness_get	 = seven_segment_tens_get,
		.max_brightness	 = LED_MODE_SEVEN_SEGMENT_MAX,
	},
	[LED_TYPE_7SEG_TENS_POINT] = {
		.name			 = "as4610::7seg_tens_point",
		.default_trigger = "unused",
		.brightness_set	 = seven_segment_tens_point_set,
		.brightness_get	 = seven_segment_tens_point_get,
		.max_brightness	 = LED_MODE_GREEN,
	},
	[LED_TYPE_7SEG_DIGITS] = {
		.name			 = "as4610::7seg_digits",
		.default_trigger = "unused",
		.brightness_set	 = seven_segment_digits_set,
		.brightness_get	 = seven_segment_digits_get,
		.max_brightness	 = LED_MODE_SEVEN_SEGMENT_MAX,
	},
	[LED_TYPE_7SEG_DIGITS_POINT] = {
		.name			 = "as4610::7seg_digits_point",
		.default_trigger = "unused",
		.brightness_set	 = seven_segment_digits_point_set,
		.brightness_get	 = seven_segment_digits_point_get,
		.max_brightness	 = LED_MODE_GREEN,
	},
	[LED_TYPE_FAN] = {
		.name			 = "as4610::fan",
		.default_trigger = "unused",
		.brightness_set	 = as4610_led_fan_set,
		.brightness_get	 = as4610_led_fan_get,
		.max_brightness	 = LED_MODE_AUTO_BLINKING,
	},
	[LED_TYPE_POE] = {
		.name			 = "as4610::poe",
		.default_trigger = "unused",
		.brightness_set	 = as4610_led_poe_alarm_set,
		.brightness_get	 = as4610_led_poe_alarm_get,
		.max_brightness	 = LED_MODE_AMBER_BLINK,
	},
	[LED_TYPE_ALARM] = {
		.name			 = "as4610::alarm",
		.default_trigger = "unused",
		.brightness_set	 = as4610_led_poe_alarm_set,
		.brightness_get	 = as4610_led_poe_alarm_get,
		.max_brightness	 = LED_MODE_AMBER_BLINK,
	},
};

static int as4610_led_probe(struct platform_device *pdev)
{
	int ret = 0, i;

	int pid = as4610_product_id();
	if (pid == PID_UNKNOWN) {
		return -ENODEV;
	}

	ledctl = kzalloc(sizeof(struct as4610_led_data), GFP_KERNEL);
	if (!ledctl) {
		return -ENOMEM;
	}

	ledctl->led_map = as4610_ledmaps[pid];
	mutex_init(&ledctl->update_lock);

	for (i = 0; i < NUM_OF_LED; i++) {
		if (!(ledctl->led_map & BIT(i))) {
			continue;
		}

		ret = led_classdev_register(&pdev->dev, &as4610_leds[i]);
		if (ret < 0) {
			goto error;
		}
	}

	return 0;

error:
	for (i = i-1; i >= 0; i--) {
		/* only unregister the LEDs that were successfully registered */
		if (!(ledctl->led_map & BIT(i))) {
			continue;
		}

		led_classdev_unregister(&as4610_leds[i]);
	}
	kfree(ledctl);

	return ret;
}

static int as4610_led_remove(struct platform_device *pdev)
{
	int i;

	for (i = 0; i < NUM_OF_LED; i++) {
		if (!(ledctl->led_map & BIT(i))) {
			continue;
		}

		led_classdev_unregister(&as4610_leds[i]);
	}
	kfree(ledctl);

	return 0;
}

static const struct platform_device_id as4610_led_id[] = {
	{ "as4610_led", 0 },
	{}
};
MODULE_DEVICE_TABLE(platform, as4610_led_id);

static struct platform_driver as4610_led_driver = {
	.probe		= as4610_led_probe,
	.remove		= as4610_led_remove,
	.id_table	= as4610_led_id,
	.driver		= {
	.name	= DRVNAME,
	.owner	= THIS_MODULE,
	},
};

module_platform_driver(as4610_led_driver);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as4610_led driver");
MODULE_LICENSE("GPL");

