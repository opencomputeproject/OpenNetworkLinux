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
        if(!ONLP_OID_IS_PSU(_id))               \
        {                                       \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define I2C_BUS                 0x04
#define CPLD_ADDR               0x28
#define PSU_PRESENT_REGISTER    0x04
#define PSU_STATUS_REGISTER     0x05
#define PSU_ADDR_LEN            1
#define PSU_DATA_LEN            1
#define PSU_STATUS_ALL_GOOD     0xbf
#define PSU_STATUS_PRESENT      0x30
#define PSU_ID                  1

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(PSU_ID), "PSU", 0 },
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
    INT4  rv          = ONLP_STATUS_OK;
    UINT4 u4PSUStatus = 0;
    
    VALIDATE(id);
    
    *info = pinfo[ONLP_OID_ID_GET(id)];	
    
    rv =  ifnOS_LINUX_BmcI2CGet(I2C_BUS, CPLD_ADDR, PSU_PRESENT_REGISTER, PSU_ADDR_LEN, &u4PSUStatus, PSU_DATA_LEN);
    
    if(rv == ONLP_STATUS_OK)
    {
        if (u4PSUStatus != PSU_STATUS_PRESENT) 
        {
            info->status &= ~ONLP_PSU_STATUS_PRESENT;
            return ONLP_STATUS_OK;
        }
    }
    else
    {
        AIM_LOG_ERROR("Unable to read PSU present status: %d\r\n", rv);
    }
    
    info->status |= ONLP_PSU_STATUS_PRESENT;
    
    u4PSUStatus = 0;
    
    rv =  ifnOS_LINUX_BmcI2CGet(I2C_BUS, CPLD_ADDR, PSU_STATUS_REGISTER, PSU_ADDR_LEN, &u4PSUStatus, PSU_DATA_LEN);
    
    if(rv == ONLP_STATUS_OK)
    {
        if (u4PSUStatus != PSU_STATUS_ALL_GOOD) 
        {
            info->status |=  ONLP_PSU_STATUS_FAILED;
        }
    }
    else
    {
        AIM_LOG_ERROR("Unable to read PSU failed/good status: %d\r\n", rv);
    }
        
    return rv;
}

int 
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
