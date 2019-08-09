/*
 * Copyright (C)  Jostar yang <jostar_yang@accton.com.tw>
 *
 * This module supports the accton cpld that hold the channel select
 * mechanism for other i2c slave devices, such as SFP.
 * This includes the:
 *	 Accton asgvolt64 CPLD
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
 * and
 *	pca9540.c from Jean Delvare <khali@linux-fr.org>.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/version.h>
#include <linux/stat.h>
#include <linux/hwmon-sysfs.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>

#define I2C_RW_RETRY_COUNT				10
#define I2C_RW_RETRY_INTERVAL			60 /* ms */
#define FAN_DUTY_CYCLE_REG_MASK         0x1F
#define FAN_MAX_DUTY_CYCLE              100
#define FAN_REG_VAL_TO_SPEED_RPM_STEP   114 // R.P.M value = read value x3.79*60/2

static LIST_HEAD(cpld_client_list);
static struct mutex     list_lock;

struct cpld_client_node {
    struct i2c_client *client;
    struct list_head   list;
};

enum cpld_type {
    asgvolt64_cpld1,
    asgvolt64_cpld2,
    asgvolt64_cpld3
};

struct asgvolt64_cpld_data {
    enum cpld_type   type;
    struct device   *hwmon_dev;
    struct mutex     update_lock;
    char             valid;           /* != 0 if registers are valid */
    unsigned long    last_updated;    /* In jiffies */
};

static const struct i2c_device_id asgvolt64_cpld_id[] = {
    { "asgvlot64_cpld1", asgvolt64_cpld1},
    { "asgvlot64_cpld2", asgvolt64_cpld2},
    { "asgvlot64_cpld3", asgvolt64_cpld3},
    { }
};
MODULE_DEVICE_TABLE(i2c, asgvolt64_cpld_id);

#define TRANSCEIVER_PRESENT_ATTR_ID(index)   	MODULE_PRESENT_##index
#define TRANSCEIVER_TXDISABLE_ATTR_ID(index)   	MODULE_TXDISABLE_##index
#define TRANSCEIVER_RXLOS_ATTR_ID(index)   		MODULE_RXLOS_##index
#define TRANSCEIVER_TXFAULT_ATTR_ID(index)   	MODULE_TXFAULT_##index


enum asgvolt64_cpld_sysfs_attributes {
	CPLD_VERSION,
	ACCESS,
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
	TRANSCEIVER_PRESENT_ATTR_ID(51),
	TRANSCEIVER_PRESENT_ATTR_ID(52),
	TRANSCEIVER_PRESENT_ATTR_ID(53),
	TRANSCEIVER_PRESENT_ATTR_ID(54),
	TRANSCEIVER_PRESENT_ATTR_ID(55),
	TRANSCEIVER_PRESENT_ATTR_ID(56),	
	TRANSCEIVER_PRESENT_ATTR_ID(57),
	TRANSCEIVER_PRESENT_ATTR_ID(58),
	TRANSCEIVER_PRESENT_ATTR_ID(59),
	TRANSCEIVER_PRESENT_ATTR_ID(60),
	TRANSCEIVER_PRESENT_ATTR_ID(61),
	TRANSCEIVER_PRESENT_ATTR_ID(62),
	TRANSCEIVER_PRESENT_ATTR_ID(63),
	TRANSCEIVER_PRESENT_ATTR_ID(64),
	TRANSCEIVER_PRESENT_ATTR_ID(65),
	TRANSCEIVER_PRESENT_ATTR_ID(66),
	TRANSCEIVER_PRESENT_ATTR_ID(67),
	TRANSCEIVER_PRESENT_ATTR_ID(68),
	TRANSCEIVER_PRESENT_ATTR_ID(69),
	TRANSCEIVER_PRESENT_ATTR_ID(70),
	TRANSCEIVER_PRESENT_ATTR_ID(71),
	TRANSCEIVER_PRESENT_ATTR_ID(72),
	TRANSCEIVER_PRESENT_ATTR_ID(73),
	TRANSCEIVER_PRESENT_ATTR_ID(74),

	TRANSCEIVER_RXLOS_ATTR_ID(67),
	TRANSCEIVER_RXLOS_ATTR_ID(68),
	TRANSCEIVER_RXLOS_ATTR_ID(69),
	TRANSCEIVER_RXLOS_ATTR_ID(70),
	TRANSCEIVER_RXLOS_ATTR_ID(71),
	TRANSCEIVER_RXLOS_ATTR_ID(72),
	TRANSCEIVER_RXLOS_ATTR_ID(73),
	TRANSCEIVER_RXLOS_ATTR_ID(74),
	
	TRANSCEIVER_TXFAULT_ATTR_ID(1),
	TRANSCEIVER_TXFAULT_ATTR_ID(2),
	TRANSCEIVER_TXFAULT_ATTR_ID(3),
	TRANSCEIVER_TXFAULT_ATTR_ID(4),
	TRANSCEIVER_TXFAULT_ATTR_ID(5),
	TRANSCEIVER_TXFAULT_ATTR_ID(6),
	TRANSCEIVER_TXFAULT_ATTR_ID(7),
	TRANSCEIVER_TXFAULT_ATTR_ID(8),
	TRANSCEIVER_TXFAULT_ATTR_ID(9),
	TRANSCEIVER_TXFAULT_ATTR_ID(10),
	TRANSCEIVER_TXFAULT_ATTR_ID(11),
	TRANSCEIVER_TXFAULT_ATTR_ID(12),
	TRANSCEIVER_TXFAULT_ATTR_ID(13),
	TRANSCEIVER_TXFAULT_ATTR_ID(14),
	TRANSCEIVER_TXFAULT_ATTR_ID(15),
	TRANSCEIVER_TXFAULT_ATTR_ID(16),
	TRANSCEIVER_TXFAULT_ATTR_ID(17),
	TRANSCEIVER_TXFAULT_ATTR_ID(18),
	TRANSCEIVER_TXFAULT_ATTR_ID(19),
	TRANSCEIVER_TXFAULT_ATTR_ID(20),
	TRANSCEIVER_TXFAULT_ATTR_ID(21),
	TRANSCEIVER_TXFAULT_ATTR_ID(22),
	TRANSCEIVER_TXFAULT_ATTR_ID(23),
	TRANSCEIVER_TXFAULT_ATTR_ID(24),
	TRANSCEIVER_TXFAULT_ATTR_ID(25),
	TRANSCEIVER_TXFAULT_ATTR_ID(26),
	TRANSCEIVER_TXFAULT_ATTR_ID(27),
	TRANSCEIVER_TXFAULT_ATTR_ID(28),
	TRANSCEIVER_TXFAULT_ATTR_ID(29),
	TRANSCEIVER_TXFAULT_ATTR_ID(30),
	TRANSCEIVER_TXFAULT_ATTR_ID(31),
	TRANSCEIVER_TXFAULT_ATTR_ID(32),
	TRANSCEIVER_TXFAULT_ATTR_ID(33),
	TRANSCEIVER_TXFAULT_ATTR_ID(34),
	TRANSCEIVER_TXFAULT_ATTR_ID(35),
	TRANSCEIVER_TXFAULT_ATTR_ID(36),
	TRANSCEIVER_TXFAULT_ATTR_ID(37),
	TRANSCEIVER_TXFAULT_ATTR_ID(38),
	TRANSCEIVER_TXFAULT_ATTR_ID(39),
	TRANSCEIVER_TXFAULT_ATTR_ID(40),
	TRANSCEIVER_TXFAULT_ATTR_ID(41),
	TRANSCEIVER_TXFAULT_ATTR_ID(42),
	TRANSCEIVER_TXFAULT_ATTR_ID(43),
	TRANSCEIVER_TXFAULT_ATTR_ID(44),
	TRANSCEIVER_TXFAULT_ATTR_ID(45),	
	TRANSCEIVER_TXFAULT_ATTR_ID(46),
	TRANSCEIVER_TXFAULT_ATTR_ID(47),
	TRANSCEIVER_TXFAULT_ATTR_ID(48),
	TRANSCEIVER_TXFAULT_ATTR_ID(49),
	TRANSCEIVER_TXFAULT_ATTR_ID(50),
	TRANSCEIVER_TXFAULT_ATTR_ID(51),
	TRANSCEIVER_TXFAULT_ATTR_ID(52),
	TRANSCEIVER_TXFAULT_ATTR_ID(53),
	TRANSCEIVER_TXFAULT_ATTR_ID(54),
	TRANSCEIVER_TXFAULT_ATTR_ID(55),	
	TRANSCEIVER_TXFAULT_ATTR_ID(56),
	TRANSCEIVER_TXFAULT_ATTR_ID(57),
	TRANSCEIVER_TXFAULT_ATTR_ID(58),
	TRANSCEIVER_TXFAULT_ATTR_ID(59),
	TRANSCEIVER_TXFAULT_ATTR_ID(60),
	TRANSCEIVER_TXFAULT_ATTR_ID(61),
	TRANSCEIVER_TXFAULT_ATTR_ID(62),
	TRANSCEIVER_TXFAULT_ATTR_ID(63),
	TRANSCEIVER_TXFAULT_ATTR_ID(64),
	TRANSCEIVER_TXFAULT_ATTR_ID(67),
	TRANSCEIVER_TXFAULT_ATTR_ID(68),
	TRANSCEIVER_TXFAULT_ATTR_ID(69),
	TRANSCEIVER_TXFAULT_ATTR_ID(70),
	TRANSCEIVER_TXFAULT_ATTR_ID(71),
	TRANSCEIVER_TXFAULT_ATTR_ID(72),
	TRANSCEIVER_TXFAULT_ATTR_ID(73),
	TRANSCEIVER_TXFAULT_ATTR_ID(74),
	
	TRANSCEIVER_TXDISABLE_ATTR_ID(1),
	TRANSCEIVER_TXDISABLE_ATTR_ID(2),
	TRANSCEIVER_TXDISABLE_ATTR_ID(3),
	TRANSCEIVER_TXDISABLE_ATTR_ID(4),
	TRANSCEIVER_TXDISABLE_ATTR_ID(5),
	TRANSCEIVER_TXDISABLE_ATTR_ID(6),
	TRANSCEIVER_TXDISABLE_ATTR_ID(7),
	TRANSCEIVER_TXDISABLE_ATTR_ID(8),
	TRANSCEIVER_TXDISABLE_ATTR_ID(9),
	TRANSCEIVER_TXDISABLE_ATTR_ID(10),
	TRANSCEIVER_TXDISABLE_ATTR_ID(11),
	TRANSCEIVER_TXDISABLE_ATTR_ID(12),
	TRANSCEIVER_TXDISABLE_ATTR_ID(13),
	TRANSCEIVER_TXDISABLE_ATTR_ID(14),
	TRANSCEIVER_TXDISABLE_ATTR_ID(15),
	TRANSCEIVER_TXDISABLE_ATTR_ID(16),
	TRANSCEIVER_TXDISABLE_ATTR_ID(17),
	TRANSCEIVER_TXDISABLE_ATTR_ID(18),
	TRANSCEIVER_TXDISABLE_ATTR_ID(19),
	TRANSCEIVER_TXDISABLE_ATTR_ID(20),
	TRANSCEIVER_TXDISABLE_ATTR_ID(21),
	TRANSCEIVER_TXDISABLE_ATTR_ID(22),
	TRANSCEIVER_TXDISABLE_ATTR_ID(23),
	TRANSCEIVER_TXDISABLE_ATTR_ID(24),
	TRANSCEIVER_TXDISABLE_ATTR_ID(25),
	TRANSCEIVER_TXDISABLE_ATTR_ID(26),
	TRANSCEIVER_TXDISABLE_ATTR_ID(27),
	TRANSCEIVER_TXDISABLE_ATTR_ID(28),
	TRANSCEIVER_TXDISABLE_ATTR_ID(29),
	TRANSCEIVER_TXDISABLE_ATTR_ID(30),
	TRANSCEIVER_TXDISABLE_ATTR_ID(31),
	TRANSCEIVER_TXDISABLE_ATTR_ID(32),
	TRANSCEIVER_TXDISABLE_ATTR_ID(33),
	TRANSCEIVER_TXDISABLE_ATTR_ID(34),
	TRANSCEIVER_TXDISABLE_ATTR_ID(35),
	TRANSCEIVER_TXDISABLE_ATTR_ID(36),
	TRANSCEIVER_TXDISABLE_ATTR_ID(37),
	TRANSCEIVER_TXDISABLE_ATTR_ID(38),
	TRANSCEIVER_TXDISABLE_ATTR_ID(39),
	TRANSCEIVER_TXDISABLE_ATTR_ID(40),
	TRANSCEIVER_TXDISABLE_ATTR_ID(41),
	TRANSCEIVER_TXDISABLE_ATTR_ID(42),
	TRANSCEIVER_TXDISABLE_ATTR_ID(43),
	TRANSCEIVER_TXDISABLE_ATTR_ID(44),
	TRANSCEIVER_TXDISABLE_ATTR_ID(45),
	TRANSCEIVER_TXDISABLE_ATTR_ID(46),
	TRANSCEIVER_TXDISABLE_ATTR_ID(47),
	TRANSCEIVER_TXDISABLE_ATTR_ID(48),
	TRANSCEIVER_TXDISABLE_ATTR_ID(49),
	TRANSCEIVER_TXDISABLE_ATTR_ID(50),
	TRANSCEIVER_TXDISABLE_ATTR_ID(51),
	TRANSCEIVER_TXDISABLE_ATTR_ID(52),
	TRANSCEIVER_TXDISABLE_ATTR_ID(53),
	TRANSCEIVER_TXDISABLE_ATTR_ID(54),
	TRANSCEIVER_TXDISABLE_ATTR_ID(55),
	TRANSCEIVER_TXDISABLE_ATTR_ID(56),
	TRANSCEIVER_TXDISABLE_ATTR_ID(57),
	TRANSCEIVER_TXDISABLE_ATTR_ID(58),
	TRANSCEIVER_TXDISABLE_ATTR_ID(59),
	TRANSCEIVER_TXDISABLE_ATTR_ID(60),
	TRANSCEIVER_TXDISABLE_ATTR_ID(61),
	TRANSCEIVER_TXDISABLE_ATTR_ID(62),
	TRANSCEIVER_TXDISABLE_ATTR_ID(63),
	TRANSCEIVER_TXDISABLE_ATTR_ID(64),
	TRANSCEIVER_TXDISABLE_ATTR_ID(67),
	TRANSCEIVER_TXDISABLE_ATTR_ID(68),
	TRANSCEIVER_TXDISABLE_ATTR_ID(69),
	TRANSCEIVER_TXDISABLE_ATTR_ID(70),
	TRANSCEIVER_TXDISABLE_ATTR_ID(71),
	TRANSCEIVER_TXDISABLE_ATTR_ID(72),
	TRANSCEIVER_TXDISABLE_ATTR_ID(73),
	TRANSCEIVER_TXDISABLE_ATTR_ID(74),
};

/* sysfs attributes for hwmon 
 */
static ssize_t show_status(struct device *dev, struct device_attribute *da,
             char *buf);
static ssize_t set_tx_disable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t access(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t show_version(struct device *dev, struct device_attribute *da,
             char *buf);
static int asgvolt64_cpld_read_internal(struct i2c_client *client, u8 reg);
static int asgvolt64_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value);

/* sfp transceiver attributes */
#define DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
    static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, show_status, NULL, MODULE_PRESENT_##index); \
	static SENSOR_DEVICE_ATTR(module_tx_disable_##index, S_IRUGO | S_IWUSR, show_status, set_tx_disable, MODULE_TXDISABLE_##index); \
	static SENSOR_DEVICE_ATTR(module_rx_los_##index, S_IRUGO, show_status, NULL, MODULE_RXLOS_##index);  \
	static SENSOR_DEVICE_ATTR(module_tx_fault_##index, S_IRUGO, show_status, NULL, MODULE_TXFAULT_##index); 
	
#define DECLARE_SFP_TRANSCEIVER_ATTR(index)  \
    &sensor_dev_attr_module_present_##index.dev_attr.attr, \
	&sensor_dev_attr_module_tx_disable_##index.dev_attr.attr, \
	&sensor_dev_attr_module_rx_los_##index.dev_attr.attr,     \
	&sensor_dev_attr_module_tx_fault_##index.dev_attr.attr

/* qsfp transceiver attributes */
#define DECLARE_QSFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
    static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, show_status, NULL, MODULE_PRESENT_##index);

#define DECLARE_QSFP_TRANSCEIVER_ATTR(index)  \
    &sensor_dev_attr_module_present_##index.dev_attr.attr
    
/*gpon transceiver attributes*/
#define DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
    static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, show_status, NULL, MODULE_PRESENT_##index); \
	static SENSOR_DEVICE_ATTR(module_tx_disable_##index, S_IRUGO | S_IWUSR, show_status, set_tx_disable, MODULE_TXDISABLE_##index); \	
	static SENSOR_DEVICE_ATTR(module_tx_fault_##index, S_IRUGO, show_status, NULL, MODULE_TXFAULT_##index); 
	
#define DECLARE_GPON_TRANSCEIVER_ATTR(index)  \
    &sensor_dev_attr_module_present_##index.dev_attr.attr, \
	&sensor_dev_attr_module_tx_disable_##index.dev_attr.attr, \
	&sensor_dev_attr_module_tx_fault_##index.dev_attr.attr

static SENSOR_DEVICE_ATTR(version, S_IRUGO, show_version, NULL, CPLD_VERSION);
static SENSOR_DEVICE_ATTR(access, S_IWUSR, NULL, access, ACCESS);



/* transceiver attributes */
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(1);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(2);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(3);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(4);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(5);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(6);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(7);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(8);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(9);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(10);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(11);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(12);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(13);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(14);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(15);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(16);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(17);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(18);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(19);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(20);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(21);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(22);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(23);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(24);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(25);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(26);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(27);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(28);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(29);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(30);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(31);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(32);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(33);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(34);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(35);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(36);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(37);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(38);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(39);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(40);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(41);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(42);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(43);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(44);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(45);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(46);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(47);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(48);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(49);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(50);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(51);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(52);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(53);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(54);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(55);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(56);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(57);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(58);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(59);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(60);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(61);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(62);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(63);
DECLARE_GPON_TRANSCEIVER_SENSOR_DEVICE_ATTR(64);

DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(67);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(68);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(69);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(70);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(71);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(72);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(73);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(74);

DECLARE_QSFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(65);
DECLARE_QSFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(66);

static struct attribute *asgvolt64_cpld1_attributes[] = {
    &sensor_dev_attr_version.dev_attr.attr,
    &sensor_dev_attr_access.dev_attr.attr,
    /*xfp*/
	DECLARE_GPON_TRANSCEIVER_ATTR(1),
	DECLARE_GPON_TRANSCEIVER_ATTR(2),
	DECLARE_GPON_TRANSCEIVER_ATTR(3),
	DECLARE_GPON_TRANSCEIVER_ATTR(4),
	DECLARE_GPON_TRANSCEIVER_ATTR(5),
	DECLARE_GPON_TRANSCEIVER_ATTR(6),
	DECLARE_GPON_TRANSCEIVER_ATTR(7),
	DECLARE_GPON_TRANSCEIVER_ATTR(8),
	DECLARE_GPON_TRANSCEIVER_ATTR(9),
	DECLARE_GPON_TRANSCEIVER_ATTR(10),	
	DECLARE_GPON_TRANSCEIVER_ATTR(11),
	DECLARE_GPON_TRANSCEIVER_ATTR(12),
	DECLARE_GPON_TRANSCEIVER_ATTR(13),
	DECLARE_GPON_TRANSCEIVER_ATTR(14),
	DECLARE_GPON_TRANSCEIVER_ATTR(15),
	DECLARE_GPON_TRANSCEIVER_ATTR(16),
	DECLARE_GPON_TRANSCEIVER_ATTR(17),
	DECLARE_GPON_TRANSCEIVER_ATTR(18),
	DECLARE_GPON_TRANSCEIVER_ATTR(21),
	DECLARE_GPON_TRANSCEIVER_ATTR(22),
	DECLARE_GPON_TRANSCEIVER_ATTR(25),
	DECLARE_GPON_TRANSCEIVER_ATTR(26),
	/*100g qsfp*/
	DECLARE_QSFP_TRANSCEIVER_ATTR(65),
	DECLARE_QSFP_TRANSCEIVER_ATTR(66),
	NULL
};

static struct attribute *asgvolt64_cpld2_attributes[] = {
    &sensor_dev_attr_version.dev_attr.attr,
    &sensor_dev_attr_access.dev_attr.attr,
    /*xfp*/
    DECLARE_GPON_TRANSCEIVER_ATTR(19),
	DECLARE_GPON_TRANSCEIVER_ATTR(20),
	DECLARE_GPON_TRANSCEIVER_ATTR(23),
	DECLARE_GPON_TRANSCEIVER_ATTR(24),
	DECLARE_GPON_TRANSCEIVER_ATTR(26),
	DECLARE_GPON_TRANSCEIVER_ATTR(27),
	DECLARE_GPON_TRANSCEIVER_ATTR(28),
	DECLARE_GPON_TRANSCEIVER_ATTR(29),
	DECLARE_GPON_TRANSCEIVER_ATTR(30),
	DECLARE_GPON_TRANSCEIVER_ATTR(31),
	DECLARE_GPON_TRANSCEIVER_ATTR(32),
	DECLARE_GPON_TRANSCEIVER_ATTR(33),
	DECLARE_GPON_TRANSCEIVER_ATTR(34),
	DECLARE_GPON_TRANSCEIVER_ATTR(37),
	DECLARE_GPON_TRANSCEIVER_ATTR(38),
	DECLARE_GPON_TRANSCEIVER_ATTR(41),
	DECLARE_GPON_TRANSCEIVER_ATTR(42),
	DECLARE_GPON_TRANSCEIVER_ATTR(44),
	DECLARE_GPON_TRANSCEIVER_ATTR(45),
	DECLARE_GPON_TRANSCEIVER_ATTR(46),
	DECLARE_GPON_TRANSCEIVER_ATTR(47),
	DECLARE_GPON_TRANSCEIVER_ATTR(48),
	/*25g sfp*/
	DECLARE_SFP_TRANSCEIVER_ATTR(67),
	DECLARE_SFP_TRANSCEIVER_ATTR(68),
	DECLARE_SFP_TRANSCEIVER_ATTR(69),
	DECLARE_SFP_TRANSCEIVER_ATTR(70),
	DECLARE_SFP_TRANSCEIVER_ATTR(71),
	DECLARE_SFP_TRANSCEIVER_ATTR(72),
	DECLARE_SFP_TRANSCEIVER_ATTR(73),
	DECLARE_SFP_TRANSCEIVER_ATTR(74),
	
	NULL
};

static struct attribute *asgvolt64_cpld3_attributes[] = {
    &sensor_dev_attr_version.dev_attr.attr,
    &sensor_dev_attr_access.dev_attr.attr,
    /*xfp*/
    DECLARE_GPON_TRANSCEIVER_ATTR(35),	
	DECLARE_GPON_TRANSCEIVER_ATTR(36),
    DECLARE_GPON_TRANSCEIVER_ATTR(39),
	DECLARE_GPON_TRANSCEIVER_ATTR(40),
	DECLARE_GPON_TRANSCEIVER_ATTR(43),	
    DECLARE_GPON_TRANSCEIVER_ATTR(49),
	DECLARE_GPON_TRANSCEIVER_ATTR(50),
	DECLARE_GPON_TRANSCEIVER_ATTR(51),
	DECLARE_GPON_TRANSCEIVER_ATTR(52),
	DECLARE_GPON_TRANSCEIVER_ATTR(53),
	DECLARE_GPON_TRANSCEIVER_ATTR(54),
	DECLARE_GPON_TRANSCEIVER_ATTR(55),
	DECLARE_GPON_TRANSCEIVER_ATTR(56),
	DECLARE_GPON_TRANSCEIVER_ATTR(57),
	DECLARE_GPON_TRANSCEIVER_ATTR(58),
	DECLARE_GPON_TRANSCEIVER_ATTR(59),
	DECLARE_GPON_TRANSCEIVER_ATTR(60),
	DECLARE_GPON_TRANSCEIVER_ATTR(61),
	DECLARE_GPON_TRANSCEIVER_ATTR(62),
	DECLARE_GPON_TRANSCEIVER_ATTR(63),
	DECLARE_GPON_TRANSCEIVER_ATTR(64),
	NULL
};

static const struct attribute_group asgvolt64_cpld1_group = {
	.attrs = asgvolt64_cpld1_attributes,
};


static const struct attribute_group asgvolt64_cpld2_group = {
	.attrs = asgvolt64_cpld2_attributes,
};

static const struct attribute_group asgvolt64_cpld3_group = {
	.attrs = asgvolt64_cpld3_attributes,
};


static ssize_t show_status(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct asgvolt64_cpld_data *data = i2c_get_clientdata(client);
    int status = 0;
    u8 reg = 0, mask = 0, revert = 0, addr=0x60;
    
    switch (attr->index)
    {
        /*100g QSFP*/
        case MODULE_PRESENT_65 ... MODULE_PRESENT_66:
            addr=0x60;
            reg=0x34;
            mask = 0x1 << (attr->index- MODULE_PRESENT_65);
            break;
        /*25g sfp*/
        case MODULE_PRESENT_67 ... MODULE_PRESENT_74:
            addr=0x61;
            reg=0x9;
            mask = 0x1 << (attr->index- MODULE_PRESENT_67);
            break;
        case MODULE_TXFAULT_67 ... MODULE_TXFAULT_74:
            addr=0x61;
            reg=0x17;
            mask = 0x1 << (attr->index- MODULE_TXFAULT_67);
            break;
        case MODULE_TXDISABLE_67 ... MODULE_TXDISABLE_74:
            reg=0x25;
            addr=0x61;
            mask = 0x1 << (attr->index- MODULE_TXFAULT_67);
            break;
        case MODULE_RXLOS_67 ... MODULE_RXLOS_74:
            reg=0x43;
            addr=0x61;
            mask = 0x1 << (attr->index- MODULE_RXLOS_67);
            break;
                        
        /*gpon*/
        /*cpld1:present_1, 0x3*/
        case MODULE_PRESENT_1 ... MODULE_PRESENT_2:
            addr=0x60;
            reg=0x3;
            mask=0x1 << (attr->index-MODULE_PRESENT_1);
            break;
        case MODULE_PRESENT_5 ... MODULE_PRESENT_6:
            addr=0x60;
            reg=0x3;
            mask=0x1 << (attr->index-MODULE_PRESENT_5 + 2);
            break;
        case MODULE_PRESENT_9 ... MODULE_PRESENT_10:
            addr=0x60;
            reg=0x3;
            mask=0x1 << (attr->index-MODULE_PRESENT_9 + 4);
            break;
        case MODULE_PRESENT_13 ... MODULE_PRESENT_14:
            addr=0x60;
            reg=0x3;
            mask=0x1 << (attr->index-MODULE_PRESENT_13 + 6);
            break;
        /*cpld1:present_2, 0x5, */    
        case MODULE_PRESENT_15 ... MODULE_PRESENT_16:
            addr=0x60;
            reg=0x5;
            mask=0x1 << ((attr->index==MODULE_PRESENT_15)?1:0);
            break;
        case MODULE_PRESENT_11 ... MODULE_PRESENT_12:
            addr=0x60;
            reg=0x5;
            mask=0x1 << ((attr->index==MODULE_PRESENT_11)?3:2);
            break;
        case MODULE_PRESENT_7 ... MODULE_PRESENT_8:
            addr=0x60;
            reg=0x5;
            mask=0x1 << ((attr->index==MODULE_PRESENT_7)?5:4);
            break;
        case MODULE_PRESENT_3 ... MODULE_PRESENT_4:
            addr=0x60;
            reg=0x5;
            mask=0x1 << ((attr->index==MODULE_PRESENT_3)?7:6);
            break; 
        /*cpld1:present_3, 0x7*/    
        case MODULE_PRESENT_17 ... MODULE_PRESENT_18:
            addr=0x60;
            reg=0x7;
            mask=0x1 << ((attr->index==MODULE_PRESENT_17)?0:1);
            break;
        case MODULE_PRESENT_21 ... MODULE_PRESENT_22:
            addr=0x60;
            reg=0x7;
            mask=0x1 << ((attr->index==MODULE_PRESENT_21)?2:3);
            break;
        case MODULE_PRESENT_25 ... MODULE_PRESENT_26:
            addr=0x60;
            reg=0x7;
            mask=0x1 << ((attr->index==MODULE_PRESENT_25)?4:5);
            break; 
        
        /*cpld1:tx_fault_1,0x9*/ 
        case MODULE_TXFAULT_1 ... MODULE_TXFAULT_2:
            addr=0x60;
            reg=0x9;
            mask=0x1 << (attr->index==MODULE_TXFAULT_1?0:1);
            break;
        case MODULE_TXFAULT_5 ... MODULE_TXFAULT_6:
            addr=0x60;
            reg=0x9;
            mask=0x1 << (attr->index==MODULE_TXFAULT_5?2:3);
            break;        
        case MODULE_TXFAULT_9 ... MODULE_TXFAULT_10:
            addr=0x60;
            reg=0x9;
            mask=0x1 << (attr->index==MODULE_PRESENT_9?4:5);
            break;
        case MODULE_TXFAULT_13 ... MODULE_TXFAULT_14:
            addr=0x60;
            reg=0x9;
            mask=0x1 << (attr->index==MODULE_TXFAULT_13?6:7);
            break;
        /*cpld1:tx_fault_2, 0x11*/
         case MODULE_TXFAULT_15 ... MODULE_TXFAULT_16:
            addr=0x60;
            reg=0x11;
            mask=0x1 << ((attr->index==MODULE_TXFAULT_15)?1:0);
            break;
        case MODULE_TXFAULT_11 ... MODULE_TXFAULT_12:
            addr=0x60;
            reg=0x11;
            mask=0x1 << ((attr->index==MODULE_TXFAULT_11)?3:2);
            break;
        case MODULE_TXFAULT_7 ... MODULE_TXFAULT_8:
            addr=0x60;
            reg=0x11;
            mask=0x1 << ((attr->index==MODULE_TXFAULT_7)?5:4);
            break;
        case MODULE_TXFAULT_3 ... MODULE_TXFAULT_4:
            addr=0x60;
            reg=0x11;
            mask=0x1 << ((attr->index==MODULE_TXFAULT_3)?7:6);
            break;
        /*cpld1:tx_fault_3, 0x13*/
        case MODULE_TXFAULT_17 ... MODULE_TXFAULT_18:
            addr=0x60;
            reg=0x13;
            mask=0x1 << ((attr->index==MODULE_TXFAULT_17)?0:1);
            break;
        case MODULE_TXFAULT_21 ... MODULE_TXFAULT_22:
            addr=0x60;
            reg=0x13;
            mask=0x1 << ((attr->index==MODULE_TXFAULT_21)?2:3);
            break;
        case MODULE_TXFAULT_25 ... MODULE_TXFAULT_26:
            addr=0x60;
            reg=0x13;
            mask=0x1 << ((attr->index==MODULE_TXFAULT_25)?4:5);
            break;
            
        /*cpld1:tx_disable_1,0x15*/ 
        case MODULE_TXDISABLE_1 ... MODULE_TXDISABLE_2:
            addr=0x60;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_1?0:1);
            break;
        case MODULE_TXDISABLE_5 ... MODULE_TXDISABLE_6:
            addr=0x60;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_5?2:3);
            break;        
        case MODULE_TXDISABLE_9 ... MODULE_TXDISABLE_10:
            addr=0x60;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_9?4:5);
            break;
        case MODULE_TXDISABLE_13 ... MODULE_TXDISABLE_14:
            addr=0x60;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_13?6:7);
            break;
        /*cpld1:tx_disable_2, 0x17*/
         case MODULE_TXDISABLE_15 ... MODULE_TXDISABLE_16:
            addr=0x60;
            reg=0x17;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_15)?1:0);
            break;
        case MODULE_TXDISABLE_11 ... MODULE_TXDISABLE_12:
            addr=0x60;
            reg=0x17;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_11)?3:2);
            break;
        case MODULE_TXDISABLE_7 ... MODULE_TXDISABLE_8:
            addr=0x60;
            reg=0x17;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_7)?5:4);
            break;
        case MODULE_TXDISABLE_3 ... MODULE_TXDISABLE_4:
            addr=0x60;
            reg=0x17;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_3)?7:6);
            break;
        /*cpld1:tx_disable_3, 0x19*/
        case MODULE_TXDISABLE_17 ... MODULE_TXDISABLE_18:
            addr=0x60;
            reg=0x19;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_17)?0:1);
            break;
        case MODULE_TXDISABLE_21 ... MODULE_TXDISABLE_22:
            addr=0x60;
            reg=0x19;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_21)?2:3);
            break;
        case MODULE_TXDISABLE_25 ... MODULE_TXDISABLE_26:
            addr=0x60;
            reg=0x19;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_25)?4:5);
            break;
        
        /*cpld2:present_1, 0x3*/
        case MODULE_PRESENT_29 ... MODULE_PRESENT_30:
            addr=0x61;
            reg=0x3;
            mask=0x1 << (attr->index==MODULE_PRESENT_29?0:1);
            break;
        case MODULE_PRESENT_31 ... MODULE_PRESENT_32:
            addr=0x61;
            reg=0x3;
            mask=0x1 << (attr->index==MODULE_PRESENT_31?3:2);
            break;
        case MODULE_PRESENT_27 ... MODULE_PRESENT_28:
            addr=0x61;
            reg=0x3;
            mask=0x1 << (attr->index==MODULE_PRESENT_27?5:4);
            break;
        case MODULE_PRESENT_23 ... MODULE_PRESENT_24:
            addr=0x61;
            reg=0x3;
            mask=0x1 << (attr->index==MODULE_PRESENT_23?7:6);
            break;
        /*cpld2:present_2, 0x5*/
        case MODULE_PRESENT_19 ... MODULE_PRESENT_20:
            addr=0x61;
            reg=0x5;
            mask=0x1 << (attr->index==MODULE_PRESENT_19?1:0);
            break;
        case MODULE_PRESENT_33 ... MODULE_PRESENT_34:
            addr=0x61;
            reg=0x5;
            mask=0x1 << (attr->index==MODULE_PRESENT_33?2:3);
            break;
        case MODULE_PRESENT_37 ... MODULE_PRESENT_38:
            addr=0x61;
            reg=0x5;
            mask=0x1 << (attr->index==MODULE_PRESENT_37?4:5);
            break;
        case MODULE_PRESENT_41 ... MODULE_PRESENT_42:
            addr=0x61;
            reg=0x5;
            mask=0x1 << (attr->index==MODULE_PRESENT_41?6:7);
            break;
        /*cpld2:present_3, 0x7*/
        case MODULE_PRESENT_45 ... MODULE_PRESENT_46:
            addr=0x61;
            reg=0x7;
            mask=0x1 << (attr->index==MODULE_PRESENT_45?0:1);
            break;
        case MODULE_PRESENT_47 ... MODULE_PRESENT_48:
            addr=0x61;
            reg=0x7;
            mask=0x1 << (attr->index==MODULE_PRESENT_47?3:2);
            break;
        case MODULE_PRESENT_44:
            addr=0x61;
            reg=0x7;
            mask=0x1 << 4;
            break;
        
        /*cpld2:tx_fault_1, 0x11*/
        case MODULE_TXFAULT_29 ... MODULE_TXFAULT_30:
            addr=0x61;
            reg=0x11;
            mask=0x1 << (attr->index==MODULE_TXFAULT_29?0:1);
            break;
        case MODULE_TXFAULT_31 ... MODULE_TXFAULT_32:
            addr=0x61;
            reg=0x11;
            mask=0x1 << (attr->index==MODULE_TXFAULT_31?3:2);
            break;
        case MODULE_TXFAULT_27 ... MODULE_TXFAULT_28:
            addr=0x61;
            reg=0x11;
            mask=0x1 << (attr->index==MODULE_TXFAULT_27?5:4);
            break;
        case MODULE_TXFAULT_23 ... MODULE_TXFAULT_24:
            addr=0x61;
            reg=0x11;
            mask=0x1 << (attr->index==MODULE_TXFAULT_23?7:6);
            break;
         /*cpld2:tx_fault_2, 0x13*/
        case MODULE_TXFAULT_19 ... MODULE_TXFAULT_20:
            addr=0x61;
            reg=0x13;
            mask=0x1 << (attr->index==MODULE_TXFAULT_19?1:0);
            break;
        case MODULE_TXFAULT_33 ... MODULE_TXFAULT_34:
            addr=0x61;
            reg=0x13;
            mask=0x1 << (attr->index==MODULE_TXFAULT_33?2:3);
            break;
        case MODULE_TXFAULT_37 ... MODULE_TXFAULT_38:
            addr=0x61;
            reg=0x13;
            mask=0x1 << (attr->index==MODULE_TXFAULT_37?4:5);
            break;
        case MODULE_TXFAULT_41 ... MODULE_TXFAULT_42:
            addr=0x61;
            reg=0x13;
            mask=0x1 << (attr->index==MODULE_TXFAULT_41?6:7);
            break;
        /*cpld2:tx_fault_3, 0x15*/
        case MODULE_TXFAULT_45 ... MODULE_TXFAULT_46:
            addr=0x61;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXFAULT_45?0:1);
            break;
        case MODULE_TXFAULT_47 ... MODULE_TXFAULT_48:
            addr=0x61;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXFAULT_47?3:2);
            break;
        case MODULE_TXFAULT_44:
            addr=0x61;
            reg=0x15;
            mask=0x1 << 4;
            break;
        
        /*cpld2:tx_disable_1, 0x19*/
         case MODULE_TXDISABLE_29 ... MODULE_TXDISABLE_30:
            addr=0x61;
            reg=0x19;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_29?0:1);
            break;
        case MODULE_TXDISABLE_31 ... MODULE_TXDISABLE_32:
            addr=0x61;
            reg=0x19;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_31?3:2);
            break;
        case MODULE_TXDISABLE_27 ... MODULE_TXDISABLE_28:
            addr=0x61;
            reg=0x19;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_27?5:4);
            break;
        case MODULE_TXDISABLE_23 ... MODULE_TXDISABLE_24:
            addr=0x61;
            reg=0x19;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_23?7:6);
            break;
        /*cpld2:tx_disable_2, 0x21*/
        case MODULE_TXDISABLE_19 ... MODULE_TXDISABLE_20:
            addr=0x61;
            reg=0x21;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_19?1:0);
            break;
        case MODULE_TXDISABLE_33 ... MODULE_TXDISABLE_34:
            addr=0x61;
            reg=0x21;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_33?2:3);
            break;
        case MODULE_TXDISABLE_37 ... MODULE_TXDISABLE_38:
            addr=0x61;
            reg=0x21;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_37?4:5);
            break;
        case MODULE_TXDISABLE_41 ... MODULE_TXDISABLE_42:
            addr=0x61;
            reg=0x21;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_41?6:7);
            break;
        /*cpld2:tx_disable_3, 0x23*/
        case MODULE_TXDISABLE_45 ... MODULE_TXDISABLE_46:
            addr=0x61;
            reg=0x23;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_45?0:1);
            break;
        case MODULE_TXDISABLE_47 ... MODULE_TXDISABLE_48:
            addr=0x61;
            reg=0x23;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_47?3:2);
            break;
        case MODULE_TXDISABLE_44:
            addr=0x61;
            reg=0x23;
            mask=0x1 << 4;
            break;
          
        /*cpld3:present_1, 0x3*/
        case MODULE_PRESENT_43:
            addr=0x62;
            reg=0x3;
            mask=0x1;
            break;
        case MODULE_PRESENT_39 ... MODULE_PRESENT_40:
            addr=0x62;
            reg=0x3;
            mask=0x1 << (attr->index==MODULE_PRESENT_39?2:1);
            break;
        case MODULE_PRESENT_35 ... MODULE_PRESENT_36:
            addr=0x62;
            reg=0x3;
            mask=0x1 << (attr->index==MODULE_PRESENT_35?4:3);
            break;
        case MODULE_PRESENT_49 ... MODULE_PRESENT_50:
            addr=0x62;
            reg=0x3;
            mask=0x1 << (attr->index==MODULE_PRESENT_49?5:6);
            break;
        case MODULE_PRESENT_53:
            addr=0x62;
            reg=0x3;
            mask=0x1 <<7;
            break;
        /*cpld3:present_2, 0x5*/
        case MODULE_PRESENT_54:
            addr=0x62;
            reg=0x5;
            mask=0x1;
            break;
        case MODULE_PRESENT_57 ... MODULE_PRESENT_58:
            addr=0x62;
            reg=0x5;
            mask=0x1 << (attr->index==MODULE_PRESENT_57?1:2);
            break;
        case MODULE_PRESENT_61 ... MODULE_PRESENT_62:
            addr=0x62;
            reg=0x5;
            mask=0x1 << (attr->index==MODULE_PRESENT_61?3:4);
            break;
        case MODULE_PRESENT_63 ... MODULE_PRESENT_64:
            addr=0x62;
            reg=0x5;
            mask=0x1 << (attr->index==MODULE_PRESENT_63?6:5);
            break;
        case MODULE_PRESENT_60:
            addr=0x62;
            reg=0x5;
            mask=0x1 <<7;
            break;
        /*cpld3:present_3, 0x7*/
        case MODULE_PRESENT_59:
            addr=0x62;
            reg=0x7;
            mask=0x1;
            break;
        case MODULE_PRESENT_55 ... MODULE_PRESENT_56:
            addr=0x62;
            reg=0x7;
            mask=0x1 << (attr->index==MODULE_PRESENT_55?2:1);
            break;
        case MODULE_PRESENT_51 ... MODULE_PRESENT_52:
            addr=0x62;
            reg=0x7;
            mask=0x1 << (attr->index==MODULE_PRESENT_51?4:3);
            break;
            
        /*cpld3:tx_fault_1, 0x9*/
        case MODULE_TXFAULT_43:
            addr=0x62;
            reg=0x9;
            mask=0x1;
            break;
        case MODULE_TXFAULT_39 ... MODULE_TXFAULT_40:
            addr=0x62;
            reg=0x9;
            mask=0x1 << (attr->index==MODULE_TXFAULT_39?2:1);
            break;
        case MODULE_TXFAULT_35 ... MODULE_TXFAULT_36:
            addr=0x62;
            reg=0x9;
            mask=0x1 << (attr->index==MODULE_TXFAULT_35?4:3);
            break;
        case MODULE_TXFAULT_49 ... MODULE_TXFAULT_50:
            addr=0x62;
            reg=0x9;
            mask=0x1 << (attr->index==MODULE_TXFAULT_49?5:6);
            break;
        case MODULE_TXFAULT_53:
            addr=0x62;
            reg=0x9;
            mask=0x1 <<7;
            break;
        /*cpld3:tx_fault_2, 0x11*/
        case MODULE_TXFAULT_54:
            addr=0x62;
            reg=0x11;
            mask=0x1;
            break;
        case MODULE_TXFAULT_57 ... MODULE_TXFAULT_58:
            addr=0x62;
            reg=0x11;
            mask=0x1 << (attr->index==MODULE_TXFAULT_57?1:2);
            break;
        case MODULE_TXFAULT_61 ... MODULE_TXFAULT_62:
            addr=0x62;
            reg=0x11;
            mask=0x1 << (attr->index==MODULE_TXFAULT_61?3:4);
            break;
        case MODULE_TXFAULT_63 ... MODULE_TXFAULT_64:
            addr=0x62;
            reg=0x11;
            mask=0x1 << (attr->index==MODULE_TXFAULT_63?6:5);
            break;
        case MODULE_TXFAULT_60:
            addr=0x62;
            reg=0x11;
            mask=0x1 <<7;
            break;
        /*cpld3:tx_fault_3, 0x13*/
        case MODULE_TXFAULT_59:
            addr=0x62;
            reg=0x13;
            mask=0x1;
            break;
        case MODULE_TXFAULT_55 ... MODULE_TXFAULT_56:
            addr=0x62;
            reg=0x13;
            mask=0x1 << (attr->index==MODULE_TXFAULT_55?2:1);
            break;
        case MODULE_TXFAULT_51 ... MODULE_TXFAULT_52:
            addr=0x62;
            reg=0x13;
            mask=0x1 << (attr->index==MODULE_TXFAULT_51?4:3);
            break;
                      
        /*cpld3:tx_disable_1, 0x15*/
        case MODULE_TXDISABLE_43:
            addr=0x62;
            reg=0x15;
            mask=0x1;
            break;
        case MODULE_TXDISABLE_39 ... MODULE_TXDISABLE_40:
            addr=0x62;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_39?2:1);
            break;
        case MODULE_TXDISABLE_35 ... MODULE_TXDISABLE_36:
            addr=0x62;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_35?4:3);
            break;
        case MODULE_TXDISABLE_49 ... MODULE_TXDISABLE_50:
            addr=0x62;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_49?5:6);
            break;
        case MODULE_TXDISABLE_53:
            addr=0x62;
            reg=0x15;
            mask=0x1 <<7;
            break;
        /*cpld3:tx_disable_2, 0x17*/
        case MODULE_TXDISABLE_54:
            addr=0x62;
            reg=0x17;
            mask=0x1;
            break;
        case MODULE_TXDISABLE_57 ... MODULE_TXDISABLE_58:
            addr=0x62;
            reg=0x17;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_57?1:2);
            break;
        case MODULE_TXDISABLE_61 ... MODULE_TXDISABLE_62:
            addr=0x62;
            reg=0x17;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_61?3:4);
            break;
        case MODULE_TXDISABLE_63 ... MODULE_TXDISABLE_64:
            addr=0x62;
            reg=0x17;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_63?6:5);
            break;
        case MODULE_TXDISABLE_60:
            addr=0x62;
            reg=0x17;
            mask=0x1 <<7;
            break;
        /*cpld3:tx_disable_3, 0x19*/ 
        case MODULE_TXDISABLE_59:
            addr=0x62;
            reg=0x19;
            mask=0x1;
            break;
        case MODULE_TXDISABLE_55 ... MODULE_TXDISABLE_56:
            addr=0x62;
            reg=0x19;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_55?2:1);
            break;
        case MODULE_TXDISABLE_51 ... MODULE_TXDISABLE_52:
            addr=0x62;
            reg=0x19;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_51?4:3);
            break;
	    default:
		    return 0;
    }

    if( attr->index >= MODULE_PRESENT_1 && attr->index <= MODULE_PRESENT_74 )        
    {
        revert = 1;
    }

    mutex_lock(&data->update_lock);
	status = asgvolt64_cpld_read_internal(client, reg);
	if (unlikely(status < 0)) {
		goto exit;
	}
	    
	mutex_unlock(&data->update_lock);
	    
	return sprintf(buf, "%d\n", revert ? !(status & mask) : !!(status & mask));

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t set_tx_disable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct asgvolt64_cpld_data *data = i2c_get_clientdata(client);
	long disable;
	int status;
    u8 reg = 0, mask = 0, addr=0x60;
     
	status = kstrtol(buf, 10, &disable);
	if (status) {
		//return status;
		status=1;
	}
	else
	    status=0;
    
    switch (attr->index)
    {
         /*25g sfp*/
         case MODULE_TXDISABLE_67 ... MODULE_TXDISABLE_74:
            addr=0x61;
            reg=0x25;
            mask=0x1 << (attr->index - MODULE_TXDISABLE_67);
            break;
        
        /*gpon*/
        /*cpld1:tx_disable_1,0x15*/
        case MODULE_TXDISABLE_1 ... MODULE_TXDISABLE_2:
            addr=0x60;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_1?0:1);
            break;
        case MODULE_TXDISABLE_5 ... MODULE_TXDISABLE_6:
            addr=0x60;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_5?2:3);
            break;        
        case MODULE_TXDISABLE_9 ... MODULE_TXDISABLE_10:
            addr=0x60;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_9?4:5);
            break;
        case MODULE_TXDISABLE_13 ... MODULE_TXDISABLE_14:
            addr=0x60;
            reg=0x15;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_13?6:7);
            break;
         /*cpld1:tx_disable_2, 0x17*/
        case MODULE_TXDISABLE_15 ... MODULE_TXDISABLE_16:
            addr=0x60;
            reg=0x17;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_15)?1:0);
            break;
        case MODULE_TXDISABLE_11 ... MODULE_TXDISABLE_12:
            addr=0x60;
            reg=0x17;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_11)?3:2);
            break;
        case MODULE_TXDISABLE_7 ... MODULE_TXDISABLE_8:
            addr=0x60;
            reg=0x17;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_7)?5:4);
            break;
        case MODULE_TXDISABLE_3 ... MODULE_TXDISABLE_4:
            addr=0x60;
            reg=0x17;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_3)?7:6);
            break;
        /*cpld1:tx_disable_3, 0x19*/
        case MODULE_TXDISABLE_17 ... MODULE_TXDISABLE_18:
            addr=0x60;
            reg=0x19;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_17)?0:1);
            break;
        case MODULE_TXDISABLE_21 ... MODULE_TXDISABLE_22:
            addr=0x60;
            reg=0x19;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_21)?2:3);
            break;
        case MODULE_TXDISABLE_25 ... MODULE_TXDISABLE_26:
            addr=0x60;
            reg=0x19;
            mask=0x1 << ((attr->index==MODULE_TXDISABLE_25)?4:5);
            break;
            
         /*cpld2:tx_disable_1, 0x19*/
         case MODULE_TXDISABLE_29 ... MODULE_TXDISABLE_30:
            addr=0x61;
            reg=0x19;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_29?0:1);
            break;
        case MODULE_TXDISABLE_31 ... MODULE_TXDISABLE_32:
            addr=0x61;
            reg=0x19;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_31?3:2);
            break;
        case MODULE_TXDISABLE_27 ... MODULE_TXDISABLE_28:
            addr=0x61;
            reg=0x19;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_27?5:4);
            break;
        case MODULE_TXDISABLE_23 ... MODULE_TXDISABLE_24:
            addr=0x61;
            reg=0x19;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_23?7:6);
            break;
        /*cpld2:tx_disable_2, 0x21*/
        case MODULE_TXDISABLE_19 ... MODULE_TXDISABLE_20:
            addr=0x61;
            reg=0x21;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_19?1:0);
            break;
        case MODULE_TXDISABLE_33 ... MODULE_TXDISABLE_34:
            addr=0x61;
            reg=0x21;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_33?2:3);
            break;
        case MODULE_TXDISABLE_37 ... MODULE_TXDISABLE_38:
            addr=0x61;
            reg=0x21;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_37?4:5);
            break;
        case MODULE_TXDISABLE_41 ... MODULE_TXDISABLE_42:
            addr=0x61;
            reg=0x21;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_41?6:7);
            break;
        /*cpld2:tx_disable_3, 0x23*/
        case MODULE_TXDISABLE_45 ... MODULE_TXDISABLE_46:
            addr=0x61;
            reg=0x23;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_45?0:1);
            break;
        case MODULE_TXDISABLE_47 ... MODULE_TXDISABLE_48:
            addr=0x61;
            reg=0x23;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_47?3:2);
            break;
        case MODULE_TXDISABLE_44:
            addr=0x61;
            reg=0x23;
            mask=0x1 << 4;
            break;
	    default:
		    return 0;
    }

    /* Read current status */
    mutex_lock(&data->update_lock);
	status = asgvolt64_cpld_read_internal(client, reg);
	if (unlikely(status < 0)) {
		goto exit;
	}
	/* Update tx_disable status */
	if (disable) {
		status |= mask;
	}
	else {
	    
	    status &= ~mask;
	}
    status = asgvolt64_cpld_write_internal(client, reg, status);
	if (unlikely(status < 0)) {
		goto exit;
	}
    
    mutex_unlock(&data->update_lock);
    return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t access(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	int status;
	u32 addr, val;
    struct i2c_client *client = to_i2c_client(dev);
    struct asgvolt64_cpld_data *data = i2c_get_clientdata(client);
    
	if (sscanf(buf, "0x%x 0x%x", &addr, &val) != 2) {
		return -EINVAL;
	}

	if (addr > 0xFF || val > 0xFF) {
		return -EINVAL;
	}

	mutex_lock(&data->update_lock);
	status = asgvolt64_cpld_write_internal(client, addr, val);
	if (unlikely(status < 0)) {
		goto exit;
	}
	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static void asgvolt64_cpld_add_client(struct i2c_client *client)
{
    struct cpld_client_node *node = kzalloc(sizeof(struct cpld_client_node), GFP_KERNEL);

    if (!node) {
        dev_dbg(&client->dev, "Can't allocate cpld_client_node (0x%x)\n", client->addr);
        return;
    }

    node->client = client;

	mutex_lock(&list_lock);
    list_add(&node->list, &cpld_client_list);
	mutex_unlock(&list_lock);
}

static void asgvolt64_cpld_remove_client(struct i2c_client *client)
{
    struct list_head    *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int found = 0;

	mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client == client) {
            found = 1;
            break;
        }
    }

    if (found) {
        list_del(list_node);
        kfree(cpld_node);
    }

	mutex_unlock(&list_lock);
}

static ssize_t show_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0;
    struct i2c_client *client = to_i2c_client(dev);
	
	val = i2c_smbus_read_byte_data(client, 0x1);

    if (val < 0) {
        dev_dbg(&client->dev, "cpld(0x%x) reg(0x1) err %d\n", client->addr, val);
    }
	
    return sprintf(buf, "%d\n", val);
}


/*
 * I2C init/probing/exit functions
 */
static int asgvolt64_cpld_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adap = to_i2c_adapter(client->dev.parent);
	struct asgvolt64_cpld_data *data;
	int ret = -ENODEV;

	const struct attribute_group *group = NULL;

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE))
		goto exit;

	data = kzalloc(sizeof(struct asgvolt64_cpld_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
    mutex_init(&data->update_lock);
	data->type = id->driver_data;
	   
    /* Register sysfs hooks */
    switch (data->type)
    {    
        case asgvolt64_cpld1:
            group = &asgvolt64_cpld1_group;
            break;            
        case asgvolt64_cpld2:
            group = &asgvolt64_cpld2_group;
            break;
        case asgvolt64_cpld3:
            group = &asgvolt64_cpld3_group;
            break;
            
        default:
            break;
    }

    if (group)
    {
        ret = sysfs_create_group(&client->dev.kobj, group);
        if (ret) {
            goto exit_free;
        }
    }

    asgvolt64_cpld_add_client(client);
    return 0;

exit_free:
    kfree(data);
exit:
	return ret;
}

static int asgvolt64_cpld_remove(struct i2c_client *client)
{
    struct asgvolt64_cpld_data *data = i2c_get_clientdata(client);
    const struct attribute_group *group = NULL;

    asgvolt64_cpld_remove_client(client);

    /* Remove sysfs hooks */
    switch (data->type)
    {
        case asgvolt64_cpld1:
            group = &asgvolt64_cpld1_group;
            break;
        case asgvolt64_cpld2:
            group = &asgvolt64_cpld2_group;
            break;
        case asgvolt64_cpld3:
            group = &asgvolt64_cpld3_group;
            break;
        default:
            break;
    }

    if (group) {
        sysfs_remove_group(&client->dev.kobj, group);
    }

    kfree(data);

    return 0;
}

static int asgvolt64_cpld_read_internal(struct i2c_client *client, u8 reg)
{
	int status = 0, retry = I2C_RW_RETRY_COUNT;

	while (retry) {
		status = i2c_smbus_read_byte_data(client, reg);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

    return status;
}

static int asgvolt64_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value)
{
	int status = 0, retry = I2C_RW_RETRY_COUNT;
    
	while (retry) {
		status = i2c_smbus_write_byte_data(client, reg, value);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

    return status;
}

int asgvolt64_cpld_read(unsigned short cpld_addr, u8 reg)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EPERM;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client->addr == cpld_addr) {
            ret = asgvolt64_cpld_read_internal(cpld_node->client, reg);
    		break;
        }
    }

	mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(asgvolt64_cpld_read);

int asgvolt64_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EIO;
    
	mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client->addr == cpld_addr) {
            ret = asgvolt64_cpld_write_internal(cpld_node->client, reg, value);
            break;
        }
    }

	mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(asgvolt64_cpld_write);

static struct i2c_driver asgvolt64_cpld_driver = {
	.driver		= {
		.name	= "asgvolt64_cpld",
		.owner	= THIS_MODULE,
	},
	.probe		= asgvolt64_cpld_probe,
	.remove		= asgvolt64_cpld_remove,
	.id_table	= asgvolt64_cpld_id,
};

static int __init asgvolt64_cpld_init(void)
{
    mutex_init(&list_lock);
    return i2c_add_driver(&asgvolt64_cpld_driver);
}

static void __exit asgvolt64_cpld_exit(void)
{
    i2c_del_driver(&asgvolt64_cpld_driver);
}

MODULE_AUTHOR("Jostar Yang <jostar_yang@accton.com.tw>");
MODULE_DESCRIPTION("Accton I2C CPLD driver");
MODULE_LICENSE("GPL");

module_init(asgvolt64_cpld_init);
module_exit(asgvolt64_cpld_exit);

