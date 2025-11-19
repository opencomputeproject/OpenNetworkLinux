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
#include "accton_ipmi_intf.h"

#define DRVNAME "as9737_32db_sys"

#define IPMI_READ_MAX_LEN 128

#define EEPROM_NAME "eeprom"
#define EEPROM_SIZE 256    /*    256 byte eeprom */

#define IPMI_SYSEEPROM_READ_CMD 0x18
#define IPMI_CPLD_VER_READ_CMD 0x20
#define IPMI_CPLD_REG_READ_CMD 0x22
#define IPMI_SEND_THERMAL_DATA_CMD 0x13

static int as9737_32db_sys_probe(struct platform_device *pdev);
static int as9737_32db_sys_remove(struct platform_device *pdev);
static ssize_t show(struct device *dev,
			struct device_attribute *da, char *buf);
static ssize_t set_bmc_thermal_data(struct device *dev, struct device_attribute *da,
				const char *buf, size_t count);

struct as9737_32db_sys_data {
	struct platform_device *pdev;
	struct mutex update_lock;
	char valid; /* != 0 if registers are valid */
	unsigned long last_updated;    /* In jiffies */
	struct ipmi_data ipmi;
	unsigned char ipmi_resp_eeprom[EEPROM_SIZE];
	unsigned char ipmi_resp_cpld[2];
	unsigned char ipmi_tx_data[3];
	struct bin_attribute eeprom; /* eeprom data */
};

struct as9737_32db_sys_data *data = NULL;

static struct platform_driver as9737_32db_sys_driver = {
	.probe = as9737_32db_sys_probe,
	.remove = as9737_32db_sys_remove,
	.driver = {
		.name = DRVNAME,
		.owner = THIS_MODULE,
	},
};

enum as9737_32db_sys_sysfs_attrs {
	CPU_EC_VER,
	FPGA_VER,
	BIOS_FLASH_ID,
	THERMAL_DATA
};

static SENSOR_DEVICE_ATTR(cpu_ec_version, S_IRUGO, show, NULL, CPU_EC_VER);
static SENSOR_DEVICE_ATTR(fpga_version, S_IRUGO, show, NULL, FPGA_VER);
static SENSOR_DEVICE_ATTR(bios_flash_id, S_IRUGO, show, NULL, BIOS_FLASH_ID);
static SENSOR_DEVICE_ATTR(bmc_thermal_data, S_IWUSR, NULL, set_bmc_thermal_data, THERMAL_DATA);

static struct attribute *as9737_32db_sys_attributes[] = {
	&sensor_dev_attr_cpu_ec_version.dev_attr.attr,
	&sensor_dev_attr_fpga_version.dev_attr.attr,
	&sensor_dev_attr_bios_flash_id.dev_attr.attr,
	&sensor_dev_attr_bmc_thermal_data.dev_attr.attr,
	NULL
};

static const struct attribute_group as9737_32db_sys_group = {
	.attrs = as9737_32db_sys_attributes,
};

static ssize_t sys_eeprom_read(loff_t off, char *buf, size_t count)
{
	int status = 0;
	unsigned char length = 0;

	if ((off + count) > EEPROM_SIZE)
		return -EINVAL;

	length = (count >= IPMI_READ_MAX_LEN) ? IPMI_READ_MAX_LEN : count;
	data->ipmi_tx_data[0] = (off & 0xff);
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
	eeprom->read = sysfs_bin_read;
	eeprom->size = EEPROM_SIZE;
	eeprom->write = NULL;

	/* Create eeprom file */
	return sysfs_create_bin_file(kobj, eeprom);
}

static int sysfs_eeprom_cleanup(struct kobject *kobj,
				struct bin_attribute *eeprom)
{
	sysfs_remove_bin_file(kobj, eeprom);
	return 0;
}

static struct as9737_32db_sys_data *as9737_32db_sys_update_reg(
					struct device_attribute *da)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	int status = 0;
	unsigned char ipmi_cmd = IPMI_CPLD_VER_READ_CMD;
	unsigned char tx_data_len = 1; 

	data->valid = 0;

	switch (attr->index) {
	case CPU_EC_VER:
		ipmi_cmd = IPMI_CPLD_VER_READ_CMD;
		tx_data_len = 1;
		data->ipmi_tx_data[0] = 0x21;
		break;

	case FPGA_VER:
		ipmi_cmd = IPMI_CPLD_VER_READ_CMD;
		tx_data_len = 1;
		data->ipmi_tx_data[0] = 0x60;
		break;

	case BIOS_FLASH_ID:
		ipmi_cmd = IPMI_CPLD_REG_READ_CMD;
		tx_data_len = 2;
		data->ipmi_tx_data[0] = 0x60;
		data->ipmi_tx_data[1] = 0xFA;
		break;
	default:
		goto exit;
	}

	status = ipmi_send_message(&data->ipmi, ipmi_cmd,
					data->ipmi_tx_data, tx_data_len,
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

static ssize_t show(struct device *dev,
				struct device_attribute *da, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	int count = 0;
	int error = 0;
	unsigned char bios_flash_id;

	mutex_lock(&data->update_lock);

	data = as9737_32db_sys_update_reg(da);
	if (!data->valid) {
		error = -EIO;
		goto exit;
	}

	switch (attr->index) {
	case CPU_EC_VER:
		count = sprintf(buf, "%02X.%02X\n", data->ipmi_resp_cpld[0], 
				data->ipmi_resp_cpld[1]);
		break;

	case FPGA_VER:
		count = sprintf(buf, "%02X\n", data->ipmi_resp_cpld[0]);
		break;

	case BIOS_FLASH_ID:
		bios_flash_id = data->ipmi_resp_cpld[0] & 0x3;
		count = sprintf(buf, "%d\n", (bios_flash_id == 0 || 
				bios_flash_id == 2) ? 1 : 2); /*1: master, 2: slave*/
		break;

	default:
		error = -EIO;
		goto exit;
	}
	
	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return error;
}

static ssize_t set_bmc_thermal_data(struct device *dev, struct device_attribute *da,
				const char *buf, size_t count)
{
	int status;
	int args;
	char *opt, tmp[32] = {0};
	char *tmp_p;
	size_t copy_size;
	u8 input[3] = {0};

	copy_size = (count < sizeof(tmp)) ? count : sizeof(tmp) - 1;
	#ifdef __STDC_LIB_EXT1__
	memcpy_s(tmp, copy_size, buf, copy_size);
	#else
	memcpy(tmp, buf, copy_size);
	#endif
	tmp[copy_size] = '\0';

	args = 0;
	tmp_p = strim(tmp);
	while (args < 3 && (opt = strsep(&tmp_p, " ")) != NULL) {
		if (kstrtou8(opt, 10, &input[args]) == 0) {
			args++;
		}
	}
	if (args != 3) {
		return -EINVAL;
	}

	mutex_lock(&data->update_lock);

	data->ipmi_tx_data[0] = input[0];
	data->ipmi_tx_data[1] = input[1];
	data->ipmi_tx_data[2] = input[2];
	status = ipmi_send_message(&data->ipmi, IPMI_SEND_THERMAL_DATA_CMD,
				data->ipmi_tx_data, 3,
				NULL, 0);

	if (unlikely(status != 0))
		goto exit;

	if (unlikely(data->ipmi.rx_result != 0)) {
		status = -EINVAL;
		goto exit;
	}

	status = count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static int as9737_32db_sys_probe(struct platform_device *pdev)
{
	int status = -1;

	/* Register sysfs hooks */
	status = sysfs_eeprom_init(&pdev->dev.kobj, &data->eeprom);
	if (status)
		goto exit;

	/* Register sysfs hooks */
	status = sysfs_create_group(&pdev->dev.kobj, &as9737_32db_sys_group);
	if (status)
		goto exit;

	dev_info(&pdev->dev, "device created\n");

	return 0;

exit:
	return status;
}

static int as9737_32db_sys_remove(struct platform_device *pdev)
{
	sysfs_eeprom_cleanup(&pdev->dev.kobj, &data->eeprom);
	sysfs_remove_group(&pdev->dev.kobj, &as9737_32db_sys_group);

	return 0;
}

static int __init as9737_32db_sys_init(void)
{
	int ret;

	data = kzalloc(sizeof(struct as9737_32db_sys_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto alloc_err;
	}

	mutex_init(&data->update_lock);

	ret = platform_driver_register(&as9737_32db_sys_driver);
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
	platform_driver_unregister(&as9737_32db_sys_driver);
dri_reg_err:
	kfree(data);
alloc_err:
	return ret;
}

static void __exit as9737_32db_sys_exit(void)
{
	ipmi_destroy_user(data->ipmi.user);
	platform_device_unregister(data->pdev);
	platform_driver_unregister(&as9737_32db_sys_driver);
	kfree(data);
}

MODULE_AUTHOR("Roger Ho <roger530_ho@edge-core.com>");
MODULE_DESCRIPTION("as9737_32db_sys driver");
MODULE_LICENSE("GPL");

module_init(as9737_32db_sys_init);
module_exit(as9737_32db_sys_exit);
