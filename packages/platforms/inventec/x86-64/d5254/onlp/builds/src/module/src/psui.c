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


typedef enum hwmon_psu_state_e {
    HWMON_PSU_NORMAL = 0,
    HWMON_PSU_UNPOWERED = 2,       //010
    HWMON_PSU_FAULT = 4,           //100
    HWMON_PSU_NOT_INSTALLED = 7    //111
} hwmon_psu_state_t;

/*
 * Get all information about the given PSU oid.
 */
#define MAKE_PSU_NODE_INFO(id)		\
    {					\
        {				\
            ONLP_PSU_ID_CREATE(ONLP_PSU_##id), "PSU-"#id, 0,		\
            {								\
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_1_ON_PSU##id),	\
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_2_ON_PSU##id),	\
                ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_##id)			\
            }				\
        },				\
        "","", 0, 0			\
    }

static onlp_psu_info_t __onlp_psu_info[ONLP_PSU_COUNT] = {
    MAKE_PSU_NODE_INFO(1),
    MAKE_PSU_NODE_INFO(2)
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

    VALIDATE(id);

    if(local_id >= ONLP_PSU_MAX) {
        return ONLP_STATUS_E_INVALID;
    }


    *info = __onlp_psu_info[LOCAL_ID_TO_INFO_IDX(local_id)]; /* Set the onlp_oid_hdr_t */

    ret = onlp_file_read(temp, ONLP_CONFIG_INFO_STR_MAX, &len, INV_HWMON_PREFIX"psoc_psu%d_vendor", local_id);
    if(ret != ONLP_STATUS_OK) { return ret; }
    /*remove the '\n'*/
    temp[strlen((char*)temp)-1] = 0;
    snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "%s", temp);


    memset(temp, 0, ONLP_CONFIG_INFO_STR_MAX);
    ret = onlp_file_read(temp, ONLP_CONFIG_INFO_STR_MAX, &len, INV_HWMON_PREFIX"psoc_psu%d_serial", local_id);
    if(ret != ONLP_STATUS_OK) { return ret; }
    /*remove the '\n'*/
    temp[strlen((char*)temp)-1] = 0;
    snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "%s", temp);

    ret = onlp_psui_status_get(id, &info->status);
    if(ret != ONLP_STATUS_OK) { return ret; }

    if(info->status & ONLP_PSU_STATUS_PRESENT) {
        info->caps = ONLP_PSU_CAPS_AC;
        /*millivolts*/
        ret = onlp_file_read_int(&info->mvin, INV_HWMON_PREFIX"psoc_psu%d_vin", local_id);
        if(ret != ONLP_STATUS_OK) { return ret; }
        if(info->mvin >= 0) {
            info->caps |= ONLP_PSU_CAPS_VIN;
        }
        ret = onlp_file_read_int(&info->mvout, INV_HWMON_PREFIX"psoc_psu%d_vout", local_id);
        if(ret != ONLP_STATUS_OK) { return ret; }
        if(info->mvout >= 0) {
            info->caps |= ONLP_PSU_CAPS_VOUT;
        }

        /* milliamps */
        ret = onlp_file_read_int(&info->miin, INV_HWMON_PREFIX"psoc_psu%d_iin", local_id);
        if(ret != ONLP_STATUS_OK) { return ret; }
        if(info->miin >= 0) {
            info->caps |= ONLP_PSU_CAPS_IIN;
        }
        ret = onlp_file_read_int(&info->miout, INV_HWMON_PREFIX"psoc_psu%d_iout", local_id);
        if(ret != ONLP_STATUS_OK) { return ret; }
        if(info->miout >= 0) {
            info->caps |= ONLP_PSU_CAPS_IOUT;
        }

        /* milliwatts */
        ret = onlp_file_read_int(&info->mpin, INV_HWMON_PREFIX"psoc_psu%d_pin", local_id);
        if(ret != ONLP_STATUS_OK) { return ret; }
        if(info->mpin >= 0) {
            info->caps |= ONLP_PSU_CAPS_PIN;
        }
        ret = onlp_file_read_int(&info->mpout, INV_HWMON_PREFIX"psoc_psu%d_pout", local_id);
        if(ret != ONLP_STATUS_OK) { return ret; }
        if(info->mpout >= 0) {
            info->caps |= ONLP_PSU_CAPS_POUT;
        }
    }
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
    hwmon_psu_state_t psu_state;
    int local_id;
    VALIDATE(id);
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    local_id = ONLP_OID_ID_GET(id);
    if(local_id >= ONLP_PSU_MAX) {
        result = ONLP_STATUS_E_INVALID;
    } else {
        result = onlp_file_read((uint8_t*)&buf, ONLP_CONFIG_INFO_STR_MAX, &len, "%s""psu%d", INV_HWMON_PREFIX, local_id);
        if( result != ONLP_STATUS_OK ) {return result;}
        psu_state = (uint8_t)strtoul(buf, NULL, 0);
        if( psu_state == HWMON_PSU_UNPOWERED) {
            *rv = ONLP_PSU_STATUS_PRESENT|ONLP_PSU_STATUS_UNPLUGGED;
        } else if ( psu_state == HWMON_PSU_NORMAL) {
            *rv = ONLP_PSU_STATUS_PRESENT;
        } else if( psu_state == HWMON_PSU_FAULT) {
            *rv = ONLP_PSU_STATUS_PRESENT|ONLP_PSU_STATUS_FAILED;
        } else if( psu_state == HWMON_PSU_NOT_INSTALLED) {
            *rv = 0;
        } else {
            result = ONLP_STATUS_E_INVALID;
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
        info = &__onlp_psu_info[LOCAL_ID_TO_INFO_IDX(local_id)];
        *rv = info->hdr;
    }
    return result;
}


int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

