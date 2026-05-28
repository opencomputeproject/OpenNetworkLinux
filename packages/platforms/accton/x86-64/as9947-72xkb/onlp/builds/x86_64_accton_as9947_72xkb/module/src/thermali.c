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
    "/sys/devices/platform/as9947_72xkb_thermal*temp1_input",
    "/sys/devices/platform/as9947_72xkb_thermal*temp2_input",
    "/sys/devices/platform/as9947_72xkb_thermal*temp3_input",
    "/sys/devices/platform/as9947_72xkb_thermal*temp4_input",
    "/sys/devices/platform/as9947_72xkb_thermal*temp5_input",
    "/sys/devices/platform/as9947_72xkb_thermal*temp6_input",
    "/sys/devices/platform/as9947_72xkb_thermal*temp7_input",
    "/sys/devices/platform/as9947_72xkb_thermal*temp8_input",
    "/sys/devices/platform/as9947_72xkb_thermal*temp9_input",
    "/sys/devices/platform/as9947_72xkb_thermal*temp10_input",
    "/sys/devices/platform/as9947_72xkb_psu*psu1_temp1_input",
    "/sys/devices/platform/as9947_72xkb_psu*psu1_temp2_input",
    "/sys/devices/platform/as9947_72xkb_psu*psu1_temp3_input",
    "/sys/devices/platform/as9947_72xkb_psu*psu2_temp1_input",
    "/sys/devices/platform/as9947_72xkb_psu*psu2_temp2_input",
    "/sys/devices/platform/as9947_72xkb_psu*psu2_temp3_input"
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
        ONLP_THERMAL_CAPS_ALL, 0, {95000, 100000, 101000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "MB_RearRight_temp(0x4F)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {70000, 75000, 76000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "MB_RearLeft_temp(0x4E)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {63000, 68000, 69000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "MB_FrontRight_temp(0x4A)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {59000, 64000, 65000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "MB_FrontLeft_temp(0x4B)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {65000, 70000, 71000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "MZB_CenterLeft_temp(0x48)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {68000, 73000, 74000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD), "MZB_FrontLeft_temp(0x49)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {77000, 82000, 83000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_7_ON_MAIN_BROAD), "MB_CenterLeft_temp(0x4C)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {67000, 72000, 73000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_8_ON_MAIN_BROAD), "MB_CenterRight_temp(0x4D)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {72000, 77000, 78000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_FAN_BROAD), "FCB_Up_temp(0x4D)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {74000, 79000, 80000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_FAN_BROAD), "FCB_Down_temp(0x4E)", 0, {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {71000, 76000, 77000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {57000, 62000, 67000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU1), "PSU-1 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU1_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {95000, 125000, 135000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_PSU1), "PSU-1 Thermal Sensor 3", ONLP_PSU_ID_CREATE(PSU1_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {95000, 135000, 145000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {57000, 62000, 67000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU2), "PSU-2 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU2_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {95000, 125000, 135000}
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_PSU2), "PSU-2 Thermal Sensor 3", ONLP_PSU_ID_CREATE(PSU2_ID), {0} },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, {95000, 135000, 145000}
    }
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
    int pid, val, ret;
    VALIDATE(id);

    tid = ONLP_OID_ID_GET(id);
    *info = tinfo[tid];

    if (tid == THERMAL_CPU_CORE) {
        return onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
    }

    switch(tid){
        case THERMAL_1_ON_PSU1:
        case THERMAL_2_ON_PSU1:
        case THERMAL_3_ON_PSU1:
        case THERMAL_1_ON_PSU2:
        case THERMAL_2_ON_PSU2:
        case THERMAL_3_ON_PSU2:
            if((tid >= THERMAL_1_ON_PSU1) && (tid <=THERMAL_3_ON_PSU1))
                pid = 1;
            else
                pid = 2;
            /* Get PSU power good status */
            ret = psu_status_info_get(pid, "power_good", &val);
            if (ret < 0)
            {
                AIM_LOG_ERROR("Unable to read PSU(%d) node(power_good)\r\n", pid);
            }

            if (val != PSU_STATUS_POWER_GOOD) {
                info->status |= ONLP_THERMAL_STATUS_FAILED;
                info->mcelsius = 0;
                return ONLP_STATUS_OK;
            }
            else
            {
                return onlp_file_read_int(&info->mcelsius, devfiles__[tid]);
            }
            break;
        default:
            break;
    }

    return onlp_file_read_int(&info->mcelsius, devfiles__[tid]);
}
