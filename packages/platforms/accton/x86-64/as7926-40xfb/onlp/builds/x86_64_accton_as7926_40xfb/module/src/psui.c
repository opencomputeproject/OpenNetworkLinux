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
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#define AIM_FREE_IF_PTR(p) \
    do \
    { \
        if (p) { \
            aim_free(p); \
            p = NULL; \
        } \
    } while (0)

int
onlp_psui_sw_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_psui_sw_denit(void)
{
    return ONLP_STATUS_OK;
}

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", ONLP_OID_CHASSIS, { 0 }, 0 },
        { 0 }, { 0 }, 0, 0, 0, 0, 0, 0, 0, 0,
    },
    {
        { ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", ONLP_OID_CHASSIS, { 0 }, 0 },
        { 0 }, { 0 }, 0, 0, 0, 0, 0, 0, 0, 0,
    }
};

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int val = 0;
    int ret = ONLP_STATUS_OK;
    int pid = ONLP_OID_ID_GET(id);

    *info = pinfo[pid]; /* Set the onlp_oid_hdr_t */

    /* Get the present state */
    ONLP_TRY(onlp_file_read_int(&val, "%s""psu%d_present", PSU_SYSFS_PATH, pid));
    if (val != PSU_STATUS_PRESENT) {
        ONLP_OID_STATUS_FLAG_CLR(info, PRESENT);
        return ONLP_STATUS_OK;
    }

    ONLP_OID_STATUS_FLAG_SET(info, PRESENT);
    info->type = ONLP_PSU_TYPE_AC;

    /* Get power good status */
    ONLP_TRY(onlp_file_read_int(&val, "%s""psu%d_power_good", PSU_SYSFS_PATH, pid));
    if (val != PSU_STATUS_POWER_GOOD) {
        ONLP_OID_STATUS_FLAG_SET(info, FAILED);
    }

    /* Read voltage, current and power */
    val = 0;
    ret = onlp_file_read_int(&val, "%s""psu%d_vin", PSU_SYSFS_PATH, pid);
    if (ONLP_SUCCESS(ret) && val) {
        info->mvin  = val;
        info->caps |= ONLP_PSU_CAPS_GET_VIN;
    }

    val = 0;
    ret = onlp_file_read_int(&val, "%s""psu%d_vout", PSU_SYSFS_PATH, pid);
    if (ONLP_SUCCESS(ret) && val) {
        info->mvout = val;
        info->caps |= ONLP_PSU_CAPS_GET_VOUT;
    }

    val = 0;
    ret = onlp_file_read_int(&val, "%s""psu%d_iin", PSU_SYSFS_PATH, pid);
    if (ONLP_SUCCESS(ret) && val) {
        info->miin = val;
        info->caps |= ONLP_PSU_CAPS_GET_IIN;
    }

    val = 0;
    ret = onlp_file_read_int(&val, "%s""psu%d_iout", PSU_SYSFS_PATH, pid);
    if (ONLP_SUCCESS(ret) && val) {
        info->miout = val;
        info->caps |= ONLP_PSU_CAPS_GET_IOUT;
    }

    val = 0;
    ret = onlp_file_read_int(&val, "%s""psu%d_pin", PSU_SYSFS_PATH, pid);
    if (ONLP_SUCCESS(ret) && val) {
        info->mpin = val;
        info->caps |= ONLP_PSU_CAPS_GET_PIN;
    }

    val = 0;
    ret = onlp_file_read_int(&val, "%s""psu%d_pout", PSU_SYSFS_PATH, pid);
    if (ONLP_SUCCESS(ret) && val) {
        info->mpout = val;
        info->caps |= ONLP_PSU_CAPS_GET_POUT;
    }

    /* Set the associated oid_table */
    val = 0;
    ret = onlp_file_read_int(&val, "%s""psu%d_fan1_input", PSU_SYSFS_PATH, pid);
    if (ONLP_SUCCESS(ret)) {
        info->hdr.coids[0] = ONLP_FAN_ID_CREATE(pid + CHASSIS_FAN_COUNT);
    }

    val = 0;
    ret = onlp_file_read_int(&val, "%s""psu%d_temp1_input", PSU_SYSFS_PATH, pid);
    if (ONLP_SUCCESS(ret)) {
        info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(pid + CHASSIS_THERMAL_COUNT);
    }

    /* Read model */
    char *string = NULL;
    int len = onlp_file_read_str(&string, "%s""psu%d_model", PSU_SYSFS_PATH, pid);
    if (string && len) {
        memcpy(info->model, string, len);
        info->model[len] = '\0';
    }
    AIM_FREE_IF_PTR(string);

    /* Read serial */
    len = onlp_file_read_str(&string, "%s""psu%d_serial", PSU_SYSFS_PATH, pid);
    if (string && len) {
        memcpy(info->serial, string, len);
        info->serial[len] = '\0';
    }
    AIM_FREE_IF_PTR(string);

    return ONLP_STATUS_OK;
}

int
onlp_psui_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    onlp_psu_info_t info;

    ONLP_TRY(onlp_psui_info_get(id, &info));
    *hdr = info.hdr;
    return 0;
}

int
onlp_psui_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, PSU1_ID, PSU2_ID);
}
