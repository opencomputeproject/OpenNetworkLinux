// SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0
/*
 * An hwmon driver for the LITE-ON Power Module
 *
 * Copyright (C) 2014 Accton Technology Corporation.
 * Roger Ho <roger530_ho@accton.com>
 *
 * Based on ad7414.c
 * Copyright 2006 Stefan Roese <sr at denx.de>, DENX Software Engineering
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
#include <linux/kthread.h>
#include <linux/delay.h>

#define DRVNAME "as9817_64_psu"

#define I2C_RW_RETRY_COUNT             (10)
#define I2C_RW_RETRY_INTERVAL          (60)	/* ms */

#define MAX_FAN_DUTY_CYCLE 100

#define EXIT_IF_POWER_FAILED(c) \
    do { \
        if (as9817_64_psu_is_powergood(c) != 1) \
            goto exit; \
    } while (0)

#define SLEEP_IF_INTERVAL(pInterval) \
    do { \
        int interval = atomic_read(pInterval); \
        if (interval > 0) \
            msleep(interval); \
    } while (0)

/* SLEEP_IF_INTERVAL should be called before EXIT_IF_POWER_FAILED.
 * It is known that accessing PSU when power failed might cause problems.
 * So it is better to do sleep before checking power status because it avoids
 * the risk that power status changes to failed during the sleep period.
 */
#define VALIDATE_POWERGOOD_AND_INTERVAL(client, pInterval) \
    do { \
        SLEEP_IF_INTERVAL(pInterval); \
        EXIT_IF_POWER_FAILED(client); \
    } while (0)

/* Addresses scanned
 */
static const unsigned short normal_i2c[] = { 0x58, 0x59, I2C_CLIENT_END };

enum chips {
	PS_2302_6L,
	MAX_CHIP_ID
};

struct pmbus_register_value {
	u8 capability;		/* Register value */
	u16 status_word;	/* Register value */
	u8 fan_fault;		/* Register value */
	u8 over_temp;		/* Register value */
	u8 line_status;		/* Register value */
	u16 v_in;		/* Register value */
	u16 v_out;		/* Register value */
	u16 i_in;		/* Register value */
	u16 i_out;		/* Register value */
	u16 p_in;		/* Register value */
	u16 p_out;		/* Register value */
	u8 vout_mode;		/* Register value */
	u16 temp_input[3];	/* Register value */
	u16 fan_speed;		/* Register value */
	u16 fan_duty_cycle[2];	/* Register value */
	u8 fan_dir[4];		/* Register value */
	u8 pmbus_revision;	/* Register value */
	u8 mfr_serial[16];	/* Register value */
	u8 mfr_id[16];		/* Register value */
	u8 mfr_model[18];	/* Register value */
	u8 mfr_revsion[10];	/* Register value */
	u16 mfr_vin_min;	/* Register value */
	u16 mfr_vin_max;	/* Register value */
	u16 mfr_iin_max;	/* Register value */
	u16 mfr_iout_max;	/* Register value */
	u16 mfr_pin_max;	/* Register value */
	u16 mfr_pout_max;	/* Register value */
	u16 mfr_vout_min;	/* Register value */
	u16 mfr_vout_max;	/* Register value */
	u16 mfr_fan_speed_min;	/* Register value */
	u16 mfr_fan_speed_max;	/* Register value */
};

/*
pmbus_threshold_value {
    v_out_lo, v_out_hi,
    p_out_hi, p_out_hi_crit,
    temp3_max, temp3_crit
};
*/

int64_t ac_high_line[] = {
	11600, 12800,
	3300000000, 3600000000,
	65000, 75000
};

int64_t ac_low_line[] = {
	11600, 12800,
	3300000000, 3600000000,
	65000, 75000
};

/* Each client has this additional data
 */
struct as9817_64_psu_data {
	struct i2c_client *client;
	struct device *hwmon_dev;
	struct mutex update_lock;
	atomic_t access_interval;
	char valid;		/* !=0 if registers are valid */
	unsigned long last_updated;	/* In jiffies */
	u8 chip;		/* chip id */
	struct pmbus_register_value reg_val;
};

extern int as9817_64_cpld_read(u8 cpld_addr, u8 reg);
extern int as9817_64_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);
extern int as9817_64_psu_is_powergood(struct i2c_client *client);

static ssize_t show_vout(struct device *dev, struct device_attribute *da,
			 char *buf);
static ssize_t show_byte(struct device *dev, struct device_attribute *da,
			 char *buf);
static ssize_t show_word(struct device *dev, struct device_attribute *da,
			 char *buf);
static ssize_t show_linear(struct device *dev, struct device_attribute *da,
			   char *buf);
static ssize_t show_fan_fault(struct device *dev, struct device_attribute *da,
			      char *buf);
static ssize_t show_over_temp(struct device *dev, struct device_attribute *da,
			      char *buf);
static ssize_t show_ascii(struct device *dev, struct device_attribute *da,
			  char *buf);
static ssize_t show_threshold(struct device *dev, struct device_attribute *da,
			      char *buf);
static int as9817_64_psu_update_data(struct i2c_client *client,
				     struct pmbus_register_value *data);
static struct as9817_64_psu_data *as9817_64_psu_update_device(struct i2c_client
							      *client);
static ssize_t set_fan_duty_cycle(struct device *dev,
				  struct device_attribute *da, const char *buf,
				  size_t count);
static ssize_t set_power_cycle(struct device *dev, struct device_attribute *da,
			       const char *buf, size_t count);
static int as9817_64_psu_write_byte(struct i2c_client *client, u8 reg,
				    u8 value);
static int as9817_64_psu_write_word(struct i2c_client *client, u8 reg,
				    u16 value);

enum as9817_64_psu_sysfs_attributes {
	PSU_POWER_ON = 0,
	PSU_TEMP_FAULT,
	PSU_POWER_GOOD,
	PSU_FAN1_FAULT,
	PSU_FAN_DIRECTION,
	PSU_OVER_TEMP,
	PSU_LINE_STATUS,
	PSU_V_IN,
	PSU_V_OUT,
	PSU_I_IN,
	PSU_I_OUT,
	PSU_P_IN,
	PSU_P_OUT,
	PSU_P_OUT_UV,		/*In Unit of microVolt, instead of mini. */
	PSU_TEMP1_INPUT,
	PSU_TEMP2_INPUT,
	PSU_TEMP3_INPUT,
	PSU_FAN1_SPEED,
	PSU_FAN1_DUTY_CYCLE,
	PSU_PMBUS_REVISION,
	PSU_SERIAL_NUM,
	PSU_MFR_ID,
	PSU_MFR_MODEL,
	PSU_MFR_REVISION,
	PSU_MFR_SERIAL,
	PSU_MFR_VIN_MIN,
	PSU_MFR_VIN_MAX,
	PSU_MFR_VOUT_MIN,
	PSU_MFR_VOUT_MAX,
	PSU_MFR_IIN_MAX,
	PSU_MFR_IOUT_MAX,
	PSU_MFR_PIN_MAX,
	PSU_MFR_POUT_MAX,
	PSU_MFR_FAN_SPEED_MAX,
	PSU_MFR_FAN_SPEED_MIN,
	PSU_VOUT_WARN_LO,
	PSU_VOUT_WARN_HI,
	PSU_POUT_WARN_HI,
	PSU_POUT_CRIT_HI,
	PSU_TEMP3_WARN_HI,
	PSU_TEMP3_CRIT_HI,
	PSU_POWER_CYCLE,
};

/* sysfs attributes for hwmon
 */
static SENSOR_DEVICE_ATTR(psu_power_on, S_IRUGO, show_word, NULL, PSU_POWER_ON);
static SENSOR_DEVICE_ATTR(psu_temp_fault, S_IRUGO, show_word, NULL,
			  PSU_TEMP_FAULT);
static SENSOR_DEVICE_ATTR(psu_power_good, S_IRUGO, show_word, NULL,
			  PSU_POWER_GOOD);
static SENSOR_DEVICE_ATTR(psu_fan1_fault, S_IRUGO, show_fan_fault, NULL,
			  PSU_FAN1_FAULT);
static SENSOR_DEVICE_ATTR(psu_over_temp, S_IRUGO, show_over_temp, NULL,
			  PSU_OVER_TEMP);
static SENSOR_DEVICE_ATTR(psu_line_status, S_IRUGO, show_byte, NULL,
			  PSU_LINE_STATUS);
static SENSOR_DEVICE_ATTR(psu_v_in, S_IRUGO, show_linear, NULL, PSU_V_IN);
static SENSOR_DEVICE_ATTR(psu_v_out, S_IRUGO, show_vout, NULL, PSU_V_OUT);
static SENSOR_DEVICE_ATTR(psu_i_in, S_IRUGO, show_linear, NULL, PSU_I_IN);
static SENSOR_DEVICE_ATTR(psu_i_out, S_IRUGO, show_linear, NULL, PSU_I_OUT);
static SENSOR_DEVICE_ATTR(psu_p_in, S_IRUGO, show_linear, NULL, PSU_P_IN);
static SENSOR_DEVICE_ATTR(psu_p_out, S_IRUGO, show_linear, NULL, PSU_P_OUT);
static SENSOR_DEVICE_ATTR(psu_temp1_input, S_IRUGO, show_linear, NULL,
			  PSU_TEMP1_INPUT);
static SENSOR_DEVICE_ATTR(psu_temp2_input, S_IRUGO, show_linear, NULL,
			  PSU_TEMP2_INPUT);
static SENSOR_DEVICE_ATTR(psu_temp3_input, S_IRUGO, show_linear, NULL,
			  PSU_TEMP3_INPUT);
static SENSOR_DEVICE_ATTR(psu_fan1_speed_rpm, S_IRUGO, show_linear, NULL,
			  PSU_FAN1_SPEED);
static SENSOR_DEVICE_ATTR(psu_fan1_duty_cycle_percentage, S_IWUSR | S_IRUGO,
			  show_linear, set_fan_duty_cycle, PSU_FAN1_DUTY_CYCLE);
static SENSOR_DEVICE_ATTR(psu_fan_dir, S_IRUGO, show_ascii, NULL,
			  PSU_FAN_DIRECTION);
static SENSOR_DEVICE_ATTR(psu_pmbus_revision, S_IRUGO, show_byte, NULL,
			  PSU_PMBUS_REVISION);
static SENSOR_DEVICE_ATTR(psu_serial_num, S_IRUGO, show_ascii, NULL,
			  PSU_SERIAL_NUM);
static SENSOR_DEVICE_ATTR(psu_mfr_id, S_IRUGO, show_ascii, NULL, PSU_MFR_ID);
static SENSOR_DEVICE_ATTR(psu_mfr_model, S_IRUGO, show_ascii, NULL,
			  PSU_MFR_MODEL);
static SENSOR_DEVICE_ATTR(psu_mfr_revision, S_IRUGO, show_ascii, NULL,
			  PSU_MFR_REVISION);
static SENSOR_DEVICE_ATTR(psu_mfr_serial, S_IRUGO, show_ascii, NULL,
			  PSU_MFR_SERIAL);
static SENSOR_DEVICE_ATTR(psu_mfr_vin_min, S_IRUGO, show_linear, NULL,
			  PSU_MFR_VIN_MIN);
static SENSOR_DEVICE_ATTR(psu_mfr_vin_max, S_IRUGO, show_linear, NULL,
			  PSU_MFR_VIN_MAX);
static SENSOR_DEVICE_ATTR(psu_mfr_vout_min, S_IRUGO, show_linear, NULL,
			  PSU_MFR_VOUT_MIN);
static SENSOR_DEVICE_ATTR(psu_mfr_vout_max, S_IRUGO, show_linear, NULL,
			  PSU_MFR_VOUT_MAX);
static SENSOR_DEVICE_ATTR(psu_mfr_iin_max, S_IRUGO, show_linear, NULL,
			  PSU_MFR_IIN_MAX);
static SENSOR_DEVICE_ATTR(psu_mfr_iout_max, S_IRUGO, show_linear, NULL,
			  PSU_MFR_IOUT_MAX);
static SENSOR_DEVICE_ATTR(psu_mfr_pin_max, S_IRUGO, show_linear, NULL,
			  PSU_MFR_PIN_MAX);
static SENSOR_DEVICE_ATTR(psu_mfr_pout_max, S_IRUGO, show_linear, NULL,
			  PSU_MFR_POUT_MAX);
static SENSOR_DEVICE_ATTR(psu_mfr_fan_speed_min, S_IRUGO, show_linear, NULL,
			  PSU_MFR_FAN_SPEED_MIN);
static SENSOR_DEVICE_ATTR(psu_mfr_fan_speed_max, S_IRUGO, show_linear, NULL,
			  PSU_MFR_FAN_SPEED_MAX);
static SENSOR_DEVICE_ATTR(psu_v_out_min, S_IRUGO, show_threshold, NULL,
			  PSU_VOUT_WARN_LO);
static SENSOR_DEVICE_ATTR(psu_v_out_max, S_IRUGO, show_threshold, NULL,
			  PSU_VOUT_WARN_HI);
static SENSOR_DEVICE_ATTR(psu_p_out_max, S_IRUGO, show_threshold, NULL,
			  PSU_POUT_WARN_HI);
static SENSOR_DEVICE_ATTR(psu_p_out_crit, S_IRUGO, show_threshold, NULL,
			  PSU_POUT_CRIT_HI);
static SENSOR_DEVICE_ATTR(psu_temp3_max, S_IRUGO, show_threshold, NULL,
			  PSU_TEMP3_WARN_HI);
static SENSOR_DEVICE_ATTR(psu_temp3_crit, S_IRUGO, show_threshold, NULL,
			  PSU_TEMP3_CRIT_HI);
static SENSOR_DEVICE_ATTR(psu_power_cycle, S_IWUSR, NULL, set_power_cycle,
			  PSU_POWER_CYCLE);

/*Duplicate nodes for lm-sensors.*/
static SENSOR_DEVICE_ATTR(in0_input, S_IRUGO, show_vout, NULL, PSU_V_OUT);
static SENSOR_DEVICE_ATTR(curr1_input, S_IRUGO, show_linear, NULL, PSU_I_OUT);
static SENSOR_DEVICE_ATTR(power1_input, S_IRUGO, show_linear, NULL,
			  PSU_P_OUT_UV);
static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, show_linear, NULL,
			  PSU_TEMP1_INPUT);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO, show_linear, NULL,
			  PSU_TEMP2_INPUT);
static SENSOR_DEVICE_ATTR(temp3_input, S_IRUGO, show_linear, NULL,
			  PSU_TEMP3_INPUT);
static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, show_linear, NULL,
			  PSU_FAN1_SPEED);
static SENSOR_DEVICE_ATTR(temp1_fault, S_IRUGO, show_word, NULL,
			  PSU_TEMP_FAULT);
static SENSOR_DEVICE_ATTR(temp2_fault, S_IRUGO, show_word, NULL,
			  PSU_TEMP_FAULT);
static SENSOR_DEVICE_ATTR(temp3_fault, S_IRUGO, show_word, NULL,
			  PSU_TEMP_FAULT);

static struct attribute *as9817_64_psu_attributes[] = {
	&sensor_dev_attr_psu_power_on.dev_attr.attr,
	&sensor_dev_attr_psu_temp_fault.dev_attr.attr,
	&sensor_dev_attr_psu_power_good.dev_attr.attr,
	&sensor_dev_attr_psu_fan1_fault.dev_attr.attr,
	&sensor_dev_attr_psu_over_temp.dev_attr.attr,
	&sensor_dev_attr_psu_line_status.dev_attr.attr,
	&sensor_dev_attr_psu_v_in.dev_attr.attr,
	&sensor_dev_attr_psu_v_out.dev_attr.attr,
	&sensor_dev_attr_psu_i_in.dev_attr.attr,
	&sensor_dev_attr_psu_i_out.dev_attr.attr,
	&sensor_dev_attr_psu_p_in.dev_attr.attr,
	&sensor_dev_attr_psu_p_out.dev_attr.attr,
	&sensor_dev_attr_psu_temp1_input.dev_attr.attr,
	&sensor_dev_attr_psu_temp2_input.dev_attr.attr,
	&sensor_dev_attr_psu_temp3_input.dev_attr.attr,
	&sensor_dev_attr_psu_fan1_speed_rpm.dev_attr.attr,
	&sensor_dev_attr_psu_fan1_duty_cycle_percentage.dev_attr.attr,
	&sensor_dev_attr_psu_fan_dir.dev_attr.attr,
	&sensor_dev_attr_psu_pmbus_revision.dev_attr.attr,
	&sensor_dev_attr_psu_serial_num.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_id.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_model.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_revision.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_serial.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_vin_min.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_vin_max.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_pout_max.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_iin_max.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_pin_max.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_vout_min.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_vout_max.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_iout_max.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_fan_speed_min.dev_attr.attr,
	&sensor_dev_attr_psu_mfr_fan_speed_max.dev_attr.attr,
	&sensor_dev_attr_psu_v_out_min.dev_attr.attr,
	&sensor_dev_attr_psu_v_out_max.dev_attr.attr,
	&sensor_dev_attr_psu_p_out_max.dev_attr.attr,
	&sensor_dev_attr_psu_p_out_crit.dev_attr.attr,
	&sensor_dev_attr_psu_temp3_max.dev_attr.attr,
	&sensor_dev_attr_psu_temp3_crit.dev_attr.attr,
	&sensor_dev_attr_psu_power_cycle.dev_attr.attr,
	/*Duplicate nodes for lm-sensors. */
	&sensor_dev_attr_in0_input.dev_attr.attr,
	&sensor_dev_attr_curr1_input.dev_attr.attr,
	&sensor_dev_attr_power1_input.dev_attr.attr,
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_temp1_fault.dev_attr.attr,
	&sensor_dev_attr_temp2_fault.dev_attr.attr,
	&sensor_dev_attr_temp3_fault.dev_attr.attr,
	NULL
};

/* To prevent hardware errors,
 * access new PMBus registers should be skipped
 * if the chip ID is not in the following list.
 * Return 1 by default if the register is not listed
 */
static int pmbus_register_supported(u8 chip, u8 reg)
{
	int i = 0;

	struct reg_chip_data {
		u8 reg;
		u32 chips;
	};

	struct reg_chip_data supported_list[] = { };

	if (chip >= MAX_CHIP_ID)
		return 0;

	for (i = 0; i < ARRAY_SIZE(supported_list); i++) {
		if (reg != supported_list[i].reg)
			continue;

		return ! !(supported_list[i].chips & BIT(chip));
	}

	return 1;
}

static ssize_t show_byte(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as9817_64_psu_data *data = dev_get_drvdata(dev);
	ssize_t ret = 0;

	as9817_64_psu_update_device(data->client);

	mutex_lock(&data->update_lock);
	if (!data->valid) {
		goto exit;
	}

	switch (attr->index) {
	case PSU_PMBUS_REVISION:
		ret = sprintf(buf, "%d\n", data->reg_val.pmbus_revision);
		break;
	case PSU_LINE_STATUS:
		ret = sprintf(buf, "0x%02x\n", data->reg_val.line_status);
		break;
	default:
		break;
	}

 exit:
	mutex_unlock(&data->update_lock);
	return ret;
}

static ssize_t show_word(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as9817_64_psu_data *data = dev_get_drvdata(dev);
	u16 status = 0;

	as9817_64_psu_update_device(data->client);

	mutex_lock(&data->update_lock);
	if (!data->valid) {
		goto exit;
	}

	switch (attr->index) {
	case PSU_POWER_ON:	/* psu_power_on, low byte bit 6 of status_word, 0=>ON, 1=>OFF */
		status = (data->reg_val.status_word & 0x40) ? 0 : 1;
		break;
	case PSU_TEMP_FAULT:	/* psu_temp_fault, low byte bit 2 of status_word, 0=>Normal, 1=>temp fault */
		status = (data->reg_val.status_word & 0x4) >> 2;
		break;
	case PSU_POWER_GOOD:	/* psu_power_good, high byte bit 3 of status_word, 0=>OK, 1=>FAIL */
		status = (data->reg_val.status_word & 0x800) ? 0 : 1;
		break;
	default:
		goto exit;
	}

	mutex_unlock(&data->update_lock);
	return sprintf(buf, "%d\n", status);

 exit:
	mutex_unlock(&data->update_lock);
	return 0;
}

static int two_complement_to_int(u16 data, u8 valid_bit, int mask)
{
	u16 valid_data = data & mask;
	bool is_negative = valid_data >> (valid_bit - 1);

	return is_negative ? (-(((~valid_data) & mask) + 1)) : valid_data;
}

static ssize_t set_fan_duty_cycle(struct device *dev,
				  struct device_attribute *da, const char *buf,
				  size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as9817_64_psu_data *data = dev_get_drvdata(dev);
	int nr = (attr->index == PSU_FAN1_DUTY_CYCLE) ? 0 : 1;
	long speed;
	int error;

	as9817_64_psu_update_device(data->client);

	error = kstrtol(buf, 10, &speed);
	if (error)
		return error;

	if (speed < 0 || speed > MAX_FAN_DUTY_CYCLE)
		return -EINVAL;

	mutex_lock(&data->update_lock);
	data->reg_val.fan_duty_cycle[nr] = speed;
	as9817_64_psu_write_word(data->client, 0x3B + nr,
				 data->reg_val.fan_duty_cycle[nr]);
	mutex_unlock(&data->update_lock);

	return count;
}

static ssize_t set_power_cycle(struct device *dev, struct device_attribute *da,
			       const char *buf, size_t count)
{
	struct as9817_64_psu_data *data = dev_get_drvdata(dev);
	long enable;
	int error;

	error = kstrtol(buf, 10, &enable);
	if (error) {
		return error;
	}

	mutex_lock(&data->update_lock);
	as9817_64_psu_write_byte(data->client, 0xEE, enable);
	mutex_unlock(&data->update_lock);

	return count;
}

static ssize_t show_linear(struct device *dev, struct device_attribute *da,
			   char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as9817_64_psu_data *data = dev_get_drvdata(dev);
	u16 value = 0;
	int exponent, mantissa;
	int multiplier = 1000;

	as9817_64_psu_update_device(data->client);

	mutex_lock(&data->update_lock);
	if (!data->valid) {
		goto exit;
	}

	switch (attr->index) {
	case PSU_V_IN:
		value = data->reg_val.v_in;
		break;
	case PSU_V_OUT:
		value = data->reg_val.v_out;
		break;
	case PSU_I_IN:
		value = data->reg_val.i_in;
		break;
	case PSU_I_OUT:
		value = data->reg_val.i_out;
		break;
	case PSU_P_IN:
		value = data->reg_val.p_in;
		break;
	case PSU_P_OUT_UV:
		multiplier = 1000000;	/*For lm-sensors, unit is micro-Volt. */
		/*Passing through */
	case PSU_P_OUT:
		value = data->reg_val.p_out;
		break;
	case PSU_TEMP1_INPUT:
	case PSU_TEMP2_INPUT:
	case PSU_TEMP3_INPUT:
		value = data->reg_val.temp_input[attr->index - PSU_TEMP1_INPUT];
		break;
	case PSU_FAN1_SPEED:
		value = data->reg_val.fan_speed;
		multiplier = 1;
		break;
	case PSU_FAN1_DUTY_CYCLE:
		value = data->reg_val.fan_duty_cycle[0];
		multiplier = 1;
		break;
	case PSU_MFR_VIN_MIN:
		value = data->reg_val.mfr_vin_min;
		break;
	case PSU_MFR_VIN_MAX:
		value = data->reg_val.mfr_vin_max;
		break;
	case PSU_MFR_VOUT_MIN:
		value = data->reg_val.mfr_vout_min;
		break;
	case PSU_MFR_VOUT_MAX:
		value = data->reg_val.mfr_vout_max;
		break;
	case PSU_MFR_PIN_MAX:
		value = data->reg_val.mfr_pin_max;
		break;
	case PSU_MFR_POUT_MAX:
		value = data->reg_val.mfr_pout_max;
		break;
	case PSU_MFR_IOUT_MAX:
		value = data->reg_val.mfr_iout_max;
		break;
	case PSU_MFR_IIN_MAX:
		value = data->reg_val.mfr_iin_max;
		break;
	case PSU_MFR_FAN_SPEED_MIN:
		multiplier = 1;
		value = data->reg_val.mfr_fan_speed_min;
		break;
	case PSU_MFR_FAN_SPEED_MAX:
		multiplier = 1;
		value = data->reg_val.mfr_fan_speed_max;
		break;
	default:
		goto exit;
	}
	mutex_unlock(&data->update_lock);

	exponent = two_complement_to_int(value >> 11, 5, 0x1f);
	mantissa = two_complement_to_int(value & 0x7ff, 11, 0x7ff);
	return (exponent >= 0) ? sprintf(buf, "%d\n", (mantissa << exponent) * multiplier) :
				sprintf(buf, "%d\n", (mantissa * multiplier) / (1 << -exponent));

 exit:
	mutex_unlock(&data->update_lock);
	return 0;
}

static ssize_t show_fan_fault(struct device *dev, struct device_attribute *da,
			      char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as9817_64_psu_data *data = dev_get_drvdata(dev);
	u8 shift = 0;
	u8 fan_fault = 0;

	as9817_64_psu_update_device(data->client);

	mutex_lock(&data->update_lock);
	if (!data->valid) {
		goto exit;
	}

	fan_fault = data->reg_val.fan_fault;
	mutex_unlock(&data->update_lock);

	shift = (attr->index == PSU_FAN1_FAULT) ? 7 : 6;
	return sprintf(buf, "%d\n", fan_fault >> shift);

 exit:
	mutex_unlock(&data->update_lock);
	return 0;
}

static ssize_t show_over_temp(struct device *dev, struct device_attribute *da,
			      char *buf)
{
	struct as9817_64_psu_data *data = dev_get_drvdata(dev);
	u8 over_temp = 0;

	as9817_64_psu_update_device(data->client);

	mutex_lock(&data->update_lock);
	if (!data->valid) {
		goto exit;
	}

	over_temp = data->reg_val.over_temp;
	mutex_unlock(&data->update_lock);
	return sprintf(buf, "%d\n", over_temp >> 7);

 exit:
	mutex_unlock(&data->update_lock);
	return 0;
}

static ssize_t show_ascii(struct device *dev, struct device_attribute *da,
			  char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as9817_64_psu_data *data = dev_get_drvdata(dev);
	ssize_t ret = 0;
	u8 *ptr = NULL;

	as9817_64_psu_update_device(data->client);

	mutex_lock(&data->update_lock);
	if (!data->valid) {
		goto exit;
	}

	switch (attr->index) {
	case PSU_FAN_DIRECTION:	/* psu_fan_dir */
		if (data->chip == PS_2302_6L) {
			memcpy(data->reg_val.fan_dir, "F2B", 3);
			data->reg_val.fan_dir[3] = '\0';
		}
		ptr = data->reg_val.fan_dir;
		break;
	case PSU_MFR_SERIAL:	/* psu_mfr_serial */
		ptr = data->reg_val.mfr_serial + 1;	/* The first byte is the count byte of string. */
		break;
	case PSU_MFR_ID:	/* psu_mfr_id */
		ptr = data->reg_val.mfr_id + 1;	/* The first byte is the count byte of string. */
		break;
	case PSU_MFR_MODEL:	/* psu_mfr_model */
		ptr = data->reg_val.mfr_model + 1;	/* The first byte is the count byte of string. */
		break;
	case PSU_MFR_REVISION:	/* psu_mfr_revision */
		ptr = data->reg_val.mfr_revsion + 1;
		break;
	default:
		goto exit;
	}

	ret = sprintf(buf, "%s\n", ptr);

 exit:
	mutex_unlock(&data->update_lock);
	return ret;
}

static ssize_t show_vout_by_mode(struct device *dev,
				 struct device_attribute *da, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as9817_64_psu_data *data = dev_get_drvdata(dev);
	int exponent, mantissa;
	int multiplier = 1000;

	as9817_64_psu_update_device(data->client);

	mutex_lock(&data->update_lock);
	if (!data->valid) {
		goto exit;
	}

	exponent = two_complement_to_int(data->reg_val.vout_mode, 5, 0x1f);
	switch (attr->index) {
	case PSU_MFR_VOUT_MIN:
		mantissa = data->reg_val.mfr_vout_min;
		break;
	case PSU_MFR_VOUT_MAX:
		mantissa = data->reg_val.mfr_vout_max;
		break;
	case PSU_V_OUT:
		mantissa = data->reg_val.v_out;
		break;
	default:
		goto exit;
	}
	mutex_unlock(&data->update_lock);

	return (exponent > 0) ? sprintf(buf, "%d\n", (mantissa << exponent) * multiplier) :
				sprintf(buf, "%d\n", (mantissa * multiplier) / (1 << -exponent));

 exit:
	mutex_unlock(&data->update_lock);
	return 0;
}

static ssize_t show_vout(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct as9817_64_psu_data *data = dev_get_drvdata(dev);

	if (data->chip == PS_2302_6L) {
		return show_vout_by_mode(dev, da, buf);
	} else {
		return show_linear(dev, da, buf);
	}
}

static ssize_t show_threshold(struct device *dev, struct device_attribute *da,
			      char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct as9817_64_psu_data *data = dev_get_drvdata(dev);
	int64_t *val = NULL;
	ssize_t ret = 0;

	as9817_64_psu_update_device(data->client);

	mutex_lock(&data->update_lock);
	if (!data->valid) {
		goto exit;
	}

	switch (data->reg_val.line_status) {
	case 0:		/* Low line, 50Hz, AC Present */
	case 4:		/* Low line, 60Hz, AC Present */
		val = ac_low_line;
		break;
	case 1:		/* No AC Input. */
		val = ac_low_line;
		break;
	case 2:		/* High line, 50Hz, AC Present */
	case 6:		/* High line, 60Hz, AC Present */
		val = ac_high_line;
		break;
	default:
		dev_dbg(dev, "LINE_STATUS: 0x%02x\n",
			data->reg_val.line_status);
		ret = sprintf(buf, "0\n");
		goto exit;
	}

	switch (attr->index) {
	case PSU_VOUT_WARN_LO ... PSU_TEMP3_CRIT_HI:
		ret = sprintf(buf, "%lld\n", val[attr->index - PSU_VOUT_WARN_LO]);
		break;
	default:
		break;
	}

 exit:
	mutex_unlock(&data->update_lock);
	return ret;
}

static const struct attribute_group as9817_64_psu_group = {
	.attrs = as9817_64_psu_attributes,
};

__ATTRIBUTE_GROUPS(as9817_64_psu);

static int as9817_64_psu_probe(struct i2c_client *client,
			       const struct i2c_device_id *dev_id)
{
	struct as9817_64_psu_data *data;
	int status;

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_BYTE_DATA |
				     I2C_FUNC_SMBUS_WORD_DATA |
				     I2C_FUNC_SMBUS_I2C_BLOCK)) {
		status = -EIO;
		dev_err(&client->dev, "-EIO\n");
		goto exit;
	}

	data = kzalloc(sizeof(struct as9817_64_psu_data), GFP_KERNEL);
	if (!data) {
		status = -ENOMEM;
		dev_err(&client->dev, "-ENOMEM\n");

		goto exit;
	}
	i2c_set_clientdata(client, data);
	data->client = client;
	mutex_init(&data->update_lock);
	data->chip = dev_id->driver_data;
	dev_info(&client->dev, "chip found\n");

	data->hwmon_dev =
	    hwmon_device_register_with_groups(&client->dev, client->name, data,
					      as9817_64_psu_groups);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		dev_err(&client->dev, "err %d\n", status);
		goto exit_free;
	}

	atomic_set(&data->access_interval, 0);

	dev_info(&client->dev, "%s: psu '%s' @ 0x%02x\n",
		 dev_name(data->hwmon_dev), client->name, client->addr);

	return 0;

 exit_free:
	kfree(data);
 exit:

	return status;
}

static int as9817_64_psu_remove(struct i2c_client *client)
{
	struct as9817_64_psu_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	kfree(data);

	return 0;
}

static const struct i2c_device_id as9817_64_psu_id[] = {
	{"ps_2302_6l", PS_2302_6L},
	{}
};

MODULE_DEVICE_TABLE(i2c, as9817_64_psu_id);

static struct i2c_driver as9817_64_psu_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		   .name = "as9817_64_psu",
		   },
	.probe = as9817_64_psu_probe,
	.remove = as9817_64_psu_remove,
	.id_table = as9817_64_psu_id,
	.address_list = normal_i2c,
};

static int as9817_64_psu_read_byte(struct i2c_client *client, u8 reg)
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

static int as9817_64_psu_write_byte(struct i2c_client *client, u8 reg, u8 value)
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

static int as9817_64_psu_read_word(struct i2c_client *client, u8 reg)
{
	int status = 0, retry = I2C_RW_RETRY_COUNT;

	while (retry) {
		status = i2c_smbus_read_word_data(client, reg);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

	return status;
}

static int as9817_64_psu_write_word(struct i2c_client *client, u8 reg,
				    u16 value)
{
	int status = 0, retry = I2C_RW_RETRY_COUNT;

	while (retry) {
		status = i2c_smbus_write_word_data(client, reg, value);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

	return status;
}

static int as9817_64_psu_read_block(struct i2c_client *client, u8 command,
				    u8 * data, int data_len)
{
	int status = 0, retry = I2C_RW_RETRY_COUNT;

	while (retry) {
		status = i2c_smbus_read_i2c_block_data(client, command, data_len,
						  data);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

	if (unlikely(status != data_len)) {
		status = -EIO;
		goto abort;
	}

 abort:
	return status;

}

struct reg_data_byte {
	u8 reg;
	u8 *value;
};

struct reg_data_word {
	u8 reg;
	u16 *value;
};

static char *get_fan_dir_by_model_name(struct i2c_client *client,
				       char *ptr_model, char *fan_dir,
				       int command)
{
	char *ptr_fan;

	ptr_fan = fan_dir;

	if (!strncmp("PS-2162-6L", ptr_model + 1, strlen("PS-2162-6L"))) {	/* hard code: spec do not define */
		memcpy(fan_dir, "F2B", 3);
	} else {
		ptr_fan = NULL;
	}

	dev_dbg(&client->dev, "Model name is %s, get by read_word is (%s)\n",
		ptr_model + 1, ptr_fan);
	return ptr_fan;
}

#define NULL_TERMINATE_BY_LENGTH(field) \
    do { \
        length = data->field[0]; \
        if (length > 0) { \
            data->field[length + 1] = '\0'; \
        } \
    } while (0)

static int as9817_64_psu_update_data(struct i2c_client *client,
				     struct pmbus_register_value *data)
{
	struct as9817_64_psu_data *driver_data = i2c_get_clientdata(client);
	int i, status, length;
	u8 command;
	u8 fan_dir[5] = { 0 };
	struct reg_data_byte regs_byte[] = {
		{0x19, &data->capability},
		{0x20, &data->vout_mode},
		{0x7d, &data->over_temp},
		{0x81, &data->fan_fault},
		{0x98, &data->pmbus_revision},
		{0xd8, &data->line_status},
	};
	struct reg_data_word regs_word[] = {
		{0x79, &data->status_word},
		{0x88, &data->v_in},
		{0x8b, &data->v_out},
		{0x8c, &data->i_out},
		{0x89, &data->i_in},
		{0x96, &data->p_out},
		{0x97, &data->p_in},
		{0x8d, &(data->temp_input[0])},
		{0x8e, &(data->temp_input[1])},
		{0x8f, &(data->temp_input[2])},
		{0x3b, &(data->fan_duty_cycle[0])},
		{0x3c, &(data->fan_duty_cycle[1])},
		{0x90, &data->fan_speed},
		{0xa0, &data->mfr_vin_min},
		{0xa1, &data->mfr_vin_max},
		{0xa2, &data->mfr_iin_max},
		{0xa3, &data->mfr_pin_max},
		{0xa4, &data->mfr_vout_min},
		{0xa5, &data->mfr_vout_max},
		{0xa6, &data->mfr_iout_max},
		{0xa7, &data->mfr_pout_max},
		{0xc3, &data->mfr_fan_speed_max},
		{0xc4, &data->mfr_fan_speed_min},
	};

	/* Read byte data */
	for (i = 0; i < ARRAY_SIZE(regs_byte); i++) {
		if (!pmbus_register_supported(driver_data->chip, regs_byte[i].reg)) {
			continue;
		}

		VALIDATE_POWERGOOD_AND_INTERVAL(client, &driver_data->access_interval);

		status = as9817_64_psu_read_byte(client, regs_byte[i].reg);
		if (status < 0) {
			dev_dbg(&client->dev, "reg 0x%02x, err %d\n",
				regs_byte[i].reg, status);
			*(regs_byte[i].value) = 0;
			goto exit;
		} else {
			*(regs_byte[i].value) = status;
		}
	}

	/* Read word data */
	for (i = 0; i < ARRAY_SIZE(regs_word); i++) {
		if (!pmbus_register_supported(driver_data->chip, regs_word[i].reg)) {
			continue;
		}

		VALIDATE_POWERGOOD_AND_INTERVAL(client, &driver_data->access_interval);

		status = as9817_64_psu_read_word(client, regs_word[i].reg);
		if (status < 0) {
			dev_dbg(&client->dev, "reg 0x%02x, err %d\n",
				regs_word[i].reg, status);
			*(regs_word[i].value) = 0;
			goto exit;
		} else {
			*(regs_word[i].value) = status;
		}
	}

	/* Read mfr_id */
	command = 0x99;
	if (pmbus_register_supported(driver_data->chip, command)) {
		VALIDATE_POWERGOOD_AND_INTERVAL(client, &driver_data->access_interval);

		status = as9817_64_psu_read_block(client, command, data->mfr_id,
						  ARRAY_SIZE(data->mfr_id) - 1);
		if (status < 0) {
			dev_dbg(&client->dev, "reg 0x%02x, err %d\n", command,
				status);
			goto exit;
		}

		NULL_TERMINATE_BY_LENGTH(mfr_id);
	}

	/* Read mfr_model */
	command = 0x9a;
	if (pmbus_register_supported(driver_data->chip, command)) {
		VALIDATE_POWERGOOD_AND_INTERVAL(client, &driver_data->access_interval);

		status = as9817_64_psu_read_block(client, command, data->mfr_model,
					     ARRAY_SIZE(data->mfr_model) - 1);

		if (status < 0) {
			dev_dbg(&client->dev, "reg 0x%02x, err %d\n", command,
				status);
			goto exit;
		}

		NULL_TERMINATE_BY_LENGTH(mfr_model);
	}

	/* Read fan_direction */
	command = 0xC3;
	if (pmbus_register_supported(driver_data->chip, command)) {
		char *fan_ptr;

		VALIDATE_POWERGOOD_AND_INTERVAL(client, &driver_data->access_interval);

		fan_ptr = get_fan_dir_by_model_name(client, data->mfr_model, fan_dir,
					      command);
		if (fan_ptr != NULL) {
			strncpy(data->fan_dir, fan_dir, ARRAY_SIZE(data->fan_dir) - 1);
			data->fan_dir[ARRAY_SIZE(data->fan_dir) - 1] = '\0';
		} else {
			status = as9817_64_psu_read_block(client, command, fan_dir,
						     ARRAY_SIZE(fan_dir) - 1);
			if (status < 0) {
				dev_dbg(&client->dev, "reg 0x%02x, err %d\n",
					command, status);
				goto exit;
			}
			strncpy(data->fan_dir, fan_dir + 1, ARRAY_SIZE(data->fan_dir) - 1);
			data->fan_dir[ARRAY_SIZE(data->fan_dir) - 1] = '\0';
		}
	}

	command = 0x9e;
	if (pmbus_register_supported(driver_data->chip, command)) {

		/* Read first byte to determine the length of data */
		VALIDATE_POWERGOOD_AND_INTERVAL(client, &driver_data->access_interval);

		status = as9817_64_psu_read_block(client, command, data->mfr_serial,
					     ARRAY_SIZE(data->mfr_serial) - 1);
		if (status < 0) {
			dev_dbg(&client->dev, "reg 0x%02x, err %d\n", command,
				status);
			goto exit;
		}

		NULL_TERMINATE_BY_LENGTH(mfr_serial);
	}

	/* Read mfr_revsion */
	command = 0x9b;
	if (pmbus_register_supported(driver_data->chip, command)) {
		VALIDATE_POWERGOOD_AND_INTERVAL(client, &driver_data->access_interval);

		status = as9817_64_psu_read_block(client, command, data->mfr_revsion,
					     ARRAY_SIZE(data->mfr_revsion) - 1);
		if (status < 0) {
			dev_dbg(&client->dev, "reg 0x%02x, err %d\n", command,
				status);
			goto exit;
		}

		NULL_TERMINATE_BY_LENGTH(mfr_revsion);
	}

	return 1;		/* Return 1 for valid data, 0 for invalid */

 exit:
	return 0;
}

static struct as9817_64_psu_data *as9817_64_psu_update_device(struct i2c_client
							      *client)
{
	struct as9817_64_psu_data *data = i2c_get_clientdata(client);
	int valid = 0;
	struct pmbus_register_value reg_val = { 0 };

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ + HZ / 2)
	    || !data->valid) {
		valid = as9817_64_psu_update_data(client, &reg_val);

		if (valid) {
			memcpy(&data->reg_val, &reg_val, sizeof(reg_val));
		} else {
			memset(&data->reg_val, 0, sizeof(reg_val));

			/* PMBus STATUS_WORD(0x79): psu_power_on, low byte bit 6, 0=>ON, 1=>OFF */
			data->reg_val.status_word |= 0x40;

			/* PMBus STATUS_WORD(0x79): psu_power_good, high byte bit 3, 0=>OK, 1=>FAIL */
			data->reg_val.status_word |= 0x800;
		}

		data->last_updated = jiffies;
		data->valid = 1;
	}

	mutex_unlock(&data->update_lock);

	return 0;
}

module_i2c_driver(as9817_64_psu_driver);
MODULE_AUTHOR("Roger Ho <roger530_ho@accton.com>");
MODULE_DESCRIPTION("AS9817-64 PSU driver");
MODULE_LICENSE("GPL");
