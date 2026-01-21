/*
 * Copyright (C)  Willy Liu <willy_liu@accton.com>
 *
 * This module supports the accton fpga via pcie that read/write reg
 * mechanism to get OSFP/SFP status ...etc.
 * This includes the:
 *     Accton amx3200 FPGA
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

#define __STDC_WANT_LIB_EXT1__ 1
#include <linux/string.h>

/***********************************************
 *       variable define
 * *********************************************/
#define DRVNAME                        "amx3200_fpga"

/*
 * PCIE BAR address
 */
#define BAR2_NUM                       2
#define REGION_LEN                     0xFF
#define FPGA_PCI_VENDOR_ID             0x11AA
#define FPGA_PCI_DEVICE_ID             0x1556
#define FPGA_TARGET_DEVICE_SLED1       0x02
#define FPGA_TARGET_DEVICE_SLED2       0x03
#define FPGA_PCIE_START_OFFSET         0x10000000

/***********************************************
 *       macro define
 * *********************************************/
#define pcie_err(fmt, args...) \
        printk(KERN_ERR "["DRVNAME"]: " fmt " ", ##args)

#define pcie_info(fmt, args...) \
        printk(KERN_ERR "["DRVNAME"]: " fmt " ", ##args)

/***********************************************
 *       structure & variable declare
 * *********************************************/
typedef struct pci_fpga_device_s {
    void  __iomem *base_addr2;   /* Data region2 base address */
    resource_size_t data_region2;
    struct pci_dev  *pci_dev;
    struct platform_device *sfp_pdev;
} pci_fpga_device_t;

enum PCI_FPGA_DEV {
    FPGA_SLED1 = 0,
    FPGA_SLED2,
    FPGA_DEV_COUNT
};

struct amx3200_fpga_data {
    unsigned long       last_updated;    /* In jiffies */
    pci_fpga_device_t   fpga_dev[FPGA_DEV_COUNT];
    struct mutex update_lock;
};

static struct platform_device *pdev = NULL;

static inline unsigned int fpga_read16(void __iomem *addr)
{
    return ioread16(addr);
}

static inline void fpga_write16(void __iomem *addr, u16 val)
{
    iowrite16(val, addr);
}

static inline unsigned int fpga_read8(void __iomem *addr)
{
    return ioread8(addr);
}

static inline void fpga_write8(void __iomem *addr, u16 val)
{
    iowrite8(val, addr);
}

static inline unsigned int fpga_read32(void __iomem *addr)
{
    return ioread32(addr);
}

static inline void fpga_write32(void __iomem *addr, u32 val)
{
    iowrite32(val, addr);
}

int amx3200_fpga_read32(int sled_id, u32 reg)
{
    int ret = -EPERM;
    struct amx3200_fpga_data *fpga_ctl;

    if (sled_id < 0 || sled_id >= FPGA_DEV_COUNT)
        return -EINVAL;

    if (reg%4 != 0)
        return -EINVAL;

    if (!pdev)
        return -EPERM;

    fpga_ctl = dev_get_drvdata(&pdev->dev);
    if (!fpga_ctl)
        return -EPERM;

    mutex_lock(&fpga_ctl->update_lock);
    ret = fpga_read32(fpga_ctl->fpga_dev[sled_id].base_addr2 + reg);
    mutex_unlock(&fpga_ctl->update_lock);
    return ret;
}
EXPORT_SYMBOL(amx3200_fpga_read32);

int amx3200_fpga_write32(int sled_id, u32 reg, u32 value)
{
    struct amx3200_fpga_data *fpga_ctl;

    if (sled_id < 0 || sled_id >= FPGA_DEV_COUNT)
        return -EINVAL;

    if (reg%4 != 0)
        return -EINVAL;

    if (!pdev)
        return -EPERM;

    fpga_ctl = dev_get_drvdata(&pdev->dev);
    if (!fpga_ctl)
        return -EPERM;

    mutex_lock(&fpga_ctl->update_lock);
    fpga_write32(fpga_ctl->fpga_dev[sled_id].base_addr2 + reg, value);
    mutex_unlock(&fpga_ctl->update_lock);
    return 0;
}
EXPORT_SYMBOL(amx3200_fpga_write32);

static int amx3200_pcie_fpga_stat_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct amx3200_fpga_data *fpga_ctl;
    struct pci_dev *pcidev = NULL;
    struct resource *ret;
    int status = 0, err = 0;

    fpga_ctl = devm_kzalloc(dev, sizeof(struct amx3200_fpga_data), GFP_KERNEL);
    if (!fpga_ctl) {
        return -ENOMEM;
    }
    mutex_init(&fpga_ctl->update_lock);
    platform_set_drvdata(pdev, fpga_ctl);

    while ((pcidev = pci_get_device(FPGA_PCI_VENDOR_ID, FPGA_PCI_DEVICE_ID, pcidev)) != NULL)
    {
        /* if (pcidev->bus->number == FPGA_TARGET_DEVICE_SLED1) and SLED1 present */
        if (pcidev->bus->number == FPGA_TARGET_DEVICE_SLED1) {
            fpga_ctl->fpga_dev[FPGA_SLED1].pci_dev = pcidev;
            err = pci_enable_device(pcidev);
            if (err != 0) {
                dev_err(dev, "Cannot enable PCI device(%x:%x)\n",
                             FPGA_PCI_VENDOR_ID, FPGA_PCI_DEVICE_ID);
                status = -ENODEV;
                goto exit_pci_disable1;
            }

            /* enable PCI bus-mastering */
            pci_set_master(pcidev);

            fpga_ctl->fpga_dev[FPGA_SLED1].base_addr2 = pci_iomap(pcidev, BAR2_NUM, 0);
            if (fpga_ctl->fpga_dev[FPGA_SLED1].base_addr2 == NULL) {
                dev_err(dev, "Failed to map BAR2 for SLED1\n");
                status = -EIO;
                goto exit_pci_disable1;
            }

            fpga_ctl->fpga_dev[FPGA_SLED1].data_region2 = pci_resource_start(pcidev, BAR2_NUM) + FPGA_PCIE_START_OFFSET;
            ret = request_mem_region(fpga_ctl->fpga_dev[FPGA_SLED1].data_region2, REGION_LEN, DRVNAME"_sled1");
            if (ret == NULL) {
                dev_err(dev, "[%s] cannot request region\n", DRVNAME"_sled1");
                status = -EIO;
                goto exit_pci_iounmap1;
            }

            dev_info(dev, "(BAR%d resource: Start=0x%lx, Length=0x%x)", BAR2_NUM,
                        (unsigned long)fpga_ctl->fpga_dev[FPGA_SLED1].data_region2, REGION_LEN);

            fpga_ctl->fpga_dev[FPGA_SLED1].sfp_pdev = platform_device_register_simple("amx3200_sled1_sfp", FPGA_SLED1, NULL, 0);
            if (IS_ERR(fpga_ctl->fpga_dev[FPGA_SLED1].sfp_pdev)) {
                status = PTR_ERR(fpga_ctl->fpga_dev[FPGA_SLED1].sfp_pdev);
                goto exit_pci_release1;
            }
        }

        /* if (pcidev->bus->number == FPGA_TARGET_DEVICE_SLED2) and SLED1 present */
        if (pcidev->bus->number == FPGA_TARGET_DEVICE_SLED2) {
            fpga_ctl->fpga_dev[FPGA_SLED2].pci_dev = pcidev;
            err = pci_enable_device(pcidev);
            if (err != 0) {
                dev_err(dev, "Cannot enable PCI device(%x:%x)\n",
                             FPGA_PCI_VENDOR_ID, FPGA_PCI_DEVICE_ID);
                status = -ENODEV;
                goto exit_pci_disable2;
            }

            /* enable PCI bus-mastering */
            pci_set_master(pcidev);

            fpga_ctl->fpga_dev[FPGA_SLED2].base_addr2 = pci_iomap(pcidev, BAR2_NUM, 0);
            if (fpga_ctl->fpga_dev[FPGA_SLED2].base_addr2 == NULL) {
                dev_err(dev, "Failed to map BAR2 for SLED2\n");
                status = -EIO;
                goto exit_pci_disable2;
            }

            fpga_ctl->fpga_dev[FPGA_SLED2].data_region2 = pci_resource_start(pcidev, BAR2_NUM) + FPGA_PCIE_START_OFFSET;
            ret = request_mem_region(fpga_ctl->fpga_dev[FPGA_SLED2].data_region2, REGION_LEN, DRVNAME"_sled2");
            if (ret == NULL) {
                dev_err(dev, "[%s] cannot request region\n", DRVNAME"_sled2");
                status = -EIO;
                goto exit_pci_iounmap2;
            }

            dev_info(dev, "(BAR%d resource: Start=0x%lx, Length=0x%x)", BAR2_NUM,
                        (unsigned long)fpga_ctl->fpga_dev[FPGA_SLED2].data_region2, REGION_LEN);

            fpga_ctl->fpga_dev[FPGA_SLED2].sfp_pdev = platform_device_register_simple("amx3200_sled2_sfp", FPGA_SLED2, NULL, 0);
            if (IS_ERR(fpga_ctl->fpga_dev[FPGA_SLED2].sfp_pdev)) {
                status = PTR_ERR(fpga_ctl->fpga_dev[FPGA_SLED2].sfp_pdev);
                goto exit_pci_release2;
            }
        }
    }

    return 0;

exit_pci_release2:
    release_mem_region(fpga_ctl->fpga_dev[FPGA_SLED2].data_region2, REGION_LEN);
exit_pci_iounmap2:
    pci_iounmap(fpga_ctl->fpga_dev[FPGA_SLED2].pci_dev,
                fpga_ctl->fpga_dev[FPGA_SLED2].base_addr2);
exit_pci_disable2:
    pci_disable_device(fpga_ctl->fpga_dev[FPGA_SLED2].pci_dev);
	platform_device_unregister(fpga_ctl->fpga_dev[FPGA_SLED1].sfp_pdev);
exit_pci_release1:
    release_mem_region(fpga_ctl->fpga_dev[FPGA_SLED1].data_region2, REGION_LEN);
exit_pci_iounmap1:
    pci_iounmap(fpga_ctl->fpga_dev[FPGA_SLED1].pci_dev,
                fpga_ctl->fpga_dev[FPGA_SLED1].base_addr2);
exit_pci_disable1:
    pci_disable_device(fpga_ctl->fpga_dev[FPGA_SLED1].pci_dev);

    return status;
}

static int amx3200_pcie_fpga_stat_remove(struct platform_device *pdev)
{
    int i = 0;
    struct amx3200_fpga_data *fpga_ctl = platform_get_drvdata(pdev);

    for (i = 0; i < ARRAY_SIZE(fpga_ctl->fpga_dev); i++) {
        if (!pci_is_enabled(fpga_ctl->fpga_dev[i].pci_dev))
            continue;

        if (fpga_ctl->fpga_dev[i].sfp_pdev)
            platform_device_unregister(fpga_ctl->fpga_dev[i].sfp_pdev);
        release_mem_region(fpga_ctl->fpga_dev[i].data_region2, REGION_LEN);
        pci_iounmap(fpga_ctl->fpga_dev[i].pci_dev, fpga_ctl->fpga_dev[i].base_addr2);
        pci_disable_device(fpga_ctl->fpga_dev[i].pci_dev);
    }

    return 0;
}

static struct platform_driver pcie_fpga_port_stat_driver = {
    .probe      = amx3200_pcie_fpga_stat_probe,
    .remove     = amx3200_pcie_fpga_stat_remove,
    .driver     = {
        .owner = THIS_MODULE,
        .name  = DRVNAME,
    },
};

static int __init amx3200_pcie_fpga_init(void)
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

static void __exit amx3200_pcie_fpga_exit(void)
{
    platform_device_unregister(pdev);
    platform_driver_unregister(&pcie_fpga_port_stat_driver);
}

module_init(amx3200_pcie_fpga_init);
module_exit(amx3200_pcie_fpga_exit);

MODULE_AUTHOR("Willy Liu <willy_liu@accton.com>");
MODULE_DESCRIPTION("AMX3200 FPGA via PCIE");
MODULE_LICENSE("GPL");
