/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <onlplib/file.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#define PSU_STATUS_PRESENT 1
#define PSU_STATUS_POWER_GOOD 1

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] = {
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0, {0} },
    },
    {
        { ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0, {0} },
    }
};

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int val   = 0;
    int ret   = ONLP_STATUS_OK;
    int pid = ONLP_OID_ID_GET(id);
    int thermal_count = 0;
    VALIDATE(id);

    memset(info, 0, sizeof(onlp_psu_info_t));
    *info = pinfo[pid]; /* Set the onlp_oid_hdr_t */

    /* Get the present state */
    ret = onlp_file_read_int(&val, PSU_STATUS_SYSFS_FORMAT, pid, "present");
    if (ret < 0) {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_E_INTERNAL;
    }

    if (val != PSU_STATUS_PRESENT) {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    }
    info->status |= ONLP_PSU_STATUS_PRESENT;

    /* Get power good status */
    ret = onlp_file_read_int(&val, PSU_STATUS_SYSFS_FORMAT, pid, "power_good");
    if (ret < 0) {
        info->status |=  ONLP_PSU_STATUS_FAILED;
        return ONLP_STATUS_E_INTERNAL;
    }

    if (val != PSU_STATUS_POWER_GOOD) {
        info->status |=  ONLP_PSU_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }

    /* Set capability
     */
    info->caps = ONLP_PSU_CAPS_AC;

    /* Set the associated oid_table */
    thermal_count = CHASSIS_THERMAL_COUNT;

    info->hdr.coids[0] = ONLP_THERMAL_ID_CREATE(thermal_count + (pid-1)*NUM_OF_THERMAL_PER_PSU + 1);
    info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(thermal_count + (pid-1)*NUM_OF_THERMAL_PER_PSU + 2);
    info->hdr.coids[2] = ONLP_THERMAL_ID_CREATE(thermal_count + (pid-1)*NUM_OF_THERMAL_PER_PSU + 3);
    info->hdr.coids[3] = ONLP_FAN_ID_CREATE(pid + CHASSIS_FAN_COUNT);

    /* Read voltage, current and power */
    val = 0;
    ret = psu_pmbus_info_get(pid, "psu_v_in", &val);
    if (ret == ONLP_STATUS_OK && val) {
        info->mvin  = val;
        info->caps |= ONLP_PSU_CAPS_VIN;
    }

    val = 0;
    ret = psu_pmbus_info_get(pid, "psu_i_in", &val);
    if (ret == ONLP_STATUS_OK && val) {
        info->miin  = val;
        info->caps |= ONLP_PSU_CAPS_IIN;
    }

    val = 0;
    ret = psu_pmbus_info_get(pid, "psu_p_in", &val);
    if (ret == ONLP_STATUS_OK && val) {
        info->mpin  = val;
        info->caps |= ONLP_PSU_CAPS_PIN;
    }

    val = 0;
    ret = psu_pmbus_info_get(pid, "psu_v_out", &val);
    if (ret == ONLP_STATUS_OK && val) {
        info->mvout = val;
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }

    val = 0;
    ret = psu_pmbus_info_get(pid, "psu_i_out", &val);
    if (ret == ONLP_STATUS_OK && val) {
        info->miout = val;
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }

    val = 0;
    ret = psu_pmbus_info_get(pid, "psu_p_out", &val);
    if (ret == ONLP_STATUS_OK && val) {
        info->mpout = val;
        info->caps |= ONLP_PSU_CAPS_POUT;
    }

    psu_pmbus_str_get(pid, info->model, sizeof(info->model), "psu_mfr_model");
    psu_pmbus_str_get(pid, info->serial, sizeof(info->serial), "psu_mfr_serial");
    return ONLP_STATUS_OK;
}
