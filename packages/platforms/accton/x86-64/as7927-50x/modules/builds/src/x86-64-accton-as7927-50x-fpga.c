/*
 * Copyright (C)  Willy Liu <willy_liu@accton.com>
 *
 * This module supports the accton fpga via pcie that read/write reg
 * mechanism to get OSFP/SFP status ...etc.
 * This includes the:
 *     Accton as7927_50x FPGA
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
#define DRVNAME                        "as7927_50x_fpga"
#define OCORES_I2C_DRVNAME             "ocores-as7927"
#define PORT_NUM                       50 /* 40 SFP28 + 8 SFP56 + 1 400G OSFP-DD + 1 800G QSFP-DD */
/*
 * PCIE BAR address
 */
#define BAR0_NUM                       0

#define REGION_LEN                     0xFF
#define FPGA_PCI_VENDOR_ID             0x10EE
#define FPGA_PCI_DEVICE_ID             0x7021

#define SPI_BUSY_MASK_CPLD1            0x01
#define SPI_BUSY_MASK_CPLD2            0x02

/***********************************************
 *      FPGA
 * *********************************************/
#define FPGA_PCIE_START_OFFSET         0x0000
#define FPGA_BOARD_INFO_REG            (FPGA_PCIE_START_OFFSET + 0x00)
#define FPGA_MAJOR_VER_REG             (FPGA_PCIE_START_OFFSET + 0x01)
#define FPGA_MINOR_VER_REG             (FPGA_PCIE_START_OFFSET + 0x02)

/***********************************************
 *       CPLD1
 * *********************************************/
#define CPLD1_PCIE_START_OFFSET        0x2000
/* Port CPLD1 VER */
#define CPLD1_MAJOR_VER_REG            (CPLD1_PCIE_START_OFFSET + 0x00)
#define CPLD1_MINOR_VER_REG            (CPLD1_PCIE_START_OFFSET + 0x01)
/* SFP P0-24 RXLOS */
#define XCVR_P7_P0_RXLOS_REG           (CPLD1_PCIE_START_OFFSET + 0x21)
#define XCVR_P15_P8_RXLOS_REG          (CPLD1_PCIE_START_OFFSET + 0x22)
#define XCVR_P23_P16_RXLOS_REG         (CPLD1_PCIE_START_OFFSET + 0x23)
#define XCVR_P24_RXLOS_REG             (CPLD1_PCIE_START_OFFSET + 0x24)
/* SFP P0-24 TX FAULT */
#define XCVR_P7_P0_TXFAULT_REG         (CPLD1_PCIE_START_OFFSET + 0x25)
#define XCVR_P15_P8_TXFAULT_REG        (CPLD1_PCIE_START_OFFSET + 0x26)
#define XCVR_P23_P16_TXFAULT_REG       (CPLD1_PCIE_START_OFFSET + 0x27)
#define XCVR_P24_TXFAULT_REG           (CPLD1_PCIE_START_OFFSET + 0x28)
/* SFP P0-24 TX DISABLE */
#define XCVR_P7_P0_TXDIS_REG           (CPLD1_PCIE_START_OFFSET + 0x29)
#define XCVR_P15_P8_TXDIS_REG          (CPLD1_PCIE_START_OFFSET + 0x2A)
#define XCVR_P23_P16_TXDIS_REG         (CPLD1_PCIE_START_OFFSET + 0x2B)
#define XCVR_P24_TXDIS_REG             (CPLD1_PCIE_START_OFFSET + 0x2C)
/* SFP P0-24 EFUSE */
#define XCVR_P7_P0_EFUSE_REG           (CPLD1_PCIE_START_OFFSET + 0x30)
#define XCVR_P15_P8_EFUSE_REG          (CPLD1_PCIE_START_OFFSET + 0x31)
#define XCVR_P23_P16_EFUSE_REG         (CPLD1_PCIE_START_OFFSET + 0x32)
#define XCVR_P24_EFUSE_REG             (CPLD1_PCIE_START_OFFSET + 0x33)
/* SFP P0-24 MOD */
#define XCVR_P7_P0_PRESENT_REG         (CPLD1_PCIE_START_OFFSET + 0x50)
#define XCVR_P15_P8_PRESENT_REG        (CPLD1_PCIE_START_OFFSET + 0x51)
#define XCVR_P23_P16_PRESENT_REG       (CPLD1_PCIE_START_OFFSET + 0x52)
#define XCVR_P24_PRESENT_REG           (CPLD1_PCIE_START_OFFSET + 0x53)

/***********************************************
 *       CPLD2
 * *********************************************/
#define CPLD2_PCIE_START_OFFSET        0x3000
/* Port CPLD2 VER */
#define CPLD2_MAJOR_VER_REG            (CPLD2_PCIE_START_OFFSET + 0x00)
#define CPLD2_MINOR_VER_REG            (CPLD2_PCIE_START_OFFSET + 0x01)
/* Port 25-47 MOD */
#define XCVR_P32_P25_PRESENT_REG       (CPLD2_PCIE_START_OFFSET + 0x06)
#define XCVR_P40_P33_PRESENT_REG       (CPLD2_PCIE_START_OFFSET + 0x07)
#define XCVR_P47_P41_PRESENT_REG       (CPLD2_PCIE_START_OFFSET + 0x08)
/* Port 48-49 QSFP-DD PRESENT, LP MODE, POWER GOOD */
#define XCVR_P49_P48_QSFPDD_REG        (CPLD2_PCIE_START_OFFSET + 0x12)
/* Port 48-49 QSFP-DD RST */
#define XCVR_P49_P48_QSFPDD_RST_REG     (CPLD2_PCIE_START_OFFSET + 0x13)
/* Port 25-47 EFUSE */
#define XCVR_P32_P25_EFUSE_REG         (CPLD2_PCIE_START_OFFSET + 0x26)
#define XCVR_P40_P33_EFUSE_REG         (CPLD2_PCIE_START_OFFSET + 0x27)
#define XCVR_P47_P41_EFUSE_REG         (CPLD2_PCIE_START_OFFSET + 0x28)
/* Port 48-49 QSFP-DD ENABLE */
#define XCVR_P49_P48_EN_REG            (CPLD2_PCIE_START_OFFSET + 0x29)
/* Port 25-47 RXLOS */
#define XCVR_P32_P25_RXLOS_REG         (CPLD2_PCIE_START_OFFSET + 0x30)
#define XCVR_P40_P33_RXLOS_REG         (CPLD2_PCIE_START_OFFSET + 0x31)
#define XCVR_P47_P41_RXLOS_REG         (CPLD2_PCIE_START_OFFSET + 0x32)
/* Port 25-47 TX FAULT */
#define XCVR_P32_P25_TXFAULT_REG       (CPLD2_PCIE_START_OFFSET + 0x33)
#define XCVR_P40_P33_TXFAULT_REG       (CPLD2_PCIE_START_OFFSET + 0x34)
#define XCVR_P47_P41_TXFAULT_REG       (CPLD2_PCIE_START_OFFSET + 0x35)
/* Port 25-47 TX DISABLE */
#define XCVR_P32_P25_TXDIS_REG         (CPLD2_PCIE_START_OFFSET + 0x40)
#define XCVR_P40_P33_TXDIS_REG         (CPLD2_PCIE_START_OFFSET + 0x41)
#define XCVR_P47_P41_TXDIS_REG         (CPLD2_PCIE_START_OFFSET + 0x42)

#define TRANSCEIVER_PRESENT_ATTR_ID(index)           MODULE_PRESENT_##index
#define TRANSCEIVER_LPMODE_ATTR_ID(index)            MODULE_LPMODE_##index
#define TRANSCEIVER_RESET_ATTR_ID(index)             MODULE_RESET_##index
#define TRANSCEIVER_TX_DISABLE_ATTR_ID(index)        MODULE_TX_DISABLE_##index
#define TRANSCEIVER_TX_FAULT_ATTR_ID(index)          MODULE_TX_FAULT_##index
#define TRANSCEIVER_RX_LOS_ATTR_ID(index)            MODULE_RX_LOS_##index
#define TRANSCEIVER_EFUSE_ATTR_ID(index)             MODULE_EFUSE_##index
#define TRANSCEIVER_ENABLE_ATTR_ID(index)            MODULE_ENABLE_##index

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
    void  __iomem *data_base_addr0;
    resource_size_t data_region0;
    resource_size_t data_region1;
    resource_size_t data_region2;
    struct pci_dev  *pci_dev;
    struct platform_device *fpga_i2c[PORT_NUM];
} pci_fpga_device_t;

/*fpga port status*/
struct as7927_50x_fpga_data {
    u8                  cpld_reg[2];
    unsigned long       last_updated;    /* In jiffies */
    pci_fpga_device_t   pci_fpga_dev;
};

static struct platform_device *pdev = NULL;
extern spinlock_t cpld_access_lock;
extern int wait_spi(u32 mask, unsigned long timeout);

/***********************************************
 *       enum define
 * *********************************************/
 enum fpga_sysfs_attributes {
    /* transceiver attributes */
    TRANSCEIVER_PRESENT_ATTR_ID(1),
    TRANSCEIVER_PRESENT_ATTR_ID(2),
    TRANSCEIVER_PRESENT_ATTR_ID(3),
    TRANSCEIVER_PRESENT_ATTR_ID(4),
    TRANSCEIVER_PRESENT_ATTR_ID(5),
    TRANSCEIVER_PRESENT_ATTR_ID(6),
    TRANSCEIVER_PRESENT_ATTR_ID(7),
    TRANSCEIVER_PRESENT_ATTR_ID(8),
    TRANSCEIVER_PRESENT_ATTR_ID(9),
    TRANSCEIVER_PRESENT_ATTR_ID(10),
    TRANSCEIVER_PRESENT_ATTR_ID(11),
    TRANSCEIVER_PRESENT_ATTR_ID(12),
    TRANSCEIVER_PRESENT_ATTR_ID(13),
    TRANSCEIVER_PRESENT_ATTR_ID(14),
    TRANSCEIVER_PRESENT_ATTR_ID(15),
    TRANSCEIVER_PRESENT_ATTR_ID(16),
    TRANSCEIVER_PRESENT_ATTR_ID(17),
    TRANSCEIVER_PRESENT_ATTR_ID(18),
    TRANSCEIVER_PRESENT_ATTR_ID(19),
    TRANSCEIVER_PRESENT_ATTR_ID(20),
    TRANSCEIVER_PRESENT_ATTR_ID(21),
    TRANSCEIVER_PRESENT_ATTR_ID(22),
    TRANSCEIVER_PRESENT_ATTR_ID(23),
    TRANSCEIVER_PRESENT_ATTR_ID(24),
    TRANSCEIVER_PRESENT_ATTR_ID(25),
    TRANSCEIVER_PRESENT_ATTR_ID(26),
    TRANSCEIVER_PRESENT_ATTR_ID(27),
    TRANSCEIVER_PRESENT_ATTR_ID(28),
    TRANSCEIVER_PRESENT_ATTR_ID(29),
    TRANSCEIVER_PRESENT_ATTR_ID(30),
    TRANSCEIVER_PRESENT_ATTR_ID(31),
    TRANSCEIVER_PRESENT_ATTR_ID(32),
    TRANSCEIVER_PRESENT_ATTR_ID(33),
    TRANSCEIVER_PRESENT_ATTR_ID(34),
    TRANSCEIVER_PRESENT_ATTR_ID(35),
    TRANSCEIVER_PRESENT_ATTR_ID(36),
    TRANSCEIVER_PRESENT_ATTR_ID(37),
    TRANSCEIVER_PRESENT_ATTR_ID(38),
    TRANSCEIVER_PRESENT_ATTR_ID(39),
    TRANSCEIVER_PRESENT_ATTR_ID(40),
    TRANSCEIVER_PRESENT_ATTR_ID(41),
    TRANSCEIVER_PRESENT_ATTR_ID(42),
    TRANSCEIVER_PRESENT_ATTR_ID(43),
    TRANSCEIVER_PRESENT_ATTR_ID(44),
    TRANSCEIVER_PRESENT_ATTR_ID(45),
    TRANSCEIVER_PRESENT_ATTR_ID(46),
    TRANSCEIVER_PRESENT_ATTR_ID(47),
    TRANSCEIVER_PRESENT_ATTR_ID(48),
    TRANSCEIVER_PRESENT_ATTR_ID(49),
    TRANSCEIVER_PRESENT_ATTR_ID(50),
    /*TX DISABLE */
    TRANSCEIVER_TX_DISABLE_ATTR_ID(1),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(2),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(3),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(4),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(5),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(6),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(7),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(8),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(9),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(10),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(11),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(12),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(13),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(14),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(15),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(16),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(17),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(18),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(19),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(20),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(21),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(22),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(23),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(24),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(25),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(26),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(27),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(28),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(29),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(30),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(31),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(32),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(33),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(34),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(35),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(36),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(37),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(38),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(39),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(40),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(41),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(42),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(43),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(44),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(45),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(46),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(47),
    TRANSCEIVER_TX_DISABLE_ATTR_ID(48),
    /* TX FAULT */
    TRANSCEIVER_TX_FAULT_ATTR_ID(1),
    TRANSCEIVER_TX_FAULT_ATTR_ID(2),
    TRANSCEIVER_TX_FAULT_ATTR_ID(3),
    TRANSCEIVER_TX_FAULT_ATTR_ID(4),
    TRANSCEIVER_TX_FAULT_ATTR_ID(5),
    TRANSCEIVER_TX_FAULT_ATTR_ID(6),
    TRANSCEIVER_TX_FAULT_ATTR_ID(7),
    TRANSCEIVER_TX_FAULT_ATTR_ID(8),
    TRANSCEIVER_TX_FAULT_ATTR_ID(9),
    TRANSCEIVER_TX_FAULT_ATTR_ID(10),
    TRANSCEIVER_TX_FAULT_ATTR_ID(11),
    TRANSCEIVER_TX_FAULT_ATTR_ID(12),
    TRANSCEIVER_TX_FAULT_ATTR_ID(13),
    TRANSCEIVER_TX_FAULT_ATTR_ID(14),
    TRANSCEIVER_TX_FAULT_ATTR_ID(15),
    TRANSCEIVER_TX_FAULT_ATTR_ID(16),
    TRANSCEIVER_TX_FAULT_ATTR_ID(17),
    TRANSCEIVER_TX_FAULT_ATTR_ID(18),
    TRANSCEIVER_TX_FAULT_ATTR_ID(19),
    TRANSCEIVER_TX_FAULT_ATTR_ID(20),
    TRANSCEIVER_TX_FAULT_ATTR_ID(21),
    TRANSCEIVER_TX_FAULT_ATTR_ID(22),
    TRANSCEIVER_TX_FAULT_ATTR_ID(23),
    TRANSCEIVER_TX_FAULT_ATTR_ID(24),
    TRANSCEIVER_TX_FAULT_ATTR_ID(25),
    TRANSCEIVER_TX_FAULT_ATTR_ID(26),
    TRANSCEIVER_TX_FAULT_ATTR_ID(27),
    TRANSCEIVER_TX_FAULT_ATTR_ID(28),
    TRANSCEIVER_TX_FAULT_ATTR_ID(29),
    TRANSCEIVER_TX_FAULT_ATTR_ID(30),
    TRANSCEIVER_TX_FAULT_ATTR_ID(31),
    TRANSCEIVER_TX_FAULT_ATTR_ID(32),
    TRANSCEIVER_TX_FAULT_ATTR_ID(33),
    TRANSCEIVER_TX_FAULT_ATTR_ID(34),
    TRANSCEIVER_TX_FAULT_ATTR_ID(35),
    TRANSCEIVER_TX_FAULT_ATTR_ID(36),
    TRANSCEIVER_TX_FAULT_ATTR_ID(37),
    TRANSCEIVER_TX_FAULT_ATTR_ID(38),
    TRANSCEIVER_TX_FAULT_ATTR_ID(39),
    TRANSCEIVER_TX_FAULT_ATTR_ID(40),
    TRANSCEIVER_TX_FAULT_ATTR_ID(41),
    TRANSCEIVER_TX_FAULT_ATTR_ID(42),
    TRANSCEIVER_TX_FAULT_ATTR_ID(43),
    TRANSCEIVER_TX_FAULT_ATTR_ID(44),
    TRANSCEIVER_TX_FAULT_ATTR_ID(45),
    TRANSCEIVER_TX_FAULT_ATTR_ID(46),
    TRANSCEIVER_TX_FAULT_ATTR_ID(47),
    TRANSCEIVER_TX_FAULT_ATTR_ID(48),
    /* RX LOS */
    TRANSCEIVER_RX_LOS_ATTR_ID(1),
    TRANSCEIVER_RX_LOS_ATTR_ID(2),
    TRANSCEIVER_RX_LOS_ATTR_ID(3),
    TRANSCEIVER_RX_LOS_ATTR_ID(4),
    TRANSCEIVER_RX_LOS_ATTR_ID(5),
    TRANSCEIVER_RX_LOS_ATTR_ID(6),
    TRANSCEIVER_RX_LOS_ATTR_ID(7),
    TRANSCEIVER_RX_LOS_ATTR_ID(8),
    TRANSCEIVER_RX_LOS_ATTR_ID(9),
    TRANSCEIVER_RX_LOS_ATTR_ID(10),
    TRANSCEIVER_RX_LOS_ATTR_ID(11),
    TRANSCEIVER_RX_LOS_ATTR_ID(12),
    TRANSCEIVER_RX_LOS_ATTR_ID(13),
    TRANSCEIVER_RX_LOS_ATTR_ID(14),
    TRANSCEIVER_RX_LOS_ATTR_ID(15),
    TRANSCEIVER_RX_LOS_ATTR_ID(16),
    TRANSCEIVER_RX_LOS_ATTR_ID(17),
    TRANSCEIVER_RX_LOS_ATTR_ID(18),
    TRANSCEIVER_RX_LOS_ATTR_ID(19),
    TRANSCEIVER_RX_LOS_ATTR_ID(20),
    TRANSCEIVER_RX_LOS_ATTR_ID(21),
    TRANSCEIVER_RX_LOS_ATTR_ID(22),
    TRANSCEIVER_RX_LOS_ATTR_ID(23),
    TRANSCEIVER_RX_LOS_ATTR_ID(24),
    TRANSCEIVER_RX_LOS_ATTR_ID(25),
    TRANSCEIVER_RX_LOS_ATTR_ID(26),
    TRANSCEIVER_RX_LOS_ATTR_ID(27),
    TRANSCEIVER_RX_LOS_ATTR_ID(28),
    TRANSCEIVER_RX_LOS_ATTR_ID(29),
    TRANSCEIVER_RX_LOS_ATTR_ID(30),
    TRANSCEIVER_RX_LOS_ATTR_ID(31),
    TRANSCEIVER_RX_LOS_ATTR_ID(32),
    TRANSCEIVER_RX_LOS_ATTR_ID(33),
    TRANSCEIVER_RX_LOS_ATTR_ID(34),
    TRANSCEIVER_RX_LOS_ATTR_ID(35),
    TRANSCEIVER_RX_LOS_ATTR_ID(36),
    TRANSCEIVER_RX_LOS_ATTR_ID(37),
    TRANSCEIVER_RX_LOS_ATTR_ID(38),
    TRANSCEIVER_RX_LOS_ATTR_ID(39),
    TRANSCEIVER_RX_LOS_ATTR_ID(40),
    TRANSCEIVER_RX_LOS_ATTR_ID(41),
    TRANSCEIVER_RX_LOS_ATTR_ID(42),
    TRANSCEIVER_RX_LOS_ATTR_ID(43),
    TRANSCEIVER_RX_LOS_ATTR_ID(44),
    TRANSCEIVER_RX_LOS_ATTR_ID(45),
    TRANSCEIVER_RX_LOS_ATTR_ID(46),
    TRANSCEIVER_RX_LOS_ATTR_ID(47),
    TRANSCEIVER_RX_LOS_ATTR_ID(48),
    /* EFUSE */
    TRANSCEIVER_EFUSE_ATTR_ID(1),
    TRANSCEIVER_EFUSE_ATTR_ID(2),
    TRANSCEIVER_EFUSE_ATTR_ID(3),
    TRANSCEIVER_EFUSE_ATTR_ID(4),
    TRANSCEIVER_EFUSE_ATTR_ID(5),
    TRANSCEIVER_EFUSE_ATTR_ID(6),
    TRANSCEIVER_EFUSE_ATTR_ID(7),
    TRANSCEIVER_EFUSE_ATTR_ID(8),
    TRANSCEIVER_EFUSE_ATTR_ID(9),
    TRANSCEIVER_EFUSE_ATTR_ID(10),
    TRANSCEIVER_EFUSE_ATTR_ID(11),
    TRANSCEIVER_EFUSE_ATTR_ID(12),
    TRANSCEIVER_EFUSE_ATTR_ID(13),
    TRANSCEIVER_EFUSE_ATTR_ID(14),
    TRANSCEIVER_EFUSE_ATTR_ID(15),
    TRANSCEIVER_EFUSE_ATTR_ID(16),
    TRANSCEIVER_EFUSE_ATTR_ID(17),
    TRANSCEIVER_EFUSE_ATTR_ID(18),
    TRANSCEIVER_EFUSE_ATTR_ID(19),
    TRANSCEIVER_EFUSE_ATTR_ID(20),
    TRANSCEIVER_EFUSE_ATTR_ID(21),
    TRANSCEIVER_EFUSE_ATTR_ID(22),
    TRANSCEIVER_EFUSE_ATTR_ID(23),
    TRANSCEIVER_EFUSE_ATTR_ID(24),
    TRANSCEIVER_EFUSE_ATTR_ID(25),
    TRANSCEIVER_EFUSE_ATTR_ID(26),
    TRANSCEIVER_EFUSE_ATTR_ID(27),
    TRANSCEIVER_EFUSE_ATTR_ID(28),
    TRANSCEIVER_EFUSE_ATTR_ID(29),
    TRANSCEIVER_EFUSE_ATTR_ID(30),
    TRANSCEIVER_EFUSE_ATTR_ID(31),
    TRANSCEIVER_EFUSE_ATTR_ID(32),
    TRANSCEIVER_EFUSE_ATTR_ID(33),
    TRANSCEIVER_EFUSE_ATTR_ID(34),
    TRANSCEIVER_EFUSE_ATTR_ID(35),
    TRANSCEIVER_EFUSE_ATTR_ID(36),
    TRANSCEIVER_EFUSE_ATTR_ID(37),
    TRANSCEIVER_EFUSE_ATTR_ID(38),
    TRANSCEIVER_EFUSE_ATTR_ID(39),
    TRANSCEIVER_EFUSE_ATTR_ID(40),
    TRANSCEIVER_EFUSE_ATTR_ID(41),
    TRANSCEIVER_EFUSE_ATTR_ID(42),
    TRANSCEIVER_EFUSE_ATTR_ID(43),
    TRANSCEIVER_EFUSE_ATTR_ID(44),
    TRANSCEIVER_EFUSE_ATTR_ID(45),
    TRANSCEIVER_EFUSE_ATTR_ID(46),
    TRANSCEIVER_EFUSE_ATTR_ID(47),
    TRANSCEIVER_EFUSE_ATTR_ID(48),
    /* QSFP-DD LP MODE and RESET */
    TRANSCEIVER_ENABLE_ATTR_ID(49),
    TRANSCEIVER_ENABLE_ATTR_ID(50),
    TRANSCEIVER_RESET_ATTR_ID(49),
    TRANSCEIVER_RESET_ATTR_ID(50),
    TRANSCEIVER_LPMODE_ATTR_ID(49),
    TRANSCEIVER_LPMODE_ATTR_ID(50),
    FPGA_VERSION,
    CPLD1_VERSION,
    CPLD2_VERSION,
};

/***********************************************
 *       function declare
 * *********************************************/
static ssize_t status_read(struct device *dev, struct device_attribute *da,
             char *buf);
static ssize_t status_write(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);

#define DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
        static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, status_read, NULL, MODULE_PRESENT_##index); \
        static SENSOR_DEVICE_ATTR(module_rx_los_##index, S_IRUGO, status_read, NULL, MODULE_RX_LOS_##index); \
        static SENSOR_DEVICE_ATTR(module_tx_fault_##index, S_IRUGO, status_read, NULL, MODULE_TX_FAULT_##index); \
        static SENSOR_DEVICE_ATTR(module_tx_disable_##index, S_IRUGO|S_IWUSR, status_read, status_write, MODULE_TX_DISABLE_##index); \
        static SENSOR_DEVICE_ATTR(module_efuse_##index, S_IRUGO|S_IWUSR, status_read, status_write, MODULE_EFUSE_##index)
#define DECLARE_TRANSCEIVER_ATTR(index) \
        &sensor_dev_attr_module_present_##index.dev_attr.attr, \
        &sensor_dev_attr_module_rx_los_##index.dev_attr.attr, \
        &sensor_dev_attr_module_tx_fault_##index.dev_attr.attr, \
        &sensor_dev_attr_module_tx_disable_##index.dev_attr.attr, \
        &sensor_dev_attr_module_efuse_##index.dev_attr.attr

/* sfp 1-48 transceiver attributes */
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(1);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(2);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(3);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(4);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(5);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(6);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(7);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(8);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(9);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(10);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(11);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(12);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(13);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(14);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(15);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(16);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(17);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(18);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(19);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(20);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(21);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(22);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(23);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(24);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(25);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(26);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(27);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(28);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(29);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(30);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(31);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(32);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(33);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(34);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(35);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(36);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(37);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(38);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(39);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(40);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(41);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(42);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(43);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(44);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(45);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(46);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(47);
DECLARE_TRANSCEIVER_SENSOR_DEVICE_ATTR(48);
/* QSFP-DD 49, 50 */
static SENSOR_DEVICE_ATTR(module_enable_49, S_IRUGO|S_IWUSR, status_read, status_write, MODULE_ENABLE_49);
static SENSOR_DEVICE_ATTR(module_enable_50, S_IRUGO|S_IWUSR, status_read, status_write, MODULE_ENABLE_50);
static SENSOR_DEVICE_ATTR(module_present_49, S_IRUGO, status_read, NULL, MODULE_PRESENT_49);
static SENSOR_DEVICE_ATTR(module_present_50, S_IRUGO, status_read, NULL, MODULE_PRESENT_50);
static SENSOR_DEVICE_ATTR(module_reset_49, S_IRUGO|S_IWUSR, status_read, status_write, MODULE_RESET_49);
static SENSOR_DEVICE_ATTR(module_reset_50, S_IRUGO|S_IWUSR, status_read, status_write, MODULE_RESET_50);
static SENSOR_DEVICE_ATTR(module_lpmode_49, S_IRUGO|S_IWUSR, status_read, status_write, MODULE_LPMODE_49);
static SENSOR_DEVICE_ATTR(module_lpmode_50, S_IRUGO|S_IWUSR, status_read, status_write, MODULE_LPMODE_50);
/* VERSION */
static SENSOR_DEVICE_ATTR(fpga_version, S_IRUGO, status_read, NULL, FPGA_VERSION);
static SENSOR_DEVICE_ATTR(cpld1_version, S_IRUGO, status_read, NULL, CPLD1_VERSION);
static SENSOR_DEVICE_ATTR(cpld2_version, S_IRUGO, status_read, NULL, CPLD2_VERSION);
static struct attribute *fpga_transceiver_attributes[] = {
    DECLARE_TRANSCEIVER_ATTR(1),
    DECLARE_TRANSCEIVER_ATTR(2),
    DECLARE_TRANSCEIVER_ATTR(3),
    DECLARE_TRANSCEIVER_ATTR(4),
    DECLARE_TRANSCEIVER_ATTR(5),
    DECLARE_TRANSCEIVER_ATTR(6),
    DECLARE_TRANSCEIVER_ATTR(7),
    DECLARE_TRANSCEIVER_ATTR(8),
    DECLARE_TRANSCEIVER_ATTR(9),
    DECLARE_TRANSCEIVER_ATTR(10),
    DECLARE_TRANSCEIVER_ATTR(11),
    DECLARE_TRANSCEIVER_ATTR(12),
    DECLARE_TRANSCEIVER_ATTR(13),
    DECLARE_TRANSCEIVER_ATTR(14),
    DECLARE_TRANSCEIVER_ATTR(15),
    DECLARE_TRANSCEIVER_ATTR(16),
    DECLARE_TRANSCEIVER_ATTR(17),
    DECLARE_TRANSCEIVER_ATTR(18),
    DECLARE_TRANSCEIVER_ATTR(19),
    DECLARE_TRANSCEIVER_ATTR(20),
    DECLARE_TRANSCEIVER_ATTR(21),
    DECLARE_TRANSCEIVER_ATTR(22),
    DECLARE_TRANSCEIVER_ATTR(23),
    DECLARE_TRANSCEIVER_ATTR(24),
    DECLARE_TRANSCEIVER_ATTR(25),
    DECLARE_TRANSCEIVER_ATTR(26),
    DECLARE_TRANSCEIVER_ATTR(27),
    DECLARE_TRANSCEIVER_ATTR(28),
    DECLARE_TRANSCEIVER_ATTR(29),
    DECLARE_TRANSCEIVER_ATTR(30),
    DECLARE_TRANSCEIVER_ATTR(31),
    DECLARE_TRANSCEIVER_ATTR(32),
    DECLARE_TRANSCEIVER_ATTR(33),
    DECLARE_TRANSCEIVER_ATTR(34),
    DECLARE_TRANSCEIVER_ATTR(35),
    DECLARE_TRANSCEIVER_ATTR(36),
    DECLARE_TRANSCEIVER_ATTR(37),
    DECLARE_TRANSCEIVER_ATTR(38),
    DECLARE_TRANSCEIVER_ATTR(39),
    DECLARE_TRANSCEIVER_ATTR(40),
    DECLARE_TRANSCEIVER_ATTR(41),
    DECLARE_TRANSCEIVER_ATTR(42),
    DECLARE_TRANSCEIVER_ATTR(43),
    DECLARE_TRANSCEIVER_ATTR(44),
    DECLARE_TRANSCEIVER_ATTR(45),
    DECLARE_TRANSCEIVER_ATTR(46),
    DECLARE_TRANSCEIVER_ATTR(47),
    DECLARE_TRANSCEIVER_ATTR(48),
    &sensor_dev_attr_module_enable_49.dev_attr.attr,
    &sensor_dev_attr_module_enable_50.dev_attr.attr,
    &sensor_dev_attr_module_present_49.dev_attr.attr,
    &sensor_dev_attr_module_present_50.dev_attr.attr,
    &sensor_dev_attr_module_reset_49.dev_attr.attr,
    &sensor_dev_attr_module_reset_50.dev_attr.attr,
    &sensor_dev_attr_module_lpmode_49.dev_attr.attr,
    &sensor_dev_attr_module_lpmode_50.dev_attr.attr,
    &sensor_dev_attr_fpga_version.dev_attr.attr,
    &sensor_dev_attr_cpld1_version.dev_attr.attr,
    &sensor_dev_attr_cpld2_version.dev_attr.attr,
    NULL
};

static const struct attribute_group fpga_port_stat_group = {
    .attrs = fpga_transceiver_attributes,
};

struct attribute_mapping {
    u16 attr_base;
    u16 reg;
    u8 revert;
};

/* Define an array of attribute mappings */
static struct attribute_mapping attribute_mappings[] = {
    [MODULE_PRESENT_1 ... MODULE_PRESENT_8] = {MODULE_PRESENT_1, XCVR_P7_P0_PRESENT_REG, 1},
    [MODULE_PRESENT_9 ... MODULE_PRESENT_16] = {MODULE_PRESENT_9, XCVR_P15_P8_PRESENT_REG, 1},
    [MODULE_PRESENT_17 ... MODULE_PRESENT_24] = {MODULE_PRESENT_17, XCVR_P23_P16_PRESENT_REG, 1},
    [MODULE_PRESENT_25] = {MODULE_PRESENT_25, XCVR_P24_PRESENT_REG, 1},
    [MODULE_PRESENT_26 ... MODULE_PRESENT_33] = {MODULE_PRESENT_26, XCVR_P32_P25_PRESENT_REG, 1},
    [MODULE_PRESENT_34 ... MODULE_PRESENT_41] = {MODULE_PRESENT_34, XCVR_P40_P33_PRESENT_REG, 1},
    [MODULE_PRESENT_42 ... MODULE_PRESENT_48] = {MODULE_PRESENT_42, XCVR_P47_P41_PRESENT_REG, 1},

    [MODULE_RX_LOS_1 ... MODULE_RX_LOS_8] = {MODULE_RX_LOS_1, XCVR_P7_P0_RXLOS_REG, 0},
    [MODULE_RX_LOS_9 ... MODULE_RX_LOS_16] = {MODULE_RX_LOS_9, XCVR_P15_P8_RXLOS_REG, 0},
    [MODULE_RX_LOS_17 ... MODULE_RX_LOS_24] = {MODULE_RX_LOS_17, XCVR_P23_P16_RXLOS_REG, 0},
    [MODULE_RX_LOS_25] = {MODULE_RX_LOS_25, XCVR_P24_RXLOS_REG, 0},
    [MODULE_RX_LOS_26 ... MODULE_RX_LOS_33] = {MODULE_RX_LOS_26, XCVR_P32_P25_RXLOS_REG, 0},
    [MODULE_RX_LOS_34 ... MODULE_RX_LOS_41] = {MODULE_RX_LOS_34, XCVR_P40_P33_RXLOS_REG, 0},
    [MODULE_RX_LOS_42 ... MODULE_RX_LOS_48] = {MODULE_RX_LOS_42, XCVR_P47_P41_RXLOS_REG, 0},

    [MODULE_TX_FAULT_1 ... MODULE_TX_FAULT_8] = {MODULE_TX_FAULT_1, XCVR_P7_P0_TXFAULT_REG, 0},
    [MODULE_TX_FAULT_9 ... MODULE_TX_FAULT_16] = {MODULE_TX_FAULT_9, XCVR_P15_P8_TXFAULT_REG, 0},
    [MODULE_TX_FAULT_17 ... MODULE_TX_FAULT_24] = {MODULE_TX_FAULT_17, XCVR_P23_P16_TXFAULT_REG, 0},
    [MODULE_TX_FAULT_25] = {MODULE_TX_FAULT_25, XCVR_P24_TXFAULT_REG, 0},
    [MODULE_TX_FAULT_26 ... MODULE_TX_FAULT_33] = {MODULE_TX_FAULT_26, XCVR_P32_P25_TXFAULT_REG, 0},
    [MODULE_TX_FAULT_34 ... MODULE_TX_FAULT_41] = {MODULE_TX_FAULT_34, XCVR_P40_P33_TXFAULT_REG, 0},
    [MODULE_TX_FAULT_42 ... MODULE_TX_FAULT_48] = {MODULE_TX_FAULT_42, XCVR_P47_P41_TXFAULT_REG, 0},

    [MODULE_TX_DISABLE_1 ... MODULE_TX_DISABLE_8] = {MODULE_TX_DISABLE_1, XCVR_P7_P0_TXDIS_REG, 0},
    [MODULE_TX_DISABLE_9 ... MODULE_TX_DISABLE_16] = {MODULE_TX_DISABLE_9, XCVR_P15_P8_TXDIS_REG, 0},
    [MODULE_TX_DISABLE_17 ... MODULE_TX_DISABLE_24] = {MODULE_TX_DISABLE_17, XCVR_P23_P16_TXDIS_REG, 0},
    [MODULE_TX_DISABLE_25] = {MODULE_TX_DISABLE_25, XCVR_P24_TXDIS_REG, 0},
    [MODULE_TX_DISABLE_26 ... MODULE_TX_DISABLE_33] = {MODULE_TX_DISABLE_26, XCVR_P32_P25_TXDIS_REG, 0},
    [MODULE_TX_DISABLE_34 ... MODULE_TX_DISABLE_41] = {MODULE_TX_DISABLE_34, XCVR_P40_P33_TXDIS_REG, 0},
    [MODULE_TX_DISABLE_42 ... MODULE_TX_DISABLE_48] = {MODULE_TX_DISABLE_42, XCVR_P47_P41_TXDIS_REG, 0},

    [MODULE_EFUSE_1 ... MODULE_EFUSE_8] = {MODULE_EFUSE_1, XCVR_P7_P0_EFUSE_REG, 1},
    [MODULE_EFUSE_9 ... MODULE_EFUSE_16] = {MODULE_EFUSE_9, XCVR_P15_P8_EFUSE_REG, 1},
    [MODULE_EFUSE_17 ... MODULE_EFUSE_24] = {MODULE_EFUSE_17, XCVR_P23_P16_EFUSE_REG, 1},
    [MODULE_EFUSE_25] = {MODULE_EFUSE_25, XCVR_P24_EFUSE_REG, 1},
    [MODULE_EFUSE_26 ... MODULE_EFUSE_33] = {MODULE_EFUSE_26, XCVR_P32_P25_EFUSE_REG, 1},
    [MODULE_EFUSE_34 ... MODULE_EFUSE_41] = {MODULE_EFUSE_34, XCVR_P40_P33_EFUSE_REG, 1},
    [MODULE_EFUSE_42 ... MODULE_EFUSE_48] = {MODULE_EFUSE_42, XCVR_P47_P41_EFUSE_REG, 1},

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

static ssize_t status_read(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct as7927_50x_fpga_data *fpga_ctl = dev_get_drvdata(dev);
    ssize_t ret = -EINVAL;
    u16 reg;
    u8 major, minor, reg_val;
    u8 bits_shift;

    switch(attr->index)
    {
        case FPGA_VERSION:
            major = ioread8(fpga_ctl->pci_fpga_dev.data_base_addr0 + FPGA_MAJOR_VER_REG);
            minor = ioread8(fpga_ctl->pci_fpga_dev.data_base_addr0 + FPGA_MINOR_VER_REG);
            ret = sprintf(buf, "%x.%x\n", major, minor);
            break;
        case CPLD1_VERSION:
            LOCK(&cpld_access_lock);
            reg = CPLD1_MAJOR_VER_REG;
            major = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + reg,
                              SPI_BUSY_MASK_CPLD1);
            reg = CPLD1_MINOR_VER_REG;
            minor = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + reg,
                              SPI_BUSY_MASK_CPLD1);
            UNLOCK(&cpld_access_lock);

            ret = sprintf(buf, "%x.%X\n", major, minor);
            break;
        case CPLD2_VERSION:
            LOCK(&cpld_access_lock);
            reg = CPLD2_MAJOR_VER_REG;
            major = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + reg,
                              SPI_BUSY_MASK_CPLD2);
            reg = CPLD2_MINOR_VER_REG;
            minor = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + reg,
                              SPI_BUSY_MASK_CPLD2);
            UNLOCK(&cpld_access_lock);

            ret = sprintf(buf, "%x.%x\n", major, minor);
            break;
        case MODULE_PRESENT_1 ... MODULE_PRESENT_48:
        case MODULE_RX_LOS_1 ... MODULE_RX_LOS_48:
        case MODULE_TX_FAULT_1 ... MODULE_TX_FAULT_48:
        case MODULE_TX_DISABLE_1 ... MODULE_TX_DISABLE_48:
        case MODULE_EFUSE_1 ... MODULE_EFUSE_48:
            reg = attribute_mappings[attr->index].reg;
            LOCK(&cpld_access_lock);
            if ((reg & 0xF000) == CPLD1_PCIE_START_OFFSET) {
                reg_val = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + reg,
                                    SPI_BUSY_MASK_CPLD1);
            } else if ((reg & 0xF000) == CPLD2_PCIE_START_OFFSET) {
                reg_val = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + reg,
                                    SPI_BUSY_MASK_CPLD2);
            }
            UNLOCK(&cpld_access_lock);

            bits_shift = attr->index - attribute_mappings[attr->index].attr_base;
            reg_val = (reg_val >> bits_shift) & 0x01;
            if (attribute_mappings[attr->index].revert) {
                reg_val = !reg_val;
            }
            ret = sprintf(buf, "%u\n", reg_val);
            break;
        case MODULE_PRESENT_49 ... MODULE_PRESENT_50:
            LOCK(&cpld_access_lock);
            reg_val = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + XCVR_P49_P48_QSFPDD_REG, SPI_BUSY_MASK_CPLD2);
            UNLOCK(&cpld_access_lock);
            bits_shift = attr->index - MODULE_PRESENT_49;
            reg_val = !((reg_val >> (4 + bits_shift)) & 0x01);
            ret = sprintf(buf, "%u\n", reg_val);
            break;
        case MODULE_LPMODE_49 ... MODULE_LPMODE_50:
            LOCK(&cpld_access_lock);
            reg_val = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + XCVR_P49_P48_QSFPDD_REG, SPI_BUSY_MASK_CPLD2);
            UNLOCK(&cpld_access_lock);
            bits_shift = attr->index - MODULE_LPMODE_49;
            reg_val = (reg_val >> (2 + bits_shift)) & 0x01;
            ret = sprintf(buf, "%u\n", reg_val);
            break;
        case MODULE_RESET_49 ... MODULE_RESET_50:
            LOCK(&cpld_access_lock);
            reg_val = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + XCVR_P49_P48_QSFPDD_RST_REG, SPI_BUSY_MASK_CPLD2);
            UNLOCK(&cpld_access_lock);
            bits_shift = attr->index - MODULE_RESET_49;
            reg_val = !((reg_val >> (bits_shift)) & 0x01);
            ret = sprintf(buf, "%u\n", reg_val);
            break;
        case MODULE_ENABLE_49 ... MODULE_ENABLE_50:
            LOCK(&cpld_access_lock);
            reg_val = fpga_read(fpga_ctl->pci_fpga_dev.data_base_addr0 + XCVR_P49_P48_EN_REG, SPI_BUSY_MASK_CPLD2);
            UNLOCK(&cpld_access_lock);
            bits_shift = attr->index - MODULE_ENABLE_49;
            reg_val = (reg_val >> bits_shift) & 0x01;
            ret = sprintf(buf, "%u\n", reg_val);
            break;
        default:
            break;
    }

    return ret;
}

static ssize_t status_write(struct device *dev, struct device_attribute *da,
                            const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct as7927_50x_fpga_data *fpga_ctl = dev_get_drvdata(dev);
    void __iomem *addr;
    int status;
    u16 reg;
    u8 input;
    u8 reg_val, bit_mask, should_set_bit;
    u32 spi_mask;

    status = kstrtou8(buf, 10, &input);
    if (status) {
        return status;
    }

    addr = fpga_ctl->pci_fpga_dev.data_base_addr0;
    switch(attr->index)
    {

        case MODULE_EFUSE_1 ... MODULE_EFUSE_48:
            reg = attribute_mappings[attr->index].reg;
            if ((reg & 0xF000) == CPLD1_PCIE_START_OFFSET) {
                spi_mask = SPI_BUSY_MASK_CPLD1;
            } else if ((reg & 0xF000) == CPLD2_PCIE_START_OFFSET) {
                spi_mask = SPI_BUSY_MASK_CPLD2;
            }

            bit_mask = 0x01 << (attr->index - attribute_mappings[attr->index].attr_base);
            should_set_bit = attribute_mappings[attr->index].revert ? !input : input;

            LOCK(&cpld_access_lock);
            reg_val = fpga_read(addr + reg, spi_mask);
            if (should_set_bit) {
                reg_val |= bit_mask;
            } else {
                reg_val &= ~bit_mask;
            }
            fpga_write(addr + reg, reg_val, spi_mask);
            UNLOCK(&cpld_access_lock);
            break;
        case MODULE_TX_DISABLE_1 ... MODULE_TX_DISABLE_48:
            reg = attribute_mappings[attr->index].reg;
            if ((reg & 0xF000) == CPLD1_PCIE_START_OFFSET) {
                spi_mask = SPI_BUSY_MASK_CPLD1;
            } else if ((reg & 0xF000) == CPLD2_PCIE_START_OFFSET) {
                spi_mask = SPI_BUSY_MASK_CPLD2;
            }

            bit_mask = 0x01 << (attr->index - attribute_mappings[attr->index].attr_base);
            should_set_bit = attribute_mappings[attr->index].revert ? !input : input;

            LOCK(&cpld_access_lock);
            reg_val = fpga_read(addr + reg, spi_mask);
            if (should_set_bit) {
                reg_val |= bit_mask;
            } else {
                reg_val &= ~bit_mask;
            }
            fpga_write(addr + reg, reg_val, spi_mask);
            UNLOCK(&cpld_access_lock);
            break;
        case MODULE_LPMODE_49 ... MODULE_LPMODE_50:
            LOCK(&cpld_access_lock);
            reg = XCVR_P49_P48_QSFPDD_REG;
            spi_mask = SPI_BUSY_MASK_CPLD2;
            reg_val = fpga_read(addr + reg, spi_mask);
            bit_mask = 0x01 << (attr->index - MODULE_LPMODE_49 + 2);
            should_set_bit = input;
            if (should_set_bit) {
                reg_val |= bit_mask;
            } else {
                reg_val &= ~bit_mask;
            }
            fpga_write(addr + reg, reg_val, spi_mask);
            UNLOCK(&cpld_access_lock);
            break;
        case MODULE_RESET_49 ... MODULE_RESET_50:
            LOCK(&cpld_access_lock);
            reg = XCVR_P49_P48_QSFPDD_RST_REG;
            spi_mask = SPI_BUSY_MASK_CPLD2;
            reg_val = fpga_read(addr + reg, spi_mask);
            bit_mask = 0x01 << (attr->index - MODULE_RESET_49);
            should_set_bit = !input;
            if (should_set_bit) {
                reg_val |= bit_mask;
            } else {
                reg_val &= ~bit_mask;
            }
            fpga_write(addr + reg, reg_val, spi_mask);
            UNLOCK(&cpld_access_lock);
            break;
        case MODULE_ENABLE_49 ... MODULE_ENABLE_50:
            LOCK(&cpld_access_lock);
            reg = XCVR_P49_P48_EN_REG;
            spi_mask = SPI_BUSY_MASK_CPLD2;
            reg_val = fpga_read(addr + reg, spi_mask);
            bit_mask = 0x01 << (attr->index - MODULE_ENABLE_49);
            should_set_bit = input;
            if (should_set_bit) {
                reg_val |= bit_mask;
            } else {
                reg_val &= ~bit_mask;
            }
            fpga_write(addr + reg, reg_val, spi_mask);
            UNLOCK(&cpld_access_lock);
            break;
        default:
            break;
    }

    return count;
}

struct _port_data {
    u16 offset;
    u16 mask; /* SPI Busy mask : 0x01 --> CPLD1, 0x02 --> CPLD2 */
};
/* ============PCIe Bar Offset to I2C Master Mapping============== */
static const struct _port_data port[PORT_NUM]= {
    /* CPLD1 I2C SFP28 port 1-24 */
    {0x2100, SPI_BUSY_MASK_CPLD1},/* 0x2100 - 0x2110  CPLD1 I2C Master SFP28 Port0 */
    {0x2120, SPI_BUSY_MASK_CPLD1},/* 0x2120 - 0x2130  CPLD1 I2C Master SFP28 Port1 */
    {0x2140, SPI_BUSY_MASK_CPLD1},/* 0x2140 - 0x2150  CPLD1 I2C Master SFP28 Port2 */
    {0x2160, SPI_BUSY_MASK_CPLD1},/* 0x2160 - 0x2170  CPLD1 I2C Master SFP28 Port3 */
    {0x2180, SPI_BUSY_MASK_CPLD1},/* 0x2180 - 0x2190  CPLD1 I2C Master SFP28 Port4 */
    {0x21A0, SPI_BUSY_MASK_CPLD1},/* 0x21A0 - 0x21B0  CPLD1 I2C Master SFP28 Port5 */
    {0x21C0, SPI_BUSY_MASK_CPLD1},/* 0x21C0 - 0x21D0  CPLD1 I2C Master SFP28 Port6 */
    {0x21E0, SPI_BUSY_MASK_CPLD1},/* 0x21E0 - 0x21F0  CPLD1 I2C Master SFP28 Port7 */
    {0x2200, SPI_BUSY_MASK_CPLD1},/* 0x2200 - 0x2210  CPLD1 I2C Master SFP28 Port8 */
    {0x2220, SPI_BUSY_MASK_CPLD1},/* 0x2220 - 0x2230  CPLD1 I2C Master SFP28 Port9 */
    {0x2240, SPI_BUSY_MASK_CPLD1},/* 0x2240 - 0x2250  CPLD1 I2C Master SFP28 Port10 */
    {0x2260, SPI_BUSY_MASK_CPLD1},/* 0x2260 - 0x2270  CPLD1 I2C Master SFP28 Port11 */
    {0x2280, SPI_BUSY_MASK_CPLD1},/* 0x2280 - 0x2290  CPLD1 I2C Master SFP28 Port12 */
    {0x22A0, SPI_BUSY_MASK_CPLD1},/* 0x22A0 - 0x22B0  CPLD1 I2C Master SFP28 Port13 */
    {0x22C0, SPI_BUSY_MASK_CPLD1},/* 0x22C0 - 0x22D0  CPLD1 I2C Master SFP28 Port14 */
    {0x22E0, SPI_BUSY_MASK_CPLD1},/* 0x22E0 - 0x22F0  CPLD1 I2C Master SFP28 Port15 */
    {0x2300, SPI_BUSY_MASK_CPLD1},/* 0x2300 - 0x2310  CPLD1 I2C Master SFP28 Port16 */
    {0x2320, SPI_BUSY_MASK_CPLD1},/* 0x2320 - 0x2330  CPLD1 I2C Master SFP28 Port17 */
    {0x2340, SPI_BUSY_MASK_CPLD1},/* 0x2340 - 0x2350  CPLD1 I2C Master SFP28 Port18 */
    {0x2360, SPI_BUSY_MASK_CPLD1},/* 0x2360 - 0x2370  CPLD1 I2C Master SFP28 Port19 */
    {0x2380, SPI_BUSY_MASK_CPLD1},/* 0x2380 - 0x2390  CPLD1 I2C Master SFP28 Port20 */
    {0x23A0, SPI_BUSY_MASK_CPLD1},/* 0x23A0 - 0x23B0  CPLD1 I2C Master SFP28 Port21 */
    {0x23C0, SPI_BUSY_MASK_CPLD1},/* 0x23C0 - 0x23D0  CPLD1 I2C Master SFP28 Port22 */
    {0x23E0, SPI_BUSY_MASK_CPLD1},/* 0x23E0 - 0x23F0  CPLD1 I2C Master SFP28 Port23 */
    {0x2400, SPI_BUSY_MASK_CPLD1},/* 0x2400 - 0x2410  CPLD1 I2C Master SFP28 Port24 */
    /* CPLD2 I2C SFP28 Port25-49 */
    {0x3100, SPI_BUSY_MASK_CPLD2},/* 0x3100 - 0x3110  CPLD1 I2C Master SFP28 Port25 */
    {0x3120, SPI_BUSY_MASK_CPLD2},/* 0x3120 - 0x3130  CPLD1 I2C Master SFP28 Port26 */
    {0x3140, SPI_BUSY_MASK_CPLD2},/* 0x3140 - 0x3150  CPLD1 I2C Master SFP28 Port27 */
    {0x3160, SPI_BUSY_MASK_CPLD2},/* 0x3160 - 0x3170  CPLD1 I2C Master SFP28 Port28 */
    {0x3180, SPI_BUSY_MASK_CPLD2},/* 0x3180 - 0x3190  CPLD1 I2C Master SFP28 Port29 */
    {0x31A0, SPI_BUSY_MASK_CPLD2},/* 0x31A0 - 0x31B0  CPLD1 I2C Master SFP28 Port30 */
    {0x31C0, SPI_BUSY_MASK_CPLD2},/* 0x31C0 - 0x31D0  CPLD1 I2C Master SFP28 Port31 */
    {0x31E0, SPI_BUSY_MASK_CPLD2},/* 0x31E0 - 0x31F0  CPLD1 I2C Master SFP28 Port32 */
    {0x3200, SPI_BUSY_MASK_CPLD2},/* 0x3200 - 0x3210  CPLD1 I2C Master SFP28 Port33 */
    {0x3220, SPI_BUSY_MASK_CPLD2},/* 0x3220 - 0x3230  CPLD1 I2C Master SFP28 Port34 */
    {0x3240, SPI_BUSY_MASK_CPLD2},/* 0x3240 - 0x3250  CPLD1 I2C Master SFP28 Port35 */
    {0x3260, SPI_BUSY_MASK_CPLD2},/* 0x3260 - 0x3270  CPLD1 I2C Master SFP28 Port36 */
    {0x3280, SPI_BUSY_MASK_CPLD2},/* 0x3280 - 0x3290  CPLD1 I2C Master SFP28 Port37 */
    {0x32A0, SPI_BUSY_MASK_CPLD2},/* 0x32A0 - 0x32B0  CPLD1 I2C Master SFP28 Port38 */
    {0x32C0, SPI_BUSY_MASK_CPLD2},/* 0x32C0 - 0x32D0  CPLD1 I2C Master SFP28 Port39 */
    {0x32E0, SPI_BUSY_MASK_CPLD2},/* 0x32E0 - 0x32F0  CPLD1 I2C Master SFP28 Port40 */
    {0x3300, SPI_BUSY_MASK_CPLD2},/* 0x3300 - 0x3310  CPLD1 I2C Master SFP28 Port41 */
    {0x3320, SPI_BUSY_MASK_CPLD2},/* 0x3320 - 0x3330  CPLD1 I2C Master SFP28 Port42 */
    {0x3340, SPI_BUSY_MASK_CPLD2},/* 0x3340 - 0x3350  CPLD1 I2C Master SFP28 Port43 */
    {0x3360, SPI_BUSY_MASK_CPLD2},/* 0x3360 - 0x3370  CPLD1 I2C Master SFP28 Port44 */
    {0x3380, SPI_BUSY_MASK_CPLD2},/* 0x3380 - 0x3390  CPLD1 I2C Master SFP28 Port45 */
    {0x33A0, SPI_BUSY_MASK_CPLD2},/* 0x33A0 - 0x33B0  CPLD1 I2C Master SFP28 Port46 */
    {0x33C0, SPI_BUSY_MASK_CPLD2},/* 0x33C0 - 0x33D0  CPLD1 I2C Master SFP28 Port47 */
    {0x3400, SPI_BUSY_MASK_CPLD2},/* 0x3400 - 0x3410  CPLD1 I2C Master SFP28 Port48 */
    {0x3420, SPI_BUSY_MASK_CPLD2},/* 0x3420 - 0x3430  CPLD1 I2C Master SFP28 Port49 */
};

static struct ocores_i2c_platform_data as7927_50x_platform_data = {
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

    err = platform_device_add_data(pdev, &as7927_50x_platform_data,
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

static int as7927_50x_pcie_fpga_stat_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct as7927_50x_fpga_data *fpga_ctl;
    struct pci_dev *pcidev;
    struct resource *ret;
    int i;
    int status = 0, err = 0;
    unsigned long bar_base;

    fpga_ctl = devm_kzalloc(dev, sizeof(struct as7927_50x_fpga_data), GFP_KERNEL);
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
     * to request all Region 0 because another
     * address will be allocated by the i2c-ocores.ko.
     */
    fpga_ctl->pci_fpga_dev.data_base_addr0 = pci_iomap(pcidev, BAR0_NUM, 0);
    if (fpga_ctl->pci_fpga_dev.data_base_addr0 == NULL) {
        dev_err(dev, "Failed to map BAR0\n");
        status = -EIO;
        goto exit_pci_disable;
    }
    /* FPGA */
    fpga_ctl->pci_fpga_dev.data_region0 = pci_resource_start(pcidev, BAR0_NUM);
    ret = request_mem_region(fpga_ctl->pci_fpga_dev.data_region0, REGION_LEN, DRVNAME"_fpga");
    if (ret == NULL) {
        dev_err(dev, "[%s] cannot request region\n", DRVNAME"_fpga");
        status = -EIO;
        goto exit_pci_iounmap0;
    }
    dev_info(dev, "(BAR%d resource: Start=0x%lx, Length=0x%x)", BAR0_NUM,
                  (unsigned long)fpga_ctl->pci_fpga_dev.data_region0, REGION_LEN);
    /* CPLD1 */
    fpga_ctl->pci_fpga_dev.data_region1 = pci_resource_start(pcidev, BAR0_NUM) + CPLD1_PCIE_START_OFFSET;
    ret = request_mem_region(fpga_ctl->pci_fpga_dev.data_region1, REGION_LEN, DRVNAME"_cpld1");
    if (ret == NULL) {
        dev_err(dev, "[%s] cannot request region\n", DRVNAME"_cpld1");
        status = -EIO;
        goto exit_pci_iounmap1;
    }
    dev_info(dev, "(BAR%d resource: Start=0x%lx, Length=0x%x)", BAR0_NUM,
                  (unsigned long)fpga_ctl->pci_fpga_dev.data_region1, REGION_LEN);
    /* CPLD2 */
    fpga_ctl->pci_fpga_dev.data_region2 = pci_resource_start(pcidev, BAR0_NUM) + CPLD2_PCIE_START_OFFSET;
    ret = request_mem_region(fpga_ctl->pci_fpga_dev.data_region2, REGION_LEN, DRVNAME"_cpld2");
    if (ret == NULL) {
        dev_err(dev, "[%s] cannot request region\n", DRVNAME"_cpld2");
        status = -EIO;
        goto exit_pci_iounmap2;
    }
    dev_info(dev, "(BAR%d resource: Start=0x%lx, Length=0x%x)", BAR0_NUM,
                  (unsigned long)fpga_ctl->pci_fpga_dev.data_region2, REGION_LEN);

    /* Create I2C ocore devices first, then create the FPGA sysfs.
     * To prevent the application from accessing an ocore device
     * that has not been fully created due to the port status
     * being present.
     */

    /*
     * Create ocore_i2c device for OSFP EEPROM
     */
    for (i = 0; i < PORT_NUM; i++) {
        bar_base = pci_resource_start(pcidev, BAR0_NUM);
        fpga_ctl->pci_fpga_dev.fpga_i2c[i] =
            ocore_i2c_device_add((i | (port[i].mask << 8)), bar_base, port[i].offset);
        if (IS_ERR(fpga_ctl->pci_fpga_dev.fpga_i2c[i])) {
            status = PTR_ERR(fpga_ctl->pci_fpga_dev.fpga_i2c[i]);
            dev_err(dev, "rc:%d, unload Port%u[0x%ux] device\n",
                         status, i, port[i].offset);
            goto exit_ocores_device;
        }
    }

    status = sysfs_create_group(&pdev->dev.kobj, &fpga_port_stat_group);
    if (status) {
        goto exit_ocores_device;
    }

    return 0;

exit_ocores_device:
    while (i > 0) {
        i--;
        platform_device_unregister(fpga_ctl->pci_fpga_dev.fpga_i2c[i]);
    }
    release_mem_region(fpga_ctl->pci_fpga_dev.data_region2, REGION_LEN);
exit_pci_iounmap2:
    release_mem_region(fpga_ctl->pci_fpga_dev.data_region1, REGION_LEN);
exit_pci_iounmap1:
    release_mem_region(fpga_ctl->pci_fpga_dev.data_region0, REGION_LEN);
exit_pci_iounmap0:
    pci_iounmap(fpga_ctl->pci_fpga_dev.pci_dev, fpga_ctl->pci_fpga_dev.data_base_addr0);
exit_pci_disable:
    pci_disable_device(fpga_ctl->pci_fpga_dev.pci_dev);

    return status;
}

static int as7927_50x_pcie_fpga_stat_remove(struct platform_device *pdev)
{
    struct as7927_50x_fpga_data *fpga_ctl = platform_get_drvdata(pdev);

    if (pci_is_enabled(fpga_ctl->pci_fpga_dev.pci_dev)) {
        int i;
        sysfs_remove_group(&pdev->dev.kobj, &fpga_port_stat_group);
        /* Unregister ocore_i2c device */
        for (i = 0; i < PORT_NUM; i++) {
            platform_device_unregister(fpga_ctl->pci_fpga_dev.fpga_i2c[i]);
        }
        release_mem_region(fpga_ctl->pci_fpga_dev.data_region2, REGION_LEN);
        release_mem_region(fpga_ctl->pci_fpga_dev.data_region1, REGION_LEN);
        release_mem_region(fpga_ctl->pci_fpga_dev.data_region0, REGION_LEN);
        pci_iounmap(fpga_ctl->pci_fpga_dev.pci_dev, fpga_ctl->pci_fpga_dev.data_base_addr0);
        pci_disable_device(fpga_ctl->pci_fpga_dev.pci_dev);
    }

    return 0;
}

static struct platform_driver pcie_fpga_port_stat_driver = {
    .probe      = as7927_50x_pcie_fpga_stat_probe,
    .remove     = as7927_50x_pcie_fpga_stat_remove,
    .driver     = {
        .owner = THIS_MODULE,
        .name  = DRVNAME,
    },
};

static int __init as7927_50x_pcie_fpga_init(void)
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

static void __exit as7927_50x_pcie_fpga_exit(void)
{
    platform_device_unregister(pdev);
    platform_driver_unregister(&pcie_fpga_port_stat_driver);
}


module_init(as7927_50x_pcie_fpga_init);
module_exit(as7927_50x_pcie_fpga_exit);

MODULE_AUTHOR("Willy Liu <willy_liu@accton.com>");
MODULE_DESCRIPTION("AS7927-50X FPGA via PCIE");
MODULE_LICENSE("GPL");
