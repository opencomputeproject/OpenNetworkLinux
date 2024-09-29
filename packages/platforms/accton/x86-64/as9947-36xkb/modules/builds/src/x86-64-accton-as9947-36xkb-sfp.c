/*
 * Copyright (C)  Willy Liu <willy_liu@accton.com.tw>
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

#define DRVNAME "as9947_36xkb_sfp"
#define ACCTON_IPMI_NETFN       0x34
/* SFP */
#define IPMI_SFP_READ_CMD       0x1C
#define IPMI_SFP_WRITE_CMD      0x1D

#define IPMI_SFP_PRESENT_CMD    0x10
#define IPMI_SFP_TXDIS_CMD      0x1
#define IPMI_SFP_TXFAULT_CMD    0x12
#define IPMI_SFP_RXLOS_CMD      0x13
/* QSFP */
#define IPMI_QSFP_READ_CMD      0x10
#define IPMI_QSFP_WRITE_CMD     0x11

#define IPMI_QSFP_PRESENT_CMD    0x10
#define IPMI_QSFP_RESET_CMD      0x11
#define IPMI_QSFP_LPMODE_CMD     0x12

#define IPMI_TIMEOUT		(20 * HZ)
#define IPMI_DATA_MAX_LEN       128

#define NUM_OF_SFP              4
#define NUM_OF_QSFP             36
#define NUM_OF_PORT             (NUM_OF_SFP + NUM_OF_QSFP)

static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static ssize_t set_sfp_txdisable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t show_sfp(struct device *dev, struct device_attribute *da, char *buf);

static ssize_t set_qsfp_reset(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t set_qsfp_lpmode(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t show_qsfp(struct device *dev, struct device_attribute *da, char *buf);
static int as9947_36xkb_sfp_probe(struct platform_device *pdev);
static int as9947_36xkb_sfp_remove(struct platform_device *pdev);
static ssize_t show_all(struct device *dev, struct device_attribute *da, char *buf);
static struct as9947_36xkb_sfp_data *as9947_36xkb_sfp_update_present(void);
static struct as9947_36xkb_sfp_data *as9947_36xkb_sfp_update_txdisable(void);
static struct as9947_36xkb_sfp_data *as9947_36xkb_sfp_update_txfault(void);
static struct as9947_36xkb_sfp_data *as9947_36xkb_sfp_update_rxlos(void);
static struct as9947_36xkb_sfp_data *as9947_36xkb_qsfp_update_present(void);
static struct as9947_36xkb_sfp_data *as9947_36xkb_qsfp_update_lpmode(void);
static struct as9947_36xkb_sfp_data *as9947_36xkb_qsfp_update_reset(void);

struct ipmi_data {
	struct completion   read_complete;
	struct ipmi_addr	address;
    struct ipmi_user * user;
	int                 interface;

	struct kernel_ipmi_msg tx_message;
	long                   tx_msgid;

	void            *rx_msg_data;
	unsigned short   rx_msg_len;
	unsigned char    rx_result;
	int              rx_recv_type;

	struct ipmi_user_hndl ipmi_hndlrs;
};

enum module_status {
    SFP_PRESENT = 0,
    SFP_TXDISABLE,
    SFP_TXFAULT,
    SFP_RXLOS,
    NUM_OF_SFP_STATUS,

    QSFP_PRESENT = 0,
    QSFP_RESET,
    QSFP_LPMODE,
    NUM_OF_QSFP_STATUS,

    PRESENT_ALL = 0,
    RXLOS_ALL,
};

struct ipmi_sfp_resp_data {
    char          sfp_valid[NUM_OF_SFP_STATUS];        /* != 0 if registers are valid */
    unsigned long sfp_last_updated[NUM_OF_SFP_STATUS]; /* In jiffies */
    unsigned char sfp_resp[NUM_OF_SFP_STATUS][NUM_OF_SFP]; /* 0: present,  1: tx-disable
                                                                 2: tx-fault, 3: rx-los  */
    char          qsfp_valid[NUM_OF_QSFP_STATUS];       /* != 0 if registers are valid */
    unsigned long qsfp_last_updated[NUM_OF_QSFP_STATUS]; /* In jiffies */
    unsigned char qsfp_resp[NUM_OF_QSFP_STATUS][NUM_OF_QSFP]; /* 0: present, 1: reset, 
                                                                 2: low power mode */
};

struct as9947_36xkb_sfp_data {
    struct platform_device *pdev;
    struct mutex     update_lock;
    struct ipmi_data ipmi;
    struct ipmi_sfp_resp_data ipmi_resp;
    unsigned char ipmi_tx_data[3];
};

struct as9947_36xkb_sfp_data *data = NULL;

static struct platform_driver as9947_36xkb_sfp_driver = {
    .probe      = as9947_36xkb_sfp_probe,
    .remove     = as9947_36xkb_sfp_remove,
    .driver     = {
        .name   = DRVNAME,
        .owner  = THIS_MODULE,
    },
};

#define SFP_PRESENT_ATTR_ID(port)		SFP##port##_PRESENT
#define SFP_TXDISABLE_ATTR_ID(port)	    SFP##port##_TXDISABLE
#define SFP_TXFAULT_ATTR_ID(port)		SFP##port##_TXFAULT
#define SFP_RXLOS_ATTR_ID(port)		    SFP##port##_RXLOS

#define SFP_ATTR(port) \
    SFP_PRESENT_ATTR_ID(port),    \
    SFP_TXDISABLE_ATTR_ID(port),  \
    SFP_TXFAULT_ATTR_ID(port),    \
    SFP_RXLOS_ATTR_ID(port)

#define QSFP_PRESENT_ATTR_ID(port)		QSFP##port##_PRESENT
#define QSFP_RESET_ATTR_ID(port)		QSFP##port##_RESET
#define QSFP_LPMODE_ATTR_ID(port)       QSFP##port##_LPMODE

#define QSFP_ATTR(port) \
    QSFP_PRESENT_ATTR_ID(port),    \
    QSFP_RESET_ATTR_ID(port),      \
    QSFP_LPMODE_ATTR_ID(port)

enum as9947_36xkb_sfp_sysfs_attrs {
	SFP_ATTR(1),
    SFP_ATTR(2),
    SFP_ATTR(3),
    SFP_ATTR(4),
    NUM_OF_SFP_ATTR,
    NUM_OF_PER_SFP_ATTR = (NUM_OF_SFP_ATTR/NUM_OF_SFP),
};

enum as9947_36xkb_qsfp_sysfs_attrs {
    QSFP_ATTR(5),
    QSFP_ATTR(6),
    QSFP_ATTR(7),
    QSFP_ATTR(8),
    QSFP_ATTR(9),
    QSFP_ATTR(10),
    QSFP_ATTR(11),
    QSFP_ATTR(12),
    QSFP_ATTR(13),
    QSFP_ATTR(14),
    QSFP_ATTR(15),
    QSFP_ATTR(16),
    QSFP_ATTR(17),
    QSFP_ATTR(18),
    QSFP_ATTR(19),
    QSFP_ATTR(20),
    QSFP_ATTR(21),
    QSFP_ATTR(22),
    QSFP_ATTR(23),
    QSFP_ATTR(24),
    QSFP_ATTR(25),
    QSFP_ATTR(26),
    QSFP_ATTR(27),
    QSFP_ATTR(28),
    QSFP_ATTR(29),
    QSFP_ATTR(30),
    QSFP_ATTR(31),
    QSFP_ATTR(32),
    QSFP_ATTR(33),
    QSFP_ATTR(34),
    QSFP_ATTR(35),
    QSFP_ATTR(36),
    QSFP_ATTR(37),
    QSFP_ATTR(38),
    QSFP_ATTR(39),
    QSFP_ATTR(40),
    NUM_OF_QSFP_ATTR,
    NUM_OF_PER_QSFP_ATTR = (NUM_OF_QSFP_ATTR/NUM_OF_QSFP),
};

/* sfp attributes */
#define DECLARE_SFP_SENSOR_DEVICE_ATTR(port) \
	static SENSOR_DEVICE_ATTR(module_present_##port, S_IRUGO, show_sfp, NULL, SFP##port##_PRESENT); \
	static SENSOR_DEVICE_ATTR(module_tx_disable_##port, S_IWUSR | S_IRUGO, show_sfp, set_sfp_txdisable, SFP##port##_TXDISABLE); \
	static SENSOR_DEVICE_ATTR(module_tx_fault_##port, S_IRUGO, show_sfp, NULL, SFP##port##_TXFAULT); \
	static SENSOR_DEVICE_ATTR(module_rx_los_##port, S_IRUGO, show_sfp, NULL, SFP##port##_RXLOS)
#define DECLARE_SFP_ATTR(port) \
    &sensor_dev_attr_module_present_##port.dev_attr.attr, \
    &sensor_dev_attr_module_tx_disable_##port.dev_attr.attr, \
    &sensor_dev_attr_module_tx_fault_##port.dev_attr.attr, \
    &sensor_dev_attr_module_rx_los_##port.dev_attr.attr

/* qsfp attributes */
#define DECLARE_QSFP_SENSOR_DEVICE_ATTR(port) \
	static SENSOR_DEVICE_ATTR(module_present_##port, S_IRUGO, show_qsfp, NULL, QSFP##port##_PRESENT); \
	static SENSOR_DEVICE_ATTR(module_reset_##port, S_IWUSR | S_IRUGO, show_qsfp, set_qsfp_reset, QSFP##port##_RESET); \
	static SENSOR_DEVICE_ATTR(module_lpmode_##port, S_IWUSR | S_IRUGO, show_qsfp, set_qsfp_lpmode, QSFP##port##_LPMODE)
#define DECLARE_QSFP_ATTR(port) \
    &sensor_dev_attr_module_present_##port.dev_attr.attr, \
    &sensor_dev_attr_module_reset_##port.dev_attr.attr, \
    &sensor_dev_attr_module_lpmode_##port.dev_attr.attr

static SENSOR_DEVICE_ATTR(module_present_all, S_IRUGO, show_all, NULL, PRESENT_ALL);
static SENSOR_DEVICE_ATTR(module_rxlos_all, S_IRUGO, show_all, NULL, RXLOS_ALL);

DECLARE_SFP_SENSOR_DEVICE_ATTR(1);
DECLARE_SFP_SENSOR_DEVICE_ATTR(2);
DECLARE_SFP_SENSOR_DEVICE_ATTR(3);
DECLARE_SFP_SENSOR_DEVICE_ATTR(4);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(5);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(6);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(7);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(8);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(9);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(10);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(11);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(12);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(13);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(14);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(15);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(16);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(17);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(18);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(19);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(20);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(21);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(22);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(23);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(24);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(25);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(26);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(27);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(28);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(29);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(30);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(31);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(32);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(33);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(34);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(35);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(36);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(37);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(38);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(39);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(40);

static struct attribute *as9947_36xkb_sfp_attributes[] = {
    /* sfp attributes */
    DECLARE_SFP_ATTR(1),
    DECLARE_SFP_ATTR(2),
    DECLARE_SFP_ATTR(3),
    DECLARE_SFP_ATTR(4),
    DECLARE_QSFP_ATTR(5),
    DECLARE_QSFP_ATTR(6),
    DECLARE_QSFP_ATTR(7),
    DECLARE_QSFP_ATTR(8),
    DECLARE_QSFP_ATTR(9),
    DECLARE_QSFP_ATTR(10),
    DECLARE_QSFP_ATTR(11),
    DECLARE_QSFP_ATTR(12),
    DECLARE_QSFP_ATTR(13),
    DECLARE_QSFP_ATTR(14),
    DECLARE_QSFP_ATTR(15),
    DECLARE_QSFP_ATTR(16),
    DECLARE_QSFP_ATTR(17),
    DECLARE_QSFP_ATTR(18),
    DECLARE_QSFP_ATTR(19),
    DECLARE_QSFP_ATTR(20),
    DECLARE_QSFP_ATTR(21),
    DECLARE_QSFP_ATTR(22),
    DECLARE_QSFP_ATTR(23),
    DECLARE_QSFP_ATTR(24),
    DECLARE_QSFP_ATTR(25),
    DECLARE_QSFP_ATTR(26),
    DECLARE_QSFP_ATTR(27),
    DECLARE_QSFP_ATTR(28),
    DECLARE_QSFP_ATTR(29),
    DECLARE_QSFP_ATTR(30),
    DECLARE_QSFP_ATTR(31),
    DECLARE_QSFP_ATTR(32),
    DECLARE_QSFP_ATTR(33),
    DECLARE_QSFP_ATTR(34),
    DECLARE_QSFP_ATTR(35),
    DECLARE_QSFP_ATTR(36),
    DECLARE_QSFP_ATTR(37),
    DECLARE_QSFP_ATTR(38),
    DECLARE_QSFP_ATTR(39),
    DECLARE_QSFP_ATTR(40),
    &sensor_dev_attr_module_present_all.dev_attr.attr,
    &sensor_dev_attr_module_rxlos_all.dev_attr.attr,
    NULL
};

static const struct attribute_group as9947_36xkb_sfp_group = {
    .attrs = as9947_36xkb_sfp_attributes,
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
static int ipmi_send_message(struct ipmi_data *ipmi, unsigned char cmd,
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

static struct as9947_36xkb_sfp_data *as9947_36xkb_sfp_update_present(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.sfp_last_updated[SFP_PRESENT] + HZ) && 
        data->ipmi_resp.sfp_valid[SFP_PRESENT]) {
        return data;
    }

    data->ipmi_resp.sfp_valid[SFP_PRESENT] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = IPMI_SFP_PRESENT_CMD;
    status = ipmi_send_message(&data->ipmi, IPMI_SFP_READ_CMD, 
                                data->ipmi_tx_data, 1,
                                data->ipmi_resp.sfp_resp[SFP_PRESENT], 
                                sizeof(data->ipmi_resp.sfp_resp[SFP_PRESENT]));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->ipmi_resp.sfp_last_updated[SFP_PRESENT] = jiffies;
    data->ipmi_resp.sfp_valid[SFP_PRESENT] = 1;

exit:
    return data;
}

static struct as9947_36xkb_sfp_data *as9947_36xkb_sfp_update_txdisable(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.sfp_last_updated[SFP_TXDISABLE] + HZ * 5) && 
        data->ipmi_resp.sfp_valid[SFP_TXDISABLE]) {
        return data;
    }

    data->ipmi_resp.sfp_valid[SFP_TXDISABLE] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = IPMI_SFP_TXDIS_CMD;
    status = ipmi_send_message(&data->ipmi, IPMI_SFP_READ_CMD, 
                                data->ipmi_tx_data, 1,
                                data->ipmi_resp.sfp_resp[SFP_TXDISABLE], 
                                sizeof(data->ipmi_resp.sfp_resp[SFP_TXDISABLE]));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->ipmi_resp.sfp_last_updated[SFP_TXDISABLE] = jiffies;
    data->ipmi_resp.sfp_valid[SFP_TXDISABLE] = 1;

exit:
    return data;
}

static struct as9947_36xkb_sfp_data *as9947_36xkb_sfp_update_txfault(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.sfp_last_updated[SFP_TXFAULT] + HZ * 5) && 
        data->ipmi_resp.sfp_valid[SFP_TXFAULT]) {
        return data;
    }

    data->ipmi_resp.sfp_valid[SFP_TXFAULT] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = IPMI_SFP_TXFAULT_CMD;
    status = ipmi_send_message(&data->ipmi, IPMI_SFP_READ_CMD, 
                                data->ipmi_tx_data, 1,
                                data->ipmi_resp.sfp_resp[SFP_TXFAULT], 
                                sizeof(data->ipmi_resp.sfp_resp[SFP_TXFAULT]));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->ipmi_resp.sfp_last_updated[SFP_TXFAULT] = jiffies;
    data->ipmi_resp.sfp_valid[SFP_TXFAULT] = 1;

exit:
    return data;
}

static struct as9947_36xkb_sfp_data *as9947_36xkb_sfp_update_rxlos(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.sfp_last_updated[SFP_RXLOS] + HZ * 5) && 
        data->ipmi_resp.sfp_valid[SFP_RXLOS]) {
        return data;
    }

    data->ipmi_resp.sfp_valid[SFP_RXLOS] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = IPMI_SFP_RXLOS_CMD;
    status = ipmi_send_message(&data->ipmi, IPMI_SFP_READ_CMD, 
                                data->ipmi_tx_data, 1,
                                data->ipmi_resp.sfp_resp[SFP_RXLOS], 
                                sizeof(data->ipmi_resp.sfp_resp[SFP_RXLOS]));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->ipmi_resp.sfp_last_updated[SFP_RXLOS] = jiffies;
    data->ipmi_resp.sfp_valid[SFP_RXLOS] = 1;

exit:
    return data;
}

static struct as9947_36xkb_sfp_data *as9947_36xkb_qsfp_update_present(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.qsfp_last_updated[QSFP_PRESENT] + HZ) && 
        data->ipmi_resp.qsfp_valid[QSFP_PRESENT]) {
        return data;
    }

    data->ipmi_resp.qsfp_valid[QSFP_PRESENT] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = IPMI_QSFP_PRESENT_CMD;
    status = ipmi_send_message(&data->ipmi, IPMI_QSFP_READ_CMD, 
                                data->ipmi_tx_data, 1,
                                data->ipmi_resp.qsfp_resp[QSFP_PRESENT], 
                                sizeof(data->ipmi_resp.qsfp_resp[QSFP_PRESENT]));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->ipmi_resp.qsfp_last_updated[QSFP_PRESENT] = jiffies;
    data->ipmi_resp.qsfp_valid[QSFP_PRESENT] = 1;

exit:
    return data;
}

static struct as9947_36xkb_sfp_data *as9947_36xkb_qsfp_update_reset(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.qsfp_last_updated[QSFP_RESET] + HZ * 5) && 
        data->ipmi_resp.qsfp_valid[QSFP_RESET]) {
        return data;
    }

    data->ipmi_resp.qsfp_valid[QSFP_RESET] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = IPMI_QSFP_RESET_CMD;
    status = ipmi_send_message(&data->ipmi, IPMI_QSFP_READ_CMD, 
                                data->ipmi_tx_data, 1,
                                data->ipmi_resp.qsfp_resp[QSFP_RESET], 
                                sizeof(data->ipmi_resp.qsfp_resp[QSFP_RESET]));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->ipmi_resp.qsfp_last_updated[QSFP_RESET] = jiffies;
    data->ipmi_resp.qsfp_valid[QSFP_RESET] = 1;

exit:
    return data;
}

static struct as9947_36xkb_sfp_data *as9947_36xkb_qsfp_update_lpmode(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.qsfp_last_updated[QSFP_LPMODE] + HZ * 5) && 
        data->ipmi_resp.qsfp_valid[QSFP_LPMODE]) {
        return data;
    }

    data->ipmi_resp.qsfp_valid[QSFP_LPMODE] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = IPMI_QSFP_LPMODE_CMD;
    status = ipmi_send_message(&data->ipmi, IPMI_QSFP_READ_CMD, 
                                data->ipmi_tx_data, 1,
                                data->ipmi_resp.qsfp_resp[QSFP_LPMODE], 
                                sizeof(data->ipmi_resp.qsfp_resp[QSFP_LPMODE]));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->ipmi_resp.qsfp_last_updated[QSFP_LPMODE] = jiffies;
    data->ipmi_resp.qsfp_valid[QSFP_LPMODE] = 1;

exit:
    return data;
}

static ssize_t show_all(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    u64 values = 0;
    int i;

	switch (attr->index) {
		case PRESENT_ALL:
        {
            mutex_lock(&data->update_lock);
            
            data = as9947_36xkb_sfp_update_present();
            if (!data->ipmi_resp.sfp_valid[SFP_PRESENT]) {
                mutex_unlock(&data->update_lock);
                return -EIO;
            }

            data = as9947_36xkb_qsfp_update_present();
            if (!data->ipmi_resp.qsfp_valid[QSFP_PRESENT]) {
                mutex_unlock(&data->update_lock);
                return -EIO;
            }

            /* Update qsfp present status */
            for (i = (NUM_OF_QSFP-1); i >= 0; i--) {
                values <<= 1;
                values |= (data->ipmi_resp.qsfp_resp[QSFP_PRESENT][i] & 0x1);
            }

            /* Update sfp present status */
            for (i = (NUM_OF_SFP-1); i >= 0; i--) {
                values <<= 1;
                values |= (data->ipmi_resp.sfp_resp[SFP_PRESENT][i] & 0x1);
            }
            
            mutex_unlock(&data->update_lock);

            /* Return values 1 -> 54 in order */
            return sprintf(buf, "%.2x %.2x %.2x %.2x %.2x %.2x %.2x\n",
                           (unsigned int)(0xFF & values),
                           (unsigned int)(0xFF & (values >> 8)),
                           (unsigned int)(0xFF & (values >> 16)),
                           (unsigned int)(0xFF & (values >> 24)),
                           (unsigned int)(0xFF & (values >> 32)),
                           (unsigned int)(0xFF & (values >> 40)),
                           (unsigned int)(0x3F & (values >> 48)));
        }
		case RXLOS_ALL:
        {
            mutex_lock(&data->update_lock);

            data = as9947_36xkb_sfp_update_rxlos();
            if (!data->ipmi_resp.sfp_valid[SFP_RXLOS]) {
                mutex_unlock(&data->update_lock);
                return -EIO;
            }

            /* Update sfp rxlos status */
            for (i = (NUM_OF_SFP-1); i >= 0; i--) {
                values <<= 1;
                values |= (data->ipmi_resp.sfp_resp[SFP_RXLOS][i] & 0x1);
            }

            mutex_unlock(&data->update_lock);

            /* Return values 1 -> 48 in order */
            return sprintf(buf, "%.2x %.2x %.2x %.2x %.2x %.2x\n",
                           (unsigned int)(0xFF & values),
                           (unsigned int)(0xFF & (values >> 8)),
                           (unsigned int)(0xFF & (values >> 16)),
                           (unsigned int)(0xFF & (values >> 24)),
                           (unsigned int)(0xFF & (values >> 32)),
                           (unsigned int)(0xFF & (values >> 40)));
        }
        default:
        break;
    }

    return 0;
}

static ssize_t show_sfp(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_SFP_ATTR; /* port id, 0 based */
    int value = 0;
    int error = 0;

    mutex_lock(&data->update_lock);

	switch (attr->index) {
		case SFP1_PRESENT:
        case SFP2_PRESENT:
		case SFP3_PRESENT:
        case SFP4_PRESENT:
        {
            data = as9947_36xkb_sfp_update_present();
            if (!data->ipmi_resp.sfp_valid[SFP_PRESENT]) {
                error = -EIO;
                goto exit;
            }

            value = data->ipmi_resp.sfp_resp[SFP_PRESENT][pid];
			break;
        }
		case SFP1_TXDISABLE:
        case SFP2_TXDISABLE:
		case SFP3_TXDISABLE:
        case SFP4_TXDISABLE:
        {
            data = as9947_36xkb_sfp_update_txdisable();
            if (!data->ipmi_resp.sfp_valid[SFP_TXDISABLE]) {
                error = -EIO;
                goto exit;
            }

            value = !data->ipmi_resp.sfp_resp[SFP_TXDISABLE][pid];
            break;
        }
		case SFP1_TXFAULT:
        case SFP2_TXFAULT:
		case SFP3_TXFAULT:
        case SFP4_TXFAULT:
        {
            data = as9947_36xkb_sfp_update_txfault();
            if (!data->ipmi_resp.sfp_valid[SFP_TXFAULT]) {
                error = -EIO;
                goto exit;
            }

            value = data->ipmi_resp.sfp_resp[SFP_TXFAULT][pid];
            break;
        }
		case SFP1_RXLOS:
        case SFP2_RXLOS:
		case SFP3_RXLOS:
        case SFP4_RXLOS:
        {
            data = as9947_36xkb_sfp_update_rxlos();
            if (!data->ipmi_resp.sfp_valid[SFP_RXLOS]) {
                error = -EIO;
                goto exit;
            }

            value = data->ipmi_resp.sfp_resp[SFP_RXLOS][pid];
            break;
        }
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

static ssize_t set_sfp_txdisable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	long disable;
	int status;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_SFP_ATTR; /* port id, 0 based */

	status = kstrtol(buf, 10, &disable);
	if (status) {
		return status;
	}

    disable = !disable; /* the IPMI cmd is 0 for tx-disable and 1 for tx-enable */

    mutex_lock(&data->update_lock);

    /* Send IPMI write command */
    data->ipmi_tx_data[0] = pid + 1; /* Port ID base id for ipmi start from 1 */
    data->ipmi_tx_data[1] = IPMI_SFP_TXDIS_CMD;
    data->ipmi_tx_data[2] = disable;
    status = ipmi_send_message(&data->ipmi, IPMI_SFP_WRITE_CMD,
                                data->ipmi_tx_data, sizeof(data->ipmi_tx_data), NULL, 0);
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    /* Update to ipmi_resp buffer to prevent from the impact of lazy update */
    data->ipmi_resp.sfp_resp[SFP_TXDISABLE][pid] = disable;
    status = count;

exit:
    mutex_unlock(&data->update_lock);
    return status;
}

static ssize_t show_qsfp(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_QSFP_ATTR; /* port id, 0 based */
    int value = 0;
    int error = 0;

    mutex_lock(&data->update_lock);

	switch (attr->index) {
        case QSFP5_PRESENT:
        case QSFP6_PRESENT:
        case QSFP7_PRESENT:
        case QSFP8_PRESENT:
        case QSFP9_PRESENT:
        case QSFP10_PRESENT:
        case QSFP11_PRESENT:
        case QSFP12_PRESENT:
        case QSFP13_PRESENT:
        case QSFP14_PRESENT:
        case QSFP15_PRESENT:
        case QSFP16_PRESENT:
        case QSFP17_PRESENT:
        case QSFP18_PRESENT:
        case QSFP19_PRESENT:
        case QSFP20_PRESENT:
        case QSFP21_PRESENT:
        case QSFP22_PRESENT:
        case QSFP23_PRESENT:
        case QSFP24_PRESENT:
        case QSFP25_PRESENT:
        case QSFP26_PRESENT:
        case QSFP27_PRESENT:
        case QSFP28_PRESENT:
		case QSFP29_PRESENT:
        case QSFP30_PRESENT:
		case QSFP31_PRESENT:
        case QSFP32_PRESENT:
		case QSFP33_PRESENT:
        case QSFP34_PRESENT:
        case QSFP35_PRESENT:
        case QSFP36_PRESENT:
        case QSFP37_PRESENT:
        case QSFP38_PRESENT:
        case QSFP39_PRESENT:
        case QSFP40_PRESENT:
        {
            data = as9947_36xkb_qsfp_update_present();
            if (!data->ipmi_resp.qsfp_valid[QSFP_PRESENT]) {
                error = -EIO;
                goto exit;
            }

            value = data->ipmi_resp.qsfp_resp[QSFP_PRESENT][pid];
			break;
        }
		case QSFP5_RESET:
		case QSFP6_RESET:
		case QSFP7_RESET:
		case QSFP8_RESET:
		case QSFP9_RESET:
		case QSFP10_RESET:
        case QSFP11_RESET:
        case QSFP12_RESET:
        case QSFP13_RESET:
        case QSFP14_RESET:
        case QSFP15_RESET:
        case QSFP16_RESET:
        case QSFP17_RESET:
        case QSFP18_RESET:
        case QSFP19_RESET:
        case QSFP20_RESET:
        case QSFP21_RESET:
        case QSFP22_RESET:
        case QSFP23_RESET:
        case QSFP24_RESET:
        case QSFP25_RESET:
        case QSFP26_RESET:
        case QSFP27_RESET:
        case QSFP28_RESET:
        case QSFP29_RESET:
        case QSFP30_RESET:
        case QSFP31_RESET:
        case QSFP32_RESET:
        case QSFP33_RESET:
        case QSFP34_RESET:
        case QSFP35_RESET:
        case QSFP36_RESET:
        case QSFP37_RESET:
        case QSFP38_RESET:
        case QSFP39_RESET:
        case QSFP40_RESET:
        {
            data = as9947_36xkb_qsfp_update_reset();
            if (!data->ipmi_resp.qsfp_valid[QSFP_RESET]) {
                error = -EIO;
                goto exit;
            }

            value = !data->ipmi_resp.qsfp_resp[QSFP_RESET][pid];
			break;
        }
		case QSFP5_LPMODE:
		case QSFP6_LPMODE:
		case QSFP7_LPMODE:
		case QSFP8_LPMODE:
		case QSFP9_LPMODE:
		case QSFP10_LPMODE:
        case QSFP11_LPMODE:
        case QSFP12_LPMODE:
        case QSFP13_LPMODE:
        case QSFP14_LPMODE:
        case QSFP15_LPMODE:
        case QSFP16_LPMODE:
        case QSFP17_LPMODE:
        case QSFP18_LPMODE:
        case QSFP19_LPMODE:
        case QSFP20_LPMODE:
        case QSFP21_LPMODE:
        case QSFP22_LPMODE:
        case QSFP23_LPMODE:
        case QSFP24_LPMODE:
        case QSFP25_LPMODE:
        case QSFP26_LPMODE:
        case QSFP27_LPMODE:
        case QSFP28_LPMODE:
        case QSFP29_LPMODE:
        case QSFP30_LPMODE:
        case QSFP31_LPMODE:
        case QSFP32_LPMODE:
        case QSFP33_LPMODE:
        case QSFP34_LPMODE:
        case QSFP35_LPMODE:
        case QSFP36_LPMODE:
        case QSFP37_LPMODE:
        case QSFP38_LPMODE:
        case QSFP39_LPMODE:
        case QSFP40_LPMODE:
        {
            data = as9947_36xkb_qsfp_update_lpmode();
            if (!data->ipmi_resp.qsfp_valid[QSFP_LPMODE]) {
                error = -EIO;
                goto exit;
            }

            value = data->ipmi_resp.qsfp_resp[QSFP_LPMODE][pid];
			break;
        }        
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

static ssize_t set_qsfp_reset(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	long reset;
	int status;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_QSFP_ATTR; /* port id, 0 based */
    
	status = kstrtol(buf, 10, &reset);
	if (status) {
		return status;
	}

    reset = !reset; /* the IPMI cmd is 0 for reset and 1 for out of reset */

    mutex_lock(&data->update_lock);

    /* Send IPMI write command */
    data->ipmi_tx_data[0] = pid + 1; /* Port ID base id for ipmi start from 1 */
    data->ipmi_tx_data[1] = IPMI_QSFP_RESET_CMD;
    data->ipmi_tx_data[2] = reset;
    status = ipmi_send_message(&data->ipmi, IPMI_QSFP_WRITE_CMD,
                                data->ipmi_tx_data, sizeof(data->ipmi_tx_data), NULL, 0);
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    /* Update to ipmi_resp buffer to prevent from the impact of lazy update */
    data->ipmi_resp.qsfp_resp[QSFP_RESET][pid] = reset;
    status = count;
    
exit:
    mutex_unlock(&data->update_lock);
    return status;
}

static ssize_t set_qsfp_lpmode(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	long lpmode;
	int status;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_QSFP_ATTR; /* port id, 0 based */
    
	status = kstrtol(buf, 10, &lpmode);
	if (status) {
		return status;
	}

    mutex_lock(&data->update_lock);

    /* Send IPMI write command */
    data->ipmi_tx_data[0] = pid + 1; /* Port ID base id for ipmi start from 1 */
    data->ipmi_tx_data[1] = IPMI_QSFP_LPMODE_CMD;
    data->ipmi_tx_data[2] = lpmode;  /* 0: High Power Mode, 1: Low Power Mode */
    status = ipmi_send_message(&data->ipmi, IPMI_QSFP_WRITE_CMD,
                                data->ipmi_tx_data, sizeof(data->ipmi_tx_data), NULL, 0);
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    /* Update to ipmi_resp buffer to prevent from the impact of lazy update */
    data->ipmi_resp.qsfp_resp[QSFP_LPMODE][pid] = lpmode;
    status = count;
    
exit:
    mutex_unlock(&data->update_lock);
    return status;
}

static int as9947_36xkb_sfp_probe(struct platform_device *pdev)
{
    int status = -1;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &as9947_36xkb_sfp_group);
    if (status)
        goto exit;

    dev_info(&pdev->dev, "device created\n");

    return 0;

exit:
    return status;
}

static int as9947_36xkb_sfp_remove(struct platform_device *pdev)
{
    sysfs_remove_group(&pdev->dev.kobj, &as9947_36xkb_sfp_group);

    return 0;
}

static int __init as9947_36xkb_sfp_init(void)
{
    int ret;

    data = kzalloc(sizeof(struct as9947_36xkb_sfp_data), GFP_KERNEL);
    if (!data) {
        ret = -ENOMEM;
        goto alloc_err;
    }

	mutex_init(&data->update_lock);

    ret = platform_driver_register(&as9947_36xkb_sfp_driver);
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
    platform_driver_unregister(&as9947_36xkb_sfp_driver);
dri_reg_err:
    kfree(data);
alloc_err:
    return ret;
}

static void __exit as9947_36xkb_sfp_exit(void)
{
    ipmi_destroy_user(data->ipmi.user);
    platform_device_unregister(data->pdev);
    platform_driver_unregister(&as9947_36xkb_sfp_driver);
    kfree(data);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("AS9947-36XKB sfp driver");
MODULE_LICENSE("GPL");

module_init(as9947_36xkb_sfp_init);
module_exit(as9947_36xkb_sfp_exit);

