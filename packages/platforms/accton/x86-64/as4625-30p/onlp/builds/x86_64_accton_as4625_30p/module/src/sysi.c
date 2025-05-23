/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"
#include "x86_64_accton_as4625_30p_int.h"
#include "x86_64_accton_as4625_30p_log.h"

#define BIOS_VER_PATH "/sys/devices/virtual/dmi/id/bios_version"
#define PREFIX_PATH_ON_CPLD_DEV "/sys/bus/i2c/devices/0-0064/"
#define REFERENCE_THERMAL_ID (ONLP_THERMAL_ID_CREATE(1 + 2)) /* MB_LeftFront_temp(0x4D) */

/* Fan speed levels corresponding to thermal conditions */
enum {
	LEVEL_FAN_0 = 0,
	LEVEL_FAN_1,
	LEVEL_FAN_2,
	LEVEL_FAN_3,
	LEVEL_FAN_4,
	LEVEL_FAN_5,
	LEVEL_FAN_6,
	LEVEL_FAN_7,
	LEVEL_FAN_8,
	LEVEL_FAN_9,
	LEVEL_FAN_MAX,
	LEVEL_FAN_TOTAL
};
#define LEVEL_FAN_DEF LEVEL_FAN_4
#define LEVEL_FAN_SHUTDOWN LEVEL_FAN_MAX

/* Fan control policy: maps temperature thresholds to fan duty and states */
typedef struct fan_ctrl_policy {
	int duty_cycle;
	int temp_threshold_high;
	int temp_threshold_low;
} fan_ctrl_policy_t;

/* Fan control policy table */
fan_ctrl_policy_t thermal_policy[] = {
	[LEVEL_FAN_0]      = {19, 23000, 20000},
	[LEVEL_FAN_1]        = {32, 34000, 31000},
	[LEVEL_FAN_2]        = {38, 40000, 37000},
	[LEVEL_FAN_3]        = {44, 42000, 39000},
	[LEVEL_FAN_4]        = {50, 44000, 41000},
	[LEVEL_FAN_5]        = {57, 46000, 43000},
	[LEVEL_FAN_6]        = {63, 50000, 47000},
	[LEVEL_FAN_7]        = {69, 53000, 50000},
	[LEVEL_FAN_8]        = {82, 55000, 52000},
	[LEVEL_FAN_9]        = {94, 56000, 53000},
	[LEVEL_FAN_MAX]      = {100, 57000, 54000},
};

const char*
onlp_sysi_platform_get(void)
{
	return "x86-64-accton-as4625-30p-r0";
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

	AIM_FREE_IF_PTR(rdata);
	*size = 0;
	return ONLP_STATUS_E_INTERNAL;
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{
	AIM_FREE_IF_PTR(data);
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
	int i;
	onlp_oid_t* e = table;
	memset(table, 0, max*sizeof(onlp_oid_t));

	/* 4 Thermal sensors on the chassis */
	for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
		*e++ = ONLP_THERMAL_ID_CREATE(i);
	}

	/* 6 LEDs on the chassis */
	for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
		*e++ = ONLP_LED_ID_CREATE(i);
	}

	/* 2 PSUs on the chassis */
	for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
		*e++ = ONLP_PSU_ID_CREATE(i);
	}

	/* 3 Fans on the chassis */
	for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
		*e++ = ONLP_FAN_ID_CREATE(i);
	}

	return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
	onlp_onie_info_t onie;
	int ver_major = 0, ver_minor = 0;
	char *bios_ver = NULL;

	if(onlp_file_read_str(&bios_ver, BIOS_VER_PATH) < 0)
		return ONLP_STATUS_E_INTERNAL;

	if (onlp_onie_decode_file(&onie, IDPROM_PATH) < 0)
		return ONLP_STATUS_E_INTERNAL;

	if(onlp_file_read_int(&ver_major, "%s/version_major", PREFIX_PATH_ON_CPLD_DEV) < 0)
		return ONLP_STATUS_E_INTERNAL;

	if(onlp_file_read_int(&ver_minor, "%s/version_minor", PREFIX_PATH_ON_CPLD_DEV) < 0)
		return ONLP_STATUS_E_INTERNAL;

	pi->cpld_versions = aim_fstrdup("\r\n\t   CPU CPLD(0x64): %02X.%02X",
					ver_major, ver_minor);

	pi->other_versions = aim_fstrdup("\r\n\t   BIOS: %s\r\n\t   ONIE: %s",
					bios_ver, onie.onie_version);

	onlp_onie_info_free(&onie);
	AIM_FREE_IF_PTR(bios_ver);

	return 0;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
	AIM_FREE_IF_PTR(pi->cpld_versions);
	AIM_FREE_IF_PTR(pi->other_versions);
}

int onlp_sysi_platform_manage_fans(void)
{
	static int prev_warning = 0;
	static int prev_sys_temp = 0;
	static int prev_duty_cycle = 50;
	int warning = 0;
	int shutdown = 0;
	int sys_temp = ONLP_STATUS_E_INVALID;
	int fan_status_error = 0, temp_status_error = 0;
	int fan_fail = 0;
	onlp_thermal_info_t thermali[CHASSIS_THERMAL_COUNT-1];
	int i = 0;
	int duty_cycle = 0;

	/* 1. refresh fan status */
	fan_fail = 0;
	fan_status_error = 0;
	for(i = 1; i <= CHASSIS_FAN_COUNT; i++){
		onlp_fan_info_t fan_info;
		if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) 
			!= ONLP_STATUS_OK) {
			AIM_LOG_WARN("Unable to get fan(%s) status.\r\n", fan_info.hdr.description);
			fan_status_error = 1;
			break;
		}

		if (fan_info.status & ONLP_FAN_STATUS_FAILED || 
			!(fan_info.status & ONLP_FAN_STATUS_PRESENT)) {
			AIM_LOG_WARN("Fan(%s) is not working\r\n", fan_info.hdr.description);
			fan_fail = 1;
			break;
		}
	}

	/* 2. refresh temperature status */
	temp_status_error = 0;
	for(i = 0; i < CHASSIS_THERMAL_COUNT-1; i++){
		if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+2), &thermali[i]) 
			!= ONLP_STATUS_OK) {
			thermali[i].status = ONLP_STATUS_E_MISSING;
			AIM_LOG_WARN("Unable to read thermal(%s) status.\n\r", thermali[i].hdr.description);
			temp_status_error = 1;
		} else { 
			/* MB_LeftFront_temp(0x4D) */
			if (REFERENCE_THERMAL_ID == thermali[i].hdr.id) {
				sys_temp = thermali[i].mcelsius;
			}
		}
	}

	if(fan_fail || fan_status_error || temp_status_error || sys_temp == ONLP_STATUS_E_INVALID){
		AIM_LOG_WARN("Error occurred while updating fan and thermal status\n\r");
	} else {
		if(prev_sys_temp != sys_temp){
			if (prev_sys_temp < sys_temp) { /* Handle temperature rising scenario */
				AIM_LOG_INFO("Temperature RISING: prev_temp=%d, curr_temp=%d", prev_sys_temp, sys_temp);
				for (i = 0; i < AIM_ARRAYSIZE(thermal_policy); i++) {
					duty_cycle = thermal_policy[i].duty_cycle;
					if (sys_temp <= thermal_policy[i].temp_threshold_high) {
						break;
					}
				}
				AIM_LOG_INFO("Temperature RISING: Final duty_cycle selected: %d", duty_cycle);
			} else if (prev_sys_temp > sys_temp) { /* Handle temperature falling scenario */
				AIM_LOG_INFO("Temperature FALLING: prev_temp=%d, curr_temp=%d", prev_sys_temp, sys_temp);
				for (i = AIM_ARRAYSIZE(thermal_policy) - 1; i >= 0; i--) {
					duty_cycle = thermal_policy[i].duty_cycle;
					if (sys_temp >= thermal_policy[i].temp_threshold_low) {
						break;
					}
				}
				AIM_LOG_INFO("Temperature FALLING: Final duty_cycle selected: %d", duty_cycle);
			}
			prev_sys_temp = sys_temp;
		} else {
			duty_cycle = prev_duty_cycle;
		}
	}

	/* 3. check temperature of each thermal sensor */
	for(i = 0; i < CHASSIS_THERMAL_COUNT-1; i++){
		if(thermali[i].status == ONLP_STATUS_E_MISSING) {
			continue;
		}
		if(thermali[i].mcelsius >= thermali[i].thresholds.shutdown){
			warning = 1;
			shutdown = 1;
			AIM_LOG_WARN("thermal(%s) temperature(%d) reach shutdown threshold(%d)\n\r",
				thermali[i].hdr.description, thermali[i].mcelsius, thermali[i].thresholds.shutdown);
		} else if(thermali[i].mcelsius >= thermali[i].thresholds.error){
			warning = 1;
			AIM_LOG_WARN("thermal(%s) temperature(%d) reach high threshold(%d)\n\r",
				thermali[i].hdr.description, thermali[i].mcelsius, thermali[i].thresholds.error);
		}
	}

	/* 4. action */
	if(warning || shutdown || fan_fail || fan_status_error || temp_status_error || sys_temp == ONLP_STATUS_E_INVALID) {
		duty_cycle = thermal_policy[LEVEL_FAN_MAX].duty_cycle;
	}
	if(prev_duty_cycle != duty_cycle) {
		AIM_LOG_INFO("Fan duty cycle changed: %d -> %d", prev_duty_cycle, duty_cycle);
		for(i = 1; i <= CHASSIS_FAN_COUNT; i++){
			onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), duty_cycle);
		}
		prev_duty_cycle = duty_cycle;
	}

	if(prev_warning != warning){
		if(warning) {
			AIM_LOG_WARN("Alarm for temperature high is detected\n\r");
		} else {
			AIM_LOG_INFO("Alarm for temperature high is cleared\n\r");
		}
		prev_warning = warning;
	}

	if(shutdown){
		AIM_LOG_WARN("Alarm-Critical for temperature critical is detected, disable PoE\n\r");
		AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical",
				"Alarm-Critical for temperature critical is detected, disable PoE\n\r");
		onlp_file_write_int(0, PREFIX_PATH_ON_CPLD_DEV"pwr_enable_poe");

		AIM_LOG_WARN("Alarm-Critical for temperature critical is detected, trigger thermal shutdown\n\r");
		AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical", 
				"Alarm-Critical for temperature critical is detected, trigger thermal shutdown\n\r");
		/* Sync log buffer to disk */
		system("sync;sync;sync");
		system("/sbin/fstrim -av");
		sleep(5);
		onlp_file_write_int(1, PREFIX_PATH_ON_CPLD_DEV"thermal_shutdown");
	}
	return 0;
}
