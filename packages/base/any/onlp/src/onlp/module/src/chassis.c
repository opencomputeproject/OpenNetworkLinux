/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
#include <onlp/chassis.h>
#include <onlp/platformi/chassisi.h>
#include <onlp/attribute.h>
#include <AIM/aim.h>
#include "onlp_log.h"
#include "onlp_int.h"
#include "onlp_locks.h"


static int
onlp_chassis_sw_init_locked__(void)
{
    return onlp_chassisi_sw_init();
}
ONLP_LOCKED_API0(onlp_chassis_sw_init);

static int
onlp_chassis_hw_init_locked__(uint32_t flags)
{
    return onlp_chassisi_hw_init(flags);
}
ONLP_LOCKED_API1(onlp_chassis_hw_init, uint32_t, flags);

static int
onlp_chassis_sw_denit_locked__(void)
{
    return onlp_chassisi_sw_denit();
}
ONLP_LOCKED_API0(onlp_chassis_sw_denit);

static int
onlp_chassis_hdr_get_locked__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    ONLP_OID_CHASSIS_VALIDATE(oid);
    ONLP_PTR_VALIDATE_ZERO(hdr);
    memset(hdr, 0, sizeof(*hdr));
    int rv = onlp_chassisi_hdr_get(ONLP_OID_ID_GET(oid), hdr);
    hdr->id = oid;
    onlp_oid_hdr_sort(hdr);
    return rv;
}
ONLP_LOCKED_API2(onlp_chassis_hdr_get, onlp_oid_t, oid, onlp_oid_hdr_t*, hdr);


static int
onlp_chassis_info_get_locked__(onlp_oid_t oid, onlp_chassis_info_t* cip)
{
    ONLP_OID_CHASSIS_VALIDATE(oid);
    ONLP_PTR_VALIDATE_ZERO(cip);
    int rv = onlp_chassisi_info_get(ONLP_OID_ID_GET(oid), cip);
    cip->hdr.id = oid;
    onlp_oid_hdr_sort(&cip->hdr);
    return rv;
}
ONLP_LOCKED_API2(onlp_chassis_info_get,onlp_oid_t, oid,
                 onlp_chassis_info_t*, rv);

int
onlp_chassis_info_to_user_json(onlp_chassis_info_t* info, cJSON** cjp, uint32_t flags)
{
    int rv;
    cJSON* object = NULL;
    if(ONLP_SUCCESS(rv = onlp_info_to_user_json_create(&info->hdr, &object, flags))) {
        rv = onlp_info_to_user_json_finish(&info->hdr, object, cjp, flags);
    }
    return rv;
}

int
onlp_chassis_info_to_json(onlp_chassis_info_t* info, cJSON** cjp, uint32_t flags)
{
    cJSON* cj = NULL;
    ONLP_IF_ERROR_RETURN(onlp_info_to_json_create(&info->hdr, &cj, flags));
    return onlp_info_to_json_finish(&info->hdr, cj, cjp, flags);
}

int
onlp_chassis_info_from_json(cJSON* cj, onlp_chassis_info_t* info)
{
    memset(info, 0, sizeof(*info));
    return onlp_oid_hdr_from_json(cj, &info->hdr);
}

int
onlp_chassis_environment_to_json(cJSON** cjp, uint32_t flags)
{
    int rv;
    onlp_chassis_info_t ci;
    onlp_oid_t* oidp;

    /**
     * All flags except ONLP_OID_JSON_FLAG_TO_USER_JSON are forbidden.
     */
    flags &= ONLP_OID_JSON_FLAG_TO_USER_JSON;

    rv = onlp_chassis_info_get(ONLP_OID_CHASSIS, &ci);
    if(ONLP_FAILURE(rv)) {
        AIM_LOG_ERROR("Error retrieving chassis information: %{onlp_error}", rv);
        return rv;
    }

    *cjp = cJSON_CreateObject();

    /** All Chassis Fans */
    ONLP_OID_TABLE_ITER_TYPE(ci.hdr.coids, oidp, FAN) {
        rv = onlp_oid_to_json(*oidp, cjp, flags);
        if(ONLP_FAILURE(rv)) {
            AIM_LOG_ERROR("onlp_oid_to_json(%{onlp_oid}) failed: %{onlp_status}",
                          *oidp, rv);
        }
    }

    /** All Chassis Thermals */
    ONLP_OID_TABLE_ITER_TYPE(ci.hdr.coids, oidp, THERMAL) {
        rv = onlp_oid_to_json(*oidp, cjp, flags);
        if(ONLP_FAILURE(rv)) {
            AIM_LOG_ERROR("onlp_oid_to_json(%{onlp_oid}) failed: %{onlp_status}",
                          *oidp, rv);
        }
    }

    /** All PSUs with children */
    ONLP_OID_TABLE_ITER_TYPE(ci.hdr.coids, oidp, PSU) {
        rv = onlp_oid_to_json(*oidp, cjp, flags | ONLP_OID_JSON_FLAG_RECURSIVE);
        if(ONLP_FAILURE(rv)) {
            AIM_LOG_ERROR("onlp_oid_to_json(%{onlp_oid}) failed: %{onlp_status}",
                          *oidp, rv);
        }
    }

    return rv;
}

int
onlp_chassis_environment_show(aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    cJSON* cjp = NULL;
    rv = onlp_chassis_environment_to_json(&cjp, flags);
    if(ONLP_FAILURE(rv) || cjp == NULL) {
        aim_printf(pvs, "Error retrieving chassis environment: %{onlp_status}\n",
                   rv);
    }
    else {
        cjson_util_yaml_pvs(pvs, cjp);
        cJSON_Delete(cjp);
    }
    return rv;
}

/**
 * @brief Construct the Chassis debug JSON object.
 * @param [out] rv Receives the JSON object.
 */
int
onlp_chassis_debug_get_json(cJSON** rvp)
{
    int rv;
    cJSON* cj = cJSON_CreateObject();
    cJSON* onie_info, *asset_info;

    if(ONLP_SUCCESS(rv = onlp_attribute_onie_info_get_json(ONLP_OID_CHASSIS, &onie_info))) {
        cJSON_AddItemToObject(cj, "ONIE Information", onie_info);
    }
    if(ONLP_SUCCESS(rv = onlp_attribute_asset_info_get_json(ONLP_OID_CHASSIS, &asset_info))) {
        cJSON_AddItemToObject(cj, "Asset Information", asset_info);
    }
    rv = onlp_oid_to_json(ONLP_OID_CHASSIS, &cj,
                          ONLP_OID_JSON_FLAG_RECURSIVE);
    *rvp = cj;
    return rv;
}

int
onlp_chassis_debug_show(aim_pvs_t* pvs)
{
    int rv;
    cJSON* cjp;
    if(ONLP_SUCCESS(rv = onlp_chassis_debug_get_json(&cjp))) {
        cjson_util_yaml_pvs(pvs, cjp);
        cJSON_Delete(cjp);
    }
    return rv;
}
