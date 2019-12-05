/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 MiTAC Computing Technology Corporation.
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
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlplib/mmap.h>

#include "platform_lib.h"

#define prefix_path "/sys/bus/i2c/devices/7-0066/"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum id_led_mode {
    ID_LED_MODE_OFF = 0,
    ID_LED_MODE_BLUE,
    ID_LED_MODE_BLUE_BLINK_PER_SEC,
    ID_LED_MODE_BLUE_BLINK_PER_2SEC,
    ID_LED_MODE_UNKNOWN
};

enum status_led_mode {
    STATUS_LED_MODE_OFF = 0,
    STATUS_LED_MODE_GREEN,
    STATUS_LED_MODE_GREEN_BLINK_PER_SEC,
    STATUS_LED_MODE_GREEN_BLINK_PER_2SEC,
    STATUS_LED_MODE_UNKNOWN
};

enum fan_led_mode {
    FAN_LED_MODE_OFF = 0,
    FAN_LED_MODE_GREEN,
    FAN_LED_MODE_UNKNOWN
};

static char last_path[][20] =  /* must map with onlp_led_id */
{
    "reserved",
    "status_led",
    "id_led",
    "fan_led_1",
    "fan_led_2",
    "fan_led_3",
    "fan_led_4",
    "fan_led_5",
    "fan_led_6",
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_STATUS), "Chassis LED 1 (STATUS LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_ID), "Chassis LED 2 (ID LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_BLUE | ONLP_LED_CAPS_BLUE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_1), "Chassis LED 3 (FAN1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_2), "Chassis LED 4 (FAN2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_3), "Chassis LED 5 (FAN3 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_4), "Chassis LED 6 (FAN4 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_5), "Chassis LED 7 (FAN5 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_6), "Chassis LED 8 (FAN6 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
    },
};

static int driver_to_onlp_led_mode(enum onlp_led_id id, int value)
{
    switch (id)
    {
        case LED_STATUS:
            if (value == STATUS_LED_MODE_OFF)
                return ONLP_LED_MODE_OFF;
            if (value == STATUS_LED_MODE_GREEN)
                return ONLP_LED_MODE_GREEN;
            if ((value == STATUS_LED_MODE_GREEN_BLINK_PER_SEC) || (value == STATUS_LED_MODE_GREEN_BLINK_PER_2SEC))
                return ONLP_LED_MODE_GREEN_BLINKING;
            AIM_LOG_ERROR("Status led value (%d) is illegal \r\n", value);
            return ONLP_LED_MODE_OFF;
        case LED_ID:
            if (value == ID_LED_MODE_OFF)
                return ONLP_LED_MODE_OFF;
            if (value == ID_LED_MODE_BLUE)
                return ONLP_LED_MODE_BLUE;
            if ((value == ID_LED_MODE_BLUE_BLINK_PER_SEC) || (value == ID_LED_MODE_BLUE_BLINK_PER_2SEC))
                return ONLP_LED_MODE_BLUE_BLINKING;
            AIM_LOG_ERROR("ID led value (%d) is illegal \r\n", value);
            return ONLP_LED_MODE_OFF;
        case LED_FAN_1:
        case LED_FAN_2:
        case LED_FAN_3:
        case LED_FAN_4:
        case LED_FAN_5:
        case LED_FAN_6:
            if (value == FAN_LED_MODE_OFF)
                return ONLP_LED_MODE_OFF;
            if (value == FAN_LED_MODE_GREEN)
                return ONLP_LED_MODE_GREEN;
            AIM_LOG_ERROR("Fan led value (%d) is illegal \r\n", value);
            return ONLP_LED_MODE_OFF;
        default:
            AIM_LOG_ERROR("led(%d) not matched any ONLP_LED_MODE \r\n", id);
            break;
    }
    return ONLP_LED_MODE_OFF;
}

static int onlp_to_driver_led_mode(enum onlp_led_id id, onlp_led_mode_t onlp_led_mode)
{
    switch (id)
    {
        case LED_STATUS:
            if (onlp_led_mode == ONLP_LED_MODE_OFF)
                return STATUS_LED_MODE_OFF;
            if (onlp_led_mode == ONLP_LED_MODE_GREEN)
                return STATUS_LED_MODE_GREEN;
            if (onlp_led_mode == ONLP_LED_MODE_GREEN_BLINKING)
                return STATUS_LED_MODE_GREEN_BLINK_PER_SEC;
            AIM_LOG_ERROR("Illegal led mode (%d) for status led \r\n", onlp_led_mode);
            return STATUS_LED_MODE_OFF;
        case LED_ID:
            if (onlp_led_mode == ONLP_LED_MODE_OFF)
                return ID_LED_MODE_OFF;
            if (onlp_led_mode == ONLP_LED_MODE_BLUE)
                return ID_LED_MODE_BLUE;
            if (onlp_led_mode == ONLP_LED_MODE_BLUE_BLINKING)
                return ID_LED_MODE_BLUE_BLINK_PER_SEC;
            AIM_LOG_ERROR("Illegal led mode (%d) for id led \r\n", onlp_led_mode);
            return ID_LED_MODE_OFF;
        case LED_FAN_1:
        case LED_FAN_2:
        case LED_FAN_3:
        case LED_FAN_4:
        case LED_FAN_5:
        case LED_FAN_6:
            if (onlp_led_mode == ONLP_LED_MODE_OFF)
                return FAN_LED_MODE_OFF;
            if (onlp_led_mode == ONLP_LED_MODE_GREEN)
                return FAN_LED_MODE_GREEN;
            AIM_LOG_ERROR("Illegal led mode (%d) for fan led \r\n", onlp_led_mode);
            return FAN_LED_MODE_OFF;
        default:
            AIM_LOG_ERROR("led(%d) not matched any ONLP_LED_MODE \r\n", id);
            break;
    }
    return ONLP_STATUS_OK;
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int local_id;
    int value;
    char node_path[SYSFS_NODE_PATH_MAX_LEN] = {0};

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    /* get sysfs node */
    sprintf(node_path, "%s/%s", prefix_path, last_path[local_id]);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    /* Get LED mode */
    if (onlp_file_read_int_hex(&value, node_path) < 0) {
        DEBUG_PRINT("%s(%d)\r\n", __FUNCTION__, __LINE__);
        return ONLP_STATUS_E_INTERNAL;
    }

    info->mode = driver_to_onlp_led_mode(local_id, value);

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
    char node_path[SYSFS_NODE_PATH_MAX_LEN] = {0};

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    /* get sysfs node */
    sprintf(node_path, "%s/%s", prefix_path, last_path[local_id]);

    if (onlp_file_write_int(onlp_to_driver_led_mode(local_id, mode), node_path) != 0)
    {
        return ONLP_STATUS_E_INTERNAL;
    }

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
