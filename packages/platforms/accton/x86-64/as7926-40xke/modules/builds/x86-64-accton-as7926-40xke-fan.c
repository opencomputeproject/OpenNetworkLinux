/*
 * A hwmon driver for the Accton as7926-40xke fan
 *
 * Copyright (C) 2019  Edgecore Networks Corporation.
 * Jostar Yang <jostar_yang@edge-core.com>
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

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/dmi.h>

#define DRVNAME "as7926_40xke_fan"

extern int accton_i2c_cpld_read (unsigned short cpld_addr, u8 reg);
static struct as7926_40xke_fan_data *as7926_40xke_fan_update_device(struct device *dev);
static ssize_t fan_show_value(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);
static ssize_t set_temp(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);
extern int as7926_40xke_cpld_write(int bus_num, unsigned short cpld_addr, u8 reg, u8 value);

/* fan related data, the index should match sysfs_fan_attributes
 */
static const u8 fan_reg[] = {
    0x0E,       /* fan 1-8 present status in FAN board*/
    0x0F,       /* fan 9-12 present status in FAN board*/
    0x11,       /* fan PWM(for all fan) */
    0x12,       /* front fan 1 speed(rpm) */
    0x13,       /* front fan 2 speed(rpm) */
    0x14,       /* front fan 3 speed(rpm) */
    0x15,       /* front fan 4 speed(rpm) */
    0x16,       /* front fan 5 speed(rpm) */
    0x17,       /* front fan 6 speed(rpm) */
    0x18,       /* front fan 7 speed(rpm) */
    0x19,       /* front fan 8 speed(rpm) */
    0x1A,       /* front fan 9 speed(rpm) */
    0x1B,       /* front fan 10 speed(rpm) */
    0x1C,       /* front fan 11 speed(rpm) */
    0x1D,       /* front fan 12 speed(rpm) */
    0x22,       /* rear fan 1 speed(rpm) */
    0x23,       /* rear fan 2 speed(rpm) */
    0x24,       /* rear fan 3 speed(rpm) */
    0x25,       /* rear fan 4 speed(rpm) */
    0x26,       /* rear fan 5 speed(rpm) */
    0x27,       /* rear fan 6 speed(rpm) */
    0x28,       /* rear fan 7 speed(rpm) */
    0x29,       /* rear fan 8 speed(rpm) */
    0x2A,       /* rear fan 9 speed(rpm) */
    0x2B,       /* rear fan 10 speed(rpm) */
    0x2C,       /* rear fan 11 speed(rpm) */
    0x2D,       /* rear fan 12 speed(rpm) */
    0x50,       /* CPU temp */
    0x51,       /* Main Board Bottom ASIC Temp */
    0x52,       /* Main Board Bottom TMP432_1 Temp */
    0x53,       /* Main Board Bottom TMP432_2 Temp */
    0x54,       /* Main Board Bottom TMP432_3 Temp */
    0x55,       /* Main Board Bottom LM75_1 Temp */
    0x56,       /* Main Board Bottom LM75_2 Temp */
    0x57,       /* Main Board Bottom LM75_3 Temp */
    0x58,       /* Main Board Bottom LM75_4 Temp */
    0x60,       /* QSFP-DD Board LM75_1 Temp */
    0x61,       /* QSFP-DD Board LM75_2 Temp  */
    0x62,       /* QSFP-DD Board LM75_3 Temp  */
};

/* Each client has this additional data */
struct as7926_40xke_fan_data {
    struct device   *hwmon_dev;
    struct mutex     update_lock;
    char             valid;           /* != 0 if registers are valid */
    unsigned long    last_updated;    /* In jiffies */
    u8               reg_val[ARRAY_SIZE(fan_reg)]; /* Register value */
};

enum fan_id {
    FAN1_ID,
    FAN2_ID,
    FAN3_ID,
    FAN4_ID,
    FAN5_ID,
    FAN6_ID,
    FAN7_ID,
    FAN8_ID,
    FAN9_ID,
    FAN10_ID,
    FAN11_ID,
    FAN12_ID
};

enum sysfs_fan_attributes {
    FAN_PRESENT_REG1,
    FAN_PRESENT_REG2,
    FAN_DUTY_CYCLE_PERCENTAGE, /* Only one CPLD register to control duty cycle for all fans */
    FAN1_FRONT_SPEED_RPM,
    FAN2_FRONT_SPEED_RPM,
    FAN3_FRONT_SPEED_RPM,
    FAN4_FRONT_SPEED_RPM,
    FAN5_FRONT_SPEED_RPM,
    FAN6_FRONT_SPEED_RPM,
    FAN7_FRONT_SPEED_RPM,
    FAN8_FRONT_SPEED_RPM,
    FAN9_FRONT_SPEED_RPM,
    FAN10_FRONT_SPEED_RPM,
    FAN11_FRONT_SPEED_RPM,
    FAN12_FRONT_SPEED_RPM,
    FAN1_REAR_SPEED_RPM,
    FAN2_REAR_SPEED_RPM,
    FAN3_REAR_SPEED_RPM,
    FAN4_REAR_SPEED_RPM,
    FAN5_REAR_SPEED_RPM,
    FAN6_REAR_SPEED_RPM,
    FAN7_REAR_SPEED_RPM,
    FAN8_REAR_SPEED_RPM,
    FAN9_REAR_SPEED_RPM,
    FAN10_REAR_SPEED_RPM,
    FAN11_REAR_SPEED_RPM,
    FAN12_REAR_SPEED_RPM,
    TEMP1_INPUT,
    TEMP2_INPUT,
    TEMP3_INPUT,
    TEMP4_INPUT,
    TEMP5_INPUT,
    TEMP6_INPUT,
    TEMP7_INPUT,
    TEMP8_INPUT,
    TEMP9_INPUT,
    TEMP10_INPUT,
    TEMP11_INPUT,
    TEMP12_INPUT,
    FAN1_PRESENT,
    FAN2_PRESENT,
    FAN3_PRESENT,
    FAN4_PRESENT,
    FAN5_PRESENT,
    FAN6_PRESENT,
    FAN7_PRESENT,
    FAN8_PRESENT,
    FAN9_PRESENT,
    FAN10_PRESENT,
    FAN11_PRESENT,
    FAN12_PRESENT,
    FAN1_FAULT,
    FAN2_FAULT,
    FAN3_FAULT,
    FAN4_FAULT,
    FAN5_FAULT,
    FAN6_FAULT,
    FAN7_FAULT,
    FAN8_FAULT,
    FAN9_FAULT,
    FAN10_FAULT,
    FAN11_FAULT,
    FAN12_FAULT,
};

/* Define attributes
 */
#define DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(index, index2) \
    static SENSOR_DEVICE_ATTR(fan##index##_fault, S_IRUGO, fan_show_value, NULL, FAN##index##_FAULT);\
    static SENSOR_DEVICE_ATTR(fan##index2##_fault, S_IRUGO, fan_show_value, NULL, FAN##index##_FAULT)

#define DECLARE_FAN_FAULT_ATTR(index, index2)      &sensor_dev_attr_fan##index##_fault.dev_attr.attr, \
                                                   &sensor_dev_attr_fan##index2##_fault.dev_attr.attr

#define DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_duty_cycle_percentage, S_IWUSR | S_IRUGO, fan_show_value, set_duty_cycle, FAN##index##_DUTY_CYCLE_PERCENTAGE)
#define DECLARE_FAN_DUTY_CYCLE_ATTR(index) &sensor_dev_attr_fan##index##_duty_cycle_percentage.dev_attr.attr

#define DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_present, S_IRUGO, fan_show_value, NULL, FAN##index##_PRESENT)
#define DECLARE_FAN_PRESENT_ATTR(index)      &sensor_dev_attr_fan##index##_present.dev_attr.attr

#define DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(index, index2) \
    static SENSOR_DEVICE_ATTR(fan##index##_front_speed_rpm, S_IRUGO, fan_show_value, NULL, FAN##index##_FRONT_SPEED_RPM);\
    static SENSOR_DEVICE_ATTR(fan##index##_rear_speed_rpm, S_IRUGO, fan_show_value, NULL, FAN##index##_REAR_SPEED_RPM);\
    static SENSOR_DEVICE_ATTR(fan##index##_input, S_IRUGO, fan_show_value, NULL, FAN##index##_FRONT_SPEED_RPM);\
    static SENSOR_DEVICE_ATTR(fan##index2##_input, S_IRUGO, fan_show_value, NULL, FAN##index##_REAR_SPEED_RPM)
#define DECLARE_FAN_SPEED_RPM_ATTR(index, index2)  &sensor_dev_attr_fan##index##_front_speed_rpm.dev_attr.attr, \
                                                   &sensor_dev_attr_fan##index##_rear_speed_rpm.dev_attr.attr,  \
                                                   &sensor_dev_attr_fan##index##_input.dev_attr.attr,  \
                                                   &sensor_dev_attr_fan##index2##_input.dev_attr.attr

#define DECLARE_TEMP_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(temp##index##_input, S_IRUGO, fan_show_value, NULL, TEMP##index##_INPUT)
#define DECLARE_TEMP_SENSOR_ATTR(index)      &sensor_dev_attr_temp##index##_input.dev_attr.attr

#define DECLARE_CPU_MAC_TEMP_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(temp##index##_input, S_IWUSR | S_IRUGO, fan_show_value, set_temp, TEMP##index##_INPUT)
#define DECLARE_CPU_MAC_TEMP_SENSOR_ATTR(index)      &sensor_dev_attr_temp##index##_input.dev_attr.attr

/* 10 fan fault attributes in this platform */
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(1, 13);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(2, 14);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(3, 15);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(4, 16);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(5, 17);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(6, 18);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(7, 19);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(8, 20);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(9, 21);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(10, 22);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(11, 23);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(12, 24);
/* 10 fan speed(rpm) attributes in this platform */
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(1, 13);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(2, 14);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(3, 15);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(4, 16);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(5, 17);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(6, 18);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(7, 19);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(8, 20);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(9, 21);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(10, 22);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(11, 23);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(12, 24);
/* 10 fan present attributes in this platform */
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(2);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(3);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(4);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(5);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(6);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(7);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(8);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(9);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(10);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(11);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(12);
/* 1 fan duty cycle attribute in this platform */
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR();

/* 12 thermal sensor attributes in this platform */
DECLARE_CPU_MAC_TEMP_SENSOR_DEV_ATTR(1);
DECLARE_CPU_MAC_TEMP_SENSOR_DEV_ATTR(2);
DECLARE_TEMP_SENSOR_DEV_ATTR(3);
DECLARE_TEMP_SENSOR_DEV_ATTR(4);
DECLARE_TEMP_SENSOR_DEV_ATTR(5);
DECLARE_TEMP_SENSOR_DEV_ATTR(6);
DECLARE_TEMP_SENSOR_DEV_ATTR(7);
DECLARE_TEMP_SENSOR_DEV_ATTR(8);
DECLARE_TEMP_SENSOR_DEV_ATTR(9);
DECLARE_TEMP_SENSOR_DEV_ATTR(10);
DECLARE_TEMP_SENSOR_DEV_ATTR(11);
DECLARE_TEMP_SENSOR_DEV_ATTR(12);

static struct attribute *as7926_40xke_fan_attributes[] = {
    /* fan related attributes */
    DECLARE_FAN_FAULT_ATTR(1, 13),
    DECLARE_FAN_FAULT_ATTR(2, 14),
    DECLARE_FAN_FAULT_ATTR(3, 15),
    DECLARE_FAN_FAULT_ATTR(4, 16),
    DECLARE_FAN_FAULT_ATTR(5, 17),
    DECLARE_FAN_FAULT_ATTR(6, 18),
    DECLARE_FAN_FAULT_ATTR(7, 19),
    DECLARE_FAN_FAULT_ATTR(8, 20),
    DECLARE_FAN_FAULT_ATTR(9, 21),
    DECLARE_FAN_FAULT_ATTR(10, 22),
    DECLARE_FAN_FAULT_ATTR(11, 23),
    DECLARE_FAN_FAULT_ATTR(12, 24),
    DECLARE_FAN_SPEED_RPM_ATTR(1, 13),
    DECLARE_FAN_SPEED_RPM_ATTR(2, 14),
    DECLARE_FAN_SPEED_RPM_ATTR(3, 15),
    DECLARE_FAN_SPEED_RPM_ATTR(4, 16),
    DECLARE_FAN_SPEED_RPM_ATTR(5, 17),
    DECLARE_FAN_SPEED_RPM_ATTR(6, 18),
    DECLARE_FAN_SPEED_RPM_ATTR(7, 19),
    DECLARE_FAN_SPEED_RPM_ATTR(8, 20),
    DECLARE_FAN_SPEED_RPM_ATTR(9, 21),
    DECLARE_FAN_SPEED_RPM_ATTR(10, 22),
    DECLARE_FAN_SPEED_RPM_ATTR(11, 23),
    DECLARE_FAN_SPEED_RPM_ATTR(12, 24),
    DECLARE_FAN_PRESENT_ATTR(1),
    DECLARE_FAN_PRESENT_ATTR(2),
    DECLARE_FAN_PRESENT_ATTR(3),
    DECLARE_FAN_PRESENT_ATTR(4),
    DECLARE_FAN_PRESENT_ATTR(5),
    DECLARE_FAN_PRESENT_ATTR(6),
    DECLARE_FAN_PRESENT_ATTR(7),
    DECLARE_FAN_PRESENT_ATTR(8),
    DECLARE_FAN_PRESENT_ATTR(9),
    DECLARE_FAN_PRESENT_ATTR(10),
    DECLARE_FAN_PRESENT_ATTR(11),
    DECLARE_FAN_PRESENT_ATTR(12),
    DECLARE_FAN_DUTY_CYCLE_ATTR(),
    /* temperature related attributes */
    DECLARE_CPU_MAC_TEMP_SENSOR_ATTR(1),
    DECLARE_CPU_MAC_TEMP_SENSOR_ATTR(2),
    DECLARE_TEMP_SENSOR_ATTR(3),
    DECLARE_TEMP_SENSOR_ATTR(4),
    DECLARE_TEMP_SENSOR_ATTR(5),
    DECLARE_TEMP_SENSOR_ATTR(6),
    DECLARE_TEMP_SENSOR_ATTR(7),
    DECLARE_TEMP_SENSOR_ATTR(8),
    DECLARE_TEMP_SENSOR_ATTR(9),
    DECLARE_TEMP_SENSOR_ATTR(10),
    DECLARE_TEMP_SENSOR_ATTR(11),
    DECLARE_TEMP_SENSOR_ATTR(12),
    NULL
};

#define FAN_DUTY_CYCLE_REG_MASK         0xF
#define FAN_MAX_DUTY_CYCLE              100
#define FAN_REG_VAL_TO_SPEED_RPM_STEP   150

static int as7926_40xke_fan_read_value(struct i2c_client *client, u8 reg)
{
    return i2c_smbus_read_byte_data(client, reg);
}

static int as7926_40xke_fan_write_value(struct i2c_client *client, u8 reg, u8 value)
{
    return i2c_smbus_write_byte_data(client, reg, value);
}

/* fan utility functions
 */
static u32 reg_val_to_duty_cycle(u8 reg_val)
{
    reg_val &= FAN_DUTY_CYCLE_REG_MASK;

    if (!reg_val) {
        return 0;
    }

    if (reg_val == 0xF) {
        return FAN_MAX_DUTY_CYCLE;
    }

    return (reg_val * 6) + 10;
}

static u8 duty_cycle_to_reg_val(u8 duty_cycle)
{
    if (duty_cycle < 16) {
        return 0;
    }

    if (duty_cycle >= 100) {
        return 0xF;
    }

    return (duty_cycle - 10) / 6;
}

static u32 reg_val_to_speed_rpm(u8 reg_val)
{
    return (u32)reg_val * FAN_REG_VAL_TO_SPEED_RPM_STEP;
}

static u8 reg_val_to_is_present(u8 reg_val, enum fan_id id)
{
    u8 mask = (1 << id);
    return !(reg_val & mask);
}

static u8 is_fan_fault(struct as7926_40xke_fan_data *data, enum fan_id id)
{
    u8 ret = 1;
    int front_fan_index = FAN1_FRONT_SPEED_RPM + id;
    int rear_fan_index  = FAN1_REAR_SPEED_RPM  + id;

    /* Check if the speed of front or rear fan is ZERO,
     */
    if (reg_val_to_speed_rpm(data->reg_val[front_fan_index]) &&
        reg_val_to_speed_rpm(data->reg_val[rear_fan_index]))  {
        ret = 0;
    }

    return ret;
}

static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count)
{
    int error, value;
    struct i2c_client *client = to_i2c_client(dev);

    error = kstrtoint(buf, 10, &value);
    if (error)
        return error;

    if (value < 0 || value > FAN_MAX_DUTY_CYCLE)
        return -EINVAL;

    as7926_40xke_fan_write_value(client, 0x33, 0); /* Disable fan speed watch dog */
    as7926_40xke_fan_write_value(client, fan_reg[FAN_DUTY_CYCLE_PERCENTAGE], duty_cycle_to_reg_val(value));
    return count;
}

static int twos_complement_to_int(u16 data, u8 valid_bit, int mask)
{
	u16 valid_data	 = data & mask;
	bool is_negative = valid_data >> (valid_bit - 1);

	return is_negative ? (-(((~valid_data) & mask) + 1)) : valid_data;
}

static int reg_val_to_thermal_value(u8 reg_val)
{
    int thermal_data;
    thermal_data =  twos_complement_to_int((unsigned short)reg_val, 8, 0xff);
    return thermal_data;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct as7926_40xke_fan_data *data = as7926_40xke_fan_update_device(dev);
    ssize_t ret = 0;

    if (data->valid) {
        switch (attr->index) {
            case FAN_DUTY_CYCLE_PERCENTAGE:
            {
                u32 duty_cycle = reg_val_to_duty_cycle(data->reg_val[FAN_DUTY_CYCLE_PERCENTAGE]);
                ret = sprintf(buf, "%u\n", duty_cycle);
                break;
            }
            case FAN1_FRONT_SPEED_RPM:
            case FAN2_FRONT_SPEED_RPM:
            case FAN3_FRONT_SPEED_RPM:
            case FAN4_FRONT_SPEED_RPM:
            case FAN5_FRONT_SPEED_RPM:
            case FAN6_FRONT_SPEED_RPM:
            case FAN7_FRONT_SPEED_RPM:
            case FAN8_FRONT_SPEED_RPM:
            case FAN9_FRONT_SPEED_RPM:
            case FAN10_FRONT_SPEED_RPM:
            case FAN11_FRONT_SPEED_RPM:
            case FAN12_FRONT_SPEED_RPM:
            case FAN1_REAR_SPEED_RPM:
            case FAN2_REAR_SPEED_RPM:
            case FAN3_REAR_SPEED_RPM:
            case FAN4_REAR_SPEED_RPM:
            case FAN5_REAR_SPEED_RPM:
            case FAN6_REAR_SPEED_RPM:
            case FAN7_REAR_SPEED_RPM:
            case FAN8_REAR_SPEED_RPM:
            case FAN9_REAR_SPEED_RPM:
            case FAN10_REAR_SPEED_RPM:
            case FAN11_REAR_SPEED_RPM:
            case FAN12_REAR_SPEED_RPM:
            {
                ret = sprintf(buf, "%u\n", reg_val_to_speed_rpm(data->reg_val[attr->index]));
                break;
            }
            case FAN1_PRESENT:
            case FAN2_PRESENT:
            case FAN3_PRESENT:
            case FAN4_PRESENT:
            case FAN5_PRESENT:
            case FAN6_PRESENT:
            case FAN7_PRESENT:
            case FAN8_PRESENT:
            {
                ret = sprintf(buf, "%d\n",
                              reg_val_to_is_present(data->reg_val[FAN_PRESENT_REG1],
                              attr->index - FAN1_PRESENT));
                              /*bit0-7*/
                break;
            }
            case FAN9_PRESENT:
            case FAN10_PRESENT:
            case FAN11_PRESENT:
            case FAN12_PRESENT:
            {
                ret = sprintf(buf, "%d\n",
                              reg_val_to_is_present(data->reg_val[FAN_PRESENT_REG2],
                              (attr->index - FAN9_PRESENT) +4));
                               /*bit4-7*/
                break;
            }
            case FAN1_FAULT:
            case FAN2_FAULT:
            case FAN3_FAULT:
            case FAN4_FAULT:
            case FAN5_FAULT:
            case FAN6_FAULT:
            case FAN7_FAULT:
            case FAN8_FAULT:
            case FAN9_FAULT:
            case FAN10_FAULT:
            case FAN11_FAULT:
            case FAN12_FAULT:
            {
                ret = sprintf(buf, "%d\n", is_fan_fault(data, attr->index - FAN1_FAULT));
                break;
            }
            case TEMP1_INPUT:
            case TEMP2_INPUT:
            case TEMP3_INPUT:
            case TEMP4_INPUT:
            case TEMP5_INPUT:
            case TEMP6_INPUT:
            case TEMP7_INPUT:
            case TEMP8_INPUT:
            case TEMP9_INPUT:
            case TEMP10_INPUT:
            case TEMP11_INPUT:
            case TEMP12_INPUT:
            {
                ret = sprintf(buf, "%u\n",
                              (1000 * (reg_val_to_thermal_value(data->reg_val[attr->index]))));
                break;
            }
            default:
                break;
        }
    }

    return ret;
}

static ssize_t set_temp(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count)
{
    long temp;
    int status;
    int bus, addr, reg;
    struct i2c_client *client = to_i2c_client(dev);
    struct as7926_40xke_fan_data *data = i2c_get_clientdata(client);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

    status = kstrtol(buf, 10, &temp);
    if (status) {
        return status;
    }

    if (temp > 127 || temp < -128) {
        return -EINVAL;
    }

    mutex_lock(&data->update_lock);

    /* write to cpld */
    switch (attr->index) {
        case TEMP1_INPUT:
            bus  = 11;
            addr = 0x60;
            reg  = 0x30;
            break;
        case TEMP2_INPUT:
            bus  = 11;
            addr = 0x60;
            reg  = 0x31;
            break;
        default:
            status = -EINVAL;
            goto exit;
    }

    status = as7926_40xke_cpld_write(bus, addr, reg, (u8)temp);
    if (unlikely(status != 0)) {
        goto exit;
    }

    data->valid = 0;
    status = count;

exit:
    mutex_unlock(&data->update_lock);
    return status;
}
static const struct attribute_group as7926_40xke_fan_group = {
    .attrs = as7926_40xke_fan_attributes,
};

static struct as7926_40xke_fan_data *as7926_40xke_fan_update_device(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct as7926_40xke_fan_data *data = i2c_get_clientdata(client);
    int status;

    mutex_lock(&data->update_lock);

    if (time_after(jiffies, data->last_updated + HZ + HZ / 2) ||
        !data->valid) {
        int i;

        dev_dbg(&client->dev, "Starting as7926_40xke_fan update\n");
        data->valid = 0;

        /* Update fan data
         */
        for (i = 0; i < ARRAY_SIZE(data->reg_val); i++) {
            status = as7926_40xke_fan_read_value(client, fan_reg[i]);
            if (status < 0) {
                data->valid = 0;
                mutex_unlock(&data->update_lock);
                dev_dbg(&client->dev, "reg %d, err %d\n", fan_reg[i], status);
                return data;
            }
            else {
                data->reg_val[i] = status;
            }
        }

        data->last_updated = jiffies;
        data->valid = 1;
    }

    mutex_unlock(&data->update_lock);

    return data;
}

static int as7926_40xke_fan_probe(struct i2c_client *client,
            const struct i2c_device_id *dev_id)
{
    struct as7926_40xke_fan_data *data;
    int status;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        status = -EIO;
        goto exit;
    }

    data = kzalloc(sizeof(struct as7926_40xke_fan_data), GFP_KERNEL);
    if (!data) {
        status = -ENOMEM;
        goto exit;
    }

    i2c_set_clientdata(client, data);
    data->valid = 0;
    mutex_init(&data->update_lock);

    dev_info(&client->dev, "chip found\n");

    /* Register sysfs hooks */
    status = sysfs_create_group(&client->dev.kobj, &as7926_40xke_fan_group);
    if (status) {
        goto exit_free;
    }

    data->hwmon_dev = hwmon_device_register_with_info(&client->dev, DRVNAME, NULL, NULL, NULL);
    if (IS_ERR(data->hwmon_dev)) {
        status = PTR_ERR(data->hwmon_dev);
        goto exit_remove;
    }

    dev_info(&client->dev, "%s: fan '%s'\n",
         dev_name(data->hwmon_dev), client->name);

    return 0;

exit_remove:
    sysfs_remove_group(&client->dev.kobj, &as7926_40xke_fan_group);
exit_free:
    kfree(data);
exit:

    return status;
}

static int as7926_40xke_fan_remove(struct i2c_client *client)
{
    struct as7926_40xke_fan_data *data = i2c_get_clientdata(client);
    hwmon_device_unregister(data->hwmon_dev);
    sysfs_remove_group(&client->dev.kobj, &as7926_40xke_fan_group);

    return 0;
}

/* Addresses to scan */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

static const struct i2c_device_id as7926_40xke_fan_id[] = {
    { "as7926_40xke_fan", 0 },
    {}
};
MODULE_DEVICE_TABLE(i2c, as7926_40xke_fan_id);

static struct i2c_driver as7926_40xke_fan_driver = {
    .class        = I2C_CLASS_HWMON,
    .driver = {
        .name     = DRVNAME,
    },
    .probe        = as7926_40xke_fan_probe,
    .remove       = as7926_40xke_fan_remove,
    .id_table     = as7926_40xke_fan_id,
    .address_list = normal_i2c,
};

static int __init as7926_40xke_fan_init(void)
{
    return i2c_add_driver(&as7926_40xke_fan_driver);
}

static void __exit as7926_40xke_fan_exit(void)
{
    i2c_del_driver(&as7926_40xke_fan_driver);
}

module_init(as7926_40xke_fan_init);
module_exit(as7926_40xke_fan_exit);

MODULE_AUTHOR("Jostar Yang <jostar_yang@edge-core.com>");
MODULE_DESCRIPTION("as7926_40xke_fan driver");
MODULE_LICENSE("GPL");

