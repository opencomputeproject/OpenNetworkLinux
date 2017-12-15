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
 ***********************************************************/
#include <onlp/platformi/ledi.h>
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
        if(!ONLP_OID_IS_LED(_id))               \
        {                                       \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/*
 * Get the information for the given LED OID.
 */
 typedef struct led_light_mode_map 
{
    enum onlp_led_id id;
    enum led_light_mode driver_led_mode;
    enum onlp_led_mode_e onlp_led_mode;
} led_light_mode_map_t;

led_light_mode_map_t led_map[] = 
{
    {LED_PSU, LED_MODE_OFF,          ONLP_LED_MODE_OFF},
    {LED_PSU, LED_MODE_GREEN,        ONLP_LED_MODE_GREEN},
    {LED_PSU, LED_MODE_YELLOW,       ONLP_LED_MODE_YELLOW},
    {LED_PSU, LED_MODE_YELLOW_BLINK, ONLP_LED_MODE_YELLOW_BLINKING},
    {LED_SYS, LED_MODE_OFF,          ONLP_LED_MODE_OFF},
    {LED_SYS, LED_MODE_GREEN,        ONLP_LED_MODE_GREEN},
    {LED_SYS, LED_MODE_GREEN_BLINK,  ONLP_LED_MODE_GREEN_BLINKING},
    {LED_SYS, LED_MODE_RED,          ONLP_LED_MODE_RED},
    {LED_FAN, LED_MODE_OFF,          ONLP_LED_MODE_OFF},
    {LED_FAN, LED_MODE_GREEN,        ONLP_LED_MODE_GREEN},
    {LED_FAN, LED_MODE_AMBER,        ONLP_LED_MODE_ORANGE},
    {LED_FAN_TRAY_1, LED_MODE_OFF,   ONLP_LED_MODE_OFF},
    {LED_FAN_TRAY_1, LED_MODE_FAN_TRAY_GREEN, ONLP_LED_MODE_GREEN},
    {LED_FAN_TRAY_1, LED_MODE_FAN_TRAY_RED,   ONLP_LED_MODE_RED},
    {LED_FAN_TRAY_2, LED_MODE_OFF,            ONLP_LED_MODE_OFF},
    {LED_FAN_TRAY_2, LED_MODE_FAN_TRAY_GREEN, ONLP_LED_MODE_GREEN},
    {LED_FAN_TRAY_2, LED_MODE_FAN_TRAY_RED,   ONLP_LED_MODE_RED},
    {LED_FAN_TRAY_3, LED_MODE_OFF,            ONLP_LED_MODE_OFF},
    {LED_FAN_TRAY_3, LED_MODE_FAN_TRAY_GREEN, ONLP_LED_MODE_GREEN},
    {LED_FAN_TRAY_3, LED_MODE_FAN_TRAY_RED,   ONLP_LED_MODE_RED},
    {LED_FAN_TRAY_4, LED_MODE_OFF,            ONLP_LED_MODE_OFF},
    {LED_FAN_TRAY_4, LED_MODE_FAN_TRAY_GREEN, ONLP_LED_MODE_GREEN},
    {LED_FAN_TRAY_4, LED_MODE_FAN_TRAY_RED,   ONLP_LED_MODE_RED},
    {LED_FAN_TRAY_5, LED_MODE_OFF,            ONLP_LED_MODE_OFF},
    {LED_FAN_TRAY_5, LED_MODE_FAN_TRAY_GREEN, ONLP_LED_MODE_GREEN},
    {LED_FAN_TRAY_5, LED_MODE_FAN_TRAY_RED,   ONLP_LED_MODE_RED}
};

static onlp_led_info_t linfo[] =
{
    { },
    {
        { ONLP_LED_ID_CREATE(LED_PSU), "PSU LED (FRONT)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_YELLOW_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_SYS), "SYSTEM LED (FRONT)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "FAN LED (FRONT)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY_1), "FAN TRAY 1", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY_2), "FAN TRAY 2", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY_3), "FAN TRAY 3", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY_4), "FAN TRAY 4", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY_5), "FAN TRAY 5", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
};

static int driver_to_onlp_led_mode(enum onlp_led_id id, enum led_light_mode driver_led_mode)
{
    int i, nsize = sizeof(led_map)/sizeof(led_map[0]);
    
    for (i = 0; i < nsize; i++)
    {
        if (id == led_map[i].id && driver_led_mode == led_map[i].driver_led_mode)
        {
            return led_map[i].onlp_led_mode;
        }
    }
    
    return 0;
}

static int onlp_to_driver_led_mode(enum onlp_led_id id, onlp_led_mode_t onlp_led_mode)
{
    int i, nsize = sizeof(led_map)/sizeof(led_map[0]);
    
    for(i = 0; i < nsize; i++)
    {
        if (id == led_map[i].id && onlp_led_mode == led_map[i].onlp_led_mode)
        {
            return led_map[i].driver_led_mode;
        }
    }
    
    return 0;
}

int onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}

int onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  rv          = ONLP_STATUS_OK;
    int  local_id    = 0;
    uint32_t LedMode = 0;
            
    VALIDATE(id);
    
    local_id = ONLP_OID_ID_GET(id);
    *info = linfo[local_id];

    /* Get LED mode */
    switch(local_id)
    {
        case LED_PSU:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, LED_PSU_REGISTER, &LedMode, DATA_LEN);
            LedMode = (LedMode >> 6 ) & 0x03;                                                           
            break;                                                                                          
                                                                                                            
        case LED_SYS:                                                                                    
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, LED_SYS_REGISTER, &LedMode, DATA_LEN);
            LedMode = (LedMode >> 4) & 0x03;                                                            
            break;                                                                                          
                                                                                                            
        case LED_FAN:                                                                                       
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, LED_FAN_REGISTER, &LedMode, DATA_LEN);
            LedMode = (LedMode >> 2) & 0x03;
            break;
        
        case LED_FAN_TRAY_1:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, LED_FAN_TRAY_1_REGISTER, &LedMode, DATA_LEN);
            LedMode = (LedMode >> 6 ) & 0x03;                                                           
            break;

        case LED_FAN_TRAY_2:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, LED_FAN_TRAY_2_REGISTER, &LedMode, DATA_LEN);
            LedMode = (LedMode >> 4) & 0x03;                                                          
            break;              
        
        case LED_FAN_TRAY_3:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, LED_FAN_TRAY_3_REGISTER, &LedMode, DATA_LEN);
            LedMode = (LedMode >> 2) & 0x03;                                                          
            break; 
            
        case LED_FAN_TRAY_4:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, LED_FAN_TRAY_4_REGISTER, &LedMode, DATA_LEN);
            LedMode = LedMode & 0x03;                                                          
            break; 
            
        case LED_FAN_TRAY_5:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, LED_FAN_TRAY_5_REGISTER, &LedMode, DATA_LEN);
            LedMode = (LedMode >> 6) & 0x03;                                                          
            break;
            
        default:
            AIM_LOG_ERROR("Invalid LED ID!!\n");
            rv = ONLP_STATUS_E_PARAM;
    }
    
    if( rv == ONLP_STATUS_OK)
    {
        info->mode = driver_to_onlp_led_mode(local_id, LedMode);
    
        /* Set the on/off status */
        if (info->mode != ONLP_LED_MODE_OFF) 
        {
            info->status |= ONLP_LED_STATUS_ON;
        }
    }
    
    return rv;
}

int onlp_ledi_set(onlp_oid_t id, int on_or_off)
{
    VALIDATE(id);
        
    if (!on_or_off)
    {
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    }
    
    return ONLP_STATUS_E_UNSUPPORTED;
}

int onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    int  rv             = ONLP_STATUS_OK;
    int  local_id       = 0;
    uint32_t Addr       = 0;
    uint32_t NewLedMode = 0;
    uint32_t OldLedMode = 0;
    onlp_led_mode_t driver_led_mode = 0;
    
    VALIDATE(id);
    
    local_id = ONLP_OID_ID_GET(id);
    driver_led_mode = onlp_to_driver_led_mode(local_id, mode);
    
    switch(local_id)
    {
        case LED_PSU:
            AIM_LOG_ERROR("PSU LED (FRONT) is read only!!\n");
            rv = ONLP_STATUS_E_UNSUPPORTED;
            break;
        
        case LED_SYS:
            Addr = LED_SYS_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, Addr, &OldLedMode, DATA_LEN);
            NewLedMode = ( (OldLedMode & 0xCF) | ((driver_led_mode << 4) & 0x30) );
            break;
            
        case LED_FAN:
            Addr = LED_FAN_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, Addr, &OldLedMode, DATA_LEN);
            NewLedMode = ( (OldLedMode & 0xF3) | ((driver_led_mode << 2) & 0x0C) );
            break;
            
        case LED_FAN_TRAY_1:
            Addr = LED_FAN_TRAY_1_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, Addr, &OldLedMode, DATA_LEN);
            NewLedMode = ( (OldLedMode & 0x3F) | ((driver_led_mode << 6) & 0xC0) );
            break;
        
        case LED_FAN_TRAY_2:
            Addr = LED_FAN_TRAY_2_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, Addr, &OldLedMode, DATA_LEN);
            NewLedMode = ( (OldLedMode & 0xCF) | ((driver_led_mode << 4) & 0x30) );
            break;
            
        case LED_FAN_TRAY_3:
            Addr = LED_FAN_TRAY_3_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, Addr, &OldLedMode, DATA_LEN);
            NewLedMode = ( (OldLedMode & 0xF3) | ((driver_led_mode << 2) & 0x0C) );
            break;
            
        case LED_FAN_TRAY_4:
            Addr = LED_FAN_TRAY_4_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, Addr, &OldLedMode, DATA_LEN);
            NewLedMode = ( (OldLedMode & 0xFC) | (driver_led_mode & 0x03) );
            break;
        
        case LED_FAN_TRAY_5:
            Addr = LED_FAN_TRAY_5_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, SWPLD_1_ADDR, Addr, &OldLedMode, DATA_LEN);
            NewLedMode = ( (OldLedMode & 0x3F) | ((driver_led_mode << 6) & 0xC0) );
            break;
            
        default:
            rv = ONLP_STATUS_E_PARAM;
            AIM_LOG_ERROR("Invalid LED ID!!\n");
    }
    
    if(rv == ONLP_STATUS_OK)
    {
        rv = ifnOS_LINUX_BmcI2CSet(I2C_BMC_BUS_3, SWPLD_1_ADDR, Addr, NewLedMode, DATA_LEN);
    }
    
    return rv;
}

