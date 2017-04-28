/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017  Delta Networks, Inc. 
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
#include <onlplib/i2c.h>
#include "platform_lib.h"

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
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_FAN), "FRONT LED (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_SYS), "FRONT LED  (SYS LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_PWR), "FRONT LED  (PWR LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1), "FAN TRAY 1 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2), "FAN TRAY 2 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3), "FAN TRAY 3 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4), "FAN TRAY 4 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
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
    int r_data = 0, fantray_present = -1;
    VALIDATE(id);
    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    dev_info_t dev_info;
    dev_info.bus = I2C_BUS_3;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;

    /* Set front panel's  mode of leds */
    r_data = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,LED_REG); 
    int local_id = ONLP_OID_ID_GET(id);
    switch(local_id)
    {
        case LED_FRONT_FAN:
            if((r_data & 0xc0) == 0x80)
                info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0xc0) == 0x40)
                info->mode = ONLP_LED_MODE_ORANGE;
            else if((r_data & 0xc0) == 0xc0)
                info->mode = ONLP_LED_MODE_ORANGE_BLINKING;
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_FRONT_SYS:
            if((r_data & 0x30) == 0x10)
                   info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0x30) == 0x20)
                   info->mode = ONLP_LED_MODE_ORANGE;
            else if((r_data & 0x30) == 0x00)
                 info->mode = ONLP_LED_MODE_GREEN_BLINKING;
            else
                return ONLP_STATUS_E_INTERNAL;
            break;
        case LED_FRONT_PWR:
            if((r_data & 0x06) == 0x04)
                info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0x06) == 0x02)
                info->mode = ONLP_LED_MODE_ORANGE;
            else if((r_data & 0x06) == 0x06)
                info->mode = ONLP_LED_MODE_ORANGE_BLINKING;
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_1:
            dev_info.addr = FAN_TRAY_1;
            r_data = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG);
            fantray_present = dni_i2c_lock_read(NULL, &dev_info);
            if(fantray_present >= 0)
            {
                if((r_data & 0x01) == 0x01)
                    info->mode = ONLP_LED_MODE_GREEN;
                else 
                    info->mode = ONLP_LED_MODE_ORANGE;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_2:
            dev_info.addr = FAN_TRAY_2;
            r_data = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG);
            fantray_present = dni_i2c_lock_read(NULL, &dev_info);
            if(fantray_present >= 0)
            {
                if((r_data & 0x04) == 0x04)
                    info->mode = ONLP_LED_MODE_GREEN;
                else
                    info->mode = ONLP_LED_MODE_ORANGE;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_3:
            dev_info.addr = FAN_TRAY_3;
            r_data = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG);
            fantray_present = dni_i2c_lock_read(NULL, &dev_info);
            if(fantray_present >= 0)
            {
                if((r_data & 0x10) == 0x10)
                    info->mode = ONLP_LED_MODE_GREEN;
                else
                    info->mode = ONLP_LED_MODE_ORANGE;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_4:
            dev_info.addr = FAN_TRAY_4;
            r_data = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG);
            fantray_present = dni_i2c_lock_read(NULL, &dev_info);
            if(fantray_present >= 0)
            {
                if((r_data & 0x40) == 0x40)
                    info->mode = ONLP_LED_MODE_GREEN;
                else
                    info->mode = ONLP_LED_MODE_ORANGE;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        default:
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
    VALIDATE(id);
    if(on_or_off == 0)
        onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    else
        onlp_ledi_mode_set(id,ONLP_LED_MODE_AUTO);
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
    VALIDATE(id);
    int local_id = ONLP_OID_ID_GET(id);
    int i = 0, count = 0 ,fan_board_not_present_count = 0 , fan_stat2_reg_mask = 0 , fan_stat1_reg_mask = 0 ;
    int fantray_present = -1, rpm = 0, rpm1 = 0;
    uint8_t front_panel_led_value, power_state,fan_tray_led_reg_value, fan_led_status_value, fan_tray_pres_value;
    uint8_t psu1_state, psu2_state, alarm_reg_value, fan_tray_interface_detected_value;
    dev_info_t dev_info;
    dev_info.bus = I2C_BUS_3;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;

    front_panel_led_value = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,LED_REG);
    fan_tray_led_reg_value = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG);
    fan_led_status_value = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,FAN_STAT1_REG);
    fan_tray_pres_value = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,FAN_STAT2_REG);
    alarm_reg_value = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,ALARM_REG);
    
    switch(local_id)    
        {   
        case LED_FRONT_FAN: 
            /* Clean the bit 7,6 */
            front_panel_led_value &= ~0xC0;
            fan_board_not_present_count = 0;
            /* Read cpld fan status to check present. Fan tray 1-4 */            
            for(i = 0; i < 4; i++)
            {
                fan_stat2_reg_mask = 0x01 << i;         
                fan_stat1_reg_mask = 0x01 << (i * 2);   
                if((fan_tray_pres_value & fan_stat2_reg_mask) == fan_stat2_reg_mask)
                    fan_board_not_present_count++;
                else if((fan_led_status_value & fan_stat1_reg_mask) == fan_stat1_reg_mask)
                    count++;
            }
            /* Set front light of FAN */
            if(count == ALL_FAN_TRAY_EXIST)
            {
                front_panel_led_value |= 0x80;/*Solid green, FAN operates normally.*/
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,LED_REG, front_panel_led_value);
            }
            else if (fan_board_not_present_count > 0)
            {
                front_panel_led_value |= 0xc0;/*Blinking Yellow , FAN is failed */
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,LED_REG, front_panel_led_value);
            }
            else
            {
                front_panel_led_value |= 0x40;/*Solid Amber FAN operating is NOT present */
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,LED_REG, front_panel_led_value);
            }
            
            break;

        case LED_FRONT_PWR:
            /* Clean bit 2,1 */
            front_panel_led_value &= ~0x06;
            /* switch CPLD to PSU 1 */
            dev_info.bus = I2C_BUS_6;
            dev_info.addr = PSU1_EEPROM;
            psu1_state = dni_i2c_lock_read(NULL, &dev_info);
            /* switch CPLD to PSU 2 */
            dev_info.addr = PSU2_EEPROM;
            psu2_state = dni_i2c_lock_read(NULL, &dev_info);

            if(psu1_state == 1 && psu2_state == 1)
            {
                power_state = dni_lock_cpld_read_attribute(MASTER_CPLD_PATH,PSU_STAT_REG);
              
                if((power_state & 0x40) == 0x40 || (power_state & 0x04) == 0x04)               
                {
                    front_panel_led_value |= 0x06; /*Blinking Amber*/
                    dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,LED_REG, front_panel_led_value);
                }
                else
                {
                    front_panel_led_value |= 0x04;  /*Solid Green*/
                    dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,LED_REG, front_panel_led_value);
                }
            }
            else
                front_panel_led_value |= 0x02; /*Solid Amber*/
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,LED_REG, front_panel_led_value);
            break;
  
        case LED_FRONT_SYS:
            /* Clean bit 4,5 */
            front_panel_led_value &= ~0x30;
            fan_board_not_present_count = 0; 
            /* Read fan eeprom to check present */
            for(i = 0;i < 4; i++)
            {
                fan_stat2_reg_mask = 0x01 << i;         
                if((fan_tray_pres_value & fan_stat2_reg_mask) == fan_stat2_reg_mask)
                    fan_board_not_present_count++;
             }
            if(fan_board_not_present_count > 0 || (alarm_reg_value & 0xff) == 0xff)
            {
                fan_tray_interface_detected_value = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,INTERRUPT_REG);                 
                if(fan_tray_interface_detected_value == 0xfe || (alarm_reg_value & 0xff) == 0xff)
                {
                    front_panel_led_value |= 0x20;
                    dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH, LED_REG, front_panel_led_value);
                }
            }
            else
            {
                front_panel_led_value |= 0x10;
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH, LED_REG, front_panel_led_value);
            }
            break;
      
        case LED_REAR_FAN_TRAY_1:
            dev_info.addr = FAN_TRAY_1;
            fantray_present = dni_i2c_lock_read(NULL, &dev_info);
            rpm = dni_i2c_lock_read_attribute(NULL, FAN1_FRONT);
            rpm1 = dni_i2c_lock_read_attribute(NULL, FAN1_REAR);
            fan_tray_led_reg_value &= ~0x03;
            if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
            {
                fan_tray_led_reg_value |= 0x01;/*Solid Green*/
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            else
            {
                fan_tray_led_reg_value |= 0x02;/*Solid Amber*/
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            break;

        case LED_REAR_FAN_TRAY_2:
            dev_info.addr = FAN_TRAY_2;
            fantray_present = dni_i2c_lock_read(NULL, &dev_info);
            rpm = dni_i2c_lock_read_attribute(NULL, FAN2_FRONT);
            rpm1 = dni_i2c_lock_read_attribute(NULL, FAN2_REAR);
            fan_tray_led_reg_value &= ~0x0c;
           
            if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
            {
                fan_tray_led_reg_value |= 0x04;/*Solid Green*/
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            else
            {
                fan_tray_led_reg_value |= 0x08;/*Solid Amber*/
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            break;

        case LED_REAR_FAN_TRAY_3:
            dev_info.addr = FAN_TRAY_3;
            fantray_present = dni_i2c_lock_read(NULL, &dev_info);
            fan_tray_led_reg_value &= ~0x30;
            rpm = dni_i2c_lock_read_attribute(NULL, FAN3_FRONT);
            rpm1 = dni_i2c_lock_read_attribute(NULL, FAN3_REAR);
            if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
            {
                fan_tray_led_reg_value |= 0x10;/*Solid Green*/
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            else
            {
                fan_tray_led_reg_value |= 0x20;/*Solid Amber*/
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            break;

        case LED_REAR_FAN_TRAY_4:
            dev_info.addr = FAN_TRAY_4;
            fantray_present = dni_i2c_lock_read(NULL, &dev_info);
            fan_tray_led_reg_value &= ~0xc0;
            rpm = dni_i2c_lock_read_attribute(NULL, FAN4_FRONT);
            rpm1 = dni_i2c_lock_read_attribute(NULL, FAN4_REAR);
            if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm !=0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
            {
                fan_tray_led_reg_value |= 0x40; /*Solid Green*/
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            else
            {
                fan_tray_led_reg_value |= 0x80;/*Solid Amber*/
                dni_lock_cpld_write_attribute(SLAVE_CPLD_PATH,FAN_TRAY_LED_REG, fan_tray_led_reg_value);
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
