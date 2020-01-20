/*
 * baseboard.c - Celestica Baseboard CPLD I2C driver.
 * Copyright (C) 2019 Celestica Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


// TODO: User regmap for more descriptive register access. See MFD
// TODO: Add support of legacy i2c bus and smbus_emulated bus.


#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/i2c.h>
#include <linux/acpi.h>
#include <linux/hwmon.h>

/**
 * CPLD register address for read and write.
 */
#define VERSION_ADDR	0x00
#define SCRATCH_ADDR	0x01
#define REBOOT_CAUSE	0x06
#define SYS_LED_ADDR	0x62
#define ALRM_LED_ADDR	0x63

/* Private data for baseboard CPLD */
struct baseboard_data {
	// led_groups
	struct regmap *regmap;
	struct i2c_client *client;
	struct mutex lock;
};

// implement regmap
// Need to expose the reg read/write function to other modules that 
// shared same i2c read write function.
// Create 2 more sub-system devices: hwmon(reg-access), leds


/* System reboot cause recorded in CPLD */
static const struct {
	const char *reason;
	u8 reset_code;
} reboot_causes[] = {
	{"POR",           0x11},
	{"soft-warm-rst", 0x22},
	{"soft-cold-rst", 0x33},
	{"warm-rst",      0x44},
	{"cold-rst",      0x55},
	{"wdt-rst",       0x66},
	{"power-cycle",   0x77}
};

/* Show string version in Major.minor */
static ssize_t version_show(struct device *dev, 
			    struct device_attribute *attr, 
			    char *buf)
{
	struct baseboard_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int value;

	value = i2c_smbus_read_byte_data(client, VERSION_ADDR);
	if(value < 0)
		return value;

	return sprintf(buf, "%d.%d\n", value >> 4, value & 0x0F);
}

static ssize_t scratch_show(struct device *dev, 
			    struct device_attribute *attr, 
			    char *buf)
{
	struct baseboard_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int value;

	value = i2c_smbus_read_byte_data(client, SCRATCH_ADDR);
	if(value < 0)
		return value;

	return sprintf(buf, "0x%.2x\n", value);
}

static ssize_t scratch_store(struct device *dev, 
			     struct device_attribute *attr, 
			     const char *buf, size_t count)
{
	u8 value;
	ssize_t status;
	struct baseboard_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;

	status = kstrtou8(buf, 0, &value);
	if(status != 0)
		return status;
	status = i2c_smbus_write_byte_data(client, SCRATCH_ADDR, value);
	if(status == 0)
		status = count;
	return status;
}

static ssize_t reboot_cause_show(struct device *dev, 
				 struct device_attribute *attr, 
				 char *buf)
{
	struct baseboard_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	ssize_t status;
	int reg, i;

	reg = i2c_smbus_read_byte_data(client, REBOOT_CAUSE);
	if(reg < 0)
		return reg;

	status = 0;
	dev_dbg(dev,"reboot: 0x%x\n", (u8)reg);
	for(i = 0; i < ARRAY_SIZE(reboot_causes); i++){
		if((u8)reg == reboot_causes[i].reset_code){
			status = sprintf(buf, "%s\n", 
					 reboot_causes[i].reason);
			break;
		}
	}
	return status;
}

DEVICE_ATTR_RO(version);
DEVICE_ATTR_RW(scratch);
DEVICE_ATTR_RO(reboot_cause);

static struct attribute *baseboard_info_attrs[] = {
	&dev_attr_version.attr,
	&dev_attr_scratch.attr,
	&dev_attr_reboot_cause.attr,
	NULL,
};
ATTRIBUTE_GROUPS(baseboard_info);

static int baseboard_probe(struct i2c_client *client, 
			   const struct i2c_device_id *id)
{
	int err;
	struct device *dev;
	struct baseboard_data *data;
	struct device *hwmon_dev;

	dev = &client->dev;

	if (!i2c_check_functionality(client->adapter,
		I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	data = devm_kzalloc(dev, sizeof(struct baseboard_data), 
		GFP_KERNEL);

	if (!data){
		err = -ENOMEM;
		goto fail_alloc_baseboard_data;
	} 
	data->client = client;
	hwmon_dev = devm_hwmon_device_register_with_groups(dev, client->name,
							   data, 
							   baseboard_info_groups);
	if (IS_ERR(hwmon_dev)){
		err = PTR_ERR(hwmon_dev);
		goto fail_alloc_baseboard_data;
	}

	return 0;

fail_alloc_baseboard_data:
	return err;
}

static const struct i2c_device_id baseboard_ids[] = {
	{ "baseboard", 0x0d },
	{ /* END OF List */ }
};
MODULE_DEVICE_TABLE(i2c, baseboard_ids);

struct i2c_driver baseboard_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = "baseboard",
		.owner = THIS_MODULE,
	},
	.probe = baseboard_probe,
	.id_table = baseboard_ids,
};

module_i2c_driver(baseboard_driver);

MODULE_AUTHOR("Celestica Inc.");
MODULE_DESCRIPTION("Celestica CPLD baseboard driver");
MODULE_VERSION("1.0.0");
MODULE_LICENSE("GPL");