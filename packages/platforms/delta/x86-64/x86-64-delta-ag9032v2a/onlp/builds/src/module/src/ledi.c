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
        ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_SYS), "FRONT LED  (SYS LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_PWR1), "FRONT LED  (PWR1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_PWR2), "FRONT LED (PWR2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
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
    lockinit();
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int local_id;
    int r_data = 0;
    int bit_data = 0;

    VALIDATE(id);
    local_id = ONLP_OID_ID_GET(id);
    *info = linfo[ONLP_OID_ID_GET(id)];

    if(dni_bmc_check() == BMC_ON){
        switch(local_id)
        {
            case LED_FRONT_FAN:
                if( dni_bmc_data_get(I2C_BUS_3, SWPLD_1_ADDR, SYS_LED1_REGISTER, 1, &bit_data) != ONLP_STATUS_OK){
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data = bit_data;
                if((r_data & 0x03) == 0x01)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data & 0x03) == 0x02)
                    info->mode = ONLP_LED_MODE_RED;
                else if ((r_data & 0x03) == 0x03 || (r_data & 0x03) == 0x00)
                    info->mode = ONLP_LED_MODE_OFF;
                else
                    return ONLP_STATUS_E_INTERNAL;
                break;

            case LED_FRONT_SYS:
                if( dni_bmc_data_get(I2C_BUS_3, SWPLD_1_ADDR, SYS_LED1_REGISTER, 1, &bit_data) != ONLP_STATUS_OK){
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data = bit_data;
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
                if( dni_bmc_data_get(I2C_BUS_3, SWPLD_1_ADDR, SYS_LED1_REGISTER, 1, &bit_data) != ONLP_STATUS_OK){
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data = bit_data;
                if((r_data & 0xc0) == 0x40)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data & 0xc0) == 0x80)
                    info->mode = ONLP_LED_MODE_RED;
                else if ((r_data & 0xc0) == 0xc0 || (r_data & 0xc0) == 0x00)
                    info->mode = ONLP_LED_MODE_OFF;
                else
                    return ONLP_STATUS_E_INTERNAL;
                break;

            case LED_FRONT_PWR2:
                if( dni_bmc_data_get(I2C_BUS_3, SWPLD_1_ADDR, SYS_LED1_REGISTER, 1, &bit_data) != ONLP_STATUS_OK){
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data = bit_data;
                if((r_data & 0x30) == 0x10)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data & 0x30) == 0x20)
                    info->mode = ONLP_LED_MODE_RED;
                else if ((r_data & 0x30) == 0x30 || (r_data & 0x30) == 0x00)
                    info->mode = ONLP_LED_MODE_OFF;
                else
                    return ONLP_STATUS_E_INTERNAL;
                break;

            case LED_REAR_FAN_TRAY_1:
                if( dni_bmc_data_get(I2C_BUS_3, SWPLD_1_ADDR, SYS_LED3_REGISTER, 1, &bit_data) != ONLP_STATUS_OK){
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data = bit_data;
                if(dni_fan_present(LED_REAR_FAN_TRAY_1) == ONLP_STATUS_OK){
                    if((r_data & 0xc0) == 0x40)
                        info->mode = ONLP_LED_MODE_GREEN;
                    else if((r_data & 0xc0) == 0x80)
                        info->mode = ONLP_LED_MODE_RED;
                }
                else
                    info->mode = ONLP_LED_MODE_OFF;
                break;

            case LED_REAR_FAN_TRAY_2:
                if( dni_bmc_data_get(I2C_BUS_3, SWPLD_1_ADDR, FAN_LED_REGISTER, 1, &bit_data) != ONLP_STATUS_OK){
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data = bit_data;
                if(dni_fan_present(LED_REAR_FAN_TRAY_2) == ONLP_STATUS_OK){
                    if((r_data & 0x03) == 0x01)
                        info->mode = ONLP_LED_MODE_GREEN;
                    else if((r_data & 0x03) == 0x02)
                        info->mode = ONLP_LED_MODE_RED;
                }
                else
                    info->mode = ONLP_LED_MODE_OFF;
                break;
            case LED_REAR_FAN_TRAY_3:
                if( dni_bmc_data_get(I2C_BUS_3, SWPLD_1_ADDR, FAN_LED_REGISTER, 1, &bit_data) != ONLP_STATUS_OK){
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data = bit_data;
                if(dni_fan_present(LED_REAR_FAN_TRAY_3) == ONLP_STATUS_OK){
                    if((r_data & 0x0c) == 0x04)
                        info->mode = ONLP_LED_MODE_GREEN;
                    else if((r_data & 0x0c) == 0x08)
                        info->mode = ONLP_LED_MODE_RED;
                }
                else
                    info->mode = ONLP_LED_MODE_OFF;
                break;
            case LED_REAR_FAN_TRAY_4:
                if( dni_bmc_data_get(I2C_BUS_3, SWPLD_1_ADDR, FAN_LED_REGISTER, 1, &bit_data) != ONLP_STATUS_OK){
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data = bit_data;

                if(dni_fan_present(LED_REAR_FAN_TRAY_4) == ONLP_STATUS_OK){
                    if((r_data & 0x30) == 0x10)
                        info->mode = ONLP_LED_MODE_GREEN;
                    else if((r_data & 0x30) == 0x20)
                        info->mode = ONLP_LED_MODE_RED;
                }
                else
                    info->mode = ONLP_LED_MODE_OFF;
                break;

            case LED_REAR_FAN_TRAY_5:
                if( dni_bmc_data_get(I2C_BUS_3, SWPLD_1_ADDR, FAN_LED_REGISTER, 1, &bit_data) != ONLP_STATUS_OK){
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data = bit_data;

                if(dni_fan_present(LED_REAR_FAN_TRAY_5) == ONLP_STATUS_OK){
                    if((r_data & 0xc0) == 0x40)
                        info->mode = ONLP_LED_MODE_GREEN;
                    else if((r_data & 0xc0) == 0x80)
                        info->mode = ONLP_LED_MODE_RED;
                }
                else
                    info->mode = ONLP_LED_MODE_OFF;
                break;
        }

    }
    else{
        switch(local_id)
        {
            case LED_FRONT_FAN:
                r_data = dni_lock_cpld_read_attribute(SWPLD1_PATH,SYS_LED1_REGISTER);
                if((r_data & 0x03) == 0x01)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data & 0x03) == 0x02)
                    info->mode = ONLP_LED_MODE_RED;
                else if ((r_data & 0x03) == 0x03 || (r_data & 0x03) == 0x00)
                    info->mode = ONLP_LED_MODE_OFF;
                else
                    return ONLP_STATUS_E_INTERNAL;
                break;

            case LED_FRONT_SYS:
                r_data = dni_lock_cpld_read_attribute(SWPLD1_PATH,SYS_LED1_REGISTER);
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
                r_data = dni_lock_cpld_read_attribute(SWPLD1_PATH,SYS_LED1_REGISTER);
                if((r_data & 0xc0) == 0x40)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data & 0xc0) == 0x80)
                    info->mode = ONLP_LED_MODE_RED;
                else if ((r_data & 0xc0) == 0xc0 || (r_data & 0xc0) == 0x00)
                    info->mode = ONLP_LED_MODE_OFF;
                else
                    return ONLP_STATUS_E_INTERNAL;
                break;

            case LED_FRONT_PWR2:
                r_data = dni_lock_cpld_read_attribute(SWPLD1_PATH,SYS_LED1_REGISTER);
                if((r_data & 0x30) == 0x10)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data & 0x30) == 0x20)
                    info->mode = ONLP_LED_MODE_RED;
                else if ((r_data & 0x30) == 0x30 || (r_data & 0x30) == 0x00)
                    info->mode = ONLP_LED_MODE_OFF;
                else
                    return ONLP_STATUS_E_INTERNAL;
                break;

            case LED_REAR_FAN_TRAY_1:
                r_data = dni_lock_cpld_read_attribute(SWPLD1_PATH,SYS_LED3_REGISTER);
                if(dni_fan_present(LED_REAR_FAN_TRAY_1) == ONLP_STATUS_OK){
                    if((r_data & 0xc0) == 0x40)
                        info->mode = ONLP_LED_MODE_GREEN;
                    else if((r_data & 0xc0) == 0x80)
                        info->mode = ONLP_LED_MODE_RED;
                }
                else
                    info->mode = ONLP_LED_MODE_OFF;
                break;

            case LED_REAR_FAN_TRAY_2:
                r_data = dni_lock_cpld_read_attribute(SWPLD1_PATH,FAN_LED_REGISTER);
                if(dni_fan_present(LED_REAR_FAN_TRAY_2) == ONLP_STATUS_OK){
                    if((r_data & 0x03) == 0x01)
                        info->mode = ONLP_LED_MODE_GREEN;
                    else if((r_data & 0x03) == 0x02)
                        info->mode = ONLP_LED_MODE_RED;
                }
                else
                    info->mode = ONLP_LED_MODE_OFF;
                break;

            case LED_REAR_FAN_TRAY_3:
                r_data = dni_lock_cpld_read_attribute(SWPLD1_PATH,FAN_LED_REGISTER);
                if(dni_fan_present(LED_REAR_FAN_TRAY_3) == ONLP_STATUS_OK){
                    if((r_data & 0x0c) == 0x04)
                        info->mode = ONLP_LED_MODE_GREEN;
                    else if((r_data & 0x0c) == 0x08)
                        info->mode = ONLP_LED_MODE_RED;
                }
                else
                    info->mode = ONLP_LED_MODE_OFF;
                break;

            case LED_REAR_FAN_TRAY_4:
                r_data = dni_lock_cpld_read_attribute(SWPLD1_PATH,FAN_LED_REGISTER);
                if(dni_fan_present(LED_REAR_FAN_TRAY_4) == ONLP_STATUS_OK){
                    if((r_data & 0x30) == 0x10)
                        info->mode = ONLP_LED_MODE_GREEN;
                    else if((r_data & 0x30) == 0x20)
                        info->mode = ONLP_LED_MODE_RED;
                }
                else
                    info->mode = ONLP_LED_MODE_OFF;
            break;

            case LED_REAR_FAN_TRAY_5:
                r_data = dni_lock_cpld_read_attribute(SWPLD1_PATH,FAN_LED_REGISTER);
                if(dni_fan_present(LED_REAR_FAN_TRAY_5) == ONLP_STATUS_OK){
                    if((r_data & 0xc0) == 0x40)
                        info->mode = ONLP_LED_MODE_GREEN;
                    else if((r_data & 0xc0) == 0x80)
                        info->mode = ONLP_LED_MODE_RED;
                }
                else
                    info->mode = ONLP_LED_MODE_OFF;
                break;
        }
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
    VALIDATE(id);
    int local_id = ONLP_OID_ID_GET(id);
    uint8_t front_panel_led_value = 0;
    uint8_t fan_tray_led_reg_value = 0;

    if(dni_bmc_check() == BMC_ON){
        return ONLP_STATUS_OK;
    }
    else 
    {
        switch(local_id)
        {
            case LED_FRONT_FAN:
                front_panel_led_value = dni_lock_cpld_read_attribute(SWPLD1_PATH,SYS_LED1_REGISTER);
                front_panel_led_value &= ~0x03;

                if(mode == ONLP_LED_MODE_GREEN){
                    front_panel_led_value |= 0x01;
                }
                else if(mode == ONLP_LED_MODE_RED){
                    front_panel_led_value |= 0x02;
                }
                else{
                    front_panel_led_value = front_panel_led_value;
                }

                if(dni_lock_cpld_write_attribute(SWPLD1_PATH,SYS_LED1_REGISTER,front_panel_led_value) != 0){
                    AIM_LOG_ERROR("Unable to set led(%d) status\r\n",local_id);
                    return ONLP_STATUS_E_INTERNAL;
                }
                break;
            case LED_FRONT_SYS:
                front_panel_led_value = dni_lock_cpld_read_attribute(SWPLD1_PATH,SYS_LED1_REGISTER);
                front_panel_led_value &= ~0x0c;

                if(mode == ONLP_LED_MODE_GREEN){
                    front_panel_led_value |= 0x04;
                }
                else if(mode == ONLP_LED_MODE_RED){
                    front_panel_led_value |= 0x08;
                }
                else if(mode == ONLP_LED_MODE_GREEN_BLINKING){
                    front_panel_led_value |= 0x0c;
                }
                else{
                    front_panel_led_value = front_panel_led_value;
                }
                if(dni_lock_cpld_write_attribute(SWPLD1_PATH,SYS_LED1_REGISTER,front_panel_led_value) != 0){
                    AIM_LOG_ERROR("Unable to set led(%d) status\r\n",local_id);
                    return ONLP_STATUS_E_INTERNAL;
                }
                break;
            case LED_FRONT_PWR1:
                front_panel_led_value = dni_lock_cpld_read_attribute(SWPLD1_PATH,SYS_LED1_REGISTER);
                front_panel_led_value &= ~0xc0;

                if(mode == ONLP_LED_MODE_GREEN){
                    front_panel_led_value |= 0x40;
                }
                else if(mode == ONLP_LED_MODE_RED){
                    front_panel_led_value |= 0x80;
                }
                else{
                    front_panel_led_value = front_panel_led_value;
                }
                if(dni_lock_cpld_write_attribute(SWPLD1_PATH,SYS_LED1_REGISTER,front_panel_led_value) != 0){
                    AIM_LOG_ERROR("Unable to set led(%d) status\r\n",local_id);
                    return ONLP_STATUS_E_INTERNAL;
                }
                break;
            case LED_FRONT_PWR2:
                front_panel_led_value = dni_lock_cpld_read_attribute(SWPLD1_PATH,SYS_LED1_REGISTER);
                front_panel_led_value &= ~0x30;

                if(mode == ONLP_LED_MODE_GREEN){
                    front_panel_led_value |= 0x10;
                }
                else if(mode == ONLP_LED_MODE_RED){
                    front_panel_led_value |= 0x20;
                }
                else{
                    front_panel_led_value = front_panel_led_value;
                }

                if(dni_lock_cpld_write_attribute(SWPLD1_PATH,SYS_LED1_REGISTER,front_panel_led_value) != 0){
                    AIM_LOG_ERROR("Unable to set led(%d) status\r\n",local_id);
                    return ONLP_STATUS_E_INTERNAL;
                }
                break;
            case LED_REAR_FAN_TRAY_1:
                fan_tray_led_reg_value = dni_lock_cpld_read_attribute(SWPLD1_PATH, SYS_LED3_REGISTER);
                fan_tray_led_reg_value &= ~0xc0;

                if(mode == ONLP_LED_MODE_GREEN){
                    fan_tray_led_reg_value |= 0x40;
                }
                else if(mode == ONLP_LED_MODE_RED){
                    fan_tray_led_reg_value |= 0x80;
                }
                else{
                    fan_tray_led_reg_value = fan_tray_led_reg_value;;
                }

                if(dni_lock_cpld_write_attribute(SWPLD1_PATH, SYS_LED3_REGISTER, fan_tray_led_reg_value) != 0){
                    AIM_LOG_ERROR("Unable to set led(%d) status\r\n",local_id);
                    return ONLP_STATUS_E_INTERNAL;
                }
                break;
            case LED_REAR_FAN_TRAY_2:
                fan_tray_led_reg_value = dni_lock_cpld_read_attribute(SWPLD1_PATH, FAN_LED_REGISTER);
                fan_tray_led_reg_value &= ~0x03;

                if(mode == ONLP_LED_MODE_GREEN){
                    fan_tray_led_reg_value |= 0x01;
                }
                else if(mode == ONLP_LED_MODE_RED){
                    fan_tray_led_reg_value |= 0x02;
                }
                else{
                    fan_tray_led_reg_value = fan_tray_led_reg_value;;
                }

                if(dni_lock_cpld_write_attribute(SWPLD1_PATH, FAN_LED_REGISTER, fan_tray_led_reg_value) != 0){
                    AIM_LOG_ERROR("Unable to set led(%d) status\r\n",local_id);
                    return ONLP_STATUS_E_INTERNAL;
                }
                break;
            case LED_REAR_FAN_TRAY_3:
                fan_tray_led_reg_value = dni_lock_cpld_read_attribute(SWPLD1_PATH, FAN_LED_REGISTER);
                fan_tray_led_reg_value &= ~0x0c;

                if(mode == ONLP_LED_MODE_GREEN){
                    fan_tray_led_reg_value |= 0x04;
                }
                else if(mode == ONLP_LED_MODE_RED){
                    fan_tray_led_reg_value |= 0x08;
                }
                else{
                    fan_tray_led_reg_value = fan_tray_led_reg_value;;
                }

                if(dni_lock_cpld_write_attribute(SWPLD1_PATH, FAN_LED_REGISTER, fan_tray_led_reg_value) != 0){
                    AIM_LOG_ERROR("Unable to set led(%d) status\r\n",local_id);
                    return ONLP_STATUS_E_INTERNAL;
                }
                break;
            case LED_REAR_FAN_TRAY_4:
                fan_tray_led_reg_value = dni_lock_cpld_read_attribute(SWPLD1_PATH, FAN_LED_REGISTER);
                fan_tray_led_reg_value &= ~0x30;

                if(mode == ONLP_LED_MODE_GREEN){
                    fan_tray_led_reg_value |= 0x10;
                }
                else if(mode == ONLP_LED_MODE_RED){
                    fan_tray_led_reg_value |= 0x20;
                }
                else{
                    fan_tray_led_reg_value = fan_tray_led_reg_value;;
                }

                if(dni_lock_cpld_write_attribute(SWPLD1_PATH, FAN_LED_REGISTER, fan_tray_led_reg_value) != 0){
                    AIM_LOG_ERROR("Unable to set led(%d) status\r\n",local_id);
                    return ONLP_STATUS_E_INTERNAL;
                }
                break;
            case LED_REAR_FAN_TRAY_5:
                fan_tray_led_reg_value = dni_lock_cpld_read_attribute(SWPLD1_PATH, FAN_LED_REGISTER);
                fan_tray_led_reg_value &= ~0xc0;

                if(mode == ONLP_LED_MODE_GREEN){
                    fan_tray_led_reg_value |= 0x40;
                }
                else if(mode == ONLP_LED_MODE_RED){
                    fan_tray_led_reg_value |= 0x80;
                }
                else{
                    fan_tray_led_reg_value = fan_tray_led_reg_value;;
                }

                if(dni_lock_cpld_write_attribute(SWPLD1_PATH, FAN_LED_REGISTER, fan_tray_led_reg_value) != 0){
                    AIM_LOG_ERROR("Unable to set led(%d) status\r\n",local_id);
                    return ONLP_STATUS_E_INTERNAL;
                }
                break;
        }
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

