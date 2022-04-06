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

#define AIM_FREE_IF_PTR(p) \
    do \
    { \
        if (p) { \
            aim_free(p); \
            p = NULL; \
        } \
    } while (0)

static int
psu_status_info_get(int id, char *node, int *value)
{
    int  ret = 0;
    *value = 0;
    char *path[] = { PSU1_AC_EEPROM_PREFIX, PSU2_AC_EEPROM_PREFIX };

    ret = onlp_file_read_int(value, "%s%s", path[id-1], node);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ret;
}

static int
psu_pmbus_info_get(int id, char *node, int *value)
{
    int  ret = 0;
    *value = 0;
    char *path[] = { PSU1_AC_PMBUS_PREFIX, PSU2_AC_PMBUS_PREFIX };

    ret = onlp_file_read_int(value, "%s%s", path[id-1], node);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ret;
}

int get_psu_eeprom_str(int id, char *data_buf, int data_len, char *data_name)
{
    int   len    = 0;
    char *path[] = { PSU1_AC_EEPROM_PREFIX, PSU2_AC_EEPROM_PREFIX };
    char *str = NULL;

    /* Read attribute */
    len = onlp_file_read_str(&str, "%s%s", path[id-1], data_name);
    if (!str || len <= 0) {
        AIM_FREE_IF_PTR(str);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (len > data_len) {
        AIM_FREE_IF_PTR(str);
        return ONLP_STATUS_E_INVALID;
    }

    aim_strlcpy(data_buf, str, len+1);
    AIM_FREE_IF_PTR(str);
    return ONLP_STATUS_OK;
}

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
    VALIDATE(id);

    memset(info, 0, sizeof(onlp_psu_info_t));
    *info = pinfo[pid]; /* Set the onlp_oid_hdr_t */

    /* Get the present state */
    if (psu_status_info_get(pid, "psu_present", &val) != 0) {
        AIM_LOG_ERROR("Unable to read present status from psu(%d)\r\n", pid);
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_E_INTERNAL;
    }

    if (val != PSU_STATUS_PRESENT) {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    }
    info->status |= ONLP_PSU_STATUS_PRESENT;


    /* Get power good status */
    if (psu_status_info_get(pid, "psu_power_good", &val) != 0) {
        AIM_LOG_ERROR("Unable to read power good status from psu(%d)\r\n", pid);
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

    if (info->status & ONLP_PSU_STATUS_FAILED) {
        return ONLP_STATUS_OK;
    }

    /* Set the associated oid_table */
    info->hdr.coids[0] = ONLP_THERMAL_ID_CREATE(CHASSIS_THERMAL_COUNT + (pid-1)*NUM_OF_THERMAL_PER_PSU + 1);
    info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(CHASSIS_THERMAL_COUNT + (pid-1)*NUM_OF_THERMAL_PER_PSU + 2);

    /* Read voltage, current and power */
    if (psu_pmbus_info_get(pid, "psu_v_in", &val) == 0) {
        info->mvin  = val;
        info->caps |= ONLP_PSU_CAPS_VIN;
    }

    if (psu_pmbus_info_get(pid, "psu_i_in", &val) == 0) {
        info->miin = val;
        info->caps |= ONLP_PSU_CAPS_IIN;
    }

    if (psu_pmbus_info_get(pid, "psu_p_in", &val) == 0) {
        info->mpin  = val;
        info->caps |= ONLP_PSU_CAPS_PIN;
    }

    if (psu_pmbus_info_get(pid, "psu_v_out", &val) == 0) {
        info->mvout = val;
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }

    if (psu_pmbus_info_get(pid, "psu_i_out", &val) == 0) {
        info->miout = val;
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }

    if (psu_pmbus_info_get(pid, "psu_p_out", &val) == 0) {
        info->mpout = val;
        info->caps |= ONLP_PSU_CAPS_POUT;
    }

    get_psu_eeprom_str(pid, info->model, AIM_ARRAYSIZE(info->model), "psu_model_name");
    get_psu_eeprom_str(pid, info->serial, AIM_ARRAYSIZE(info->serial), "psu_serial_number");

    return ret;
}
