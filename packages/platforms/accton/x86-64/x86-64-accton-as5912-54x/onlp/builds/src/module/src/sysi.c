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
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#include "x86_64_accton_as5912_54x_int.h"
#include "x86_64_accton_as5912_54x_log.h"

#define CPLD_VERSION_FORMAT			"/sys/bus/i2c/devices/%s/version"
#define NUM_OF_CPLD         		2

static char* cpld_path[NUM_OF_CPLD] =
{
 "4-0060",
 "5-0062"
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as5912-54x-r0";
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
    
    /* 5 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 5 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 6 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int   i, v[NUM_OF_CPLD] = {0};

    for (i=0; i < AIM_ARRAYSIZE(cpld_path); i++) {
        v[i] = 0;

        if(onlp_file_read_int(v+i, CPLD_VERSION_FORMAT , cpld_path[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    pi->cpld_versions = aim_fstrdup("%d.%d", v[0], v[1]);
    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

#define FAN_DUTY_MAX  (100)
#define FAN_DUTY_MID  (69)
#define FAN_DUTY_MIN  (38)

#define FANCTRL_DIR_FACTOR               (ONLP_FAN_STATUS_B2F)
#define FANCTRL_DIR_FACTOR_DUTY_ADDON    (6)

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
    int fanduty;
    int fanduty_min = FAN_DUTY_MIN;
    int fanduty_mid = FAN_DUTY_MID;

	*adjusted = 0;
    fanduty_min += (fi[0].status & FANCTRL_DIR_FACTOR) ? FANCTRL_DIR_FACTOR_DUTY_ADDON : 0;
    fanduty_mid += (fi[0].status & FANCTRL_DIR_FACTOR) ? FANCTRL_DIR_FACTOR_DUTY_ADDON : 0;

    if (onlp_file_read_int(&fanduty, FAN_NODE(fan_duty_cycle_percentage)) < 0) {
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }    

    /* Bring fan speed to max if current speed is not expected
     */
    if (fanduty != fanduty_min && fanduty != fanduty_mid && fanduty != FAN_DUTY_MAX) {
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }

    return ONLP_STATUS_OK;
}

static int
sysi_fanctrl_single_thermal_sensor_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                          onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                          int *adjusted)
{
	int i;
    *adjusted = 0;

    /* When anyone higher than 50 degrees, all fans run with duty 100%.
     */
    for (i = (THERMAL_1_ON_MAIN_BROAD); i <= (THERMAL_3_ON_MAIN_BROAD); i++) {
        if (ti[i-1].mcelsius < 50000) {
            continue;
        }

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }

    /* When anyone higher than 45 degrees, all fans run with duty 62.5%.
     */
    for (i = (THERMAL_1_ON_MAIN_BROAD); i <= (THERMAL_3_ON_MAIN_BROAD); i++) {
        if (ti[i-1].mcelsius < 45000) {
            continue;
        }

        int fanduty_mid = FAN_DUTY_MID;
        fanduty_mid += (fi[0].status & FANCTRL_DIR_FACTOR) ? FANCTRL_DIR_FACTOR_DUTY_ADDON : 0;

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), fanduty_mid);
    }

    return ONLP_STATUS_OK;
}

static int
sysi_fanctrl_overall_thermal_sensor_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                           onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                           int *adjusted)
{
    int fanduty_min = FAN_DUTY_MIN;
    int fanduty_mid = FAN_DUTY_MID;
    int i, num_of_sensor = 0, temp_avg = 0;

	*adjusted = 0;
    fanduty_min += (fi[0].status & FANCTRL_DIR_FACTOR) ? FANCTRL_DIR_FACTOR_DUTY_ADDON : 0;
    fanduty_mid += (fi[0].status & FANCTRL_DIR_FACTOR) ? FANCTRL_DIR_FACTOR_DUTY_ADDON : 0;

    for (i = (THERMAL_1_ON_MAIN_BROAD); i <= (THERMAL_3_ON_MAIN_BROAD); i++) {
        num_of_sensor++;
        temp_avg += ti[i-1].mcelsius;
    }

    temp_avg /= num_of_sensor;

    if (temp_avg >= 45000) {
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }
    else if (temp_avg >= 40000) {
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), fanduty_mid);
    }
    else if (temp_avg < 35000) {
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), fanduty_min);
    }

    return ONLP_STATUS_OK;
}

typedef int (*fan_control_policy)(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                  onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                  int *adjusted);

fan_control_policy fan_control_policies[] = {
sysi_fanctrl_fan_fault_policy,
sysi_fanctrl_fan_absent_policy,
sysi_fanctrl_fan_unknown_speed_policy,
sysi_fanctrl_single_thermal_sensor_policy,
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
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    /* Get thermal sensor status
     */
    for (i = 0; i < CHASSIS_THERMAL_COUNT; i++) {
        rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+1), &ti[i]);
        
        if (rc != ONLP_STATUS_OK) {
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
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

