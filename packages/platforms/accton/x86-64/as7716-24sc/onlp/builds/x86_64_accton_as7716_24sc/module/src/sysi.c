/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 Accton Technology Corporation.
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
#include "platform_lib.h"

#include "x86_64_accton_as7716_24sc_int.h"
#include "x86_64_accton_as7716_24sc_log.h"

#define CPLD_VERSION_FORMAT			"/sys/bus/i2c/devices/%s/version"
#define NUM_OF_CPLD         		3

static char* cpld_path[NUM_OF_CPLD] =
{
 "11-0060",
 "12-0062",
 "13-0064"
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as7716-24sc-r0";
}

#if 0
int
onlp_sysi_init(void)
{
	/* init dsp */
	int i = 0, ret;
	int dsp_busses[] = {41, 43, 45, 47, 49, 51, 53, 55};

	for (i = 0; i < AIM_ARRAYSIZE(dsp_busses); i++) {
		printf("%s(%d)\r\n", __FUNCTION__, __LINE__);
		ret = dsp_initialize(dsp_busses[i]);
		if (ret < 0) {
			DEBUG_PRINT("%s(%d): dsp_initialize failed, dsp_id(%d), bus(%d), error code(%d)\r\n", __FUNCTION__, __LINE__, i, dsp_busses[i], ret);
			continue;
		}

		printf("%s(%d)\r\n", __FUNCTION__, __LINE__);
	}

    return ONLP_STATUS_OK;
}
#endif

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    int ret = ONLP_STATUS_OK;
    int i = 0;
    uint8_t* rdata = aim_zmalloc(256);

    for (i = 0; i < 128; i++) {
        ret = onlp_i2c_readw(0, 0x56, i*2, ONLP_I2C_F_FORCE);
        if (ret < 0) {
            aim_free(rdata);
            *size = 0;
            return ret;
        }

        rdata[i*2]   = ret & 0xff;
        rdata[i*2+1] = (ret >> 8) & 0xff;
    }

    *size = 256;
    *data = rdata;

    return ONLP_STATUS_OK;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));
    
    /* Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int   i, v[NUM_OF_CPLD] = {0};

    for (i = 0; i < AIM_ARRAYSIZE(cpld_path); i++) {
        v[i] = 0;

        if(onlp_file_read_int(v+i, CPLD_VERSION_FORMAT , cpld_path[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    pi->cpld_versions = aim_fstrdup("%d.%d.%d", v[0], v[1], v[2]);
    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int
onlp_sysi_platform_manage_init(void)
{
    /*
     * Bring the fan to max.
     * These will be reduced after the first platform management sequence.
     */
    return 0;
}

#define FAN_DUTY_MAX  (100)
#define FAN_DUTY_MIN  (52)

static int
sysi_fanctrl_fan_fault_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                              onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                              int *adjusted)
{
	int i;
    *adjusted = 0;

    /* Bring fan speed to FAN_DUTY_MAX if any fan is not operational */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (!(fi[i].status & ONLP_FAN_STATUS_FAILED)) {
            continue;
        }

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }

    return ONLP_STATUS_OK;
}

static int 
sysi_fanctrl_fan_absent_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                               onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                               int *adjusted)
{
	int i;
    *adjusted = 0;

    /* Bring fan speed to FAN_DUTY_MAX if fan is not present */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (fi[i].status & ONLP_FAN_STATUS_PRESENT) {
            continue;
        }

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }

    return ONLP_STATUS_OK;
}

static int
sysi_fanctrl_fan_unknown_speed_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                      onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                      int *adjusted)
{
	int i, match = 0;
    int fanduty;
	int legal_duties[] = {FAN_DUTY_MIN, 64, 76, 88, FAN_DUTY_MAX};

	*adjusted = 0;

    if (onlp_file_read_int(&fanduty, FAN_NODE(fan_duty_cycle_percentage)) < 0) {
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MIN);
    }    

    /* Bring fan speed to min if current speed is not expected
     */
    for (i = 0; i < AIM_ARRAYSIZE(legal_duties); i++) {
		if (fanduty != legal_duties[i]) {
			continue;
		}

		match = 1;
		break;
    }

	if (!match) {
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MIN);
	}

    return ONLP_STATUS_OK;
}

struct thermal_threshold {
    int tid; /* thermal id */
    int threshold[4];
};

struct thermal_threshold threshold_data[CHASSIS_THERMAL_COUNT] = {
{ /* Not Used*/ },
{ THERMAL_1_ON_MAIN_BROAD, {38000, 42000, 46000, 50000}},
{ THERMAL_2_ON_MAIN_BROAD, {48000, 51000, 54000, 57000}},
{ THERMAL_3_ON_MAIN_BROAD, {40000, 43000, 48000, 52000}},
{ THERMAL_4_ON_MAIN_BROAD, {40000, 48000, 53000, 60000}},
{ THERMAL_5_ON_MAIN_BROAD, {64000, 66000, 68000, 70000}},
{ /* Not Used*/ },
};

static int
sysi_fanctrl_overall_thermal_sensor_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                           onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                           int *adjusted)
{
    int i, new_duty = FAN_DUTY_MIN;

	*adjusted = 1;

    for (i = (THERMAL_1_ON_MAIN_BROAD); i <= (THERMAL_5_ON_MAIN_BROAD); i++) {
        if (ti[i-1].mcelsius > threshold_data[i-1].threshold[3]) {
            new_duty = FAN_DUTY_MAX;
        }
        else if (ti[i-1].mcelsius > threshold_data[i-1].threshold[2]) {
            new_duty = (new_duty > 88) ? new_duty : 88;
        }
        else if (ti[i-1].mcelsius > threshold_data[i-1].threshold[1]) {
            new_duty = (new_duty > 76) ? new_duty : 76;
        }
        else if (ti[i-1].mcelsius > threshold_data[i-1].threshold[0]) {
            new_duty = (new_duty > 64) ? new_duty : 64;
        }
    }

    return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), new_duty);
}

typedef int (*fan_control_policy)(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                  onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                  int *adjusted);

fan_control_policy fan_control_policies[] = {
sysi_fanctrl_fan_fault_policy,
sysi_fanctrl_fan_absent_policy,
sysi_fanctrl_fan_unknown_speed_policy,
sysi_fanctrl_overall_thermal_sensor_policy,
};

int
onlp_sysi_platform_manage_fans(void)
{
    int i, rc;
    onlp_fan_info_t fi[CHASSIS_FAN_COUNT];
    onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT];

    memset(fi, 0, sizeof(fi));
    memset(ti, 0, sizeof(ti));

    /* Get fan status
     */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        rc = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i+1), &fi[i]);

        if (rc != ONLP_STATUS_OK) {
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
			AIM_LOG_ERROR("Unable to get fan(%d) status\r\n", i+1);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    /* Get thermal sensor status
     */
    for (i = 0; i < CHASSIS_THERMAL_COUNT; i++) {
        rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+1), &ti[i]);
        
        if (rc != ONLP_STATUS_OK) {
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
			AIM_LOG_ERROR("Unable to get thermal(%d) status\r\n", i+1);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    /* Apply thermal policy according the policy list,
     * If fan duty is adjusted by one of the policies, skip the others
     */
    for (i = 0; i < AIM_ARRAYSIZE(fan_control_policies); i++) {
        int adjusted = 0;

        rc = fan_control_policies[i](fi, ti, &adjusted);
        if (!adjusted) {
            continue;
        }

        return rc;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

