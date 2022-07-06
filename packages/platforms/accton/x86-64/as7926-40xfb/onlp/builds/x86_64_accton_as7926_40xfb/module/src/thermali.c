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

enum onlp_thermal_id {
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAINBOARD,  /* Main Board Bottom TMP432_1 Temp */
    THERMAL_2_ON_MAINBOARD,  /* Main Board Bottom TMP432_2 Temp */
    THERMAL_3_ON_MAINBOARD,  /* Main Board Bottom TMP432_3 Temp */
    THERMAL_4_ON_MAINBOARD,  /* Main Board Bottom LM75_1 Temp */
    THERMAL_5_ON_MAINBOARD,  /* Main Board Bottom LM75_2 Temp */
    THERMAL_6_ON_MAINBOARD,  /* Main Board Bottom LM75_3 Temp */
    THERMAL_7_ON_MAINBOARD,  /* Main Board Bottom LM75_4 Temp */
    THERMAL_8_ON_MAINBOARD,  /* Main Board Bottom LM75_5 Temp */
    THERMAL_9_ON_MAINBOARD,  /* Main Board Bottom LM75_6 Temp */
    THERMAL_10_ON_MAINBOARD, /* Main Board Bottom LM75_7 Temp */
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
    ONLP_THERMAL_MAX
};

static char* ipmi_devfiles__[] = { /* must map with onlp_thermal_id */
    NULL,
    NULL, /* CPU_CORE files */
    "/sys/devices/platform/as7926_40xfb_thermal/temp1_input",
    "/sys/devices/platform/as7926_40xfb_thermal/temp2_input",
    "/sys/devices/platform/as7926_40xfb_thermal/temp3_input",
    "/sys/devices/platform/as7926_40xfb_thermal/temp4_input",
    "/sys/devices/platform/as7926_40xfb_thermal/temp5_input",
    "/sys/devices/platform/as7926_40xfb_thermal/temp6_input",
    "/sys/devices/platform/as7926_40xfb_thermal/temp7_input",
    "/sys/devices/platform/as7926_40xfb_thermal/temp8_input",
    "/sys/devices/platform/as7926_40xfb_thermal/temp9_input",
    "/sys/devices/platform/as7926_40xfb_thermal/temp10_input",
    "/sys/devices/platform/as7926_40xfb_psu/psu1_temp1_input",
    "/sys/devices/platform/as7926_40xfb_psu/psu2_temp1_input",
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
      .thresholds = { 90000, 95000, 100000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAINBOARD),
          .description = "Main Board TMP432_1",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 84000, 87000, 92000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAINBOARD),
          .description = "Main Board TMP432_2",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 110000, 115000, 120000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAINBOARD),
          .description = "Main Board TMP432_3",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 110000, 115000, 120000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAINBOARD),
          .description = "Main Board LM75_1",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 70000, 73000, 78000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAINBOARD),
          .description = "Main Board LM75_2",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 72000, 75000, 80000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAINBOARD),
          .description = "Main Board LM75_3",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 73000, 76000, 81000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_7_ON_MAINBOARD),
          .description = "Main Board LM75_4",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 70000, 73000, 78000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_8_ON_MAINBOARD),
          .description = "Main Board LM75_5",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 62000, 65000, 70000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_9_ON_MAINBOARD),
          .description = "Main Board LM75_6",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 84000, 87000, 92000 }
    },
    {
      .hdr = {
          .id = ONLP_THERMAL_ID_CREATE(THERMAL_10_ON_MAINBOARD),
          .description = "Main Board LM75_7",
          .poid = ONLP_OID_CHASSIS,
          .status = ONLP_OID_STATUS_FLAG_PRESENT
      },
      .caps = ONLP_THERMAL_CAPS_ALL,
      .mcelsius = 0,
      .thresholds = { 76000, 79000, 84000 }
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
      .thresholds = { 62000, 67000, 72000 }
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
      .thresholds = { 62000, 67000, 72000 }
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
    else if (tid >= THERMAL_1_ON_PSU1 && tid <= THERMAL_1_ON_PSU2) {
        int val = 0;
        int pid = (tid - THERMAL_1_ON_PSU1 + 1);

        ONLP_TRY(onlp_file_read_int(&val, "%s""psu%d_present", PSU_SYSFS_PATH, pid));
        if (val != PSU_STATUS_PRESENT) {
            ONLP_OID_STATUS_FLAG_CLR(info, PRESENT);
            return ONLP_STATUS_OK;
        }
    }

    return onlp_file_read_int(&info->mcelsius, ipmi_devfiles__[tid]);
}

int
onlp_thermali_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    int tid = ONLP_OID_ID_GET(id);
    *hdr = thermal_info_table__[tid].hdr;

    if (tid >= THERMAL_1_ON_PSU1 && tid <= THERMAL_1_ON_PSU2) {
        int val = 0;
        int pid = (tid - THERMAL_1_ON_PSU1 + 1);

        ONLP_TRY(onlp_file_read_int(&val, "%s""psu%d_present", PSU_SYSFS_PATH, pid));
        if (val != PSU_STATUS_PRESENT) {
            ONLP_OID_STATUS_FLAG_CLR(hdr, PRESENT);
        }
    }

    return ONLP_STATUS_OK;
}

int
onlp_thermali_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, ONLP_THERMAL_MAX-1);
}
