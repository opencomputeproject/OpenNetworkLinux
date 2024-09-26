/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
#include <stdio.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "platform_lib.h"

/*
 * Get the information for the given LED OID.
 */

static onlp_led_info_t led_info[] =
{
    { }, // Not used *
    {
        { LED_OID_SYSTEM, "Chassis LED (SYS LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_YELLOW_BLINKING |
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING ,
    },
    {
        { LED_OID_SYNC, "Chassis LED (SYNC LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_YELLOW_BLINKING |
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING ,
    },
    {
        { LED_OID_GPS, "Chassis LED (GPS LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_YELLOW_BLINKING |
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING ,
    },
};

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    lock_init();
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int led_id, rc=ONLP_STATUS_OK;

    led_id = ONLP_OID_ID_GET(id);
    *info = led_info[led_id];

    switch (led_id) {
        case LED_ID_SYS_SYS:
        case LED_ID_SYS_SYNC:
        case LED_ID_SYS_GPS:
            rc = sys_led_info_get(info, led_id);
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
            break;
    }

    return rc;
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
    int led_id, rc=ONLP_STATUS_OK;

    led_id = ONLP_OID_ID_GET(id);

    switch (led_id) {
        case LED_ID_SYS_SYS:
        case LED_ID_SYS_SYNC:
        case LED_ID_SYS_GPS:
            rc = sys_led_set(led_id, on_or_off);
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
            break;
    }

    return rc;
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
    int led_id, rc=ONLP_STATUS_OK;
    int color, blink;

    led_id = ONLP_OID_ID_GET(id);

    switch (mode) {
        case ONLP_LED_MODE_GREEN:
            color = LED_COLOR_GREEN;
            blink = LED_STABLE;
            break;
        case ONLP_LED_MODE_GREEN_BLINKING:
            color = LED_COLOR_GREEN;
            blink = LED_BLINKING;
            break;
        case ONLP_LED_MODE_YELLOW:
            color = LED_COLOR_YELLOW;
            blink = LED_STABLE;
            break;
        case ONLP_LED_MODE_YELLOW_BLINKING:
            color = LED_COLOR_YELLOW;
            blink = LED_BLINKING;
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
            break;
    }

    switch (led_id) {
        case LED_ID_SYS_SYS:
        case LED_ID_SYS_SYNC:
        case LED_ID_SYS_GPS:
            rc = sys_led_mode_set(led_id, color, blink);
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
            break;
    }

    return rc;
}

int
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_OK;
}
