/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2019 Delta Electronics, Inc.
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
#include "vendor_driver_pool.h"
#include "vendor_i2c_device_list.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum onlp_led_id
{
    LED_RESERVED = 0,
    LED1,
    LED2,
    LED3,
    LED4,
};

// led_size_list = 4
int mode_sum[4] =
{
    4,
    4,
    4,
    3
};

#define LED_INFO_ENTRY_INIT(_id, _desc, _caps, _defaultm) \
    {                                                     \
        {                                                 \
            .id = ONLP_LED_ID_CREATE(_id),                \
            .description = _desc,                         \
            .poid = 0                                     \
        },                                                \
            .caps = _caps,                                \
            .mode = _defaultm,                            \
            .status = ONLP_LED_STATUS_PRESENT,            \
    }

static onlp_led_info_t onlp_led_info[] =
    {
        {}, /* Not used */
        LED_INFO_ENTRY_INIT(LED1, "sysled-power-1",
                            (ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO), ONLP_LED_MODE_ON),
        LED_INFO_ENTRY_INIT(LED2, "sysled-power-2",
                            (ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO), ONLP_LED_MODE_ON),
        LED_INFO_ENTRY_INIT(LED3, "sysled-system",
                            (ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING), ONLP_LED_MODE_ON),
        LED_INFO_ENTRY_INIT(LED4, "sysled-fan",
                            (ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN), ONLP_LED_MODE_ON),
};

int
onlp_ledi_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    *hdr = onlp_led_info[ONLP_OID_ID_GET(id)].hdr;
    return ONLP_STATUS_OK;
}

int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int rv = 0, nid = ONLP_OID_ID_GET(id) - 1, cpld_idx = 0, mode_idx = 0;
    vendor_dev_led_pin_t *curr_mode;
    uint8_t mode = 0;

    *info = onlp_led_info[ONLP_OID_ID_GET(id)];

    void *busDrv = (void *)vendor_find_driver_by_name(sysled_dev_list[nid].bus_drv_name);
    cpld_dev_driver_t *cpld =
        (cpld_dev_driver_t *)vendor_find_driver_by_name(sysled_dev_list[nid].dev_drv_name);

    cpld_idx = vendor_find_cpld_idx(sysled_color_list[nid]->addr);
    if(cpld_idx < 0) return ONLP_STATUS_E_INTERNAL;

    vendor_dev_do_oc(cpld_o_list[cpld_idx]);
    rv = cpld->readb(
        busDrv,
        sysled_color_list[nid]->bus,
        sysled_color_list[nid]->addr,
        sysled_color_list[nid]->offset,
        &mode);
    vendor_dev_do_oc(cpld_c_list[cpld_idx]);

    if(rv < 0) return ONLP_STATUS_E_INTERNAL;
    curr_mode = sysled_color_list[nid];

    for (mode_idx = 0; mode_idx < mode_sum[nid]; mode_idx++)
    {
        if(((mode & curr_mode->mask) == curr_mode->match) ? 1 : 0)
        {
            info->mode = curr_mode->mode;
            return ONLP_STATUS_OK;
        }
        curr_mode++;
    }

    AIM_LOG_ERROR("Function: %s, Unknown LED mode.", __FUNCTION__);
    return ONLP_STATUS_E_INTERNAL;
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
    int rv = 0, nid = ONLP_OID_ID_GET(id) - 1, cpld_idx = 0, mode_idx = 0;;

    vendor_dev_led_pin_t *led_list;

    void *busDrv = (void *)vendor_find_driver_by_name(sysled_dev_list[nid].bus_drv_name);
    cpld_dev_driver_t *cpld =
        (cpld_dev_driver_t *)vendor_find_driver_by_name(sysled_dev_list[nid].dev_drv_name);

    cpld_idx = vendor_find_cpld_idx(sysled_color_list[nid]->addr);
    if(cpld_idx < 0) return ONLP_STATUS_E_INTERNAL;

    led_list = sysled_color_list[nid];
    for (mode_idx = 0; mode_idx < mode_sum[nid]; mode_idx++)
    {
        if(mode == led_list->mode)
        {
            vendor_dev_do_oc(cpld_o_list[cpld_idx]);

            rv = cpld->writeb(
                busDrv,
                sysled_color_list[nid]->bus,
                sysled_color_list[nid]->addr,
                sysled_color_list[nid]->offset,
                led_list->match);
            vendor_dev_do_oc(cpld_c_list[cpld_idx]);

            if(rv < 0) return ONLP_STATUS_E_INTERNAL;

            return ONLP_STATUS_OK;
        }

        led_list++;
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