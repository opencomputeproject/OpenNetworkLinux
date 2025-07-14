// SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0
/*
 * Copyright (C) 2024 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com>
 *
 * Based on:
 *	pca954x.c from Kumar Gala <galak@kernel.crashing.org>
 * Copyright (C) 2006
 *
 * Based on:
 *	pca954x.c from Ken Harrenstien
 * Copyright (C) 2004 Google, Inc. (Ken Harrenstien)
 *
 * Based on:
 *	i2c-virtual_cb.c from Brian Kuschak <bkuschak@yahoo.com>
 * and
 *	pca9540.c from Jean Delvare <khali@linux-fr.org>.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/stat.h>
#include <linux/sysfs.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/hwmon-sysfs.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>

#define DRVNAME "as7515_24x_led"

#define FPGA_BLINKING_REG (0x8F)

static ssize_t set_led(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t show_led(struct device *dev, struct device_attribute *attr,
			char *buf);

extern int as7515_24x_fpga_read(u8 reg);
extern int as7515_24x_fpga_write(u8 reg, u8 value);
static int as7515_24x_led_probe(struct platform_device *pdev);
static int as7515_24x_led_remove(struct platform_device *pdev);

enum led_type {
	LED_TYPE_FAN,
	LED_TYPE_LOC,
	LED_TYPE_PSU1,
	LED_TYPE_PSU2,
	LED_TYPE_DIAG,
	LED_COUNT
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

struct as7515_24x_led_data {
	struct mutex update_lock;
};

struct as7515_24x_led_data *data = NULL;

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

struct led_config {
	enum led_type type;
	enum led_light_mode mode;
	struct color_reg rgb[RGB_COUNT];
};

static struct led_config led_config_data[] = {
	/*  Type			 Mode */
	{LED_TYPE_DIAG, LED_MODE_ORANGE,
		{{0x8B, 0xFF}, {0x8A, 0x3F}, {0x89, 0x00}}}, /* R G B	  */
	{LED_TYPE_DIAG, LED_MODE_GREEN,
		{{0x8B, 0x00}, {0x8A, 0xFF}, {0x89, 0x00}}},
	{LED_TYPE_DIAG, LED_MODE_GREEN_BLINKING,
		{{0x8B, 0x00}, {0x8A, 0xFF}, {0x89, 0x00}}},

	{LED_TYPE_LOC, LED_MODE_BLUE_BLINKING,
		{{0x85, 0x00}, {0x84, 0x00}, {0x83, 0xFF}}},
	{LED_TYPE_LOC, LED_MODE_OFF,
		{{0x85, 0x00}, {0x84, 0x00}, {0x83, 0x00}}},

	{LED_TYPE_FAN, LED_MODE_ORANGE,
		{{0x88, 0xFF}, {0x87, 0x3F}, {0x86, 0x00}}},
	{LED_TYPE_FAN, LED_MODE_GREEN,
		{{0x88, 0x00}, {0x87, 0xFF}, {0x86, 0x00}}},
	{LED_TYPE_FAN, LED_MODE_OFF,
		{{0x88, 0x00}, {0x87, 0x00}, {0x86, 0x00}}},

	{LED_TYPE_PSU1, LED_MODE_ORANGE,
		{{0x8E, 0xFF}, {0x8D, 0x20}, {0x8C, 0x00}}},
	{LED_TYPE_PSU1, LED_MODE_GREEN,
		{{0x8E, 0x00}, {0x8D, 0xFF}, {0x8C, 0x00}}},
	{LED_TYPE_PSU1, LED_MODE_OFF,
		{{0x8E, 0x00}, {0x8D, 0x00}, {0x8C, 0x00}}},

	{LED_TYPE_PSU2, LED_MODE_ORANGE,
		{{0x82, 0xFF}, {0x81, 0x20}, {0x80, 0x00}}},
	{LED_TYPE_PSU2, LED_MODE_GREEN,
		{{0x82, 0x00}, {0x81, 0xFF}, {0x80, 0x00}}},
	{LED_TYPE_PSU2, LED_MODE_OFF,
		{{0x82, 0x00}, {0x81, 0x00}, {0x80, 0x00}}}
};

static SENSOR_DEVICE_ATTR(led_psu1, S_IWUSR | S_IRUGO, show_led, set_led,
							LED_TYPE_PSU1);
static SENSOR_DEVICE_ATTR(led_psu2, S_IWUSR | S_IRUGO, show_led, set_led,
							LED_TYPE_PSU2);
static SENSOR_DEVICE_ATTR(led_loc, S_IWUSR | S_IRUGO, show_led, set_led,
							LED_TYPE_LOC);
static SENSOR_DEVICE_ATTR(led_diag, S_IWUSR | S_IRUGO, show_led, set_led,
							LED_TYPE_DIAG);
static SENSOR_DEVICE_ATTR(led_fan, S_IWUSR | S_IRUGO, show_led, set_led,
							LED_TYPE_FAN);

static struct attribute *as7515_24x_led_attributes[] = {
	&sensor_dev_attr_led_psu1.dev_attr.attr,
	&sensor_dev_attr_led_psu2.dev_attr.attr,
	&sensor_dev_attr_led_loc.dev_attr.attr,
	&sensor_dev_attr_led_fan.dev_attr.attr,
	&sensor_dev_attr_led_diag.dev_attr.attr,
	NULL
};

static const struct attribute_group led_group[] = {
	{ .attrs = as7515_24x_led_attributes }
};

static int led_read_value(u8 reg)
{
	return as7515_24x_fpga_read(reg);
}

static int led_write_value(u8 reg, u8 value)
{
	return as7515_24x_fpga_write(reg, value);
}

static int _led_validate_mode(enum led_type type, enum led_light_mode mode)
{
	int i, status = -EINVAL;

	for (i = 0; i < ARRAY_SIZE(led_config_data); i++) {
		if (type != led_config_data[i].type)
			continue;

		if (mode != led_config_data[i].mode)
			continue;

		/* led mode is found */
		status = 0;
		break;
	}

	return status;
}

static enum led_light_mode _led_get_mode_from_rgb(enum led_type type)
{
	u8 match = 0;
	int i, j, status;
	u8 rgb_val[RGB_COUNT];

	for (i = 0; i < ARRAY_SIZE(led_config_data); i++) {
		if (type != led_config_data[i].type)
			continue;

		for (j = 0; j < ARRAY_SIZE(led_config_data[i].rgb); j++) {
			status = led_read_value(led_config_data[i].rgb[j].reg_addr);
			if (status < 0)
				return LED_MODE_UNKNOWN;

			rgb_val[j] = status;
		}

		match = 1;
		break;
	}

	if (!match)
		return LED_MODE_UNKNOWN;

	for (i = 0; i < ARRAY_SIZE(led_config_data); i++) {
		if (type != led_config_data[i].type)
			continue;

		if ((led_config_data[i].rgb[RGB_RED].reg_val   == rgb_val[RGB_RED]) &&
			(led_config_data[i].rgb[RGB_GREEN].reg_val == rgb_val[RGB_GREEN]) &&
			(led_config_data[i].rgb[RGB_BLUE].reg_val  == rgb_val[RGB_BLUE])) {
			return led_config_data[i].mode;
		}
	}

	return LED_MODE_UNKNOWN;
}

static int led_get_blinking(enum led_type type)
{
	int status;

	status = led_read_value(FPGA_BLINKING_REG);
	if (status < 0)
		return 0;

	return !!(BIT(type) & status);
}

static int led_set_blinking(enum led_type type, int blinking)
{
	int status;

	status = led_read_value(FPGA_BLINKING_REG);
	if (status < 0)
		return status;

	if (blinking)
		status |= BIT(type);
	else
		status &= ~BIT(type);

	return led_write_value(FPGA_BLINKING_REG, status);
}

static int led_is_blinking_mode(enum led_light_mode mode)
{
	if (mode == LED_MODE_OFF || mode == LED_MODE_UNKNOWN)
		return 0;

	return (mode % 2);
}

static enum led_light_mode led_get_mode(enum led_type type)
{
	int status, blinking = 0;
	enum led_light_mode mode = LED_MODE_UNKNOWN;

	mode = _led_get_mode_from_rgb(type);
	if (mode == LED_MODE_OFF || mode == LED_MODE_UNKNOWN)
		return mode;

	/* transit to blinking mode if blinking is enabled */
	blinking = led_get_blinking(type);
	if (blinking) {
		if (!led_is_blinking_mode(mode))
			mode += 1; /* normal mode to blinking mode */
	}
	else {
		if (led_is_blinking_mode(mode))
			mode -= 1; /* blinking mode to normal mode */
	}

	/* Validate led mode */
	status = _led_validate_mode(type, mode);
	if (status)
		return LED_MODE_UNKNOWN;

	return mode;
}

static int _led_set_mode_rgb(enum led_type type, enum led_light_mode mode)
{
	int i, j, status;

	for (i = 0; i < ARRAY_SIZE(led_config_data); i++) {
		if (type != led_config_data[i].type)
			continue;

		if (mode != led_config_data[i].mode)
			continue;

		for (j = 0; j < ARRAY_SIZE(led_config_data[i].rgb); j++) {
			status = led_write_value(led_config_data[i].rgb[j].reg_addr,
									led_config_data[i].rgb[j].reg_val);
			if (status < 0)
				return status;
		}
	}

	return 0;
}

static int led_set_mode(enum led_type type, enum led_light_mode mode)
{
	int status = 0;

	/* Validate led mode */
	status = _led_validate_mode(type, mode);
	if (status)
		return status;

	status = _led_set_mode_rgb(type, mode);
	if (status < 0)
		return status;

	return led_set_blinking(type, led_is_blinking_mode(mode));
}

static ssize_t show_led(struct device *dev, struct device_attribute *da,
							char *buf)
{
	struct as7515_24x_led_data *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	enum led_light_mode mode = LED_MODE_UNKNOWN;

	mutex_lock(&data->update_lock);
	mode = led_get_mode(attr->index);
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n", mode);
}

static ssize_t set_led(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	long mode;
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as7515_24x_led_data *data = dev_get_drvdata(dev);

	status = kstrtol(buf, 10, &mode);
	if (status)
		return status;

	mutex_lock(&data->update_lock);

	status = led_set_mode(attr->index, mode);
	if (status < 0)
		goto exit;

	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static int as7515_24x_led_probe(struct platform_device *pdev)
{
	int status = -1;
	int i = 0;
	struct as7515_24x_led_data *data;

	data = kzalloc(sizeof(struct as7515_24x_led_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	mutex_init(&data->update_lock);
	platform_set_drvdata(pdev, data);

	/* Register sysfs hooks */
	for (i = 0; i < ARRAY_SIZE(led_group); i++) {
		/* Register sysfs hooks */
		status = sysfs_create_group(&pdev->dev.kobj, &led_group[i]);
		if (status)
			goto exit_sysfs_group;
	}

	dev_info(&pdev->dev, "device created\n");
	return 0;

exit_sysfs_group:
	for (--i; i >= 0; i--) {
		sysfs_remove_group(&pdev->dev.kobj, &led_group[i]);
	}

	kfree(data);
	return status;
}

static int as7515_24x_led_remove(struct platform_device *pdev)
{
	struct as7515_24x_led_data *data = platform_get_drvdata(pdev);
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(led_group); i++) {
		sysfs_remove_group(&pdev->dev.kobj, &led_group[i]);
	}

	kfree(data);
	return 0;
}

static struct platform_device_id as7515_24x_led_id[] = {
	{ DRVNAME, 0 },
	{}
};
MODULE_DEVICE_TABLE(platform, as7515_24x_led_id);

static struct platform_driver as7515_24x_led_driver = {
	.probe = as7515_24x_led_probe,
	.remove = as7515_24x_led_remove,
	.id_table	= as7515_24x_led_id,
	.driver = {
		.name = DRVNAME,
		.owner = THIS_MODULE,
	},
};

module_platform_driver(as7515_24x_led_driver);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as7515_24x_led driver");
MODULE_LICENSE("GPL");
