/*
 * A Power CPLD IPMI kernel dirver for alphanetworks scg60d0-484t
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

#define DRVNAME 					"scg60d0_pwr_cpld"
#define IPMI_APP_NETFN              0x6

#define IPMI_READ_WRITE_CMD 		0x52
#define IPMI_TIMEOUT				(20 * HZ)
#define IPMI_ERR_RETRY_TIMES		1

#define IPMI_CPLD_BUS  				0x9
#define IPMI_CPLD_ADDR  			0xbe
#define IPMI_CPLD_READ_ONE_BYTE  	0x1

#define IPMI_CPLD_OFFSET_VERSION    0x0
#define IPMI_CPLD_OFFSET_PSU1  		0x3
#define IPMI_CPLD_OFFSET_PSU2       0x4

#define PSU_STATUS_BIT_PRESENT      0x2
#define PSU_STATUS_BIT_POWER_OK     0x1
#define CPLD_VERSION_BITS_MASK    	0xF


static unsigned int debug = 0;
module_param(debug, uint, S_IRUGO);
MODULE_PARM_DESC(debug, "Set DEBUG mode. Default is disabled.");


#define DEBUG_PRINT(fmt, args...)                                        \
    if (debug == 1)                                                      \
		printk (KERN_INFO "[%s,%d]: " fmt "\r\n", __FUNCTION__, __LINE__, ##args)


static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static ssize_t show_psu(struct device *dev,  struct device_attribute *da, char *buf);
static ssize_t show_cpld_version(struct device *dev, struct device_attribute *da, char *buf);
static int scg60d0_pwr_cpld_probe(struct platform_device *pdev);
static int scg60d0_pwr_cpld_remove(struct platform_device *pdev);

enum psu_id {
	PSU_1,
	PSU_2,
	NUM_OF_PSU
};

enum psu_data_index {
	PSU_PRESENT = 0,
	PSU_POWER_GOOD_CPLD,

};

struct ipmi_data {
	struct completion   	read_complete;
	struct ipmi_addr		address;
	ipmi_user_t         	user;
	int                 	interface;

	struct kernel_ipmi_msg 	tx_message;
	long                   	tx_msgid;

	void            		*rx_msg_data;
	unsigned short   		rx_msg_len;
	unsigned char    		rx_result;
	int              		rx_recv_type;

	struct ipmi_user_hndl 	ipmi_hndlrs;
};

struct ipmi_psu_resp_data {
	unsigned char   		status[32];

};

struct scg60d0_pwr_cpld_data {
	struct platform_device 			*pdev;
	struct mutex 					update_lock;
	char 							valid[2]; 				/* != 0 if registers are valid, 0: PSU1, 1: PSU2 */
	char             				cpld_ver_valid;      	/* != 0 if registers are valid */
	unsigned long 					last_updated[2];		/* In jiffies, 0: PSU1, 1: PSU2 */
	struct ipmi_data 				ipmi;
	struct ipmi_psu_resp_data 		ipmi_resp[2]; 			/* 0: PSU1, 1: PSU2 */
	unsigned char    				ipmi_resp_cpld;         /* PWR CPLD */
	unsigned char 					ipmi_tx_data[4];
};

struct scg60d0_pwr_cpld_data *data = NULL;

static struct platform_driver scg60d0_pwr_cpld_driver = {
	.probe      = scg60d0_pwr_cpld_probe,
	.remove     = scg60d0_pwr_cpld_remove,
	.driver     = {
		.name   = DRVNAME,
		.owner  = THIS_MODULE,
	},
};

#define PSU_PRESENT_ATTR_ID(index) 		PSU##index##_PRESENT
#define PSU_POWERGOOD_ATTR_ID(index) 	PSU##index##_POWER_GOOD

#define PSU_ATTR(psu_id) \
	PSU_PRESENT_ATTR_ID(psu_id), \
	PSU_POWERGOOD_ATTR_ID(psu_id)
	
enum scg60d0_pwr_cpld_sysfs_attrs {
	/* psu attributes */
	PSU_ATTR(1),
	PSU_ATTR(2),
	NUM_OF_PSU_ATTR,
	NUM_OF_PER_PSU_ATTR = (NUM_OF_PSU_ATTR/NUM_OF_PSU) ,
	PWR_CPLD_VER = NUM_OF_PSU_ATTR
};

/* psu attributes */
#define DECLARE_PSU_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(psu##index##_present,	S_IRUGO, show_psu, \
				  NULL, PSU##index##_PRESENT); \
	static SENSOR_DEVICE_ATTR(psu##index##_power_good, S_IRUGO, show_psu, \
				  NULL, PSU##index##_POWER_GOOD)

#define DECLARE_PSU_ATTR(index) \
	&sensor_dev_attr_psu##index##_present.dev_attr.attr, \
	&sensor_dev_attr_psu##index##_power_good.dev_attr.attr

DECLARE_PSU_SENSOR_DEVICE_ATTR(1);
DECLARE_PSU_SENSOR_DEVICE_ATTR(2);
static SENSOR_DEVICE_ATTR(pwr_cpld_ver, S_IRUGO, show_cpld_version, NULL, PWR_CPLD_VER);

static struct attribute *scg60d0_pwr_cpld_attributes[] = {
	/* psu attributes */
	DECLARE_PSU_ATTR(1),
	DECLARE_PSU_ATTR(2),
	/* pwr cpld attributes */
	&sensor_dev_attr_pwr_cpld_ver.dev_attr.attr,
	NULL

};

static const struct attribute_group scg60d0_pwr_cpld_group = {
	.attrs = scg60d0_pwr_cpld_attributes,
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

static struct scg60d0_pwr_cpld_data *
scg60d0_pwr_cpld_update_device(struct device_attribute *da)
{
	struct sensor_device_attribute 	*attr = to_sensor_dev_attr(da);
	unsigned char 					pid = attr->index / NUM_OF_PER_PSU_ATTR;
	int status = 0;

	if (time_before(jiffies, data->last_updated[pid] + HZ * 5) && 
		        data->valid[pid])
		return data;

	data->valid[pid] = 0;
	/* Get PSU status from ipmi */
	data->ipmi_tx_data[0] = IPMI_CPLD_BUS;
    data->ipmi_tx_data[1] = IPMI_CPLD_ADDR;
	data->ipmi_tx_data[2] = IPMI_CPLD_READ_ONE_BYTE;

	switch (pid) 
	{
		case PSU_1:
			data->ipmi_tx_data[3] = IPMI_CPLD_OFFSET_PSU1;
			break;
		case PSU_2:
			data->ipmi_tx_data[3] = IPMI_CPLD_OFFSET_PSU2;
			break;
		default:
			status = -EIO;
			goto exit;
	}

	status = ipmi_send_message(&data->ipmi, IPMI_READ_WRITE_CMD,
				   data->ipmi_tx_data, 4,
				   data->ipmi_resp[pid].status,
				   sizeof(data->ipmi_resp[pid].status));
	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}


	data->last_updated[pid] = jiffies;
	data->valid[pid] = 1;

exit:
	return data;
}

static struct scg60d0_pwr_cpld_data *
scg60d0_pwr_cpld_update_cpld_ver(unsigned char cpld_addr)
{
	int status = 0;

	data->cpld_ver_valid = 0;
	data->ipmi_tx_data[0] = IPMI_CPLD_BUS;
    data->ipmi_tx_data[1] = cpld_addr;
	data->ipmi_tx_data[2] = IPMI_CPLD_READ_ONE_BYTE;
	data->ipmi_tx_data[3] = IPMI_CPLD_OFFSET_VERSION;

	status = ipmi_send_message(&data->ipmi, IPMI_READ_WRITE_CMD,
				   data->ipmi_tx_data, 4,
				   &data->ipmi_resp_cpld, 
				   sizeof(data->ipmi_resp_cpld));

	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

	data->cpld_ver_valid = 1;
exit:
	return data;
}

#define VALIDATE_PRESENT_RETURN(id) \
do { \
	if (present == 0) { \
		mutex_unlock(&data->update_lock);   \
		return -ENXIO; \
	} \
} while (0)

static ssize_t show_psu(struct device *dev, struct device_attribute *da,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	unsigned char pid = attr->index / NUM_OF_PER_PSU_ATTR;
	u32 value = 0;
	int present = 0;
	int error = 0;
	int multiplier = 1000;
	u8 mask = 0;

	mutex_lock(&data->update_lock);

	data = scg60d0_pwr_cpld_update_device(da);
	if (!data->valid[pid]) {
		error = -EIO;
		goto exit;
	}
	
	DEBUG_PRINT("scg60d0_484t show_psu: data->ipmi_resp[%d].status[0]:0x%x",
				pid, data->ipmi_resp[pid].status[0]);

	mask = 0x1 << PSU_STATUS_BIT_PRESENT;
	present = !!(data->ipmi_resp[pid].status[0] & mask);

	switch (attr->index) {
	case PSU1_PRESENT:
	case PSU2_PRESENT:
		value = present;
		break;
	case PSU1_POWER_GOOD:
	case PSU2_POWER_GOOD:
		VALIDATE_PRESENT_RETURN(pid);
		mask = 0x1 << PSU_STATUS_BIT_POWER_OK;
		value = !!(data->ipmi_resp[pid].status[0] & mask);
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

static ssize_t show_cpld_version(struct device *dev, 
                struct device_attribute *da, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	unsigned char cpld_addr = 0, value = 0;
	int error = 0;

	switch (attr->index) {
	case PWR_CPLD_VER:
		cpld_addr = IPMI_CPLD_ADDR;
		break;
	default:
		return -EINVAL;
	}

	mutex_lock(&data->update_lock);

	data = scg60d0_pwr_cpld_update_cpld_ver(cpld_addr);
	if (!data->valid) {
		error = -EIO;
		goto exit;
	}

	value = data->ipmi_resp_cpld & CPLD_VERSION_BITS_MASK;
	mutex_unlock(&data->update_lock);
	
	DEBUG_PRINT("scg60d0_484t show_cpld_version: data->ipmi_resp_cpld:0x%x", data->ipmi_resp_cpld);
				
	return sprintf(buf, "%d\n", value);

exit:
	mutex_unlock(&data->update_lock);
	return error;    
}

static int scg60d0_pwr_cpld_probe(struct platform_device *pdev)
{
	int status = -1;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &scg60d0_pwr_cpld_group);
	if (status) {
		goto exit;
	}

	dev_info(&pdev->dev, "device created\n");

	return 0;

exit:
	return status;
}

static int scg60d0_pwr_cpld_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &scg60d0_pwr_cpld_group);
	return 0;
}

static int __init scg60d0_pwr_cpld_init(void)
{
	int ret;

	data = kzalloc(sizeof(struct scg60d0_pwr_cpld_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto alloc_err;
	}

	mutex_init(&data->update_lock);
	data->valid[0] = 0;
	data->valid[1] = 0;

	ret = platform_driver_register(&scg60d0_pwr_cpld_driver);
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
	platform_driver_unregister(&scg60d0_pwr_cpld_driver);
dri_reg_err:
	kfree(data);
alloc_err:
	return ret;
}

static void __exit scg60d0_pwr_cpld_exit(void)
{
	ipmi_destroy_user(data->ipmi.user);
	platform_device_unregister(data->pdev);
	platform_driver_unregister(&scg60d0_pwr_cpld_driver);
	kfree(data);
}

module_init(scg60d0_pwr_cpld_init);
module_exit(scg60d0_pwr_cpld_exit);

MODULE_AUTHOR("Alpha-SID6");
MODULE_DESCRIPTION("Alphanetworks scg60d0-484t Power CPLD driver");
MODULE_LICENSE("GPL");
