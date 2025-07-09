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
#include <onlplib/i2c.h>
#include <unistd.h>
#include <onlplib/file.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define PSU1_ID 1
#define PSU2_ID 2


#define PSU_PATH "/sys/switch/psu/"


 

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0 ,
            {
                ONLP_THERMAL_ID_CREATE(THERMAL_PSU1_TEMP_1),
                ONLP_THERMAL_ID_CREATE(THERMAL_PSU1_TEMP_2),
                ONLP_FAN_ID_CREATE(FAN_ON_PSU1),
            }
        },
    },
    {
        { ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0 ,
            {
                ONLP_THERMAL_ID_CREATE(THERMAL_PSU2_TEMP_1),
                ONLP_THERMAL_ID_CREATE(THERMAL_PSU2_TEMP_2),
                ONLP_FAN_ID_CREATE(FAN_ON_PSU2),
            }
        },
    }
};

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}


int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int pid, value;
    char path[80] = {0};
    int len;
    VALIDATE(id);
    pid  = ONLP_OID_ID_GET(id);

    *info = pinfo[pid]; /* Set the onlp_oid_hdr_t */

    /* Get the present status */
    // /sys/switch/psu/psu[n]/present
    sprintf(path, "%spsu%d/present", PSU_PATH, pid);
    if (0 > onlp_file_read_int(&value, path ))
        return ONLP_STATUS_E_INTERNAL;

    if (value!=1) {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;        
        return ONLP_STATUS_OK;
    }
    info->status |= ONLP_PSU_STATUS_PRESENT;
    info->caps = ONLP_PSU_CAPS_AC; 

    /* Get model name */
    // /sys_switch/psu/psu[n]/model_name
    sprintf(path, "%spsu%d/model_name", PSU_PATH, pid);
    if (0 > onlp_file_read((uint8_t*)info->model,sizeof(info->model),&len, path ))
        return ONLP_STATUS_E_INTERNAL;   
    /* Get serial number */
    // /sys_switch/psu/psu[n]/serial_number
    sprintf(path, "%spsu%d/serial_number", PSU_PATH, pid);
    if (0 > onlp_file_read((uint8_t*)info->serial,sizeof(info->serial),&len, path ))
        return ONLP_STATUS_E_INTERNAL;   
 
    /* Read vin */
    // /sys_switch/psu/psu[n]/in_vol
    sprintf(path, "%spsu%d/in_vol", PSU_PATH, pid);
    if (0 > onlp_file_read_int(&value, path ))
        return ONLP_STATUS_E_INTERNAL;   
    if (value > 0) {
        info->mvin = value;
        info->caps |= ONLP_PSU_CAPS_VIN;
    }else{
        info->status |=ONLP_PSU_STATUS_UNPLUGGED;
        return ONLP_STATUS_OK;
    }

    /* Get power good status */
    // /sys_switch/psu/psu[n]/out_status
    sprintf(path, "%spsu%d/out_status", PSU_PATH, pid);
    if (0 > onlp_file_read_int(&value, path ))
        return ONLP_STATUS_E_INTERNAL;    

    if (value!=1) {
        info->status |= ONLP_PSU_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }

    /* Get input output power status */



    /* Read iin */
    // /sys_switch/psu/psu[n]/in_curr
    sprintf(path, "%spsu%d/in_curr", PSU_PATH, pid);
    if (0 > onlp_file_read_int(&value, path ))
        return ONLP_STATUS_E_INTERNAL;   
    if (value >= 0) {
        info->miin = value;
        info->caps |= ONLP_PSU_CAPS_IIN;
    }
 
    /* Get pin */
    // /sys_switch/psu/psu[n]/in_power
    sprintf(path, "%spsu%d/in_power", PSU_PATH, pid);
    if (0 > onlp_file_read_int(&value, path ))
        return ONLP_STATUS_E_INTERNAL;   
    if (value >= 0) {
        info->mpin = value;
        info->caps |= ONLP_PSU_CAPS_PIN;
    }

    /* Read iout */
    // /sys_switch/psu/psu[n]/out_curr
    sprintf(path, "%spsu%d/out_curr", PSU_PATH, pid);
    if (0 > onlp_file_read_int(&value, path ))
        return ONLP_STATUS_E_INTERNAL;   
    if (value >= 0) {
        info->miout = value;
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }

    /* Read pout */
    // /sys_switch/psu/psu[n]/out_power
    sprintf(path, "%spsu%d/out_power", PSU_PATH, pid);
    if (0 > onlp_file_read_int(&value, path ))
        return ONLP_STATUS_E_INTERNAL;   
    if (value >= 0) {
        info->mpout = value;
        info->caps |= ONLP_PSU_CAPS_POUT;
    }

    /* Get vout */
    // /sys_switch/psu/psu[n]/out_vol
    sprintf(path, "%spsu%d/out_vol", PSU_PATH, pid);
    if (0 > onlp_file_read_int(&value, path ))
        return ONLP_STATUS_E_INTERNAL;   
    if (value >= 0) {
        info->mvout = value;
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }


    return ONLP_STATUS_OK;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
