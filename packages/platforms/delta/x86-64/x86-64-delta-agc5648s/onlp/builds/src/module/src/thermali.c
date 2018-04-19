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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <onlp/platformi/thermali.h>
#include "x86_64_delta_agc5648s_log.h"

#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlplib/mmap.h>
#include <onlplib/i2c.h>

#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do                                          \
    {                                           \
        if(!ONLP_OID_IS_THERMAL(_id))           \
        {                                       \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/* Static values */
static onlp_thermal_info_t linfo[] = {
    { }, /* Not used */
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_AMBI_ON_MAIN_BOARD), "Thermal Sensor (AMBI)", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_KBP1_ON_MAIN_BOARD), "Thermal Sensor (KBP1)", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_KBP2_ON_MAIN_BOARD), "Thermal Sensor (KBP2)", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_JER1_ON_MAIN_BOARD), "Thermal Sensor (JER1)", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_JER2_ON_MAIN_BOARD), "Thermal Sensor (JER2)", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
};

int onlp_thermali_init(void)
{
    return ONLP_STATUS_OK;
}

int onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    int rv             = ONLP_STATUS_OK;
    int local_id       = 0;
    int temp_base      = 1000;
    uint32_t temp_data = 0;
    
    VALIDATE(id);
    
    local_id = ONLP_OID_ID_GET(id);
    *info = linfo[local_id];
    
    switch(local_id)
    {
        case THERMAL_CPU_CORE:
            rv = onlp_i2c_read(I2C_BUS_1, THERMAL_CPU_ADDR, THERMAL_REGISTER, DATA_LEN, (uint8_t*)&temp_data, 0);
            break;
                    
        case THERMAL_AMBI_ON_MAIN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Temp_1", &temp_data);
            break;
            
        case THERMAL_KBP1_ON_MAIN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Temp_2", &temp_data);
            break;
            
        case THERMAL_KBP2_ON_MAIN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Temp_3", &temp_data);
            break;

        case THERMAL_JER1_ON_MAIN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Temp_4", &temp_data);
            break;

        case THERMAL_JER2_ON_MAIN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Temp_5", &temp_data);
            break;            
            
        default:
            AIM_LOG_ERROR("Invalid Thermal ID!!\n");
            return ONLP_STATUS_E_PARAM;
    }
    
    if(rv == ONLP_STATUS_OK)
    {
        info->mcelsius = temp_data * temp_base;
    }
    
    return rv;
}

int 
onlp_thermali_ioctl(int id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}