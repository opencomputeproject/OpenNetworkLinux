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
    {LED_FAN,  LED_MODE_OFF,         ONLP_LED_MODE_OFF},
    {LED_FAN,  LED_MODE_GREEN,       ONLP_LED_MODE_GREEN},
    {LED_FAN,  LED_MODE_RED,         ONLP_LED_MODE_RED},
    {LED_SYS,  LED_MODE_OFF,         ONLP_LED_MODE_OFF},
    {LED_SYS,  LED_MODE_GREEN,       ONLP_LED_MODE_GREEN},
    {LED_SYS,  LED_MODE_GREEN_BLINK, ONLP_LED_MODE_GREEN_BLINKING},
    {LED_SYS,  LED_MODE_SYS_RED,     ONLP_LED_MODE_RED},
    {LED_PSU2, LED_MODE_OFF,         ONLP_LED_MODE_OFF},
    {LED_PSU2, LED_MODE_GREEN,       ONLP_LED_MODE_GREEN},
    {LED_PSU2, LED_MODE_RED,         ONLP_LED_MODE_RED},
    {LED_PSU1, LED_MODE_OFF,         ONLP_LED_MODE_OFF},
    {LED_PSU1, LED_MODE_GREEN,       ONLP_LED_MODE_GREEN},
    {LED_PSU1, LED_MODE_RED,         ONLP_LED_MODE_RED},
    {LED_FAN_TRAY_1, LED_MODE_FAN_TRAY_AMBER, ONLP_LED_MODE_ORANGE},
    {LED_FAN_TRAY_1, LED_MODE_FAN_TRAY_GREEN, ONLP_LED_MODE_GREEN},
    {LED_FAN_TRAY_2, LED_MODE_FAN_TRAY_AMBER, ONLP_LED_MODE_ORANGE},
    {LED_FAN_TRAY_2, LED_MODE_FAN_TRAY_GREEN, ONLP_LED_MODE_GREEN},
    {LED_FAN_TRAY_3, LED_MODE_FAN_TRAY_AMBER, ONLP_LED_MODE_ORANGE},
    {LED_FAN_TRAY_3, LED_MODE_FAN_TRAY_GREEN, ONLP_LED_MODE_GREEN},
    {LED_FAN_TRAY_4, LED_MODE_FAN_TRAY_AMBER, ONLP_LED_MODE_ORANGE},
    {LED_FAN_TRAY_4, LED_MODE_FAN_TRAY_GREEN, ONLP_LED_MODE_GREEN},
};

static onlp_led_info_t linfo[] =
{
    { },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "FAN LED (FRONT PANEL)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_SYS), "SYSTEM LED (FRONT PANEL)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU1), "PSU1 LED (FRONT PANEL)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU2), "PSU2 LED (FRONT PANEL)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY_1), "FAN TRAY 1", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY_2), "FAN TRAY 2", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY_3), "FAN TRAY 3", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY_4), "FAN TRAY 4", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
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
        case LED_FAN:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LED_FAN_SYS_FRONT_REGISTER, &LedMode, 1);
            LedMode = ((LedMode >> LED_FAN_FRONT_BIT) & 0x03);                                                           
            break;

        case LED_SYS:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LED_FAN_SYS_FRONT_REGISTER, &LedMode, 1);
            LedMode = ((LedMode >> LED_SYS_FRONT_BIT) & 0x03); 
            break;
                                                                                                            
        case LED_PSU1:                                                                                       
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_1_ADDR, PSU_STATUS_REGISTER, &LedMode, 1);
            
            if ( ((LedMode >> PSU1_INT_BIT) & 0x01) == PSU_INT_HAPPEN_STATUS )
            {
                LedMode = LED_MODE_RED;
            }
            else if( ((LedMode >> PSU1_POWER_GOOD_BIT) & 0x01) == PSU_POWER_GOOD_STATUS )
            {
                LedMode = LED_MODE_GREEN;
            }
            else
            {
                LedMode = LED_MODE_OFF;
            } 
            break;
            
        case LED_PSU2:                                                                                    
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_1_ADDR, PSU_STATUS_REGISTER, &LedMode, 1);
            
            if ( ((LedMode >> PSU2_INT_BIT) & 0x01) == PSU_INT_HAPPEN_STATUS )
            {
                LedMode = LED_MODE_RED;
            }
            else if( ((LedMode >> PSU2_POWER_GOOD_BIT) & 0x01) == PSU_POWER_GOOD_STATUS )
            {
                LedMode = LED_MODE_GREEN;
            }
            else
            {
                LedMode = LED_MODE_OFF;
            }                                                        
            break;                                                                                          
                                                                                                            
        case LED_FAN_TRAY_1:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LED_FAN_TRAY_REGISTER, &LedMode, 1);
            LedMode = (LedMode >> LED_FAN_TRAY_1_BIT ) & 0x01;
            break;

        case LED_FAN_TRAY_2:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LED_FAN_TRAY_REGISTER, &LedMode, 1);
            LedMode = (LedMode >> LED_FAN_TRAY_2_BIT) & 0x01;
            break;              
        
        case LED_FAN_TRAY_3:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LED_FAN_TRAY_REGISTER, &LedMode, 1);
            LedMode = (LedMode >> LED_FAN_TRAY_3_BIT) & 0x01;
            break; 
            
        case LED_FAN_TRAY_4:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LED_FAN_TRAY_REGISTER, &LedMode, 1);
            LedMode = (LedMode >> LED_FAN_TRAY_4_BIT) & 0x01;
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
    uint32_t LEDAddr    = 0;
    uint32_t NewLedMode = 0;
    uint32_t OldLedMode = 0;
    onlp_led_mode_t driver_led_mode = 0;
    
    VALIDATE(id);
    
    local_id = ONLP_OID_ID_GET(id);
    driver_led_mode = onlp_to_driver_led_mode(local_id, mode);
    
    switch(local_id)
    {
        case LED_FAN:
            LEDAddr = LED_FAN_SYS_FRONT_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LEDAddr, &OldLedMode, 1);
            NewLedMode = ( (OldLedMode & 0x3F) | ((driver_led_mode << 6) & 0xC0) );
            break;
        
        case LED_SYS:
            LEDAddr = LED_FAN_SYS_FRONT_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LEDAddr, &OldLedMode, 1);
            NewLedMode = ( (OldLedMode & 0xCF) | ((driver_led_mode << 4) & 0x30) );
            break;
            
        case LED_PSU1:
        case LED_PSU2:
            AIM_LOG_ERROR("PSU LED (FRONT) is read only!!\n");
            rv = ONLP_STATUS_E_UNSUPPORTED;
            break;
            
        case LED_FAN_TRAY_1:
            LEDAddr = LED_FAN_TRAY_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LEDAddr, &OldLedMode, 1);
            NewLedMode = ( (OldLedMode & 0x7F) | ((driver_led_mode << 7) & 0x01) );
            break;
        
        case LED_FAN_TRAY_2:
            LEDAddr = LED_FAN_TRAY_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LEDAddr, &OldLedMode, 1);
            NewLedMode = ( (OldLedMode & 0xBF) | ((driver_led_mode << 6) & 0x01) );
            break;
            
        case LED_FAN_TRAY_3:
            LEDAddr = LED_FAN_TRAY_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LEDAddr, &OldLedMode, 1);
            NewLedMode = ( (OldLedMode & 0xDF) | ((driver_led_mode << 5) & 0x01) );
            break;
            
        case LED_FAN_TRAY_4:
            LEDAddr = LED_FAN_TRAY_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LEDAddr, &OldLedMode, 1);
            NewLedMode = ( (OldLedMode & 0xEF) | ((driver_led_mode << 4) & 0x01) );
            break;
            
        default:
            rv = ONLP_STATUS_E_PARAM;
            AIM_LOG_ERROR("Invalid LED ID!!\n");
    }
    
    if(rv == ONLP_STATUS_OK)
    {
        rv = ifnOS_LINUX_BmcI2CSet(I2C_BMC_BUS_5, SWPLD_2_ADDR, LEDAddr, NewLedMode, 1);
    }
    
    return rv;
}

