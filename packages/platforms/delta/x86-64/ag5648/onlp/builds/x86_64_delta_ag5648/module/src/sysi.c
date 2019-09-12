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

#include "x86_64_delta_ag5648_int.h"
#include "x86_64_delta_ag5648_log.h"
#include "platform_lib.h"

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-delta-ag5648-r0";
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

    if(temper <= 50)
    {
        *percentage = 40;
        level = 1;
    }
    else if(temper > 50 && temper <= 55)
    {
        *percentage = 60;
        level = 2;
    }
    else if(temper > 55 && temper <= 60)
    {
        *percentage = 80;
        level = 3;
    }
    else if(temper > 60 && temper <= 65)
    {
        *percentage = 90;
        level = 4;
    }
     else if(temper > 65)
    {
        *percentage = 100;
        level = 5;
    }
    else
    {
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
    int sys_cpld_version = 0 , master_cpld_version = 0 ,slave_cpld_version = 0 ;

    sys_cpld_version = dni_lock_cpld_read_attribute(SYS_CPLD_PATH,SYS_VERSION_REG);
    master_cpld_version = dni_lock_cpld_read_attribute(MASTER_CPLD_PATH,MASTER_VERSION_REG);
    slave_cpld_version = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,SLAVE_VERSION_REG);

    pi->cpld_versions = aim_fstrdup("SYSTEM-CPLD = %d, MASTER-CPLD = %d, SLAVE-CPLD = %d", sys_cpld_version, master_cpld_version, slave_cpld_version);

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

    /* 9 Thermal sensors on the chassis */
    for (i = 1; i <= NUM_OF_THERMAL_ON_BOARDS; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 7 LEDs on the chassis */
    for (i = 1; i <= NUM_OF_LED_ON_BOARDS; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 8 Fans on the chassis */
    for (i = 1; i <= NUM_OF_FAN_ON_FAN_BOARD; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= NUM_OF_PSU_ON_PSU_BOARD; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_manage_fans(void)
{
    int i = 0;
    int new_percentage;
    int highest_temp = 0;
    onlp_thermal_info_t thermal[NUM_OF_THERMAL_ON_BOARDS];
    /* Get current temperature */
    if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE),        &thermal[0]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_CPU_BOARD),  &thermal[1]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_FAN_BOARD),  &thermal[2]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BOARD), &thermal[3]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BOARD), &thermal[4]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BOARD), &thermal[5]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BOARD), &thermal[6]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1),       &thermal[7]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2),       &thermal[8]) != ONLP_STATUS_OK  
       )
    {
            /* Setting all fans speed to maximum */
            new_percentage = SPEED_100_PERCENTAGE;
            onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_1_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED  * new_percentage / 100);
            onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_2_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED  * new_percentage / 100);
            onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_3_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED  * new_percentage / 100);
            onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_4_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED  * new_percentage / 100);
            onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_5_ON_FAN_BOARD), MAX_REAR_FAN_SPEED  * new_percentage / 100);
            onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_6_ON_FAN_BOARD), MAX_REAR_FAN_SPEED  * new_percentage / 100);
            onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_7_ON_FAN_BOARD), MAX_REAR_FAN_SPEED  * new_percentage / 100);
            onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_8_ON_FAN_BOARD), MAX_REAR_FAN_SPEED  * new_percentage / 100);
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU1) , new_percentage);
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU2) , new_percentage);
                                                                                                            
            AIM_LOG_ERROR("Unable to read thermal status");
            return ONLP_STATUS_E_INTERNAL;
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

    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_1_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_2_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_3_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_4_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_5_ON_FAN_BOARD), MAX_REAR_FAN_SPEED  * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_6_ON_FAN_BOARD), MAX_REAR_FAN_SPEED  * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_7_ON_FAN_BOARD), MAX_REAR_FAN_SPEED  * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_8_ON_FAN_BOARD), MAX_REAR_FAN_SPEED  * new_percentage / 100);

    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU1) , new_percentage);
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU2) , new_percentage);
    
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    int fantray_present = -1, rpm = 0, rpm1 = 0;
    uint8_t psu1_state, psu2_state, power_state, fan_tray_interface_detected_value;
    int fan_tray_pres_value, fan_board_not_present_count,i ,fan_stat_reg_mask;


    dev_info_t dev_info;
    dev_info.bus = I2C_BUS_3;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;

    /* Fan tray 1 */
    dev_info.addr = FAN_TRAY_1;
    fantray_present = dni_i2c_lock_read(NULL, &dev_info);
    rpm = dni_i2c_lock_read_attribute(NULL, FAN1_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN1_REAR);
    if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
    {
        /* Green */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1),ONLP_LED_MODE_GREEN);
    }
    else
    {
        /* Red */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1),ONLP_LED_MODE_RED);
    }

    /* Fan tray 2 */
    dev_info.addr = FAN_TRAY_2;
    fantray_present = dni_i2c_lock_read(NULL, &dev_info);
    rpm = dni_i2c_lock_read_attribute(NULL, FAN2_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN2_REAR);
    if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
    {
        /* Green */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2),ONLP_LED_MODE_GREEN);
    }
    else
    {
        /* Red */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2),ONLP_LED_MODE_RED);
    }

    /* Fan tray 3 */
    dev_info.addr = FAN_TRAY_3;
    fantray_present = dni_i2c_lock_read(NULL, &dev_info);
    rpm = dni_i2c_lock_read_attribute(NULL, FAN3_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN3_REAR);
    if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
    {
        /* Green */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3),ONLP_LED_MODE_GREEN);
    }
    else
    {
        /* Red */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3),ONLP_LED_MODE_RED);
    }

    /* Fan tray 4 */
    dev_info.addr = FAN_TRAY_4;
    fantray_present = dni_i2c_lock_read(NULL, &dev_info);
    rpm = dni_i2c_lock_read_attribute(NULL, FAN4_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN4_REAR);
    if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
    {
        /* Green */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4),ONLP_LED_MODE_GREEN);
    }
    else
    {
        /* Red */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4),ONLP_LED_MODE_RED);
    }

    /* FRONT FAN & SYS LED */
    fan_tray_pres_value = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,FAN_STAT2_REG);
	  fan_board_not_present_count = 0;
	
    for(i = 0;i < 4; i++)
    {
        fan_stat_reg_mask = 0x01 << i;
        if((fan_tray_pres_value & fan_stat_reg_mask) == fan_stat_reg_mask)
            fan_board_not_present_count++;
    }
	
    if(fan_board_not_present_count == 0 && dni_fan_speed_good() == FAN_SPEED_NORMALLY)
    {   
        /* Green FAN operates normally */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN),ONLP_LED_MODE_GREEN);
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_SYS),ONLP_LED_MODE_GREEN);
    }
    else
    {
        /* Solid Amber FAN or more failed*/
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN),ONLP_LED_MODE_ORANGE);	
        fan_tray_interface_detected_value = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,INTERRUPT_REG);
	
        if(fan_tray_interface_detected_value == 0xfe || (fan_tray_pres_value & 0x10) != 0x10)
        {
            onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_SYS),ONLP_LED_MODE_ORANGE);
        }
    }

    /* Set front light of PWR */
    dev_info.bus = I2C_BUS_6;
    dev_info.addr = PSU1_EEPROM;
    psu1_state = dni_i2c_lock_read(NULL, &dev_info);
 
    dev_info.addr = PSU2_EEPROM;
    psu2_state = dni_i2c_lock_read(NULL, &dev_info);
    power_state = dni_lock_cpld_read_attribute(MASTER_CPLD_PATH,PSU_STAT_REG);

    if( psu1_state == 1 && psu2_state == 1 && (power_state & 0x22) == 0x22 )
    {
        /* Green */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_GREEN);
    }	
	  else if((power_state & 0x42) == 0x42 || (power_state & 0x24) == 0x24)
    {			
         /* Blinking Amber */			
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_ORANGE_BLINKING);
    }
    else if ( ( power_state & 0x42 ) != 0x42 || ( power_state & 0x24 ) != 0x24 )
	  {
	    /* Red */  
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_ORANGE);
    }
	  else 
		onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_OFF);
			
    return ONLP_STATUS_OK;
}

