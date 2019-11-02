/*
 * Copyright (C)  Brandon Chuang <brandon_chuang@accton.com.tw>
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

#define DRVNAME "as5916_54xks_psu"
#define ACCTON_IPMI_NETFN       0x34
#define IPMI_PSU_READ_CMD       0x16
#define IPMI_PSU_MODEL_NAME_CMD 0x10
#define IPMI_PSU_SERIAL_NUM_CMD 0x11
#define IPMI_TIMEOUT		    (5 * HZ)
#define IPMI_ERR_RETRY_TIMES    1
#define IPMI_MODEL_SERIAL_LEN   32

static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static ssize_t show_linear(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t show_vout(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t show_psu(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t show_string(struct device *dev, struct device_attribute *attr, char *buf);
static int as5916_54xks_psu_probe(struct platform_device *pdev);
static int as5916_54xks_psu_remove(struct platform_device *pdev);

enum psu_id {
    PSU_1,
    PSU_2,
    NUM_OF_PSU
};

enum psu_data_index {
    PSU_PRESENT = 0,
    PSU_TEMP_FAULT,
    PSU_POWER_GOOD_CPLD,
    PSU_POWER_GOOD_PMBUS,
    PSU_OVER_VOLTAGE,
    PSU_OVER_CURRENT,
    PSU_POWER_ON,
    PSU_VIN0,
    PSU_VIN1,
    PSU_VOUT0,
    PSU_VOUT1,
    PSU_IOUT0,
    PSU_IOUT1,
    PSU_TEMP0,
    PSU_TEMP1,
    PSU_FAN0,
    PSU_FAN1,
    PSU_POUT0,
    PSU_POUT1,
    PSU_VOUT_MODE,
    PSU_STATUS_COUNT,
    PSU_MODEL = 0,
    PSU_SERIAL = 0
};

struct ipmi_data {
	struct completion   read_complete;
	struct ipmi_addr	address;
	ipmi_user_t         user;
	int                 interface;

	struct kernel_ipmi_msg tx_message;
	long                   tx_msgid;

	void            *rx_msg_data;
	unsigned short   rx_msg_len;
	unsigned char    rx_result;
	int              rx_recv_type;

	struct ipmi_user_hndl ipmi_hndlrs;
};

struct ipmi_psu_resp_data {
    unsigned char   status[20];
    char   serial[IPMI_MODEL_SERIAL_LEN+1];
    char   model[IPMI_MODEL_SERIAL_LEN+1];
};

struct as5916_54xks_psu_data {
    struct platform_device *pdev;
    struct mutex     update_lock;
    char             valid[2]; /* != 0 if registers are valid, 0: PSU1, 1: PSU2 */
    unsigned long    last_updated[2];    /* In jiffies, 0: PSU1, 1: PSU2 */
    struct ipmi_data ipmi;
    struct ipmi_psu_resp_data ipmi_resp[2]; /* 0: PSU1, 1: PSU2 */ 
    unsigned char ipmi_tx_data[2];
};

struct as5916_54xks_psu_data *data = NULL;

static struct platform_driver as5916_54xks_psu_driver = {
    .probe      = as5916_54xks_psu_probe,
    .remove     = as5916_54xks_psu_remove,
    .driver     = {
        .name   = DRVNAME,
        .owner  = THIS_MODULE,
    },
};

#define PSU_PRESENT_ATTR_ID(index)		PSU##index##_PRESENT
#define PSU_POWERGOOD_ATTR_ID(index)  	PSU##index##_POWER_GOOD
#define PSU_VOUT_ATTR_ID(index)         PSU##index##_VOUT
#define PSU_IOUT_ATTR_ID(index)         PSU##index##_IOUT
#define PSU_POUT_ATTR_ID(index)         PSU##index##_POUT
#define PSU_MODEL_ATTR_ID(index)        PSU##index##_MODEL
#define PSU_SERIAL_ATTR_ID(index)       PSU##index##_SERIAL
#define PSU_TEMP_INPUT_ATTR_ID(index)   PSU##index##_TEMP_INPUT
#define PSU_FAN_INPUT_ATTR_ID(index)    PSU##index##_FAN_INPUT

#define PSU_ATTR(psu_id) \
    PSU_PRESENT_ATTR_ID(psu_id),    \
    PSU_POWERGOOD_ATTR_ID(psu_id),  \
    PSU_VOUT_ATTR_ID(psu_id),       \
    PSU_IOUT_ATTR_ID(psu_id),       \
    PSU_POUT_ATTR_ID(psu_id),       \
    PSU_MODEL_ATTR_ID(psu_id),      \
    PSU_SERIAL_ATTR_ID(psu_id),     \
    PSU_TEMP_INPUT_ATTR_ID(psu_id), \
    PSU_FAN_INPUT_ATTR_ID(psu_id)

enum as5916_54x_psu_sysfs_attrs {
	/* psu attributes */
    PSU_ATTR(1),
    PSU_ATTR(2),
    NUM_OF_PSU_ATTR,
    NUM_OF_PER_PSU_ATTR = (NUM_OF_PSU_ATTR/NUM_OF_PSU)
};

/* psu attributes */
#define DECLARE_PSU_SENSOR_DEVICE_ATTR(index) \
	static SENSOR_DEVICE_ATTR(psu##index##_present,    S_IRUGO, show_psu, NULL, PSU##index##_PRESENT); \
	static SENSOR_DEVICE_ATTR(psu##index##_power_good, S_IRUGO, show_psu, NULL, PSU##index##_POWER_GOOD); \
	static SENSOR_DEVICE_ATTR(psu##index##_vout, S_IRUGO, show_vout,  NULL, PSU##index##_VOUT); \
	static SENSOR_DEVICE_ATTR(psu##index##_iout, S_IRUGO, show_linear,  NULL, PSU##index##_IOUT); \
	static SENSOR_DEVICE_ATTR(psu##index##_pout, S_IRUGO, show_linear,  NULL, PSU##index##_POUT); \
	static SENSOR_DEVICE_ATTR(psu##index##_model, S_IRUGO, show_string,  NULL, PSU##index##_MODEL); \
	static SENSOR_DEVICE_ATTR(psu##index##_serial, S_IRUGO, show_string,  NULL, PSU##index##_SERIAL);\
	static SENSOR_DEVICE_ATTR(psu##index##_temp1_input, S_IRUGO, show_psu,  NULL, PSU##index##_TEMP_INPUT); \
	static SENSOR_DEVICE_ATTR(psu##index##_fan1_input, S_IRUGO, show_psu,  NULL, PSU##index##_FAN_INPUT)	
#define DECLARE_PSU_ATTR(index) \
    &sensor_dev_attr_psu##index##_present.dev_attr.attr, \
    &sensor_dev_attr_psu##index##_power_good.dev_attr.attr, \
    &sensor_dev_attr_psu##index##_vout.dev_attr.attr, \
    &sensor_dev_attr_psu##index##_iout.dev_attr.attr, \
    &sensor_dev_attr_psu##index##_pout.dev_attr.attr, \
    &sensor_dev_attr_psu##index##_model.dev_attr.attr, \
    &sensor_dev_attr_psu##index##_serial.dev_attr.attr,\
    &sensor_dev_attr_psu##index##_temp1_input.dev_attr.attr, \
    &sensor_dev_attr_psu##index##_fan1_input.dev_attr.attr

DECLARE_PSU_SENSOR_DEVICE_ATTR(1);
DECLARE_PSU_SENSOR_DEVICE_ATTR(2);

static struct attribute *as5916_54xks_psu_attributes[] = {
    /* psu attributes */
    DECLARE_PSU_ATTR(1),
    DECLARE_PSU_ATTR(2),
    NULL
};

static const struct attribute_group as5916_54xks_psu_group = {
    .attrs = as5916_54xks_psu_attributes,
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
        status = _ipmi_send_message(ipmi, cmd, tx_data, tx_len, rx_data, rx_len);
        if (unlikely(status != 0)) {
            dev_err(&data->pdev->dev, "ipmi_send_message_%d err status(%d)\r\n", retry, status);
            continue;
        }

        if (unlikely(ipmi->rx_result != 0)) {
            dev_err(&data->pdev->dev, "ipmi_send_message_%d err rx_result(%d)\r\n", retry, ipmi->rx_result);
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
	} else
		ipmi->rx_msg_len = 0;

	ipmi_free_recv_msg(msg);
	complete(&ipmi->read_complete);
}

static struct as5916_54xks_psu_data *as5916_54xks_psu_update_device(struct device_attribute *da)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_PSU_ATTR;
    int status = 0;

    if (time_before(jiffies, data->last_updated[pid] + HZ * 5) && data->valid[pid]) {
        return data;
    }

    data->valid[pid] = 0;
    data->ipmi_resp[pid].status[PSU_VOUT_MODE] = 0xff; /* To be compatible for older BMC firmware */

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = pid + 1; /* PSU ID base id for ipmi start from 1 */
    status = ipmi_send_message(&data->ipmi, IPMI_PSU_READ_CMD, data->ipmi_tx_data, 1,
                                data->ipmi_resp[pid].status, sizeof(data->ipmi_resp[pid].status));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    /* Get model name from ipmi */
    data->ipmi_tx_data[1] = IPMI_PSU_MODEL_NAME_CMD;
    status = ipmi_send_message(&data->ipmi, IPMI_PSU_READ_CMD, data->ipmi_tx_data, 2,
                                data->ipmi_resp[pid].model, sizeof(data->ipmi_resp[pid].model) - 1);
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    /* Get serial number from ipmi */
    data->ipmi_tx_data[1] = IPMI_PSU_SERIAL_NUM_CMD;
    status = ipmi_send_message(&data->ipmi, IPMI_PSU_READ_CMD,  data->ipmi_tx_data, 2,
                                data->ipmi_resp[pid].serial, sizeof(data->ipmi_resp[pid].serial) - 1);
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->last_updated[pid] = jiffies;
    data->valid[pid] = 1;

exit:
    return data;
}

#define VALIDATE_PRESENT_RETURN(id) \
{ \
    if (data->ipmi_resp[id].status[PSU_PRESENT] != 0) { \
        mutex_unlock(&data->update_lock);   \
        return -ENXIO; \
    } \
}

static int two_complement_to_int(u16 data, u8 valid_bit, int mask)
{
	u16  valid_data  = data & mask;
	bool is_negative = valid_data >> (valid_bit - 1);

	return is_negative ? (-(((~valid_data) & mask) + 1)) : valid_data;
}

static ssize_t show_linear(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_PSU_ATTR;
    u16 value = 0;
    int error = 0;
	int exponent = 0, mantissa = 0;
	int multiplier = 1000;

    mutex_lock(&data->update_lock);

    data = as5916_54xks_psu_update_device(da);
    if (!data->valid[pid]) {
        error = -EIO;
        goto exit;
    }

	switch (attr->index) {
		case PSU1_VOUT:
		case PSU2_VOUT:
            VALIDATE_PRESENT_RETURN(pid);
			value = ((u16)data->ipmi_resp[pid].status[PSU_VOUT0] |
                     (u16)data->ipmi_resp[pid].status[PSU_VOUT1] << 8);
			break;
		case PSU1_IOUT:
		case PSU2_IOUT:
            VALIDATE_PRESENT_RETURN(pid);
			value = ((u16)data->ipmi_resp[pid].status[PSU_IOUT0] |
                     (u16)data->ipmi_resp[pid].status[PSU_IOUT1] << 8);
			break;
		case PSU1_POUT:
		case PSU2_POUT:
            VALIDATE_PRESENT_RETURN(pid);
			value = ((u16)data->ipmi_resp[pid].status[PSU_POUT0] |
                     (u16)data->ipmi_resp[pid].status[PSU_POUT1] << 8);
			break;
		default:
			error = -EINVAL;
            goto exit;
	}

    mutex_unlock(&data->update_lock);

	exponent = two_complement_to_int(value >> 11, 5, 0x1f);
	mantissa = two_complement_to_int(value & 0x7ff, 11, 0x7ff);

	return (exponent >= 0) ? sprintf(buf, "%d\n", (mantissa << exponent) * multiplier) :
							 sprintf(buf, "%d\n", (mantissa * multiplier) / (1 << -exponent));

exit:
    mutex_unlock(&data->update_lock);
    return error;
}

static ssize_t show_vout(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_PSU_ATTR;
    u16 value = 0;
    int error = 0;
	int exponent = 0, mantissa = 0;
	int multiplier = 1000;
    u8  vout_mode = 0xff;    

    mutex_lock(&data->update_lock);

    data = as5916_54xks_psu_update_device(da);
    if (!data->valid[pid]) {
        error = -EIO;
        goto exit;
    }

    vout_mode = (u8)data->ipmi_resp[pid].status[PSU_VOUT_MODE];
	value     = ((u16)data->ipmi_resp[pid].status[PSU_VOUT0] |
                 (u16)data->ipmi_resp[pid].status[PSU_VOUT1] << 8);

    mutex_unlock(&data->update_lock);

    if (vout_mode == 0xff) {
        exponent = two_complement_to_int(value >> 11, 5, 0x1f);
        mantissa = two_complement_to_int(value & 0x7ff, 11, 0x7ff);
    }
    else if (!(vout_mode & 0xe0)){
        exponent = two_complement_to_int(vout_mode & 0x1f, 5, 0x1f);
        mantissa = two_complement_to_int(value & 0xffff, 16, 0xffff);
    }
    else {
        return -EINVAL;
    }    

	return (exponent >= 0) ? sprintf(buf, "%d\n", (mantissa << exponent) * multiplier) :
							 sprintf(buf, "%d\n", (mantissa * multiplier) / (1 << -exponent));

exit:
    mutex_unlock(&data->update_lock);
    return error;
}

static ssize_t show_psu(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_PSU_ATTR;
    int value = 0;
    int error = 0;

    mutex_lock(&data->update_lock);

    data = as5916_54xks_psu_update_device(da);
    if (!data->valid[pid]) {
        error = -EIO;
        goto exit;
    }

	switch (attr->index) {
		case PSU1_PRESENT:
		case PSU2_PRESENT:
            value = !(data->ipmi_resp[pid].status[PSU_PRESENT]);
			break;
		case PSU1_POWER_GOOD:
		case PSU2_POWER_GOOD:
            VALIDATE_PRESENT_RETURN(pid);
			value = data->ipmi_resp[pid].status[PSU_POWER_GOOD_CPLD];
			break;
		case PSU1_TEMP_INPUT:
		case PSU2_TEMP_INPUT:
            VALIDATE_PRESENT_RETURN(pid);
			value = ((u32)data->ipmi_resp[pid].status[PSU_TEMP0] |
                     (u32)data->ipmi_resp[pid].status[PSU_TEMP1] << 8) * 1000;
			break;
		case PSU1_FAN_INPUT:
		case PSU2_FAN_INPUT:
            VALIDATE_PRESENT_RETURN(pid);
			value = ((u32)data->ipmi_resp[pid].status[PSU_FAN0] |
                     (u32)data->ipmi_resp[pid].status[PSU_FAN1] << 8);
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

static ssize_t show_string(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_PSU_ATTR;
    char *str = NULL;
    int error = 0;

    mutex_lock(&data->update_lock);

    data = as5916_54xks_psu_update_device(da);
    if (!data->valid[pid]) {
        error = -EIO;
        goto exit;
    }

	switch (attr->index) {
		case PSU1_MODEL:
		case PSU2_MODEL:
            VALIDATE_PRESENT_RETURN(pid);
            str = data->ipmi_resp[pid].model;
			break;
		case PSU1_SERIAL:
		case PSU2_SERIAL:
            VALIDATE_PRESENT_RETURN(pid);
            str = data->ipmi_resp[pid].serial;
			break;
		default:
			error = -EINVAL;
            goto exit;
    }

    mutex_unlock(&data->update_lock);
	return sprintf(buf, "%s\n", str);

exit:
    mutex_unlock(&data->update_lock);
    return error;
}

static int as5916_54xks_psu_probe(struct platform_device *pdev)
{
    int status = -1;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &as5916_54xks_psu_group);
	if (status) {
		goto exit;
	}


    dev_info(&pdev->dev, "device created\n");

    return 0;

exit:
    return status;
}

static int as5916_54xks_psu_remove(struct platform_device *pdev)
{
    sysfs_remove_group(&pdev->dev.kobj, &as5916_54xks_psu_group);
    return 0;
}

static int __init as5916_54xks_psu_init(void)
{
    int ret;

    data = kzalloc(sizeof(struct as5916_54xks_psu_data), GFP_KERNEL);
    if (!data) {
        ret = -ENOMEM;
        goto alloc_err;
    }

	mutex_init(&data->update_lock);

    ret = platform_driver_register(&as5916_54xks_psu_driver);
    if (ret < 0) {
        goto dri_reg_err;
    }

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
    platform_driver_unregister(&as5916_54xks_psu_driver);
dri_reg_err:
    kfree(data);
alloc_err:
    return ret;
}

static void __exit as5916_54xks_psu_exit(void)
{
    ipmi_destroy_user(data->ipmi.user);
    platform_device_unregister(data->pdev);
    platform_driver_unregister(&as5916_54xks_psu_driver);
    kfree(data);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("AS5916 54XKS PSU driver");
MODULE_LICENSE("GPL");

module_init(as5916_54xks_psu_init);
module_exit(as5916_54xks_psu_exit);

