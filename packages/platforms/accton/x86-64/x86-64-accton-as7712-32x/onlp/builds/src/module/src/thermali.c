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
#include <onlp/platformi/base.h>
#include "platform_lib.h"

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BOARD,
    THERMAL_2_ON_MAIN_BOARD,
    THERMAL_3_ON_MAIN_BOARD,
    THERMAL_4_ON_MAIN_BOARD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

static char* devfiles__[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    NULL,                  /* CPU_CORE files */
    "/sys/bus/i2c/devices/3-0048*temp1_input",
    "/sys/bus/i2c/devices/3-0049*temp1_input",
    "/sys/bus/i2c/devices/3-004a*temp1_input",
    "/sys/bus/i2c/devices/3-004b*temp1_input",
    "/sys/bus/i2c/devices/11-005b*psu_temp1_input",
    "/sys/bus/i2c/devices/10-0058*psu_temp1_input",
};

static char* cpu_coretemp_files[] =
    {
        "/sys/devices/platform/coretemp.0*temp2_input",
        "/sys/devices/platform/coretemp.0*temp3_input",
        "/sys/devices/platform/coretemp.0*temp4_input",
        "/sys/devices/platform/coretemp.0*temp5_input",
        NULL,
    };

/* Static values */
static onlp_thermal_info_t thermal_info_table__[] = {
    { }, /* Not used */
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_CPU_CORE, "CPU Core"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_1_ON_MAIN_BOARD, "Chassis Thermal Sensor 1"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_2_ON_MAIN_BOARD, "Chassis Thermal Sensor 2"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_3_ON_MAIN_BOARD, "Chassis Thermal Sensor 3"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_4_ON_MAIN_BOARD, "Chassis Thermal Sensor 4"),
    ONLP_PSU_THERMAL_INFO_ENTRY_INIT(THERMAL_1_ON_PSU1, "PSU-1 Thermal Sensor 1", PSU1_ID),
    ONLP_PSU_THERMAL_INFO_ENTRY_INIT(THERMAL_1_ON_PSU2, "PSU-2 Thermal Sensor 1", PSU2_ID),
};

int
onlp_thermali_info_get(onlp_oid_id_t id, onlp_thermal_info_t* info)
{
    ONLP_OID_INFO_ASSIGN(id, thermal_info_table__, info);
    if(id == THERMAL_CPU_CORE) {
        return onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
    }
    else {
        return onlp_file_read_int(&info->mcelsius, devfiles__[id]);
    }
}

int
onlp_thermali_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    *hdr = thermal_info_table__[id].hdr;
    return 0;
}
