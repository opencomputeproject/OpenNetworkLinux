/*
 * Copyright (C)  Roger Ho <roger530_ho@edge-core.com>
 *
 * Based on:
 *    pca954x.c from Kumar Gala <galak@kernel.crashing.org>
 * Copyright (C) 2006
 *
 * Based on:
 *    pca954x.c from Ken Harrenstien
 * Copyright (C) 2004 Google, Inc. (Ken Harrenstien)
 *
 * Based on:
 *    i2c-virtual_cb.c from Brian Kuschak <bkuschak@yahoo.com>
 * and
 *    pca9540.c from Jean Delvare <khali@linux-fr.org>.
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
#include <linux/string_helpers.h>

#define DRVNAME "as9817_64_sys"
#define ACCTON_IPMI_NETFN 0x34

#define IPMI_TIMEOUT (5 * HZ)
#define IPMI_ERR_RETRY_TIMES 1
#define IPMI_READ_MAX_LEN 128

#define IPMI_CPLD_READ_CMD 0x20
#define IPMI_OTP_PROTECT_CMD 0x94

static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static int as9817_64_sys_probe(struct platform_device *pdev);
static int as9817_64_sys_remove(struct platform_device *pdev);
static ssize_t show_version(struct device *dev,
                                struct device_attribute *da, char *buf);
static ssize_t set_otp_protect(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);

struct ipmi_data {
    struct completion read_complete;
    struct ipmi_addr address;
    struct ipmi_user * user;
    int interface;

    struct kernel_ipmi_msg tx_message;
    long tx_msgid;

    void *rx_msg_data;
    unsigned short rx_msg_len;
    unsigned char rx_result;
    int rx_recv_type;

    struct ipmi_user_hndl ipmi_hndlrs;
};

struct as9817_64_sys_data {
    struct platform_device *pdev;
    struct mutex update_lock;
    char valid; /* != 0 if registers are valid */
    unsigned long last_updated;    /* In jiffies */
    struct ipmi_data ipmi;
    unsigned char ipmi_resp_cpld[2];
    unsigned char ipmi_tx_data[2];
};

struct as9817_64_sys_data *data = NULL;

static struct platform_driver as9817_64_sys_driver = {
    .probe = as9817_64_sys_probe,
    .remove = as9817_64_sys_remove,
    .driver = {
        .name = DRVNAME,
        .owner = THIS_MODULE,
    },
};

enum as9817_64_sys_sysfs_attrs {
    FPGA_VER, /* FPGA version */
    OTP_PROTECT
};

static SENSOR_DEVICE_ATTR(fpga_version, S_IRUGO, show_version, NULL, FPGA_VER);
static SENSOR_DEVICE_ATTR(otp_protect, S_IWUSR, NULL, set_otp_protect, OTP_PROTECT);

static struct attribute *as9817_64_sys_attributes[] = {
    &sensor_dev_attr_fpga_version.dev_attr.attr,
    &sensor_dev_attr_otp_protect.dev_attr.attr,
    NULL
};

static const struct attribute_group as9817_64_sys_group = {
    .attrs = as9817_64_sys_attributes,
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

    char *cmdline = kstrdup_quotable_cmdline(current, GFP_KERNEL);

    int i = 0;
    char raw_cmd[20] = "";
    sprintf(raw_cmd, "0x%02x", cmd);
    if(tx_len) {
        for(i = 0; i < tx_len; i++)
            sprintf(raw_cmd + strlen(raw_cmd), " 0x%02x", tx_data[i]);
    }

    for (retry = 0; retry <= IPMI_ERR_RETRY_TIMES; retry++) {
        status = _ipmi_send_message(ipmi,cmd, tx_data, tx_len, rx_data, 
                 rx_len);
        if (unlikely(status != 0)) {
            dev_err(&data->pdev->dev,
                    "ipmi_send_message_%d err status(%d)[%s] raw_cmd=[%s] tx_msgid=(%02x)\r\n",
                    retry, status, cmdline ? cmdline : "", raw_cmd, 
                    (int)ipmi->tx_msgid);
            continue;
        }

        if (unlikely(ipmi->rx_result != 0)) {
            dev_err(&data->pdev->dev,
                    "ipmi_send_message_%d err rx_result(%d)[%s] raw_cmd=[%s] tx_msgid=(%02x)\r\n",
                    retry, ipmi->rx_result, cmdline ? cmdline : "", raw_cmd, 
                    (int)ipmi->tx_msgid);
            continue;
        }

        break;
    }

    if (cmdline) 
        kfree(cmdline);

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
    }
    else {
        ipmi->rx_msg_len = 0;
    }

    ipmi_free_recv_msg(msg);
    complete(&ipmi->read_complete);
}

static struct as9817_64_sys_data *as9817_64_sys_update_fpga_ver(void)
{
    int status = 0;

    data->valid = 0;
    data->ipmi_tx_data[0] = 0x60;
    status = ipmi_send_message(&data->ipmi, IPMI_CPLD_READ_CMD,
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

static ssize_t show_version(struct device *dev,
                                struct device_attribute *da, char *buf)
{
    unsigned char major;
    unsigned char minor;
    int error = 0;

    mutex_lock(&data->update_lock);

    data = as9817_64_sys_update_fpga_ver();
    if (!data->valid) {
        error = -EIO;
        goto exit;
    }

    major = data->ipmi_resp_cpld[0];
    minor = data->ipmi_resp_cpld[1];
    mutex_unlock(&data->update_lock);
    return sprintf(buf, "%d.%d\n", major, minor);

exit:
    mutex_unlock(&data->update_lock);
    return error;
}

static ssize_t set_otp_protect(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count)
{
    long otp_protect;
    int status;

    status = kstrtol(buf, 10, &otp_protect);
    if (status)
        return status;

    if (!otp_protect)
        return count;

    mutex_lock(&data->update_lock);

    data->ipmi_tx_data[0] = 3;
    status = ipmi_send_message(&data->ipmi, IPMI_OTP_PROTECT_CMD,
                                data->ipmi_tx_data, 1,
                                NULL, 0);
    if (unlikely(status != 0))
        goto exit;

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    status = count;

exit:
    mutex_unlock(&data->update_lock);
    return status;
}

static int as9817_64_sys_probe(struct platform_device *pdev)
{
    int status = -1;

    /* Register sysfs hooks */
    status = sysfs_create_group(&pdev->dev.kobj, &as9817_64_sys_group);
    if (status)
        goto exit;

    dev_info(&pdev->dev, "device created\n");

    return 0;

exit:
    return status;
}

static int as9817_64_sys_remove(struct platform_device *pdev)
{
    sysfs_remove_group(&pdev->dev.kobj, &as9817_64_sys_group);

    return 0;
}

static int __init as9817_64_sys_init(void)
{
    int ret;

    data = kzalloc(sizeof(struct as9817_64_sys_data), GFP_KERNEL);
    if (!data) {
        ret = -ENOMEM;
        goto alloc_err;
    }

    mutex_init(&data->update_lock);

    ret = platform_driver_register(&as9817_64_sys_driver);
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
    platform_driver_unregister(&as9817_64_sys_driver);
dri_reg_err:
    kfree(data);
alloc_err:
    return ret;
}

static void __exit as9817_64_sys_exit(void)
{
    ipmi_destroy_user(data->ipmi.user);
    platform_device_unregister(data->pdev);
    platform_driver_unregister(&as9817_64_sys_driver);
    kfree(data);
}

MODULE_AUTHOR("Roger Ho <roger530_ho@edge-core.com>");
MODULE_DESCRIPTION("as9817_64_sys driver");
MODULE_LICENSE("GPL");

module_init(as9817_64_sys_init);
module_exit(as9817_64_sys_exit);
