/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 (C) Delta Networks, Inc.
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
#include <unistd.h>
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/platformi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_delta_ag9032v1_int.h"
#include "x86_64_delta_ag9032v1_log.h"
#include "platform_lib.h"

// U57 , U334 ,U38,U40 ,U240
static thermal_fan_t thermal_level[5]={
    { {200,64,60,56,52,45},{65,58,54,50,46,0},9},
    { {200,64,60,56,52,45},{65,58,54,50,46,0},9},
    { {200,79,75,71,67,63},{82,73,69,65,61,0},9},
    { {200,69,65,61,57,53},{71,63,59,55,51,0},9},
    { {200,54,49,44,39,39},{55,46,41,36,0,0},9}
};

const char*
onlp_platformi_get(void)
{
    return "x86-64-delta-ag9032v1-r0";
}

int
onlp_platformi_sw_init(void)
{
    lockinit();
    return ONLP_STATUS_OK;
}

int
onlp_platformi_manage_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_platformi_manage_fans(void)
{
     int i=0,j=0;
     int highest_temp = 0;
     int current_temp = 0;
     int highest_lv = LEVEL_6;
     onlp_thermal_info_t thermal[8];
     int new_duty_percentage;
    /* Get current temperature
     */
    if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), &thermal[0]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_CPU_BOARD), &thermal[1]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_FAN_BOARD), &thermal[2]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_SW_BOARD), &thermal[3]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_SW_BOARD), &thermal[4]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_SW_BOARD), &thermal[5]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), &thermal[6]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), &thermal[7]) != ONLP_STATUS_OK )
    {
        /* Setting all fans speed to maximum */
        new_duty_percentage = SPEED_100_PERCENTAGE;
        for(i = 1 ; i <= 12; i++)
        {
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), new_duty_percentage);
        }
        AIM_LOG_ERROR("Unable to read thermal status");
        return ONLP_STATUS_E_INTERNAL;
    }

    for(i=0;i<5;i++){
        current_temp=thermal[i+1].mcelsius/1000;
        if(thermal_level[i].current_lv == 9){ // initialize the temp level
            thermal_level[i].current_lv = LEVEL_4;
        }
        j=thermal_level[i].current_lv;
        if(current_temp < thermal_level[i].temp_L[j] && thermal_level[i].current_lv < LEVEL_6 ){ // goto lower level
            thermal_level[i].current_lv++;
        }
        else if(current_temp > thermal_level[i].temp_H[j] && thermal_level[i].current_lv > LEVEL_1 ){    // goto upper level
            thermal_level[i].current_lv--;
        }
        else{      // keep in curent level
            // keep in the curent level and
        }
        if(highest_lv > thermal_level[i].current_lv)
            highest_lv = thermal_level[i].current_lv;  //update the highest level
    }

    switch (highest_lv)
    {
        case LEVEL_1:
        case LEVEL_2:
            new_duty_percentage = SPEED_100_PERCENTAGE;
            break;
        case LEVEL_3:
            new_duty_percentage = SPEED_80_PERCENTAGE;
            break;
        case LEVEL_4:
            new_duty_percentage = SPEED_60_PERCENTAGE;
            break;
        case LEVEL_5:
            new_duty_percentage = SPEED_40_PERCENTAGE;
            break;
        case LEVEL_6:
            new_duty_percentage = SPEED_30_PERCENTAGE;
            break;
         default:
            new_duty_percentage = SPEED_100_PERCENTAGE;
            break;

    }
    /* Set speed on fan 1-10*/
    for(i = 1 ; i <= 10; i++)
    {
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), new_duty_percentage);
    }

    /*Set fans' speed of PSU 1, 2
     */
    if(highest_temp >= 0 && highest_temp <= 55)
    {
        new_duty_percentage = SPEED_60_PERCENTAGE;
    }
    else if(highest_temp > 55)
    {
        new_duty_percentage = SPEED_100_PERCENTAGE;
    }
    else
    {
        new_duty_percentage = SPEED_100_PERCENTAGE;
        AIM_LOG_ERROR("Unable to get thermal temperature");
    }
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU1) , new_duty_percentage);
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU2) , new_duty_percentage);

    return ONLP_STATUS_OK;
}

int
onlp_platformi_manage_leds(void)
{
    /* Set front lights: fan, power supply 1, 2
     */
    int  rpm, rpm1,i=0,count=0, state;
    uint8_t present_bit = 0x00;
    uint8_t power_state;
    mux_info_t mux_info;
    dev_info_t dev_info;

    mux_info.offset = SWPLD_PSU_FAN_I2C_MUX_REG;
    mux_info.channel = 0x07;
    mux_info.flags = DEFAULT_FLAG;

    dev_info.bus = I2C_BUS_3;
    dev_info.addr = FAN_IO_CTL;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;
    dev_info.size = 1;

    present_bit = dni_i2c_lock_read(&mux_info, &dev_info);
    /* Fan tray 1 */
    rpm = dni_i2c_lock_read_attribute(NULL, FAN5_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN5_REAR);
    if((present_bit & (1 << 4)) == 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
    {/* Green light */

            onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1),ONLP_LED_MODE_GREEN);
    }
    else
    {/* Red light */

        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1),ONLP_LED_MODE_RED);
    }

    /* Fan tray 2 */
    rpm = dni_i2c_lock_read_attribute(NULL, FAN4_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN4_REAR);
    if((present_bit & (1 << 3)) == 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
    {/* Green light */

            onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2),ONLP_LED_MODE_GREEN);
    }
    else
    {/* Red light */

        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2),ONLP_LED_MODE_RED);
    }

    /* Fan tray 3 */
    rpm = dni_i2c_lock_read_attribute(NULL, FAN3_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN3_REAR);
    if((present_bit & (1 << 2)) == 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
    {/* Green light */

            onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3),ONLP_LED_MODE_GREEN);
    }
    else
    {/* Red light */

        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3),ONLP_LED_MODE_RED);
    }

    /* Fan tray 4 */
    rpm = dni_i2c_lock_read_attribute(NULL, FAN2_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN2_REAR);
    if((present_bit & (1 << 1)) == 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
    {/* Green light */

            onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4),ONLP_LED_MODE_GREEN);
    }
    else
    {/* Red light */

        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4),ONLP_LED_MODE_RED);
    }

    /* Fan tray 5 */
    rpm = dni_i2c_lock_read_attribute(NULL, FAN1_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN1_REAR);
    if((present_bit & 1) == 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
    {/* Green light */

            onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_5),ONLP_LED_MODE_GREEN);
    }
    else
    {/* Red light */

        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_5),ONLP_LED_MODE_RED);
    }

        /* FRONT FAN LED & SYS */
    for(i = 0; i < 5; i++)
    {
        //mux_info.channel = i;
        //dev_info.addr = FAN_TRAY_1 + i;
        //fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
        //      if( fantray_present >= 0)
        present_bit = dni_i2c_lock_read(&mux_info, &dev_info);
        if( (present_bit & (1 << i)) == 0)
            count++;
    }
                /* Set front light of FAN */
    if(count == ALL_FAN_TRAY_EXIST && dni_fan_speed_good() == FAN_SPEED_NORMALLY)
    {
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN), ONLP_LED_MODE_GREEN);
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_SYS), ONLP_LED_MODE_GREEN);
    }
    else
    {

        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN), ONLP_LED_MODE_ORANGE);
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_SYS), ONLP_LED_MODE_RED);
    }
    /* Set front light of PWR1 */
    dev_info.bus = I2C_BUS_4;
    dev_info.addr = PSU_EEPROM;
    mux_info.channel = 0x00;
    state = dni_i2c_lock_read(&mux_info, &dev_info);

    /* Check the state of PSU 1, "state = 1, PSU exists' */
    if(state > 0)
    {
        power_state = dni_lock_swpld_read_attribute(CTL_REG);
        /* Set the light of PSU */
        if((power_state&0x80) != 0x80)
        {
         /* Red */
         onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR1), ONLP_LED_MODE_ORANGE_BLINKING);
         }
         else if((power_state & 0x80) == 0x80)
         {
         /* Green */
         onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR1), ONLP_LED_MODE_GREEN);
          }
     }
     else
          onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR1), ONLP_LED_MODE_OFF);
        /* Set front light of PWR1 */
    dev_info.bus = I2C_BUS_4;
    dev_info.addr = PSU_EEPROM;
    mux_info.channel = 0x20;
    state = dni_i2c_lock_read(&mux_info, &dev_info);

    /* Check the state of PSU 2, "state = 1, PSU exists' */
    if(state > 0)
    {
        power_state = dni_lock_swpld_read_attribute(CTL_REG);
        /* Set the light of PSU */
        if((power_state&0x40) != 0x40)
        {
         /* Red */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR2), ONLP_LED_MODE_ORANGE_BLINKING);
        }
        else if((power_state & 0x40) == 0x40)
        {
        /* Green */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR2), ONLP_LED_MODE_GREEN);
        }
    }
    else
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR2), ONLP_LED_MODE_OFF);


    return ONLP_STATUS_OK;
}
