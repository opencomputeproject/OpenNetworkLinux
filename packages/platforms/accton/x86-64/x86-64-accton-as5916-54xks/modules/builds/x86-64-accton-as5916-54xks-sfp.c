/*
 * Copyright (C)  Brandon Chuang <brandon_chuang@accton.com.tw>
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

#define DRVNAME "as5916_54xks_sfp"
#define ACCTON_IPMI_NETFN       0x34
#define IPMI_QSFP_READ_CMD      0x10
#define IPMI_QSFP_WRITE_CMD     0x11
#define IPMI_SFP_READ_CMD       0x1C
#define IPMI_SFP_WRITE_CMD      0x1D
#define IPMI_TIMEOUT		(20 * HZ)
#define IPMI_DATA_MAX_LEN       128

#define SFP_EEPROM_SIZE         768
#define QSFP_EEPROM_SIZE        640

#define NUM_OF_SFP              48
#define NUM_OF_QSFP             6
#define NUM_OF_PORT             (NUM_OF_SFP + NUM_OF_QSFP)

static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static ssize_t set_sfp(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t show_sfp(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t set_qsfp_txdisable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t set_qsfp_reset(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t set_qsfp_lpmode(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t show_qsfp(struct device *dev, struct device_attribute *da, char *buf);
static int as5916_54xks_sfp_probe(struct platform_device *pdev);
static int as5916_54xks_sfp_remove(struct platform_device *pdev);
static ssize_t show_all(struct device *dev, struct device_attribute *da, char *buf);
static struct as5916_54xks_sfp_data *as5916_54xks_sfp_update_present(void);
static struct as5916_54xks_sfp_data *as5916_54xks_sfp_update_txdisable(void);
static struct as5916_54xks_sfp_data *as5916_54xks_sfp_update_txfault(void);
static struct as5916_54xks_sfp_data *as5916_54xks_sfp_update_rxlos(void);
static struct as5916_54xks_sfp_data *as5916_54xks_qsfp_update_present(void);
static struct as5916_54xks_sfp_data *as5916_54xks_qsfp_update_txdisable(void);
static struct as5916_54xks_sfp_data *as5916_54xks_qsfp_update_reset(void);

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

enum module_status {
    SFP_PRESENT = 0,
    SFP_TXDISABLE,
    SFP_TXFAULT,
    SFP_RXLOS,
    NUM_OF_SFP_STATUS,

    QSFP_PRESENT = 0,
    QSFP_TXDISABLE,
    QSFP_RESET,
    QSFP_LPMODE,
    NUM_OF_QSFP_STATUS,

    PRESENT_ALL = 0,
    RXLOS_ALL
};

struct ipmi_sfp_resp_data {
    unsigned char eeprom[IPMI_DATA_MAX_LEN];
    char          eeprom_valid;

    char          sfp_valid[NUM_OF_SFP_STATUS];        /* != 0 if registers are valid */
    unsigned long sfp_last_updated[NUM_OF_SFP_STATUS]; /* In jiffies */
    unsigned char sfp_resp[NUM_OF_SFP_STATUS][NUM_OF_SFP]; /* 0: present,  1: tx-disable
                                                                 2: tx-fault, 3: rx-los  */
    char          qsfp_valid[NUM_OF_QSFP_STATUS];       /* != 0 if registers are valid */
    unsigned long qsfp_last_updated[NUM_OF_QSFP_STATUS]; /* In jiffies */
    unsigned char qsfp_resp[NUM_OF_QSFP_STATUS][NUM_OF_QSFP]; /* 0: present, 1: tx-disable, 
                                                                 2: reset  , 3: low power mode */
};

struct as5916_54xks_sfp_data {
    struct platform_device *pdev;
    struct mutex     update_lock;
    struct ipmi_data ipmi;
    struct ipmi_sfp_resp_data ipmi_resp;
    unsigned char ipmi_tx_data[3];
    struct bin_attribute eeprom[NUM_OF_PORT]; /* eeprom data */
};

struct sfp_eeprom_write_data {
    unsigned char ipmi_tx_data[4]; /* 0:port index  1:page number 2:offset 3:Data len */
    unsigned char write_buf[IPMI_DATA_MAX_LEN];
};

struct as5916_54xks_sfp_data *data = NULL;

static struct platform_driver as5916_54xks_sfp_driver = {
    .probe      = as5916_54xks_sfp_probe,
    .remove     = as5916_54xks_sfp_remove,
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
#define QSFP_TXDISABLE_ATTR_ID(port)	QSFP##port##_TXDISABLE
#define QSFP_RESET_ATTR_ID(port)		QSFP##port##_RESET
#define QSFP_LPMODE_ATTR_ID(port)       QSFP##port##_LPMODE

#define QSFP_ATTR(port) \
    QSFP_PRESENT_ATTR_ID(port),    \
    QSFP_TXDISABLE_ATTR_ID(port),  \
    QSFP_RESET_ATTR_ID(port),      \
    QSFP_LPMODE_ATTR_ID(port)

enum as5916_54x_sfp_sysfs_attrs {
	SFP_ATTR(1),
    SFP_ATTR(2),
    SFP_ATTR(3),
    SFP_ATTR(4),
    SFP_ATTR(5),
	SFP_ATTR(6),
    SFP_ATTR(7),
    SFP_ATTR(8),
    SFP_ATTR(9),
    SFP_ATTR(10),
	SFP_ATTR(11),
    SFP_ATTR(12),
    SFP_ATTR(13),
    SFP_ATTR(14),
    SFP_ATTR(15),
	SFP_ATTR(16),
    SFP_ATTR(17),
    SFP_ATTR(18),
    SFP_ATTR(19),
    SFP_ATTR(20),
	SFP_ATTR(21),
    SFP_ATTR(22),
    SFP_ATTR(23),
    SFP_ATTR(24),
    SFP_ATTR(25),
	SFP_ATTR(26),
    SFP_ATTR(27),
    SFP_ATTR(28),
    SFP_ATTR(29),
    SFP_ATTR(30),
	SFP_ATTR(31),
    SFP_ATTR(32),
    SFP_ATTR(33),
    SFP_ATTR(34),
    SFP_ATTR(35),
	SFP_ATTR(36),
    SFP_ATTR(37),
    SFP_ATTR(38),
    SFP_ATTR(39),
    SFP_ATTR(40),
	SFP_ATTR(41),
    SFP_ATTR(42),
    SFP_ATTR(43),
    SFP_ATTR(44),
    SFP_ATTR(45),
	SFP_ATTR(46),
    SFP_ATTR(47),
    SFP_ATTR(48),
    NUM_OF_SFP_ATTR,
    NUM_OF_PER_SFP_ATTR = (NUM_OF_SFP_ATTR/NUM_OF_SFP),
};

enum as5916_54xks_qsfp_sysfs_attrs {
    QSFP_ATTR(49),
    QSFP_ATTR(50),
	QSFP_ATTR(51),
    QSFP_ATTR(52),
    QSFP_ATTR(53),
    QSFP_ATTR(54),
    NUM_OF_QSFP_ATTR,
    NUM_OF_PER_QSFP_ATTR = (NUM_OF_QSFP_ATTR/NUM_OF_QSFP),
};

/* sfp attributes */
#define DECLARE_SFP_SENSOR_DEVICE_ATTR(port) \
	static SENSOR_DEVICE_ATTR(module_present_##port, S_IRUGO, show_sfp, NULL, SFP##port##_PRESENT); \
	static SENSOR_DEVICE_ATTR(module_tx_disable_##port, S_IWUSR | S_IRUGO, show_sfp, set_sfp, SFP##port##_TXDISABLE); \
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
	static SENSOR_DEVICE_ATTR(module_tx_disable_##port, S_IWUSR | S_IRUGO, show_qsfp, set_qsfp_txdisable, QSFP##port##_TXDISABLE); \
	static SENSOR_DEVICE_ATTR(module_reset_##port, S_IWUSR | S_IRUGO, show_qsfp, set_qsfp_reset, QSFP##port##_RESET); \
	static SENSOR_DEVICE_ATTR(module_lpmode_##port, S_IWUSR | S_IRUGO, show_qsfp, set_qsfp_lpmode, QSFP##port##_LPMODE)
#define DECLARE_QSFP_ATTR(port) \
    &sensor_dev_attr_module_present_##port.dev_attr.attr, \
    &sensor_dev_attr_module_tx_disable_##port.dev_attr.attr, \
    &sensor_dev_attr_module_reset_##port.dev_attr.attr, \
    &sensor_dev_attr_module_lpmode_##port.dev_attr.attr

static SENSOR_DEVICE_ATTR(module_present_all, S_IRUGO, show_all, NULL, PRESENT_ALL); \
static SENSOR_DEVICE_ATTR(module_rxlos_all, S_IRUGO, show_all, NULL, RXLOS_ALL); \

DECLARE_SFP_SENSOR_DEVICE_ATTR(1);
DECLARE_SFP_SENSOR_DEVICE_ATTR(2);
DECLARE_SFP_SENSOR_DEVICE_ATTR(3);
DECLARE_SFP_SENSOR_DEVICE_ATTR(4);
DECLARE_SFP_SENSOR_DEVICE_ATTR(5);
DECLARE_SFP_SENSOR_DEVICE_ATTR(6);
DECLARE_SFP_SENSOR_DEVICE_ATTR(7);
DECLARE_SFP_SENSOR_DEVICE_ATTR(8);
DECLARE_SFP_SENSOR_DEVICE_ATTR(9);
DECLARE_SFP_SENSOR_DEVICE_ATTR(10);
DECLARE_SFP_SENSOR_DEVICE_ATTR(11);
DECLARE_SFP_SENSOR_DEVICE_ATTR(12);
DECLARE_SFP_SENSOR_DEVICE_ATTR(13);
DECLARE_SFP_SENSOR_DEVICE_ATTR(14);
DECLARE_SFP_SENSOR_DEVICE_ATTR(15);
DECLARE_SFP_SENSOR_DEVICE_ATTR(16);
DECLARE_SFP_SENSOR_DEVICE_ATTR(17);
DECLARE_SFP_SENSOR_DEVICE_ATTR(18);
DECLARE_SFP_SENSOR_DEVICE_ATTR(19);
DECLARE_SFP_SENSOR_DEVICE_ATTR(20);
DECLARE_SFP_SENSOR_DEVICE_ATTR(21);
DECLARE_SFP_SENSOR_DEVICE_ATTR(22);
DECLARE_SFP_SENSOR_DEVICE_ATTR(23);
DECLARE_SFP_SENSOR_DEVICE_ATTR(24);
DECLARE_SFP_SENSOR_DEVICE_ATTR(25);
DECLARE_SFP_SENSOR_DEVICE_ATTR(26);
DECLARE_SFP_SENSOR_DEVICE_ATTR(27);
DECLARE_SFP_SENSOR_DEVICE_ATTR(28);
DECLARE_SFP_SENSOR_DEVICE_ATTR(29);
DECLARE_SFP_SENSOR_DEVICE_ATTR(30);
DECLARE_SFP_SENSOR_DEVICE_ATTR(31);
DECLARE_SFP_SENSOR_DEVICE_ATTR(32);
DECLARE_SFP_SENSOR_DEVICE_ATTR(33);
DECLARE_SFP_SENSOR_DEVICE_ATTR(34);
DECLARE_SFP_SENSOR_DEVICE_ATTR(35);
DECLARE_SFP_SENSOR_DEVICE_ATTR(36);
DECLARE_SFP_SENSOR_DEVICE_ATTR(37);
DECLARE_SFP_SENSOR_DEVICE_ATTR(38);
DECLARE_SFP_SENSOR_DEVICE_ATTR(39);
DECLARE_SFP_SENSOR_DEVICE_ATTR(40);
DECLARE_SFP_SENSOR_DEVICE_ATTR(41);
DECLARE_SFP_SENSOR_DEVICE_ATTR(42);
DECLARE_SFP_SENSOR_DEVICE_ATTR(43);
DECLARE_SFP_SENSOR_DEVICE_ATTR(44);
DECLARE_SFP_SENSOR_DEVICE_ATTR(45);
DECLARE_SFP_SENSOR_DEVICE_ATTR(46);
DECLARE_SFP_SENSOR_DEVICE_ATTR(47);
DECLARE_SFP_SENSOR_DEVICE_ATTR(48);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(49);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(50);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(51);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(52);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(53);
DECLARE_QSFP_SENSOR_DEVICE_ATTR(54);

static struct attribute *as5916_54xks_sfp_attributes[] = {
    /* sfp attributes */
    DECLARE_SFP_ATTR(1),
    DECLARE_SFP_ATTR(2),
    DECLARE_SFP_ATTR(3),
    DECLARE_SFP_ATTR(4),
    DECLARE_SFP_ATTR(5),
    DECLARE_SFP_ATTR(6),
    DECLARE_SFP_ATTR(7),
    DECLARE_SFP_ATTR(8),
    DECLARE_SFP_ATTR(9),
    DECLARE_SFP_ATTR(10),
    DECLARE_SFP_ATTR(11),
    DECLARE_SFP_ATTR(12),
    DECLARE_SFP_ATTR(13),
    DECLARE_SFP_ATTR(14),
    DECLARE_SFP_ATTR(15),
    DECLARE_SFP_ATTR(16),
    DECLARE_SFP_ATTR(17),
    DECLARE_SFP_ATTR(18),
    DECLARE_SFP_ATTR(19),
    DECLARE_SFP_ATTR(20),
    DECLARE_SFP_ATTR(21),
    DECLARE_SFP_ATTR(22),
    DECLARE_SFP_ATTR(23),
    DECLARE_SFP_ATTR(24),
    DECLARE_SFP_ATTR(25),
    DECLARE_SFP_ATTR(26),
    DECLARE_SFP_ATTR(27),
    DECLARE_SFP_ATTR(28),
    DECLARE_SFP_ATTR(29),
    DECLARE_SFP_ATTR(30),
    DECLARE_SFP_ATTR(31),
    DECLARE_SFP_ATTR(32),
    DECLARE_SFP_ATTR(33),
    DECLARE_SFP_ATTR(34),
    DECLARE_SFP_ATTR(35),
    DECLARE_SFP_ATTR(36),
    DECLARE_SFP_ATTR(37),
    DECLARE_SFP_ATTR(38),
    DECLARE_SFP_ATTR(39),
    DECLARE_SFP_ATTR(40),
    DECLARE_SFP_ATTR(41),
    DECLARE_SFP_ATTR(42),
    DECLARE_SFP_ATTR(43),
    DECLARE_SFP_ATTR(44),
    DECLARE_SFP_ATTR(45),
    DECLARE_SFP_ATTR(46),
    DECLARE_SFP_ATTR(47),
    DECLARE_SFP_ATTR(48),
    DECLARE_QSFP_ATTR(49),
    DECLARE_QSFP_ATTR(50),
    DECLARE_QSFP_ATTR(51),
    DECLARE_QSFP_ATTR(52),
    DECLARE_QSFP_ATTR(53),
    DECLARE_QSFP_ATTR(54),
    &sensor_dev_attr_module_present_all.dev_attr.attr,
    &sensor_dev_attr_module_rxlos_all.dev_attr.attr,
    NULL
};

static const struct attribute_group as5916_54xks_sfp_group = {
    .attrs = as5916_54xks_sfp_attributes,
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

static struct as5916_54xks_sfp_data *as5916_54xks_sfp_update_present(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.sfp_last_updated[SFP_PRESENT] + HZ) && 
        data->ipmi_resp.sfp_valid[SFP_PRESENT]) {
        return data;
    }

    mutex_lock(&data->update_lock);

    data->ipmi_resp.sfp_valid[SFP_PRESENT] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = 0x10;
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
    mutex_unlock(&data->update_lock);
    return data;
}

static struct as5916_54xks_sfp_data *as5916_54xks_sfp_update_txdisable(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.sfp_last_updated[SFP_TXDISABLE] + HZ * 5) && 
        data->ipmi_resp.sfp_valid[SFP_TXDISABLE]) {
        return data;
    }

    mutex_lock(&data->update_lock);

    data->ipmi_resp.sfp_valid[SFP_TXDISABLE] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = 0x01;
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
    mutex_unlock(&data->update_lock);
    return data;
}

static struct as5916_54xks_sfp_data *as5916_54xks_sfp_update_txfault(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.sfp_last_updated[SFP_TXFAULT] + HZ * 5) && 
        data->ipmi_resp.sfp_valid[SFP_TXFAULT]) {
        return data;
    }

    mutex_lock(&data->update_lock);

    data->ipmi_resp.sfp_valid[SFP_TXFAULT] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = 0x12;
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
    mutex_unlock(&data->update_lock);
    return data;
}

static struct as5916_54xks_sfp_data *as5916_54xks_sfp_update_rxlos(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.sfp_last_updated[SFP_RXLOS] + HZ * 5) && 
        data->ipmi_resp.sfp_valid[SFP_RXLOS]) {
        return data;
    }

    mutex_lock(&data->update_lock);

    data->ipmi_resp.sfp_valid[SFP_RXLOS] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = 0x13;
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
    mutex_unlock(&data->update_lock);
    return data;
}

static struct as5916_54xks_sfp_data *as5916_54xks_qsfp_update_present(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.qsfp_last_updated[QSFP_PRESENT] + HZ) && 
        data->ipmi_resp.qsfp_valid[QSFP_PRESENT]) {
        return data;
    }

    mutex_lock(&data->update_lock);

    data->ipmi_resp.qsfp_valid[QSFP_PRESENT] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = 0x10;
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
    mutex_unlock(&data->update_lock);
    return data;
}

static struct as5916_54xks_sfp_data *as5916_54xks_qsfp_update_txdisable(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.qsfp_last_updated[QSFP_TXDISABLE] + HZ * 5) && 
        data->ipmi_resp.qsfp_valid[QSFP_TXDISABLE]) {
        return data;
    }

    mutex_lock(&data->update_lock);

    data->ipmi_resp.qsfp_valid[QSFP_TXDISABLE] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = 0x01;
    status = ipmi_send_message(&data->ipmi, IPMI_QSFP_READ_CMD, 
                                data->ipmi_tx_data, 1,
                                data->ipmi_resp.qsfp_resp[QSFP_TXDISABLE], 
                                sizeof(data->ipmi_resp.qsfp_resp[QSFP_TXDISABLE]));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->ipmi_resp.qsfp_last_updated[QSFP_TXDISABLE] = jiffies;
    data->ipmi_resp.qsfp_valid[QSFP_TXDISABLE] = 1;

exit:
    mutex_unlock(&data->update_lock);
    return data;
}

static struct as5916_54xks_sfp_data *as5916_54xks_qsfp_update_reset(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.qsfp_last_updated[QSFP_RESET] + HZ * 5) && 
        data->ipmi_resp.qsfp_valid[QSFP_RESET]) {
        return data;
    }

    mutex_lock(&data->update_lock);

    data->ipmi_resp.qsfp_valid[QSFP_RESET] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = 0x11;
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
    mutex_unlock(&data->update_lock);
    return data;
}

static struct as5916_54xks_sfp_data *as5916_54xks_qsfp_update_lpmode(void)
{
    int status = 0;

    if (time_before(jiffies, data->ipmi_resp.qsfp_last_updated[QSFP_LPMODE] + HZ * 5) && 
        data->ipmi_resp.qsfp_valid[QSFP_LPMODE]) {
        return data;
    }

    mutex_lock(&data->update_lock);

    data->ipmi_resp.qsfp_valid[QSFP_LPMODE] = 0;

    /* Get status from ipmi */
    data->ipmi_tx_data[0] = 0x12;
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
    mutex_unlock(&data->update_lock);
    return data;
}

static ssize_t show_all(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct as5916_54xks_sfp_data *data = NULL;
    u64 values = 0;
    int i;

	switch (attr->index) {
		case PRESENT_ALL:
        {
            data = as5916_54xks_sfp_update_present();
            if (!data->ipmi_resp.sfp_valid[SFP_PRESENT]) {
                return -EIO;
            }

            data = as5916_54xks_qsfp_update_present();
            if (!data->ipmi_resp.qsfp_valid[QSFP_PRESENT]) {
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
            data = as5916_54xks_sfp_update_rxlos();
            if (!data->ipmi_resp.sfp_valid[SFP_RXLOS]) {
                return -EIO;
            }

            /* Update sfp rxlos status */
            for (i = (NUM_OF_SFP-1); i >= 0; i--) {
                values <<= 1;
                values |= !(data->ipmi_resp.sfp_resp[SFP_RXLOS][i] & 0x1);
            }

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
    struct as5916_54xks_sfp_data *data = NULL;
    int value = 0;

	switch (attr->index) {
		case SFP1_PRESENT:
        case SFP2_PRESENT:
		case SFP3_PRESENT:
        case SFP4_PRESENT:
		case SFP5_PRESENT:
        case SFP6_PRESENT:
		case SFP7_PRESENT:
        case SFP8_PRESENT:
		case SFP9_PRESENT:
        case SFP10_PRESENT:
		case SFP11_PRESENT:
        case SFP12_PRESENT:
		case SFP13_PRESENT:
        case SFP14_PRESENT:
		case SFP15_PRESENT:
        case SFP16_PRESENT:
		case SFP17_PRESENT:
        case SFP18_PRESENT:
		case SFP19_PRESENT:
        case SFP20_PRESENT:
		case SFP21_PRESENT:
        case SFP22_PRESENT:
		case SFP23_PRESENT:
        case SFP24_PRESENT:
		case SFP25_PRESENT:
        case SFP26_PRESENT:
		case SFP27_PRESENT:
        case SFP28_PRESENT:
		case SFP29_PRESENT:
        case SFP30_PRESENT:
		case SFP31_PRESENT:
        case SFP32_PRESENT:
		case SFP33_PRESENT:
        case SFP34_PRESENT:
		case SFP35_PRESENT:
        case SFP36_PRESENT:
		case SFP37_PRESENT:
        case SFP38_PRESENT:
		case SFP39_PRESENT:
        case SFP40_PRESENT:
		case SFP41_PRESENT:
        case SFP42_PRESENT:
		case SFP43_PRESENT:
        case SFP44_PRESENT:
		case SFP45_PRESENT:
        case SFP46_PRESENT:
		case SFP47_PRESENT:
        case SFP48_PRESENT:
        {
            data = as5916_54xks_sfp_update_present();
            if (!data->ipmi_resp.sfp_valid[SFP_PRESENT]) {
                return -EIO;
            }

            value = data->ipmi_resp.sfp_resp[SFP_PRESENT][pid];
			break;
        }
		case SFP1_TXDISABLE:
        case SFP2_TXDISABLE:
		case SFP3_TXDISABLE:
        case SFP4_TXDISABLE:
		case SFP5_TXDISABLE:
        case SFP6_TXDISABLE:
		case SFP7_TXDISABLE:
        case SFP8_TXDISABLE:
		case SFP9_TXDISABLE:
        case SFP10_TXDISABLE:
		case SFP11_TXDISABLE:
        case SFP12_TXDISABLE:
		case SFP13_TXDISABLE:
        case SFP14_TXDISABLE:
		case SFP15_TXDISABLE:
        case SFP16_TXDISABLE:
		case SFP17_TXDISABLE:
        case SFP18_TXDISABLE:
		case SFP19_TXDISABLE:
        case SFP20_TXDISABLE:
		case SFP21_TXDISABLE:
        case SFP22_TXDISABLE:
		case SFP23_TXDISABLE:
        case SFP24_TXDISABLE:
		case SFP25_TXDISABLE:
        case SFP26_TXDISABLE:
		case SFP27_TXDISABLE:
        case SFP28_TXDISABLE:
		case SFP29_TXDISABLE:
        case SFP30_TXDISABLE:
		case SFP31_TXDISABLE:
        case SFP32_TXDISABLE:
		case SFP33_TXDISABLE:
        case SFP34_TXDISABLE:
		case SFP35_TXDISABLE:
        case SFP36_TXDISABLE:
		case SFP37_TXDISABLE:
        case SFP38_TXDISABLE:
		case SFP39_TXDISABLE:
        case SFP40_TXDISABLE:
		case SFP41_TXDISABLE:
        case SFP42_TXDISABLE:
		case SFP43_TXDISABLE:
        case SFP44_TXDISABLE:
		case SFP45_TXDISABLE:
        case SFP46_TXDISABLE:
		case SFP47_TXDISABLE:
        case SFP48_TXDISABLE:
        {
            data = as5916_54xks_sfp_update_txdisable();
            if (!data->ipmi_resp.sfp_valid[SFP_TXDISABLE]) {
                return -EIO;
            }

            value = !data->ipmi_resp.sfp_resp[SFP_TXDISABLE][pid];
            break;
        }
		case SFP1_TXFAULT:
        case SFP2_TXFAULT:
		case SFP3_TXFAULT:
        case SFP4_TXFAULT:
		case SFP5_TXFAULT:
        case SFP6_TXFAULT:
		case SFP7_TXFAULT:
        case SFP8_TXFAULT:
		case SFP9_TXFAULT:
        case SFP10_TXFAULT:
		case SFP11_TXFAULT:
        case SFP12_TXFAULT:
		case SFP13_TXFAULT:
        case SFP14_TXFAULT:
		case SFP15_TXFAULT:
        case SFP16_TXFAULT:
		case SFP17_TXFAULT:
        case SFP18_TXFAULT:
		case SFP19_TXFAULT:
        case SFP20_TXFAULT:
		case SFP21_TXFAULT:
        case SFP22_TXFAULT:
		case SFP23_TXFAULT:
        case SFP24_TXFAULT:
		case SFP25_TXFAULT:
        case SFP26_TXFAULT:
		case SFP27_TXFAULT:
        case SFP28_TXFAULT:
		case SFP29_TXFAULT:
        case SFP30_TXFAULT:
		case SFP31_TXFAULT:
        case SFP32_TXFAULT:
		case SFP33_TXFAULT:
        case SFP34_TXFAULT:
		case SFP35_TXFAULT:
        case SFP36_TXFAULT:
		case SFP37_TXFAULT:
        case SFP38_TXFAULT:
		case SFP39_TXFAULT:
        case SFP40_TXFAULT:
		case SFP41_TXFAULT:
        case SFP42_TXFAULT:
		case SFP43_TXFAULT:
        case SFP44_TXFAULT:
		case SFP45_TXFAULT:
        case SFP46_TXFAULT:
		case SFP47_TXFAULT:
        case SFP48_TXFAULT:
        {
            data = as5916_54xks_sfp_update_txfault();
            if (!data->ipmi_resp.sfp_valid[SFP_TXFAULT]) {
                return -EIO;
            }

            value = data->ipmi_resp.sfp_resp[SFP_TXFAULT][pid];
            break;
        }
		case SFP1_RXLOS:
        case SFP2_RXLOS:
		case SFP3_RXLOS:
        case SFP4_RXLOS:
		case SFP5_RXLOS:
        case SFP6_RXLOS:
		case SFP7_RXLOS:
        case SFP8_RXLOS:
		case SFP9_RXLOS:
        case SFP10_RXLOS:
		case SFP11_RXLOS:
        case SFP12_RXLOS:
		case SFP13_RXLOS:
        case SFP14_RXLOS:
		case SFP15_RXLOS:
        case SFP16_RXLOS:
		case SFP17_RXLOS:
        case SFP18_RXLOS:
		case SFP19_RXLOS:
        case SFP20_RXLOS:
		case SFP21_RXLOS:
        case SFP22_RXLOS:
		case SFP23_RXLOS:
        case SFP24_RXLOS:
		case SFP25_RXLOS:
        case SFP26_RXLOS:
		case SFP27_RXLOS:
        case SFP28_RXLOS:
		case SFP29_RXLOS:
        case SFP30_RXLOS:
		case SFP31_RXLOS:
        case SFP32_RXLOS:
		case SFP33_RXLOS:
        case SFP34_RXLOS:
		case SFP35_RXLOS:
        case SFP36_RXLOS:
		case SFP37_RXLOS:
        case SFP38_RXLOS:
		case SFP39_RXLOS:
        case SFP40_RXLOS:
		case SFP41_RXLOS:
        case SFP42_RXLOS:
		case SFP43_RXLOS:
        case SFP44_RXLOS:
		case SFP45_RXLOS:
        case SFP46_RXLOS:
		case SFP47_RXLOS:
        case SFP48_RXLOS:
        {
            data = as5916_54xks_sfp_update_rxlos();
            if (!data->ipmi_resp.sfp_valid[SFP_RXLOS]) {
                return -EIO;
            }

            value = !data->ipmi_resp.sfp_resp[SFP_RXLOS][pid];
            break;
        }
		default:
			return -EINVAL;
	}

	return sprintf(buf, "%d\n", value);
}

static ssize_t set_sfp(struct device *dev, struct device_attribute *da,
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

    /* Send IPMI write command */
    data->ipmi_tx_data[0] = pid + 1; /* Port ID base id for ipmi start from 1 */
    data->ipmi_tx_data[1] = 0x01;
    data->ipmi_tx_data[2] = disable;
    status = ipmi_send_message(&data->ipmi, IPMI_SFP_WRITE_CMD,
                                data->ipmi_tx_data, sizeof(data->ipmi_tx_data), NULL, 0);
    if (unlikely(status != 0)) {
        return status;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        return -EIO;
    }

    /* Update to ipmi_resp buffer to prevent from the impact of lazy update */
    data->ipmi_resp.sfp_resp[SFP_TXDISABLE][pid] = disable;

    return count;
}

static ssize_t show_qsfp(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_QSFP_ATTR; /* port id, 0 based */
    struct as5916_54xks_sfp_data *data = NULL;
    int value = 0;

	switch (attr->index) {
		case QSFP49_PRESENT:
        case QSFP50_PRESENT:
		case QSFP51_PRESENT:
        case QSFP52_PRESENT:
		case QSFP53_PRESENT:
        case QSFP54_PRESENT:
        {
            data = as5916_54xks_qsfp_update_present();
            if (!data->ipmi_resp.qsfp_valid[QSFP_PRESENT]) {
                return -EIO;
            }

            value = data->ipmi_resp.qsfp_resp[QSFP_PRESENT][pid];
			break;
        }
		case QSFP49_TXDISABLE:
		case QSFP50_TXDISABLE:
		case QSFP51_TXDISABLE:
		case QSFP52_TXDISABLE:
		case QSFP53_TXDISABLE:
		case QSFP54_TXDISABLE:
        {
            data = as5916_54xks_qsfp_update_txdisable();
            if (!data->ipmi_resp.qsfp_valid[QSFP_TXDISABLE]) {
                return -EIO;
            }

            value = !!data->ipmi_resp.qsfp_resp[QSFP_TXDISABLE][pid];
			break;
        }
		case QSFP49_RESET:
		case QSFP50_RESET:
		case QSFP51_RESET:
		case QSFP52_RESET:
		case QSFP53_RESET:
		case QSFP54_RESET:
        {
            data = as5916_54xks_qsfp_update_reset();
            if (!data->ipmi_resp.qsfp_valid[QSFP_RESET]) {
                return -EIO;
            }

            value = !data->ipmi_resp.qsfp_resp[QSFP_RESET][pid];
			break;
        }
		case QSFP49_LPMODE:
		case QSFP50_LPMODE:
		case QSFP51_LPMODE:
		case QSFP52_LPMODE:
		case QSFP53_LPMODE:
		case QSFP54_LPMODE:
        {
            data = as5916_54xks_qsfp_update_lpmode();
            if (!data->ipmi_resp.qsfp_valid[QSFP_LPMODE]) {
                return -EIO;
            }

            value = data->ipmi_resp.qsfp_resp[QSFP_LPMODE][pid];
			break;
        }        
		default:
			return -EINVAL;
	}

	return sprintf(buf, "%d\n", value);
}

static ssize_t set_qsfp_txdisable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	long disable;
	int status;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char pid = attr->index / NUM_OF_PER_QSFP_ATTR; /* port id, 0 based */
    struct as5916_54xks_sfp_data *data = NULL;

    data = as5916_54xks_qsfp_update_present();
    if (!data->ipmi_resp.qsfp_valid[QSFP_PRESENT]) {
        return -EIO;
    }
    
    if (!data->ipmi_resp.qsfp_resp[QSFP_PRESENT][pid]) {
        return -ENXIO;
    }
    
	status = kstrtol(buf, 10, &disable);
	if (status) {
		return status;
	}

    /* Send IPMI write command */
    data->ipmi_tx_data[0] = pid + 1; /* Port ID base id for ipmi start from 1 */
    data->ipmi_tx_data[1] = 0x01;
    data->ipmi_tx_data[2] = disable ? 0xf : 0;
    status = ipmi_send_message(&data->ipmi, IPMI_QSFP_WRITE_CMD,
                                data->ipmi_tx_data, sizeof(data->ipmi_tx_data), NULL, 0);
    if (unlikely(status != 0)) {
        return status;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        return -EIO;
    }

    /* Update to ipmi_resp buffer to prevent from the impact of lazy update */
    data->ipmi_resp.qsfp_resp[QSFP_TXDISABLE][pid] = disable;

    return count;
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

    /* Send IPMI write command */
    data->ipmi_tx_data[0] = pid + 1; /* Port ID base id for ipmi start from 1 */
    data->ipmi_tx_data[1] = 0x11;
    data->ipmi_tx_data[2] = reset;
    status = ipmi_send_message(&data->ipmi, IPMI_QSFP_WRITE_CMD,
                                data->ipmi_tx_data, sizeof(data->ipmi_tx_data), NULL, 0);
    if (unlikely(status != 0)) {
        return status;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        return -EIO;
    }

    /* Update to ipmi_resp buffer to prevent from the impact of lazy update */
    data->ipmi_resp.qsfp_resp[QSFP_RESET][pid] = reset;

    return count;
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

    /* Send IPMI write command */
    data->ipmi_tx_data[0] = pid + 1; /* Port ID base id for ipmi start from 1 */
    data->ipmi_tx_data[1] = 0x12;
    data->ipmi_tx_data[2] = lpmode;  /* 0: High Power Mode, 1: Low Power Mode */
    status = ipmi_send_message(&data->ipmi, IPMI_QSFP_WRITE_CMD,
                                data->ipmi_tx_data, sizeof(data->ipmi_tx_data), NULL, 0);
    if (unlikely(status != 0)) {
        return status;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        return -EIO;
    }

    /* Update to ipmi_resp buffer to prevent from the impact of lazy update */
    data->ipmi_resp.qsfp_resp[QSFP_LPMODE][pid] = lpmode;

    return count;
}

/*************************************************************************************
SFP:  PS: Index of SFP is 1~48
Offset   0 ~ 127: Addr 0x50 Offset   0~127         IPMI CMD: "ipmitool raw 0x34 0x1C <index of SFP> 0"
Offset 128 ~ 255: Addr 0x50 Offset 128~255         IPMI CMD: "ipmitool raw 0x34 0x1C <index of SFP> 1"
Offset 256 ~ 383: Addr 0x51 Offset   0~127         IPMI CMD: "ipmitool raw 0x34 0x1C <index of SFP> 2"
Offset 384 ~ 511: Addr 0x51 Offset 128~255(Page 0) IPMI CMD: "ipmitool raw 0x34 0x1C <index of SFP> 3"
Offset 512 ~ 639: Addr 0x51 Offset 128~255(Page 1) IPMI CMD: "ipmitool raw 0x34 0x1C <index of SFP> 4"
Offset 640 ~ 767: Addr 0x51 Offset 128~255(Page 2) IPMI CMD: "ipmitool raw 0x34 0x1C <index of SFP> 5"

QSFP:  PS: index of QSFP is 1~6"
Offset   0 ~ 127: Addr 0x50 Offset   0~127         IPMI CMD: "ipmitool raw 0x34 0x10 <index of QSFP> 0"
Offset 128 ~ 255: Addr 0x50 Offset 128~255(Page 0) IPMI CMD: "ipmitool raw 0x34 0x10 <index of QSFP> 1"
Offset 256 ~ 383: Addr 0x50 Offset 128~255(Page 1) IPMI CMD: "ipmitool raw 0x34 0x10 <index of QSFP> 2"
Offset 384 ~ 511: Addr 0x50 Offset 128~255(Page 2) IPMI CMD: "ipmitool raw 0x34 0x10 <index of QSFP> 3"
Offset 512 ~ 639: Addr 0x50 Offset 128~255(Page 3) IPMI CMD: "ipmitool raw 0x34 0x10 <index of QSFP> 4"
**************************************************************************************/
static ssize_t sfp_eeprom_read(loff_t off, char *buf, size_t count, int port)
{
    int status = 0;
    unsigned char cmd           = (port <= NUM_OF_SFP) ? IPMI_SFP_READ_CMD : IPMI_QSFP_READ_CMD;
    unsigned char ipmi_port_id  = (port <= NUM_OF_SFP) ? port : (port - NUM_OF_SFP);
    unsigned char ipmi_page     = off / IPMI_DATA_MAX_LEN;
    unsigned char length        = IPMI_DATA_MAX_LEN - (off % IPMI_DATA_MAX_LEN);

    data->ipmi_resp.eeprom_valid = 0;
    data->ipmi_tx_data[0] = ipmi_port_id;
    data->ipmi_tx_data[1] = ipmi_page; 
    status = ipmi_send_message(&data->ipmi, cmd, data->ipmi_tx_data, 2,
                                data->ipmi_resp.eeprom, IPMI_DATA_MAX_LEN);
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    /* Calculate return length */
    if (count < length) {
        length = count;
    }

    memcpy(buf, data->ipmi_resp.eeprom + (off % IPMI_DATA_MAX_LEN), length);
    data->ipmi_resp.eeprom_valid = 1;
    return length;

exit:
    return status;
}


static ssize_t sfp_bin_read(struct file *filp, struct kobject *kobj,
		struct bin_attribute *attr,
		char *buf, loff_t off, size_t count)
{
	ssize_t retval = 0;
    u64 port = 0;

	if (unlikely(!count)) {
		return count;
	}

    port = (u64)(attr->private);

	/*
	 * Read data from chip, protecting against concurrent updates
	 * from this host
	 */
	mutex_lock(&data->update_lock);

	while (count) {
		ssize_t status;

		status = sfp_eeprom_read(off, buf, count, port);
		if (status <= 0) {
			if (retval == 0) {
				retval = status;
			}
			break;
		}

		buf += status;
		off += status;
		count -= status;
		retval += status;
	}

	mutex_unlock(&data->update_lock);
	return retval;

}

/*************************************************************************************
SFP:  PS: Index of SFP is 1~48
ipmitool raw 0x34 0x1d <index of SFP> <page number> <offset> <Data len> <Data>
Offset   0 ~ 127: Addr 0x50 Offset   0~127         IPMI CMD: "ipmitool raw 0x34 0x1d <index of SFP> 0 offset <Data len> <Data>"
Offset 128 ~ 255: Addr 0x50 Offset 128~255         IPMI CMD: "ipmitool raw 0x34 0x1d <index of SFP> 1 offset <Data len> <Data>"
Offset 256 ~ 383: Addr 0x51 Offset   0~127         IPMI CMD: "ipmitool raw 0x34 0x1d <index of SFP> 2 offset <Data len> <Data>"
Offset 384 ~ 511: Addr 0x51 Offset 128~255(Page 0) IPMI CMD: "ipmitool raw 0x34 0x1d <index of SFP> 3 offset <Data len> <Data>"
Offset 512 ~ 639: Addr 0x51 Offset 128~255(Page 1) IPMI CMD: "ipmitool raw 0x34 0x1d <index of SFP> 4 offset <Data len> <Data>"
Offset 640 ~ 767: Addr 0x51 Offset 128~255(Page 2) IPMI CMD: "ipmitool raw 0x34 0x1d <index of SFP> 5 offset <Data len> <Data>"

QSFP:  PS: index of QSFP is 1~6"
ipmitool raw 0x34 0x11 <index of QSFP> <page number> <offset> <Data len> <Data>
Offset   0 ~ 127: Addr 0x50 Offset   0~127         IPMI CMD: "ipmitool raw 0x34 0x11 <index of QSFP> 0 offset <Data len> <Data>"
Offset 128 ~ 255: Addr 0x50 Offset 128~255(Page 0) IPMI CMD: "ipmitool raw 0x34 0x11 <index of QSFP> 1 offset <Data len> <Data>"
Offset 256 ~ 383: Addr 0x50 Offset 128~255(Page 1) IPMI CMD: "ipmitool raw 0x34 0x11 <index of QSFP> 2 offset <Data len> <Data>"
Offset 384 ~ 511: Addr 0x50 Offset 128~255(Page 2) IPMI CMD: "ipmitool raw 0x34 0x11 <index of QSFP> 3 offset <Data len> <Data>"
Offset 512 ~ 639: Addr 0x50 Offset 128~255(Page 3) IPMI CMD: "ipmitool raw 0x34 0x11 <index of QSFP> 4 offset <Data len> <Data>"
**************************************************************************************/
static ssize_t sfp_eeprom_write(loff_t off, char *buf, size_t count, int port)
{
    int status = 0;
    unsigned char cmd           = (port <= NUM_OF_SFP) ? IPMI_SFP_WRITE_CMD : IPMI_QSFP_WRITE_CMD;
    unsigned char ipmi_port_id  = (port <= NUM_OF_SFP) ? port : (port - NUM_OF_SFP);
    unsigned char ipmi_page     = off / IPMI_DATA_MAX_LEN;
    unsigned char length        = IPMI_DATA_MAX_LEN - (off % IPMI_DATA_MAX_LEN);
    struct sfp_eeprom_write_data wdata;

    /* Calculate write length */
    if (count < length) {
        length = count;
    }

    wdata.ipmi_tx_data[0] = ipmi_port_id;
    wdata.ipmi_tx_data[1] = ipmi_page;
    wdata.ipmi_tx_data[2] = (off % IPMI_DATA_MAX_LEN);
    wdata.ipmi_tx_data[3] = length;
    memcpy(&wdata.write_buf, buf, length);
    status = ipmi_send_message(&data->ipmi, cmd, &wdata.ipmi_tx_data[0], 
                                length + sizeof(wdata.ipmi_tx_data), NULL, 0);
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    return length;

exit:
    return status;
}

static ssize_t sfp_bin_write(struct file *filp, struct kobject *kobj,
				struct bin_attribute *attr,
				char *buf, loff_t off, size_t count)
{
	ssize_t retval = 0;
	u64 port = 0;

	if (unlikely(!count)) {
		return count;
	}

	port = (u64)(attr->private);

	/*
	 * Write data to chip, protecting against concurrent updates
	 * from this host, but not from other I2C masters.
	 */
	mutex_lock(&data->update_lock);

	while (count) {
		ssize_t status;

		status = sfp_eeprom_write(off, buf, count, port);
		if (status <= 0) {
			if (retval == 0) {
				retval = status;
			}
			break;
		}

		buf += status;
		off += status;
		count -= status;
		retval += status;
	}

	mutex_unlock(&data->update_lock);
	return retval;
}

#define EEPROM_FORMAT "module_eeprom_%d"

static int sysfs_eeprom_init(struct kobject *kobj, struct bin_attribute *eeprom, u64 port)
{
    int ret = 0;
    char *eeprom_name = NULL;
    
    eeprom_name = kzalloc(32, GFP_KERNEL);
    if (!eeprom_name) {
        ret = -ENOMEM;
        goto alloc_err;
    }

    sprintf(eeprom_name, EEPROM_FORMAT, (int)port);
    sysfs_bin_attr_init(eeprom);
	eeprom->attr.name = eeprom_name;
	eeprom->attr.mode = S_IRUGO | S_IWUSR;
	eeprom->read	  = sfp_bin_read;
	eeprom->write	  = sfp_bin_write;
	eeprom->size	  = (port <= NUM_OF_SFP) ? SFP_EEPROM_SIZE : QSFP_EEPROM_SIZE;
    eeprom->private   = (void*)port;

	/* Create eeprom file */
	ret = sysfs_create_bin_file(kobj, eeprom);
    if (unlikely(ret != 0)) {
        goto bin_err;
    }

    return ret;

bin_err:
    kfree(eeprom_name);
alloc_err:
    return ret;
}

static int sysfs_eeprom_cleanup(struct kobject *kobj, struct bin_attribute *eeprom)
{
	sysfs_remove_bin_file(kobj, eeprom);
	return 0;
}

static int as5916_54xks_sfp_probe(struct platform_device *pdev)
{
    int status = -1;
    int i = 0;

    for (i = 0; i < NUM_OF_PORT; i++) {
        /* Register sysfs hooks */
    	status = sysfs_eeprom_init(&pdev->dev.kobj, &data->eeprom[i],
    	                           i+1/* port name start from 1*/);
    	if (status) {
    		goto exit;
    	}
    }

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &as5916_54xks_sfp_group);
	if (status) {
		goto exit;
	}



    dev_info(&pdev->dev, "device created\n");

    return 0;

exit:
    /* Remove the eeprom attributes which were created successfully */
    for (--i; i >= 0; i--) {
        sysfs_eeprom_cleanup(&pdev->dev.kobj, &data->eeprom[i]);
    }
    
    return status;
}

static int as5916_54xks_sfp_remove(struct platform_device *pdev)
{
    int i = 0;

    for (i = 0; i < NUM_OF_PORT; i++) {
        sysfs_eeprom_cleanup(&pdev->dev.kobj, &data->eeprom[i]);
    }

    sysfs_remove_group(&pdev->dev.kobj, &as5916_54xks_sfp_group);
    return 0;
}

static int __init as5916_54xks_sfp_init(void)
{
    int ret;

    data = kzalloc(sizeof(struct as5916_54xks_sfp_data), GFP_KERNEL);
    if (!data) {
        ret = -ENOMEM;
        goto alloc_err;
    }

	mutex_init(&data->update_lock);

    ret = platform_driver_register(&as5916_54xks_sfp_driver);
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
    platform_driver_unregister(&as5916_54xks_sfp_driver);
dri_reg_err:
    kfree(data);
alloc_err:
    return ret;
}

static void __exit as5916_54xks_sfp_exit(void)
{
    ipmi_destroy_user(data->ipmi.user);
    platform_device_unregister(data->pdev);
    platform_driver_unregister(&as5916_54xks_sfp_driver);
    kfree(data);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("AS5916 54XKS sfp driver");
MODULE_LICENSE("GPL");

module_init(as5916_54xks_sfp_init);
module_exit(as5916_54xks_sfp_exit);

