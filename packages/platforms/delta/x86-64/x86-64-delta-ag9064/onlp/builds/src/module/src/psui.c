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
 *
 *
 ***********************************************************/
#include <onlp/platformi/psui.h>
#include "x86_64_delta_ag9064_int.h"

#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlplib/mmap.h>

#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do                                          \
    {                                           \
        if(!ONLP_OID_IS_PSU(_id))               \
        {                                       \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(PSU_1), "PSU 1", 0 },
        {"DPS-1300AB-6 D"},
    },
    {
        { ONLP_PSU_ID_CREATE(PSU_2), "PSU 2", 0 },
        {"DPS-1300AB-6 D"},
    }
};

int onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

int onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int rv                = ONLP_STATUS_OK;
    int local_id          = 0;
    uint32_t PSUStatus    = 0;
    uint32_t PSUIsPresent = 0;
    uint32_t PSUIsGood    = 0;
    
    VALIDATE(id);
    
    local_id = ONLP_OID_ID_GET(id);
    *info = pinfo[local_id];
    
    rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_1_ADDR, PSU_STATUS_REGISTER, &PSUStatus, 1);
    
    if(rv == ONLP_STATUS_OK)
    {
        switch(local_id)
        {
            case PSU_1:
                PSUIsPresent = PSUStatus >> PSU1_PRESENT_BIT;
                PSUIsGood = (PSUStatus >> PSU1_POWER_GOOD_BIT) & 0x01;
                break;
            
            case PSU_2:
                PSUIsPresent = (PSUStatus >> PSU2_PRESENT_BIT) & 0x01;
                PSUIsGood = (PSUStatus >> PSU2_POWER_GOOD_BIT) & 0x01;
                break;
            
            default:
                AIM_LOG_ERROR("Invalid PSU ID!!\n");
                return ONLP_STATUS_E_PARAM;
        }
        
        if (PSUIsPresent != PSU_PRESENT_STATUS) 
        {
            info->status &= ~ONLP_PSU_STATUS_PRESENT;
            return ONLP_STATUS_OK;
        }
        else
        {
            info->status |= ONLP_PSU_STATUS_PRESENT;
        }
        
        if (PSUIsGood != PSU_POWER_GOOD_STATUS) 
        {
            info->status |=  ONLP_PSU_STATUS_FAILED;
        }
    }
    else
    {
        AIM_LOG_ERROR("Unable to read PSU present status: %d\r\n", rv);
        return ONLP_STATUS_E_INVALID;
    }
        
    return rv;
}

int onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
