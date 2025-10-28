/*
 * Copyright (C)  Jake Lin <jake_lin@edge-core.com>
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

#define DRVNAME "as7946_74xkb_thermal"
#define IPMI_THERMAL_READ_CMD 0x12
#define IPMI_THERMAL_WRITE_CMD 0x13

static ssize_t show_temp(struct device *dev, struct device_attribute *attr,
	char *buf);
static int as7946_74xkb_thermal_probe(struct platform_device *pdev);
static int as7946_74xkb_thermal_remove(struct platform_device *pdev);

enum temp_data_index {
	TEMP_ADDR,
	TEMP_FAULT,
	TEMP_INPUT,
	TEMP_DATA_COUNT
};

struct as7946_74xkb_thermal_data {
	struct platform_device *pdev;
	struct mutex update_lock;
	char valid;		   /* != 0 if registers are valid */
	unsigned long last_updated;	/* In jiffies */
	char   ipmi_resp[27]; /* 3 bytes for each thermal */
	struct ipmi_data ipmi;
	unsigned char ipmi_tx_data[2];  /* 0: thermal id, 1: temp */
};

struct as7946_74xkb_thermal_data *data = NULL;

static struct platform_driver as7946_74xkb_thermal_driver = {
	.probe = as7946_74xkb_thermal_probe,
	.remove = as7946_74xkb_thermal_remove,
	.driver = {
		.name = DRVNAME,
		.owner = THIS_MODULE,
	},
};

enum as7946_74xkb_thermal_sysfs_attrs {
	TEMP1_INPUT,
	TEMP2_INPUT,
	TEMP3_INPUT,
	TEMP4_INPUT,
	TEMP5_INPUT,
	TEMP6_INPUT,
	TEMP7_INPUT,
	TEMP8_INPUT,
	TEMP9_INPUT,
};

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, show_temp, NULL, TEMP1_INPUT);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO, show_temp, NULL, TEMP2_INPUT);
static SENSOR_DEVICE_ATTR(temp3_input, S_IRUGO, show_temp, NULL, TEMP3_INPUT);
static SENSOR_DEVICE_ATTR(temp4_input, S_IRUGO, show_temp, NULL, TEMP4_INPUT);
static SENSOR_DEVICE_ATTR(temp5_input, S_IRUGO, show_temp, NULL, TEMP5_INPUT);
static SENSOR_DEVICE_ATTR(temp6_input, S_IRUGO, show_temp, NULL, TEMP6_INPUT);
static SENSOR_DEVICE_ATTR(temp7_input, S_IRUGO, show_temp, NULL, TEMP7_INPUT);
static SENSOR_DEVICE_ATTR(temp8_input, S_IRUGO, show_temp, NULL, TEMP8_INPUT);
static SENSOR_DEVICE_ATTR(temp9_input, S_IRUGO, show_temp, NULL, TEMP9_INPUT);

static struct attribute *as7946_74xkb_thermal_attributes[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp4_input.dev_attr.attr,
	&sensor_dev_attr_temp5_input.dev_attr.attr,
	&sensor_dev_attr_temp6_input.dev_attr.attr,
	&sensor_dev_attr_temp7_input.dev_attr.attr,
	&sensor_dev_attr_temp8_input.dev_attr.attr,
	&sensor_dev_attr_temp9_input.dev_attr.attr,
	NULL
};

static const struct attribute_group as7946_74xkb_thermal_group = {
	.attrs = as7946_74xkb_thermal_attributes,
};

static ssize_t show_temp(struct device *dev, struct device_attribute *da,
							char *buf)
{
	int status = 0;
	int index  = 0;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ * 5) || !data->valid) {
		data->valid = 0;

		status = ipmi_send_message(&data->ipmi, IPMI_THERMAL_READ_CMD, NULL, 0,
									data->ipmi_resp, sizeof(data->ipmi_resp));
		if (unlikely(status != 0))
			goto exit;

		if (unlikely(data->ipmi.rx_result != 0)) {
			status = -EIO;
			goto exit;
		}

		data->last_updated = jiffies;
		data->valid = 1;
	}

	/* Get temp fault status */
	index = attr->index * TEMP_DATA_COUNT + TEMP_FAULT;
	if (unlikely(data->ipmi_resp[index] == 0)) {
		status = -EIO;
		goto exit;
	}

	/* Get temperature in degree celsius */
	index = attr->index * TEMP_DATA_COUNT + TEMP_INPUT;
	status = ((s8)data->ipmi_resp[index]) * 1000;

	mutex_unlock(&data->update_lock);
	return sprintf(buf, "%d\n", status);

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static int as7946_74xkb_thermal_probe(struct platform_device *pdev)
{
	int status = -1;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &as7946_74xkb_thermal_group);
	if (status)
		goto exit;

	dev_info(&pdev->dev, "device created\n");

	return 0;

exit:
	return status;
}

static int as7946_74xkb_thermal_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &as7946_74xkb_thermal_group);

	return 0;
}

static int __init as7946_74xkb_thermal_init(void)
{
	int ret;

	data = kzalloc(sizeof(struct as7946_74xkb_thermal_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto alloc_err;
	}

	mutex_init(&data->update_lock);
	data->valid = 0;

	ret = platform_driver_register(&as7946_74xkb_thermal_driver);
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
	platform_driver_unregister(&as7946_74xkb_thermal_driver);
dri_reg_err:
	kfree(data);
alloc_err:
	return ret;
}

static void __exit as7946_74xkb_thermal_exit(void)
{
	ipmi_destroy_user(data->ipmi.user);
	platform_device_unregister(data->pdev);
	platform_driver_unregister(&as7946_74xkb_thermal_driver);
	kfree(data);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("AS7946 74xkb Thermal driver");
MODULE_LICENSE("GPL");

module_init(as7946_74xkb_thermal_init);
module_exit(as7946_74xkb_thermal_exit);
