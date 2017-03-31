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
#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlplib/mmap.h>
#include <onlplib/i2c.h>
#include "platform_lib.h"

#define ALL_FAN_TRAY_EXIST 4
#define PREFIX_PATH_ON_BOARD "/sys/bus/i2c/devices/"

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
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_SYS), "FRONT LED  (SYS LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_PWR), "FRONT LED  (PWR LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1), "REAR LED (FAN TRAY 1)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2), "REAR LED (FAN TRAY 2)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3), "REAR LED (FAN TRAY 3)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4), "REAR LED (FAN TRAY 4)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_AUTO,
    }
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
    int r_data = 0, r_data1 = 0, fantray_present = -1;
    VALIDATE(id);
    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];
    
    mux_info_t mux_info;
    mux_info.bus = I2C_BUS_5;
    mux_info.addr = SWPLD;
    mux_info.offset = FAN_MUX_REG;
    mux_info.channel = 0x07;
    mux_info.flags = DEFAULT_FLAG;

    dev_info_t dev_info;
    dev_info.bus = I2C_BUS_3;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;

    /* Set front panel's  mode of leds */
    r_data = dni_lock_cpld_read_attribute(SWPLD_PATH, LED_REG);
    r_data1 = dni_lock_cpld_read_attribute(SWPLD_PATH, FAN_TRAY_LED_REG);
    int local_id = ONLP_OID_ID_GET(id);
    switch(local_id)
    {
        case LED_FRONT_FAN:
            if((r_data & 0x02) == 0x02)
	        info->mode = ONLP_LED_MODE_GREEN;
	    else if((r_data & 0x01) == 0x01)
	        info->mode = ONLP_LED_MODE_ORANGE;
            break;
        case LED_FRONT_SYS:
            if((r_data & 0x10) == 0x10)
	        info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0x20) == 0x20)
	        info->mode = ONLP_LED_MODE_ORANGE;
	    else
		return ONLP_STATUS_E_INTERNAL;
            break;
        case LED_FRONT_PWR:
            if((r_data & 0x08) == 0x08)
	        info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0x04) == 0x04)
	        info->mode = ONLP_LED_MODE_ORANGE;
	    else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_1:
            mux_info.channel= 0x00;
            dev_info.addr = FAN_TRAY_1;
            dev_info.bus = I2C_BUS_3;
            fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
            if(fantray_present >= 0)
            {
                if((r_data1 & 0x40) == 0x40)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data1 & 0x80) == 0x80)
                    info->mode = ONLP_LED_MODE_ORANGE;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_2:
            mux_info.channel= 0x01;
            dev_info.addr = FAN_TRAY_2;
            dev_info.bus = I2C_BUS_3;
            fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
            if(fantray_present >= 0)
            {
                if((r_data1 & 0x10) == 0x10)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data1 & 0x20) == 0x20)
                    info->mode = ONLP_LED_MODE_ORANGE;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_3:
            mux_info.channel= 0x02;
            dev_info.addr = FAN_TRAY_3;
            dev_info.bus = I2C_BUS_3;
            fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
            if(fantray_present >= 0)
            {
            if((r_data1 & 0x04) == 0x04)
                info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data1 & 0x08) == 0x08)
                info->mode = ONLP_LED_MODE_ORANGE;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_4:
            mux_info.channel= 0x03;
            dev_info.addr = FAN_TRAY_4;
            dev_info.bus = I2C_BUS_3;
            fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
            if(fantray_present >= 0)
            {
                if((r_data1 & 0x01) == 0x01)
                    info->mode = ONLP_LED_MODE_GREEN;
                else if((r_data1 & 0x02) == 0x02)
                    info->mode = ONLP_LED_MODE_ORANGE;
            }
            else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        
        default:
            break;
    }

    /* Set the on/off status */
    if (info->mode == ONLP_LED_MODE_OFF) {
        info->status |= ONLP_LED_STATUS_FAILED;
    } else {
	    info->status |=ONLP_LED_STATUS_PRESENT;
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
    int i = 0, count = 0 ;
    int fantray_present = -1 ,rpm = 0,rpm1 = 0;
    uint8_t front_panel_led_value, fan_tray_led_value, power_state;
    

    mux_info_t mux_info;
    mux_info.bus = I2C_BUS_5;
    mux_info.addr = SWPLD;
    mux_info.offset = FAN_MUX_REG;
    mux_info.channel = 0x07;
    mux_info.flags = DEFAULT_FLAG;

    dev_info_t dev_info;
    dev_info.bus = I2C_BUS_3;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;


    front_panel_led_value = dni_lock_cpld_read_attribute(SWPLD_PATH,LED_REG);
    fan_tray_led_value = dni_lock_cpld_read_attribute(SWPLD_PATH,FAN_TRAY_LED_REG);
    switch(local_id)
    {
        case LED_FRONT_FAN:
            /* Clean the bit 1,0 */
            front_panel_led_value &= ~0x3;
            /* Read fan eeprom to check present */
            for(i = 0;i < 4; i++)
            {
                mux_info.channel = i;
                /* FAN TRAT 1~4: 0x52 , 0x53, 0x54, 0x55 */
                dev_info.addr = FAN_TRAY_1 + i;
                fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
                if( fantray_present >= 0 )
                    count++;
            }
            /* Set front light of FAN */
            if(count == ALL_FAN_TRAY_EXIST)
            {
                front_panel_led_value|=0x02;
                dni_lock_cpld_write_attribute(SWPLD_PATH, LED_REG, front_panel_led_value);
            }
            else
            {
                front_panel_led_value|=0x01;
                dni_lock_cpld_write_attribute(SWPLD_PATH, LED_REG, front_panel_led_value);
            }
    
            break;
        case LED_FRONT_PWR:
            /* Clean bit 3,2 */
            front_panel_led_value &= ~0x0C;
            /* switch CPLD to PSU 1 */
            dev_info.bus = I2C_BUS_4;
            dev_info.addr = PSU_EEPROM;
            mux_info.channel = 0x00;

            /* Check the state of PSU 1, "state = 1, PSU exists' */
            power_state = dni_lock_cpld_read_attribute(SWPLD_PATH, PSU_PWR_REG);
            /* Set the light of PSU */
            if((power_state&0x80) != 0x80)
            {
                /* Red */
                front_panel_led_value|=0x04;
                dni_lock_cpld_write_attribute(SWPLD_PATH, LED_REG, front_panel_led_value);
            }
            else if((power_state&0x80)==0x80)
            {
                /* Green */
                front_panel_led_value|=0x08;
                dni_lock_cpld_write_attribute(SWPLD_PATH, LED_REG, front_panel_led_value);
            }
            else
                dni_lock_cpld_write_attribute(SWPLD_PATH,LED_REG, front_panel_led_value);
            break;

        case LED_REAR_FAN_TRAY_1:
            mux_info.channel= 0x00;
            dev_info.addr = FAN_TRAY_1;
            dev_info.bus = I2C_BUS_3;
            fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
            rpm = dni_i2c_lock_read_attribute(NULL, FAN4_FRONT);
            rpm1 = dni_i2c_lock_read_attribute(NULL, FAN4_REAR);
            fan_tray_led_value &= ~0xC0;
            if(fantray_present >= 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
            {
                /* Green */
                fan_tray_led_value |=0x40;
                dni_lock_cpld_write_attribute(SWPLD_PATH,FAN_TRAY_LED_REG,fan_tray_led_value);
            }
            else
            {
                /* Red */
                fan_tray_led_value |=0x80;
                dni_lock_cpld_write_attribute(SWPLD_PATH,FAN_TRAY_LED_REG,fan_tray_led_value);
            }
            break;
        case LED_REAR_FAN_TRAY_2:
            mux_info.channel= 0x01;
            dev_info.addr = FAN_TRAY_2;
            dev_info.bus = I2C_BUS_3;
            fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
            rpm = dni_i2c_lock_read_attribute(NULL, FAN3_FRONT);
            rpm1 = dni_i2c_lock_read_attribute(NULL, FAN3_REAR);
            fan_tray_led_value &= ~0x30;

            if(fantray_present >= 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
            {
                /* Green */
                fan_tray_led_value |=0x10;
                dni_lock_cpld_write_attribute(SWPLD_PATH,FAN_TRAY_LED_REG,fan_tray_led_value);
            }
            else
            {
                /* Red */
                fan_tray_led_value |=0x20;
                dni_lock_cpld_write_attribute(SWPLD_PATH,FAN_TRAY_LED_REG,fan_tray_led_value);
            }
            break;
        case LED_REAR_FAN_TRAY_3:
            mux_info.channel= 0x02;
            dev_info.bus = I2C_BUS_3;
            dev_info.addr = FAN_TRAY_3;
            fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
            rpm = dni_i2c_lock_read_attribute(NULL, FAN2_FRONT);
            rpm1 = dni_i2c_lock_read_attribute(NULL, FAN2_REAR);
            fan_tray_led_value &= ~0x0c;
            if(fantray_present >= 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
            {
                /* Green */
                fan_tray_led_value |=0x04;
                dni_lock_cpld_write_attribute(SWPLD_PATH,FAN_TRAY_LED_REG,fan_tray_led_value);
            }
            else
            {
                /* Red */
                fan_tray_led_value |=0x08;
                dni_lock_cpld_write_attribute(SWPLD_PATH,FAN_TRAY_LED_REG,fan_tray_led_value);
            }
            break;
        case LED_REAR_FAN_TRAY_4:
            mux_info.channel= 0x03;
            dev_info.addr = FAN_TRAY_4;
            dev_info.bus = I2C_BUS_3;
            fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
            rpm = dni_i2c_lock_read_attribute(NULL, FAN1_FRONT);
            rpm1 = dni_i2c_lock_read_attribute(NULL, FAN1_REAR);
            fan_tray_led_value &= ~0x03;
            if(fantray_present >= 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
            {
                /* Green */
                fan_tray_led_value |=0x01;
                dni_lock_cpld_write_attribute(SWPLD_PATH,FAN_TRAY_LED_REG,fan_tray_led_value);
            }
            else
            {
                /* Red */
                 fan_tray_led_value |=0x02;
                 dni_lock_cpld_write_attribute(SWPLD_PATH,FAN_TRAY_LED_REG,fan_tray_led_value);
             }
             break;
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



