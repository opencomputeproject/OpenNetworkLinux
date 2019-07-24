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
#include "x86_64_delta_wb2448_log.h"

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
		
#define THERMAL_CPU_I2C_BUS	0x00
#define THERMAL_12_I2C_BUS	0x01
#define THERMAL_34_I2C_BUS	0x04
#define THERMAL_CPU_ADDR	0x4D
#define THERMAL_1_ADDR		0x4B
#define THERMAL_2_ADDR		0x4C
#define THERMAL_3_ADDR		0x49
#define THERMAL_4_ADDR		0x4A
#define THERMAL_REGISTER	0x00
#define THERMAL_ADDR_LEN	0x01
#define THERMAL_DATA_LEN	0x01

enum onlp_thermal_id
{    
    THERMAL_RESERVED = 0,    
    THERMAL_CPU_CORE,    
    THERMAL_1_ON_MAIN_BROAD,    
    THERMAL_2_ON_MAIN_BROAD,    
    THERMAL_3_ON_MAIN_BROAD,    
    THERMAL_4_ON_MAIN_BROAD,
};

/* Static values */
static onlp_thermal_info_t linfo[] = {
    { }, /* Not used */
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "Chassis Thermal Sensor 1", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "Chassis Thermal Sensor 2", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "Chassis Thermal Sensor 3", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "Chassis Thermal Sensor 4", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
};

int
onlp_thermali_init(void)
{
    return ONLP_STATUS_OK;
}

int 
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    INT4 rv         = ONLP_STATUS_OK;
    INT4 local_id   = 0;
    INT4 temp_base  = 1000;
    UINT4 u4Data    = 0;
            
    VALIDATE(id);
    
    local_id = ONLP_OID_ID_GET(id);
    
    *info = linfo[ONLP_OID_ID_GET(id)];	
    
    switch(local_id)
    {
        case THERMAL_CPU_CORE:
            rv = ifnOS_LINUX_BmcI2CGet(THERMAL_CPU_I2C_BUS, THERMAL_CPU_ADDR, THERMAL_REGISTER, THERMAL_ADDR_LEN, &u4Data, THERMAL_DATA_LEN);
            break;
            
        case THERMAL_1_ON_MAIN_BROAD:
            rv = ifnOS_LINUX_BmcI2CGet(THERMAL_12_I2C_BUS, THERMAL_1_ADDR, THERMAL_REGISTER, THERMAL_ADDR_LEN, &u4Data, THERMAL_DATA_LEN);
            break;
            
        case THERMAL_2_ON_MAIN_BROAD:
            rv = ifnOS_LINUX_BmcI2CGet(THERMAL_12_I2C_BUS, THERMAL_2_ADDR, THERMAL_REGISTER, THERMAL_ADDR_LEN, &u4Data, THERMAL_DATA_LEN);
            break;
            
        case THERMAL_3_ON_MAIN_BROAD:
            rv = ifnOS_LINUX_BmcI2CGet(THERMAL_34_I2C_BUS, THERMAL_3_ADDR, THERMAL_REGISTER, THERMAL_ADDR_LEN, &u4Data, THERMAL_DATA_LEN);
            break;
            
        case THERMAL_4_ON_MAIN_BROAD:
            rv = ifnOS_LINUX_BmcI2CGet(THERMAL_34_I2C_BUS, THERMAL_4_ADDR, THERMAL_REGISTER, THERMAL_ADDR_LEN, &u4Data, THERMAL_DATA_LEN);
            break;		
            
        default:
            AIM_LOG_ERROR("Invalid Thermal ID!!\n");
            return ONLP_STATUS_E_PARAM;
    }
    
    if(rv == ONLP_STATUS_OK)
    {
        info->mcelsius = u4Data * temp_base;
    }
    
    return rv;
}

int 
onlp_thermali_ioctl(int id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}