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
#include <stdio.h>
#include <string.h>
#include "platform_lib.h"
#include <onlp/platformi/psui.h>
#include <onlplib/file.h>


#define PSUI_PLATFORM_PSU_MODEL "DPS-800AB-37D"

typedef enum psoc_psu_state_e {
    PSOC_PSU_NORMAL = 0,
    PSOC_PSU_NA_1,            //001
    PSOC_PSU_UNPOWERED,       //010
    PSOC_PSU_NA_3,            //011
    PSOC_PSU_FAULT,           //100
    PSOC_PSU_NA_5,            //101
    PSOC_PSU_NA_6,            //110
    PSOC_PSU_NOT_INSTALLED    //111
} psoc_psu_state_t;


typedef struct psui_info_s {
    char vendor[ONLP_CONFIG_INFO_STR_MAX];
    char serial[ONLP_CONFIG_INFO_STR_MAX];
    char state[ONLP_CONFIG_INFO_STR_MAX];
    char vin[ONLP_CONFIG_INFO_STR_MAX];
    char vout[ONLP_CONFIG_INFO_STR_MAX];
    char iin[ONLP_CONFIG_INFO_STR_MAX];
    char iout[ONLP_CONFIG_INFO_STR_MAX];
    char pin[ONLP_CONFIG_INFO_STR_MAX];
    char pout[ONLP_CONFIG_INFO_STR_MAX];
} psui_info_t;


static psui_info_t __info_list[ONLP_PSU_COUNT] = {
    {},
    {
        "/sys/class/hwmon/hwmon1/device/psoc_psu1_vendor",
        "/sys/class/hwmon/hwmon1/device/psoc_psu1_serial",
        "/sys/class/hwmon/hwmon1/device/psu1",
        "/sys/class/hwmon/hwmon1/device/psoc_psu1_vin",
        "/sys/class/hwmon/hwmon1/device/psoc_psu1_vout",
        "/sys/class/hwmon/hwmon1/device/psoc_psu1_iin",
        "/sys/class/hwmon/hwmon1/device/psoc_psu1_iout",
        "/sys/class/hwmon/hwmon1/device/psoc_psu1_pin",
        "/sys/class/hwmon/hwmon1/device/psoc_psu1_pout"
    },
    {
        "/sys/class/hwmon/hwmon1/device/psoc_psu2_vendor",
        "/sys/class/hwmon/hwmon1/device/psoc_psu2_serial",
        "/sys/class/hwmon/hwmon1/device/psu2",
        "/sys/class/hwmon/hwmon1/device/psoc_psu2_vin",
        "/sys/class/hwmon/hwmon1/device/psoc_psu2_vout",
        "/sys/class/hwmon/hwmon1/device/psoc_psu2_iin",
        "/sys/class/hwmon/hwmon1/device/psoc_psu2_iout",
        "/sys/class/hwmon/hwmon1/device/psoc_psu2_pin",
        "/sys/class/hwmon/hwmon1/device/psoc_psu2_pout"
    }
};


/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t __onlp_psu_info[ONLP_PSU_COUNT] = {
    { }, /* Not used */
    {
        .hdr = {
            .id = ONLP_PSU_ID_CREATE(ONLP_PSU_1),
            .description = "PSU-1",
            .poid = ONLP_OID_CHASSIS,
            .coids = {
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_1_ON_PSU1),
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_2_ON_PSU1),
                ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_1)
            },
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .model = "",
        .serial = "",
        .caps = ONLP_PSU_CAPS_GET_VIN|ONLP_PSU_CAPS_GET_VOUT|ONLP_PSU_CAPS_GET_IIN|ONLP_PSU_CAPS_GET_IOUT|ONLP_PSU_CAPS_GET_PIN|ONLP_PSU_CAPS_GET_POUT,
        .type = ONLP_PSU_TYPE_DC12,
    },
    {
        .hdr = {
            .id = ONLP_PSU_ID_CREATE(ONLP_PSU_2),
            .description = "PSU-2",
            .coids = {
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_1_ON_PSU2),
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_2_ON_PSU2),
                ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_2)
            },
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .model = "",
        .serial = "",
        .caps = ONLP_PSU_CAPS_GET_VIN|ONLP_PSU_CAPS_GET_VOUT|ONLP_PSU_CAPS_GET_IIN|ONLP_PSU_CAPS_GET_IOUT|ONLP_PSU_CAPS_GET_PIN|ONLP_PSU_CAPS_GET_POUT,
        .type = ONLP_PSU_TYPE_DC12,
    }
};


int
onlp_psui_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, ONLP_PSU_MAX-1);
}


int
onlp_psui_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    psoc_psu_state_t psoc_state;

    *hdr =  __onlp_psu_info[id].hdr;

    ONLP_TRY(onlp_file_read_int((int*)&psoc_state, __info_list[id].state));

    if( PSOC_PSU_UNPOWERED == psoc_state) {
        ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
        ONLP_OID_STATUS_FLAG_SET(hdr, UNPLUGGED);
    } else if ( PSOC_PSU_NORMAL == psoc_state) {
        ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
    } else if( PSOC_PSU_FAULT == psoc_state) {
        ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
        ONLP_OID_STATUS_FLAG_SET(hdr, FAILED);
    } else {
        ONLP_OID_STATUS_FLAGS_CLR(hdr);
    }

    return ONLP_STATUS_OK;
}

int
onlp_psui_info_get(onlp_oid_id_t id, onlp_psu_info_t* info)
{
    ONLP_TRY(onlp_psui_hdr_get(id, &info->hdr));

    /* model */
    ONLP_TRY(onlp_file_read_str_dst(info->model, sizeof(info->model), __info_list[id].vendor));

    /* serial */
    ONLP_TRY(onlp_file_read_str_dst(info->serial, sizeof(info->serial), __info_list[id].serial));

    /*millivolts*/
    ONLP_TRY(onlp_file_read_int(&info->mvin, __info_list[id].vin));
    ONLP_TRY(onlp_file_read_int(&info->mvout, __info_list[id].vout));

    /* milliamps */
    ONLP_TRY(onlp_file_read_int(&info->miin, __info_list[id].iin));
    ONLP_TRY(onlp_file_read_int(&info->miout, __info_list[id].iout));

    /* milliwatts */
    ONLP_TRY(onlp_file_read_int(&info->mpin, __info_list[id].pin));
    ONLP_TRY(onlp_file_read_int(&info->mpout, __info_list[id].pout));

    return ONLP_STATUS_OK;
}
