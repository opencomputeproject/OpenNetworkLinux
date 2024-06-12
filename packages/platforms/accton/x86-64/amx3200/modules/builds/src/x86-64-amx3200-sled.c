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
#include <linux/hwmon-sysfs.h>
#include <linux/ipmi.h>
#include <linux/ipmi_smi.h>
#include <linux/platform_device.h>

#define DRVNAME "amx3200_sled"
#define ACCTON_IPMI_NETFN 0x34
#define IPMI_CPLD_READ_CMD 0x22
#define IPMI_CPLD_WRITE_CMD 0x23
/* MAIN CPLD Address */
#define IPMI_MAIN_CPLD_REG 0x60
/* CPLD Register */
#define IPMI_SLED1_POWER_STATUS_CMD 0x12
#define IPMI_SLED2_POWER_STATUS_CMD 0x13
#define IPMI_SLED_INTERRUPT_MASK_CMD 0x26
#define IPMI_SLED_INTERRUPT_STATUS_CMD 0x25
/* SLED Present bit*/
#define IPMI_SLED1_INTERRUPT_STATUS_BIT 0x1
#define IPMI_SLED2_INTERRUPT_STATUS_BIT 0x2
#define IPMI_SLED1_ALL_POWER_GOOD_BIT   0x4
#define IPMI_SLED2_ALL_POWER_GOOD_BIT   0x4
/* CPLD Register value */
#define IPMI_SLED1_INTERRUPT_MASK_VALUE 0x1
#define IPMI_SLED2_INTERRUPT_MASK_VALUE 0x2

#define IPMI_TIMEOUT (5 * HZ)
#define IPMI_ERR_RETRY_TIMES 1

static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static ssize_t show_sled_present(struct device *dev, struct device_attribute *attr,
            char *buf);
static ssize_t show_sled_power_status(struct device *dev, struct device_attribute *attr,
            char *buf);
static int amx3200_sled_probe(struct platform_device *pdev);
static int amx3200_sled_remove(struct platform_device *pdev);

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

struct amx3200_sled_data {
    struct platform_device *pdev;
    struct mutex update_lock;
    char valid;           /* != 0 if registers are valid */
    unsigned long last_updated;    /* In jiffies */
    unsigned char ipmi_resp;
    struct ipmi_data ipmi;
    unsigned char ipmi_tx_data[3];
};

struct amx3200_sled_data *data = NULL;

static struct platform_driver amx3200_sled_driver = {
    .probe = amx3200_sled_probe,
    .remove = amx3200_sled_remove,
    .driver = {
        .name = DRVNAME,
        .owner = THIS_MODULE,
    },
};


enum amx3200_sled_sysfs_attrs {
    SLED1_PRESENT,
    SLED2_PRESENT,
    SLED1_ALL_POWER_GOOD,
    SLED2_ALL_POWER_GOOD,
};

static SENSOR_DEVICE_ATTR(sled_1_present, S_IWUSR | S_IRUGO, show_sled_present, NULL,
                            SLED1_PRESENT);
static SENSOR_DEVICE_ATTR(sled_2_present, S_IWUSR | S_IRUGO, show_sled_present, NULL,
                            SLED2_PRESENT);
static SENSOR_DEVICE_ATTR(sled_1_all_power_good, S_IWUSR | S_IRUGO, show_sled_power_status, NULL,
                            SLED1_ALL_POWER_GOOD);
static SENSOR_DEVICE_ATTR(sled_2_all_power_good, S_IWUSR | S_IRUGO, show_sled_power_status, NULL,
                            SLED2_ALL_POWER_GOOD);

static struct attribute *amx3200_sled_attributes[] = {
    &sensor_dev_attr_sled_1_present.dev_attr.attr,
    &sensor_dev_attr_sled_2_present.dev_attr.attr,
    &sensor_dev_attr_sled_1_all_power_good.dev_attr.attr,
    &sensor_dev_attr_sled_2_all_power_good.dev_attr.attr,
    NULL
};

static const struct attribute_group amx3200_sled_group = {
    .attrs = amx3200_sled_attributes,
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
    {
        printk("_ipmi_send_message err %d \n", err);
        goto addr_err;
    }

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
            dev_err(&data->pdev->dev, "ipmi_send_message_%d err status(%d)\r\n",
                                        retry, status);
            continue;
        }

        if (unlikely(ipmi->rx_result != 0)) {
            dev_err(&data->pdev->dev, "ipmi_send_message_%d err result(%d)\r\n",
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

static struct amx3200_sled_data *amx3200_sled_present_update_device(void)
{
    int status = 0;
    unsigned char set_data;

    if (time_before(jiffies, data->last_updated + HZ * 5) && data->valid) {
        return data;
    }

    data->valid = 0;
    data->ipmi_tx_data[0] = IPMI_MAIN_CPLD_REG;
    data->ipmi_tx_data[1] = IPMI_SLED_INTERRUPT_MASK_CMD;
    /* Enable the interrupt mask before read the present status */
    /* Read the interrupt mask 0x26 */
    status = ipmi_send_message(&data->ipmi, IPMI_CPLD_READ_CMD,
                                data->ipmi_tx_data, 2,
                                &data->ipmi_resp, sizeof(data->ipmi_resp));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }
    /* Enable the interrupt mask 0x26 bit0 and bit1 */
    set_data = (data->ipmi_resp | IPMI_SLED1_INTERRUPT_MASK_VALUE |
                IPMI_SLED2_INTERRUPT_MASK_VALUE);
    data->ipmi_tx_data[0] = IPMI_MAIN_CPLD_REG;
    data->ipmi_tx_data[1] = IPMI_SLED_INTERRUPT_MASK_CMD;
    data->ipmi_tx_data[2] = set_data;
    status = ipmi_send_message(&data->ipmi, IPMI_CPLD_WRITE_CMD,
                                data->ipmi_tx_data, 3,
                                NULL, 0);
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->ipmi_tx_data[0] = IPMI_MAIN_CPLD_REG;
    data->ipmi_tx_data[1] = IPMI_SLED_INTERRUPT_STATUS_CMD;
    status = ipmi_send_message(&data->ipmi, IPMI_CPLD_READ_CMD,
                                data->ipmi_tx_data, 2,
                                &data->ipmi_resp, sizeof(data->ipmi_resp));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->last_updated = jiffies;
    data->valid = 1;


exit:
    return data;
}


static ssize_t show_sled_present(struct device *dev, struct device_attribute *da,
                            char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    bool value = 0;
    int error = 0;

    mutex_lock(&data->update_lock);

    data = amx3200_sled_present_update_device();
    if (!data->valid) {
        error = -EIO;
        goto exit;
    }

    switch (attr->index) {
    case SLED1_PRESENT:
        value = !(data->ipmi_resp & IPMI_SLED1_INTERRUPT_STATUS_BIT);
        break;
    case SLED2_PRESENT:
        value = !(data->ipmi_resp & IPMI_SLED2_INTERRUPT_STATUS_BIT);
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

static struct amx3200_sled_data *amx3200_sled1_power_update_device(void)
{
    int status = 0;

    if (time_before(jiffies, data->last_updated + HZ * 5) && data->valid) {
        return data;
    }

    data->valid = 0;
    data->ipmi_tx_data[0] = IPMI_MAIN_CPLD_REG;
    data->ipmi_tx_data[1] = IPMI_SLED1_POWER_STATUS_CMD;
    /* Enable the interrupt mask before read the present status */
    /* Read the interrupt mask 0x26 */
    status = ipmi_send_message(&data->ipmi, IPMI_CPLD_READ_CMD,
                                data->ipmi_tx_data, 2,
                                &data->ipmi_resp, sizeof(data->ipmi_resp));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->last_updated = jiffies;
    data->valid = 1;


exit:
    return data;
}

static struct amx3200_sled_data *amx3200_sled2_power_update_device(void)
{
    int status = 0;

    if (time_before(jiffies, data->last_updated + HZ * 5) && data->valid) {
        return data;
    }

    data->valid = 0;
    data->ipmi_tx_data[0] = IPMI_MAIN_CPLD_REG;
    data->ipmi_tx_data[1] = IPMI_SLED2_POWER_STATUS_CMD;
    /* Enable the interrupt mask before read the present status */
    /* Read the interrupt mask 0x26 */
    status = ipmi_send_message(&data->ipmi, IPMI_CPLD_READ_CMD,
                                data->ipmi_tx_data, 2,
                                &data->ipmi_resp, sizeof(data->ipmi_resp));
    if (unlikely(status != 0)) {
        goto exit;
    }

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->last_updated = jiffies;
    data->valid = 1;


exit:
    return data;
}

static ssize_t show_sled_power_status(struct device *dev, struct device_attribute *da,
                            char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    bool value = 0;
    int error = 0;

    mutex_lock(&data->update_lock);

    switch (attr->index) {
    case SLED1_ALL_POWER_GOOD:
        data = amx3200_sled1_power_update_device();
        if (!data->valid) {
            error = -EIO;
            goto exit;
        }
        value = ((data->ipmi_resp & IPMI_SLED1_ALL_POWER_GOOD_BIT) >> 2);
        break;
    case SLED2_ALL_POWER_GOOD:
        data = amx3200_sled2_power_update_device();
        if (!data->valid) {
            error = -EIO;
            goto exit;
        }
        value = ((data->ipmi_resp & IPMI_SLED2_ALL_POWER_GOOD_BIT) >> 2);
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

static int amx3200_sled_probe(struct platform_device *pdev)
{
    int status = -1;

    /* Register sysfs hooks */
    status = sysfs_create_group(&pdev->dev.kobj, &amx3200_sled_group);
    if (status)
        goto exit;

    dev_info(&pdev->dev, "device created\n");

    return 0;

exit:
    return status;
}

static int amx3200_sled_remove(struct platform_device *pdev)
{
    sysfs_remove_group(&pdev->dev.kobj, &amx3200_sled_group);

    return 0;
}

static int __init amx3200_sled_init(void)
{
    int ret;

    data = kzalloc(sizeof(struct amx3200_sled_data), GFP_KERNEL);
    if (!data) {
        ret = -ENOMEM;
        goto alloc_err;
    }

    mutex_init(&data->update_lock);
    data->valid = 0;

    ret = platform_driver_register(&amx3200_sled_driver);
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
    platform_driver_unregister(&amx3200_sled_driver);
dri_reg_err:
    kfree(data);
alloc_err:
    return ret;
}

static void __exit amx3200_sled_exit(void)
{
    ipmi_destroy_user(data->ipmi.user);
    platform_device_unregister(data->pdev);
    platform_driver_unregister(&amx3200_sled_driver);
    kfree(data);
}

MODULE_AUTHOR("Willy Liu <willy_liu@accton.com>");
MODULE_DESCRIPTION("amx3200_sled driver");
MODULE_LICENSE("GPL");

module_init(amx3200_sled_init);
module_exit(amx3200_sled_exit);
