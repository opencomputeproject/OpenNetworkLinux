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

#define PATH_LENGTH 50
#define PSU_CAPS ONLP_PSU_CAPS_GET_VIN|ONLP_PSU_CAPS_GET_VOUT|ONLP_PSU_CAPS_GET_IIN|ONLP_PSU_CAPS_GET_IOUT|ONLP_PSU_CAPS_GET_PIN| ONLP_PSU_CAPS_GET_POUT

typedef enum hwmon_psu_state_e {
    HWMON_PSU_NORMAL = 0,
    HWMON_PSU_UNPOWERED = 2,       //010
    HWMON_PSU_FAULT = 4,           //100
    HWMON_PSU_NOT_INSTALLED = 7    //111
} hwmon_psu_state_t;

/*
 * Get all information about the given PSU oid.
 */
#define MAKE_PSU_NODE_INFO(id)                                      \
    {                                                               \
        {                                                    \
            ONLP_PSU_ID_CREATE(ONLP_PSU_##id), "PSU-"#id, ONLP_OID_CHASSIS,   \
	        {                                          \
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_1_ON_PSU##id),  \
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_2_ON_PSU##id),  \
                ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_##id)               \
            }, 0,                                            \
        },                                                          \
        "", "",  PSU_CAPS, ONLP_PSU_TYPE_DC12,                      \
    }

static onlp_psu_info_t __onlp_psu_info[] = {
    {},
    MAKE_PSU_NODE_INFO(1),
    MAKE_PSU_NODE_INFO(2)
};
int inv_psui_status_get(onlp_oid_t id, uint32_t* rv);

int
onlp_psui_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, ONLP_PSU_MAX-1);
}

int
onlp_psui_info_get(onlp_oid_id_t id, onlp_psu_info_t* info)
{
    int ret   = ONLP_STATUS_OK;
    onlp_oid_hdr_t *hdr;

    *info = __onlp_psu_info[id]; /* Set the onlp_oid_hdr_t */

    hdr = &info->hdr;
    ret = onlp_psui_hdr_get( id, hdr);
    if(ret != ONLP_STATUS_OK) {
        return ret;
    }
    if( !(hdr->status&ONLP_OID_STATUS_FLAG_PRESENT) ) {
        return ret;
    }

    char*  info_path=hwmon_path(INV_HWMON_BASE);
    char *list1[2]= {info->model,info->serial};
    int* list2[8]= {0,0,&info->mvin,&info->mvout,&info->miin,&info->miout,&info->mpin,&info->mpout};
    char* path_list[8]= {"vendor","serial","vin","vout","iin","iout","pin","pout"};

    int i=0;
    for (i=0; i<8; i++) {
        char path[ONLP_CONFIG_INFO_STR_MAX];
        snprintf(path,ONLP_CONFIG_INFO_STR_MAX,"%spsoc_psu%d_%s",info_path,id,path_list[i]);
        if(i<2) {
            ONLP_TRY(onlp_file_read_str_dst( list1[i], ONLP_CONFIG_INFO_STR_MAX, path ));
        } else {
            ONLP_TRY(onlp_file_read_int( list2[i], path ));
        }
    }
    return ret;
}


int inv_psui_status_get(onlp_oid_id_t id, uint32_t* rv)
{
    int result = ONLP_STATUS_OK;
    hwmon_psu_state_t psu_state;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    char*  info_path=hwmon_path(INV_HWMON_BASE);
    char path[PATH_LENGTH];
    snprintf(path, PATH_LENGTH ,"%spsu%d",info_path, id);
    ONLP_TRY(onlp_file_read((uint8_t*)&buf, ONLP_CONFIG_INFO_STR_MAX, &len, path));
    psu_state = (uint8_t)strtoul(buf, NULL, 0);
    if( psu_state == HWMON_PSU_UNPOWERED) {
        *rv = ONLP_OID_STATUS_FLAG_PRESENT |ONLP_OID_STATUS_FLAG_UNPLUGGED;
    } else if ( psu_state == HWMON_PSU_NORMAL) {
        *rv = ONLP_OID_STATUS_FLAG_PRESENT;
    } else if( psu_state == HWMON_PSU_FAULT) {
        *rv = ONLP_OID_STATUS_FLAG_PRESENT |ONLP_OID_STATUS_FLAG_FAILED;
    } else if( psu_state == HWMON_PSU_NOT_INSTALLED) {
        *rv = 0;
    } else {
        result = ONLP_STATUS_E_INVALID;
    }
    return result;
}

int onlp_psui_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    int result = ONLP_STATUS_OK;
    onlp_psu_info_t* info;

    info = &__onlp_psu_info[id];
    *hdr  = info->hdr;
    result = inv_psui_status_get( id , &hdr->status);

    return result;
}


int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

