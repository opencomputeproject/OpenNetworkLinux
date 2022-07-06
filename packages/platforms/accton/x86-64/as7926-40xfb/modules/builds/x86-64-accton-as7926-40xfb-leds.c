// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * A hwmon driver for the as7926_40xfb_led
 *
 * Copyright (C) 2019 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@edge-core.com>
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

#define DRVNAME "as7926_40xfb_led"
#define ACCTON_IPMI_NETFN 0x34
#define IPMI_LED_READ_CMD 0x1A
#define IPMI_LED_WRITE_CMD 0x1B
#define IPMI_TIMEOUT (5 * HZ)
#define IPMI_ERR_RETRY_TIMES 1

#define LED_7SGMT_BUS 32
#define LED_7SGMT_I2C_ADDR 0x20
#define LED_7SGMT_STATUS_REG_OFFSET 0x02
#define LED_7SGMT_CONFIG_REG_OFFSET 0x06

/* The map of 7-segment LED register value to digital number.
   index 0 is for digital number 0, index 10 is for OFF
 */
static const int regval_map_7sgmt[] = {
	0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x83, 0xD8, 0x80, 0x98, 0xFF
};

static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static ssize_t set_led(struct device *dev, struct device_attribute *da,
		       const char *buf, size_t count);
static ssize_t set_7sgmt_led(struct device *dev, struct device_attribute *da,
		       const char *buf, size_t count);
static ssize_t show_led(struct device *dev, struct device_attribute *attr,
			char *buf);
static ssize_t show_7sgmt_led(struct device *dev, struct device_attribute *attr,
			char *buf);
static int as7926_40xfb_led_probe(struct platform_device *pdev);
static int as7926_40xfb_led_remove(struct platform_device *pdev);

extern int as7926_40xfb_cpld_read(int bus_num, unsigned short cpld_addr, u8 reg);
extern int as7926_40xfb_cpld_write(int bus_num, unsigned short cpld_addr, u8 reg,
		       u8 value);

struct ipmi_data {
	struct completion read_complete;
	struct ipmi_addr address;
	struct ipmi_user *user;
	int interface;

	struct kernel_ipmi_msg tx_message;
	long tx_msgid;

	void *rx_msg_data;
	unsigned short rx_msg_len;
	unsigned char rx_result;
	int rx_recv_type;

	struct ipmi_user_hndl ipmi_hndlrs;
};

struct as7926_40xfb_sys_led_data {
	struct platform_device *pdev;
	struct mutex update_lock;
	char valid;		/* != 0 if registers are valid */
	unsigned long last_updated;	/* In jiffies */
	unsigned char ipmi_resp[4];	/* 0:PSU 1:FAN 2:DIAG 3:LOC */
	struct ipmi_data ipmi;
};

struct as7926_40xfb_7segment_led_data {
	struct platform_device *pdev;
	struct mutex update_lock;
	char valid;		/* != 0 if registers are valid */
	unsigned long last_updated;	/* In jiffies */
	unsigned char regval[2]; /* 0:LEFT 1:RIGHT */
};

struct as7926_40xfb_sys_led_data *data_sys = NULL;
struct as7926_40xfb_7segment_led_data *data_7sgmt = NULL;

static struct platform_driver as7926_40xfb_led_driver = {
	.probe = as7926_40xfb_led_probe,
	.remove = as7926_40xfb_led_remove,
	.driver = {
		   .name = DRVNAME,
		   .owner = THIS_MODULE,
		   },
};

enum ipmi_led_light_mode {
	IPMI_LED_MODE_OFF,
	IPMI_LED_MODE_RED = 2,
	IPMI_LED_MODE_RED_BLINKING = 3,
	IPMI_LED_MODE_GREEN = 4,
	IPMI_LED_MODE_GREEN_BLINKING = 5,
	IPMI_LED_MODE_BLUE = 8,
	IPMI_LED_MODE_BLUE_BLINKING = 9,
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

enum as7926_40xfb_led_sysfs_attrs {
	LED_PSU,
	LED_LOC,
	LED_DIAG,
	LED_FAN,
	LED_7SGMT_LEFT,
	LED_7SGMT_RIGHT
};

static SENSOR_DEVICE_ATTR(led_loc, S_IWUSR | S_IRUGO, show_led, set_led,
			  LED_LOC);
static SENSOR_DEVICE_ATTR(led_diag, S_IWUSR | S_IRUGO, show_led, set_led,
			  LED_DIAG);
static SENSOR_DEVICE_ATTR(led_psu, S_IRUGO, show_led, NULL, LED_PSU);
static SENSOR_DEVICE_ATTR(led_fan, S_IRUGO, show_led, NULL, LED_FAN);
static SENSOR_DEVICE_ATTR(led_7sgmt_left, S_IWUSR | S_IRUGO, show_7sgmt_led,
			  set_7sgmt_led, LED_7SGMT_LEFT);
static SENSOR_DEVICE_ATTR(led_7sgmt_right, S_IWUSR | S_IRUGO, show_7sgmt_led,
			  set_7sgmt_led, LED_7SGMT_RIGHT);

static struct attribute *as7926_40xfb_led_attributes[] = {
	&sensor_dev_attr_led_loc.dev_attr.attr,
	&sensor_dev_attr_led_diag.dev_attr.attr,
	&sensor_dev_attr_led_psu.dev_attr.attr,
	&sensor_dev_attr_led_fan.dev_attr.attr,
	&sensor_dev_attr_led_7sgmt_left.dev_attr.attr,
	&sensor_dev_attr_led_7sgmt_right.dev_attr.attr,
	NULL
};

static const struct attribute_group as7926_40xfb_led_group = {
	.attrs = as7926_40xfb_led_attributes,
};

/* Functions to talk to the IPMI layer */

/* Initialize IPMI address, message buffers and user data */
static int init_ipmi_data(struct ipmi_data *ipmi, int iface, struct device *dev)
{
	int err;

	init_completion(&ipmi->read_complete);

	/* Initialize IPMI address */
	ipmi->address.addr_type = IPMI_SYSTEM_INTERFACE_ADDR_TYPE;
	ipmi->address.channel = IPMI_BMC_CHANNEL;
	ipmi->address.data[0] = 0;
	ipmi->interface = iface;

	/* Initialize message buffers */
	ipmi->tx_msgid = 0;
	ipmi->tx_message.netfn = ACCTON_IPMI_NETFN;

	ipmi->ipmi_hndlrs.ipmi_recv_hndl = ipmi_msg_handler;

	/* Create IPMI messaging interface user */
	err = ipmi_create_user(ipmi->interface, &ipmi->ipmi_hndlrs,
			       ipmi, &ipmi->user);
	if (err < 0) {
		dev_err(dev, "Unable to register user with IPMI "
			"interface %d\n", ipmi->interface);
		return -EACCES;
	}

	return 0;
}

/* Send an IPMI command */
static int _ipmi_send_message(struct ipmi_data *ipmi, unsigned char cmd,
			      unsigned char *tx_data, unsigned short tx_len,
			      unsigned char *rx_data, unsigned short rx_len)
{
	int err;

	ipmi->tx_message.cmd = cmd;
	ipmi->tx_message.data = tx_data;
	ipmi->tx_message.data_len = tx_len;
	ipmi->rx_msg_data = rx_data;
	ipmi->rx_msg_len = rx_len;

	err = ipmi_validate_addr(&ipmi->address, sizeof(ipmi->address));
	if (err)
		goto addr_err;

	ipmi->tx_msgid++;
	err = ipmi_request_settime(ipmi->user, &ipmi->address, ipmi->tx_msgid,
				   &ipmi->tx_message, ipmi, 0, 0, 0);
	if (err)
		goto ipmi_req_err;

	err = wait_for_completion_timeout(&ipmi->read_complete, IPMI_TIMEOUT);
	if (!err)
		goto ipmi_timeout_err;

	return 0;

 ipmi_timeout_err:
	err = -ETIMEDOUT;
	dev_err(&data_sys->pdev->dev, "request_timeout=%x\n", err);
	return err;
 ipmi_req_err:
	dev_err(&data_sys->pdev->dev, "request_settime=%x\n", err);
	return err;
 addr_err:
	dev_err(&data_sys->pdev->dev, "validate_addr=%x\n", err);
	return err;
}

/* Send an IPMI command with retry */
static int ipmi_send_message(struct ipmi_data *ipmi, unsigned char cmd,
			     unsigned char *tx_data, unsigned short tx_len,
			     unsigned char *rx_data, unsigned short rx_len)
{
	int status = 0, retry = 0;

	for (retry = 0; retry <= IPMI_ERR_RETRY_TIMES; retry++) {
		status =
		    _ipmi_send_message(ipmi, cmd, tx_data, tx_len, rx_data,
				       rx_len);
		if (unlikely(status != 0)) {
			dev_err(&data_sys->pdev->dev,
				"ipmi_send_message_%d err status(%d)\r\n",
				retry, status);
			continue;
		}

		if (unlikely(ipmi->rx_result != 0)) {
			dev_err(&data_sys->pdev->dev,
				"ipmi_send_message_%d err result(%d)\r\n",
				retry, ipmi->rx_result);
			continue;
		}

		break;
	}

	return status;
}

/* Dispatch IPMI messages to callers */
static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data)
{
	unsigned short rx_len;
	struct ipmi_data *ipmi = user_msg_data;

	if (msg->msgid != ipmi->tx_msgid) {
		dev_err(&data_sys->pdev->dev, "Mismatch between received msgid "
			"(%02x) and transmitted msgid (%02x)!\n",
			(int)msg->msgid, (int)ipmi->tx_msgid);
		ipmi_free_recv_msg(msg);
		return;
	}

	ipmi->rx_recv_type = msg->recv_type;
	if (msg->msg.data_len > 0)
		ipmi->rx_result = msg->msg.data[0];
	else
		ipmi->rx_result = IPMI_UNKNOWN_ERR_COMPLETION_CODE;

	if (msg->msg.data_len > 1) {
		rx_len = msg->msg.data_len - 1;
		if (ipmi->rx_msg_len < rx_len)
			rx_len = ipmi->rx_msg_len;
		ipmi->rx_msg_len = rx_len;
		memcpy(ipmi->rx_msg_data, msg->msg.data + 1, ipmi->rx_msg_len);
	} else
		ipmi->rx_msg_len = 0;

	ipmi_free_recv_msg(msg);
	complete(&ipmi->read_complete);
}

static struct as7926_40xfb_sys_led_data *as7926_40xfb_sys_led_update_device(void)
{
	int status = 0;

	if (time_before(jiffies, data_sys->last_updated + HZ * 5) && data_sys->valid) {
		return data_sys;
	}

	data_sys->valid = 0;
	status = ipmi_send_message(&data_sys->ipmi, IPMI_LED_READ_CMD, NULL, 0,
				   data_sys->ipmi_resp, sizeof(data_sys->ipmi_resp));
	if (unlikely(status != 0)) {
		goto exit;
	}

	if (unlikely(data_sys->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

	data_sys->last_updated = jiffies;
	data_sys->valid = 1;

 exit:
	return data_sys;
}

static ssize_t show_led(struct device *dev, struct device_attribute *da,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	int value = 0;
	int error = 0;

	if (attr->index == LED_PSU || attr->index == LED_FAN)
		return sprintf(buf, "%d\n", LED_MODE_AUTO);

	mutex_lock(&data_sys->update_lock);

	as7926_40xfb_sys_led_update_device();
	if (!data_sys->valid) {
		error = -EIO;
		goto exit;
	}

	switch (data_sys->ipmi_resp[attr->index]) {
	case IPMI_LED_MODE_OFF:
		value = LED_MODE_OFF;
		break;
	case IPMI_LED_MODE_RED:
		value = LED_MODE_RED;
		break;
	case IPMI_LED_MODE_RED_BLINKING:
		value = LED_MODE_RED_BLINKING;
		break;
	case IPMI_LED_MODE_GREEN:
		value = LED_MODE_GREEN;
		break;
	case IPMI_LED_MODE_GREEN_BLINKING:
		value = LED_MODE_GREEN_BLINKING;
		break;
	case IPMI_LED_MODE_BLUE:
		value = LED_MODE_BLUE;
		break;
	case IPMI_LED_MODE_BLUE_BLINKING:
		value = LED_MODE_BLUE_BLINKING;
		break;
	default:
		error = -EINVAL;
		goto exit;
	}

	mutex_unlock(&data_sys->update_lock);
	return sprintf(buf, "%d\n", value);

 exit:
	mutex_unlock(&data_sys->update_lock);
	return error;
}

static ssize_t set_led(struct device *dev, struct device_attribute *da,
		       const char *buf, size_t count)
{
	long mode;
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

	status = kstrtol(buf, 10, &mode);
	if (status)
		return status;

	mutex_lock(&data_sys->update_lock);

	as7926_40xfb_sys_led_update_device();
	if (!data_sys->valid) {
		status = -EIO;
		goto exit;
	}

	data_sys->ipmi_resp[0] = attr->index + 1;

	switch (mode) {
	case LED_MODE_OFF:
		data_sys->ipmi_resp[1] = IPMI_LED_MODE_OFF;
		break;
	case LED_MODE_RED:
		data_sys->ipmi_resp[1] = IPMI_LED_MODE_RED;
		break;
	case LED_MODE_RED_BLINKING:
		data_sys->ipmi_resp[1] = IPMI_LED_MODE_RED_BLINKING;
		break;
	case LED_MODE_GREEN:
		data_sys->ipmi_resp[1] = IPMI_LED_MODE_GREEN;
		break;
	case LED_MODE_GREEN_BLINKING:
		data_sys->ipmi_resp[1] = IPMI_LED_MODE_GREEN_BLINKING;
		break;
	case LED_MODE_BLUE:
		data_sys->ipmi_resp[1] = IPMI_LED_MODE_BLUE;
		break;
	case LED_MODE_BLUE_BLINKING:
		data_sys->ipmi_resp[1] = IPMI_LED_MODE_BLUE_BLINKING;
		break;
	default:
		status = -EINVAL;
		goto exit;
	}

	/* Send IPMI write command */
	status = ipmi_send_message(&data_sys->ipmi, IPMI_LED_WRITE_CMD,
				   data_sys->ipmi_resp, 2, NULL, 0);
	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data_sys->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

	data_sys->valid = 0; /* Force next update if led is set successfully */
	status = count;

 exit:
	mutex_unlock(&data_sys->update_lock);
	return status;
}

static struct as7926_40xfb_7segment_led_data *as7926_40xfb_7sgmt_led_update_device(void)
{
	int i, status = 0;

	if (time_before(jiffies, data_7sgmt->last_updated + HZ * 5) && data_7sgmt->valid) {
		return data_7sgmt;
	}

	data_7sgmt->valid = 0;

	/* Read led status */
	for (i = 0; i < ARRAY_SIZE(data_7sgmt->regval); i++) {
		status = as7926_40xfb_cpld_read(LED_7SGMT_BUS, LED_7SGMT_I2C_ADDR, LED_7SGMT_STATUS_REG_OFFSET+i);
		if (status < 0) {
			dev_dbg(&data_7sgmt->pdev->dev, "7-segment led reg (0x%x) err %d\n", LED_7SGMT_STATUS_REG_OFFSET+i, status);
			goto exit;
		}
		else {
			data_7sgmt->regval[i] = status;
		}
	}

	data_7sgmt->last_updated = jiffies;
	data_7sgmt->valid = 1;

 exit:
	return data_7sgmt;
}

static int seven_sgmt_led_regval_to_digit(int regval)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(regval_map_7sgmt); i++) {
		if (regval == regval_map_7sgmt[i])
			return i;
	}

	return -EINVAL;
}

static int seven_sgmt_led_digit_to_regval(int digit)
{
	if (digit < 0 || digit >= ARRAY_SIZE(regval_map_7sgmt))
		return -EINVAL;

	return regval_map_7sgmt[digit];
}

static ssize_t show_7sgmt_led(struct device *dev, struct device_attribute *da,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	int status = 0;

	mutex_lock(&data_7sgmt->update_lock);

	as7926_40xfb_7sgmt_led_update_device();
	if (!data_7sgmt->valid) {
		status = -EIO;
		goto exit;
	}

	status = data_7sgmt->regval[attr->index - LED_7SGMT_LEFT];
	status = seven_sgmt_led_regval_to_digit(status);
	if (status < 0)
		goto exit;

	mutex_unlock(&data_7sgmt->update_lock);
	return sprintf(buf, "%d\n", status);

 exit:
	mutex_unlock(&data_7sgmt->update_lock);
	return status;
}

static ssize_t set_7sgmt_led(struct device *dev, struct device_attribute *da,
		       const char *buf, size_t count)
{
	long mode;
	int reg;
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

	status = kstrtol(buf, 10, &mode);
	if (status)
		return status;

	mutex_lock(&data_7sgmt->update_lock);

	/* enable write access for 7-segment led */
	reg = LED_7SGMT_CONFIG_REG_OFFSET + (attr->index - LED_7SGMT_LEFT);
	status = as7926_40xfb_cpld_write(LED_7SGMT_BUS, LED_7SGMT_I2C_ADDR, reg, 0);
	if (status < 0)
		goto exit;

	status = seven_sgmt_led_digit_to_regval(mode);
	if (status < 0)
		goto exit;

	reg = LED_7SGMT_STATUS_REG_OFFSET + (attr->index - LED_7SGMT_LEFT);
	status = as7926_40xfb_cpld_write(LED_7SGMT_BUS, LED_7SGMT_I2C_ADDR, reg, status);
	if (status < 0)
		goto exit;

	data_7sgmt->valid = 0; /* Force next update if led is set successfully */
	status = count;

 exit:
	mutex_unlock(&data_7sgmt->update_lock);
	return status;
}

static int as7926_40xfb_led_probe(struct platform_device *pdev)
{
	int status = -1;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &as7926_40xfb_led_group);
	if (status)
		goto exit;

	dev_info(&pdev->dev, "device created\n");

	return 0;

 exit:
	return status;
}

static int as7926_40xfb_led_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &as7926_40xfb_led_group);

	return 0;
}

static int __init as7926_40xfb_led_init(void)
{
	int ret;

	data_sys = kzalloc(sizeof(struct as7926_40xfb_sys_led_data), GFP_KERNEL);
	if (!data_sys) {
		ret = -ENOMEM;
		goto alloc_err;
	}

	data_7sgmt = kzalloc(sizeof(struct as7926_40xfb_7segment_led_data), GFP_KERNEL);
	if (!data_7sgmt) {
		ret = -ENOMEM;
		goto alloc_err_7sgmt;
	}

	mutex_init(&data_sys->update_lock);
	mutex_init(&data_7sgmt->update_lock);
	data_sys->valid = 0;
	data_7sgmt->valid = 0;

	ret = platform_driver_register(&as7926_40xfb_led_driver);
	if (ret < 0)
		goto dri_reg_err;

	data_sys->pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
	if (IS_ERR(data_sys->pdev)) {
		ret = PTR_ERR(data_sys->pdev);
		goto dev_reg_err;
	}

	/* Set up IPMI interface */
	ret = init_ipmi_data(&data_sys->ipmi, 0, &data_sys->pdev->dev);
	if (ret)
		goto ipmi_err;

	data_7sgmt->pdev = data_sys->pdev;

	return 0;

 ipmi_err:
	platform_device_unregister(data_sys->pdev);
 dev_reg_err:
	platform_driver_unregister(&as7926_40xfb_led_driver);
 dri_reg_err:
	kfree(data_7sgmt);
 alloc_err_7sgmt:
	kfree(data_sys);
 alloc_err:
	return ret;
}

static void __exit as7926_40xfb_led_exit(void)
{
	ipmi_destroy_user(data_sys->ipmi.user);
	platform_device_unregister(data_sys->pdev);
	platform_driver_unregister(&as7926_40xfb_led_driver);
	kfree(data_7sgmt);
	kfree(data_sys);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("AS7926 40XFB led driver");
MODULE_LICENSE("GPL");

module_init(as7926_40xfb_led_init);
module_exit(as7926_40xfb_led_exit);
