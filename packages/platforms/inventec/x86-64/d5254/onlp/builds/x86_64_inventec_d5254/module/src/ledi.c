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
#include <onlp/platformi/ledi.h>
#include <onlplib/file.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "platform_lib.h"

/* LED related data
 */

/* CAPS*/
#define CPLD_LED_GREEN_CAPS ONLP_LED_CAPS_OFF|ONLP_LED_CAPS_GREEN|ONLP_LED_CAPS_GREEN_BLINKING
#define CPLD_LED_RED_CAPS ONLP_LED_CAPS_OFF|ONLP_LED_CAPS_RED|ONLP_LED_CAPS_RED_BLINKING
#define PSOC_LED_GREEN_CAPS ONLP_LED_CAPS_OFF|ONLP_LED_CAPS_GREEN
#define PSOC_LED_RED_CAPS ONLP_LED_CAPS_OFF|ONLP_LED_CAPS_RED


typedef enum platform_led_color_e {
    PLATFORM_LED_COLOR_NONE,
    PLATFORM_LED_COLOR_RED,
    PLATFORM_LED_COLOR_GREEN,
    PLATFORM_LED_COLOR_ANY,
    PLATFORM_LED_COLOR_MAX
} platform_led_color_t;

typedef enum cpld_led_mode_e {
    CPLD_LED_MODE_OFF = 0,
    CPLD_LED_MODE_0_5_HZ = 1,
    CPLD_LED_MODE_1_HZ = 2,
    CPLD_LED_MODE_2_HZ = 3,
    CPLD_LED_MODE_ON = 7
} cpld_led_mode_t;

typedef enum led_driver_mode_e {
    LED_DRIVER_MODE_NONE,
    LED_DRIVER_MODE_CPLD,
    LED_DRIVER_MODE_PSOC
} led_driver_mode_t;


typedef struct ledi_info_s {
    platform_led_color_t color;
    led_driver_mode_t driver;
    char file[ONLP_CONFIG_INFO_STR_MAX];
} ledi_info_t;

/* function declarations*/
static onlp_led_mode_t _cpld_onlp_led_mode_convert(platform_led_color_t color, cpld_led_mode_t mode);
static int _onlp_cpld_led_mode_convert(onlp_led_mode_t onlp_led_mode, platform_led_color_t *pcolor, cpld_led_mode_t *pmode);
static int _cpld_onlp_ledi_info_get(onlp_oid_id_t id, onlp_led_info_t* info);
static int _psoc_onlp_ledi_info_get(onlp_oid_id_t id, onlp_led_info_t* info);
static int _cpld_onlp_ledi_mode_set(onlp_oid_id_t id, onlp_led_mode_t onlp_mode);
static int _psoc_onlp_ledi_mode_set(onlp_oid_id_t id, onlp_led_mode_t onlp_mode);

static ledi_info_t __info_list[ONLP_LED_COUNT] = {
    {},
    {PLATFORM_LED_COLOR_GREEN, LED_DRIVER_MODE_CPLD, "/sys/bus/i2c/devices/0-0055*grn_led"},
    {PLATFORM_LED_COLOR_RED, LED_DRIVER_MODE_CPLD, "/sys/bus/i2c/devices/0-0055*red_led"},
    {PLATFORM_LED_COLOR_GREEN, LED_DRIVER_MODE_PSOC,"/sys/bus/i2c/devices/0-0066*fan_led_grn1"},
    {PLATFORM_LED_COLOR_RED, LED_DRIVER_MODE_PSOC,"/sys/bus/i2c/devices/0-0066*fan_led_red1"},
    {PLATFORM_LED_COLOR_GREEN, LED_DRIVER_MODE_PSOC,"/sys/bus/i2c/devices/0-0066*fan_led_grn2"},
    {PLATFORM_LED_COLOR_RED, LED_DRIVER_MODE_PSOC,"/sys/bus/i2c/devices/0-0066*fan_led_red2"},
    {PLATFORM_LED_COLOR_GREEN, LED_DRIVER_MODE_PSOC,"/sys/bus/i2c/devices/0-0066*fan_led_grn3"},
    {PLATFORM_LED_COLOR_RED, LED_DRIVER_MODE_PSOC,"/sys/bus/i2c/devices/0-0066*fan_led_red3"},
    {PLATFORM_LED_COLOR_GREEN, LED_DRIVER_MODE_PSOC,"/sys/bus/i2c/devices/0-0066*fan_led_grn4"},
    {PLATFORM_LED_COLOR_RED, LED_DRIVER_MODE_PSOC,"/sys/bus/i2c/devices/0-0066*fan_led_red4"},
    {PLATFORM_LED_COLOR_GREEN, LED_DRIVER_MODE_PSOC,"/sys/bus/i2c/devices/0-0066*fan_led_grn5"},
    {PLATFORM_LED_COLOR_RED, LED_DRIVER_MODE_PSOC,"/sys/bus/i2c/devices/0-0066*fan_led_red5"},
};


/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t __onlp_led_info[ONLP_LED_COUNT] = {
    { }, /* Not used */
    ONLP_CHASSIS_LED_INFO_ENTRY_INIT(ONLP_LED_MGMT_GREEN, "MGMT LED GREEN", CPLD_LED_GREEN_CAPS),
    ONLP_CHASSIS_LED_INFO_ENTRY_INIT(ONLP_LED_MGMT_RED, "MGMT LED RED", CPLD_LED_RED_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN1_GREEN, "FAN LED 1 GREEN", ONLP_FAN_1, PSOC_LED_GREEN_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN1_RED, "FAN LED 1 RED", ONLP_FAN_1, PSOC_LED_RED_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN2_GREEN, "FAN LED 2 GREEN", ONLP_FAN_2, PSOC_LED_GREEN_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN2_RED, "FAN LED 2 RED", ONLP_FAN_2, PSOC_LED_RED_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN3_GREEN, "FAN LED 3 GREEN", ONLP_FAN_3, PSOC_LED_GREEN_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN3_RED, "FAN LED 3 RED", ONLP_FAN_3, PSOC_LED_RED_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN4_GREEN, "FAN LED 4 GREEN", ONLP_FAN_4, PSOC_LED_GREEN_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN4_RED, "FAN LED 4 RED", ONLP_FAN_4, PSOC_LED_RED_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN5_GREEN, "FAN LED 5 GREEN", ONLP_FAN_5, PSOC_LED_GREEN_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN5_RED, "FAN LED 5 RED", ONLP_FAN_5, PSOC_LED_RED_CAPS),
};


/* convert platform led type to onlp_led_mode type*/
static onlp_led_mode_t _cpld_onlp_led_mode_convert(platform_led_color_t color, cpld_led_mode_t mode)
{
    onlp_led_mode_t ret = ONLP_LED_MODE_OFF;

    /* First select the basic state */
    switch(mode) {
    case CPLD_LED_MODE_0_5_HZ:
    case CPLD_LED_MODE_1_HZ:
    case CPLD_LED_MODE_2_HZ:
        ret = ONLP_LED_MODE_AUTO_BLINKING;
        break;
    case CPLD_LED_MODE_ON:
        ret = ONLP_LED_MODE_AUTO;
        break;
    case CPLD_LED_MODE_OFF:
    default:
        ret = ONLP_LED_MODE_OFF;
        break;
    }
    if (ret != ONLP_LED_MODE_OFF) {
        /* Add shift to color */
        switch(color) {
        case PLATFORM_LED_COLOR_RED:
            ret += (ONLP_LED_MODE_RED - ONLP_LED_MODE_AUTO);
            break;
        case PLATFORM_LED_COLOR_GREEN:
            ret += (ONLP_LED_MODE_GREEN - ONLP_LED_MODE_AUTO);
            break;
        default:
            break;
        }
    }
    return ret;
}

static int _onlp_cpld_led_mode_convert(onlp_led_mode_t onlp_led_mode, platform_led_color_t *pcolor, cpld_led_mode_t *pmode)
{
    int rv = ONLP_STATUS_OK;
    switch(onlp_led_mode) {
    case ONLP_LED_MODE_OFF:
        *pcolor = PLATFORM_LED_COLOR_ANY;
        *pmode = CPLD_LED_MODE_OFF;
        break;
    case ONLP_LED_MODE_AUTO:
        *pcolor = PLATFORM_LED_COLOR_ANY;
        *pmode = CPLD_LED_MODE_ON;
        break;
    case ONLP_LED_MODE_AUTO_BLINKING:
        *pcolor = PLATFORM_LED_COLOR_ANY;
        *pmode = CPLD_LED_MODE_1_HZ;
        break;
    case ONLP_LED_MODE_RED:
        *pcolor = PLATFORM_LED_COLOR_RED;
        *pmode = CPLD_LED_MODE_ON;
        break;
    case ONLP_LED_MODE_RED_BLINKING:
        /* cannot determine the blink level currently, just choose 1Hz*/
        *pcolor = PLATFORM_LED_COLOR_RED;
        *pmode = CPLD_LED_MODE_1_HZ;
        break;
    case ONLP_LED_MODE_GREEN:
        *pcolor = PLATFORM_LED_COLOR_GREEN;
        *pmode = CPLD_LED_MODE_ON;
        break;
    case ONLP_LED_MODE_GREEN_BLINKING:
        /* cannot determine the blink level currently, just choose 1Hz*/
        *pcolor = PLATFORM_LED_COLOR_GREEN;
        *pmode = CPLD_LED_MODE_1_HZ;
        break;

    default:
        rv = ONLP_STATUS_E_INVALID;
        break;
    }
    return rv;
}


static int _cpld_onlp_ledi_info_get(onlp_oid_id_t id, onlp_led_info_t* info)
{
    int rv = ONLP_STATUS_OK;
    platform_led_color_t color;
    cpld_led_mode_t mode;

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = __onlp_led_info[id];

    color = __info_list[id].color;

    rv = onlp_file_read_int((int*)&mode, __info_list[id].file);

    if( ONLP_STATUS_OK == rv ) {
        info->mode = _cpld_onlp_led_mode_convert(color, mode);
    }

    return rv;
}

static int _psoc_onlp_ledi_info_get(onlp_oid_id_t id, onlp_led_info_t* info)
{
    int rv = ONLP_STATUS_OK;
    int mode;
    platform_led_color_t color;

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = __onlp_led_info[id];
    color = __info_list[id].color;

    rv = onlp_file_read_int((int*)&mode, __info_list[id].file);

    if( ONLP_STATUS_OK == rv ) {

        if(!mode) {
            info->mode = ONLP_LED_MODE_OFF;
        } else {
            info->mode = ONLP_LED_MODE_AUTO;
        }

        if(info->mode != ONLP_LED_MODE_OFF) {
            switch(color) {
            case PLATFORM_LED_COLOR_RED:
                info->mode += (ONLP_LED_MODE_RED - ONLP_LED_MODE_AUTO);
                break;
            case PLATFORM_LED_COLOR_GREEN:
                info->mode += (ONLP_LED_MODE_GREEN - ONLP_LED_MODE_AUTO);
                break;
            default:
                break;
            }
        }
    }

    return rv;
}

static int _cpld_onlp_ledi_mode_set(onlp_oid_id_t id, onlp_led_mode_t onlp_mode)
{
    int rv = ONLP_STATUS_OK;
    platform_led_color_t color;
    cpld_led_mode_t cpld_mode;

    /*convert onlp_led_mode to platform mod*/
    rv = _onlp_cpld_led_mode_convert(onlp_mode, &color, &cpld_mode);

    if( ONLP_STATUS_OK == rv) {
        if((color != __info_list[id].color)&&(color != PLATFORM_LED_COLOR_ANY)) {
            rv = ONLP_STATUS_E_INVALID;
        } else {
            rv = onlp_file_write_int(cpld_mode, __info_list[id].file);
        }
    }

    return rv;
}

static int _psoc_onlp_ledi_mode_set(onlp_oid_id_t id, onlp_led_mode_t onlp_mode)
{
    int rv = ONLP_STATUS_OK;
    platform_led_color_t color;
    int psoc_mode;
    int psoc_diag;

    id = ONLP_OID_ID_GET(id);

    rv = platform_psoc_diag_enable_read(&psoc_diag);

    if( ONLP_STATUS_OK == rv ) {
        if(!psoc_diag) {
            rv = ONLP_STATUS_E_UNSUPPORTED;
        }
    }

    switch(onlp_mode) {
    case ONLP_LED_MODE_OFF:
        psoc_mode = 0;
        color = PLATFORM_LED_COLOR_ANY;
        break;
    case ONLP_LED_MODE_AUTO:
        psoc_mode = 1;
        color = PLATFORM_LED_COLOR_ANY;
        break;
    case ONLP_LED_MODE_RED:
        color = PLATFORM_LED_COLOR_RED;
        psoc_mode = 1;
        break;
    case ONLP_LED_MODE_GREEN:
        color = PLATFORM_LED_COLOR_GREEN;
        psoc_mode = 1;
        break;
    default:
        rv = ONLP_STATUS_E_INVALID;
        break;
    }

    if( ONLP_STATUS_OK == rv) {
        if((psoc_mode)&&(color != __info_list[id].color)&&(color != PLATFORM_LED_COLOR_ANY)) {
            rv = ONLP_STATUS_E_INVALID;
        } else {
            rv = onlp_file_write_int(psoc_mode, __info_list[id].file);
        }
    }
    return rv;
}

int
onlp_ledi_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, ONLP_LED_MAX-1);
}

int
onlp_ledi_info_get(onlp_oid_id_t id, onlp_led_info_t* info)
{
    int rv = ONLP_STATUS_OK;

    switch(__info_list[id].driver) {
    case LED_DRIVER_MODE_NONE:
        *info = __onlp_led_info[id];
        break;
    case LED_DRIVER_MODE_CPLD:
        rv = _cpld_onlp_ledi_info_get(id, info);
        break;
    case LED_DRIVER_MODE_PSOC:
        rv = _psoc_onlp_ledi_info_get(id, info);
        break;
    default:
        rv = ONLP_STATUS_E_UNSUPPORTED;
        break;
    }
    return rv;
}

int
onlp_ledi_mode_set(onlp_oid_id_t id, onlp_led_mode_t mode)
{
    int rv = ONLP_STATUS_OK;

    switch(__info_list[id].driver) {
    case LED_DRIVER_MODE_CPLD:
        rv = _cpld_onlp_ledi_mode_set(id, mode);
        break;
    case LED_DRIVER_MODE_PSOC:
        rv = _psoc_onlp_ledi_mode_set(id, mode);
        break;
    default:
        rv = ONLP_STATUS_E_UNSUPPORTED;
        break;
    }
    return rv;
}
