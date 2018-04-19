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
#include "x86_64_delta_agc5648s_int.h"

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

/* Static values */
static onlp_fan_info_t linfo[] = 
{
    { }, /* Not used */
    { 
        { ONLP_FAN_ID_CREATE(FAN_1_ON_FAN_BOARD), "Chassis Fan 1", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
    {
        { ONLP_FAN_ID_CREATE(FAN_2_ON_FAN_BOARD), "Chassis Fan 2", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
    { 
        { ONLP_FAN_ID_CREATE(FAN_3_ON_FAN_BOARD), "Chassis Fan 3", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
    { 
        { ONLP_FAN_ID_CREATE(FAN_4_ON_FAN_BOARD), "Chassis Fan 4", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
    { 
        { ONLP_FAN_ID_CREATE(FAN_5_ON_FAN_BOARD), "Chassis Fan 5", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
    { 
        { ONLP_FAN_ID_CREATE(FAN_6_ON_FAN_BOARD), "Chassis Fan 6", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
    { 
        { ONLP_FAN_ID_CREATE(FAN_7_ON_FAN_BOARD), "Chassis Fan 7", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
    { 
        { ONLP_FAN_ID_CREATE(FAN_8_ON_FAN_BOARD), "Chassis Fan 8", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
    { 
        { ONLP_FAN_ID_CREATE(FAN_9_ON_FAN_BOARD), "Chassis Fan 9", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
    { 
        { ONLP_FAN_ID_CREATE(FAN_10_ON_FAN_BOARD), "Chassis Fan 10", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
    { 
        { ONLP_FAN_ID_CREATE(FAN_ON_PSU1), "FAN ON PSU1", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
    { 
        { ONLP_FAN_ID_CREATE(FAN_ON_PSU2), "FAN ON PSU2", 0},
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, 0, 0, ONLP_FAN_MODE_INVALID,
    },
};

int onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}

static int ifnLinearDataToDecimal(uint16_t u16Value, uint16_t u16ManLen, uint16_t u16ExpLen)
{
    uint8_t  index;
    uint16_t ManMask = 1;
    uint16_t ExpMask = 1;
    uint16_t Mantissa, Exponent;
    
    for(index = 1; index < u16ManLen; index++)
    {
        ManMask <<= 1;
        ManMask |= 0x1;
    }
    
    for(index = 1; index < u16ExpLen; index++)
    {
        ExpMask <<= 1;
        ExpMask |= 0x1;
    }
    
    /* Didn't check the negative bit */
    Mantissa = u16Value & ManMask;
    Exponent = u16Value >> u16ManLen;
    
    for(index = 0; index < Exponent; index++)
    {
        Mantissa *= 2;
    }
    
    return Mantissa;
}


static int dni_fani_info_get_on_fanboard(int local_id, onlp_fan_info_t* info)
{
    int rv = ONLP_STATUS_OK;
    uint32_t FanSpeed = 0;
    
    switch(local_id)
    {
        case FAN_1_ON_FAN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Fan_1", &FanSpeed);
            break;
            
        case FAN_2_ON_FAN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Fan_2", &FanSpeed);
            break;
            
        case FAN_3_ON_FAN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Fan_3", &FanSpeed);
            break;
            
        case FAN_4_ON_FAN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Fan_4", &FanSpeed);
            break;
        
        case FAN_5_ON_FAN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Fan_5", &FanSpeed);
            break;
            
        case FAN_6_ON_FAN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Fan_6", &FanSpeed);
            break;
            
        case FAN_7_ON_FAN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Fan_7", &FanSpeed);
            break;
            
        case FAN_8_ON_FAN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Fan_8", &FanSpeed);
            break;
            
        case FAN_9_ON_FAN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Fan_9", &FanSpeed);
            break;

        case FAN_10_ON_FAN_BOARD:
            rv = ifnOS_LINUX_BmcGetDataByName("Fan_10", &FanSpeed);
            break;
            
        default:
            AIM_LOG_ERROR("Invalid Fan ID!!\n");
            rv = ONLP_STATUS_E_INVALID;
    }
    
    if(rv == ONLP_STATUS_OK)
    {
        info->rpm = FanSpeed;
        info->percentage = (info->rpm * 100) / X86_64_DELTA_AGC5648S_CONFIG_FAN_RPM_MAX;
    }
    
    return ONLP_STATUS_OK;
}


static int dni_fani_info_get_on_psu(int local_id, onlp_fan_info_t* info)
{
    int rv = ONLP_STATUS_OK;
    uint32_t FanSpeed = 0;
    
    switch(local_id)
    {
        case FAN_ON_PSU1:
            ifnOS_LINUX_BmcI2CSet(I2C_BMC_BUS_3, SWPLD_1_ADDR, PSU_I2C_MUX_ADDR, PSU1_EEPORM_CHANNEL, DATA_LEN);
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_2, FAN_ON_PSU1_ADDR, PMBUS_FAN_SPEED, &FanSpeed, 2);
            break;
            
        case FAN_ON_PSU2:
            ifnOS_LINUX_BmcI2CSet(I2C_BMC_BUS_3, SWPLD_1_ADDR, PSU_I2C_MUX_ADDR, PSU2_EEPORM_CHANNEL << 4, DATA_LEN);
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_2, FAN_ON_PSU2_ADDR, PMBUS_FAN_SPEED, &FanSpeed, 2);
            break;
                
        default:
            AIM_LOG_ERROR("Invalid Fan ID!!\n");
            return ONLP_STATUS_E_INVALID;
    }
    
    if(rv == ONLP_STATUS_OK)
    {
        FanSpeed = ((FanSpeed >> 8) | (FanSpeed << 8));
        FanSpeed = ifnLinearDataToDecimal( FanSpeed, 11, 5 );
        info->rpm = FanSpeed;
        info->percentage = (info->rpm * 100) / X86_64_DELTA_AGC5648S_CONFIG_FAN_RPM_MAX;
    }
    
    return rv;
}

int onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    int rv       = ONLP_STATUS_OK;
    int local_id = 0;
                
    VALIDATE(id);
    
    local_id = ONLP_OID_ID_GET(id);
    *info = linfo[ONLP_OID_ID_GET(id)];
    
    switch(local_id)
    {
        case FAN_1_ON_FAN_BOARD:
        case FAN_2_ON_FAN_BOARD:
        case FAN_3_ON_FAN_BOARD:
        case FAN_4_ON_FAN_BOARD:
        case FAN_5_ON_FAN_BOARD:
        case FAN_6_ON_FAN_BOARD:
        case FAN_7_ON_FAN_BOARD:
        case FAN_8_ON_FAN_BOARD:
        case FAN_9_ON_FAN_BOARD:
        case FAN_10_ON_FAN_BOARD:
            rv = dni_fani_info_get_on_fanboard(local_id, info);
            break;
            
        case FAN_ON_PSU1:
        case FAN_ON_PSU2:
            rv = dni_fani_info_get_on_psu(local_id, info);
            break;
                
        default:
            AIM_LOG_ERROR("Invalid Fan ID!!\n");
            rv = ONLP_STATUS_E_INVALID;
    }
    
    return rv;
}

int onlp_fani_percentage_set(onlp_oid_t id, int p)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int onlp_fani_rpm_set(onlp_oid_t id, int rpm)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int onlp_fani_mode_set(onlp_oid_t id, onlp_fan_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int onlp_fani_ioctl(onlp_oid_t fid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
