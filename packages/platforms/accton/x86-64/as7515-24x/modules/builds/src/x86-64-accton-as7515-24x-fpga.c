// SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0
/*
 * Copyright (C) 2024 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com>
 *
 * This module supports the accton fpga via pcie that read/write reg
 * mechanism to get OSFP/SFP status ...etc.
 * This includes the:
 *	 Accton as7515_24x FPGA
 *
 * Copyright (C) 2017 Finisar Corp.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/i2c-mux.h>
#include <linux/version.h>
#include <linux/stat.h>
#include <linux/hwmon-sysfs.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/time64.h>
#include <linux/string.h>

/***********************************************
 *	   variable define
 * *********************************************/
#define DRVNAME						"as7515_24x_fpga"

/*
 * PCIE BAR0 address
 */
#define BAR0_NUM					0
#define REGION_LEN					0xFF
#define FPGA_PCI_VENDOR_ID			0x1172
#define FPGA_PCI_DEVICE_ID			0xE001

#define FPGA_PCIE_START_OFFSET		0x0000
#define FPGA_BOARD_INFO_REG			(FPGA_PCIE_START_OFFSET + 0x00)
#define FPGA_MAJOR_VER_REG			(FPGA_PCIE_START_OFFSET + 0x01)
#define FPGA_MINOR_VER_REG			(FPGA_PCIE_START_OFFSET + 0x02)

/***********************************************
 *	   macro define
 * *********************************************/
#define pcie_err(fmt, args...) \
		printk(KERN_ERR "["DRVNAME"]: " fmt " ", ##args)

#define pcie_info(fmt, args...) \
		printk(KERN_ERR "["DRVNAME"]: " fmt " ", ##args)

/***********************************************
 *	   structure & variable declare
 * *********************************************/
typedef struct pci_fpga_device_s {
	void  __iomem *data_base_addr0;
	resource_size_t data_region0;
	struct pci_dev  *pci_dev;
} pci_fpga_device_t;

/*fpga port status*/
struct as7515_24x_fpga_data {
	pci_fpga_device_t   pci_fpga_dev;
	struct platform_device *led_pdev;
	struct mutex update_lock;
};

static struct platform_device *pdev = NULL;

/***********************************************
 *	   enum define
 * *********************************************/
enum fpga_sysfs_attributes {
	FPGA_VERSION
};

/***********************************************
 *	   function declare
 * *********************************************/

static ssize_t status_read(struct device *dev, struct device_attribute *da,
			 char *buf);

static SENSOR_DEVICE_ATTR(version, S_IRUGO, status_read, NULL, FPGA_VERSION);

static struct attribute *fpga_attributes[] = {
	&sensor_dev_attr_version.dev_attr.attr,
	NULL
};

static const struct attribute_group fpga_group = {
	.attrs = fpga_attributes,
};

static inline unsigned int fpga_read(void __iomem *addr)
{
	return ioread8(addr);
}

static inline void fpga_write(void __iomem *addr, u8 val)
{
	iowrite8(val, addr);
}

int as7515_24x_fpga_read(u8 reg)
{
	int ret = -EPERM;
	struct as7515_24x_fpga_data *fpga_ctl;

	if (!pdev)
		return -EPERM;

	fpga_ctl = dev_get_drvdata(&pdev->dev);
	if (!fpga_ctl)
		return -EPERM;

	mutex_lock(&fpga_ctl->update_lock);
	ret = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + reg);
	mutex_unlock(&fpga_ctl->update_lock);

	return ret;
}
EXPORT_SYMBOL(as7515_24x_fpga_read);

int as7515_24x_fpga_write(u8 reg, u8 value)
{
	struct as7515_24x_fpga_data *fpga_ctl;

	if (!pdev)
		return -EPERM;

	fpga_ctl = dev_get_drvdata(&pdev->dev);
	if (!fpga_ctl)
		return -EPERM;

	mutex_lock(&fpga_ctl->update_lock);
	fpga_write(fpga_ctl->pci_fpga_dev.data_base_addr0 + reg, value);
	mutex_unlock(&fpga_ctl->update_lock);

	return 0;
}
EXPORT_SYMBOL(as7515_24x_fpga_write);

static ssize_t status_read(struct device *dev, struct device_attribute *da, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as7515_24x_fpga_data *fpga_ctl = dev_get_drvdata(dev);
	ssize_t ret = -EINVAL;
	u16 reg;
	u8 major, minor;

	switch(attr->index)
	{
		case FPGA_VERSION:
			reg = FPGA_MAJOR_VER_REG;
			major = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + reg);
			reg = FPGA_MINOR_VER_REG;
			minor = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + reg);

			ret = sprintf(buf, "%d.%d\n", major, minor);
			break;
		default:
			break;
	}

	return ret;
}

static int as7515_24x_pcie_fpga_stat_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct as7515_24x_fpga_data *fpga_ctl;
	struct pci_dev *pcidev;
	struct resource *ret;
	int status = 0, err = 0;

	fpga_ctl = devm_kzalloc(dev, sizeof(struct as7515_24x_fpga_data), GFP_KERNEL);
	if (!fpga_ctl) {
		return -ENOMEM;
	}
	mutex_init(&fpga_ctl->update_lock);
	platform_set_drvdata(pdev, fpga_ctl);

	pcidev = pci_get_device(FPGA_PCI_VENDOR_ID, FPGA_PCI_DEVICE_ID, NULL);
	 if (!pcidev) {
		dev_err(dev, "Cannot found PCI device(%x:%x)\n",
					 FPGA_PCI_VENDOR_ID, FPGA_PCI_DEVICE_ID);
		return -ENODEV;
	}
	fpga_ctl->pci_fpga_dev.pci_dev = pcidev;

	err = pci_enable_device(pcidev);
	if (err != 0) {
		dev_err(dev, "Cannot enable PCI device(%x:%x)\n",
					 FPGA_PCI_VENDOR_ID, FPGA_PCI_DEVICE_ID);
		status = -ENODEV;
		goto exit_pci_disable;
	}
	/* enable PCI bus-mastering */
	pci_set_master(pcidev);

	/*
	 * Detect platform for changing the setting behavior of LP mode.
	 */
	fpga_ctl->pci_fpga_dev.data_base_addr0 = pci_iomap(pcidev, BAR0_NUM, 0);
	if (fpga_ctl->pci_fpga_dev.data_base_addr0 == NULL) {
		dev_err(dev, "Failed to map BAR0\n");
		status = -EIO;
		goto exit_pci_disable;
	}

	fpga_ctl->pci_fpga_dev.data_region0 = pci_resource_start(pcidev, BAR0_NUM);
	ret = request_mem_region(fpga_ctl->pci_fpga_dev.data_region0, REGION_LEN, DRVNAME);
	if (ret == NULL) {
		dev_err(dev, "[%s] cannot request region\n", DRVNAME);
		status = -EIO;
		goto exit_pci_iounmap0;
	}
	dev_info(dev, "(BAR%d resource: Start=0x%lx, Length=0x%x)", BAR0_NUM,
				  (unsigned long)fpga_ctl->pci_fpga_dev.data_region0, REGION_LEN);

	status = sysfs_create_group(&pdev->dev.kobj, &fpga_group);
	if (status)
		goto exit_pci_release0;

	fpga_ctl->led_pdev = platform_device_register_simple("as7515_24x_led", -1, NULL, 0);
	if (IS_ERR(fpga_ctl->led_pdev)) {
		status = PTR_ERR(fpga_ctl->led_pdev);
		goto exit_sysfs_group;
	}

	return 0;

exit_sysfs_group:
	sysfs_remove_group(&pdev->dev.kobj, &fpga_group);
exit_pci_release0:
	release_mem_region(fpga_ctl->pci_fpga_dev.data_region0, REGION_LEN);
exit_pci_iounmap0:
	pci_iounmap(fpga_ctl->pci_fpga_dev.pci_dev, fpga_ctl->pci_fpga_dev.data_base_addr0);
exit_pci_disable:
	pci_disable_device(fpga_ctl->pci_fpga_dev.pci_dev);

	return status;
}

static int as7515_24x_pcie_fpga_stat_remove(struct platform_device *pdev)
{
	struct as7515_24x_fpga_data *fpga_ctl = platform_get_drvdata(pdev);

	if (fpga_ctl->led_pdev)
		platform_device_unregister(fpga_ctl->led_pdev);

	if (pci_is_enabled(fpga_ctl->pci_fpga_dev.pci_dev)) {
		sysfs_remove_group(&pdev->dev.kobj, &fpga_group);
		pci_iounmap(fpga_ctl->pci_fpga_dev.pci_dev, fpga_ctl->pci_fpga_dev.data_base_addr0);
		release_mem_region(fpga_ctl->pci_fpga_dev.data_region0, REGION_LEN);
		pci_disable_device(fpga_ctl->pci_fpga_dev.pci_dev);
	}

	return 0;
}

static struct platform_driver pcie_fpga_port_stat_driver = {
	.probe	  = as7515_24x_pcie_fpga_stat_probe,
	.remove	 = as7515_24x_pcie_fpga_stat_remove,
	.driver	 = {
		.owner = THIS_MODULE,
		.name  = DRVNAME,
	},
};

static int __init as7515_24x_pcie_fpga_init(void)
{
	int status = 0;

	/*
	 * Create FPGA platform driver and device
	 */
	status = platform_driver_register(&pcie_fpga_port_stat_driver);
	if (status < 0)
		return status;

	pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		status = PTR_ERR(pdev);
		goto exit_pci;
	}

	return status;

exit_pci:
	platform_driver_unregister(&pcie_fpga_port_stat_driver);

	return status;
}

static void __exit as7515_24x_pcie_fpga_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&pcie_fpga_port_stat_driver);
}

module_init(as7515_24x_pcie_fpga_init);
module_exit(as7515_24x_pcie_fpga_exit);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com>");
MODULE_DESCRIPTION("AS7515-24X FPGA via PCIE");
MODULE_LICENSE("GPL");
