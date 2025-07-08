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
#include "x86_64_accton_as4625_54t_int.h"
#include "x86_64_accton_as4625_54t_log.h"

#define PREFIX_PATH_ON_CPLD_DEV "/sys/bus/i2c/devices/0-0064/"

#define NUM_OF_CPLD 1
#define FAN_DUTY_CYCLE_MAX (100)

#define FAN_SPEED_DEFAULT_F2B 38
#define FAN_SPEED_DEFAULT_B2F 25

#define PCB_ID_AS4625_54T_F2B  1
#define PCB_ID_AS4625_54T_B2F  2

#define HIGH_THRESHOLD_F2B 186
#define HIGH_THRESHOLD_B2F 184
#define LOW_THRESHOLD_F2B 111
#define LOW_THRESHOLD_B2F 116

#define PWM_STATE_NORMAL 0
#define PWM_STATE_CRITICAL 1

const char*
onlp_sysi_platform_get(void)
{
	return "x86-64-accton-as4625-54t-r0";
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
	return ONLP_STATUS_E_INTERNAL;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
	int i;
	onlp_oid_t* e = table;
	memset(table, 0, max*sizeof(onlp_oid_t));

	/* 6 Thermal sensors on the chassis */
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
	int ver_major = 0, ver_minor = 0;

	if(onlp_file_read_int(&ver_major, "%s/version_major", PREFIX_PATH_ON_CPLD_DEV) < 0)
		return ONLP_STATUS_E_INTERNAL;

	if(onlp_file_read_int(&ver_minor, "%s/version_minor", PREFIX_PATH_ON_CPLD_DEV) < 0)
		return ONLP_STATUS_E_INTERNAL;

	pi->cpld_versions = aim_fstrdup("\r\nCPLD ver: %.2d.%.2d", ver_major, ver_minor);

	return 0;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
	aim_free(pi->cpld_versions);
}

int get_pcb_id()
{
	int pcb_id = PCB_ID_AS4625_54T_B2F;

	if(onlp_file_read_int(&pcb_id, "%s/pcb_id", PREFIX_PATH_ON_CPLD_DEV) < 0){
		AIM_LOG_WARN("Unable to get pcb id\n\r");
		return ONLP_STATUS_E_INTERNAL;
	}

	if(pcb_id < PCB_ID_AS4625_54T_F2B || pcb_id > PCB_ID_AS4625_54T_B2F)
		pcb_id = PCB_ID_AS4625_54T_F2B;

	return pcb_id;
}

int get_default_fan_speed()
{
	int pcb_id = get_pcb_id();

	if(pcb_id == PCB_ID_AS4625_54T_F2B){
		return FAN_SPEED_DEFAULT_F2B;
	} else if(pcb_id == PCB_ID_AS4625_54T_B2F){
		return FAN_SPEED_DEFAULT_B2F;
	} else {
		return (FAN_SPEED_DEFAULT_F2B > FAN_SPEED_DEFAULT_B2F) ?
				FAN_SPEED_DEFAULT_F2B : FAN_SPEED_DEFAULT_B2F;
	}
}

int get_system_low_threshold()
{
	int pcb_id = get_pcb_id();

	if(pcb_id == PCB_ID_AS4625_54T_F2B){
		return LOW_THRESHOLD_F2B;
	} else if(pcb_id == PCB_ID_AS4625_54T_B2F){
		return LOW_THRESHOLD_B2F;
	} else {
		return (LOW_THRESHOLD_F2B < LOW_THRESHOLD_B2F) ?
				LOW_THRESHOLD_F2B : LOW_THRESHOLD_B2F;
	}
}

int get_system_high_threshold()
{
		int pcb_id = get_pcb_id();

		if(pcb_id == PCB_ID_AS4625_54T_F2B){
			return HIGH_THRESHOLD_F2B;
		} else if(pcb_id == PCB_ID_AS4625_54T_B2F) {
			return HIGH_THRESHOLD_B2F;
		} else {
			return (HIGH_THRESHOLD_F2B < HIGH_THRESHOLD_B2F) ?
					HIGH_THRESHOLD_F2B : HIGH_THRESHOLD_B2F;
		}
}

int onlp_sysi_platform_manage_fans(void)
{
	static int prev_warning = 0;
	static int warning = 0;
	static int pwm_state = 0;
	int shutdown = 0;
	int sys_temp = 0;
	int fan_status_error = 0, temp_status_error = 0;
	int fan_fail = 0;
	onlp_thermal_info_t thermali[5];
	int i = 0;

	prev_warning = warning;

	// 1. refresh fan status
	fan_fail = 0;
	fan_status_error = 0;
	for(i = 1; i <= CHASSIS_FAN_COUNT; i++){
		onlp_fan_info_t fan_info;
		if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) 
			!= ONLP_STATUS_OK) {
			AIM_LOG_WARN("Unable to get fan(%d) status.\r\n", i);
			fan_status_error = 1;
			break;
		}

		if (fan_info.status & ONLP_FAN_STATUS_FAILED || 
			!(fan_info.status & ONLP_FAN_STATUS_PRESENT)) {
			AIM_LOG_WARN("Fan(%d) is not working\r\n", i);
			fan_fail = 1;
			break;
		}
	}

	// 2. refresh temperature status
	temp_status_error = 0;
	for(i = 0; i < CHASSIS_THERMAL_COUNT-1; i++){
		if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+2), &thermali[i]) 
			!= ONLP_STATUS_OK) {
			AIM_LOG_WARN("Unable to read thermal status.\n\r");
			temp_status_error = 1;
			break;
		}

		if(i != 3) // skip lm75 at 0x4E
			sys_temp += (thermali[i].mcelsius / 1000);
	}

	if(fan_status_error || temp_status_error){
		for(i = 1; i <= CHASSIS_FAN_COUNT; i++){
			onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), FAN_DUTY_CYCLE_MAX);
			AIM_LOG_WARN("Error occurred while updating fan and thermal status\n\r");
		}
		return 0;
	}

	// 2.1 check if current state is valid
	if(pwm_state < PWM_STATE_NORMAL || pwm_state > PWM_STATE_CRITICAL)
		pwm_state = PWM_STATE_NORMAL;

	// 2.2 check system temperature
	if(sys_temp > get_system_high_threshold())
		warning = 1;
	else
		warning = 0;

	if(pwm_state == PWM_STATE_CRITICAL){
		if(sys_temp < get_system_low_threshold())
			pwm_state = PWM_STATE_NORMAL;
	} else {
		if(sys_temp > get_system_high_threshold()){
			pwm_state = PWM_STATE_CRITICAL;
			AIM_LOG_WARN("system temperature(%d) reaches high threshold(%d)\n\r",
				sys_temp, get_system_high_threshold());
		}
	}

	// 2.2 check temperature of each thermal sensor
	for(i = 0; i < CHASSIS_THERMAL_COUNT-1; i++){
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

	if(warning || shutdown)
		pwm_state = PWM_STATE_CRITICAL;

	// 3. action
	for(i = 1; i <= CHASSIS_FAN_COUNT; i++){
		if(fan_fail){
			onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), FAN_DUTY_CYCLE_MAX);
		} else {
			if(pwm_state == PWM_STATE_NORMAL)
				onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), get_default_fan_speed());
			else if(pwm_state == PWM_STATE_CRITICAL)
				onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), FAN_DUTY_CYCLE_MAX);
		}
	}

	if(prev_warning != warning){
		if(warning)
			AIM_LOG_WARN("Alarm for temperature high is detected\n\r");
		else
			AIM_LOG_INFO("Alarm for temperature high is cleared\n\r");
	}

	if(shutdown){

		/*
          Because the R0A hardware does not have the system thermal
          shutdown (0x27) function, the power control (0x03) function
          is used for thermal shutdown. So for the R0A hardware, this
          thermal_shutdown sysfs is not functional.
        */

		AIM_LOG_WARN("Alarm-Critical for temperature critical is detected, trigger thermal shutdown\n\r");
		// Sync log buffer to disk for hardware revision R01 and above
		system("sync;sync;sync");
		system("/sbin/fstrim -av");
		sleep(5);
		onlp_file_write_int(1, PREFIX_PATH_ON_CPLD_DEV"thermal_shutdown");

		AIM_LOG_WARN("Alarm-Critical for temperature critical is detected, shutdown DUT\n\r");
		// Sync log buffer to disk for hardware revision R0A
		system("sync;sync;sync");
		system("/sbin/fstrim -av");
		sleep(5);
		onlp_file_write_int(0, PREFIX_PATH_ON_CPLD_DEV"pwr_enable_mb");
	}
	return 0;
}