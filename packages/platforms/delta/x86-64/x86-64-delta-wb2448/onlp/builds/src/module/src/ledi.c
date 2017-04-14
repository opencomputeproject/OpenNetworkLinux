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
        if(!ONLP_OID_IS_LED(_id))               \
        {                                       \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define I2C_BUS             0x04
#define CPLD_ADDR           0x28
#define SYS_LED_REGISTER    0x09
#define TEMP_LED_REGISTER   0x09
#define FAN_LED_REGISTER    0x09
#define LED_ADDR_LEN        0x01
#define LED_DATA_LEN        0x01

/* LED related data
 */
enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_SYSTEM,
    LED_FAN,
    LED_TEMP
};

enum led_light_mode 
{
	LED_MODE_OFF = 0,
	LED_MODE_GREEN,
	LED_MODE_RED,
	LED_MODE_AMBER,
	LED_MODE_BLUE,
	LED_MODE_GREEN_BLINK =0 ,
	LED_MODE_AMBER_BLINK,
	LED_MODE_RED_BLINK = 3,
	LED_MODE_BLUE_BLINK,
	LED_MODE_AUTO,
	LED_MODE_UNKNOWN
};

typedef struct led_light_mode_map 
{
    enum onlp_led_id id;
    enum led_light_mode driver_led_mode;
    enum onlp_led_mode_e onlp_led_mode;
} led_light_mode_map_t;

led_light_mode_map_t led_map[] = 
{
    {LED_SYSTEM, LED_MODE_GREEN_BLINK, ONLP_LED_MODE_GREEN_BLINKING},
    {LED_SYSTEM, LED_MODE_GREEN,       ONLP_LED_MODE_GREEN},
    {LED_SYSTEM, LED_MODE_RED,         ONLP_LED_MODE_RED},
    {LED_SYSTEM, LED_MODE_RED_BLINK,   ONLP_LED_MODE_RED_BLINKING},
    {LED_FAN,  LED_MODE_GREEN, ONLP_LED_MODE_GREEN},
    {LED_FAN,  LED_MODE_RED,   ONLP_LED_MODE_RED},
    {LED_TEMP, LED_MODE_OFF,   ONLP_LED_MODE_OFF},
    {LED_TEMP, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},
    {LED_TEMP, LED_MODE_RED,   ONLP_LED_MODE_RED}
};

/*
 * Get the information for the given LED OID.
 */
 
static onlp_led_info_t linfo[] =
{
    { },
    {
        { ONLP_LED_ID_CREATE(LED_SYSTEM), "Chassis LED 1 (SYSTEM)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_RED_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "Chassis LED 2 (FAN)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_TEMP), "Chassis LED 3 (TEMP)", 0 },
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

int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}

int 
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    INT4  rv		= ONLP_STATUS_OK;
    INT4  local_id  = 0;
    UINT4 u4LedMode = 0;
            
    VALIDATE(id);
    
    local_id = ONLP_OID_ID_GET(id);
    
    *info = linfo[ONLP_OID_ID_GET(id)];	
    
    /* Get LED mode */
    switch(local_id)
    {
        case LED_SYSTEM:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BUS, CPLD_ADDR, SYS_LED_REGISTER, LED_ADDR_LEN, &u4LedMode, LED_DATA_LEN);
            u4LedMode = u4LedMode >> 6;
            break;
            
        case LED_FAN:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BUS, CPLD_ADDR, TEMP_LED_REGISTER, LED_ADDR_LEN, &u4LedMode, LED_DATA_LEN);
            u4LedMode = (u4LedMode >> 2) & 0x03;
            break;
            
        case LED_TEMP:
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BUS, CPLD_ADDR, FAN_LED_REGISTER, LED_ADDR_LEN, &u4LedMode, LED_DATA_LEN);
            u4LedMode = (u4LedMode >> 4) & 0x03;
            break;
        
        default:
            AIM_LOG_ERROR("Invalid LED ID!!\n");
            rv = ONLP_STATUS_E_PARAM;
    }
    
    if( rv == ONLP_STATUS_OK)
    {
        info->mode = driver_to_onlp_led_mode(local_id, u4LedMode);
    
        /* Set the on/off status */
        if (info->mode != ONLP_LED_MODE_OFF) 
        {
            info->status |= ONLP_LED_STATUS_ON;
        }
    }
    
    return rv;
}

/*
 * @brief Turn an LED on or off.
 * @param id The LED OID
 * @param on_or_off Led on (1) or LED off (0)
 * @param Relevant if the LED has the ON_OFF capability.
 * @note For the purposes of this function the
 * interpretation of "on" for multi-mode or multi-color LEDs
 * is up to the platform implementation.
 */
int
onlp_ledi_set(onlp_oid_t id, int on_or_off)
{
    VALIDATE(id);
        
    if (!on_or_off)
    {
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    }
    
    return ONLP_STATUS_E_UNSUPPORTED;
}

int 
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function puts the LED into the given mode. It is a more functional
 * interface for multimode LEDs.
 *
 * Only modes reported in the LED's capabilities will be attempted.
 */
int
onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    INT4  rv        = ONLP_STATUS_OK;
    INT4  local_id  = 0;
    UINT4 u4Addr    = 0;
    UINT4 u4NewLedMode = 0;
    UINT4 u4OldLedMode = 0;
    onlp_led_mode_t driver_led_mode = 0;
    
    VALIDATE(id);
    
    local_id = ONLP_OID_ID_GET(id);
    driver_led_mode = onlp_to_driver_led_mode(local_id, mode);
    
    switch(local_id)
    {
        case LED_SYSTEM:
            u4Addr = SYS_LED_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BUS, CPLD_ADDR, u4Addr, LED_ADDR_LEN, &u4OldLedMode, LED_DATA_LEN);
            u4NewLedMode = ( (u4OldLedMode & 0x3C) | ((driver_led_mode << 6) & 0xC0) );
            break;
            
        case LED_FAN:
            u4Addr = FAN_LED_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BUS, CPLD_ADDR, u4Addr, LED_ADDR_LEN, &u4OldLedMode, LED_DATA_LEN);
            u4NewLedMode = ( (u4OldLedMode & 0xF0) | ((driver_led_mode << 2) & 0x0C) );
            break;
            
        case LED_TEMP:
            u4Addr = TEMP_LED_REGISTER;
            rv = ifnOS_LINUX_BmcI2CGet(I2C_BUS, CPLD_ADDR, u4Addr, LED_ADDR_LEN, &u4OldLedMode, LED_DATA_LEN);
            u4NewLedMode = ( (u4OldLedMode & 0xCC) | ((driver_led_mode << 4) & 0x30) );
            break;
            
        default:
            rv = ONLP_STATUS_E_PARAM;
            AIM_LOG_ERROR("Invalid LED ID!!\n");
    }
    
    if(rv == ONLP_STATUS_OK)
    {
        rv = ifnOS_LINUX_BmcI2CSet(I2C_BUS, CPLD_ADDR, u4Addr, LED_ADDR_LEN, u4NewLedMode, LED_DATA_LEN);
    }
    
    return rv;
}

