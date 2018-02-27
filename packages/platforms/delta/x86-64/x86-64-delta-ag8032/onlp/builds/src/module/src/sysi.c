/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2017 Delta Networks, Inc
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
#include <stdio.h>
#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "platform_lib.h"

//platform_id_t platform_id = PLATFORM_ID_UNKNOWN;

//#define ONIE_PLATFORM_NAME "x86-64-delta-ag8032-r0"

const char*
onlp_sysi_platform_get(void)
{ 
	return "x86-64-delta-ag8032-r0";
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{ 
	char buff[128];
	cpld_reg_t CpuBoardHwVer  = CPLD_REG(CPLD_CPUPLD, 0x02, 4, 4);
	cpld_reg_t CpuBoardPldVer = CPLD_REG(CPLD_CPUPLD, 0x01, 0, 4);
	cpld_reg_t MainBoardHwVer  = CPLD_REG(CPLD_SWPLD, 0x00, 0, 4);
	cpld_reg_t MainBoardPldVer = CPLD_REG(CPLD_SWPLD, 0x01, 0, 4);


	snprintf (buff, sizeof(buff), "CpuBoardPldVer=%d,MainBoardPldVer=%d",
		cpld_reg_get(&CpuBoardPldVer),
		cpld_reg_get(&MainBoardPldVer));
    pi->cpld_versions = aim_fstrdup("%s", buff);

	snprintf (buff, sizeof(buff), "CpuBoardHwVer=%d,MainBoardHwVer=%d",
		cpld_reg_get(&CpuBoardHwVer),
		cpld_reg_get(&MainBoardHwVer));
    pi->other_versions = aim_fstrdup("%s", buff);


    return 0;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
	uint8_t* rdata = aim_zmalloc(256);

	if(!rdata){
		return ONLP_STATUS_E_INTERNAL;
	}
   
	*data = rdata;
	if(onlp_file_read(rdata, 256, size, ONIE_EEPROM_LOCATION) == ONLP_STATUS_OK) {
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
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{ 
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* Thermal sensors on the platform */
    for (i = PLAT_THERMAL_ID_1; i < PLAT_THERMAL_ID_MAX; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* LEDs on the platform */
    for (i = PLAT_LED_ID_1; i < PLAT_LED_ID_MAX; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* Fans on the platform */
    for (i = PLAT_FAN_ID_1; i < PLAT_FAN_ID_MAX; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* PSUs on the platform */
    for (i = PLAT_PSU_ID_1; i < PLAT_PSU_ID_MAX; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }
    return 0;
}

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{   
	onie->platform_name = aim_strdup("x86-64-delta_ag8032-r0");
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_fans(void)
{
	onlp_thermal_info_t thermal;
	int i;
	int temp_max;
	int rpm;

	temp_max = 0;
	for (i = PLAT_THERMAL_ID_1 ; i <= PLAT_THERMAL_ID_5; i ++) {
		if (onlp_thermali_info_get (ONLP_THERMAL_ID_CREATE(i), &thermal) == ONLP_STATUS_OK)
			if (thermal.mcelsius > temp_max)
				temp_max = thermal.mcelsius;
	}

	rpm = 7500;
    if((temp_max >= 30000) && (temp_max < 40000)) 	rpm =10000;
    if((temp_max >= 45000) && (temp_max < 55000))	rpm =13000;
    if((temp_max >= 60000) && (temp_max < 75000))	rpm =16000;
    if( temp_max >= 80000)							rpm =19000;

	for (i = PLAT_FAN_ID_1 ; i <= PLAT_FAN_ID_6 ; i ++) {
		onlp_fani_rpm_set (ONLP_FAN_ID_CREATE(i), rpm);
	}
 
    return ONLP_STATUS_OK;
}


int
onlp_sysi_platform_manage_leds(void)
{
	onlp_fan_info_t fan;
	onlp_psu_info_t psu;
	int i;
	uint32_t status;
	int led_setting;
	int global_fail;

	// fan tray led
	global_fail = 0;
	for (i = 0 ; i < 3 ; i ++) {
		status = 0;
		if (onlp_fani_info_get (ONLP_FAN_ID_CREATE(PLAT_FAN_ID_1 + i * 2 + 0),
				&fan) == ONLP_STATUS_OK) {
			status |= fan.status;
		}
		if (onlp_fani_info_get (ONLP_FAN_ID_CREATE(PLAT_FAN_ID_1 + i * 2 + 1),
				&fan) == ONLP_STATUS_OK) {
			status |= fan.status;
		}
		led_setting = ONLP_LED_MODE_GREEN;
		if (status & ONLP_FAN_STATUS_FAILED) {
			led_setting = ONLP_LED_MODE_ORANGE;
			global_fail ++;
		} else if ((status & ONLP_FAN_STATUS_PRESENT) == 0) {
			led_setting = ONLP_LED_MODE_OFF;
			global_fail ++;
		}
		printf ("fuck 111111111111 %d led_setting=%d \n", i, led_setting);
		onlp_ledi_mode_set (ONLP_LED_ID_CREATE (PLAT_LED_ID_5 + i), led_setting);
	}

	// fans led (front fan led)
	onlp_ledi_mode_set (ONLP_LED_ID_CREATE (PLAT_LED_ID_2),
		global_fail ? ONLP_LED_MODE_ORANGE : ONLP_LED_MODE_GREEN);


	// pwr1 led (front)
	led_setting = ONLP_LED_MODE_ORANGE;
	if (onlp_psui_info_get (ONLP_PSU_ID_CREATE(PLAT_PSU_ID_1), &psu) == ONLP_STATUS_OK) {
		if (psu.status & ONLP_PSU_STATUS_FAILED)
			led_setting = ONLP_LED_MODE_ORANGE;
		else if ((psu.status & ONLP_PSU_STATUS_PRESENT) == 0)
			led_setting = ONLP_LED_MODE_OFF;
		else
			led_setting = ONLP_LED_MODE_GREEN;
	}
	onlp_ledi_mode_set (ONLP_LED_ID_CREATE (PLAT_LED_ID_3), led_setting);
	
	// pwr2 led (front)
	led_setting = ONLP_LED_MODE_ORANGE;
	if (onlp_psui_info_get (ONLP_PSU_ID_CREATE(PLAT_PSU_ID_2), &psu) == ONLP_STATUS_OK) {
		if (psu.status & ONLP_PSU_STATUS_FAILED)
			led_setting = ONLP_LED_MODE_ORANGE;
		else if ((psu.status & ONLP_PSU_STATUS_PRESENT) == 0)
			led_setting = ONLP_LED_MODE_OFF;
		else
			led_setting = ONLP_LED_MODE_GREEN;
	}
	onlp_ledi_mode_set (ONLP_LED_ID_CREATE (PLAT_LED_ID_4), led_setting);
	
	// sys
	return ONLP_STATUS_OK;
}

