/*
 * A Thermal IPMI kernel dirver for alphanetworks scg60d0-484t
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

#define DRVNAME                                     "scg60d0_thermal"
#define IPMI_SENSOR_NETFN                           0x4
#define IPMI_SENSOR_READ_CMD 		                0x2d
#define IPMI_TIMEOUT                                (20 * HZ)
#define IPMI_ERR_RETRY_TIMES                        1

#define IPMI_SENSOR_READING_OFFSET_TEMP_TMP75_0     0x60
#define IPMI_SENSOR_READING_OFFSET_TEMP_TMP75_1     0x61
#define IPMI_SENSOR_READING_OFFSET_TEMP_TMP435_L    0x62
#define IPMI_SENSOR_READING_OFFSET_TEMP_TMP435_R    0x63

static unsigned int debug = 0;
module_param(debug, uint, S_IRUGO);
MODULE_PARM_DESC(debug, "Set DEBUG mode. Default is disabled.");


#define DEBUG_PRINT(fmt, args...)                                        \
    if (debug == 1)                                                      \
		printk (KERN_INFO "[%s,%d]: " fmt "\r\n", __FUNCTION__, __LINE__, ##args)


static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static ssize_t show_temp(struct device *dev, struct device_attribute *da, char *buf);
static int scg60d0_thermal_probe(struct platform_device *pdev);
static int scg60d0_thermal_remove(struct platform_device *pdev);

enum thermal_id {
	THERMAL_1,
	THERMAL_2,
	THERMAL_3,
	THERMAL_4,
	NUM_OF_THERMAL
};
enum temp_data_index {
	TEMP_INPUT = 0,
	TEMP_DATA_COUNT
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

struct scg60d0_thermal_data {
	struct platform_device 			*pdev;
	struct mutex                    update_lock;
	char                            valid[4];                   /* != 0 if registers are valid,
                                                                   0: TMP75_0, 1: TMP75_1, 2: TMP435_L, 3: TMP435_R */
	unsigned long                   last_updated[4];            /* In jiffies 0: TMP75_0, 1: TMP75_1, 2: TMP435_L, 3: TMP435_R */
	struct ipmi_data                ipmi;
	unsigned char                   ipmi_resp_temp_status[4];   /* 0: TMP75_0, 1: TMP75_1, 2: TMP435_L, 3: TMP435_R */
	unsigned char                   ipmi_tx_data[4];
};

struct scg60d0_thermal_data *data = NULL;

static struct platform_driver scg60d0_thermal_driver = {
	.probe      = scg60d0_thermal_probe,
	.remove     = scg60d0_thermal_remove,
	.driver     = {
		.name   = DRVNAME,
		.owner  = THIS_MODULE,
	},
};

#define THERMAL_INPUT_ATTR_ID(index)		TEMP##index##_INPUT
#define THERMAL_ATTR(thermal_id) \
    THERMAL_INPUT_ATTR_ID(thermal_id)
enum scg60d0_thermal_sysfs_attrs {
	/* thermal attributes */
	THERMAL_ATTR(1),
	THERMAL_ATTR(2),
	THERMAL_ATTR(3),
	THERMAL_ATTR(4),
	NUM_OF_THERMAL_ATTR,
	NUM_OF_PER_THERMAL_ATTR = (NUM_OF_THERMAL_ATTR/NUM_OF_THERMAL)
};

/* thermal attributes */
#define DECLARE_THERMAL_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(temp##index##_input, S_IRUGO, show_temp, NULL, \
                  TEMP##index##_INPUT)

#define DECLARE_THERMAL_ATTR(index) \
	&sensor_dev_attr_temp##index##_input.dev_attr.attr

DECLARE_THERMAL_SENSOR_DEVICE_ATTR(1);
DECLARE_THERMAL_SENSOR_DEVICE_ATTR(2);
DECLARE_THERMAL_SENSOR_DEVICE_ATTR(3);
DECLARE_THERMAL_SENSOR_DEVICE_ATTR(4);

static struct attribute *scg60d0_thermal_attributes[] = {
	/* thermal attributes */
	DECLARE_THERMAL_ATTR(1),
	DECLARE_THERMAL_ATTR(2),
	DECLARE_THERMAL_ATTR(3),
	DECLARE_THERMAL_ATTR(4),
	NULL
};

static const struct attribute_group scg60d0_thermal_group = {
	.attrs = scg60d0_thermal_attributes,
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
	ipmi->tx_message.netfn = IPMI_SENSOR_NETFN;

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

static struct scg60d0_thermal_data *
scg60d0_thermal_update_temp_status(struct device_attribute *da)
{
	struct sensor_device_attribute 	*attr = to_sensor_dev_attr(da);
	unsigned char                   tid = attr->index / NUM_OF_PER_THERMAL_ATTR;
	int status = 0;

	if (time_before(jiffies, data->last_updated[tid] + HZ * 5) && 
                data->valid[tid])
		return data;

	data->valid[tid] = 0;
	/* Get Thermal status from ipmi */

	switch (tid) 
	{
		case THERMAL_1:
			data->ipmi_tx_data[0] = IPMI_SENSOR_READING_OFFSET_TEMP_TMP75_0;
			break;
		case THERMAL_2:
			data->ipmi_tx_data[0] = IPMI_SENSOR_READING_OFFSET_TEMP_TMP75_1;
			break;
		case THERMAL_3:
			data->ipmi_tx_data[0] = IPMI_SENSOR_READING_OFFSET_TEMP_TMP435_L;
			break;
		case THERMAL_4:
			data->ipmi_tx_data[0] = IPMI_SENSOR_READING_OFFSET_TEMP_TMP435_R;
			break;
		default:
			status = -EIO;
			goto exit;
	}

	status = ipmi_send_message(&data->ipmi, IPMI_SENSOR_READ_CMD,
				   data->ipmi_tx_data, 1,
				   &data->ipmi_resp_temp_status[tid],
				   sizeof(data->ipmi_resp_temp_status[tid]));

	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

    data->last_updated[tid] = jiffies;
	data->valid[tid] = 1;
exit:
	return data;
}

static ssize_t show_temp(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute  *attr = to_sensor_dev_attr(da);
	unsigned char                   tid = attr->index / NUM_OF_PER_THERMAL_ATTR;
	int value = 0;
	int error = 0;

	mutex_lock(&data->update_lock);

	data = scg60d0_thermal_update_temp_status(da);
	if (!data->valid[tid]) {
		error = -EIO;
		goto exit;
	}

    DEBUG_PRINT("scg60d0_484t show_temp: ipmi_resp_temp_status[%d]:0x%x", tid, data->ipmi_resp_temp_status[tid]);

	switch (attr->index) {
	case TEMP1_INPUT:
	case TEMP2_INPUT:
	case TEMP3_INPUT:
	case TEMP4_INPUT:
		value = data->ipmi_resp_temp_status[tid] * 1000;
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

static int scg60d0_thermal_probe(struct platform_device *pdev)
{
	int status = -1;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &scg60d0_thermal_group);
	if (status) {
		goto exit;
	}
    
	dev_info(&pdev->dev, "device created\n");

	return 0;

exit:
	return status;
}

static int scg60d0_thermal_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &scg60d0_thermal_group);	

	return 0;
}

static int __init scg60d0_thermal_init(void)
{
	int ret;

	data = kzalloc(sizeof(struct scg60d0_thermal_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto alloc_err;
	}

	mutex_init(&data->update_lock);
	data->valid[0] = 0;
	data->valid[1] = 0;
	data->valid[2] = 0;
	data->valid[3] = 0;

	ret = platform_driver_register(&scg60d0_thermal_driver);
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
	platform_driver_unregister(&scg60d0_thermal_driver);
dri_reg_err:
	kfree(data);
alloc_err:
	return ret;
}

static void __exit scg60d0_thermal_exit(void)
{
	ipmi_destroy_user(data->ipmi.user);
	platform_device_unregister(data->pdev);
	platform_driver_unregister(&scg60d0_thermal_driver);
	kfree(data);
}

module_init(scg60d0_thermal_init);
module_exit(scg60d0_thermal_exit);

MODULE_AUTHOR("Alpha-SID6");
MODULE_DESCRIPTION("Alphanetworks scg60d0-484t Thermal driver");
MODULE_LICENSE("GPL");

