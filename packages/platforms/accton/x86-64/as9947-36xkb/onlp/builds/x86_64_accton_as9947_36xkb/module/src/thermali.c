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
    "/sys/devices/platform/as9947_36xkb_thermal*temp1_input",
    "/sys/devices/platform/as9947_36xkb_thermal*temp2_input",
    "/sys/devices/platform/as9947_36xkb_thermal*temp3_input",
    "/sys/devices/platform/as9947_36xkb_thermal*temp4_input",
    "/sys/devices/platform/as9947_36xkb_thermal*temp5_input",
    "/sys/devices/platform/as9947_36xkb_thermal*temp6_input",
    "/sys/devices/platform/as9947_36xkb_thermal*temp7_input",
    "/sys/devices/platform/as9947_36xkb_thermal*temp8_input",
    "/sys/devices/platform/as9947_36xkb_thermal*temp9_input",
    "/sys/devices/platform/as9947_36xkb_psu*psu1_temp1_input",
    "/sys/devices/platform/as9947_36xkb_psu*psu1_temp2_input",
    "/sys/devices/platform/as9947_36xkb_psu*psu1_temp3_input",
    "/sys/devices/platform/as9947_36xkb_psu*psu2_temp1_input",
    "/sys/devices/platform/as9947_36xkb_psu*psu2_temp2_input",
    "/sys/devices/platform/as9947_36xkb_psu*psu2_temp3_input"
};

static char* cpu_coretemp_files[] = {
    "/sys/devices/platform/coretemp.0*temp1_input",
    "/sys/devices/platform/coretemp.0*temp2_input",
    "/sys/devices/platform/coretemp.0*temp3_input",
    "/sys/devices/platform/coretemp.0*temp4_input",
    "/sys/devices/platform/coretemp.0*temp5_input",
    "/sys/devices/platform/coretemp.0*temp6_input",
    "/sys/devices/platform/coretemp.0*temp7_input",
    "/sys/devices/platform/coretemp.0*temp8_input",
    "/sys/devices/platform/coretemp.0*temp9_input",
    NULL,
};

/* Static values */
static onlp_thermal_info_t tinfo[] = {
    { }, /* Not used */
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {96000, 101000, 102000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "MB_RearRight_temp(0x4F)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {77000, 82000, 83000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "MB_FrontRight_temp(0x4E)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {62000, 67000, 68000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "MB_FrontLeft_temp(0x4A)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {73000, 78000, 79000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "MB_CenterCenter_temp(0x4B)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {74000, 79000, 80000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "MB_RearCenter_temp(0x4C) Local", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {72000, 77000, 78000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD), "MB_RearCenter_temp(0x4C) Remote", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {105000, 110000, 111000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_7_ON_MAIN_BROAD), "MB_FrontRight_temp(0x4D)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {78000, 83000, 84000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_FAN_BROAD), "FAN BOARD(0x4D)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {64000, 69000, 70000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_FAN_BROAD), "FAN BOARD(0x4E)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {64000, 69000, 70000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU1), "PSU-1 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU1_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_PSU1), "PSU-1 Thermal Sensor 3", ONLP_PSU_ID_CREATE(PSU1_ID), {0} },
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
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_PSU2), "PSU-2 Thermal Sensor 3", ONLP_PSU_ID_CREATE(PSU2_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }
};

typedef struct threshold_t {
	int warning;
	int error;
	int shutdown;
} threshold_t;

threshold_t threshold[PSU_TYPE_COUNT][NUM_OF_THERMAL_PER_PSU] = {
    [PSU_TYPE_PS_2202][0].warning  = 55000,
    [PSU_TYPE_PS_2202][0].error    = 60000,
    [PSU_TYPE_PS_2202][0].shutdown = 70000,
    [PSU_TYPE_PS_2202][1].warning  = 80000,
    [PSU_TYPE_PS_2202][1].error    = 99000,
    [PSU_TYPE_PS_2202][1].shutdown = 103000,
    [PSU_TYPE_PS_2202][2].warning  = 80000,
    [PSU_TYPE_PS_2202][2].error    = 95000,
    [PSU_TYPE_PS_2202][2].shutdown = 98000,

    [PSU_TYPE_DD_2202][0].warning  = 70000,
    [PSU_TYPE_DD_2202][0].error    = 75000,
    [PSU_TYPE_DD_2202][0].shutdown = 85000,
    [PSU_TYPE_DD_2202][1].warning  = 110000,
    [PSU_TYPE_DD_2202][1].error    = 130000,
    [PSU_TYPE_DD_2202][1].shutdown = 140000,
    [PSU_TYPE_DD_2202][2].warning  = 100000,
    [PSU_TYPE_DD_2202][2].error    = 122000,
    [PSU_TYPE_DD_2202][2].shutdown = 130000,
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
    int tid, psu_id, val = 0, psu_temp_idx;
    psu_type_t psu_mod_type = 0;
    VALIDATE(id);

    tid = ONLP_OID_ID_GET(id);
    *info = tinfo[tid];

    if (tid == THERMAL_CPU_CORE) {
        return onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
    }

    if (tid >= THERMAL_1_ON_PSU1 && tid <= THERMAL_3_ON_PSU2) {
        psu_id = tid-THERMAL_1_ON_PSU1 < NUM_OF_THERMAL_PER_PSU ? PSU1_ID : PSU2_ID;
        psu_mod_type = get_psu_type(psu_id);

        if( psu_mod_type != PSU_TYPE_UNKNOWN )
        {
            psu_temp_idx = ( tid - THERMAL_1_ON_PSU1 ) % NUM_OF_THERMAL_PER_PSU; /*0~2*/
            info->thresholds.warning  = threshold[psu_mod_type][psu_temp_idx].warning;
            info->thresholds.error    = threshold[psu_mod_type][psu_temp_idx].error;
            info->thresholds.shutdown = threshold[psu_mod_type][psu_temp_idx].shutdown;
        }

        /* Get power good status */
        onlp_file_read_int(&val, PSU_SYSFS_FORMAT, psu_id, "power_good");
        if(val != PSU_STATUS_POWER_GOOD) {
            info->status |= ONLP_THERMAL_STATUS_FAILED;
        }
    }
    return onlp_file_read_int(&info->mcelsius, devfiles__[tid]);
}
