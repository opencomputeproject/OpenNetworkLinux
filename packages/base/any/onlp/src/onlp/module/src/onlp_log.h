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

#ifndef __ONLP_LOG_H__
#define __ONLP_LOG_H__

#define AIM_LOG_MODULE_NAME onlp
#include <AIM/aim_log.h>

/* <auto.start.enum(onlp_log_flag).header> */
/** onlp_log_flag */
typedef enum onlp_log_flag_e {
    ONLP_LOG_FLAG_JSON,
    ONLP_LOG_FLAG_LAST = ONLP_LOG_FLAG_JSON,
    ONLP_LOG_FLAG_COUNT,
    ONLP_LOG_FLAG_INVALID = -1,
} onlp_log_flag_t;

/** Strings macro. */
#define ONLP_LOG_FLAG_STRINGS \
{\
    "JSON", \
}
/** Enum names. */
const char* onlp_log_flag_name(onlp_log_flag_t e);

/** Enum values. */
int onlp_log_flag_value(const char* str, onlp_log_flag_t* e, int substr);

/** Enum descriptions. */
const char* onlp_log_flag_desc(onlp_log_flag_t e);

/** validator */
#define ONLP_LOG_FLAG_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= ONLP_LOG_FLAG_JSON))

/** onlp_log_flag_map table. */
extern aim_map_si_t onlp_log_flag_map[];
/** onlp_log_flag_desc_map table. */
extern aim_map_si_t onlp_log_flag_desc_map[];
/* <auto.end.enum(onlp_log_flag).header> */

/* <auto.start.aim_custom_log_macro(ALL).header> */

/******************************************************************************
 *
 * Custom Module Log Macros
 *
 *****************************************************************************/

/** Log a module-level json */
#define ONLP_LOG_MOD_JSON(...) \
    AIM_LOG_MOD_CUSTOM(ONLP_LOG_FLAG_JSON, "JSON", __VA_ARGS__)
/** Log a module-level json with ratelimiting */
#define ONLP_LOG_MOD_RL_JSON(_rl, _time, ...)           \
    AIM_LOG_MOD_RL_CUSTOM(ONLP_LOG_FLAG_JSON, "JSON", _rl, _time, __VA_ARGS__)

/******************************************************************************
 *
 * Custom Object Log Macros
 *
 *****************************************************************************/

/** Log an object-level json */
#define ONLP_LOG_OBJ_JSON(_obj, ...) \
    AIM_LOG_OBJ_CUSTOM(_obj, ONLP_LOG_FLAG_JSON, "JSON", __VA_ARGS__)
/** Log an object-level json with ratelimiting */
#define ONLP_LOG_OBJ_RL_JSON(_obj, _rl, _time, ...) \
    AIM_LOG_OBJ_RL_CUSTOM(_obj, ONLP_LOG_FLAG_JSON, "JSON", _rl, _time, __VA_ARGS__)

/******************************************************************************
 *
 * Default Macro Mappings
 *
 *****************************************************************************/
#ifdef AIM_LOG_OBJ_DEFAULT

/** JSON -> OBJ_JSON */
#define ONLP_LOG_JSON ONLP_LOG_OBJ_JSON
/** RL_JSON -> OBJ_RL_JSON */
#define ONLP_LOG_RL_JSON ONLP_LOG_RL_OBJ_JSON


#else

/** JSON -> MOD_JSON */
#define ONLP_LOG_JSON ONLP_LOG_MOD_JSON
/** RL_JSON -> MOD_RL_JSON */
#define ONLP_LOG_RL_JSON ONLP_LOG_MOD_RL_JSON

#endif
/* <auto.end.aim_custom_log_macro(ALL).header> */

int onlp_vlog_error(uint32_t flags, int rv, const char* fmt, va_list vargs);
int onlp_log_error(uint32_t flags, int rv, const char* fmt, ...);

#endif /* __ONLP_LOG_H__ */
