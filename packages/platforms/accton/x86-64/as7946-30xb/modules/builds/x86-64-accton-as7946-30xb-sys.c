/*
 * Copyright (C)  Jake Lin <jake_lin@edge-core.com>
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

#define DRVNAME "as7946_30xb_sys"
#define ACCTON_IPMI_NETFN       0x34

#define IPMI_SYSEEPROM_READ_CMD 0x18
#define IPMI_TIMEOUT		(20 * HZ)
#define IPMI_READ_MAX_LEN       128
#define IPMI_RESET_CMD			0x65
#define IPMI_RESET_CMD_LENGTH	6

#define EEPROM_NAME		"eeprom"
#define EEPROM_SIZE		512	/*512 byte eeprom */

#define IPMI_GET_CPLD_VER_CMD   0x20
#define IPMI_GET_CPLD_CMD       0x22
#define CPU_CPLD_ADDR           0x65
#define SYSTEM_CPLD1_ADDR       0x60
#define FAN_CPLD_ADDR           0x68

static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);
static int as7946_30xb_sys_probe(struct platform_device *pdev);
static int as7946_30xb_sys_remove(struct platform_device *pdev);
static ssize_t show_cpld_version(struct device *dev, 
	struct device_attribute *da, char *buf);
static ssize_t get_reset(struct device *dev, struct device_attribute *da,
			char *buf);
static ssize_t set_reset(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);

struct ipmi_data {
	struct completion   read_complete;
	struct ipmi_addr    address;
	struct ipmi_user *  user;
	int                 interface;

	struct kernel_ipmi_msg tx_message;
	long                   tx_msgid;

	void            *rx_msg_data;
	unsigned short   rx_msg_len;
	unsigned char    rx_result;
	int              rx_recv_type;

	struct ipmi_user_hndl ipmi_hndlrs;
};

struct as7946_30xb_sys_data {
	struct platform_device *pdev;
	struct mutex     update_lock;
	char             valid;           /* != 0 if registers are valid */
	unsigned long    last_updated;    /* In jiffies */
	struct ipmi_data ipmi;
	unsigned char    ipmi_resp_eeprom[EEPROM_SIZE];
	unsigned char    ipmi_resp_cpld[2];
	unsigned char    ipmi_tx_data[3];
	unsigned char    ipmi_resp_rst[2];
	unsigned char    ipmi_tx_data_rst[IPMI_RESET_CMD_LENGTH];
	struct bin_attribute eeprom;      /* eeprom data */
};

struct as7946_30xb_sys_data *data = NULL;

static struct platform_driver as7946_30xb_sys_driver = {
	.probe      = as7946_30xb_sys_probe,
	.remove     = as7946_30xb_sys_remove,
	.driver     = {
		.name   = DRVNAME,
		.owner  = THIS_MODULE,
	},
};

enum as7946_30xb_sys_sysfs_attrs {
	CPU_CPLD_VER, /* CPU board CPLD version */
	SYSTEM_CPLD1_VER, /* System cpld1 version */
	FAN_CPLD_VER, /* FAN CPLD version */
	RESET_MAC,
};

static SENSOR_DEVICE_ATTR(cpu_cpld_ver, S_IRUGO, show_cpld_version, 
			  NULL, CPU_CPLD_VER);
static SENSOR_DEVICE_ATTR(system_cpld1_ver, S_IRUGO, show_cpld_version, 
			  NULL, SYSTEM_CPLD1_VER);
static SENSOR_DEVICE_ATTR(fan_cpld_ver, S_IRUGO, show_cpld_version, 
			  NULL, FAN_CPLD_VER);
static SENSOR_DEVICE_ATTR(reset_mac, S_IWUSR | S_IRUGO, \
			  get_reset, set_reset, RESET_MAC);

static struct attribute *as7946_30xb_sys_attributes[] = {
	&sensor_dev_attr_cpu_cpld_ver.dev_attr.attr,
	&sensor_dev_attr_system_cpld1_ver.dev_attr.attr,
	&sensor_dev_attr_fan_cpld_ver.dev_attr.attr,
	&sensor_dev_attr_reset_mac.dev_attr.attr,
	NULL
};

static const struct attribute_group as7946_30xb_sys_group = {
	.attrs = as7946_30xb_sys_attributes,
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

static ssize_t get_reset(struct device *dev, struct device_attribute *da,
			char *buf)
{
	int status = 0;

	mutex_lock(&data->update_lock);
	status = ipmi_send_message(&data->ipmi, IPMI_RESET_CMD, NULL, 0,
				   data->ipmi_resp_rst, sizeof(data->ipmi_resp_rst));
	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

	mutex_unlock(&data->update_lock);
	return sprintf(buf, "0x%x 0x%x", data->ipmi_resp_rst[0], data->ipmi_resp_rst[1]);

 exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t set_reset(struct device *dev, struct device_attribute *da,
		       const char *buf, size_t count)
{
	u32 magic[2];
	int status;

	if (sscanf(buf, "0x%x 0x%x", &magic[0], &magic[1]) != 2)
		return -EINVAL;

	if (magic[0] > 0xFF || magic[1] > 0xFF)
		return -EINVAL;

	mutex_lock(&data->update_lock);

	/* Send IPMI write command */
	data->ipmi_tx_data_rst[0] = 0;
	data->ipmi_tx_data_rst[1] = 0;
	data->ipmi_tx_data_rst[2] = 1;
	data->ipmi_tx_data_rst[3] = 1;
	data->ipmi_tx_data_rst[4] = magic[0];
	data->ipmi_tx_data_rst[5] = magic[1];

	status = ipmi_send_message(&data->ipmi, IPMI_RESET_CMD,
				   data->ipmi_tx_data_rst,
				   sizeof(data->ipmi_tx_data_rst), NULL, 0);
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

static ssize_t sys_eeprom_read(loff_t off, char *buf, size_t count)
{
	int status = 0;
	unsigned char length = 0;

	if ((off + count) > EEPROM_SIZE)
		return -EINVAL;

	length = (count >= IPMI_READ_MAX_LEN) ? IPMI_READ_MAX_LEN : count;
	data->ipmi_tx_data[0] = off;
	data->ipmi_tx_data[1] = length;
	status = ipmi_send_message(&data->ipmi, IPMI_SYSEEPROM_READ_CMD, 
					data->ipmi_tx_data, 2,
					data->ipmi_resp_eeprom + off, length);
	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EIO;
		goto exit;
	}

	status = length; /* Read length */
	memcpy(buf, data->ipmi_resp_eeprom + off, length);

exit:
    return status;
}

static ssize_t sysfs_bin_read(struct file *filp, struct kobject *kobj,
		struct bin_attribute *attr,
		char *buf, loff_t off, size_t count)
{
	ssize_t retval = 0;

	if (unlikely(!count))
		return count;

	/*
	 * Read data from chip, protecting against concurrent updates
	 * from this host
	 */
	mutex_lock(&data->update_lock);

	while (count) {
		ssize_t status;

		status = sys_eeprom_read(off, buf, count);
		if (status <= 0) {
			if (retval == 0)
				retval = status;

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

static int sysfs_eeprom_init(struct kobject *kobj, 
			     struct bin_attribute *eeprom)
{
	sysfs_bin_attr_init(eeprom);
	eeprom->attr.name = EEPROM_NAME;
	eeprom->attr.mode = S_IRUGO;
	eeprom->read	  = sysfs_bin_read;
	eeprom->write	  = NULL;
	eeprom->size	  = EEPROM_SIZE;

	/* Create eeprom file */
	return sysfs_create_bin_file(kobj, eeprom);
}

static int sysfs_eeprom_cleanup(struct kobject *kobj, 
				struct bin_attribute *eeprom)
{
	sysfs_remove_bin_file(kobj, eeprom);
	return 0;
}

static struct as7946_30xb_sys_data *
as7946_30xb_sys_update_cpld_ver(unsigned char cpld_addr)
{
	int status = 0;

	data->valid = 0;
	data->ipmi_tx_data[0] = cpld_addr;
	status = ipmi_send_message(&data->ipmi, IPMI_GET_CPLD_VER_CMD, 
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

static ssize_t show_cpld_version(struct device *dev, 
				 struct device_attribute *da, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	unsigned char cpld_addr = 0;
	unsigned int value = 0;
	int error = 0;

	switch (attr->index) {
	case CPU_CPLD_VER:
		cpld_addr = CPU_CPLD_ADDR;
		break;
	case SYSTEM_CPLD1_VER:
		cpld_addr = SYSTEM_CPLD1_ADDR;
		break;
	case FAN_CPLD_VER:
		cpld_addr = FAN_CPLD_ADDR;
		break;
	default:
		return -EINVAL;
	}

	mutex_lock(&data->update_lock);

	data = as7946_30xb_sys_update_cpld_ver(cpld_addr);
	if (!data->valid) {
		error = -EIO;
		goto exit;
	}
	switch (attr->index) {
	case CPU_CPLD_VER:
		value = data->ipmi_resp_cpld[0];
		break;
	case SYSTEM_CPLD1_VER:
		value = data->ipmi_resp_cpld[0] << 8 | data->ipmi_resp_cpld[1];
		break;
	case FAN_CPLD_VER:
		value = data->ipmi_resp_cpld[0] << 8 | data->ipmi_resp_cpld[1];
		break;
	default:
		return -EINVAL;
	}
	mutex_unlock(&data->update_lock);
	return sprintf(buf, "%d\n", value);

exit:
	mutex_unlock(&data->update_lock);
	return error;    
}

static int as7946_30xb_sys_probe(struct platform_device *pdev)
{
	int status = -1;

	/* Register sysfs hooks */
	status = sysfs_eeprom_init(&pdev->dev.kobj, &data->eeprom);
	if (status)
		goto exit;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &as7946_30xb_sys_group);
	if (status) {
		goto exit;
	}

	dev_info(&pdev->dev, "device created\n");

	return 0;

exit:
	return status;
}

static int as7946_30xb_sys_remove(struct platform_device *pdev)
{
	sysfs_eeprom_cleanup(&pdev->dev.kobj, &data->eeprom);
	sysfs_remove_group(&pdev->dev.kobj, &as7946_30xb_sys_group);

	return 0;
}

static int __init as7946_30xb_sys_init(void)
{
	int ret;

	data = kzalloc(sizeof(struct as7946_30xb_sys_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto alloc_err;
	}

	mutex_init(&data->update_lock);

	ret = platform_driver_register(&as7946_30xb_sys_driver);
	
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
	platform_driver_unregister(&as7946_30xb_sys_driver);
dri_reg_err:
	kfree(data);
alloc_err:
	return ret;
}

static void __exit as7946_30xb_sys_exit(void)
{
	ipmi_destroy_user(data->ipmi.user);
	platform_device_unregister(data->pdev);
	platform_driver_unregister(&as7946_30xb_sys_driver);
	kfree(data);
}

MODULE_AUTHOR("Jake Lin <jake_lin@edge-core.com>");
MODULE_DESCRIPTION("AS7946-30XB System driver");
MODULE_LICENSE("GPL");

module_init(as7946_30xb_sys_init);
module_exit(as7946_30xb_sys_exit);
