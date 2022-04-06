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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static char* devfiles__[] = { /* must map with onlp_thermal_id */
    NULL,
    NULL,                  /* CPU_CORE files */
    "/sys/bus/i2c/devices/51-0049*temp1_input",
    "/sys/bus/i2c/devices/52-004a*temp1_input",
    "/sys/bus/i2c/devices/53-004c*temp1_input",
    "/sys/bus/i2c/devices/45-005b*psu_temp2_input",
    "/sys/bus/i2c/devices/45-005b*psu_temp3_input",
    "/sys/bus/i2c/devices/44-0058*psu_temp2_input",
    "/sys/bus/i2c/devices/44-0058*psu_temp3_input",
};

static char* cpu_coretemp_files[] = {
    "/sys/devices/platform/coretemp.0*temp2_input",
    "/sys/devices/platform/coretemp.0*temp6_input",
    "/sys/devices/platform/coretemp.0*temp12_input",
    "/sys/devices/platform/coretemp.0*temp16_input",
    NULL,
};

/* Static values */
static onlp_thermal_info_t tinfo[] = {
    { }, /* Not used */
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "LM75-49", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "LM75-4A", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "LM75-4C", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU1), "PSU-1 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU1_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU2), "PSU-2 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU2_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }
};

typedef struct threshold_t {
    int warning;
    int error;
    int shutdown;
} threshold_t;

threshold_t threshold[FAN_DIR_COUNT][THERMAL_COUNT] = {
    [FAN_DIR_L2R][THERMAL_CPU_CORE].warning  = 78000,
    [FAN_DIR_L2R][THERMAL_CPU_CORE].error    = 95000,
    [FAN_DIR_L2R][THERMAL_CPU_CORE].shutdown = 105000,
    [FAN_DIR_L2R][THERMAL_1_ON_MAIN_BROAD].warning  = 80000,
    [FAN_DIR_L2R][THERMAL_1_ON_MAIN_BROAD].error    = 85000,
    [FAN_DIR_L2R][THERMAL_1_ON_MAIN_BROAD].shutdown = 90000,
    [FAN_DIR_L2R][THERMAL_2_ON_MAIN_BROAD].warning  = 76000,
    [FAN_DIR_L2R][THERMAL_2_ON_MAIN_BROAD].error    = 81000,
    [FAN_DIR_L2R][THERMAL_2_ON_MAIN_BROAD].shutdown = 86000,
    [FAN_DIR_L2R][THERMAL_3_ON_MAIN_BROAD].warning  = 74000,
    [FAN_DIR_L2R][THERMAL_3_ON_MAIN_BROAD].error    = 79000,
    [FAN_DIR_L2R][THERMAL_3_ON_MAIN_BROAD].shutdown = 84000,
    [FAN_DIR_L2R][THERMAL_1_ON_PSU1].warning  = 78000,
    [FAN_DIR_L2R][THERMAL_1_ON_PSU1].error    = 83000,
    [FAN_DIR_L2R][THERMAL_1_ON_PSU1].shutdown = 88000,
    [FAN_DIR_L2R][THERMAL_2_ON_PSU1].warning  = 80000,
    [FAN_DIR_L2R][THERMAL_2_ON_PSU1].error    = 85000,
    [FAN_DIR_L2R][THERMAL_2_ON_PSU1].shutdown = 90000,
    [FAN_DIR_L2R][THERMAL_1_ON_PSU2].warning  = 78000,
    [FAN_DIR_L2R][THERMAL_1_ON_PSU2].error    = 83000,
    [FAN_DIR_L2R][THERMAL_1_ON_PSU2].shutdown = 88000,
    [FAN_DIR_L2R][THERMAL_2_ON_PSU2].warning  = 80000,
    [FAN_DIR_L2R][THERMAL_2_ON_PSU2].error    = 85000,
    [FAN_DIR_L2R][THERMAL_2_ON_PSU2].shutdown = 90000,

    [FAN_DIR_R2L][THERMAL_CPU_CORE].warning  = 77000,
    [FAN_DIR_R2L][THERMAL_CPU_CORE].error    = 95000,
    [FAN_DIR_R2L][THERMAL_CPU_CORE].shutdown = 105000,
    [FAN_DIR_R2L][THERMAL_1_ON_MAIN_BROAD].warning  = 78000,
    [FAN_DIR_R2L][THERMAL_1_ON_MAIN_BROAD].error    = 83000,
    [FAN_DIR_R2L][THERMAL_1_ON_MAIN_BROAD].shutdown = 88000,
    [FAN_DIR_R2L][THERMAL_2_ON_MAIN_BROAD].warning  = 74000,
    [FAN_DIR_R2L][THERMAL_2_ON_MAIN_BROAD].error    = 79000,
    [FAN_DIR_R2L][THERMAL_2_ON_MAIN_BROAD].shutdown = 84000,
    [FAN_DIR_R2L][THERMAL_3_ON_MAIN_BROAD].warning  = 77000,
    [FAN_DIR_R2L][THERMAL_3_ON_MAIN_BROAD].error    = 82000,
    [FAN_DIR_R2L][THERMAL_3_ON_MAIN_BROAD].shutdown = 87000,
    [FAN_DIR_R2L][THERMAL_1_ON_PSU1].warning  = 81000,
    [FAN_DIR_R2L][THERMAL_1_ON_PSU1].error    = 86000,
    [FAN_DIR_R2L][THERMAL_1_ON_PSU1].shutdown = 91000,
    [FAN_DIR_R2L][THERMAL_2_ON_PSU1].warning  = 81000,
    [FAN_DIR_R2L][THERMAL_2_ON_PSU1].error    = 86000,
    [FAN_DIR_R2L][THERMAL_2_ON_PSU1].shutdown = 91000,
    [FAN_DIR_R2L][THERMAL_1_ON_PSU2].warning  = 81000,
    [FAN_DIR_R2L][THERMAL_1_ON_PSU2].error    = 86000,
    [FAN_DIR_R2L][THERMAL_1_ON_PSU2].shutdown = 91000,
    [FAN_DIR_R2L][THERMAL_2_ON_PSU2].warning  = 81000,
    [FAN_DIR_R2L][THERMAL_2_ON_PSU2].error    = 86000,
    [FAN_DIR_R2L][THERMAL_2_ON_PSU2].shutdown = 91000,
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    return ONLP_STATUS_OK;
}

/*
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */
int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    int tid;
    enum onlp_fan_dir dir;
    VALIDATE(id);

    tid = ONLP_OID_ID_GET(id);
    dir = onlp_get_fan_dir();

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = tinfo[tid];
    info->thresholds.warning  = threshold[dir][tid].warning;
    info->thresholds.error    = threshold[dir][tid].error;
    info->thresholds.shutdown = threshold[dir][tid].shutdown;

    if (tid == THERMAL_CPU_CORE) {
        return onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
    }

    return onlp_file_read_int(&info->mcelsius, devfiles__[tid]);
}
