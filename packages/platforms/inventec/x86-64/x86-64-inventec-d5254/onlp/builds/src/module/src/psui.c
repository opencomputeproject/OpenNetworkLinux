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

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

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
        {
            ONLP_PSU_ID_CREATE(ONLP_PSU_1), "PSU-1", 0,
            {
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_1_ON_PSU1),
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_2_ON_PSU1),
                ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_1)
            }
        },
        "","",ONLP_PSU_STATUS_PRESENT,
        ONLP_PSU_CAPS_DC12|ONLP_PSU_CAPS_VIN|ONLP_PSU_CAPS_VOUT|ONLP_PSU_CAPS_IIN|ONLP_PSU_CAPS_IOUT|ONLP_PSU_CAPS_PIN|ONLP_PSU_CAPS_POUT
    },
    {
        {
            ONLP_PSU_ID_CREATE(ONLP_PSU_2), "PSU-2", 0,
            {
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_1_ON_PSU2),
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_2_ON_PSU2),
                ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_2)
            }
        },
        "","",ONLP_PSU_STATUS_PRESENT,
        ONLP_PSU_CAPS_DC12|ONLP_PSU_CAPS_VIN|ONLP_PSU_CAPS_VOUT|ONLP_PSU_CAPS_IIN|ONLP_PSU_CAPS_IOUT|ONLP_PSU_CAPS_PIN|ONLP_PSU_CAPS_POUT
    }
};


int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}


int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int ret   = ONLP_STATUS_OK;
    int len;
    int local_id = ONLP_OID_ID_GET(id);
    uint8_t temp[ONLP_CONFIG_INFO_STR_MAX] = {0};
    psoc_psu_state_t psoc_state;

    VALIDATE(id);

    if(local_id >= ONLP_PSU_MAX) {
        return ONLP_STATUS_E_INVALID;
    }


    *info = __onlp_psu_info[local_id]; /* Set the onlp_oid_hdr_t */

    ret = onlp_file_read(temp, ONLP_CONFIG_INFO_STR_MAX, &len, __info_list[local_id].vendor);
    /*remove the '\n'*/
    temp[strlen((char*)temp)-1] = 0;
    snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "%s", temp);


    memset(temp, 0, ONLP_CONFIG_INFO_STR_MAX);
    ret = onlp_file_read(temp, ONLP_CONFIG_INFO_STR_MAX, &len,__info_list[local_id].serial);
    /*remove the '\n'*/
    temp[strlen((char*)temp)-1] = 0;
    snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "%s", temp);


    ret = onlp_file_read_int((int*)&psoc_state, __info_list[local_id].state);

    if( PSOC_PSU_UNPOWERED == psoc_state) {
        info->status = ONLP_PSU_STATUS_PRESENT|ONLP_PSU_STATUS_UNPLUGGED;
    } else if ( PSOC_PSU_NORMAL == psoc_state) {
        info->status = ONLP_PSU_STATUS_PRESENT;
    } else if( PSOC_PSU_FAULT == psoc_state) {
        info->status = ONLP_PSU_STATUS_PRESENT|ONLP_PSU_STATUS_FAILED;
    } else {
        info->status = 0;
    }

    /*millivolts*/
    ret = onlp_file_read_int(&info->mvin, __info_list[local_id].vin);
    ret = onlp_file_read_int(&info->mvout, __info_list[local_id].vout);

    /* milliamps */
    ret = onlp_file_read_int(&info->miin, __info_list[local_id].iin);
    ret = onlp_file_read_int(&info->miout, __info_list[local_id].iout);

    /* milliwatts */
    ret = onlp_file_read_int(&info->mpin, __info_list[local_id].pin);
    ret = onlp_file_read_int(&info->mpout, __info_list[local_id].pout);

    return ret;
}


/**
 * @brief Get the PSU's operational status.
 * @param id The PSU OID.
 * @param rv [out] Receives the operational status.
 */
int onlp_psui_status_get(onlp_oid_t id, uint32_t* rv)
{
    int result = ONLP_STATUS_OK;
    psoc_psu_state_t psoc_state;
    int local_id;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    if(local_id >= ONLP_PSU_MAX) {
        result = ONLP_STATUS_E_INVALID;
    } else {
        result = onlp_file_read_int((int*)&psoc_state, __info_list[local_id].state);

        if( PSOC_PSU_UNPOWERED == psoc_state) {
            *rv = ONLP_PSU_STATUS_PRESENT|ONLP_PSU_STATUS_UNPLUGGED;
        } else if ( PSOC_PSU_NORMAL == psoc_state) {
            *rv = ONLP_PSU_STATUS_PRESENT;
        } else if( PSOC_PSU_FAULT == psoc_state) {
            *rv = ONLP_PSU_STATUS_PRESENT|ONLP_PSU_STATUS_FAILED;
        } else {
            *rv = 0;
        }
    }
    return result;
}

/**
 * @brief Get the PSU's oid header.
 * @param id The PSU OID.
 * @param rv [out] Receives the header.
 */
int onlp_psui_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv)
{
    int result = ONLP_STATUS_OK;
    onlp_psu_info_t* info;
    int local_id;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    if(local_id >= ONLP_PSU_MAX) {
        result = ONLP_STATUS_E_INVALID;
    } else {
        info = &__onlp_psu_info[local_id];
        *rv = info->hdr;
    }
    return result;
}


int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

