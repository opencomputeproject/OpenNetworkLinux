/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2017 Delta Networks, Inc.
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
 * Fan Platform Implementation Defaults.
 *
 ***********************************************************/
#include <onlp/platformi/fani.h>
#include "x86_64_delta_wb2448_int.h"

#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlplib/mmap.h>

#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do                                          \
    {                                           \
        if(!ONLP_OID_IS_FAN(_id))               \
        {                                       \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)
		
enum onlp_fan_id
{    
    FAN_RESERVED = 0,    
    SYSTEM_FAN_1,    
    SYSTEM_FAN_2,    
};

/* Static values */
static onlp_fan_info_t linfo[] = 
{
	{ }, /* Not used */
	{ 
        { ONLP_FAN_ID_CREATE(SYSTEM_FAN_1), "Chassis System FAN 1", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
	{ 
        { ONLP_FAN_ID_CREATE(SYSTEM_FAN_2), "Chassis System FAN 2", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
};

/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}

int 
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    INT4 rv       = ONLP_STATUS_OK;
    INT4 local_id = 0;
    UINT4 u4Data  = 0;
            
    VALIDATE(id);
    
    local_id = ONLP_OID_ID_GET(id);
    
    *info = linfo[ONLP_OID_ID_GET(id)];	
    
    switch(local_id)
    {
        case SYSTEM_FAN_1:
            rv = ifnBmcFanSpeedGet("FanPWM_1", &u4Data);
            break;
            
        case SYSTEM_FAN_2:
            rv = ifnBmcFanSpeedGet("FanPWM_2", &u4Data);
            break;
                
        default:
            AIM_LOG_ERROR("Invalid Fan ID!!\n");
            rv = ONLP_STATUS_E_PARAM;
    }
    
    if(rv == ONLP_STATUS_OK)
    {
        info->rpm = u4Data;
        info->percentage = (info->rpm * 100) / X86_64_DELTA_WB2448_CONFIG_FAN_RPM_MAX;
    }
    
    return rv;
}

int 
onlp_fani_percentage_set(onlp_oid_t id, int p)
{
    INT4  local_id;
    
    VALIDATE(id);
    
    local_id = ONLP_OID_ID_GET(id);
    
    if ( p < 0 || p > 100)
    {
        AIM_LOG_ERROR("Invalid fan percentage !!");
        return ONLP_STATUS_E_PARAM;
    }
    
    if (ifnBmcFanSpeedSet(local_id - 1, p) != ONLP_STATUS_OK)
    {
        return ONLP_STATUS_E_INTERNAL;
    }
    
    return ONLP_STATUS_OK;
}

int 
onlp_fani_rpm_set(onlp_oid_t id, int rpm)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int 
onlp_fani_mode_set(onlp_oid_t id, onlp_fan_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int 
onlp_fani_ioctl(onlp_oid_t fid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
