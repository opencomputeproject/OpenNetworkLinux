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
        { ONLP_LED_ID_CREATE(LED_FRONT_PWR1), "PSU1 LED (FRONT PANEL)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_PWR2), "PSU2 LED (FRONT PANEL)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1), "FAN TRAY 1 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2), "FAN TRAY 2 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3), "FAN TRAY 3 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4), "FAN TRAY 4 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
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

    switch (local_id) {
        case LED_FRONT_FAN:
            if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, LED_FRONT_FAN_SYS_REGISTER, &r_data)
                != ONLP_STATUS_OK) {
                return ONLP_STATUS_E_INTERNAL;
            }
            if ((r_data & LED_FRONT_FAN_MASK) == LED_FRONT_FAN_MODE_GREEN)
                info->mode = ONLP_LED_MODE_GREEN;
            else if ((r_data & LED_FRONT_FAN_MASK) == LED_FRONT_FAN_MODE_RED)
                info->mode = ONLP_LED_MODE_RED;
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_FRONT_SYS:
            if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, LED_FRONT_FAN_SYS_REGISTER, &r_data)
                != ONLP_STATUS_OK) {
                return ONLP_STATUS_E_INTERNAL;
            }
            if ((r_data & LED_FRONT_SYS_MASK) == LED_FRONT_SYS_MODE_GREEN)
                info->mode = ONLP_LED_MODE_GREEN;
            else if ((r_data & LED_FRONT_SYS_MASK) == LED_FRONT_SYS_MODE_GREEN_BLK)
                info->mode = ONLP_LED_MODE_GREEN_BLINKING;
            else if ((r_data & LED_FRONT_SYS_MASK) == LED_FRONT_SYS_MODE_RED)
                info->mode = ONLP_LED_MODE_RED;
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_FRONT_PWR1:
            if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_1_ADDR, PSU_STATUS_REGISTER, &r_data)
                != ONLP_STATUS_OK) {
                return ONLP_STATUS_E_INTERNAL;
            }
            if ((r_data & LED_FRONT_PSU1_PRESENT_MASK) == LED_FRONT_PSU_PRESENT &&
                (r_data & LED_FRONT_PSU1_PWR_OK_MASK) == LED_FRONT_PSU_PWR_GOOD)
                info->mode = ONLP_LED_MODE_GREEN;
            else
                info->mode = ONLP_LED_MODE_RED;
            break;
        case LED_FRONT_PWR2:
            if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_1_ADDR, PSU_STATUS_REGISTER, &r_data)
                != ONLP_STATUS_OK) {
                return ONLP_STATUS_E_INTERNAL;
            }
            if ((r_data & LED_FRONT_PSU2_PRESENT_MASK) == LED_FRONT_PSU_PRESENT &&
                (r_data & LED_FRONT_PSU2_PWR_OK_MASK) == LED_FRONT_PSU_PWR_GOOD)
                info->mode = ONLP_LED_MODE_GREEN;
            else
                info->mode = ONLP_LED_MODE_RED;
            break;
        case LED_REAR_FAN_TRAY_1:
            if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, LED_FAN_TRAY_REGISTER, &r_data)
                != ONLP_STATUS_OK) {
                return ONLP_STATUS_E_INTERNAL;
            }
            if (dni_fan_present(LED_REAR_FAN_TRAY_1) == ONLP_STATUS_OK) {
                if ((r_data & LED_REAR_FAN1_MASK) == LED_REAR_FAN1_MODE_GREEN)
                    info->mode = ONLP_LED_MODE_GREEN;
                else
                    info->mode = ONLP_LED_MODE_ORANGE;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_2:
            if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, LED_FAN_TRAY_REGISTER, &r_data)
                != ONLP_STATUS_OK){
                return ONLP_STATUS_E_INTERNAL;
            }
            if (dni_fan_present(LED_REAR_FAN_TRAY_2) == ONLP_STATUS_OK) {
                if ((r_data & LED_REAR_FAN2_MASK) == LED_REAR_FAN2_MODE_GREEN)
                    info->mode = ONLP_LED_MODE_GREEN;
                else
                    info->mode = ONLP_LED_MODE_ORANGE;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_3:
            if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, LED_FAN_TRAY_REGISTER, &r_data)
                != ONLP_STATUS_OK){
                return ONLP_STATUS_E_INTERNAL;
            }
            if (dni_fan_present(LED_REAR_FAN_TRAY_3) == ONLP_STATUS_OK) {
                if ((r_data & LED_REAR_FAN3_MASK) == LED_REAR_FAN3_MODE_GREEN)
                    info->mode = ONLP_LED_MODE_GREEN;
                else
                    info->mode = ONLP_LED_MODE_ORANGE;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_4:
            if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, LED_FAN_TRAY_REGISTER, &r_data)
                != ONLP_STATUS_OK){
                return ONLP_STATUS_E_INTERNAL;
            }
            if (dni_fan_present(LED_REAR_FAN_TRAY_4) == ONLP_STATUS_OK) {
                if ((r_data & LED_REAR_FAN4_MASK) == LED_REAR_FAN4_MODE_GREEN)
                    info->mode = ONLP_LED_MODE_GREEN;
                else
                    info->mode = ONLP_LED_MODE_ORANGE;
            }
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

