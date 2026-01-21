/*
 * Copyright (C)  Willy Liu <willy_liu@accton.com>
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
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/ipmi.h>
#include <linux/ipmi_smi.h>
#include <linux/platform_device.h>

#define DRVNAME "amx3200_thermal"
#define ACCTON_IPMI_NETFN 0x34
#define IPMI_THERMAL_READ_CMD 0x12
#define IPMI_THERMAL_WRITE_CMD 0x13
#define THERMAL_COUNT    4
#define THERMAL_DATA_LEN 3
#define THERMAL_DATA_COUNT (THERMAL_COUNT * THERMAL_DATA_LEN)

#define IPMI_TIMEOUT (5 * HZ)
#define IPMI_ERR_RETRY_TIMES 1

static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static ssize_t show_temp(struct device *dev, struct device_attribute *attr,
    char *buf);
static ssize_t send_sled_temp(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);
static int amx3200_thermal_probe(struct platform_device *pdev);
static int amx3200_thermal_remove(struct platform_device *pdev);

enum temp_data_index {
    TEMP_ADDR,
    TEMP_FAULT,
    TEMP_INPUT,
    TEMP_DATA_COUNT
};

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

struct amx3200_thermal_data {
    struct platform_device *pdev;
    struct device   *hwmon_dev;
    struct mutex update_lock;
    char valid;           /* != 0 if registers are valid */
    unsigned long last_updated;    /* In jiffies */
    char   ipmi_resp[THERMAL_DATA_COUNT]; /* 3 bytes for each thermal */
    struct ipmi_data ipmi;
    unsigned char ipmi_tx_data[2];  /* 0: thermal id, 1: temp */
};

struct amx3200_thermal_data *data = NULL;

static struct platform_driver amx3200_thermal_driver = {
    .probe = amx3200_thermal_probe,
    .remove = amx3200_thermal_remove,
    .driver = {
        .name = DRVNAME,
        .owner = THIS_MODULE,
    },
};

enum amx3200_thermal_sysfs_attrs {
    TEMP1_INPUT, // 0x4A
    TEMP2_INPUT, // 0x4B
    TEMP3_INPUT, // 0x4D
    TEMP4_INPUT, // 0x4E
    TEMP_SLED_INPUT // SLED1 0x4C and SLED2 0x4C
};

#define DECLARE_THERMAL_SENSOR_DEVICE_ATTR(index) \
    static SENSOR_DEVICE_ATTR(temp##index##_input, S_IRUGO, show_temp, \
                    NULL, TEMP##index##_INPUT); 
/* SLED THERMAL ATTR */
static SENSOR_DEVICE_ATTR(temp_sled_input, S_IWUSR, NULL, send_sled_temp, \
                         TEMP_SLED_INPUT);

#define DECLARE_THERMAL_ATTR(index) \
    &sensor_dev_attr_temp##index##_input.dev_attr.attr
#define DECLARE_SLED_THERMAL_ATTR() \
    &sensor_dev_attr_temp_sled_input.dev_attr.attr

DECLARE_THERMAL_SENSOR_DEVICE_ATTR(1);
DECLARE_THERMAL_SENSOR_DEVICE_ATTR(2);
DECLARE_THERMAL_SENSOR_DEVICE_ATTR(3);
DECLARE_THERMAL_SENSOR_DEVICE_ATTR(4);

static struct attribute *amx3200_thermal_attrs[] = {
    DECLARE_THERMAL_ATTR(1),
    DECLARE_THERMAL_ATTR(2),
    DECLARE_THERMAL_ATTR(3),
    DECLARE_THERMAL_ATTR(4),
    DECLARE_SLED_THERMAL_ATTR(),
    NULL
};
ATTRIBUTE_GROUPS(amx3200_thermal);

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

    for (retry = 0; retry <= IPMI_ERR_RETRY_TIMES; retry++) {
        status = _ipmi_send_message(ipmi, cmd, tx_data, tx_len, rx_data, rx_len);
        if (unlikely(status != 0)) {
            dev_err(&data->pdev->dev, "ipmi cmd(%x) err status(%d)\r\n",
                                        cmd, status);
            continue;
        }

        if (unlikely(ipmi->rx_result != 0)) {
            dev_err(&data->pdev->dev, "ipmi cmd(%x) err result(%d)\r\n",
                                        cmd, ipmi->rx_result);
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

static ssize_t send_sled_temp(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count)
{
    long sled_thermal[2];
    int i = 0;
    int status;
    char *copy_str;
    char *token;

    /* string parse */
    copy_str = kstrdup(buf, GFP_KERNEL);
    if (!copy_str) {
        return -ENOMEM;
    }

    /* only support two thermal input*/
    while ((token = strsep(&copy_str, " ")) != NULL) {
        if (i < 2)
        {
            status = kstrtol(token, 10, &sled_thermal[i]);
            if (status)
                return status;
        }

        i++;
    }

    if (i != 2)
        return -EINVAL;

    kfree(copy_str);

    mutex_lock(&data->update_lock);

    data->ipmi_tx_data[0] = sled_thermal[0];
    data->ipmi_tx_data[1] = sled_thermal[1];
    status = ipmi_send_message(&data->ipmi, IPMI_THERMAL_WRITE_CMD,
                                data->ipmi_tx_data, sizeof(data->ipmi_tx_data),
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

static int amx3200_thermal_probe(struct platform_device *pdev)
{
    int status = 0;
    struct device *hwmon_dev;

    hwmon_dev = hwmon_device_register_with_info(&pdev->dev, DRVNAME, 
                    NULL, NULL, amx3200_thermal_groups);
    if (IS_ERR(data->hwmon_dev)) {
        status = PTR_ERR(data->hwmon_dev);
        return status;
    }

    mutex_lock(&data->update_lock);
    data->hwmon_dev = hwmon_dev;
    mutex_unlock(&data->update_lock);

    dev_info(&pdev->dev, "Device Created\n");

    return status;
}

static int amx3200_thermal_remove(struct platform_device *pdev)
{
    mutex_lock(&data->update_lock);
    if (data->hwmon_dev) {
        hwmon_device_unregister(data->hwmon_dev);
        data->hwmon_dev = NULL;
    }
    mutex_unlock(&data->update_lock);

    return 0;
}

static int __init amx3200_thermal_init(void)
{
    int ret;

    data = kzalloc(sizeof(struct amx3200_thermal_data), GFP_KERNEL);
    if (!data) {
        ret = -ENOMEM;
        goto alloc_err;
    }

    mutex_init(&data->update_lock);

    ret = platform_driver_register(&amx3200_thermal_driver);
    if (ret < 0)
        goto dri_reg_err;

    data->pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
    if (IS_ERR(data->pdev)) {
        ret = PTR_ERR(data->pdev);
        goto dev_reg_err;
    }

    /* Set up IPMI interface */
    ret = init_ipmi_data(&data->ipmi, 0, &data->pdev->dev);
    if (ret) {
        goto ipmi_err;
    }

    return 0;

ipmi_err:
    platform_device_unregister(data->pdev);
dev_reg_err:
    platform_driver_unregister(&amx3200_thermal_driver);
dri_reg_err:
    kfree(data);
alloc_err:
    return ret;
}

static void __exit amx3200_thermal_exit(void)
{
    if (data) {
        ipmi_destroy_user(data->ipmi.user);
        platform_device_unregister(data->pdev);
        platform_driver_unregister(&amx3200_thermal_driver);
        kfree(data);
    }
}

MODULE_AUTHOR("Willy Liu <Willy@acctone.com>");
MODULE_DESCRIPTION("amx3200_thermal driver");
MODULE_LICENSE("GPL");

module_init(amx3200_thermal_init);
module_exit(amx3200_thermal_exit);
