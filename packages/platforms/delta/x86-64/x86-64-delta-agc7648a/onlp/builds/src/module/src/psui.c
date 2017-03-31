/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017 Delta Networks, Inc.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <onlp/platformi/psui.h>
#include <onlplib/mmap.h>
#include <stdio.h>
#include <string.h>
#include "platform_lib.h"
#include <onlplib/i2c.h>

#define PSU_STATUS_PRESENT    1
#define PSU_STATUS_POWER_GOOD 1

#define PSU_NODE_MAX_INT_LEN  8
#define PSU_NODE_MAX_PATH_LEN 64

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int
dni_psu_pmbus_info_get(int id, char *node, int *value)
{
    int  ret = 0;
    char buf[PSU_NODE_MAX_INT_LEN + 1]    = {0};
    char node_path[PSU_NODE_MAX_PATH_LEN] = {0};

    *value = 0;

    if (PSU1_ID == id) {
        sprintf(node_path, "%s%s", PSU1_AC_PMBUS_PREFIX, node);
    }
    else {
        sprintf(node_path, "%s%s", PSU2_AC_PMBUS_PREFIX, node);
    }

    ret = dni_i2c_read_attribute_string(node_path, buf, sizeof(buf), 0);

    if (ret == 0) {
        *value = atoi(buf);
    }

    return ret;
}

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

int
dni_psu_present_get(int index)
{
    int state = 0;
    uint8_t present_val;

    present_val = dni_lock_cpld_read_attribute(SWPLD_PATH, PSU_PRESENT_REG);

    if(index == 1) {
        if((present_val & 0x01) == 0x00) {
            state = 1;
        } else {
            state = 0;
        }
    } else {
        if((present_val & 0x02) == 0x00) {
            state = 1;
        } else {
            state = 0;
        }
    }
    return state;
}

static int
dni_psu_info_get(onlp_psu_info_t* info)
{
    int val   = 0;
    int index = ONLP_OID_ID_GET(info->hdr.id);

    /* Set capability
     */
    info->caps = ONLP_PSU_CAPS_AC;


	if (info->status & ONLP_PSU_STATUS_FAILED) {
	    return ONLP_STATUS_OK;
	}

    /* Set the associated oid_table */
    info->hdr.coids[0] = ONLP_FAN_ID_CREATE(index + CHASSIS_FAN_COUNT);
    info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(index + CHASSIS_THERMAL_COUNT);


    char val_char[16] = { '\0' };
    char node_path[PSU_NODE_MAX_PATH_LEN] = { '\0' };
    sprintf(node_path, "%s%s", PSU1_AC_PMBUS_PREFIX, "psu_mfr_model");
    dni_i2c_read_attribute_string(node_path, val_char, sizeof(val_char), 0);
    strcpy(info->model, val_char);

    sprintf(node_path, "%s%s", PSU1_AC_PMBUS_PREFIX, "psu_mfr_serial");
    dni_i2c_read_attribute_string(node_path, val_char, sizeof(val_char), 0);
    strcpy(info->serial, val_char);

    /* Read voltage, current and power */
    if (dni_psu_pmbus_info_get(index, "psu_v_out", &val) == 0) {
        info->mvout = val;
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }

    if (dni_psu_pmbus_info_get(index, "psu_v_in", &val) == 0) {
        info->mvin = val;
        info->caps |= ONLP_PSU_CAPS_VIN;
    }

    if (dni_psu_pmbus_info_get(index, "psu_i_out", &val) == 0) {
        info->miout = val;
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }

    if (dni_psu_pmbus_info_get(index, "psu_i_in", &val) == 0) {
        info->miin = val;
        info->caps |= ONLP_PSU_CAPS_IIN;
    }

    if (dni_psu_pmbus_info_get(index, "psu_p_out", &val) == 0) {
        info->mpout = val;
        info->caps |= ONLP_PSU_CAPS_POUT;
    }

    if (dni_psu_pmbus_info_get(index, "psu_p_in", &val) == 0) {
        info->mpin = val;
        info->caps |= ONLP_PSU_CAPS_PIN;
    }

    return ONLP_STATUS_OK;
}

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0 },
    },
    {
        { ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0 },
    }
};

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int val, ret;
    int index = ONLP_OID_ID_GET(id);
    uint8_t power_good_val;

    *info = pinfo[index]; /* Set the onlp_oid_hdr_t */

    if(dni_psu_present_get(index) != 1) {
        /* PSU(%d) present fail */
        info->status |= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    } else {
        info->status |= ONLP_PSU_STATUS_PRESENT;
    }

    /* Get power good status */
    power_good_val = dni_lock_cpld_read_attribute(SWPLD_PATH, PSU_PWR_REG);
    if(index == 1){
        if((power_good_val & 0x80) == 0x80) {
            val = PSU_STATUS_POWER_GOOD;
        } else {
            val = 0;
        }
    } else if(index == 2) {
        if((power_good_val & 0x40) == 0x40) {
            val = PSU_STATUS_POWER_GOOD;
        } else {
            val = 0;
        }
    }
    if(val != PSU_STATUS_POWER_GOOD) {
        /* Unable to read PSU(%d) node(psu_power_good) */
        info->status |= ONLP_PSU_STATUS_UNPLUGGED;
    }

    /* select PSU(%d) module */
    char index_data[2];
    sprintf(index_data, "%d", index);
    dni_i2c_lock_write_attribute(NULL, index_data, PSU_SEL_PATH);

    ret = dni_psu_info_get(info);

    return ret;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

