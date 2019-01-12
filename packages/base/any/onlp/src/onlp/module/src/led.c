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
 * LED Management
 *
 ***********************************************************/
#include <onlp/onlp.h>
#include <onlp/oids.h>
#include <onlp/led.h>
#include <onlp/platformi/ledi.h>
#include "onlp_int.h"
#include "onlp_locks.h"

static int
onlp_led_sw_init_locked__(void)
{
    return onlp_ledi_sw_init();
}
ONLP_LOCKED_API0(onlp_led_sw_init);


static int
onlp_led_hw_init_locked__(uint32_t flags)
{
    return onlp_ledi_hw_init(flags);
}
ONLP_LOCKED_API1(onlp_led_hw_init, uint32_t, flags);

static int
onlp_led_sw_denit_locked__(void)
{
    return onlp_ledi_sw_denit();
}
ONLP_LOCKED_API0(onlp_led_sw_denit);


static int
onlp_led_hdr_get_locked__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    ONLP_OID_LED_VALIDATE(oid);
    ONLP_PTR_VALIDATE_ZERO(hdr);
    return onlp_log_error(0x0,
                          onlp_ledi_hdr_get(oid, hdr),
                          "ledi hdr get %{onlp_oid}", oid);
}
ONLP_LOCKED_API2(onlp_led_hdr_get, onlp_oid_t, id, onlp_oid_hdr_t*, hdr);


static int
onlp_led_info_get_locked__(onlp_oid_t oid, onlp_led_info_t* info)
{
    ONLP_OID_LED_VALIDATE(oid);
    ONLP_PTR_VALIDATE_ZERO(info);
    return onlp_log_error(0x0,
                          onlp_ledi_info_get(oid, info),
                          "ledi info get %{onlp_oid}", oid);
}
ONLP_LOCKED_API2(onlp_led_info_get, onlp_oid_t, id, onlp_led_info_t*, info);


static int
onlp_led_caps_get_locked__(onlp_oid_t oid, uint32_t* rv)
{
    onlp_oid_id_t id;

    ONLP_OID_LED_VALIDATE_GET_ID(oid, id);
    ONLP_PTR_VALIDATE_ZERO(rv);

    return onlp_log_error(0,
                          onlp_ledi_caps_get(id, rv),
                          "ledi caps get %{onlp_oid}", oid);
}
ONLP_LOCKED_API2(onlp_led_caps_get, onlp_oid_t, oid, uint32_t*, rv);


static int
onlp_led_mode_set_locked__(onlp_oid_t oid, onlp_led_mode_t mode)
{
    uint32_t caps = 0;
    onlp_oid_id_t id;

    ONLP_OID_LED_VALIDATE_GET_ID(oid, id);

    ONLP_TRY(onlp_log_error(0,
                            onlp_led_caps_get_locked__(oid, &caps),
                            "led mode set %{onlp_oid} %{onlp_led_mode}: could not get led caps.",
                            oid, mode));

    if(caps & (1 << mode)) {
        return onlp_ledi_mode_set(id, mode);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_led_mode_set, onlp_oid_t, id, onlp_led_mode_t, mode);

static int
onlp_led_char_set_locked__(onlp_oid_t oid, char c)
{
    uint32_t caps = 0;
    onlp_oid_id_t id;

    ONLP_OID_LED_VALIDATE_GET_ID(oid, id);

    ONLP_TRY(onlp_log_error(0,
                            onlp_led_caps_get_locked__(oid, &caps),
                            "led char set %{onlp_oid} %c: could not get led caps.",
                            oid, c));

    if(caps & ONLP_LED_CAPS_CHAR) {
        return onlp_ledi_char_set(id, c);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_led_char_set, onlp_oid_t, id, char, c);


int
onlp_led_info_to_user_json(onlp_led_info_t* info, cJSON** cjp, uint32_t flags)
{

    int rv;
    cJSON* cj;
    rv = onlp_info_to_user_json_create(&info->hdr, &cj, flags);
    if(rv > 0) {
        cjson_util_add_string_to_object(cj, "Mode", "%{onlp_led_mode}", info->mode);
        if(info->mode == ONLP_LED_MODE_CHAR) {
            cjson_util_add_string_to_object(cj, "Character", "%c", info->character);
        }
    }
    return onlp_info_to_user_json_finish(&info->hdr, cj, cjp, flags);
}

int
onlp_led_info_to_json(onlp_led_info_t* info, cJSON** cjp, uint32_t flags)
{
    cJSON* cj;

    ONLP_IF_ERROR_RETURN(onlp_info_to_json_create(&info->hdr, &cj, flags));
    cJSON_AddItemToObject(cj, "caps", cjson_util_flag_array(info->caps,
                                                            onlp_led_caps_map));
    cjson_util_add_string_to_object(cj, "mode",
                                    "%{onlp_led_mode}", info->mode);

    if(info->mode == ONLP_LED_MODE_CHAR) {
        cjson_util_add_string_to_object(cj, "character", "%c", info->character);
    }

    return onlp_info_to_json_finish(&info->hdr, cj, cjp, flags);
}

int
onlp_led_info_from_json(cJSON* cj, onlp_led_info_t* info)
{
    cJSON* j;
    ONLP_IF_ERROR_RETURN(onlp_oid_hdr_from_json(cj, &info->hdr));
    ONLP_IF_ERROR_RETURN(cjson_util_lookup(cj, &j, "caps"));
    ONLP_IF_ERROR_RETURN(cjson_util_array_to_flags(j, &info->caps,
                                                   onlp_led_caps_map));
    char* s;
    ONLP_IF_ERROR_RETURN(cjson_util_lookup_string(cj, &s, "mode"));
    ONLP_IF_ERROR_RETURN(onlp_led_mode_value(s, &info->mode, 1));

    if(info->mode == ONLP_LED_MODE_CHAR) {
        ONLP_IF_ERROR_RETURN(cjson_util_lookup_string(cj, &s, "character"));
        info->character = s[0];
    }

    return 0;
}
