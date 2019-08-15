/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017 Delta Networks, Inc.
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
#include "platform_lib.h"
#include <onlplib/i2c.h>
#include <onlp/platformi/ledi.h>


#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/*
 * Get the information for the given LED OID.
 */

static onlp_led_info_t linfo[] =
{
    { }, // Not used 
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_FAN), "FRONT LED (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_SYS), "FRONT LED  (SYS LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_PWR1), "FRONT LED  (PWR1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_PWR2), "FRONT LED (PWR2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1), "FAN TRAY 1 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2), "FAN TRAY 2 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3), "FAN TRAY 3 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4), "FAN TRAY 4 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_5), "FAN TRAY 5 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
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

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int rv = ONLP_STATUS_OK;
    int local_id;
    int r_data = 0;
    int bit_data = 0;
    uint8_t present_bit = 0x00;
    uint8_t bit = 0x00;
    dev_info_t dev_info;

    VALIDATE(id);
    local_id = ONLP_OID_ID_GET(id);
    *info = linfo[ONLP_OID_ID_GET(id)];

    dev_info.bus = I2C_BUS_1;
    dev_info.addr = SWPLD_1_ADDR;
    dev_info.offset = SYS_LED1_REGISTER;
    dev_info.flags = ONLP_I2C_F_FORCE;
    r_data = onlp_i2c_readb(dev_info.bus, dev_info.addr, dev_info.offset, dev_info.flags);

    rv = dni_fanpresent_info_get(&bit_data);

    if(rv == ONLP_STATUS_OK)
    {
        present_bit = bit_data;
    }
    else
    {
        rv = ONLP_STATUS_E_INVALID;
    }
    
    switch(local_id)
    {
        case LED_FRONT_FAN:
            if((r_data & 0x03) == 0x01)
                info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0x03) == 0x02)
                info->mode = ONLP_LED_MODE_ORANGE;
            else if ((r_data & 0x03) == 0x03 || (r_data & 0x03) == 0x00)
                info->mode = ONLP_LED_MODE_OFF;
            else 
                return ONLP_STATUS_E_INTERNAL;
            break;
      
        case LED_FRONT_SYS:
            if((r_data & 0x0c) == 0x04)
                info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0x0c) == 0x08)
                info->mode = ONLP_LED_MODE_GREEN_BLINKING;
            else if((r_data & 0x0c) == 0x0c)
                info->mode = ONLP_LED_MODE_RED;
            else if ((r_data & 0x0c) == 0x00)
                info->mode = ONLP_LED_MODE_OFF;
            else
                return ONLP_STATUS_E_INTERNAL;
            break;

        case LED_FRONT_PWR1:
            if((r_data & 0xc0) == 0x40)
                info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0xc0) == 0x80)
                info->mode = ONLP_LED_MODE_ORANGE_BLINKING;
            else if ((r_data & 0xc0) == 0xc0 || (r_data & 0xc0) == 0x00)
                info->mode = ONLP_LED_MODE_OFF;
            else
                return ONLP_STATUS_E_INTERNAL; 
            break;

        case LED_FRONT_PWR2:
            if((r_data & 0x30) == 0x10)
                info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0x30) == 0x20)
                info->mode = ONLP_LED_MODE_ORANGE_BLINKING;
            else if ((r_data & 0x30) == 0x30 || (r_data & 0x30) == 0x00)
                info->mode = ONLP_LED_MODE_OFF;
            else
                return ONLP_STATUS_E_INTERNAL;
            break;

        case LED_REAR_FAN_TRAY_1:
            dev_info.offset = SYS_FANLED1_REGISTER;
            r_data = onlp_i2c_readb(dev_info.bus, dev_info.addr, dev_info.offset, dev_info.flags);

            if((present_bit & ((bit+1) << 4)) == 0)
            {
                if((r_data & 0xc0) == 0x40)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data & 0xc0) == 0x80)
                    info->mode = ONLP_LED_MODE_RED;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;

        case LED_REAR_FAN_TRAY_2:
            dev_info.offset = SYS_FANLED2_REGISTER;
            r_data = onlp_i2c_readb(dev_info.bus, dev_info.addr, dev_info.offset, dev_info.flags);

            if((present_bit & ((bit+1) << 3)) == 0)
            {
                if((r_data & 0x03) == 0x01)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data & 0x03) == 0x02)
                    info->mode = ONLP_LED_MODE_RED;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;

        case LED_REAR_FAN_TRAY_3:
            dev_info.offset = SYS_FANLED2_REGISTER;
            r_data = onlp_i2c_readb(dev_info.bus, dev_info.addr, dev_info.offset, dev_info.flags);

            if((present_bit & ((bit+1) << 2)) == 0)
            {
                if((r_data & 0x0c) == 0x04)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data & 0x0c) == 0x08)
                    info->mode = ONLP_LED_MODE_RED;
            }
            else 
                info->mode = ONLP_LED_MODE_OFF;
            break;

        case LED_REAR_FAN_TRAY_4:
            dev_info.offset = SYS_FANLED2_REGISTER;
            r_data = onlp_i2c_readb(dev_info.bus, dev_info.addr, dev_info.offset, dev_info.flags);

            if((present_bit & ((bit+1) << 1)) == 0)
            {
                if((r_data & 0x30) == 0x10)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data & 0x30) == 0x20)
                    info->mode = ONLP_LED_MODE_RED;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
 
        case LED_REAR_FAN_TRAY_5:
            dev_info.offset = SYS_FANLED2_REGISTER;
            r_data = onlp_i2c_readb(dev_info.bus, dev_info.addr, dev_info.offset, dev_info.flags);

            if((present_bit & (bit+1)) == 0)
            {
                if((r_data & 0xc0) == 0x40)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data & 0xc0) == 0x80)
                    info->mode = ONLP_LED_MODE_RED;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
    }

    /* Set the on/off status */
    if (info->mode == ONLP_LED_MODE_OFF)
        info->status |= ONLP_LED_STATUS_FAILED;
    else
        info->status |=ONLP_LED_STATUS_PRESENT;

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