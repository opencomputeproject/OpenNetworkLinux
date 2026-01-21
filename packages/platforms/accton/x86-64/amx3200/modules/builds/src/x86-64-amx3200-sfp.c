/*
 * Copyright (C)  Brandon Chuang <brandon_chuang@accton.com.tw>
 *
 * Based on:
 *     pca954x.c from Kumar Gala <galak@kernel.crashing.org>
 * Copyright (C) 2006
 *
 * Based on:
 *     pca954x.c from Ken Harrenstien
 * Copyright (C) 2004 Google, Inc. (Ken Harrenstien)
 *
 * Based on:
 *     i2c-virtual_cb.c from Brian Kuschak <bkuschak@yahoo.com>
 * and
 *     pca9540.c from Jean Delvare <khali@linux-fr.org>.
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
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/delay.h>

#define DRVNAME "amx3200_sfp"

#define PORT_NUM (4 + 16) /* 4 CFP + 16 QSFP */
#define CFP_EEPROM_SIZE          256
#define NUM_OF_CFP_PORT_PER_SLED 2
#define EEPROM_FORMAT "module_eeprom_%d"

#define FPGA_PCIE_START_OFFSET                  0x10000000
#define FPGA_QSFP_RESET_REG                     (FPGA_PCIE_START_OFFSET + 0x10)
#define FPGA_QSFP_PRESENT_LPMODE_REG            (FPGA_PCIE_START_OFFSET + 0x38)

#define FPGA_CFP_RESET_REG                      (FPGA_PCIE_START_OFFSET + 0x8)
#define FPGA_CFP_PRESENT_RXLOS_REG              (FPGA_PCIE_START_OFFSET + 0x30)
#define FPGA_CFP_TXDIS_LPMODE_REG               (FPGA_PCIE_START_OFFSET + 0x30)

#define VALIDATE_CFP_PORT(p) \
    do { \
        if (p < 1 || p > 12) \
            return -EINVAL;  \
        if (p > 2 && p < 11) \
            return -EINVAL;  \
    } \
    while (0)

#define FPGA_STATUS_REG_WAIT_FOR_VALUE(sled_id, wait_val) \
    do { \
        int retval; \
        retval = fpga_wait_for_status_reg_value(sled_id, wait_val); \
        if (retval < 0) { \
            return retval; \
        } \
    } while(0)

enum PCI_FPGA_DEV {
    FPGA_SLED1 = 0,
    FPGA_SLED2,
    FPGA_DEV_COUNT
};

enum CoreMDIO_APB_Registers {
    MDIO_BASE       = 0x21300000,
    ADDRESSREG      = MDIO_BASE + 0x00000000,
    PHYADDRREG      = MDIO_BASE + 0x00000004,
    CONTROLREG      = MDIO_BASE + 0x00000008,
    STATUSREG       = MDIO_BASE + 0x0000000C,
    DATAINREG       = MDIO_BASE + 0x00000010,
    DATAOUTREG      = MDIO_BASE + 0x00000014,
    CLKPRESCALERREG = MDIO_BASE + 0x00000018
};

extern int amx3200_fpga_read32(int sled_id, u32 reg);
extern int amx3200_fpga_write32(int sled_id, u32 reg, u32 value);
static int amx300_sfp_probe(struct platform_device *pdev);
static int amx300_sfp_remove(struct platform_device *pdev);

/***********************************************
 *       transceiver enum define
 * *********************************************/
#define TRANSCEIVER_ATTR_ID_QSFP(index) \
     MODULE_PRESENT_##index =     (index-1), \
     MODULE_LPMODE_##index =      (index-1) + (PORT_NUM * 1), \
     MODULE_RESET_##index =       (index-1) + (PORT_NUM * 2)

#define TRANSCEIVER_ATTR_ID_CFP(index) \
     MODULE_PRESENT_##index      = (index-1), \
     MODULE_LPMODE_##index       = (index-1) + (PORT_NUM * 1), \
     MODULE_RESET_##index        = (index-1) + (PORT_NUM * 2), \
     MODULE_TX_DISABLE_##index   = (index-1) + (PORT_NUM * 3), \
     MODULE_RX_LOS_##index       = (index-1) + (PORT_NUM * 4)

enum fpga_sysfs_attributes {
    /* transceiver attributes */
    TRANSCEIVER_ATTR_ID_CFP(1),
    TRANSCEIVER_ATTR_ID_CFP(2),
    TRANSCEIVER_ATTR_ID_QSFP(3),
    TRANSCEIVER_ATTR_ID_QSFP(4),
    TRANSCEIVER_ATTR_ID_QSFP(5),
    TRANSCEIVER_ATTR_ID_QSFP(6),
    TRANSCEIVER_ATTR_ID_QSFP(7),
    TRANSCEIVER_ATTR_ID_QSFP(8),
    TRANSCEIVER_ATTR_ID_QSFP(9),
    TRANSCEIVER_ATTR_ID_QSFP(10),
    TRANSCEIVER_ATTR_ID_CFP(11),
    TRANSCEIVER_ATTR_ID_CFP(12),
    TRANSCEIVER_ATTR_ID_QSFP(13),
    TRANSCEIVER_ATTR_ID_QSFP(14),
    TRANSCEIVER_ATTR_ID_QSFP(15),
    TRANSCEIVER_ATTR_ID_QSFP(16),
    TRANSCEIVER_ATTR_ID_QSFP(17),
    TRANSCEIVER_ATTR_ID_QSFP(18),
    TRANSCEIVER_ATTR_ID_QSFP(19),
    TRANSCEIVER_ATTR_ID_QSFP(20),
};

/***********************************************
 *       function declare
 * *********************************************/
static ssize_t show_qsfp(struct device *dev, struct device_attribute *da,
             char *buf);
static ssize_t set_qsfp(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);
static ssize_t show_cfp(struct device *dev, struct device_attribute *da,
             char *buf);
static ssize_t set_cfp(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);

/***********************************************
 *       transceiver sysfs attributes
 **********************************************/
#define MODULE_ATTRS_QSFP(index) \
     static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, \
          show_qsfp, NULL, MODULE_PRESENT_##index); \
     static SENSOR_DEVICE_ATTR(module_reset_##index, S_IRUGO | S_IWUSR,\
          show_qsfp, set_qsfp, MODULE_RESET_##index); \
     static SENSOR_DEVICE_ATTR(module_lpmode_##index, S_IRUGO | S_IWUSR, \
          show_qsfp, set_qsfp, MODULE_LPMODE_##index); \
     static struct attribute *module_attributes##index[] = { \
          &sensor_dev_attr_module_present_##index.dev_attr.attr, \
          &sensor_dev_attr_module_reset_##index.dev_attr.attr, \
          &sensor_dev_attr_module_lpmode_##index.dev_attr.attr, \
          NULL \
     }

#define MODULE_ATTRS_CFP(index) \
     static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, \
          show_cfp, NULL, MODULE_PRESENT_##index); \
     static SENSOR_DEVICE_ATTR(module_reset_##index, S_IRUGO | S_IWUSR,\
          show_cfp, set_cfp, MODULE_RESET_##index); \
     static SENSOR_DEVICE_ATTR(module_lpmode_##index, S_IRUGO | S_IWUSR, \
          show_cfp, set_cfp, MODULE_LPMODE_##index); \
     static SENSOR_DEVICE_ATTR(module_tx_disable_##index, S_IRUGO | S_IWUSR,\
          show_cfp, set_cfp, MODULE_TX_DISABLE_##index); \
     static SENSOR_DEVICE_ATTR(module_rxlos_##index, S_IRUGO,\
          show_cfp, NULL, MODULE_RX_LOS_##index); \
     static struct attribute *module_attributes##index[] = { \
          &sensor_dev_attr_module_present_##index.dev_attr.attr, \
          &sensor_dev_attr_module_reset_##index.dev_attr.attr, \
          &sensor_dev_attr_module_lpmode_##index.dev_attr.attr, \
          &sensor_dev_attr_module_tx_disable_##index.dev_attr.attr, \
          &sensor_dev_attr_module_rxlos_##index.dev_attr.attr, \
          NULL \
     }

MODULE_ATTRS_CFP(1);
MODULE_ATTRS_CFP(2);
MODULE_ATTRS_QSFP(3);
MODULE_ATTRS_QSFP(4);
MODULE_ATTRS_QSFP(5);
MODULE_ATTRS_QSFP(6);
MODULE_ATTRS_QSFP(7);
MODULE_ATTRS_QSFP(8);
MODULE_ATTRS_QSFP(9);
MODULE_ATTRS_QSFP(10);
MODULE_ATTRS_CFP(11);
MODULE_ATTRS_CFP(12);
MODULE_ATTRS_QSFP(13);
MODULE_ATTRS_QSFP(14);
MODULE_ATTRS_QSFP(15);
MODULE_ATTRS_QSFP(16);
MODULE_ATTRS_QSFP(17);
MODULE_ATTRS_QSFP(18);
MODULE_ATTRS_QSFP(19);
MODULE_ATTRS_QSFP(20);

#define MODULE_ATTR_GROUP_CFP(index) { .attrs = module_attributes##index }
#define MODULE_ATTR_GROUP_QSFP(index) { .attrs = module_attributes##index }

static struct attribute_group sled1_port_group[] = {
     MODULE_ATTR_GROUP_CFP(1),
     MODULE_ATTR_GROUP_CFP(2),
     MODULE_ATTR_GROUP_QSFP(3),
     MODULE_ATTR_GROUP_QSFP(4),
     MODULE_ATTR_GROUP_QSFP(5),
     MODULE_ATTR_GROUP_QSFP(6),
     MODULE_ATTR_GROUP_QSFP(7),
     MODULE_ATTR_GROUP_QSFP(8),
     MODULE_ATTR_GROUP_QSFP(9),
     MODULE_ATTR_GROUP_QSFP(10)
};

static struct attribute_group sled2_port_group[] = {
     MODULE_ATTR_GROUP_CFP(11),
     MODULE_ATTR_GROUP_CFP(12),
     MODULE_ATTR_GROUP_QSFP(13),
     MODULE_ATTR_GROUP_QSFP(14),
     MODULE_ATTR_GROUP_QSFP(15),
     MODULE_ATTR_GROUP_QSFP(16),
     MODULE_ATTR_GROUP_QSFP(17),
     MODULE_ATTR_GROUP_QSFP(18),
     MODULE_ATTR_GROUP_QSFP(19),
     MODULE_ATTR_GROUP_QSFP(20)
};

static struct attribute_group *sled_port_groups[FPGA_DEV_COUNT] = {
    sled1_port_group,
    sled2_port_group
};

struct attribute_mapping {
    u16 attr_base;
    u64 reg;
    u8 invert;
};

// Define an array of attribute mappings
static struct attribute_mapping attribute_mappings[] = {
    [MODULE_PRESENT_1 ... MODULE_PRESENT_2] = {MODULE_PRESENT_1, FPGA_CFP_PRESENT_RXLOS_REG, 1},
    [MODULE_RX_LOS_1 ... MODULE_RX_LOS_2] = {MODULE_RX_LOS_1, FPGA_CFP_PRESENT_RXLOS_REG, 0},
    [MODULE_LPMODE_1 ... MODULE_LPMODE_2] = {MODULE_LPMODE_1, FPGA_CFP_TXDIS_LPMODE_REG, 0},
    [MODULE_RESET_1 ... MODULE_RESET_2] = {MODULE_RESET_1, FPGA_CFP_RESET_REG, 1},
    [MODULE_TX_DISABLE_1 ... MODULE_TX_DISABLE_2] = {MODULE_TX_DISABLE_1, FPGA_CFP_TXDIS_LPMODE_REG, 0},
    [MODULE_PRESENT_11 ... MODULE_PRESENT_12] = {MODULE_PRESENT_11, FPGA_CFP_PRESENT_RXLOS_REG, 1},
    [MODULE_RX_LOS_11 ... MODULE_RX_LOS_12] = {MODULE_RX_LOS_11, FPGA_CFP_PRESENT_RXLOS_REG, 0},
    [MODULE_LPMODE_11 ... MODULE_LPMODE_12] = {MODULE_LPMODE_11, FPGA_CFP_TXDIS_LPMODE_REG, 0},
    [MODULE_RESET_11 ... MODULE_RESET_12] = {MODULE_RESET_11, FPGA_CFP_RESET_REG, 1},
    [MODULE_TX_DISABLE_11 ... MODULE_TX_DISABLE_12] = {MODULE_TX_DISABLE_11, FPGA_CFP_TXDIS_LPMODE_REG, 0},

    [MODULE_PRESENT_3 ... MODULE_PRESENT_10] = {MODULE_PRESENT_3, FPGA_QSFP_PRESENT_LPMODE_REG, 1},
    [MODULE_PRESENT_13 ... MODULE_PRESENT_20] = {MODULE_PRESENT_13, FPGA_QSFP_PRESENT_LPMODE_REG, 1},
    [MODULE_LPMODE_3 ... MODULE_LPMODE_10] = {MODULE_LPMODE_3, FPGA_QSFP_PRESENT_LPMODE_REG, 0},
    [MODULE_LPMODE_13 ... MODULE_LPMODE_20] = {MODULE_LPMODE_13, FPGA_QSFP_PRESENT_LPMODE_REG, 0},
    [MODULE_RESET_3 ... MODULE_RESET_10] = {MODULE_RESET_3, FPGA_QSFP_RESET_REG, 1},
    [MODULE_RESET_13 ... MODULE_RESET_20] = {MODULE_RESET_13, FPGA_QSFP_RESET_REG, 1}
};

struct amx300_sled_sfp_data {
    int sled_id;
    struct platform_device *pdev;
    struct mutex     update_lock;
    struct bin_attribute eeprom[NUM_OF_CFP_PORT_PER_SLED]; /* eeprom data */
};

struct eeprom_priv {
    int port;
    struct amx300_sled_sfp_data *data;
};

static ssize_t show_qsfp(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct amx300_sled_sfp_data *data = dev_get_drvdata(dev);
    ssize_t ret = -EINVAL;
    u64 reg;
    u32 reg_val = 0;
    u8 bits_shift;

    reg = attribute_mappings[attr->index].reg;

    mutex_lock(&data->update_lock);
    reg_val = amx3200_fpga_read32(data->sled_id, reg);
    mutex_unlock(&data->update_lock);

    bits_shift = attr->index - attribute_mappings[attr->index].attr_base;

    if ((attr->index >= MODULE_LPMODE_3 && attr->index <= MODULE_LPMODE_10)
        || (attr->index >= MODULE_LPMODE_13 && attr->index <= MODULE_LPMODE_20))
        reg_val = (reg_val >> (bits_shift+8)) & 0x01;
    else
        reg_val = (reg_val >> bits_shift) & 0x01;

    if (attribute_mappings[attr->index].invert)
        reg_val = !reg_val;

    ret = sprintf(buf, "%u\n", reg_val);
    return ret;
}

static ssize_t set_qsfp(struct device *dev, struct device_attribute *da,
                            const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct amx300_sled_sfp_data *data = dev_get_drvdata(dev);
    int status;
    u64 reg;
    u8 input;
    u32 reg_val = 0;
    u16 bit_mask, should_set_bit;

    status = kstrtou8(buf, 10, &input);
    if (status)
        return status;

    reg = attribute_mappings[attr->index].reg;

    if ((attr->index >= MODULE_LPMODE_3 && attr->index <= MODULE_LPMODE_10)
            || (attr->index >= MODULE_LPMODE_13 && attr->index <= MODULE_LPMODE_20))
        bit_mask = 0x01 << ((attr->index - attribute_mappings[attr->index].attr_base) + 8);
    else
        bit_mask = 0x01 << (attr->index - attribute_mappings[attr->index].attr_base);

    should_set_bit = attribute_mappings[attr->index].invert ? !input : input;

    mutex_lock(&data->update_lock);
    reg_val = amx3200_fpga_read32(data->sled_id, reg);

    if (should_set_bit)
        reg_val |= bit_mask;
    else
        reg_val &= ~bit_mask;

    amx3200_fpga_write32(data->sled_id, reg, reg_val);
    mutex_unlock(&data->update_lock);

    return count;
}

static ssize_t show_cfp(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct amx300_sled_sfp_data *data = dev_get_drvdata(dev);
    u32 reg_val = 0;
    u32 reg = 0, mask = 0, invert = 0, bit, module_reset = 0;

    switch (attr->index) {
    case MODULE_PRESENT_1 ... MODULE_PRESENT_2:
    case MODULE_PRESENT_11 ... MODULE_PRESENT_12:
        bit = 1;
        break;
    case MODULE_RX_LOS_1 ... MODULE_RX_LOS_2:
    case MODULE_RX_LOS_11 ... MODULE_RX_LOS_12:
        bit = 2;
        break;
    case MODULE_RESET_1 ... MODULE_RESET_2:
    case MODULE_RESET_11 ... MODULE_RESET_12:
        module_reset = 1;
        bit = 16;
        break;
    case MODULE_LPMODE_1 ... MODULE_LPMODE_2:
    case MODULE_LPMODE_11 ... MODULE_LPMODE_12:
        bit = 8;
        break;
    case MODULE_TX_DISABLE_1 ... MODULE_TX_DISABLE_2:
    case MODULE_TX_DISABLE_11 ... MODULE_TX_DISABLE_12:
        bit = 9;
        break;
    default:
        return 0;
    }

    invert = attribute_mappings[attr->index].invert;
    reg = attribute_mappings[attr->index].reg;
    if (module_reset)
        mask = BIT((attr->index - attribute_mappings[attr->index].attr_base) * 1 + bit);
    else
        mask = BIT((attr->index - attribute_mappings[attr->index].attr_base) * 16 + bit);

    mutex_lock(&data->update_lock);
    reg_val = amx3200_fpga_read32(data->sled_id, reg);
    mutex_unlock(&data->update_lock);

    return sprintf(buf, "%d\n", invert ? !(reg_val & mask) : !!(reg_val & mask));
}

static ssize_t set_cfp(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count)
{
    struct amx300_sled_sfp_data *data = dev_get_drvdata(dev);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    long value;
    int status;
    u32 reg_val = 0;
    u32 reg = 0, mask = 0, invert = 0, bit, module_reset = 0;

    status = kstrtol(buf, 10, &value);
    if (status)
        return status;

    switch (attr->index) {
    case MODULE_RESET_1 ... MODULE_RESET_2:
    case MODULE_RESET_11 ... MODULE_RESET_12:
        bit = 16;
        module_reset = 1;
        break;
    case MODULE_LPMODE_1 ... MODULE_LPMODE_2:
    case MODULE_LPMODE_11 ... MODULE_LPMODE_12:
        bit = 8;
        break;
    case MODULE_TX_DISABLE_1 ... MODULE_TX_DISABLE_2:
    case MODULE_TX_DISABLE_11 ... MODULE_TX_DISABLE_12:
        bit = 9;
        break;
    default:
        return 0;
    }

    invert = attribute_mappings[attr->index].invert;
    reg = attribute_mappings[attr->index].reg;
    if (module_reset)
    {
        mask = BIT((attr->index - attribute_mappings[attr->index].attr_base) * 1 + bit);
    }
    else
    {
        mask = BIT((attr->index - attribute_mappings[attr->index].attr_base) * 16 + bit);
    }
    /* Read current status */
    mutex_lock(&data->update_lock);
    reg_val = amx3200_fpga_read32(data->sled_id, reg);
    /* Update status */
    value = invert ? !value : !!value;
    if (value)
        reg_val |= mask;
    else
        reg_val &= ~mask;

    /* Set value to FPGA */
    amx3200_fpga_write32(data->sled_id, reg, reg_val);
    mutex_unlock(&data->update_lock);
    return count;
}

static int fpga_wait_for_status_reg_value(int sled_id, int wait_value)
{
    /*
    * Ref optoe.c:
    * specs often allow 5 msec for a page write, sometimes 20 msec;
    * it's important to recover from write timeouts.
    */
    static unsigned int access_timeout = 25; /* ms */
    unsigned long timeout, access_time;
    u32 real_value = 0;

    timeout = jiffies + msecs_to_jiffies(access_timeout);

    do {
        access_time = jiffies;
        usleep_range(50, 100);

        real_value = amx3200_fpga_read32(sled_id, STATUSREG);
        if (real_value != wait_value) {
            continue;
        }

        return 0;
    } while (time_before(access_time, timeout));

    return -ETIME;
}

static ssize_t cfp_eeprom_read(loff_t off, char *buf, size_t count, int port)
{
    /* phy_addr is 0x00 for port 1/11, 0x04 for port 2/12 */
    u32 phy_addr = (port % 2) ? 0x00000000 : 0x00000004;
    u32 status = 0;
    int sled_id = 0;

    sled_id = ((port / 10) == 0) ? FPGA_SLED1 : FPGA_SLED2;
    amx3200_fpga_write32(sled_id, ADDRESSREG, 0x00000001);
    amx3200_fpga_write32(sled_id, PHYADDRREG, phy_addr);
    amx3200_fpga_write32(sled_id, CLKPRESCALERREG, 0x00000007); // 111 - MGTCLK/28
    amx3200_fpga_write32(sled_id, DATAINREG, 0x00008000 + off);
    amx3200_fpga_write32(sled_id, CONTROLREG, 0x00000000);
    FPGA_STATUS_REG_WAIT_FOR_VALUE(sled_id, 0x00000001);
    FPGA_STATUS_REG_WAIT_FOR_VALUE(sled_id, 0x00000000);
    amx3200_fpga_write32(sled_id, CONTROLREG, 0x00000003);
    FPGA_STATUS_REG_WAIT_FOR_VALUE(sled_id, 0x00000001);
    FPGA_STATUS_REG_WAIT_FOR_VALUE(sled_id, 0x00000000);

    status = amx3200_fpga_read32(sled_id, DATAOUTREG);
    *buf = (char)(status & 0xFF);
    return 1;
}

static int cfp_is_port_present(int port, struct amx300_sled_sfp_data *data)
{
    u32 reg_val = 0, mask = 0;
    int sled_id = ((port / 10) == 0) ? FPGA_SLED1 : FPGA_SLED2;

    switch (port) {
    case 1:
    case 11:
        mask = BIT(1);
        break;
    case 2:
    case 12:
        mask = BIT(17);
        break;
    default:
        return 0;
    }

    mutex_lock(&data->update_lock);
    reg_val = amx3200_fpga_read32(sled_id, FPGA_CFP_PRESENT_RXLOS_REG);
    mutex_unlock(&data->update_lock);

    return !(reg_val & mask);
}

static ssize_t cfp_bin_read(struct file *filp, struct kobject *kobj,
          struct bin_attribute *attr,
          char *buf, loff_t off, size_t count)
{
    ssize_t retval = 0;
    u64 port = 0;
    struct eeprom_priv *priv = (struct eeprom_priv *)attr->private;

    if (unlikely(!count)) {
        return count;
    }

    if (!priv) {
        return -EINVAL;
    }

    port = (u64)(priv->port);
    VALIDATE_CFP_PORT(port);
    
    if (!cfp_is_port_present(port, priv->data)) {
        return -ENXIO;
    }

    /*
     * Read data from chip, protecting against concurrent updates
     * from this host
     */
    mutex_lock(&priv->data->update_lock);

    while (count) {
        ssize_t status;

        status = cfp_eeprom_read(off, buf, count, port);
        if (status <= 0) {
            if (retval == 0) {
                retval = status;
            }

            break;
        }

        buf += status;
        off += status;
        count -= status;
        retval += status;
    }

    mutex_unlock(&priv->data->update_lock);
    return retval;
}

static int sysfs_eeprom_init(struct kobject *kobj, struct bin_attribute *eeprom,
                            u64 port, struct amx300_sled_sfp_data *data)
{
    int ret = 0;
    char *eeprom_name = NULL;
    struct eeprom_priv *priv;

    priv = kzalloc(sizeof(struct eeprom_priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    priv->data = data;
    priv->port = port;

    eeprom_name = kzalloc(32, GFP_KERNEL);
    if (!eeprom_name) {
        ret = -ENOMEM;
        goto eeprom_name_err;
    }

    sprintf(eeprom_name, EEPROM_FORMAT, (int)port);
    sysfs_bin_attr_init(eeprom);
    eeprom->attr.name = eeprom_name;
    eeprom->attr.mode = S_IRUGO;// | S_IWUSR;
    eeprom->read       = cfp_bin_read;
    eeprom->write       = NULL; // sfp_bin_write;
    eeprom->size       = CFP_EEPROM_SIZE;
    eeprom->private   = (void*)priv;

    /* Create eeprom file */
    ret = sysfs_create_bin_file(kobj, eeprom);
    if (unlikely(ret != 0)) {
        goto bin_err;
    }

    return ret;

bin_err:
    kfree(eeprom_name);
eeprom_name_err:
    kfree(priv);
    return ret;
}

static int sysfs_bin_attr_cleanup(struct kobject *kobj, struct bin_attribute *bin_attr)
{
    struct eeprom_priv *priv = (struct eeprom_priv *)bin_attr->private;
    sysfs_remove_bin_file(kobj, bin_attr);
    if (priv)
        kfree(priv);

    return 0;
}

static int amx300_sfp_probe(struct platform_device *pdev)
{
    int status = -1;
    int i = 0, j = 0, port, group_size;
    struct amx300_sled_sfp_data *data;

    data = kzalloc(sizeof(struct amx300_sled_sfp_data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    data->sled_id = (pdev->id) == 0 ? FPGA_SLED1 : FPGA_SLED2;
    mutex_init(&data->update_lock);
    platform_set_drvdata(pdev, data);

    group_size = (data->sled_id == FPGA_SLED1) ?
                 ARRAY_SIZE(sled1_port_group) : ARRAY_SIZE(sled2_port_group);

    /* Register sysfs hooks */
    for (i = 0; i < group_size; i++) {
        /* Register sysfs hooks */
        status = sysfs_create_group(&pdev->dev.kobj,
                                    &sled_port_groups[data->sled_id][i]);
        if (status)
            goto exit_sysfs_group;
    }

    for (j = 0; j < ARRAY_SIZE(data->eeprom); j++) {
        port = data->sled_id*10 + (j % 2) + 1; /*port name start from 1*/

        /* Register sysfs hooks */
        status = sysfs_eeprom_init(&pdev->dev.kobj,
                                   &data->eeprom[j],
                                   port,
                                   data);
        if (status)
            goto exit_eeprom;
    }

    dev_info(&pdev->dev, "device created\n");
    return 0;

exit_eeprom:
    /* Remove the eeprom attributes which were created successfully */
    for (--j; j >= 0; j--) {
        sysfs_bin_attr_cleanup(&pdev->dev.kobj, &data->eeprom[j]);
    }
exit_sysfs_group:
    for (--i; i >= 0; i--) {
        sysfs_remove_group(&pdev->dev.kobj, &sled_port_groups[data->sled_id][i]);
    }

    kfree(data);
    return status;
}

static int amx300_sfp_remove(struct platform_device *pdev)
{
    struct amx300_sled_sfp_data *data = platform_get_drvdata(pdev);
    int i = 0, group_size;

    group_size = (data->sled_id == FPGA_SLED1) ?
                 ARRAY_SIZE(sled1_port_group) : ARRAY_SIZE(sled2_port_group);

    for (i = 0; i < group_size; i++) {
        sysfs_remove_group(&pdev->dev.kobj, &sled_port_groups[data->sled_id][i]);
    }

    for (i = 0; i < ARRAY_SIZE(data->eeprom); i++) {
        sysfs_bin_attr_cleanup(&pdev->dev.kobj, &data->eeprom[i]);
    }

    kfree(data);
    return 0;
}

static const struct platform_device_id amx300_sfp_id[] = {
     { "amx3200_sled1_sfp", 0 },
     { "amx3200_sled2_sfp", 1 },
     {}
};
MODULE_DEVICE_TABLE(platform, amx300_sfp_id);

static struct platform_driver amx300_sfp_driver = {
    .probe      = amx300_sfp_probe,
    .remove     = amx300_sfp_remove,
     .id_table     = amx300_sfp_id,
    .driver     = {
        .name   = DRVNAME,
        .owner  = THIS_MODULE,
    },
};

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com>");
MODULE_DESCRIPTION("amx3200 sfp driver");
MODULE_LICENSE("GPL");

module_platform_driver(amx300_sfp_driver);
