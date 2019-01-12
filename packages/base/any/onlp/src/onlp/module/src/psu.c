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
 * Power Supply Management.
 *
 ***********************************************************/

#include <onlp/onlp.h>
#include <onlp/oids.h>
#include <onlp/psu.h>
#include <onlp/platformi/psui.h>
#include "onlp_int.h"
#include "onlp_locks.h"

static int
onlp_psu_sw_init_locked__(void)
{
    return onlp_psui_sw_init();
}
ONLP_LOCKED_API0(onlp_psu_sw_init);

static int
onlp_psu_hw_init_locked__(uint32_t flags)
{
    return onlp_psui_hw_init(flags);
}
ONLP_LOCKED_API1(onlp_psu_hw_init, uint32_t, flags);

static int
onlp_psu_sw_denit_locked__(void)
{
    return onlp_psui_sw_denit();
}
ONLP_LOCKED_API0(onlp_psu_sw_denit);

static int
onlp_psu_hdr_get_locked__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    int rv;
    onlp_oid_id_t id;

    ONLP_OID_PSU_VALIDATE_GET_ID(oid, id);
    ONLP_PTR_VALIDATE_ZERO(hdr);

    rv = onlp_log_error(0,
                        onlp_psui_hdr_get(id, hdr),
                        "psui hdr get %{onlp_oid}", oid);
    hdr->id = oid;
    return rv;
}
ONLP_LOCKED_API2(onlp_psu_hdr_get, onlp_oid_t, oid, onlp_oid_hdr_t*, hdr);

static int
onlp_psu_info_get_locked__(onlp_oid_t oid,  onlp_psu_info_t* info)
{
    int rv;
    onlp_oid_id_t id;

    ONLP_OID_PSU_VALIDATE_GET_ID(oid, id);
    ONLP_PTR_VALIDATE_ZERO(info);
    rv = onlp_log_error(0,
                        onlp_psui_info_get(id, info),
                        "psui info get %{onlp_oid}", oid);
    info->hdr.id = oid;
    return rv;
}
ONLP_LOCKED_API2(onlp_psu_info_get, onlp_oid_t, oid, onlp_psu_info_t*, info);

int
onlp_psu_info_to_user_json(onlp_psu_info_t* info, cJSON** cjp, uint32_t flags)
{
    int rv;
    cJSON* object;

    rv = onlp_info_to_user_json_create(&info->hdr, &object, flags);
    if(rv > 0) {

        if(info->model[0]) {
            cjson_util_add_string_to_object(object, "Model", info->model);
        }

        if(info->serial[0]) {
            cjson_util_add_string_to_object(object, "Serial", info->serial);
        }

        if(ONLP_PSU_INFO_CAP_IS_SET(info, GET_TYPE)) {
            cjson_util_add_string_to_object(object, "Type", "%{onlp_psu_type}",
                                            info->type);
        }

#define _MILLIFIELD(_cap, _name, _field)                                \
        if(ONLP_PSU_INFO_CAP_IS_SET(info, _cap)) {                      \
            cjson_util_add_string_to_object(object, _name, "%d.%d",     \
                                            ONLP_MILLI_NORMAL_INTEGER_TENTHS(info->_field)); \
        }

        _MILLIFIELD(GET_VIN,  "Vin",  mvin);
        _MILLIFIELD(GET_VOUT, "Vout", mvout);
        _MILLIFIELD(GET_IIN,  "Iin",  miin);
        _MILLIFIELD(GET_IOUT, "Iout", miout);
        _MILLIFIELD(GET_PIN,  "Pin",  mpin);
        _MILLIFIELD(GET_POUT, "Pout", mpout);
#undef _MILLIFIELD
    }

    return onlp_info_to_user_json_finish(&info->hdr, object, cjp, flags);
}

int
onlp_psu_info_to_json(onlp_psu_info_t* info, cJSON** cjp, uint32_t flags)
{
    int rv;
    cJSON* cj;

    int unsupported = (flags & ONLP_OID_JSON_FLAG_UNSUPPORTED_FIELDS);

    if(ONLP_FAILURE(rv = onlp_info_to_json_create(&info->hdr, &cj, flags))) {
        return rv;
    }
    cJSON_AddItemToObject(cj, "caps", cjson_util_flag_array(info->caps,
                                                            onlp_psu_caps_map));
    if(ONLP_PSU_INFO_CAP_IS_SET(info, GET_TYPE)) {
        cjson_util_add_string_to_object(cj, "type", "%{onlp_psu_type}",
                                        info->type);
    }

#define _FIELD(_cap, _field)                                  \
    if(ONLP_PSU_INFO_CAP_IS_SET(info, _cap) || unsupported) { \
        cJSON_AddNumberToObject(cj, #_field, info->_field);   \
    }

    _FIELD(GET_VIN, mvin);
    _FIELD(GET_VOUT, mvout);
    _FIELD(GET_IIN, miin);
    _FIELD(GET_IOUT, miout);
    _FIELD(GET_PIN, mpin);
    _FIELD(GET_POUT, mpout);
#undef _FIELD

    return onlp_info_to_json_finish(&info->hdr, cj, cjp, flags);
}

int
onlp_psu_info_from_json(cJSON* cj, onlp_psu_info_t* info)
{
    cJSON* j;

    ONLP_IF_ERROR_RETURN(onlp_oid_hdr_from_json(cj, &info->hdr));
    ONLP_IF_ERROR_RETURN(cjson_util_lookup(cj, &j, "caps"));
    ONLP_IF_ERROR_RETURN(cjson_util_array_to_flags(j, &info->caps,
                                                   onlp_psu_caps_map));

    if(ONLP_PSU_INFO_CAP_IS_SET(info, GET_VIN)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj, &info->mvin, "mvin"));
    }

    if(ONLP_PSU_INFO_CAP_IS_SET(info, GET_VOUT)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj, &info->mvout, "mvout"));
    }

    if(ONLP_PSU_INFO_CAP_IS_SET(info, GET_IIN)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj, &info->miin, "miin"));
    }

    if(ONLP_PSU_INFO_CAP_IS_SET(info, GET_IOUT)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj, &info->miout, "miout"));
    }

    if(ONLP_PSU_INFO_CAP_IS_SET(info, GET_PIN)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj, &info->mpin, "mpin"));
    }

    if(ONLP_PSU_INFO_CAP_IS_SET(info, GET_POUT)) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_int(cj, &info->mpout, "mpout"));
    }
    return 0;
}
