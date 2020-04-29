/*
 * A FPGA driver for accton_asgvolt64
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


#define DRVNAME "asgvolt64_fpga"

#define FPGA_PCIE_ADDRESS 0xfbc04000
#define PCIE_OFFSET_1        1//0x4
#define PCIE_OFFSET_2        2//0x8
#define PCIE_OFFSET_3        3//0xC

struct accton_fpga_data {
    struct platform_device *pdev;
    u32 __iomem *hw_addr;
    struct mutex	 driver_lock;
    char			 valid;		   /* != 0 if registers are valid */
};

static struct accton_fpga_data  *fpga_ctl = NULL;

enum fpga_sysfs_attributes {
    FPGA_NODE_0=0,
    FPGA_NODE_1,
    FPGA_NODE_2,
    FPGA_NODE_3
};

static ssize_t fpga_read(struct device *dev, struct device_attribute *da,
             char *buf);

static ssize_t fpga_write(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);

static SENSOR_DEVICE_ATTR(port_linkup_1, S_IRUGO | S_IWUSR, fpga_read, fpga_write, FPGA_NODE_0);
static SENSOR_DEVICE_ATTR(port_linkup_2, S_IRUGO | S_IWUSR, fpga_read, fpga_write, FPGA_NODE_1);
static SENSOR_DEVICE_ATTR(port_act_1, S_IRUGO | S_IWUSR, fpga_read, fpga_write, FPGA_NODE_2);
static SENSOR_DEVICE_ATTR(port_act_2, S_IRUGO | S_IWUSR, fpga_read, fpga_write, FPGA_NODE_3);

static struct attribute *fpga_attributes[] = {
    &sensor_dev_attr_port_linkup_1.dev_attr.attr,
    &sensor_dev_attr_port_linkup_2.dev_attr.attr,
    &sensor_dev_attr_port_act_1.dev_attr.attr,
    &sensor_dev_attr_port_act_2.dev_attr.attr,
    NULL
};

static const struct attribute_group fpga_group = {
	.attrs = fpga_attributes,
};

static ssize_t fpga_read(struct device *dev, struct device_attribute *da,
             char *buf)
{
    unsigned int value;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    
    mutex_lock(&fpga_ctl->driver_lock);
    switch(attr->index)
    {
        case FPGA_NODE_0:
            value=ioread32(fpga_ctl->hw_addr);
            break;
        case FPGA_NODE_1:
            value=ioread32(fpga_ctl->hw_addr+PCIE_OFFSET_1);
            break;
        case FPGA_NODE_2:
            value=ioread32(fpga_ctl->hw_addr+PCIE_OFFSET_2);
            break;
        case FPGA_NODE_3:
            value=ioread32(fpga_ctl->hw_addr+PCIE_OFFSET_3);
            break;
        default:
            mutex_unlock(&fpga_ctl->driver_lock);
            return  -EINVAL;
    }
    mutex_unlock(&fpga_ctl->driver_lock);
    
    return sprintf(buf, "0x%.8x\n", ~(value));
}

static ssize_t fpga_write(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
    long value;
    int status;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
   
	status = kstrtol(buf, 16, &value);
	if (status) {
		return status;
	}
    value = 0xffffffff & value;
    value =~(value);
    mutex_lock(&fpga_ctl->driver_lock);
    switch(attr->index)
    {
        case FPGA_NODE_0:
            iowrite32((u32)value, fpga_ctl->hw_addr);
            break;
        case FPGA_NODE_1:
            iowrite32((u32)value, fpga_ctl->hw_addr+PCIE_OFFSET_1);
            break;
        case FPGA_NODE_2:
            iowrite32((u32)value, fpga_ctl->hw_addr+PCIE_OFFSET_2);
            break;
        case FPGA_NODE_3:
            iowrite32((u32)value, fpga_ctl->hw_addr+PCIE_OFFSET_3);
            break;
        default:
            mutex_unlock(&fpga_ctl->driver_lock);
            return  -EINVAL;
    }    
    mutex_unlock(&fpga_ctl->driver_lock);
    
    return count;
}

static int accton_fpga_suspend(struct platform_device *dev,
        pm_message_t state)
{
    return 0;
}

static int accton_fpga_resume(struct platform_device *dev)
{
    return 0;
}

static int accton_fpga_probe(struct platform_device *pdev)
{
    int status=0;
    const struct attribute_group *group = NULL;

    fpga_ctl->hw_addr=ioremap(FPGA_PCIE_ADDRESS, 100);
    if(fpga_ctl->hw_addr==NULL)
    {
        printk("FPGA ioremap fail\n");
        return -ENOMEM;
    }    
    group = &fpga_group;
    if (group)
    {
        status=sysfs_create_group(&pdev->dev.kobj, group);
    }
   
    return status;
}

static int accton_fpga_remove(struct platform_device *pdev)
{    
    const struct attribute_group *group = NULL;
    
    if(fpga_ctl->hw_addr)
        iounmap(fpga_ctl->hw_addr);
   
    group = &fpga_group;
    if (group)
    {
        sysfs_remove_group(&pdev->dev.kobj, group);
    }   
    return 0;
}

static struct platform_driver accton_fpga_driver = {
    .probe	  = accton_fpga_probe,
    .remove	 = accton_fpga_remove,
    .suspend	= accton_fpga_suspend,
    .resume	 = accton_fpga_resume,
    .driver	 = {
        .name   = DRVNAME,
        .owner  = THIS_MODULE,
    },
};

static int __init accton_fpga_init(void)
{
    int ret;
    
    ret = platform_driver_register(&accton_fpga_driver);
    if (ret < 0) {
        goto exit;
    }
    fpga_ctl = kzalloc(sizeof(struct accton_fpga_data), GFP_KERNEL);
    if (!fpga_ctl) {
        ret = -ENOMEM;
        platform_driver_unregister(&accton_fpga_driver);
        goto exit;
    }
    mutex_init(&fpga_ctl->driver_lock);
    fpga_ctl->pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
    if (IS_ERR(fpga_ctl->pdev)) {
        ret = PTR_ERR(fpga_ctl->pdev);
        platform_driver_unregister(&accton_fpga_driver);
        kfree(fpga_ctl);
        goto exit;
    }

exit:
    return ret;
}

static void __exit accton_fpga_exit(void)
{
    platform_device_unregister(fpga_ctl->pdev);
    platform_driver_unregister(&accton_fpga_driver);
    kfree(fpga_ctl);
}

module_init(accton_fpga_init);
module_exit(accton_fpga_exit);

MODULE_AUTHOR("Jostar Yang <jostar_yang@accton.com.tw>");
MODULE_DESCRIPTION("accton_fpga driver");
MODULE_LICENSE("GPL");

