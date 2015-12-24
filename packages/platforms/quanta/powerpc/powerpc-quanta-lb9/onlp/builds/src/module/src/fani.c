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
#include <powerpc_quanta_lb9/powerpc_quanta_lb9_config.h>
#include <onlp/platformi/fani.h>

#include "powerpc_quanta_lb9_int.h"
#include "powerpc_quanta_lb9_log.h"

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
    const char* controller = powerpc_quanta_lb9_system_fan_dir();

    if(controller == NULL) {
        /* Error already reported. */
        return ONLP_STATUS_E_INTERNAL;
    }

    rv = onlp_file_read_int(&info->rpm,
                            "%s/fan%d_input", controller, id);

    if(rv == ONLP_STATUS_E_INTERNAL) {
        return rv;
    }

    if(rv == ONLP_STATUS_E_MISSING) {
        info->status &= ~1;
        return 0;
    }

    if(info->rpm <= POWERPC_QUANTA_LB9_R0_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD) {
        info->status |= ONLP_FAN_STATUS_FAILED;
    }

    /*
     * Calculate percentage based on current speed and the maximum.
     */
    info->caps |= ONLP_FAN_CAPS_GET_PERCENTAGE;
    if(info->status & ONLP_FAN_STATUS_F2B) {
        info->percentage = info->rpm * 100 / POWERPC_QUANTA_LB9_R0_CONFIG_SYSFAN_RPM_F2B_MAX;
    }
    if(info->status & ONLP_FAN_STATUS_B2F) {
        info->percentage = info->rpm * 100 / POWERPC_QUANTA_LB9_R0_CONFIG_SYSFAN_RPM_B2F_MAX;
    }

    return 0;
}

static int
psu_fan_info_get__(onlp_fan_info_t* info, int id)
{
    /* FAN5 -> PSU1 */
    /* FAN6 -> PSU2 */
    const char* dir =  powerpc_quanta_lb8_r9_system_psu_dir(id-4);

    if(dir == NULL) {
        /* Error already reported */
        return ONLP_STATUS_E_INTERNAL;
    }

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

    int direction = powerpc_quanta_lb9_system_airflow_get();
    int fid = ONLP_OID_ID_GET(id);

    if(direction == 0) {
        rv->status |= ONLP_FAN_STATUS_F2B;
        rv->caps |= ONLP_FAN_CAPS_F2B;
    }
    else {
        rv->status |= ONLP_FAN_STATUS_B2F;
        rv->caps |= ONLP_FAN_CAPS_B2F;
    }


    switch(fid)
        {
        case FAN_ID_FAN1:
        case FAN_ID_FAN2:
        case FAN_ID_FAN3:
        case FAN_ID_FAN4:
            {
                return sys_fan_info_get__(rv, fid);
            }

        case FAN_ID_FAN5:
        case FAN_ID_FAN6:
            {
                return psu_fan_info_get__(rv, fid);
            }
        }

    return ONLP_STATUS_E_INVALID;
}
