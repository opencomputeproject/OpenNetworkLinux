/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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
#include <unistd.h>

#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include <onlp/platformi/ledi.h>

#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do                                          \
    {                                           \
        if(!ONLP_OID_IS_LED(_id))               \
        {                                       \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/* LED related data */
enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_SYS,
    LED_PSU,
    LED_FAN,
    LED_BMC
};
#define LED_PATH    "/sys/switch/sysled/"
typedef struct led_syspath_s {
    enum onlp_led_id id;
    char* path;
} led_syspath_t;

typedef struct led_address_s {
    enum onlp_led_id id;
    uint8_t bus;
    uint8_t devaddr;
    uint8_t offset;
} led_address_t;

typedef struct led_mode_info_s {
    onlp_led_mode_t  mode;
    uint8_t regval;
} led_mode_info_t;

static led_syspath_t led_syspath[] =
{
    { }, /* Not used */
    {LED_SYS, LED_PATH "sys_led_status"},
    {LED_PSU, LED_PATH "psu_led_status"},
    {LED_FAN, LED_PATH "fan_led_status"},
    {LED_BMC, LED_PATH "bmc_led_status"},
};
static led_mode_info_t led_mode_info[] =
{
    {ONLP_LED_MODE_OFF      , 0},
    {ONLP_LED_MODE_GREEN    , 1},
    {ONLP_LED_MODE_YELLOW   , 2},
    {ONLP_LED_MODE_RED      , 3},
    {ONLP_LED_MODE_BLUE     , 4},
    {ONLP_LED_MODE_GREEN_BLINKING   , 5},
    {ONLP_LED_MODE_YELLOW_BLINKING  , 6},
    {ONLP_LED_MODE_RED_BLINKING     , 7},
    {ONLP_LED_MODE_BLUE_BLINKING    , 8},
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_SYS), "Chassis LED 1 (SYS)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_AUTO
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU), "Chassis LED 2 (PSU)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_AUTO
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "Chassis LED 3 (FAN)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_AUTO
    },
    {
        { ONLP_LED_ID_CREATE(LED_BMC), "Chassis LED 4 (BMC)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_AUTO
    },        
};


/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}

static int
reg_value_to_onlp_led_mode(enum onlp_led_id id, int value)
{
    int i;

    for (i = 0; i < AIM_ARRAYSIZE(led_mode_info); i++)
    {
        if (value != led_mode_info[i].regval)
        {
            continue;
        }

        return led_mode_info[i].mode;
    }

    return ONLP_LED_MODE_AUTO;
}

static int
onlp_led_mode_to_reg_value(enum onlp_led_id id, onlp_led_mode_t onlp_led_mode)
{
    int i;

    for (i = 0; i < AIM_ARRAYSIZE(led_mode_info); i++)
    {
        if (onlp_led_mode != led_mode_info[i].mode)
        {
            continue;
        }

        return led_mode_info[i].regval;
    }

    return 0;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  lid, value;

    VALIDATE(id);
    lid = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];
    if (0 > onlp_file_read_int(&value, led_syspath[lid].path ))
        return ONLP_STATUS_E_INTERNAL;
    
    info->mode = reg_value_to_onlp_led_mode(lid, value);
    info->mode = ONLP_LED_MODE_AUTO;
    /* Set the on/off status */
    if (info->mode != ONLP_LED_MODE_OFF)
    {
        info->status |= ONLP_LED_STATUS_ON;
    }

    return ONLP_STATUS_OK;
}

/*
 * Turn an LED on or off.
 *
 * This function will only be called if the LED OID supports the ONOFF
 * capability.
 *
 * What 'on' means in terms of colors or modes for multimode LEDs is
 * up to the platform to decide. This is intended as baseline toggle mechanism.
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

/*
 * This function puts the LED into the given mode. It is a more functional
 * interface for multimode LEDs.
 *
 * Only modes reported in the LED's capabilities will be attempted.
 */
int
onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    int value,lid;
    VALIDATE(id);
    lid = ONLP_OID_ID_GET(id);

    value = onlp_led_mode_to_reg_value(lid, mode);
    if (0 > onlp_file_write_int(value, led_syspath[lid].path ))
        return ONLP_STATUS_E_INTERNAL;

    return ONLP_STATUS_OK;
}

/*
 * Generic LED ioctl interface.
 */
int
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
