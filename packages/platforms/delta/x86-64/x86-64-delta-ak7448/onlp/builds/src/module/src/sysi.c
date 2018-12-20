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
#include <unistd.h>
#include <fcntl.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_delta_ak7448_int.h"
#include "x86_64_delta_ak7448_log.h"
#include "platform_lib.h"

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-delta-ak7448-r0";
}

int
onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

/******************* Utility Function *****************************************/
int
decide_percentage(int *percentage, int temper)
{
    int level;

    if(temper <= 50){
        *percentage = 50;
        level = 1;
    }
    else if(temper > 50 && temper <= 55){
        *percentage = 58;
        level = 2;
    }
    else if(temper > 55 && temper <= 60){
        *percentage = 65;
        level = 3;
    }
    else if(temper > 60 && temper <= 65){
        *percentage = 80;
        level = 4;
    }
     else if(temper > 65){
        *percentage = 100;
        level = 5;
    }
    else{
        *percentage = 100;
        level = 6;
    }
  
    return level;
}
/******************************************************************************/


int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);

    if(onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) {
        if(*size == 256) {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }

    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_UNSUPPORTED;
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{
    aim_free(data);
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    dev_info_t dev_info;
    int sys_cpld_version = 0, cpld_a_version = 0, cpld_b_version = 0;

    dev_info.bus = I2C_BUS_5;
    dev_info.addr = CPLD_A;
    dev_info.offset = CPLD_A_VERSION_REG;
    dev_info.size = 1;    
    dev_info.flags = ONLP_I2C_F_FORCE;
    cpld_a_version = dni_i2c_lock_read(NULL, &dev_info);

    dev_info.bus = I2C_BUS_2;
    dev_info.addr = SYS_CPLD;
    dev_info.offset = SYS_VERSION_REG;
    dev_info.size = 1;
    dev_info.flags = DEFAULT_FLAG;
    sys_cpld_version = dni_i2c_lock_read(NULL, &dev_info);

    cpld_b_version = (dni_lock_cpld_read_attribute(CPLD_B_PLATFORM_PATH,CPLD_B_VERSION_REG) & 0x7);

    pi->cpld_versions = aim_fstrdup("SYSTEM-CPLD = %d, CPLD-A = %d, CPLD-B = %d", sys_cpld_version, cpld_a_version, cpld_b_version);

    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 7 Thermal sensors on the chassis */
    for (i = 1; i <= NUM_OF_THERMAL_ON_BOARDS; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 6 LEDs on the chassis */
    for (i = 1; i <= NUM_OF_LED_ON_BOARDS; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 7 Fans on the chassis */
    for (i = 1; i <= NUM_OF_FAN_ON_FAN_BOARD; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 1 PSUs on the chassis */
    for (i = 1; i <= NUM_OF_PSU_ON_PSU_BOARD; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    return 0;
}

int
dni_sysi_set_fan(int new_percentage)
{
    int ret = 0;
    int i = 0;
    for(i = 1; i <= 6 ; i++)
    {
        ret = onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(i), MAX_FAN_SPEED  * new_percentage / 100);
        if(ret != ONLP_STATUS_OK)
        {
            AIM_LOG_ERROR("Unable to set fan(%d) rpm\r\n",ONLP_FAN_ID_CREATE(i));
            return ret;
        }
    }
    if(onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU1) , new_percentage) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("Unable to set fan(%d) percentage\r\n",ONLP_FAN_ID_CREATE(FAN_1_ON_PSU1));
        return ret;
    }
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_fans(void)
{
    int i = 0;
    int new_percentage;
    int highest_temp = 0;
    onlp_thermal_info_t thermal[NUM_OF_THERMAL_ON_BOARDS];

    for(i = 1; i <= NUM_OF_THERMAL_ON_BOARDS; i++)
    {
        //ret = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i),&thermal[i-1]);
        if(onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i),&thermal[i-1]) != ONLP_STATUS_OK)
        {
            /* Setting all fans speed to maximum */
            new_percentage = SPEED_100_PERCENTAGE;
            if(dni_sysi_set_fan(new_percentage) != ONLP_STATUS_OK)
            {
                AIM_LOG_ERROR("Unable to set fan\r\n");
                return ONLP_STATUS_E_INTERNAL;
            }
 
            AIM_LOG_ERROR("Unable to read status from thermal(%d)\r\n",i);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    

    for (i = 0; i < NUM_OF_THERMAL_ON_BOARDS; i++)
    {
        if (thermal[i].mcelsius > highest_temp)
        {
            highest_temp = thermal[i].mcelsius;
        }
    }

    highest_temp = highest_temp/1000;
    decide_percentage(&new_percentage, highest_temp);

    if(dni_sysi_set_fan(new_percentage) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("Unable to set fan\r\n");
    }
    
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    int fantray_present = -1, rpm = 0, rpm1 = 0, count = 0;
    int fantray_count;
    uint8_t psu_state; 
    mux_info_t mux_info;
    dev_info_t dev_info;

    mux_info.offset = FAN_I2C_MUX_SEL_REG;

    dev_info.bus = I2C_BUS_7;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;

    /* Fan tray 1 */
    dev_info.addr = FAN_TRAY;
    mux_info.channel = 0x02;
    fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
    mux_info.channel = FAN_I2C_SEL_FAN_CTRL;
    rpm = dni_i2c_lock_read_attribute(&mux_info, FAN1_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(&mux_info, FAN1_REAR);

    if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
    {
        /* Green */
        if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
    }
    else
    {
        /* Red */
        if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1),ONLP_LED_MODE_RED) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
    }

    /* Fan tray 2 */
    dev_info.addr = FAN_TRAY;
    mux_info.channel = 0x01;
    fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
    mux_info.channel = FAN_I2C_SEL_FAN_CTRL;
    rpm = dni_i2c_lock_read_attribute(&mux_info, FAN2_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(&mux_info, FAN2_REAR);

    if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
    {
        /* Green */
        if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
    }
    else
    {
        /* Red */
        if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2),ONLP_LED_MODE_RED) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
    }

    /* Fan tray 3 */
    dev_info.addr = FAN_TRAY;
    mux_info.channel = 0x00;
    fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
    mux_info.channel = FAN_I2C_SEL_FAN_CTRL;
    rpm = dni_i2c_lock_read_attribute(&mux_info, FAN3_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(&mux_info, FAN3_REAR);

    if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
    {
        /* Green */
        if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
    }
    else
    {
        /* Red */
        if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3),ONLP_LED_MODE_RED) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
    }

    /* FRONT FAN & SYS LED */
    for(fantray_count = 0; fantray_count < 3; fantray_count++)
    {
        mux_info.channel = fantray_count;
        dev_info.addr = FAN_TRAY;
        fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
        if( fantray_present >= 0)
            count++;
    }

    if(count == ALL_FAN_TRAY_EXIST && dni_fan_speed_good() == FAN_SPEED_NORMALLY)
    {   
        /* Green FAN operates normally */
        if((onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK) ||
        (onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_SYS),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK))
            return ONLP_STATUS_E_INTERNAL;
    }
    else
    {
        /* Solid Amber FAN or more failed*/
        if((onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN),ONLP_LED_MODE_ORANGE) != ONLP_STATUS_OK) ||	
        (onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_SYS),ONLP_LED_MODE_ORANGE) !=  ONLP_STATUS_OK))
            return ONLP_STATUS_E_INTERNAL;
    }

    /* Set front light of PWR */
    mux_info.offset = PSU_I2C_MUX_SEL_REG;
    mux_info.channel = PSU_I2C_SEL_PSU_EEPROM;

    dev_info.bus = I2C_BUS_4;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;
    dev_info.addr = PSU_EEPROM;

    psu_state = dni_i2c_lock_read(&mux_info, &dev_info);

    if(psu_state == 1)
    {
        /* Green */
        if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_GREEN) !=  ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
    }	
    else
    {
        if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_OFF) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
    }
			
    return ONLP_STATUS_OK;
}

