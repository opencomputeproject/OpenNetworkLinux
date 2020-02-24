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

#include "x86_64_accton_as5835_54t_int.h"
#include "x86_64_accton_as5835_54t_log.h"

#define CPLD_VERSION_FORMAT            "/sys/bus/i2c/devices/%s/version"
#define NUM_OF_CPLD                 4

static char* cpld_path[NUM_OF_CPLD] =
{
 "3-0060", /* CPLD-1 */
 "3-0061", /* CPLD-2 */
 "3-0062", /* CPLD-3 */
 "3-0063", /* Fan board CPLD */
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as5835-54t-r0";
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

    for (i = 0; i < AIM_ARRAYSIZE(cpld_path); i++) {
        v[i] = 0;

        if(onlp_file_read_int(v+i, CPLD_VERSION_FORMAT , cpld_path[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    pi->cpld_versions = aim_fstrdup("%d.%d.%d.%d", v[0], v[1], v[2], v[3]);
    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

#define FAN_DUTY_LEVEL_4  (100)
#define FAN_DUTY_LEVEL_3  (80)
#define FAN_DUTY_LEVEL_2  (65)
#define FAN_DUTY_LEVEL_1  (40)

static int
sysi_fanctrl_fan_status_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                              onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                              int *adjusted)
{
	int i;
    *adjusted = 0;

    /* Bring fan speed to FAN_DUTY_LEVEL_4 if any fan is not operational */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (!(fi[i].status & ONLP_FAN_STATUS_FAILED)) {
            continue;
        }

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_4);
    }

    /* Bring fan speed to FAN_DUTY_LEVEL_4 if fan is not present */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (fi[i].status & ONLP_FAN_STATUS_PRESENT) {
            continue;
        }

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_4);
    }

    return ONLP_STATUS_OK;
}

static int
sysi_fanctrl_thermal_sensor_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                           onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                           int *adjusted)
{
    int i, num_of_sensor = 0, temp_avg = 0;
    int fanduty;

	*adjusted = 0;

    for (i = (THERMAL_2_ON_MAIN_BROAD); i <= (THERMAL_3_ON_MAIN_BROAD); i++) {
        num_of_sensor++;
        temp_avg += ti[i-1].mcelsius;
    }

    temp_avg /= num_of_sensor;

    if (onlp_file_read_int(&fanduty, FAN_NODE(fan_duty_cycle_percentage)) < 0) {
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_4);
    }

    switch (fanduty) {
	case FAN_DUTY_LEVEL_1:
        if (temp_avg >= 49500) {
            *adjusted = 1;
            return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_2);
        }
		break;
	case FAN_DUTY_LEVEL_2:
        if (temp_avg >= 53000) {
            *adjusted = 1;
            return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_3);
        }
        else if (temp_avg <= 42700) {
            *adjusted = 1;
            return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_1);
        }
		break;
	case FAN_DUTY_LEVEL_3:
        if (temp_avg >= 57700) {
            *adjusted = 1;
            return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_4);
        }
        else if (temp_avg <= 47700) {
            *adjusted = 1;
            return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_2);
        }
		break;
	case FAN_DUTY_LEVEL_4:
        if (temp_avg <= 52700) {
            *adjusted = 1;
            return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_3);
        }
		break;
	default:
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_4);
    }

    return ONLP_STATUS_OK;
}

typedef int (*fan_control_policy)(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                  onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                  int *adjusted);

fan_control_policy fan_control_policies[] = {
sysi_fanctrl_fan_status_policy,
sysi_fanctrl_thermal_sensor_policy,
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
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_4);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    /* Get thermal sensor status
     */
    for (i = 0; i < CHASSIS_THERMAL_COUNT; i++) {
        rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+1), &ti[i]);
        
        if (rc != ONLP_STATUS_OK) {
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_LEVEL_4);
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

