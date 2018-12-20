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
#include <onlp/platformi/ledi.h>
#include <onlplib/file.h>
#include "x86_64_netberg_aurora_420_rangeley_int.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/* LED related data
 */
enum led_light_mode { /*must be the same with the definition @ kernel driver */
    LED_MODE_OFF = 0,
    LED_MODE_AMBER,
    LED_MODE_GREEN,
};

int led_light_map_mode[][2] =
{
    {LED_MODE_OFF,         ONLP_LED_MODE_OFF},
    {LED_MODE_AMBER,       ONLP_LED_MODE_ORANGE},
    {LED_MODE_GREEN,       ONLP_LED_MODE_GREEN},
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { LED_OID_LED1, "Chassis LED 1 (STAT LED)", 0 },
          ONLP_LED_STATUS_PRESENT,
          ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN,
          ONLP_LED_MODE_OFF,
    },
    {
        { LED_OID_LED2, "Chassis LED 2 (FAN LED)", 0 },
          ONLP_LED_STATUS_PRESENT,
          ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN,
          ONLP_LED_MODE_OFF,
    },
    {
        { LED_OID_LED3, "Chassis LED 3 (PSU1 LED)", 0 },
          ONLP_LED_STATUS_PRESENT,
          ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN,
          ONLP_LED_MODE_OFF,
    },
    {
        { LED_OID_LED4, "Chassis LED 4 (PSU2 LED)", 0 },
          ONLP_LED_STATUS_PRESENT,
          ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN,
          ONLP_LED_MODE_OFF,
    },
};

static int conver_led_light_mode_to_driver(int led_ligth_mode)
{
    int i, nsize = sizeof(led_light_map_mode)/sizeof(led_light_map_mode[0]);
    for(i=0; i<nsize; i++)
    {
        if (led_ligth_mode == led_light_map_mode[i][1])
        {
            return led_light_map_mode[i][0];
        }
    }
    return 0;
}

static int conver_driver_to_led_light_mode(int driver_mode)
{
    int i, nsize = sizeof(led_light_map_mode)/sizeof(led_light_map_mode[0]);
    for(i=0; i<nsize; i++)
    {
        if (driver_mode == led_light_map_mode[i][0])
        {
            return led_light_map_mode[i][1];
        }
    }
    return 0;
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    /*
     * Turn on the STAT LEDs at startup
     */
    onlp_ledi_mode_set(LED_OID_LED1, ONLP_LED_MODE_GREEN);
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    const char *file;
    int  local_id;
    int value = 0;
    int rv;

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    switch(local_id)
    {
        case LED_ID_LED1: /* STAT LED */
            file = "system_led";
            break;
        case LED_ID_LED2: /* FAN LED */
            file = "fan_led";
            break;
        case LED_ID_LED3: /* PSU1 LED */
            file = "psu1_led";
            break;
        case LED_ID_LED4: /* PSU2 LED */
            file = "psu2_led";
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }

    rv = onlp_file_read_int(&value, SYS_HWMON2_PREFIX "/%s", file);
    if (rv != ONLP_STATUS_OK)
        return rv;

    *info = linfo[local_id];
    linfo[local_id].mode = conver_driver_to_led_light_mode(value);

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
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);

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
    int  local_id;

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    switch(local_id)
    {
        case LED_ID_LED1:
          linfo[local_id].status = ONLP_LED_STATUS_PRESENT;
          if (mode != ONLP_LED_MODE_OFF)
              linfo[local_id].status |= ONLP_LED_STATUS_ON;
          linfo[local_id].mode = mode;
          return onlp_file_write_int(conver_led_light_mode_to_driver(mode), SYS_HWMON2_PREFIX "/system_led");
          break;

        default:
          return ONLP_STATUS_E_INVALID;
          break;
    }
    return ONLP_STATUS_E_INVALID;
}

/*
 * Generic LED ioctl interface.
 */
int
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

