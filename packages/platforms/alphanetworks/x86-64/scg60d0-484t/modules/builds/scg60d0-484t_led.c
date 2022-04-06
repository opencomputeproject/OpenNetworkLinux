/*
 * A LED IPMI kernel dirver for alphanetworks scg60d0-484t
 *
 * Copyright (C) 2021 Alphanetworks Technology Corporation.
 * Fillmore Chen <fillmore_chen@alphanetworks.com>
 * 
 * Based on:
 * Copyright (C) 2021 Accton Technology Corporation.
 * Copyright (C)  Alex Lai <alex_lai@edge-core.com>
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
 * 	and pca9540.c from Jean Delvare <khali@linux-fr.org>.
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

#define DRVNAME 					        "scg60d0_led"
#define IPMI_APP_NETFN                      0x6

#define IPMI_READ_WRITE_CMD 	    	    0x52
#define IPMI_TIMEOUT				        (20 * HZ)
#define IPMI_ERR_RETRY_TIMES		        1

#define IPMI_CPLD_BUS  				        0x9
#define IPMI_CPLD_ADDR  			        0xbe
#define IPMI_CPLD_READ_ONE_BYTE  	        0x1
#define IPMI_CPLD_OFFSET_LED_PSU  		    0xa
#define IPMI_CPLD_OFFSET_LED_PWR            0xb

#define PWR_LED_BIT_GREEN                   0x0         /* Value 0: amber blinking 1: green solid*/
#define PSU1_LED_BIT_GREEN                  0x0         /* Value 0: amber blinking (present) or off (not present) 1: green solid*/
#define PSU2_LED_BIT_GREEN                  0x2         /* Value 0: amber blinking (present) or off (not present) 1: green solid*/

static unsigned int debug = 0;
module_param(debug, uint, S_IRUGO);
MODULE_PARM_DESC(debug, "Set DEBUG mode. Default is disabled.");


#define DEBUG_PRINT(fmt, args...)                                        \
    if (debug == 1)                                                      \
		printk (KERN_INFO "[%s,%d]: " fmt "\r\n", __FUNCTION__, __LINE__, ##args)


static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static ssize_t show_led(struct device *dev, struct device_attribute *da, char *buf);
static int scg60d0_led_probe(struct platform_device *pdev);
static int scg60d0_led_remove(struct platform_device *pdev);

enum led_data_index {
	PWR_INDEX = 0,
 	PSU1_INDEX,
 	PSU2_INDEX
};

struct ipmi_data {
	struct completion   		read_complete;
	struct ipmi_addr    		address;
	ipmi_user_t         		user;
	int                 		interface;

	struct kernel_ipmi_msg 		tx_message;
	long                   		tx_msgid;

	void            			*rx_msg_data;
	unsigned short   			rx_msg_len;
	unsigned char    			rx_result;
	int              			rx_recv_type;

	struct ipmi_user_hndl 		ipmi_hndlrs;
};

struct scg60d0_led_data {
	struct platform_device 			*pdev;
	struct mutex                    update_lock;
	char                            valid[3];           /* != 0 if registers are valid */
                                                        /* 0: PWR LED, 1: PSU1 LED 2: PSU2 LED */
	unsigned long                   last_updated[3];    /* In jiffies  0: PWR LED, 1: PSU1 LED 2: PSU2 LED */
	struct ipmi_data ipmi;   
	unsigned char                   ipmi_resp[3];       /* 0: PWR LED, 1: PSU1 LED 2: PSU2 LED */
	unsigned char                   ipmi_tx_data[4];
};

struct scg60d0_led_data *data = NULL;

static struct platform_driver scg60d0_led_driver = {
	.probe      = scg60d0_led_probe,
	.remove     = scg60d0_led_remove,
	.driver     = {
		.name   = DRVNAME,
		.owner  = THIS_MODULE,
	},
};

enum scg60d0_led_sysfs_attrs {

	LED_PWR,
	LED_PSU1,
	LED_PSU2
};

static SENSOR_DEVICE_ATTR(led_pwr, S_IRUGO, show_led, NULL, LED_PWR);
static SENSOR_DEVICE_ATTR(led_psu1, S_IRUGO, show_led, NULL, LED_PSU1);
static SENSOR_DEVICE_ATTR(led_psu2, S_IRUGO, show_led, NULL, LED_PSU2);

static struct attribute *scg60d0_led_attributes[] = {
	&sensor_dev_attr_led_pwr.dev_attr.attr,
	&sensor_dev_attr_led_psu1.dev_attr.attr,
	&sensor_dev_attr_led_psu2.dev_attr.attr,
	NULL
};

static const struct attribute_group scg60d0_led_group = {
	.attrs = scg60d0_led_attributes,
};

/* Functions to talk to the IPMI layer */

/* Initialize IPMI address, message buffers and user data */
static int init_ipmi_data(struct ipmi_data *ipmi, int iface,
			  struct device *dev)
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
	ipmi->tx_message.netfn = IPMI_APP_NETFN;

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

	ipmi->tx_message.cmd      = cmd;
	ipmi->tx_message.data     = tx_data;
	ipmi->tx_message.data_len = tx_len;
	ipmi->rx_msg_data         = rx_data;
	ipmi->rx_msg_len          = rx_len;

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
	dev_err(&data->pdev->dev, "request_timeout=%x\n", err);
	return err;
ipmi_req_err:
	dev_err(&data->pdev->dev, "request_settime=%x\n", err);
	return err;
addr_err:
	dev_err(&data->pdev->dev, "validate_addr=%x\n", err);
	return err;
}

/* Send an IPMI command with retry */
static int ipmi_send_message(struct ipmi_data *ipmi, unsigned char cmd,
			     unsigned char *tx_data, unsigned short tx_len,
			     unsigned char *rx_data, unsigned short rx_len)
{
	int status = 0, retry = 0;

	for (retry = 0; retry <= IPMI_ERR_RETRY_TIMES; retry++) {
		status = _ipmi_send_message(ipmi, cmd, tx_data, tx_len, 
					    rx_data, rx_len);
		if (unlikely(status != 0)) {
			dev_err(&data->pdev->dev, 
				"ipmi_send_message_%d err status(%d)\r\n",
				retry, status);
			continue;
		}

		if (unlikely(ipmi->rx_result != 0)) {
			dev_err(&data->pdev->dev, 
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
	unsigned short 		rx_len;
	struct ipmi_data 	*ipmi = user_msg_data;

	if (msg->msgid != ipmi->tx_msgid) {
		dev_err(&data->pdev->dev, "Mismatch between received msgid "
			"(%02x) and transmitted msgid (%02x)!\n",
			(int)msg->msgid,
			(int)ipmi->tx_msgid);
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
	} else {
		ipmi->rx_msg_len = 0;
	}

	ipmi_free_recv_msg(msg);
	complete(&ipmi->read_complete);
}

static struct scg60d0_led_data *
scg60d0_led_update_device(struct device_attribute *da)
{
	struct sensor_device_attribute 	*attr = to_sensor_dev_attr(da);
	int status = 0;

	if (time_before(jiffies, data->last_updated[attr->index] + HZ * 5) && 
                data->valid[attr->index])
		return data;

	data->valid[attr->index] = 0;
	/* Get LED status from ipmi */
	data->ipmi_tx_data[0] = IPMI_CPLD_BUS;
    data->ipmi_tx_data[1] = IPMI_CPLD_ADDR;
	data->ipmi_tx_data[2] = IPMI_CPLD_READ_ONE_BYTE;

	switch (attr->index) 
    {
		case LED_PWR:
			data->ipmi_tx_data[3] = IPMI_CPLD_OFFSET_LED_PWR;
			break;
		case LED_PSU1:
			data->ipmi_tx_data[3] = IPMI_CPLD_OFFSET_LED_PSU;
			break;
		case LED_PSU2:
			data->ipmi_tx_data[3] = IPMI_CPLD_OFFSET_LED_PSU;
			break;
		default:
			status = -EIO;
			goto exit;
	}

	status = ipmi_send_message(&data->ipmi, IPMI_READ_WRITE_CMD,
				   data->ipmi_tx_data, 4,
				   &data->ipmi_resp[attr->index],
				   sizeof(data->ipmi_resp[attr->index]));

	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

	data->last_updated[attr->index] = jiffies;
	data->valid[attr->index] = 1;

exit:
	return data;
}

static ssize_t show_led(struct device *dev, struct device_attribute *da,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	int value = 0;
	int error = 0;
	u8 mask = 0;

	mutex_lock(&data->update_lock);

	data = scg60d0_led_update_device(da);
	if (!data->valid[attr->index]) {
		error = -EIO;
		goto exit;
	}

    DEBUG_PRINT("scg60d0_484t show_led: ipmi_resp[%d]:0x%x", attr->index, data->ipmi_resp[attr->index]);

	switch (attr->index) {
	case LED_PWR:
		mask = 0x1 << PWR_LED_BIT_GREEN;
		value = !!(data->ipmi_resp[attr->index] & mask);
		break;
	case LED_PSU1:
		mask = 0x1 << PSU1_LED_BIT_GREEN;
		value = !!(data->ipmi_resp[attr->index] & mask);
		break;
	case LED_PSU2:
		mask = 0x1 << PSU2_LED_BIT_GREEN;
		value = !!(data->ipmi_resp[attr->index] & mask);
		break;
	default:
		error = -EINVAL;
		goto exit;
	}

	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n", value);

exit:
	mutex_unlock(&data->update_lock);
	return error;
}

static int scg60d0_led_probe(struct platform_device *pdev)
{
	int status = -1;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &scg60d0_led_group);
	if (status) {
		goto exit;
	}
    
	dev_info(&pdev->dev, "device created\n");

	return 0;

exit:
	return status;
}

static int scg60d0_led_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &scg60d0_led_group);

	return 0;
}

static int __init scg60d0_led_init(void)
{
	int ret;

	data = kzalloc(sizeof(struct scg60d0_led_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto alloc_err;
	}

	mutex_init(&data->update_lock);
	data->valid[0] = 0;
	data->valid[1] = 0;
	data->valid[2] = 0;

	ret = platform_driver_register(&scg60d0_led_driver);
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
	platform_driver_unregister(&scg60d0_led_driver);
dri_reg_err:
	kfree(data);
alloc_err:
	return ret;
}

static void __exit scg60d0_led_exit(void)
{
	ipmi_destroy_user(data->ipmi.user);
	platform_device_unregister(data->pdev);
	platform_driver_unregister(&scg60d0_led_driver);
	kfree(data);
}

module_init(scg60d0_led_init);
module_exit(scg60d0_led_exit);

MODULE_AUTHOR("Alpha-SID6");
MODULE_DESCRIPTION("Alphanetworks scg60d0-484t LED driver");
MODULE_LICENSE("GPL");

