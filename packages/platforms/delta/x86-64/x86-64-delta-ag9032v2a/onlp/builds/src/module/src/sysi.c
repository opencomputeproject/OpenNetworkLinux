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
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/sysi.h>
#include "x86_64_delta_ag9032v2a_int.h"
#include "x86_64_delta_ag9032v2a_log.h"
#include "platform_lib.h"
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

int
decide_percentage(int *percentage, int temper)
{
    int level;

    if(temper <= 50)
    {
        *percentage = 50;
        level = 1;
    }
    else if(temper > 50 && temper <= 55)
    {
        *percentage = 58;
        level = 2;
    }
    else if(temper > 55 && temper <= 60)
    {
        *percentage = 65;
        level = 3;
    }
    else if(temper > 60 && temper <= 65)
    {
        *percentage = 80;
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

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-delta-ag9032v2a-r0";
}

int
onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);

    if(onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) 
    {
        if(*size == 256) 
        {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }

    aim_free(rdata);
    *size = 0;

    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int cpld_version = 0;
    cpld_version = dni_i2c_lock_read_attribute(NULL, CPU_CPLD_VERSION);
    pi->cpld_versions = aim_fstrdup("%d", cpld_version);
    
    return ONLP_STATUS_OK;
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{
    aim_free(data);
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i = 0;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    for (i = 1; i <= NUM_OF_THERMAL_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    for (i = 1; i <= NUM_OF_LED_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    for (i = 1; i <= NUM_OF_PSU_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return ONLP_STATUS_OK;
}
int
onlp_sysi_platform_manage_fans(void)
{
    int i, new_percentage, highest_temp = 0;
    onlp_thermal_info_t thermal;

    /* Get all thermal current temperature and decide fan percentage */
    for(i = 1; i <= NUM_OF_THERMAL; ++i)
    {
        if(onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i), &thermal) != ONLP_STATUS_OK)
        {
            AIM_LOG_ERROR("Unable to read thermal status");
            return ONLP_STATUS_E_INTERNAL;
        }

        thermal.mcelsius /= 1000;
        if(thermal.mcelsius > highest_temp)
        {
            highest_temp = thermal.mcelsius;
        }

        decide_percentage(&new_percentage, highest_temp);
    }

    /* Set fantray RPM and PSU fan percentage */
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_1_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_2_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_3_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_4_ON_FAN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_5_ON_FAN_BOARD), MAX_REAR_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_6_ON_FAN_BOARD), MAX_REAR_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_7_ON_FAN_BOARD), MAX_REAR_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_8_ON_FAN_BOARD), MAX_REAR_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_9_ON_FAN_BOARD), MAX_REAR_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_10_ON_FAN_BOARD), MAX_REAR_FAN_SPEED * new_percentage / 100);
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU1), new_percentage);
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU2), new_percentage);

    return ONLP_STATUS_OK;
}
int
onlp_sysi_platform_manage_leds(void)
{
    int fantray_present = -1, rpm = 0, rpm1 = 0, count = 0;
    int rv;
    int fantray_count;
    uint8_t psu_state;
    int psu_present_data = 0;
    dev_info_t dev_info;

    if( dni_bmc_check()== BMC_ON){
        rv = ONLP_STATUS_OK;
    }
    else{
        dev_info.offset = 0x00;
        dev_info.flags = DEFAULT_FLAG;

        /* Fan tray 1 */
        dev_info.addr = FAN_TRAY_1;
        dev_info.bus = I2C_BUS_25;
        fantray_present = dni_i2c_lock_read(NULL, &dev_info);
        rpm = dni_i2c_lock_read_attribute(NULL, FAN1_FRONT);
        rpm1 = dni_i2c_lock_read_attribute(NULL, FAN1_REAR);

        if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
        {
            /* Green */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }
        else
        {
            /* Red */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1),ONLP_LED_MODE_RED) != ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }
        /* Fan tray 2 */
        dev_info.addr = FAN_TRAY_2;
        dev_info.bus = I2C_BUS_24;
        fantray_present = dni_i2c_lock_read(NULL, &dev_info);
        rpm = dni_i2c_lock_read_attribute(NULL, FAN2_FRONT);
        rpm1 = dni_i2c_lock_read_attribute(NULL, FAN2_REAR);
        if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
        {
            /* Green */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }
        else
        {
            /* Red */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2),ONLP_LED_MODE_RED) != ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }

        /* Fan tray 3 */
        dev_info.addr = FAN_TRAY_3;
        dev_info.bus = I2C_BUS_23;
        fantray_present = dni_i2c_lock_read(NULL, &dev_info);
        rpm = dni_i2c_lock_read_attribute(NULL, FAN3_FRONT);
        rpm1 = dni_i2c_lock_read_attribute(NULL, FAN3_REAR);

        if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
        {
            /* Green */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }
        else
        {
            /* Red */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3),ONLP_LED_MODE_RED) != ONLP_STATUS_OK)
                                             rv = ONLP_STATUS_E_INTERNAL;
        }

        /* Fan tray 4 */
        dev_info.addr = FAN_TRAY_4;
        dev_info.bus = I2C_BUS_22;
        fantray_present = dni_i2c_lock_read(NULL, &dev_info);
        rpm = dni_i2c_lock_read_attribute(NULL, FAN4_FRONT);
        rpm1 = dni_i2c_lock_read_attribute(NULL, FAN4_REAR);

        if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
        {
            /* Green */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }
        else
        {
            /* Red */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4),ONLP_LED_MODE_RED) != ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }

        /* Fan tray 5 */
        dev_info.addr = FAN_TRAY_5;
        dev_info.bus = I2C_BUS_21;
        fantray_present = dni_i2c_lock_read(NULL, &dev_info);
        rpm = dni_i2c_lock_read_attribute(NULL, FAN5_FRONT);
        rpm1 = dni_i2c_lock_read_attribute(NULL, FAN5_REAR);

        if(fantray_present >= 0 && rpm != FAN_ZERO_RPM && rpm != 0 && rpm1 != FAN_ZERO_RPM && rpm1 != 0 )
        {
            /* Green */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_5),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }
        else
        {
            /* Red */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_5),ONLP_LED_MODE_RED) != ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }


        /* FRONT FAN & SYS LED */
        for(fantray_count = 0; fantray_count < 5; fantray_count++)
        {
            dev_info.addr = FAN_TRAY_1 + fantray_count;
            dev_info.bus = I2C_BUS_25 - fantray_count;
            fantray_present = dni_i2c_lock_read(NULL, &dev_info);
            if( fantray_present >= 0)
                count++;
        }
        if(count == ALL_FAN_TRAY_EXIST && dni_fan_speed_good() == FAN_SPEED_NORMALLY)
        {
            if((onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK) ||
                (onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_SYS),ONLP_LED_MODE_GREEN) != ONLP_STATUS_OK))
                rv = ONLP_STATUS_E_INTERNAL;
        }
        else
        {
            /* Solid Amber FAN or more failed*/
            if((onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN),ONLP_LED_MODE_RED) != ONLP_STATUS_OK) ||
              (onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_SYS),ONLP_LED_MODE_RED) !=  ONLP_STATUS_OK))
                rv = ONLP_STATUS_E_INTERNAL;
        }

        /* Set front light of PWR */
        dev_info.bus = I2C_BUS_4;
        dev_info.offset = 0x00;
        dev_info.flags = DEFAULT_FLAG;
        dev_info.addr = PSU_EEPROM;
        psu_state = dni_i2c_lock_read(NULL, &dev_info);
        psu_present_data = dni_lock_cpld_read_attribute(SWPLD1_PATH,POWER_STATUS_REGISTER); 

        if(psu_state == 1 && (psu_present_data & 0x60) == 0x60)
        {
            /* Green */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR1), ONLP_LED_MODE_GREEN) !=  ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }
        else
        {
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR1), ONLP_LED_MODE_RED) != ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }

        dev_info.bus = I2C_BUS_5;

        psu_state = dni_i2c_lock_read(NULL, &dev_info);

        if(psu_state == 1 && (psu_present_data & 0x06) == 0x06)
        {
            /* Green */
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR2), ONLP_LED_MODE_GREEN) !=  ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }
        else
        {
            if(onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR2), ONLP_LED_MODE_RED) != ONLP_STATUS_OK)
                rv = ONLP_STATUS_E_INTERNAL;
        }
        rv = ONLP_STATUS_OK;
    }

    return rv;
}
