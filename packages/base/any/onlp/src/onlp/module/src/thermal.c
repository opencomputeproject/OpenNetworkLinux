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
 * Thermal Sensor Management.
 *
 ************************************************************/
#include <onlp/thermal.h>
#include <onlp/platformi/thermali.h>
#include <onlp/oids.h>
#include "onlp_int.h"
#include "onlp_locks.h"
#include "onlp_log.h"

static int
onlp_thermal_sw_init_locked__(void)
{
    return onlp_thermali_sw_init();
}
ONLP_LOCKED_API0(onlp_thermal_sw_init);


static int
onlp_thermal_hw_init_locked__(uint32_t flags)
{
    return onlp_thermali_hw_init(flags);
}
ONLP_LOCKED_API1(onlp_thermal_hw_init, uint32_t, flags)

static int
onlp_thermal_sw_denit_locked__(void)
{
    return onlp_thermali_sw_denit();
}
ONLP_LOCKED_API0(onlp_thermal_sw_denit);

static int
onlp_thermal_hdr_get_locked__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    int rv;
    onlp_oid_id_t id;

    ONLP_OID_THERMAL_VALIDATE_GET_ID(oid, id);
    ONLP_PTR_VALIDATE_ZERO(hdr);

    rv = onlp_log_error(0,
                        onlp_thermali_hdr_get(id, hdr),
                        "thermali hdr get %{onlp_oid}", oid);
    hdr->id = oid;
    return rv;
}
ONLP_LOCKED_API2(onlp_thermal_hdr_get, onlp_oid_t, id, onlp_oid_hdr_t*, hdr);


static int
onlp_thermal_info_get_locked__(onlp_oid_t oid, onlp_thermal_info_t* info)
{
    int rv;
    onlp_oid_id_t id;

    ONLP_OID_THERMAL_VALIDATE_GET_ID(oid, id);
    ONLP_PTR_VALIDATE_ZERO(info);

    rv = onlp_log_error(0,
                        onlp_thermali_info_get(id, info),
                        "thermali info get %{onlp_oid}", oid);
    info->hdr.id = oid;
    return rv;
}
ONLP_LOCKED_API2(onlp_thermal_info_get, onlp_oid_t, oid, onlp_thermal_info_t*, info);

int
onlp_thermal_info_to_user_json(onlp_thermal_info_t* info, cJSON** cjp, uint32_t flags)
{
    int rv;
    cJSON* object;

    rv = onlp_info_to_user_json_create(&info->hdr, &object, flags);
    if(rv > 0) {

#define _MILLIFIELD(_cap, _name, _field)                                \
        if(ONLP_THERMAL_INFO_CAP_IS_SET(info, _cap)) {                  \
            cjson_util_add_string_to_object(object, _name, "%d.%d",     \
                                            ONLP_MILLI_NORMAL_INTEGER_TENTHS(info->_field)); \
        }                                                               \

        _MILLIFIELD(GET_TEMPERATURE, "Temperature", mcelsius);
    }

    return onlp_info_to_user_json_finish(&info->hdr, object, cjp, flags);

}

int
onlp_thermal_info_to_json(onlp_thermal_info_t* info, cJSON** cjp, uint32_t flags)
{
    cJSON* cj;

    ONLP_IF_ERROR_RETURN(onlp_info_to_json_create(&info->hdr, &cj, flags));
    cJSON_AddItemToObject(cj, "caps", cjson_util_flag_array(info->caps,
                                                            onlp_thermal_caps_map));
    if(ONLP_THERMAL_INFO_CAP_IS_SET(info, GET_TEMPERATURE)) {
        cJSON_AddNumberToObject(cj, "mcelsius", info->mcelsius);
    }
    if(ONLP_THERMAL_INFO_CAP_IS_SET(info, GET_WARNING_THRESHOLD)) {
        cJSON_AddNumberToObject(cj, "warning-threshold",
                                info->thresholds.warning);
    }
    if(ONLP_THERMAL_INFO_CAP_IS_SET(info, GET_ERROR_THRESHOLD)) {
        cJSON_AddNumberToObject(cj, "error-threshold",
                                info->thresholds.error);
    }
    if(ONLP_THERMAL_INFO_CAP_IS_SET(info, GET_SHUTDOWN_THRESHOLD)) {
        cJSON_AddNumberToObject(cj, "shutdown-threshold",
                                info->thresholds.shutdown);
    }
    return onlp_info_to_json_finish(&info->hdr, cj, cjp, flags);
}

int
onlp_thermal_info_from_json(cJSON* cj, onlp_thermal_info_t* info)
{
    cJSON* j;
    memset(info, 0, sizeof(*info));

    ONLP_IF_ERROR_RETURN(onlp_oid_hdr_from_json(cj, &info->hdr));
    ONLP_IF_ERROR_RETURN(cjson_util_lookup(cj, &j, "caps"));
    ONLP_IF_ERROR_RETURN(cjson_util_array_to_flags(j, &info->caps,
                                                   onlp_thermal_caps_map));
    if(ONLP_THERMAL_INFO_CAP_IS_SET(info, GET_TEMPERATURE)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj, &info->mcelsius, "mcelsius"));
    }
    if(ONLP_THERMAL_INFO_CAP_IS_SET(info, GET_WARNING_THRESHOLD)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj,
                                                   &info->thresholds.warning,
                                                   "warning-threshold"));
    }
    if(ONLP_THERMAL_INFO_CAP_IS_SET(info, GET_ERROR_THRESHOLD)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj,
                                                   &info->thresholds.error,
                                                   "error-threshold"));
    }
    if(ONLP_THERMAL_INFO_CAP_IS_SET(info, GET_SHUTDOWN_THRESHOLD)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj,
                                                   &info->thresholds.shutdown,
                                                   "shutdown-threshold"));
    }

    return 0;
}
