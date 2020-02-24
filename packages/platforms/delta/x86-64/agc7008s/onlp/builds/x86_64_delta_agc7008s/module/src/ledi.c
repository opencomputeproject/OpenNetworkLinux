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
static onlp_led_info_t led_info[] =
{
    { }, // Not used
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_FAN), "FAN LED (FRONT PANEL)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_SYS), "SYSTEM LED (FRONT PANEL)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_PWR), "PWR LED (FRONT PANEL)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
};

int onlp_ledi_init(void)
{
    lockinit();
    return ONLP_STATUS_OK;
}

int onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int local_id;
    int r_data = 0;
    local_id = ONLP_OID_ID_GET(id);

    VALIDATE(id);
    *info = led_info[ONLP_OID_ID_GET(id)];

    if (dni_bmc_data_get(BMC_BUS, SYSTEM_CPLD_ADDR, LED_REGISTER, &r_data)
        != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }

    switch (local_id) {
        case LED_FRONT_FAN:
            if ((r_data & LED_FRONT_FAN_MASK) == LED_FRONT_FAN_MODE_GREEN)
                info->mode = ONLP_LED_MODE_GREEN;
            else if ((r_data & LED_FRONT_FAN_MASK) == LED_FRONT_FAN_MODE_RED)
                info->mode = ONLP_LED_MODE_RED;
            else if ((r_data & LED_FRONT_FAN_MASK) == LED_FRONT_FAN_MODE_RED_BLK)
                info->mode = ONLP_LED_MODE_RED_BLINKING;
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_FRONT_SYS:
            if ((r_data & LED_FRONT_SYS_MASK) == LED_FRONT_SYS_MODE_GREEN)
                info->mode = ONLP_LED_MODE_GREEN;
            else if ((r_data & LED_FRONT_SYS_MASK) == LED_FRONT_SYS_MODE_GREEN_BLK)
                info->mode = ONLP_LED_MODE_GREEN_BLINKING;
            else if ((r_data & LED_FRONT_SYS_MASK) == LED_FRONT_SYS_MODE_RED)
                info->mode = ONLP_LED_MODE_RED;
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_FRONT_PWR:
            if ((r_data & LED_FRONT_PWR_MASK) == LED_FRONT_PWR_MODE_GREEN)
                info->mode = ONLP_LED_MODE_GREEN;
            else if ((r_data & LED_FRONT_PWR_MASK) == LED_FRONT_PWR_MODE_RED)
                info->mode = ONLP_LED_MODE_RED;
            else
                info->mode = ONLP_LED_MODE_OFF;

            break;
    }
    /* Set the on/off status */
    if (info->mode == ONLP_LED_MODE_OFF)
        info->status = ONLP_LED_STATUS_FAILED;
    else
        info->status |= ONLP_LED_STATUS_PRESENT;

    return ONLP_STATUS_OK;
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

int onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    return ONLP_STATUS_OK;
}

int onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

