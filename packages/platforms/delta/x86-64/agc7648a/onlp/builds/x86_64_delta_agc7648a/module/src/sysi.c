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
#include <unistd.h>
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_delta_agc7648a_int.h"
#include "x86_64_delta_agc7648a_log.h"

#include "platform_lib.h"

typedef struct cpld_dev_s{
    char name[32];
    char path[64];
}cpld_dev_t;

static cpld_dev_t cpld_dev[NUM_OF_CPLD] = 
{
    {"CPUCPLD"    , CPU_CPLD_PATH},
    {"SWPLD(U21)" , SWPLD_PATH   },
    {"SWPLD(U134)", SWPLD1_PATH  },
    {"SWPLD(U215)", SWPLD2_PATH  }
};

/******************* Utility Function *****************************************/
int 
decide_percentage(int *percentage, int temper)
{
    int level;

    if(temper <= 25)
    {
        *percentage = 40;
        level = 0;
    }
    else if(temper > 25 && temper <= 40)
    {
        *percentage = 60;
        level = 1;
    }
    else if(temper > 40 && temper <= 55)
    {
        *percentage = 80;
        level = 2;
    }
    else if(temper > 55 && temper <= 75)
    {
        *percentage = 90;
        level = 3;
    }
    else if(temper > 75)
    {
        *percentage = 100;
        level = 4;
    }
    else
    {
        *percentage = 100;
        level = 5;
    }

    return level;
}
/******************************************************************************/
const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-delta-agc7648a-r0";
}

int
onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{
    onie->platform_name = aim_strdup("x86-64-delta_agc7648a-r0");
    return ONLP_STATUS_OK;
}

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

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    char fullpath[128], buf[128], cpld_ver[256] = {0};
    int i, v[NUM_OF_CPLD] = {0};

    for(i = 0; i < NUM_OF_CPLD; ++i) 
    {
        sprintf(fullpath, "%s/version", cpld_dev[i].path);
        v[i] = dni_i2c_lock_read_attribute(NULL, fullpath);
        sprintf(buf, "%s=%d ", cpld_dev[i].name, v[i]);
        strcat(cpld_ver, buf);
    }

    pi->cpld_versions = aim_fstrdup("%s", cpld_ver);

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

    for (i = 1; i <= NUM_OF_THERMAL; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 4 LEDs on the chassis */
    for (i = 1; i <= NUM_OF_LED_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 4 Fans on the chassis */
    for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= NUM_OF_PSU_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    return 0;
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
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_1_ON_MAIN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_2_ON_MAIN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_3_ON_MAIN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_4_ON_MAIN_BOARD), MAX_FRONT_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_5_ON_MAIN_BOARD), MAX_REAR_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_6_ON_MAIN_BOARD), MAX_REAR_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_7_ON_MAIN_BOARD), MAX_REAR_FAN_SPEED * new_percentage / 100);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_8_ON_MAIN_BOARD), MAX_REAR_FAN_SPEED * new_percentage / 100);
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU1), new_percentage);
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU2), new_percentage);

    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    
    uint8_t count, power_state;
    int fantray_present = -1 ,rpm = 0,rpm1 = 0 , i;
    /* set PWR led in front panel */

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


    /* FRONT FAN & SYS LED */
    for(i = 0;i < 4; i++)
    {
        mux_info.channel = i;
        /* FAN TRAT 1~4: 0x52 , 0x53, 0x54, 0x55 */
        dev_info.addr = FAN_TRAY_1 + i;
        fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
        if( fantray_present >= 0 )
            count++;
    }
    if(count == ALL_FAN_TRAY_EXIST)
    {
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN), ONLP_LED_MODE_GREEN);
    }
    else
    {
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN), ONLP_LED_MODE_ORANGE);
    }

    dev_info.bus = I2C_BUS_4;
    dev_info.addr = PSU_EEPROM;
    mux_info.channel = 0x00;

    /* Check the state of PSU 1, "state = 1, PSU exists' */
    power_state = dni_lock_cpld_read_attribute(SWPLD_PATH, PSU_PWR_REG);
    /* Set the light of PSU */
    if((power_state&0x80) != 0x80)
    {
     /* ORANGE */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_ORANGE);
    }
    else if((power_state&0x80)==0x80)
    {
         /* Green */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_GREEN);
    }
    else
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_OFF);

    mux_info.channel= 0x00;
    dev_info.addr = FAN_TRAY_1;
    dev_info.bus = I2C_BUS_3;
    fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
    rpm = dni_i2c_lock_read_attribute(NULL, FAN4_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN4_REAR);
    if(fantray_present >= 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
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
    mux_info.channel= 0x01;
    dev_info.addr = FAN_TRAY_2;
    dev_info.bus = I2C_BUS_3;
    fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
    rpm = dni_i2c_lock_read_attribute(NULL, FAN3_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN3_REAR);
    if(fantray_present >= 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
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
    mux_info.channel= 0x02;
    dev_info.bus = I2C_BUS_3;
    dev_info.addr = FAN_TRAY_3;
    fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
    rpm = dni_i2c_lock_read_attribute(NULL, FAN2_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN2_REAR);

    if(fantray_present >= 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
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
    mux_info.channel= 0x03;
    dev_info.addr = FAN_TRAY_4;
    dev_info.bus = I2C_BUS_3;
    fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
    rpm = dni_i2c_lock_read_attribute(NULL, FAN1_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN1_REAR);
    if(fantray_present >= 0 && rpm != 960 && rpm != 0 && rpm1 != 960 && rpm1 != 0 )
    {
        /* Green */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4),ONLP_LED_MODE_GREEN);
    }
    else
    {
        /* Red */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4),ONLP_LED_MODE_RED);
    }


    /* Set front light of PWR */
    dev_info.bus = I2C_BUS_4;
    dev_info.addr = PSU_EEPROM;
    mux_info.channel = 0x00;

    /* Check the state of PSU 1, "state = 1, PSU exists' */
    power_state = dni_lock_cpld_read_attribute(SWPLD_PATH, PSU_PWR_REG);
    /* Set the light of PSU */
   
    if((power_state&0x80) == 0x80)
    {
        /* Green */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_GREEN);
    }
    else if((power_state&0x80) != 0x80)
    {
            /* Red */
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_ORANGE);
    }
    else
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), ONLP_LED_MODE_OFF);
    
    return ONLP_STATUS_OK;
}

