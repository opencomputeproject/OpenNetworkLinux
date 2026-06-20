/*
 * Copyright (C)  Willy Liu <willy_liu@accton.com>
 *
 * This module supports the accton fpga via pcie that read/write reg
 * mechanism to get OSFP/SFP status ...etc.
 * This includes the:
 *     Accton as9947_36xkb FPGA
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
#include <linux/platform_data/i2c-ocores.h>

/***********************************************
 *       variable define
 * *********************************************/
#define DRVNAME                        "as9947_36xkb_fpga"
#define OCORES_I2C_DRVNAME             "ocores-as9947"

#define PORT_NUM                       40 /* 4 SFP +36 OSFPs/QSFPDDs */

/*
 * PCIE BAR address
 */
#define BAR4_NUM                       4
#define BAR5_NUM                       5

/* need checking*/
#define REGION_LEN                     0xFF
#define FPGA_PCI_VENDOR_ID             0x1172
#define FPGA_PCI_DEVICE_ID             0xe001

#define FPGA_PCIE_START_OFFSET         0x0000
#define FPGA_MAJOR_VER_REG             0x01
#define FPGA_MINOR_VER_REG             0x02
#define SPI_BUSY_MASK_CPLD1            0x01
#define SPI_BUSY_MASK_CPLD2            0x02

#define FPGA_PCIE_START_OFFSET         0x0000
#define FPGA_BOARD_INFO_REG            (FPGA_PCIE_START_OFFSET + 0x00)

/* CPLD 1 */
#define CPLD1_PCIE_START_OFFSET        0x2000
/* CPLD 2 */
#define CPLD2_PCIE_START_OFFSET        0x3000
/***********************************************
 *       macro define
 * *********************************************/
#define pcie_err(fmt, args...) \
        printk(KERN_ERR "["DRVNAME"]: " fmt " ", ##args)

#define pcie_info(fmt, args...) \
        printk(KERN_ERR "["DRVNAME"]: " fmt " ", ##args)


#define LOCK(lock)      \
do {                                                \
    spin_lock(lock);                                \
} while (0)

#define UNLOCK(lock)    \
do {                                                \
    spin_unlock(lock);                              \
} while (0)


/***********************************************
 *       structure & variable declare
 * *********************************************/
typedef struct pci_fpga_device_s {
    void  __iomem *data_base_addr4;
    void  __iomem *data_base_addr5;
    resource_size_t data_region4;
    resource_size_t data_region5;
    struct pci_dev  *pci_dev;
    struct platform_device *fpga_i2c[PORT_NUM];
} pci_fpga_device_t;

/*fpga port status*/
struct as9947_36xkb_fpga_data {
    u8                  cpld_reg[2];
    unsigned long       last_updated;    /* In jiffies */
    pci_fpga_device_t   pci_fpga_dev;
};

static struct platform_device *pdev = NULL;
extern spinlock_t cpld_access_lock;
extern int wait_spi(u32 mask, unsigned long timeout);

struct attribute_mapping {
    u16 attr_base;
    u16 reg;
    u8 revert;
};

static inline unsigned int fpga_read(void __iomem *addr, u32 spi_mask)
{
    wait_spi(spi_mask, usecs_to_jiffies(20));
    return ioread8(addr);
}

static inline void fpga_write(void __iomem *addr, u8 val, u32 spi_mask)
{
    wait_spi(spi_mask, usecs_to_jiffies(20));
    iowrite8(val, addr);
}

struct _port_data {
    u16 offset;
    u16 mask; /* SPI Busy mask : 0x01 --> CPLD1, 0x02 --> CPLD2 */
};
/* ============PCIe Bar Offset to I2C Master Mapping============== */
static const struct _port_data port[PORT_NUM]= {
    {0x2380, SPI_BUSY_MASK_CPLD1},/* 0x2380 - 0x2390  CPLD1 I2C Master SFP28 Port0 */
    {0x23A0, SPI_BUSY_MASK_CPLD1},/* 0x23A0 - 0x23B0  CPLD1 I2C Master SFP28 Port1 */
    {0x23C0, SPI_BUSY_MASK_CPLD1},/* 0x23C0 - 0x23D0  CPLD1 I2C Master SFP28 Port2 */
    {0x23E0, SPI_BUSY_MASK_CPLD1},/* 0x23E0 - 0x23F0  CPLD1 I2C Master SFP28 Port3 */
    {0x2100, SPI_BUSY_MASK_CPLD1},/* 0x2100 - 0x2110  CPLD1 I2C Master QSFP28 Port0 */
    {0x2120, SPI_BUSY_MASK_CPLD1},/* 0x2120 - 0x2130  CPLD1 I2C Master QSFP28 Port1 */
    {0x2140, SPI_BUSY_MASK_CPLD1},/* 0x2140 - 0x2150  CPLD1 I2C Master QSFP28 Port2 */
    {0x2160, SPI_BUSY_MASK_CPLD1},/* 0x2160 - 0x2170  CPLD1 I2C Master QSFP28 Port3 */
    {0x2180, SPI_BUSY_MASK_CPLD1},/* 0x2180 - 0x2190  CPLD1 I2C Master QSFP28 Port4 */
    {0x21A0, SPI_BUSY_MASK_CPLD1},/* 0x21A0 - 0x21B0  CPLD1 I2C Master QSFP28 Port5 */
    {0x21C0, SPI_BUSY_MASK_CPLD1},/* 0x21C0 - 0x21D0  CPLD1 I2C Master QSFP28 Port6 */
    {0x21E0, SPI_BUSY_MASK_CPLD1},/* 0x21E0 - 0x21F0  CPLD1 I2C Master QSFP28 Port7 */
    {0x2200, SPI_BUSY_MASK_CPLD1},/* 0x2200 - 0x2210  CPLD1 I2C Master QSFP28 Port8 */
    {0x2220, SPI_BUSY_MASK_CPLD1},/* 0x2220 - 0x2230  CPLD1 I2C Master QSFP28 Port9 */
    {0x2240, SPI_BUSY_MASK_CPLD1},/* 0x2240 - 0x2250  CPLD1 I2C Master QSFP28 Port10 */
    {0x2260, SPI_BUSY_MASK_CPLD1},/* 0x2260 - 0x2270  CPLD1 I2C Master QSFP28 Port11 */
    {0x2280, SPI_BUSY_MASK_CPLD1},/* 0x2280 - 0x2290  CPLD1 I2C Master QSFP28 Port12 */
    {0x22A0, SPI_BUSY_MASK_CPLD1},/* 0x22A0 - 0x22B0  CPLD1 I2C Master QSFP28 Port13 */
    {0x22C0, SPI_BUSY_MASK_CPLD1},/* 0x22C0 - 0x22D0  CPLD1 I2C Master QSFP28 Port14 */
    {0x22E0, SPI_BUSY_MASK_CPLD1},/* 0x22E0 - 0x22F0  CPLD1 I2C Master QSFP28 Port15 */
    {0x2300, SPI_BUSY_MASK_CPLD1},/* 0x2300 - 0x2310  CPLD1 I2C Master QSFP28 Port16 */
    {0x2320, SPI_BUSY_MASK_CPLD1},/* 0x2320 - 0x2330  CPLD1 I2C Master QSFP28 Port17 */
    {0x2340, SPI_BUSY_MASK_CPLD1},/* 0x2340 - 0x2350  CPLD1 I2C Master QSFP28 Port18 */
    {0x2360, SPI_BUSY_MASK_CPLD1},/* 0x2360 - 0x2370  CPLD1 I2C Master QSFP28 Port19 */
    {0x3100, SPI_BUSY_MASK_CPLD2},/* 0x3100 - 0x3110  CPLD2 I2C Master QSFP28 Port20 */
    {0x3120, SPI_BUSY_MASK_CPLD2},/* 0x3120 - 0x3130  CPLD2 I2C Master QSFP28 Port21 */
    {0x3140, SPI_BUSY_MASK_CPLD2},/* 0x3140 - 0x3150  CPLD2 I2C Master QSFP28 Port22 */
    {0x3160, SPI_BUSY_MASK_CPLD2},/* 0x3160 - 0x3170  CPLD2 I2C Master QSFP28 Port23*/
    {0x3180, SPI_BUSY_MASK_CPLD2},/* 0x3180 - 0x3190  CPLD2 I2C Master QSFP28 Port24 */
    {0x31A0, SPI_BUSY_MASK_CPLD2},/* 0x31A0 - 0x31B0  CPLD2 I2C Master QSFP28 Port25 */
    {0x31C0, SPI_BUSY_MASK_CPLD2},/* 0x31C0 - 0x31D0  CPLD2 I2C Master QSFP28 Port26 */
    {0x31E0, SPI_BUSY_MASK_CPLD2},/* 0x31E0 - 0x31F0  CPLD2 I2C Master QSFP28 Port27 */
    {0x3200, SPI_BUSY_MASK_CPLD2},/* 0x3200 - 0x3210  CPLD2 I2C Master QSFP28 Port28 */
    {0x3220, SPI_BUSY_MASK_CPLD2},/* 0x3220 - 0x3230  CPLD2 I2C Master QSFP28 Port29 */
    {0x3240, SPI_BUSY_MASK_CPLD2},/* 0x3240 - 0x3250  CPLD2 I2C Master QSFP28 Port30 */
    {0x3260, SPI_BUSY_MASK_CPLD2},/* 0x3260 - 0x3270  CPLD2 I2C Master QSFP28 Port31 */
    {0x3280, SPI_BUSY_MASK_CPLD2},/* 0x3280 - 0x3290  CPLD2 I2C Master QSFP28 Port32 */
    {0x32A0, SPI_BUSY_MASK_CPLD2},/* 0x32A0 - 0x32B0  CPLD2 I2C Master QSFP28 Port33 */
    {0x32C0, SPI_BUSY_MASK_CPLD2},/* 0x32C0 - 0x34D0  CPLD2 I2C Master QSFP28 Port34 */
    {0x32E0, SPI_BUSY_MASK_CPLD2},/* 0x32E0 - 0x34F0  CPLD2 I2C Master QSFP28 Port35 */
};

static struct ocores_i2c_platform_data as9947_36xkb_platform_data = {
    .reg_io_width = 1,
    .reg_shift = 2,
    /*
     * PRER_L and PRER_H are calculated based on clock_khz and bus_khz
     * in i2c-ocores.c:ocores_init.
     */
#if 1
    /* SCL 400KHZ in FPGA spec. => PRER_L = 0x0B, PRER_H = 0x00 */
    .clock_khz = 24000,
    .bus_khz = 400,
#else
    /* SCL 100KHZ in FPGA spec. => PRER_L = 0x2F, PRER_H = 0x00 */
    .clock_khz = 24000,
    .bus_khz = 100,
#endif
};

struct platform_device *ocore_i2c_device_add(unsigned int id, unsigned long bar_base,
                                             unsigned int offset)
{
    struct resource res = DEFINE_RES_MEM(bar_base + offset, 0x20);
    struct platform_device *pdev;
    int err;

    pdev = platform_device_alloc(OCORES_I2C_DRVNAME, id);
    if (!pdev) {
        err = -ENOMEM;
        pcie_err("Port%u device allocation failed (%d)\n", (id & 0xFF), err);
        goto exit;
    }

    err = platform_device_add_resources(pdev, &res, 1);
    if (err) {
        pcie_err("Port%u device resource addition failed (%d)\n", (id & 0xFF), err);
        goto exit_device_put;
    }

    err = platform_device_add_data(pdev, &as9947_36xkb_platform_data,
                       sizeof(struct ocores_i2c_platform_data));
    if (err) {
        pcie_err("Port%u platform data allocation failed (%d)\n", (id & 0xFF), err);
        goto exit_device_put;
    }

    err = platform_device_add(pdev);
    if (err) {
        pcie_err("Port%u device addition failed (%d)\n", (id & 0xFF), err);
        goto exit_device_put;
    }

    return pdev;

exit_device_put:
    platform_device_put(pdev);
exit:
    return NULL;
}

static int as9947_36xkb_pcie_fpga_stat_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct as9947_36xkb_fpga_data *fpga_ctl;
    struct pci_dev *pcidev;
    struct resource *ret;
    int i;
    int status = 0, err = 0;
    unsigned long bar_base;

    fpga_ctl = devm_kzalloc(dev, sizeof(struct as9947_36xkb_fpga_data), GFP_KERNEL);
    if (!fpga_ctl) {
        return -ENOMEM;
    }
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
     * Cannot use 'pci_request_regions(pcidev, DRVNAME)'
     * to request all Region 1, Region 2 because another
     * address will be allocated by the i2c-ocores.ko.
     */
    fpga_ctl->pci_fpga_dev.data_base_addr4 = pci_iomap(pcidev, BAR4_NUM, 0);
    if (fpga_ctl->pci_fpga_dev.data_base_addr4 == NULL) {
        dev_err(dev, "Failed to map BAR4\n");
        status = -EIO;
        goto exit_pci_disable;
    }
    fpga_ctl->pci_fpga_dev.data_region4 = pci_resource_start(pcidev, BAR4_NUM) + CPLD1_PCIE_START_OFFSET;
    ret = request_mem_region(fpga_ctl->pci_fpga_dev.data_region4, REGION_LEN, DRVNAME"_cpld1");
    if (ret == NULL) {
        dev_err(dev, "[%s] cannot request region\n", DRVNAME"_cpld1");
        status = -EIO;
        goto exit_pci_iounmap1;
    }
    dev_info(dev, "(BAR%d resource: Start=0x%lx, Length=0x%x)", BAR4_NUM,
                  (unsigned long)fpga_ctl->pci_fpga_dev.data_region4, REGION_LEN);

    fpga_ctl->pci_fpga_dev.data_base_addr5 = pci_iomap(pcidev, BAR5_NUM, 0);
    if (fpga_ctl->pci_fpga_dev.data_base_addr5 == NULL) {
        dev_err(dev, "Failed to map BAR5\n");
        status = -EIO;
        goto exit_pci_release1;
    }
    fpga_ctl->pci_fpga_dev.data_region5 = pci_resource_start(pcidev, BAR5_NUM) + CPLD2_PCIE_START_OFFSET;
    ret = request_mem_region(fpga_ctl->pci_fpga_dev.data_region5, REGION_LEN, DRVNAME"_cpld2");
    if (ret == NULL) {
        dev_err(dev, "[%s] cannot request region\n", DRVNAME"_cpld2");
        status = -EIO;
        goto exit_pci_iounmap2;
    }
    dev_info(dev, "(BAR%d resource: Start=0x%lx, Length=0x%x)", BAR5_NUM,
                  (unsigned long)fpga_ctl->pci_fpga_dev.data_region5, REGION_LEN);

    /* Create I2C ocore devices first, then create the FPGA sysfs.
     * To prevent the application from accessing an ocore device
     * that has not been fully created due to the port status
     * being present.
     */

    /*
     * Create ocore_i2c device for OSFP EEPROM
     */
    for (i = 0; i < PORT_NUM; i++) {
        switch (i)
        {
            case 0 ... 4:
            case 5 ... 23:
                bar_base = pci_resource_start(pcidev, BAR4_NUM);
                break;
            case 24 ... 39:
                bar_base = pci_resource_start(pcidev, BAR5_NUM);
                break;
            default:
                break;
        }
        fpga_ctl->pci_fpga_dev.fpga_i2c[i] =
            ocore_i2c_device_add((i | (port[i].mask << 8)), bar_base, port[i].offset);
        if (IS_ERR(fpga_ctl->pci_fpga_dev.fpga_i2c[i])) {
            status = PTR_ERR(fpga_ctl->pci_fpga_dev.fpga_i2c[i]);
            dev_err(dev, "rc:%d, unload Port%u[0x%ux] device\n",
                         status, i, port[i].offset);
            goto exit_ocores_device;
        }
    }

    return 0;

exit_ocores_device:
    while (i > 0) {
        i--;
        platform_device_unregister(fpga_ctl->pci_fpga_dev.fpga_i2c[i]);
    }
    release_mem_region(fpga_ctl->pci_fpga_dev.data_region5, REGION_LEN);
exit_pci_iounmap2:
    pci_iounmap(fpga_ctl->pci_fpga_dev.pci_dev, fpga_ctl->pci_fpga_dev.data_base_addr5);
exit_pci_release1:
    release_mem_region(fpga_ctl->pci_fpga_dev.data_region4, REGION_LEN);
exit_pci_iounmap1:
    pci_iounmap(fpga_ctl->pci_fpga_dev.pci_dev, fpga_ctl->pci_fpga_dev.data_base_addr4);
exit_pci_disable:
    pci_disable_device(fpga_ctl->pci_fpga_dev.pci_dev);

    return status;
}

static int as9947_36xkb_pcie_fpga_stat_remove(struct platform_device *pdev)
{
    struct as9947_36xkb_fpga_data *fpga_ctl = platform_get_drvdata(pdev);

    if (pci_is_enabled(fpga_ctl->pci_fpga_dev.pci_dev)) {
        int i;
        /* Unregister ocore_i2c device */
        for (i = 0; i < PORT_NUM; i++) {
            platform_device_unregister(fpga_ctl->pci_fpga_dev.fpga_i2c[i]);
        }
        pci_iounmap(fpga_ctl->pci_fpga_dev.pci_dev, fpga_ctl->pci_fpga_dev.data_base_addr4);
        pci_iounmap(fpga_ctl->pci_fpga_dev.pci_dev, fpga_ctl->pci_fpga_dev.data_base_addr5);
        release_mem_region(fpga_ctl->pci_fpga_dev.data_region4, REGION_LEN);
        release_mem_region(fpga_ctl->pci_fpga_dev.data_region5, REGION_LEN);
        pci_disable_device(fpga_ctl->pci_fpga_dev.pci_dev);
    }

    return 0;
}

static struct platform_driver pcie_fpga_port_stat_driver = {
    .probe      = as9947_36xkb_pcie_fpga_stat_probe,
    .remove     = as9947_36xkb_pcie_fpga_stat_remove,
    .driver     = {
        .owner = THIS_MODULE,
        .name  = DRVNAME,
    },
};

static int __init as9947_36xkb_pcie_fpga_init(void)
{
    int status = 0;

    /*
     * Create FPGA platform driver and device
     */
    status = platform_driver_register(&pcie_fpga_port_stat_driver);
    if (status < 0) {
        return status;
    }

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

static void __exit as9947_36xkb_pcie_fpga_exit(void)
{
    platform_device_unregister(pdev);
    platform_driver_unregister(&pcie_fpga_port_stat_driver);
}


module_init(as9947_36xkb_pcie_fpga_init);
module_exit(as9947_36xkb_pcie_fpga_exit);

MODULE_AUTHOR("Willy Liu <willy_liu@accton.com>");
MODULE_DESCRIPTION("AS9947-36XKB FPGA via PCIE");
MODULE_LICENSE("GPL");
