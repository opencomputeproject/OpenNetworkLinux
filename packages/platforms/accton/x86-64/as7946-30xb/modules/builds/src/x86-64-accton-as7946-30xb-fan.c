/*
 * Copyright (C)  Jake Lin <jake_lin@edge-core.com>
 *
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
#include <linux/hwmon-sysfs.h>
#include <linux/ipmi.h>
#include <linux/ipmi_smi.h>
#include <linux/platform_device.h>
#include "accton_ipmi_intf.h"

#define DRVNAME "as7946_30xb_fan"
#define IPMI_FAN_READ_CMD   0x14
#define IPMI_FAN_WRITE_CMD  0x15
#define IPMI_FAN_REG_READ_CMD  0x20
#define IPMI_FAN_REG_WRITE_CMD 0x21

static ssize_t set_fan(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t show_fan(struct device *dev, struct device_attribute *attr,
			char *buf);
static ssize_t show_version(struct device *dev, struct device_attribute *da,
			char *buf);
static ssize_t show_dir(struct device *dev, struct device_attribute *da,
			char *buf);
static int as7946_30xb_fan_probe(struct platform_device *pdev);
static int as7946_30xb_fan_remove(struct platform_device *pdev);

enum fan_id {
	FAN_1,
	FAN_2,
	FAN_3,
	FAN_4,
	FAN_5,
	FAN_6,
	FAN_7,
	FAN_8,
	FAN_9,
	FAN_10,
	NUM_OF_FAN,
	NUM_OF_FAN_MODULE = NUM_OF_FAN/2
};

enum fan_data_index {
	FAN_PRESENT,
	FAN_PWM,
	FAN_SPEED0,
	FAN_SPEED1,
	FAN_DATA_COUNT
};

struct as7946_30xb_fan_data {
	struct platform_device *pdev;
	struct mutex update_lock;
	char valid; /* != 0 if registers are valid */
	unsigned long last_updated;	/* In jiffies */
	/* 4 bytes for each fan, the last 2 bytes is fan dir */
	unsigned char ipmi_resp[NUM_OF_FAN * FAN_DATA_COUNT + 2];
	unsigned char ipmi_resp_cpld[2];
	struct ipmi_data ipmi;
	unsigned char ipmi_tx_data[3];  /* 0: FAN id, 1: 0x02, 2: PWM */
};

struct as7946_30xb_fan_data *data = NULL;

static struct platform_driver as7946_30xb_fan_driver = {
	.probe = as7946_30xb_fan_probe,
	.remove = as7946_30xb_fan_remove,
	.driver = {
		.name = DRVNAME,
		.owner = THIS_MODULE,
	},
};

#define FAN_PRESENT_ATTR_ID(index) FAN##index##_PRESENT
#define FAN_PWM_ATTR_ID(index) FAN##index##_PWM
#define FAN_RPM_ATTR_ID(index) FAN##index##_INPUT
#define FAN_DIR_ATTR_ID(index) FAN##index##_DIR

#define FAN_ATTR(fan_id) \
	FAN_PRESENT_ATTR_ID(fan_id), \
	FAN_PWM_ATTR_ID(fan_id), \
	FAN_RPM_ATTR_ID(fan_id), \
	FAN_DIR_ATTR_ID(fan_id)

enum as7946_30xb_fan_sysfs_attrs {
	FAN_ATTR(1),
	FAN_ATTR(2),
	FAN_ATTR(3),
	FAN_ATTR(4),
	FAN_ATTR(5),
	FAN_ATTR(6),
	FAN_ATTR(7),
	FAN_ATTR(8),
	FAN_ATTR(9),
	FAN_ATTR(10),
	NUM_OF_FAN_ATTR,
	FAN_VERSION,
	NUM_OF_PER_FAN_ATTR = (NUM_OF_FAN_ATTR/NUM_OF_FAN),
};

/* fan attributes */
#define DECLARE_FAN_VER_SENSOR_DEVICE_ATTR() \
	static SENSOR_DEVICE_ATTR(version, S_IRUGO, show_version, NULL, FAN_VERSION)
#define DECLARE_FAN_VER_ATTR() \
	&sensor_dev_attr_version.dev_attr.attr

#define DECLARE_FAN_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_present, S_IRUGO, show_fan, NULL, \
								FAN##index##_PRESENT); \
	static SENSOR_DEVICE_ATTR(fan##index##_pwm, S_IWUSR | S_IRUGO, show_fan, \
								set_fan, FAN##index##_PWM); \
	static SENSOR_DEVICE_ATTR(fan##index##_input, S_IRUGO, show_fan, NULL, \
								FAN##index##_INPUT); \
	static SENSOR_DEVICE_ATTR(fan##index##_dir, S_IRUGO, show_dir, NULL, \
								FAN##index##_DIR)
#define DECLARE_FAN_ATTR(index) \
	&sensor_dev_attr_fan##index##_present.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_pwm.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_input.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_dir.dev_attr.attr

DECLARE_FAN_SENSOR_DEVICE_ATTR(1);
DECLARE_FAN_SENSOR_DEVICE_ATTR(2);
DECLARE_FAN_SENSOR_DEVICE_ATTR(3);
DECLARE_FAN_SENSOR_DEVICE_ATTR(4);
DECLARE_FAN_SENSOR_DEVICE_ATTR(5);
DECLARE_FAN_SENSOR_DEVICE_ATTR(6);
DECLARE_FAN_SENSOR_DEVICE_ATTR(7);
DECLARE_FAN_SENSOR_DEVICE_ATTR(8);
DECLARE_FAN_SENSOR_DEVICE_ATTR(9);
DECLARE_FAN_SENSOR_DEVICE_ATTR(10);
DECLARE_FAN_VER_SENSOR_DEVICE_ATTR();

static struct attribute *as7946_30xb_fan_attributes[] = {
	/* fan attributes */
	DECLARE_FAN_ATTR(1),
	DECLARE_FAN_ATTR(2),
	DECLARE_FAN_ATTR(3),
	DECLARE_FAN_ATTR(4),
	DECLARE_FAN_ATTR(5),
	DECLARE_FAN_ATTR(6),
	DECLARE_FAN_ATTR(7),
	DECLARE_FAN_ATTR(8),
	DECLARE_FAN_ATTR(9),
	DECLARE_FAN_ATTR(10),
	DECLARE_FAN_VER_ATTR(),
	NULL
};

static const struct attribute_group as7946_30xb_fan_group = {
	.attrs = as7946_30xb_fan_attributes,
};

static struct as7946_30xb_fan_data *as7946_30xb_fan_update_device(void)
{
	int status = 0;

	if (time_before(jiffies, data->last_updated + HZ * 5) && data->valid)
		return data;

	data->valid = 0;
	status = ipmi_send_message(&data->ipmi, IPMI_FAN_READ_CMD, NULL, 0,
								data->ipmi_resp, sizeof(data->ipmi_resp));
	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

	data->last_updated = jiffies;
	data->valid = 1;

exit:
	return data;
}

static ssize_t show_fan(struct device *dev, struct device_attribute *da,
							char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	unsigned char fid = attr->index / NUM_OF_PER_FAN_ATTR;
	int value = 0;
	int index = 0;
	int present = 0;
	int error = 0;

	mutex_lock(&data->update_lock);

	data = as7946_30xb_fan_update_device();
	if (!data->valid) {
		error = -EIO;
		goto exit;
	}

	index = fid * FAN_DATA_COUNT; /* base index */
	present = !!data->ipmi_resp[index + FAN_PRESENT];

	switch (attr->index) {
    case FAN1_PRESENT:
    case FAN2_PRESENT:
    case FAN3_PRESENT:
    case FAN4_PRESENT:
    case FAN5_PRESENT:
    case FAN6_PRESENT:
    case FAN7_PRESENT:
    case FAN8_PRESENT:
    case FAN9_PRESENT:
    case FAN10_PRESENT:
        value = present;
        break;
    case FAN1_PWM:
    case FAN2_PWM:
    case FAN3_PWM:
    case FAN4_PWM:
    case FAN5_PWM:
    case FAN6_PWM:
    case FAN7_PWM:
    case FAN8_PWM:
    case FAN9_PWM:
    case FAN10_PWM:
        index = (fid % NUM_OF_FAN_MODULE) * FAN_DATA_COUNT;
        value = data->ipmi_resp[index + FAN_PWM] + 1;
        break;
    case FAN1_INPUT:
    case FAN2_INPUT:
    case FAN3_INPUT:
    case FAN4_INPUT:
    case FAN5_INPUT:
    case FAN6_INPUT:
    case FAN7_INPUT:
    case FAN8_INPUT:
    case FAN9_INPUT:
    case FAN10_INPUT:
        value = (int)data->ipmi_resp[index + FAN_SPEED0] |
                (int)data->ipmi_resp[index + FAN_SPEED1] << 8;
        break;
    default:
        error = -EINVAL;
        goto exit;
	}

	mutex_unlock(&data->update_lock);
	return sprintf(buf, "%d\n", present ? value : 0);

exit:
	mutex_unlock(&data->update_lock);
	return error;
}

static ssize_t set_fan(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	long pwm;
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	unsigned char fid = attr->index / NUM_OF_PER_FAN_ATTR;

	status = kstrtol(buf, 10, &pwm);
	if (status)
		return status;

	pwm = pwm - 1; /* Convert pwm to register value */

	mutex_lock(&data->update_lock);

	/* Send IPMI write command */
	data->ipmi_tx_data[0] = (fid % NUM_OF_FAN_MODULE);
	data->ipmi_tx_data[1] = 0x02;
	data->ipmi_tx_data[2] = pwm;
	status = ipmi_send_message(&data->ipmi, IPMI_FAN_WRITE_CMD,
								data->ipmi_tx_data, sizeof(data->ipmi_tx_data),
								NULL, 0);
	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

	/* Update pwm to ipmi_resp buffer to prevent from the impact of lazy update */
	data->ipmi_resp[fid * FAN_DATA_COUNT + FAN_PWM] = pwm;
	status = count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static struct as7946_30xb_fan_data *as7946_30xb_fan_update_cpld_ver(void)
{
	int status = 0;

	data->valid = 0;
	data->ipmi_tx_data[0] = 0x68;
	status = ipmi_send_message(&data->ipmi, IPMI_FAN_REG_READ_CMD,
								data->ipmi_tx_data, 1,
								data->ipmi_resp_cpld,
								sizeof(data->ipmi_resp_cpld));
	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

	data->last_updated = jiffies;
	data->valid = 1;

exit:
	return data;
}

static ssize_t show_version(struct device *dev, struct device_attribute *da,
								char *buf)
{
	unsigned int value = 0;
	int error = 0;

	mutex_lock(&data->update_lock);

	data = as7946_30xb_fan_update_cpld_ver();
	if (!data->valid) {
		error = -EIO;
		goto exit;
	}

	value = data->ipmi_resp_cpld[0] << 8 | data->ipmi_resp_cpld[1];
	mutex_unlock(&data->update_lock);
	return sprintf(buf, "%d\n", value);

exit:
	mutex_unlock(&data->update_lock);
	return error;
}

static ssize_t show_dir(struct device *dev, struct device_attribute *da,
							char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	unsigned char fid = (attr->index / NUM_OF_PER_FAN_ATTR);
	int value = 0;
	int index = 0;
	int present = 0;
	int error = 0;

	mutex_lock(&data->update_lock);

	data = as7946_30xb_fan_update_device();
	if (!data->valid) {
		error = -EIO;
		goto exit;
	}

	index = fid * FAN_DATA_COUNT; /* base index */
	present = !!data->ipmi_resp[index + FAN_PRESENT];

	value = data->ipmi_resp[40] | (data->ipmi_resp[41] << 8);
	mutex_unlock(&data->update_lock);

	if (!present)
		return sprintf(buf, "0\n");
	else
		return sprintf(buf, "%s\n",
						(value & BIT(fid % NUM_OF_FAN_MODULE)) ? "B2F" : "F2B");

exit:
	mutex_unlock(&data->update_lock);
	return error;
}

static int as7946_30xb_fan_probe(struct platform_device *pdev)
{
	int status = -1;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &as7946_30xb_fan_group);
	if (status)
		goto exit;

	dev_info(&pdev->dev, "device created\n");

	return 0;

exit:
	return status;
}

static int as7946_30xb_fan_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &as7946_30xb_fan_group);

	return 0;
}

static int __init as7946_30xb_fan_init(void)
{
	int ret;

	data = kzalloc(sizeof(struct as7946_30xb_fan_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto alloc_err;
	}

	mutex_init(&data->update_lock);
	data->valid = 0;

	ret = platform_driver_register(&as7946_30xb_fan_driver);
	if (ret < 0)
		goto dri_reg_err;

	data->pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
	if (IS_ERR(data->pdev)) {
		ret = PTR_ERR(data->pdev);
		goto dev_reg_err;
	}

	/* Set up IPMI interface */
	ret = init_ipmi_data(&data->ipmi, 0, &data->pdev->dev);
	if (ret)
		goto ipmi_err;

	return 0;

ipmi_err:
	platform_device_unregister(data->pdev);
dev_reg_err:
	platform_driver_unregister(&as7946_30xb_fan_driver);
dri_reg_err:
	kfree(data);
alloc_err:
	return ret;
}

static void __exit as7946_30xb_fan_exit(void)
{
	ipmi_destroy_user(data->ipmi.user);
	platform_device_unregister(data->pdev);
	platform_driver_unregister(&as7946_30xb_fan_driver);
	kfree(data);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as7946_30xb fan driver");
MODULE_LICENSE("GPL");

module_init(as7946_30xb_fan_init);
module_exit(as7946_30xb_fan_exit);
