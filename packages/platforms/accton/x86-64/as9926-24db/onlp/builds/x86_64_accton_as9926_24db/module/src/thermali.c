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
#include <onlp/platformi/thermali.h>
#include "platform_lib.h"

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_6_ON_MAIN_BROAD,
    THERMAL_7_ON_MAIN_BROAD,
    THERMAL_8_ON_MAIN_BROAD,
    THERMAL_9_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
    ONLP_THERMAL_MAX
};


static char* ipmi_devfiles__[] =  /* must map with onlp_thermal_id */
{
    NULL,
    NULL,                  /* CPU_CORE files */
    "/sys/devices/platform/as9926_24db_thermal/temp1_input",
    "/sys/devices/platform/as9926_24db_thermal/temp2_input",
    "/sys/devices/platform/as9926_24db_thermal/temp3_input",
    "/sys/devices/platform/as9926_24db_thermal/temp4_input",
    "/sys/devices/platform/as9926_24db_thermal/temp5_input",
    "/sys/devices/platform/as9926_24db_thermal/temp6_input",
    "/sys/devices/platform/as9926_24db_thermal/temp7_input",
    "/sys/devices/platform/as9926_24db_thermal/temp8_input",
    "/sys/devices/platform/as9926_24db_thermal/temp9_input",
    "/sys/devices/platform/as9926_24db_psu/psu1_temp1_input",
    "/sys/devices/platform/as9926_24db_psu/psu2_temp1_input",
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
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE),
          .description = "CPU Core",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 83000, 93000, 103000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD),
          .description = "LM75_1 U61",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 71000, 76000, 81000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD),
          .description = "LM75_2 U83",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 71000, 76000, 81000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD),
          .description = "LM75_3 U3",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 78000, 83000, 88000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD),
          .description = "LM75_Heater_CPU U20",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 66000, 71000, 76000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD),
          .description = "LM75_5 U27",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 66000, 71000, 76000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD),
          .description = "LM75_6 U80",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 56000, 61000, 66000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_7_ON_MAIN_BROAD),
          .description = "LM75_7 U64",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 59000, 64000, 69000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_8_ON_MAIN_BROAD),
          .description = "TMP432_1 U90",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 90000, 100000, 110000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_9_ON_MAIN_BROAD),
          .description = "TMP432_2 U90",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 90000, 100000, 110000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1),
          .description = "PSU-1 Thermal Sensor 1",
          .poid = ONLP_PSU_ID_CREATE(PSU1_ID),
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 74000, 80000, 85000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2),
          .description = "PSU-2 Thermal Sensor 1",
          .poid = ONLP_PSU_ID_CREATE(PSU2_ID),
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 74000, 80000, 85000 }
    }
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_sw_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_thermali_sw_denit(void)
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
    int tid = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = thermal_info_table__[tid];

    if (tid == THERMAL_CPU_CORE) {
        return onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
    }

    return onlp_file_read_int(&info->mcelsius, ipmi_devfiles__[tid]);
}

int
onlp_thermali_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    *hdr = thermal_info_table__[id].hdr;
    return 0;
}

int
onlp_thermali_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, ONLP_THERMAL_MAX-1);
}
