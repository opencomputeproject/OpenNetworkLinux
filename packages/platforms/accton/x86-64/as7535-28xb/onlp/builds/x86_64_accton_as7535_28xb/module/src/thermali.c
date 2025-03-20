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
    "/sys/devices/platform/as7535_28xb_thermal/temp1_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp2_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp3_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp4_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp5_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp6_input",
    "/sys/devices/platform/as7535_28xb_psu/psu1_temp1_input",
    "/sys/devices/platform/as7535_28xb_psu/psu1_temp2_input",
    "/sys/devices/platform/as7535_28xb_psu/psu1_temp3_input",
    "/sys/devices/platform/as7535_28xb_psu/psu2_temp1_input",
    "/sys/devices/platform/as7535_28xb_psu/psu2_temp2_input",
    "/sys/devices/platform/as7535_28xb_psu/psu2_temp3_input",
};

static char* devfiles_r02__[] = { /* must map with onlp_thermal_id */
    NULL,
    NULL,                  /* CPU_CORE files */
    "/sys/devices/platform/as7535_28xb_thermal/temp1_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp2_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp3_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp4_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp5_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp6_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp7_input",
    "/sys/devices/platform/as7535_28xb_thermal/temp8_input",
    "/sys/devices/platform/as7535_28xb_psu/psu1_temp1_input",
    "/sys/devices/platform/as7535_28xb_psu/psu1_temp2_input",
    "/sys/devices/platform/as7535_28xb_psu/psu1_temp3_input",
    "/sys/devices/platform/as7535_28xb_psu/psu2_temp1_input",
    "/sys/devices/platform/as7535_28xb_psu/psu2_temp2_input",
    "/sys/devices/platform/as7535_28xb_psu/psu2_temp3_input",
};


static char* cpu_coretemp_files[] = {
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
        ONLP_THERMAL_CAPS_ALL, 0, {94000, 100000, 103000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "LM75-4B", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {74000, 79000, 84000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "TMP431_0x4C_U50", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {84000, 89000, 94000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "TMP431_0x4C_MAC", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {97000, 102000, 107000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "LM75-4D", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {87000, 92000, 97000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "LM75-4E", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {80000, 85000, 90000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD), "LM75-4F", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {80000, 85000, 90000}
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

/* Static values for R02*/
static onlp_thermal_info_t tinfo_r02[] = {
    { }, /* Not used */
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {94000, 100000, 103000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "LM75-4B", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {74000, 79000, 84000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "TMP431_0x4C_U50", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {84000, 89000, 94000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "TMP431_0x4C_MAC", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {97000, 102000, 107000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "LM75-4D", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {87000, 92000, 97000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "LM75-4E", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {80000, 85000, 90000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD), "LM75-4F", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {80000, 85000, 90000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_7_ON_MAIN_BROAD), "TMP431_0x4C_U93", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {82000, 87000, 92000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_8_ON_MAIN_BROAD), "TMP431_0x4C_C10", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {102000, 107000, 112000}
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

threshold_t threshold[CHASSIS_PSU_COUNT][NUM_OF_THERMAL_PER_PSU] = {
    [PSU_TYPE_PS_2601_6R][0].warning  = 85000,
    [PSU_TYPE_PS_2601_6R][0].error    = 90000,
    [PSU_TYPE_PS_2601_6R][0].shutdown = 95000,
    [PSU_TYPE_PS_2601_6R][1].warning  = 110000,
    [PSU_TYPE_PS_2601_6R][1].error    = 115000,
    [PSU_TYPE_PS_2601_6R][1].shutdown = 120000,
    [PSU_TYPE_PS_2601_6R][2].warning  = 95000,
    [PSU_TYPE_PS_2601_6R][2].error    = 100000,
    [PSU_TYPE_PS_2601_6R][2].shutdown = 105000,

    [PSU_TYPE_DD_2601_6R][0].warning  = 80000,
    [PSU_TYPE_DD_2601_6R][0].error    = 103000,
    [PSU_TYPE_DD_2601_6R][0].shutdown = 105000,
    [PSU_TYPE_DD_2601_6R][1].warning  = 82500,
    [PSU_TYPE_DD_2601_6R][1].error    = 105500,
    [PSU_TYPE_DD_2601_6R][1].shutdown = 107500,
    [PSU_TYPE_DD_2601_6R][2].warning  = 85000,
    [PSU_TYPE_DD_2601_6R][2].error    = 108000,
    [PSU_TYPE_DD_2601_6R][2].shutdown = 110000,
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
    VALIDATE(id);
    int pcb_id = 0;

    psu_type_t psu_mod_type = 0;
    int        psu_temp_idx = 0, psu_tid_start = 0;
    int        thermal_cnt  = 0;

    tid = ONLP_OID_ID_GET(id);

    pcb_id = get_pcb_id();

    if (pcb_id == 1) {
        *info = tinfo_r02[tid];
        thermal_cnt = CHASSIS_THERMAL_COUNT_R02;
    }
    else {
        *info = tinfo[tid];
        thermal_cnt = CHASSIS_THERMAL_COUNT;
    }

    psu_tid_start = thermal_cnt + 1;

    /*Assign psu temperature threshold value*/
    if( tid >= psu_tid_start ) {
        psu_mod_type = get_psu_type(tid, psu_tid_start, NULL, 0);

        if( psu_mod_type != PSU_TYPE_UNKNOWN )
        {
            psu_temp_idx = ( tid - psu_tid_start ) % NUM_OF_THERMAL_PER_PSU; /*0~2*/
            info->thresholds.warning  = threshold[psu_mod_type][psu_temp_idx].warning;
            info->thresholds.error    = threshold[psu_mod_type][psu_temp_idx].error;
            info->thresholds.shutdown = threshold[psu_mod_type][psu_temp_idx].shutdown;
        }
    }

    if (tid == THERMAL_CPU_CORE) {
        return onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
    }

    if (pcb_id == 1)
        return onlp_file_read_int(&info->mcelsius, devfiles_r02__[tid]);

    return onlp_file_read_int(&info->mcelsius, devfiles__[tid]);
}
