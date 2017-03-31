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
    
    uint8_t present_bit = 0 ,addr = 0;

    /* set PWR led in front panel */
    addr = dni_lock_cpld_read_attribute(SWPLD_PATH,LED_REG);

    /* Turn the fan led on or off */
    if((addr & 0x3) == 0 || (addr & 0x3) == 0x3 )
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN), TURN_OFF);
    }
    else
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN), TURN_ON);
    }

    if(dni_psu_present_get(1) == 1) 
    { /* PSU1 is present */
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), TURN_ON);
    }
     else 
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR), TURN_OFF);
    }
    /* Rare light fan tray 1-4 */
    present_bit = dni_lock_cpld_read_attribute(SWPLD_PATH,FAN_TRAY_LED_REG);

    if ((present_bit& 0x08) == 0x00)
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1), TURN_ON);
    }
    else
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1), TURN_OFF);
    }
    if ((present_bit& 0x04) == 0x00)
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2), TURN_ON);
    }
    else
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2), TURN_OFF);
    }
    if ((present_bit& 0x02) == 0x00)
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3), TURN_ON);
    }
    else
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3), TURN_OFF);
    }
    if ((present_bit& 0x01) == 0x00)
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4), TURN_ON);
    }
    else
    {
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4), TURN_OFF);
    }
    return ONLP_STATUS_OK;
}

