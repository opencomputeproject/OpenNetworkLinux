/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 Accton Technology Corporation.
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

#define LED_FORMAT "/sys/devices/platform/as7926_40xfb_led/%s"

/* LED related data
 */
enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_PSU,
    LED_FAN,
    LED_DIAG,
    LED_LOC,
    LED_7SGMT_LEFT,
    LED_7SGMT_RIGHT,
    ONLP_LED_MAX
};

enum led_light_mode {
    LED_MODE_OFF,
    LED_MODE_RED             = 10,
    LED_MODE_RED_BLINKING    = 11,
    LED_MODE_ORANGE          = 12,
    LED_MODE_ORANGE_BLINKING = 13,
    LED_MODE_YELLOW          = 14,
    LED_MODE_YELLOW_BLINKING = 15,
    LED_MODE_GREEN           = 16,
    LED_MODE_GREEN_BLINKING  = 17,
    LED_MODE_BLUE            = 18,
    LED_MODE_BLUE_BLINKING   = 19,
    LED_MODE_PURPLE          = 20,
    LED_MODE_PURPLE_BLINKING = 21,
    LED_MODE_AUTO            = 22,
    LED_MODE_AUTO_BLINKING   = 23,
    LED_MODE_WHITE           = 24,
    LED_MODE_WHITE_BLINKING  = 25,
    LED_MODE_CYAN            = 26,
    LED_MODE_CYAN_BLINKING   = 27,
    LED_MODE_UNKNOWN         = 99
};

typedef struct led_light_mode_map {
    enum onlp_led_id id;
    enum led_light_mode driver_led_mode;
    enum onlp_led_mode_e onlp_led_mode;
} led_light_mode_map_t;

led_light_mode_map_t led_map[] = {
    {LED_PSU,  LED_MODE_AUTO,  ONLP_LED_MODE_AUTO},
    {LED_FAN,  LED_MODE_AUTO,  ONLP_LED_MODE_AUTO},
    {LED_DIAG, LED_MODE_OFF,   ONLP_LED_MODE_OFF},
    {LED_DIAG, LED_MODE_RED,   ONLP_LED_MODE_RED},
    {LED_DIAG, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},
    {LED_DIAG, LED_MODE_BLUE,  ONLP_LED_MODE_BLUE},
    {LED_LOC,  LED_MODE_OFF,   ONLP_LED_MODE_OFF},
    {LED_LOC,  LED_MODE_RED,   ONLP_LED_MODE_RED},
    {LED_LOC,  LED_MODE_GREEN, ONLP_LED_MODE_GREEN},
    {LED_LOC,  LED_MODE_GREEN_BLINKING, ONLP_LED_MODE_GREEN_BLINKING}
};

static char *leds[] = { /* must map with onlp_led_id */
    "reserved",
    "led_psu",
    "led_fan",
    "led_diag",
    "led_loc",
    "led_7sgmt_left",
    "led_7sgmt_right"
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] = {
    { }, /* Not used */
    ONLP_CHASSIS_LED_INFO_ENTRY_INIT(LED_PSU, "Chassis LED 1 (PSU LED)",
        ONLP_LED_CAPS_AUTO),
    ONLP_CHASSIS_LED_INFO_ENTRY_INIT(LED_FAN, "Chassis LED 2 (FAN LED)",
        ONLP_LED_CAPS_AUTO),
    ONLP_CHASSIS_LED_INFO_ENTRY_INIT(LED_DIAG, "Chassis LED 3 (DIAG LED)",
        ONLP_LED_CAPS_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_BLUE |
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING),
    ONLP_CHASSIS_LED_INFO_ENTRY_INIT(LED_LOC, "Chassis LED 4 (LOC LED)",
        ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING),
    ONLP_CHASSIS_LED_INFO_ENTRY_INIT(LED_7SGMT_LEFT, "Chassis LED 5 (LEFT 7-SEGMENT LED)",
        ONLP_LED_CAPS_OFF | ONLP_LED_CAPS_CHAR),
    ONLP_CHASSIS_LED_INFO_ENTRY_INIT(LED_7SGMT_RIGHT, "Chassis LED 6 (RIGHT 7-SEGMENT LED)",
        ONLP_LED_CAPS_OFF | ONLP_LED_CAPS_CHAR)
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

int
onlp_ledi_sw_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_ledi_sw_denit(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_ledi_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    *hdr = linfo[ONLP_OID_ID_GET(oid)].hdr;
    return 0;
}

/*
 * Set the character
 * Set c as  0 to 9  will set the character accordingly.
 * Set c as other values will turn off the 7-segment led
 * */
int onlp_ledi_char_set(onlp_oid_t id, char c)
{
    int lid = ONLP_OID_ID_GET(id);

    if (!(linfo[lid].caps & ONLP_LED_CAPS_CHAR)) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch (c) {
        case '0' ... '9': c -= 48; break; // convert to integer
        default: c = 10;  break; // turn off
    }

    return onlp_file_write_int(c, LED_FORMAT, leds[lid]);
}

/** Get the current character */
int onlp_ledi_char_get(onlp_oid_t id, char* c)
{
    int  lid, value;

    lid = ONLP_OID_ID_GET(id);

    if (!(linfo[lid].caps & ONLP_LED_CAPS_CHAR)) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    /* Get LED character */
    ONLP_TRY(onlp_file_read_int(&value, LED_FORMAT, leds[lid]));

    switch (value) {
        case 0 ... 9: *c = (value + 48); break; // convert to digital number
        default: *c = 0;  break;
    }

    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  lid, value;

    lid = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    /* Get LED mode */
    if (linfo[lid].caps & ONLP_LED_CAPS_CHAR) {
        ONLP_TRY(onlp_ledi_char_get(id, &info->character));
        if (info->character) {
            info->mode = ONLP_LED_MODE_CHAR;
        }
        else {
            info->mode = ONLP_LED_MODE_OFF;
        }
    }
    else {
        ONLP_TRY(onlp_file_read_int(&value, LED_FORMAT, leds[lid]));
        info->mode = driver_to_onlp_led_mode(lid, value);
    }

    return ONLP_STATUS_OK;
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
    int lid = ONLP_OID_ID_GET(id);

    if (linfo[lid].caps & ONLP_LED_CAPS_CHAR) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    ONLP_TRY(onlp_file_write_int(onlp_to_driver_led_mode(lid , mode), LED_FORMAT, leds[lid]));
    return ONLP_STATUS_OK;
}

int
onlp_ledi_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, ONLP_LED_MAX-1);
}
