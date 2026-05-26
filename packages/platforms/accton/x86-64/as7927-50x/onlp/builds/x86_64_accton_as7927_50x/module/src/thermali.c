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
    "/sys/devices/platform/coretemp.0*temp1_input",
    "/sys/devices/platform/coretemp.0*temp2_input",
    "/sys/devices/platform/coretemp.0*temp3_input",
    "/sys/devices/platform/coretemp.0*temp4_input",
    "/sys/devices/platform/coretemp.0*temp5_input",
    "/sys/devices/platform/as7927_50x_thermal*temp1_input",
    "/sys/devices/platform/as7927_50x_thermal*temp2_input",
    "/sys/devices/platform/as7927_50x_thermal*temp3_input",
    "/sys/devices/platform/as7927_50x_thermal*temp4_input",
    "/sys/devices/platform/as7927_50x_thermal*temp5_input",
    "/sys/devices/platform/as7927_50x_thermal*temp6_input",
    "/sys/devices/platform/as7927_50x_thermal*temp7_input",
    "/sys/devices/platform/as7927_50x_thermal*temp8_input",
    "/sys/devices/platform/as7927_50x_thermal*temp9_input",
    "/sys/devices/platform/as7927_50x_thermal*temp10_input",
    "/sys/devices/platform/as7927_50x_thermal*temp11_input",
    "/sys/devices/platform/as7927_50x_thermal*temp12_input",
    "/sys/devices/platform/as7927_50x_psu*psu1_temp1_input",
    "/sys/devices/platform/as7927_50x_psu*psu1_temp2_input",
    "/sys/devices/platform/as7927_50x_psu*psu1_temp3_input",
    "/sys/devices/platform/as7927_50x_psu*psu2_temp1_input",
    "/sys/devices/platform/as7927_50x_psu*psu2_temp2_input",
    "/sys/devices/platform/as7927_50x_psu*psu2_temp3_input"
};

typedef struct {
    int warning;
    int error;
    int shutdown;
} platform_thermal_thresholds_t;

typedef struct {
    platform_thermal_thresholds_t f2b;
    platform_thermal_thresholds_t b2f;
} thermal_dir_thresholds_t;

static thermal_dir_thresholds_t threshold_dict[] = {
    [8]  = { {45100, 50100, 53100}, {50750, 55750, 58750} }, /* MB_FrontLeft */
    [9]  = { {43900, 48900, 51900}, {47130, 52130, 55130} }, /* I/OB */
    [10] = { {45300, 50300, 53300}, {48250, 53250, 56250} }, /* MB_FrontRight */
    [11] = { {47000, 52000, 55000}, {52380, 57380, 60380} }, /* MB_RearLeft */
    [12] = { {78300, 83300, 86300}, {80130, 85130, 88130} }, /* MAC_DiodeCore */
    [13] = { {85600, 90600, 93600}, {88060, 93060, 96060} }, /* MAC_DiodeNif100 */
    [14] = { {83500, 88500, 91500}, {85560, 90560, 93560} }, /* MAC_DiodeNif50 */
    [15] = { {82900, 87900, 90900}, {85310, 90310, 93310} }, /* MAC_DiodeSch */
    [16] = { {46600, 51600, 54600}, {45000, 50000, 53000} }, /* FB_FrontRight */
    [17] = { {43900, 48900, 51900}, {44380, 49380, 52380} }  /* FB_FrontLeft */
};

typedef struct {
    platform_thermal_thresholds_t dc;
    platform_thermal_thresholds_t ac;
} thermal_psu_type_thresholds_t;

static thermal_psu_type_thresholds_t psu_threshold_dict[] = {
    [1] = { { 83000,  86000,  89000}, {100000, 105000, 108000} },  /* PSU_TEMP1 */
    [2] = { {100000, 105000, 108000}, {110000, 115000, 118000} },  /* PSU_TEMP2 */
    [3] = { {110000, 115000, 118000}, { 70000,  75000,  78000} }   /* PSU_TEMP3 */
};

static onlp_thermal_info_t tinfo_base[] = {
    { }, /* Not used */
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_CPU_CORE), "CPU DIE", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, {61000, 66000, 69000} },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_CPU_CORE), "CPU Core 1", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, {61000, 66000, 69000} },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_CPU_CORE), "CPU Core 2", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, {61000, 66000, 69000} },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_4_CPU_CORE), "CPU Core 3", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, {61000, 66000, 69000} },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_5_CPU_CORE), "CPU Core 4", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, {61000, 66000, 69000} },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_CARRIER_BROAD), "CB_RearLefttemp(0x48)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_CARRIER_BROAD), "CB_FrontLeft_temp(0x49)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "MB_FrontLeft_temp(0x4A)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_IO_BROAD), "I/OB_temp(0x49)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "MB_FrontRight_temp(0x4C)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD), "MB_RearLeft_temp(0x49)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_7_ON_MAC_BROAD), "MAC_DiodeCore_temp(0x49)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_8_ON_MAC_BROAD), "MAC_DiodeNif100_temp(0x49)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_9_ON_MAC_BROAD), "MAC_DiodeNif50_temp(0x49)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_10_ON_MAC_BROAD), "MAC_DiodeSch_temp(0x49)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_FAN_BROAD), "FB_FrontRight_temp(0x4D)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_FAN_BROAD), "FB_FrontLeft_temp(0x4E)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU1), "PSU-1 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU1_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_PSU1), "PSU-1 Thermal Sensor 3", ONLP_PSU_ID_CREATE(PSU1_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU2), "PSU-2 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU2_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_PSU2), "PSU-2 Thermal Sensor 3", ONLP_PSU_ID_CREATE(PSU2_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS }
};

/*
 * This will be called to initialize the thermali subsystem.
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
    int psu_id;
    int psu_type;
    int fan_dir;
    int sensor_idx;
    platform_thermal_thresholds_t *th;

    VALIDATE(id);

    tid = ONLP_OID_ID_GET(id);
    if (tid <= 0 || tid >= THERMAL_COUNT) {
        return ONLP_STATUS_E_INVALID;
    }

    *info = tinfo_base[tid];

    if (tid >= 8 && tid <= 17) {
        fan_dir = onlp_get_fan_dir(1);
        th = (fan_dir == FAN_DIR_B2F) ? &threshold_dict[tid].b2f : &threshold_dict[tid].f2b;

        info->thresholds.warning = th->warning;
        info->thresholds.error = th->error;
        info->thresholds.shutdown = th->shutdown;
    } 
    else if (tid >= 18 && tid <= 23) {
        psu_id = (tid <= 20) ? 1 : 2;
        psu_type = onlp_get_psu_type(psu_id); 

        sensor_idx = (tid - 18) % 3 + 1;
        th = (psu_type == PSU_TYPE_DC) ? &psu_threshold_dict[sensor_idx].dc : &psu_threshold_dict[sensor_idx].ac;

        info->thresholds.warning = th->warning;
        info->thresholds.error = th->error;
        info->thresholds.shutdown = th->shutdown;
    }

    return onlp_file_read_int(&info->mcelsius, devfiles__[tid]);
}