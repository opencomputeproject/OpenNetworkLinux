/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <onlplib/file.h>
#include <onlp/platformi/ledi.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define LED_FORMAT "/sys/class/leds/as4630_54te::%s/brightness"

/* LED related data
 */
enum onlp_led_id {
    LED_RESERVED = 0,
    LED_DIAG,
    LED_PRI,
    LED_POE,
    LED_STK1,
    LED_STK2,
    LED_FAN,
    LED_PSU1,
    LED_PSU2
};

enum led_light_mode {
    LED_MODE_OFF = 0,
    LED_MODE_GREEN,
    LED_MODE_AMBER,
    LED_MODE_RED,
    LED_MODE_BLUE,
    LED_MODE_GREEN_BLINK,
    LED_MODE_AMBER_BLINK,
    LED_MODE_RED_BLINK,
    LED_MODE_BLUE_BLINK,
    LED_MODE_AUTO,
    LED_MODE_UNKNOWN
};

typedef struct led_light_mode_map {
    enum onlp_led_id id;
    enum led_light_mode driver_led_mode;
    enum onlp_led_mode_e onlp_led_mode;
} led_light_mode_map_t;

led_light_mode_map_t led_map[] = {
    {LED_DIAG, LED_MODE_OFF, ONLP_LED_MODE_OFF},
    {LED_DIAG, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},
    {LED_DIAG, LED_MODE_AMBER, ONLP_LED_MODE_ORANGE},
    {LED_DIAG, LED_MODE_GREEN_BLINK, ONLP_LED_MODE_GREEN_BLINKING},

    {LED_PRI, LED_MODE_OFF, ONLP_LED_MODE_OFF},
    {LED_PRI, LED_MODE_AMBER, ONLP_LED_MODE_ORANGE},
    {LED_PRI, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},

    {LED_POE, LED_MODE_OFF, ONLP_LED_MODE_OFF},
    {LED_POE, LED_MODE_AMBER, ONLP_LED_MODE_ORANGE},
    {LED_POE, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},

    {LED_STK1, LED_MODE_OFF, ONLP_LED_MODE_OFF},
    {LED_STK1, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},

    {LED_STK2, LED_MODE_OFF, ONLP_LED_MODE_OFF},
    {LED_STK2, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},

    {LED_PSU1, LED_MODE_AUTO, ONLP_LED_MODE_AUTO},
    {LED_PSU2, LED_MODE_AUTO, ONLP_LED_MODE_AUTO},
    {LED_FAN, LED_MODE_AUTO, ONLP_LED_MODE_AUTO},
};

static char *leds[] = { /* must map with onlp_led_id */
    NULL,
    "diag",
    "pri",
    "poe",
    "stk1",
    "stk2",
    "fan",
    "psu1",
    "psu2"
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_DIAG), "LED 1 (DIAG LED)", 0, {0} },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF |ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PRI), "LED 2 (PRI LED)", 0, {0} },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PRI), "LED 3 (POE LED)", 0, {0} },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_STK1), "LED 4 (STK1 LED)", 0, {0} },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_STK2), "LED 5 (STK2 LED)", 0, {0} },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "LED 6 (FAN LED)", 0, {0} },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU1), "LED 7 (PSU1 LED)", 0, {0} },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU2), "LED 8 (PSU2 LED)", 0, {0} },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_AUTO,
    },
};

static int driver_to_onlp_led_mode(enum onlp_led_id id, enum led_light_mode driver_led_mode)
{
    int i, nsize = sizeof(led_map)/sizeof(led_map[0]);

    for (i = 0; i < nsize; i++) {
        if (id == led_map[i].id && driver_led_mode == led_map[i].driver_led_mode) {
            return led_map[i].onlp_led_mode;
        }
    }

    return 0;
}

static int onlp_to_driver_led_mode(enum onlp_led_id id, onlp_led_mode_t onlp_led_mode)
{
    int i, nsize = sizeof(led_map)/sizeof(led_map[0]);

    for(i = 0; i < nsize; i++) {
        if (id == led_map[i].id && onlp_led_mode == led_map[i].onlp_led_mode) {
            return led_map[i].driver_led_mode;
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
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_DIAG), ONLP_LED_MODE_GREEN);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PRI), ONLP_LED_MODE_GREEN);

    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  lid, value;
    VALIDATE(id);

    lid = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    /* Get LED mode */
    if (onlp_file_read_int(&value, LED_FORMAT, leds[lid]) < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    info->mode = driver_to_onlp_led_mode(lid, value);

    /* Set the on/off status */
    if (info->mode != ONLP_LED_MODE_OFF) {
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

    if (!on_or_off) {
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    }
    else
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_GREEN);


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
    int  lid;
    VALIDATE(id);

    lid = ONLP_OID_ID_GET(id);

    if (onlp_file_write_int(onlp_to_driver_led_mode(lid , mode), LED_FORMAT, leds[lid]) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
