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
onlp_led_present__(onlp_oid_t id, onlp_led_info_t* info)
{
    int rv;

    /* Info retrieval required. */
    rv = onlp_ledi_info_get(id, info);
    if(rv < 0) {
        return rv;
    }
    /* The led must be present. */
    if((info->hdr.status & 0x1) == 0) {
        return ONLP_STATUS_E_MISSING;
    }
    return ONLP_STATUS_OK;
}
#define ONLP_LED_PRESENT_OR_RETURN(_id, _info)          \
    do {                                                \
        int _rv = onlp_led_present__(_id, _info);       \
        if(_rv < 0) {                                   \
            return _rv;                                 \
        }                                               \
    } while(0)


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
onlp_led_mode_set_locked__(onlp_oid_t id, onlp_led_mode_t mode)
{
    onlp_led_info_t info;
    ONLP_LED_PRESENT_OR_RETURN(id, &info);

    /*
     * The mode enumeration values always match
     * the capability bit positions.
     */
    if(info.caps & (1 << mode)) {
        return onlp_ledi_mode_set(id, mode);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_led_mode_set, onlp_oid_t, id, onlp_led_mode_t, mode);

static int
onlp_led_char_set_locked__(onlp_oid_t id, char c)
{
    onlp_led_info_t info;
    ONLP_LED_PRESENT_OR_RETURN(id, &info);

    /*
     * The mode enumeration values always match
     * the capability bit positions.
     */
    if(info.caps & ONLP_LED_CAPS_CHAR) {
        return onlp_ledi_char_set(id, c);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_led_char_set, onlp_oid_t, id, char, c);

/************************************************************
 *
 * Debug and Show Functions
 *
 ***********************************************************/

int
onlp_led_format(onlp_oid_t oid, onlp_oid_format_t format,
                aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    onlp_led_info_t info;

    ONLP_OID_LED_VALIDATE(oid);
    ONLP_PTR_VALIDATE(pvs);

    if(ONLP_SUCCESS(rv = onlp_led_info_get(oid, &info))) {
        rv = onlp_led_info_format(&info, format, pvs, flags);
    }

    return rv;
}

int
onlp_led_info_format(onlp_led_info_t* info, onlp_oid_format_t format,
                     aim_pvs_t* pvs, uint32_t flags)
{
    aim_printf(pvs, "%{onlp_oid_hdr} caps=%{onlp_led_caps_flags} mode=%{onlp_led_mode}\n",
               info, info->caps, info->mode);
    return 0;
}

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
