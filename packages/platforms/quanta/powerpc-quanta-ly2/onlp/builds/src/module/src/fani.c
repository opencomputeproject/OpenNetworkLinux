/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
#include <powerpc_quanta_ly2/powerpc_quanta_ly2_config.h>
#include <onlp/platformi/fani.h>

#include "powerpc_quanta_ly2_int.h"
#include "powerpc_quanta_ly2_log.h"

#include <onlplib/file.h>

int
onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}


static int
sys_fan_info_get__(onlp_fan_info_t* info, int id)
{
    int rv;

    rv = onlp_file_read_int(&info->rpm,
                            SYS_HWMON_PREFIX "/fan%d_input", id);

    if(rv == ONLP_STATUS_E_INTERNAL) {
        return rv;
    }

    if(rv == ONLP_STATUS_E_MISSING) {
        info->status &= ~1;
        return 0;
    }

    if(info->rpm <= POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD) {
        info->status |= ONLP_FAN_STATUS_FAILED;
    }

    /*
     * Calculate percentage based on current speed and the maximum.
     */
    info->caps |= ONLP_FAN_CAPS_GET_PERCENTAGE;
    if(info->status & ONLP_FAN_STATUS_F2B) {
        info->percentage = info->rpm * 100 / POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_F2B_RPM_MAX;
    }
    if(info->status & ONLP_FAN_STATUS_B2F) {
        info->percentage = info->rpm * 100 / POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_B2F_RPM_MAX;
    }
    return 0;
}

static int
psu_fan_info_get__(onlp_fan_info_t* info, int id)
{
    extern char* psu_paths[];

    /* FAN5 -> PSU1 */
    /* FAN6 -> PSU2 */
    char* dir = psu_paths[id-4];
    return onlp_file_read_int(&info->rpm, "%s/fan1_input", dir);
}



/* Onboard Fans */
static onlp_fan_info_t fans__[] = {
    { }, /* Not used */
    { { FAN_OID_FAN1,  "Right", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN2,  "Center Right", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN3,  "Center Left", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN4,  "Left", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN5,  "PSU-1 Fan", 0 }, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN6,  "PSU-2 Fan", 0 }, ONLP_FAN_STATUS_PRESENT },

};

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* rv)
{
    *rv = fans__[ONLP_OID_ID_GET(id)];
    rv->caps |= ONLP_FAN_CAPS_GET_RPM;

    int len;
    char direction[16] = {0};

    int fid = ONLP_OID_ID_GET(id);

    /* Read the current airflow direction */
    onlp_file_read((uint8_t*)direction, sizeof(direction), &len,
                   SYS_HWMON_PREFIX "/fan_dir");

#define FAN_DIR_F2B "front-to-back"
#define FAN_DIR_B2F "back-to-front"

    if(!strncmp(direction, FAN_DIR_F2B, strlen(FAN_DIR_F2B))) {
        rv->status |= ONLP_FAN_STATUS_F2B;
        rv->caps |= ONLP_FAN_CAPS_F2B;
    }
    else if(!strncmp(direction, FAN_DIR_B2F, strlen(FAN_DIR_B2F))) {
        rv->status |= ONLP_FAN_STATUS_B2F;
        rv->caps |= ONLP_FAN_CAPS_B2F;
    }
    else {
        AIM_LOG_WARN("Invalid fan direction: '%s'", direction);
    }


    switch(fid)
        {
        case FAN_ID_FAN1:
        case FAN_ID_FAN2:
        case FAN_ID_FAN3:
        case FAN_ID_FAN4:
            {
                if(rv->status & ONLP_FAN_STATUS_F2B) {
                    return sys_fan_info_get__(rv, fid);
                }
                if(rv->status & ONLP_FAN_STATUS_B2F) {
                    return sys_fan_info_get__(rv, fid+4);
                }
                return ONLP_STATUS_E_INTERNAL;
            }

        case FAN_ID_FAN5:
        case FAN_ID_FAN6:
            {
                return psu_fan_info_get__(rv, fid);
            }
        }

    return ONLP_STATUS_E_INVALID;
}


