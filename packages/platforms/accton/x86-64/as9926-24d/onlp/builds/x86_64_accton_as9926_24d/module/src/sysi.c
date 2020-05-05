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

#include "x86_64_accton_as9926_24d_int.h"
#include "x86_64_accton_as9926_24d_log.h"

#define CPLD_VERSION_FORMAT            "/sys/bus/i2c/devices/%s/version"
#define NUM_OF_CPLD                 4

static char* cpld_path[NUM_OF_CPLD] =
{
 "19-0068", /* CPLD-1 */
 "20-0061", /* CPLD-2 */
 "21-0062", /* CPLD-3 */
 "17-0066", /* Fan board CPLD */
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as9926-24d-r0";
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
    
    /* 8 Thermal sensors on the chassis */
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

enum onlp_fan_duty_cycle_percentage
{
    FAN_PERCENTAGE_MIN = 50,
	FAN_PERCENTAGE_MID = 75,
    FAN_PERCENTAGE_MAX = 100
};

/* The tA/tB/tC/tD/tCritical is defined in the thermal policy spec
 */
typedef struct temp_sensor_threshold {
	int  tid;
    int  tA;
    int  tB;
    int  tC;
    int  tD;
    int  tSd; /* Shutdown threshold */
} temp_sensor_threshold_t;

/* The thermal plan table is for F2B(Front to back) airflow direction
 */
static const temp_sensor_threshold_t temp_sensor_threshold_f2b[] = {
{THERMAL_2_ON_MAIN_BROAD, 		57000, 52000, 62000, 79000, 84000},
{THERMAL_3_ON_MAIN_BROAD, 		57000, 52000, 63000, 83000, 88000},
};

/* The thermal plan table is for B2F(Back to front) airflow direction
 */
static const temp_sensor_threshold_t temp_sensor_threshold_b2f[] = {
{THERMAL_2_ON_MAIN_BROAD, 		57000, 52000, 62000, 79000, 84000},
{THERMAL_3_ON_MAIN_BROAD, 		57000, 52000, 63000, 83000, 88000},
};

static int
sysi_fanctrl_fan_status_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                              onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                              int *adjusted)
{
	int i;
    *adjusted = 0;

    /* Bring fan speed to FAN_PERCENTAGE_MAX if any fan is not operational */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (!(fi[i].status & ONLP_FAN_STATUS_FAILED)) {
            continue;
        }

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_MAX);
    }

    /* Bring fan speed to FAN_PERCENTAGE_MAX if fan is not present */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (fi[i].status & ONLP_FAN_STATUS_PRESENT) {
            continue;
        }

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_MAX);
    }

    return ONLP_STATUS_OK;
}

static int
sysi_fanctrl_thermal_sensor_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                           onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                           int *adjusted)
{
    int i = 0, fanduty, thermal_count = 0;
    const temp_sensor_threshold_t *pThreshold = NULL;
    int aboveTa = 0, aboveTb = 0, aboveTc = 0, aboveTd = 0, aboveSd = 0, newPercentage = 0;

    *adjusted = 0;

    if (onlp_file_read_int(&fanduty, FAN_NODE(fan_duty_cycle_percentage)) < 0) {
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_MAX);
    }


    /* Apply thermal plan by air flow direction 
     */
    if (fi[0].status & ONLP_FAN_STATUS_F2B) {
        pThreshold = temp_sensor_threshold_f2b;
        thermal_count = AIM_ARRAYSIZE(temp_sensor_threshold_f2b);
    }
    else {
        pThreshold = temp_sensor_threshold_b2f;
        thermal_count = AIM_ARRAYSIZE(temp_sensor_threshold_b2f);
    }


    /* Get temperature from each thermal sensor 
     */
    for (i = 0; i < thermal_count; i++) {
        aboveTa += (ti[pThreshold[i].tid-1].mcelsius > pThreshold[i].tA);
        aboveTb += (ti[pThreshold[i].tid-1].mcelsius > pThreshold[i].tB);
        aboveTc += (ti[pThreshold[i].tid-1].mcelsius > pThreshold[i].tC);
        aboveTd += (ti[pThreshold[i].tid-1].mcelsius > pThreshold[i].tD);
        aboveSd += (ti[pThreshold[i].tid-1].mcelsius > pThreshold[i].tSd);
    }

    /* Determine if temperature above the shutdown threshold 
    */
    if (aboveSd) {
        char* path = "/sys/bus/i2c/devices/19-0060/shutdown";

        if (onlp_file_write_int(1, "/sys/bus/i2c/devices/19-0060/shutdown") != 0) {
            AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
            return ONLP_STATUS_E_INTERNAL;
        }

        return 0;
    }

    /* Adjust fan speed based on current temperature if fan speed changed
     */
	switch (fanduty) {
		case FAN_PERCENTAGE_MIN:
			newPercentage = aboveTc ? FAN_PERCENTAGE_MID : 0;
			break;
		case FAN_PERCENTAGE_MID:
	        if (aboveTd) {
	            newPercentage = FAN_PERCENTAGE_MAX;
	        }          
	        else if (!aboveTb) {
	            newPercentage = FAN_PERCENTAGE_MIN;
	        }
			break;
		case FAN_PERCENTAGE_MAX:
			newPercentage = (!aboveTa) ? FAN_PERCENTAGE_MID : 0;
			break;
		default:
            newPercentage = FAN_PERCENTAGE_MAX;
            break;
	};

    if (newPercentage != 0) {
        *adjusted = 1;
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), newPercentage);
    }
    
    return 0;
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
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_MAX);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    /* Get thermal sensor status
     */
    for (i = 0; i < CHASSIS_THERMAL_COUNT; i++) {
        rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+1), &ti[i]);
        
        if (rc != ONLP_STATUS_OK) {
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_MAX);
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

