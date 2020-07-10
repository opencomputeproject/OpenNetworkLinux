/*
 * A FPGA driver for accton_as9926_24d
 *
 * Copyright (C) 2020 Accton Technology Corporation.
 * Jostar Yang <jostar_yang@accton.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*#define DEBUG*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include <linux/dmi.h>
#include <linux/jiffies.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/pci.h>


#define DRVNAME "as9926_24d_fpga"

#define PCI_VENDOR_ID_ALTERA 0x1172
#define PCI_DEVICE_ID_ALTERA 0xE001
#define FPGA_PCIE_ADDRESS    0xF6000400
#define NUM_OF_PORT 26

struct as9926_24d_fpga_data {
    struct platform_device *pdev;
    u32 __iomem     *hw_addr;
    struct mutex     driver_lock;
    char             valid;           /* != 0 if registers are valid */
    unsigned long    last_updated; /* In jiffies */
    unsigned int     port_value[NUM_OF_PORT];
    unsigned int     port_link;
    unsigned int     port_act;
    unsigned int     port_speed_sfp;
};

static struct as9926_24d_fpga_data  *fpga_ctl = NULL;

enum fpga_sysfs_attributes {
    PORT_LINK_STATE = 0,
    PORT_ACTIVITY,
    PORT_SPEED_SFP
};

static ssize_t port_read(struct device *dev, struct device_attribute *da,
             char *buf);
static ssize_t port_write(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);

static SENSOR_DEVICE_ATTR(port_linkup, S_IRUGO | S_IWUSR, port_read, port_write, PORT_LINK_STATE);
static SENSOR_DEVICE_ATTR(port_act, S_IRUGO | S_IWUSR, port_read, port_write, PORT_ACTIVITY);
static SENSOR_DEVICE_ATTR(port_speed_sfp, S_IRUGO | S_IWUSR, port_read, port_write, PORT_SPEED_SFP);

static struct attribute *fpga_attributes[] = {
    &sensor_dev_attr_port_linkup.dev_attr.attr,
    &sensor_dev_attr_port_act.dev_attr.attr,
    &sensor_dev_attr_port_speed_sfp.dev_attr.attr,
    NULL
};

static const struct attribute_group fpga_group = {
    .attrs = fpga_attributes,
};

static ssize_t fpga_read_port_value(void)
{
    int i = 0;

    if (time_before(jiffies, fpga_ctl->last_updated + HZ / 2) && !fpga_ctl->valid) {
        return 0;
    }

    /* Read all ports data from PCIe */
    for (i = 0; i < 7 /*(ARRAY_SIZE(fpga_ctl->port_value)/4+1)*/; i++) {
        int j = 0;
        unsigned int group_value = 0; /* Read 4 ports at once */
        unsigned int port_link  = 0;
        unsigned int port_act   = 0;
        unsigned int port_value = 0;

        group_value = ioread32(fpga_ctl->hw_addr + i);

        for (j = 0; j < 4; j++) {
            int port = i*4 + j;

            if (port >= 26) {
                break;
            }

            port_value = (group_value >> (j*8)) & 0xFF;

            if (port < 24) {
                /* Update port link status */
                port_link = !!(port_value & BIT(2));
                port_act  = !!(port_value & BIT(6));
            }
            else { /* port 24-25(sfp ports) */
                port_link = !!(port_value & 0x3);
                port_act  = !!(port_value & 0x30);
                fpga_ctl->port_speed_sfp = (port_value & BIT(1)) ?
                                            fpga_ctl->port_speed_sfp | BIT(port-24) :
                                            fpga_ctl->port_speed_sfp & ~BIT(port-24);
            }


            fpga_ctl->port_value[port] = port_value;
            fpga_ctl->port_link = port_link ? fpga_ctl->port_link | BIT(port) :
                                              fpga_ctl->port_link & ~BIT(port);
            fpga_ctl->port_act  = port_act  ? fpga_ctl->port_act | BIT(port) :
                                              fpga_ctl->port_act & ~BIT(port);
        }
    }

    fpga_ctl->valid = 1;
    fpga_ctl->last_updated = jiffies;

    return 0;
}

static ssize_t fpga_write_port_value(void)
{
    int i = 0;
    int j = 0;

    /* Read all ports data from PCIe */
    for (i = 0; i < ARRAY_SIZE(fpga_ctl->port_value); i++) {
        int port_value = 0;

        if (i < 24) {
            port_value |= (fpga_ctl->port_link & BIT(i)) ? BIT(2) : 0;
            port_value |= (fpga_ctl->port_act  & BIT(i)) ? BIT(6) : 0;
        }
        else { /* port 24-25(sfp ports) */
            if (fpga_ctl->port_speed_sfp & BIT(i-24)) { /* Link at 10G */
                port_value |= (fpga_ctl->port_link & BIT(i)) ? BIT(1) : 0;
                port_value |= (fpga_ctl->port_act & BIT(i))  ? BIT(5) : 0;
            }
            else { /* Link at 1G */
                port_value |= (fpga_ctl->port_link & BIT(i)) ? BIT(0) : 0;
                port_value |= (fpga_ctl->port_act & BIT(i))  ? BIT(4) : 0;
            }
        }

        fpga_ctl->port_value[i] = port_value;
    }

    for (i = 0; i < 7; i++) {
        unsigned int group_value = 0;

        for (j = 0; j < 4; j++) {
            int port = i*4 + j;

            if (port >= 26) {
                break;
            }

            group_value |= (fpga_ctl->port_value[port] << (j*8));
        }

        if (i == 6) {
            unsigned int orig_val = 0;

            orig_val = ioread32(fpga_ctl->hw_addr + i);
            group_value = (orig_val & 0xFFFF0000) | (group_value & 0x0000FFFF);
        }

        iowrite32((u32)group_value, fpga_ctl->hw_addr + i);
    }

    fpga_ctl->valid = 0;

    return 0;
}

static ssize_t port_read(struct device *dev, struct device_attribute *da,
             char *buf)
{
    ssize_t  ret = 0;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

    mutex_lock(&fpga_ctl->driver_lock);
    fpga_read_port_value();

    switch(attr->index)
    {
        case PORT_LINK_STATE:
            ret = sprintf(buf, "0x%07X\n", fpga_ctl->port_link);
            break;
        case PORT_ACTIVITY:
            ret = sprintf(buf, "0x%07X\n", fpga_ctl->port_act);
            break;
        case PORT_SPEED_SFP:
            ret = sprintf(buf, "0x%1X\n", fpga_ctl->port_speed_sfp);
            break;
        default:
            ret = -EINVAL;
            break;
    }
    mutex_unlock(&fpga_ctl->driver_lock);

    return ret;
}

static ssize_t port_write(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count)
{
    long value;
    int status;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

    status = kstrtol(buf, 16, &value);
    if (status) {
        return status;
    }

    value &= (attr->index == PORT_SPEED_SFP) ? 0x3 : 0x3FFFFFF;

    mutex_lock(&fpga_ctl->driver_lock);

    switch(attr->index)
    {
        case PORT_LINK_STATE:
            fpga_ctl->port_link = value;
            fpga_ctl->port_act &= fpga_ctl->port_link;
            break;
        case PORT_ACTIVITY:
            fpga_ctl->port_act   = value;
            fpga_ctl->port_link |= fpga_ctl->port_act;
            break;
        case PORT_SPEED_SFP:
            fpga_ctl->port_speed_sfp = value;
            break;
        default:
            mutex_unlock(&fpga_ctl->driver_lock);
            return  -EINVAL;
    }

    fpga_write_port_value();
    mutex_unlock(&fpga_ctl->driver_lock);

    return count;
}

static int as9926_24d_fpga_probe(struct platform_device *pdev)
{
    int status = 0;
	struct pci_dev *pcidev;
	u32 val32;

	/* Find Altera register memory space */
	pcidev = pci_get_device(PCI_VENDOR_ID_ALTERA, PCI_DEVICE_ID_ALTERA, NULL);
	if (!pcidev) {
		return -ENODEV;
    }

    if (pci_write_config_byte(pcidev, 0x4, 0x2)) {
        status = -ENODEV;
        goto exit_pci;
    }

	if (pci_read_config_dword(pcidev, PCI_BASE_ADDRESS_0, &val32)) {
        status = -ENODEV;
        goto exit_pci;
    }

    fpga_ctl->hw_addr = ioremap(val32, 32);
    if (fpga_ctl->hw_addr == NULL) {
        status = -ENOMEM;
        goto exit_pci;
    }

    status = sysfs_create_group(&pdev->dev.kobj, &fpga_group);
    if (status) {
        goto exit;
    }

    return status;

exit:
    iounmap(fpga_ctl->hw_addr);
exit_pci:
    pci_dev_put(pcidev);
    return status;
}

static int as9926_24d_fpga_remove(struct platform_device *pdev)
{
    if (fpga_ctl->hw_addr) {
        iounmap(fpga_ctl->hw_addr);
    }

    sysfs_remove_group(&pdev->dev.kobj, &fpga_group);

    return 0;
}

static struct platform_driver as9926_24d_fpga_driver = {
    .probe     = as9926_24d_fpga_probe,
    .remove     = as9926_24d_fpga_remove,
    .driver     = {
        .name  = DRVNAME,
        .owner = THIS_MODULE,
    },
};

static int __init as9926_24d_fpga_init(void)
{
    int ret;

    ret = platform_driver_register(&as9926_24d_fpga_driver);
    if (ret < 0) {
        goto exit;
    }

    fpga_ctl = kzalloc(sizeof(struct as9926_24d_fpga_data), GFP_KERNEL);
    if (!fpga_ctl) {
        ret = -ENOMEM;
        platform_driver_unregister(&as9926_24d_fpga_driver);
        goto exit;
    }

    mutex_init(&fpga_ctl->driver_lock);

    fpga_ctl->pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
    if (IS_ERR(fpga_ctl->pdev)) {
        ret = PTR_ERR(fpga_ctl->pdev);
        platform_driver_unregister(&as9926_24d_fpga_driver);
        kfree(fpga_ctl);
        goto exit;
    }

exit:
    return ret;
}

static void __exit as9926_24d_fpga_exit(void)
{
    platform_device_unregister(fpga_ctl->pdev);
    platform_driver_unregister(&as9926_24d_fpga_driver);
    kfree(fpga_ctl);
}

module_init(as9926_24d_fpga_init);
module_exit(as9926_24d_fpga_exit);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("as9926_24d_fpga driver");
MODULE_LICENSE("GPL");
