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
 * Fan Management.
 *
 ***********************************************************/
#include <onlp/fan.h>
#include <onlp/platformi/fani.h>
#include <onlp/oids.h>
#include "onlp_int.h"
#include "onlp_locks.h"
#include "onlp_log.h"
#include "onlp_json.h"
#include <cjson_util/cjson_util_format.h>

/**
 * Fan Software Init
 */
static int
onlp_fan_sw_init_locked__(void)
{
    return onlp_fani_sw_init();
}
ONLP_LOCKED_API0(onlp_fan_sw_init)

/**
 * Fan Hardware Init
 */
static int
onlp_fan_hw_init_locked__(uint32_t flags)
{
    return onlp_fani_hw_init(flags);
}
ONLP_LOCKED_API1(onlp_fan_hw_init, uint32_t, flags);

static int
onlp_fan_sw_denit_locked__(void)
{
    return onlp_fani_sw_denit();
}
ONLP_LOCKED_API0(onlp_fan_sw_denit);

/**
 * Fan Header Get
 */
static int
onlp_fan_hdr_get_locked__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    int rv;
    onlp_oid_id_t id;

    ONLP_OID_FAN_VALIDATE_GET_ID(oid, id);
    ONLP_PTR_VALIDATE_ZERO(hdr);
    ONLP_IF_ERROR_RETURN(onlp_fani_id_validate(id));

    rv = onlp_log_error(0,
                        onlp_fani_hdr_get(id, hdr),
                        "fani hdr get %{onlp_oid}", oid);
    hdr->id = oid;
    return rv;
}
ONLP_LOCKED_API2(onlp_fan_hdr_get, onlp_oid_t, oid, onlp_oid_hdr_t*, hdr);

/**
 * Fan Info Get
 */
static int
onlp_fan_info_get_locked__(onlp_oid_t oid, onlp_fan_info_t* info)
{
    int rv;
    onlp_oid_id_t id;

    ONLP_OID_FAN_VALIDATE_GET_ID(oid, id);
    ONLP_PTR_VALIDATE_ZERO(info);
    ONLP_IF_ERROR_RETURN(onlp_fani_id_validate(id));

    rv = onlp_log_error(0,
                        onlp_fani_info_get(id, info),
                        "fani info get %{onlp_oid}", oid);

    info->hdr.id = oid;
    return rv;
}
ONLP_LOCKED_API2(onlp_fan_info_get, onlp_oid_t, oid, onlp_fan_info_t*, info);

/**
 * Fan Caps Get
 */
static int
onlp_fan_caps_get_locked__(onlp_oid_t oid, uint32_t* rv)
{
    onlp_oid_id_t id;

    ONLP_OID_FAN_VALIDATE_GET_ID(oid, id);
    ONLP_PTR_VALIDATE_ZERO(rv);
    ONLP_IF_ERROR_RETURN(onlp_fani_id_validate(id));

    return onlp_log_error(0,
                          onlp_fani_caps_get(id, rv),
                          "fani caps get %{onlp_oid}", oid);
}
ONLP_LOCKED_API2(onlp_fan_caps_get, onlp_oid_t, oid, uint32_t*, rv);


static int
onlp_fan_rpm_set_locked__(onlp_oid_t oid, int rpm)
{
    uint32_t caps = 0;
    onlp_oid_id_t id;


    ONLP_OID_FAN_VALIDATE_GET_ID(oid, id);
    ONLP_IF_ERROR_RETURN(onlp_fani_id_validate(id));

    ONLP_TRY(onlp_log_error(0,
                            onlp_fan_caps_get_locked__(oid, &caps),
                            "fan rpm set %{onlp_oid} %d: could not get fan caps",
                            oid, rpm));

    if(caps & ONLP_FAN_CAPS_SET_RPM) {
        return onlp_log_error(0,
                              onlp_fani_rpm_set(id, rpm),
                              "fani rpm set %{onlp_oid} %d", oid, rpm);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_fan_rpm_set, onlp_oid_t, id, int, rpm);

static int
onlp_fan_percentage_set_locked__(onlp_oid_t oid, int p)
{
    uint32_t caps = 0;
    onlp_oid_id_t id;

    ONLP_OID_FAN_VALIDATE_GET_ID(oid, id);
    ONLP_IF_ERROR_RETURN(onlp_fani_id_validate(id));

    ONLP_TRY(onlp_log_error(0,
                            onlp_fan_caps_get_locked__(oid, &caps),
                            "fan percentage set %{onlp_oid} %d: could not get fan caps",
                            oid, p));

    if(caps & ONLP_FAN_CAPS_SET_PERCENTAGE) {
        return onlp_fani_percentage_set(id, p);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_fan_percentage_set, onlp_oid_t, id, int, p);

static int
onlp_fan_dir_set_locked__(onlp_oid_t oid, onlp_fan_dir_t dir)
{
    onlp_oid_id_t id;
    uint32_t caps = 0;

    ONLP_OID_FAN_VALIDATE_GET_ID(oid, id);
    ONLP_IF_ERROR_RETURN(onlp_fani_id_validate(id));

    ONLP_TRY(onlp_log_error(0,
                            onlp_fani_caps_get(id, &caps),
                            "fan dir set %{onlp_oid} %{onlp_fan_dir}: could not get fan caps",
                            oid, dir));

    if(caps & ONLP_FAN_CAPS_SET_DIR) {
        return onlp_fani_dir_set(id, dir);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_fan_dir_set, onlp_oid_t, id, onlp_fan_dir_t, dir);


int
onlp_fan_info_to_user_json(onlp_fan_info_t* info, cJSON** cjp, uint32_t flags)
{
    int rv;
    cJSON* object;

    rv = onlp_info_to_user_json_create(&info->hdr, &object, flags);

    if(rv > 0) {
        if(ONLP_FAN_INFO_CAP_IS_SET(info, GET_RPM)) {
            cjson_util_add_string_to_object(object, "RPM", "%d", info->rpm);
        }
        if(ONLP_FAN_INFO_CAP_IS_SET(info, GET_PERCENTAGE)) {
            cjson_util_add_string_to_object(object, "Speed", "%d%%", info->percentage);
        }
        if(ONLP_FAN_INFO_CAP_IS_SET(info, GET_DIR)) {
            switch(info->dir)
                {
                case ONLP_FAN_DIR_F2B: cjson_util_add_string_to_object(object, "Airflow", "Front-To-Back"); break;
                case ONLP_FAN_DIR_B2F: cjson_util_add_string_to_object(object, "Airflow", "Back-To-Front"); break;
                default: break;
                }
        }
    }
    return onlp_info_to_user_json_finish(&info->hdr, object, cjp, flags);
}

int
onlp_fan_info_to_json(onlp_fan_info_t* info, cJSON** cjp, uint32_t flags)
{
    cJSON* cj;
    int unsupported = (flags & ONLP_OID_JSON_FLAG_UNSUPPORTED_FIELDS);

    ONLP_IF_ERROR_RETURN(onlp_info_to_json_create(&info->hdr, &cj, flags));

    cJSON_AddItemToObject(cj, "caps", cjson_util_flag_array(info->caps,
                                                            onlp_fan_caps_map));

    if(ONLP_FAN_INFO_CAP_IS_SET(info, GET_DIR) || unsupported) {
        cjson_util_add_string_to_object(cj, "dir", "%{onlp_fan_dir}",
                                        info->dir);
    }
    if(ONLP_FAN_INFO_CAP_IS_SET(info, GET_RPM) || unsupported) {
        cJSON_AddNumberToObject(cj, "rpm", info->rpm);
    }

    if(ONLP_FAN_INFO_CAP_IS_SET(info, GET_PERCENTAGE) || unsupported) {
        cJSON_AddNumberToObject(cj, "percentage", info->percentage);
    }
    if(info->model[0] || unsupported) {
        cJSON_AddStringToObject(cj, "model", info->model);
    }
    if(info->serial[0] || unsupported) {
        cJSON_AddStringToObject(cj, "serial", info->serial);
    }

    return onlp_info_to_json_finish(&info->hdr, cj, cjp, flags);
}

int
onlp_fan_info_from_json(cJSON* cj, onlp_fan_info_t* info)
{
    cJSON* j;

    ONLP_IF_ERROR_RETURN(onlp_oid_hdr_from_json(cj, &info->hdr));
    ONLP_IF_ERROR_RETURN(cjson_util_lookup(cj, &j, "caps"));
    ONLP_IF_ERROR_RETURN(cjson_util_array_to_flags(j, &info->caps,
                                                   onlp_fan_caps_map));

    if(ONLP_FAN_INFO_CAP_IS_SET(info, GET_RPM)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj, &info->rpm, "rpm"));
    }

    if(ONLP_FAN_INFO_CAP_IS_SET(info, GET_PERCENTAGE)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj, &info->percentage,
                                                   "percentage"));
    }

    if(ONLP_FAN_INFO_CAP_IS_SET(info, GET_DIR)) {
        char* dir = NULL;
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_string(cj, &dir, "dir"));
        if(ONLP_FAILURE(onlp_fan_dir_value(dir, &info->dir, 1))) {
            ONLP_LOG_JSON("%s: '%s' is not a valid fan direction.",
                          __FUNCTION__, dir);
            return ONLP_STATUS_E_PARAM;
        }
    }

    char* s;
    if((s = cjson_util_lookup_string_default(cj, NULL, "model"))) {
        aim_strlcpy(info->model, s, sizeof(info->model));
    }
    if((s = cjson_util_lookup_string_default(cj, NULL, "serial"))) {
        aim_strlcpy(info->serial, s, sizeof(info->model));
    }

    return 0;
}
