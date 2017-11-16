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
    /* Set front lights: fan, power supply 1, 2*/
    uint8_t addr, present_bit = 0x00;

    addr = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,LED_REG);
    /* Turn the fan led on or off */
    if((addr & 0xc0) == 0 ) 
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN), TURN_OFF);
    }
    else
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN), TURN_ON);
    }
    /* Set front light of SYS  */
    addr = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,LED_REG);

    if((addr & 0x30) == 0x30)
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_SYS), TURN_OFF);
    }
    else
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_SYS), TURN_ON);
    }

    /* Set front light of PSU  */
    addr = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,LED_REG);
    
    if((addr & 0x06) == 0x00)
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), TURN_OFF);
    }
    else
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), TURN_ON);
    }
 
    /* Turn on or off the FAN tray leds */
    present_bit = dni_lock_cpld_read_attribute(SLAVE_CPLD_PATH,FAN_STAT2_REG);
    if((present_bit & 0x01) == 0x00)
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1), TURN_OFF);
    }
    else
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1), TURN_ON);
    }
    if((present_bit & 0x02) == 0x00)
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2), TURN_OFF);
    }
    else
    {    
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2), TURN_ON);
    }
    if((present_bit & 0x04) == 0x00)
    {    
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3), TURN_OFF);
    }
    else
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3), TURN_ON);
    }
    if((present_bit & 0x08) == 0x00)
    {    
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4), TURN_OFF);
    }
    else
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4), TURN_ON);
    }
    return ONLP_STATUS_OK;
}

