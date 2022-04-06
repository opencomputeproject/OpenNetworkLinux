/*
 * A FAN IPMI kernel dirver for alphanetworks scg60d0-484t
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

#define DRVNAME                                 "scg60d0_fan"
#define IPMI_APP_NETFN                          0x6
#define IPMI_SENSOR_NETFN                       0x4
#define IPMI_PWM_NETFN                          0x34

#define IPMI_READ_WRITE_CMD 		            0x52
#define IPMI_SENSOR_READ_CMD 		            0x2d
#define IPMI_PWM_READ_CMD 		                0x3
#define IPMI_PWM_WRITE_CMD 		                0x4

#define IPMI_TIMEOUT                            (20 * HZ)
#define IPMI_ERR_RETRY_TIMES                    1

#define IPMI_CPLD_BUS                           0x9
#define IPMI_CPLD_ADDR                          0xbe
#define IPMI_CPLD_READ_ONE_BYTE                 0x1

#define IPMI_CPLD_OFFSET_FAN1                   0x11
#define IPMI_CPLD_OFFSET_FAN2                   0x12
#define IPMI_CPLD_OFFSET_FAN3                   0x13
#define IPMI_CPLD_OFFSET_FAN4                   0x14

#define IPMI_SENSOR_READING_OFFSET_SPEED_FAN1   0x31
#define IPMI_SENSOR_READING_OFFSET_SPEED_FAN2   0x32
#define IPMI_SENSOR_READING_OFFSET_SPEED_FAN3   0x33
#define IPMI_SENSOR_READING_OFFSET_SPEED_FAN4   0x34

#define IPMI_OEM_FAN_PWN_OFFSET_FAN1            0x1
#define IPMI_OEM_FAN_PWN_OFFSET_FAN2            0x2
#define IPMI_OEM_FAN_PWN_OFFSET_FAN3            0x3
#define IPMI_OEM_FAN_PWN_OFFSET_FAN4            0x4

#define FAN_STATUS_BIT_RPS_ERROR    2
#define FAN_STATUS_BIT_PRESENT      1
#define FAN_STATUS_BIT_DIRECTION    0

static unsigned int debug = 0;
module_param(debug, uint, S_IRUGO);
MODULE_PARM_DESC(debug, "Set DEBUG mode. Default is disabled.");


#define DEBUG_PRINT(fmt, args...)                                        \
    if (debug == 1)                                                      \
		printk (KERN_INFO "[%s,%d]: " fmt "\r\n", __FUNCTION__, __LINE__, ##args)


static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static ssize_t show_fan(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_fan_speed(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_fan_pwm(struct device *dev, struct device_attribute *da, char *buf);
static int scg60d0_fan_probe(struct platform_device *pdev);
static int scg60d0_fan_remove(struct platform_device *pdev);

enum fan_id {
	FAN_1,
	FAN_2,
	FAN_3,
	FAN_4,
	NUM_OF_FAN
};

enum fan_data_index {
	FAN_PRESENT = 0,
	FAN_FAULT,
	FAN_DIR,
	FAN_SPEED,
	FAN_PWM,
	FAN_DATA_COUNT
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

struct scg60d0_fan_data {
	struct platform_device 			*pdev;
	struct mutex                    update_lock;
	char             				fan_speed_valid[4];         /* 0: FAN1, 1: FAN2, 2: FAN3, 3: FAN4 */
	char             				fan_pwm_valid[4];           /* 0: FAN1, 1: FAN2, 2: FAN3, 3: FAN4 */
	char                            valid[4];                   /* != 0 if registers are valid,
                                                                   0: FAN1, 1: FAN2, 2: FAN3, 3: FAN4 */
	unsigned long                   last_updated[4];            /* In jiffies 0: FAN1, 1: FAN2, 2: FAN3, 3: FAN4 */
	struct ipmi_data                ipmi;
	unsigned char                   ipmi_resp_fan_status[4];    /* 0: FAN1, 1: FAN2, 2: FAN3, 3: FAN4 */
	unsigned char                   ipmi_resp_fan_speed[4];     /* 0: FAN1, 1: FAN2, 2: FAN3, 3: FAN4 */
	unsigned char                   ipmi_resp_fan_pwm[4];       /* 0: FAN1, 1: FAN2, 2: FAN3, 3: FAN4 */
	unsigned char                   ipmi_tx_data[4];
};

struct scg60d0_fan_data *data = NULL;

static struct platform_driver scg60d0_fan_driver = {
	.probe      = scg60d0_fan_probe,
	.remove     = scg60d0_fan_remove,
	.driver     = {
		.name   = DRVNAME,
		.owner  = THIS_MODULE,
	},
};

#define FAN_PRESENT_ATTR_ID(index)		FAN##index##_PRESENT
#define FAN_FAULT_ATTR_ID(index)		FAN##index##_FAULT
#define FAN_DIR_ATTR_ID(index)			FAN##index##_DIR
#define FAN_RPM_ATTR_ID(index)			FAN##index##_INPUT
#define FAN_PWM_ATTR_ID(index)			FAN##index##_PWM

#define FAN_ATTR(fan_id) \
	FAN_PRESENT_ATTR_ID(fan_id),    	\
	FAN_FAULT_ATTR_ID(fan_id),    		\
	FAN_DIR_ATTR_ID(fan_id),        	\
	FAN_RPM_ATTR_ID(fan_id),			\
	FAN_PWM_ATTR_ID(fan_id)

enum scg60d0_fan_sysfs_attrs {
	/* fan attributes */
	FAN_ATTR(1),
	FAN_ATTR(2),
	FAN_ATTR(3),
	FAN_ATTR(4),
	NUM_OF_FAN_ATTR,
	NUM_OF_PER_FAN_ATTR = (NUM_OF_FAN_ATTR/NUM_OF_FAN)
};

/* fan attributes */
#define DECLARE_FAN_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_present, S_IRUGO, show_fan, NULL, \
                  FAN##index##_PRESENT); \
	static SENSOR_DEVICE_ATTR(fan##index##_fault, S_IRUGO, show_fan, NULL, \
				  FAN##index##_FAULT); \
	static SENSOR_DEVICE_ATTR(fan##index##_dir, S_IRUGO, show_fan, NULL, \
				  FAN##index##_DIR); \
	static SENSOR_DEVICE_ATTR(fan##index##_input, S_IRUGO, show_fan_speed, NULL, \
				  FAN##index##_INPUT); \
	static SENSOR_DEVICE_ATTR(fan##index##_pwm, S_IWUSR | S_IRUGO, \
				  show_fan_pwm, NULL, FAN##index##_PWM)
                  
                  
#define DECLARE_FAN_ATTR(index) \
	&sensor_dev_attr_fan##index##_present.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_fault.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_dir.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_input.dev_attr.attr, \
	&sensor_dev_attr_fan##index##_pwm.dev_attr.attr

DECLARE_FAN_SENSOR_DEVICE_ATTR(1);
DECLARE_FAN_SENSOR_DEVICE_ATTR(2);
DECLARE_FAN_SENSOR_DEVICE_ATTR(3);
DECLARE_FAN_SENSOR_DEVICE_ATTR(4);

static struct attribute *scg60d0_fan_attributes[] = {
	/* fan attributes */
	DECLARE_FAN_ATTR(1),
	DECLARE_FAN_ATTR(2),
	DECLARE_FAN_ATTR(3),
	DECLARE_FAN_ATTR(4),
	NULL
};

static const struct attribute_group scg60d0_fan_group = {
	.attrs = scg60d0_fan_attributes,
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

static struct scg60d0_fan_data *
scg60d0_fan_update_device(struct device_attribute *da)
{
	struct sensor_device_attribute 	*attr = to_sensor_dev_attr(da);
	unsigned char                   fid = attr->index / NUM_OF_PER_FAN_ATTR;
	int status = 0;

	if (time_before(jiffies, data->last_updated[fid] + HZ * 5) && 
                data->valid[fid])
		return data;

	data->valid[fid] = 0;
	/* Get FAN status from ipmi */
    data->ipmi.tx_message.netfn = IPMI_APP_NETFN;
	data->ipmi_tx_data[0] = IPMI_CPLD_BUS;
    data->ipmi_tx_data[1] = IPMI_CPLD_ADDR;
	data->ipmi_tx_data[2] = IPMI_CPLD_READ_ONE_BYTE;

	switch (fid) 
	{
		case FAN_1:
			data->ipmi_tx_data[3] = IPMI_CPLD_OFFSET_FAN1;
			break;
		case FAN_2:
			data->ipmi_tx_data[3] = IPMI_CPLD_OFFSET_FAN2;
			break;
		case FAN_3:
			data->ipmi_tx_data[3] = IPMI_CPLD_OFFSET_FAN3;
			break;
		case FAN_4:
			data->ipmi_tx_data[3] = IPMI_CPLD_OFFSET_FAN4;
			break;
		default:
			status = -EIO;
			goto exit;
	}

	status = ipmi_send_message(&data->ipmi, IPMI_READ_WRITE_CMD,
				   data->ipmi_tx_data, 4,
				   &data->ipmi_resp_fan_status[fid],
				   sizeof(data->ipmi_resp_fan_status[fid]));
	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}


	data->last_updated[fid] = jiffies;
	data->valid[fid] = 1;

exit:
	return data;
}

static struct scg60d0_fan_data *
scg60d0_fan_update_fan_speed(struct device_attribute *da)
{
	struct sensor_device_attribute 	*attr = to_sensor_dev_attr(da);
	unsigned char                   fid = attr->index / NUM_OF_PER_FAN_ATTR;
	int status = 0;

	data->fan_speed_valid[fid] = 0;
	/* Get FAN SPEED from ipmi */
    data->ipmi.tx_message.netfn = IPMI_SENSOR_NETFN;

	switch (fid) 
	{
		case FAN_1:
			data->ipmi_tx_data[0] = IPMI_SENSOR_READING_OFFSET_SPEED_FAN1;
			break;
		case FAN_2:
			data->ipmi_tx_data[0] = IPMI_SENSOR_READING_OFFSET_SPEED_FAN2;
			break;
		case FAN_3:
			data->ipmi_tx_data[0] = IPMI_SENSOR_READING_OFFSET_SPEED_FAN3;
			break;
		case FAN_4:
			data->ipmi_tx_data[0] = IPMI_SENSOR_READING_OFFSET_SPEED_FAN4;
			break;
		default:
			status = -EIO;
			goto exit;
	}

	status = ipmi_send_message(&data->ipmi, IPMI_SENSOR_READ_CMD,
				   data->ipmi_tx_data, 1,
				   &data->ipmi_resp_fan_speed[fid],
				   sizeof(data->ipmi_resp_fan_speed[fid]));

	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

	data->fan_speed_valid[fid] = 1;
exit:
	return data;
}

static struct scg60d0_fan_data *
scg60d0_fan_update_fan_pwm(struct device_attribute *da)
{
	struct sensor_device_attribute 	*attr = to_sensor_dev_attr(da);
	unsigned char                   fid = attr->index / NUM_OF_PER_FAN_ATTR;
	int status = 0;

	data->fan_pwm_valid[fid] = 0;
	/* Get FAN SPEED from ipmi */
    data->ipmi.tx_message.netfn = IPMI_PWM_NETFN;

	switch (fid) 
	{
		case FAN_1:
			data->ipmi_tx_data[0] = IPMI_OEM_FAN_PWN_OFFSET_FAN1;
			break;
		case FAN_2:
			data->ipmi_tx_data[0] = IPMI_OEM_FAN_PWN_OFFSET_FAN2;
			break;
		case FAN_3:
			data->ipmi_tx_data[0] = IPMI_OEM_FAN_PWN_OFFSET_FAN3;
			break;
		case FAN_4:
			data->ipmi_tx_data[0] = IPMI_OEM_FAN_PWN_OFFSET_FAN4;
			break;
		default:
			status = -EIO;
			goto exit;
	}

	status = ipmi_send_message(&data->ipmi, IPMI_PWM_READ_CMD,
				   data->ipmi_tx_data, 1,
				   &data->ipmi_resp_fan_pwm[fid],
				   sizeof(data->ipmi_resp_fan_pwm[fid]));

	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

	data->fan_pwm_valid[fid] = 1;
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

static ssize_t show_fan(struct device *dev, struct device_attribute *da,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	unsigned char fid = attr->index / NUM_OF_PER_FAN_ATTR;
	int value = 0;

	int present = 0;
	int error = 0;
	u8 mask = 0;

	mutex_lock(&data->update_lock);

	data = scg60d0_fan_update_device(da);
	if (!data->valid[fid]) {
		error = -EIO;
		goto exit;
	}

	DEBUG_PRINT("scg60d0_484t show_fan: ipmi_resp_fan_status[%d]:0x%x", 
                fid, data->ipmi_resp_fan_status[fid]);

	mask = 0x1 << FAN_STATUS_BIT_PRESENT;
	present = !(data->ipmi_resp_fan_status[fid] & mask);

	switch (attr->index) {
	case FAN1_PRESENT:
	case FAN2_PRESENT:
	case FAN3_PRESENT:
	case FAN4_PRESENT:
		value = present;
		break;
	case FAN1_FAULT:
	case FAN2_FAULT:
	case FAN3_FAULT:
	case FAN4_FAULT:
		VALIDATE_PRESENT_RETURN(pid);
		mask = 0x1 << FAN_STATUS_BIT_RPS_ERROR;
		value = !!(data->ipmi_resp_fan_status[fid] & mask);
		break;
	case FAN1_DIR:
	case FAN2_DIR:
	case FAN3_DIR:
	case FAN4_DIR:
		VALIDATE_PRESENT_RETURN(pid);
		mask = 0x1 << FAN_STATUS_BIT_DIRECTION;
		value = !!(data->ipmi_resp_fan_status[fid] & mask);
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

static ssize_t show_fan_speed(struct device *dev, 
                struct device_attribute *da, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	unsigned char fid = attr->index / NUM_OF_PER_FAN_ATTR;
	int value = 0;
	int error = 0;

	mutex_lock(&data->update_lock);

	data = scg60d0_fan_update_fan_speed(da);
	if (!data->fan_speed_valid[fid]) {
		error = -EIO;
		goto exit;
	}

    DEBUG_PRINT("scg60d0_484t show_fan_speed: ipmi_resp_fan_speed[%d]:0x%x", fid, data->ipmi_resp_fan_speed[fid]);

	switch (attr->index) {
	case FAN1_INPUT:
	case FAN2_INPUT:
	case FAN3_INPUT:
	case FAN4_INPUT:
		value = data->ipmi_resp_fan_speed[fid] * 118;
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

static ssize_t show_fan_pwm(struct device *dev, 
                struct device_attribute *da, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	unsigned char fid = attr->index / NUM_OF_PER_FAN_ATTR;
	int value = 0;
	int error = 0;

	mutex_lock(&data->update_lock);

	data = scg60d0_fan_update_fan_pwm(da);
	if (!data->fan_pwm_valid[fid]) {
		error = -EIO;
		goto exit;
	}

    DEBUG_PRINT("scg60d0_484t show_fan_pwm: ipmi_resp_fan_pwm[%d]:0x%x", fid, data->ipmi_resp_fan_pwm[fid]);

	switch (attr->index) {
	case FAN1_PWM:
	case FAN2_PWM:
	case FAN3_PWM:
	case FAN4_PWM:
		value = data->ipmi_resp_fan_pwm[fid];
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

static int scg60d0_fan_probe(struct platform_device *pdev)
{
	int status = -1;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &scg60d0_fan_group);
	if (status) {
		goto exit;
	}
    
	dev_info(&pdev->dev, "device created\n");

	return 0;

exit:
	return status;
}

static int scg60d0_fan_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &scg60d0_fan_group);

	return 0;
}

static int __init scg60d0_fan_init(void)
{
	int ret;

	data = kzalloc(sizeof(struct scg60d0_fan_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto alloc_err;
	}

	mutex_init(&data->update_lock);
	data->valid[0] = 0;
	data->valid[1] = 0;
	data->valid[2] = 0;
	data->valid[3] = 0;

	ret = platform_driver_register(&scg60d0_fan_driver);
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
	platform_driver_unregister(&scg60d0_fan_driver);
dri_reg_err:
	kfree(data);
alloc_err:
	return ret;
}

static void __exit scg60d0_fan_exit(void)
{
	ipmi_destroy_user(data->ipmi.user);
	platform_device_unregister(data->pdev);
	platform_driver_unregister(&scg60d0_fan_driver);
	kfree(data);
}

module_init(scg60d0_fan_init);
module_exit(scg60d0_fan_exit);

MODULE_AUTHOR("Alpha-SID6");
MODULE_DESCRIPTION("Alphanetworks scg60d0-484t FAN driver");
MODULE_LICENSE("GPL");

