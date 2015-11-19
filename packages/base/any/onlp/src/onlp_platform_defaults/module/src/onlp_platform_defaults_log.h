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

#ifndef __ONLP_PLATFORM_DEFAULTS_LOG_H__
#define __ONLP_PLATFORM_DEFAULTS_LOG_H__

#define AIM_LOG_MODULE_NAME onlp_platform_defaults
#include <AIM/aim_log.h>
#include "onlp_platform_defaults_int.h"

/* <auto.start.aim_custom_log_macro(ALL).header> */

/******************************************************************************
 *
 * Custom Module Log Macros
 *
 *****************************************************************************/

/** Log a module-level called */
#define ONLP_PLATFORM_DEFAULTS_LOG_MOD_CALLED(...) \
    AIM_LOG_MOD_CUSTOM(ONLP_PLATFORM_DEFAULTS_LOG_FLAG_CALLED, "CALLED", __VA_ARGS__)
/** Log a module-level called with ratelimiting */
#define ONLP_PLATFORM_DEFAULTS_LOG_MOD_RL_CALLED(_rl, _time, ...)           \
    AIM_LOG_MOD_RL_CUSTOM(ONLP_PLATFORM_DEFAULTS_LOG_FLAG_CALLED, "CALLED", _rl, _time, __VA_ARGS__)

/******************************************************************************
 *
 * Custom Object Log Macros
 *
 *****************************************************************************/

/** Log an object-level called */
#define ONLP_PLATFORM_DEFAULTS_LOG_OBJ_CALLED(_obj, ...) \
    AIM_LOG_OBJ_CUSTOM(_obj, ONLP_PLATFORM_DEFAULTS_LOG_FLAG_CALLED, "CALLED", __VA_ARGS__)
/** Log an object-level called with ratelimiting */
#define ONLP_PLATFORM_DEFAULTS_LOG_OBJ_RL_CALLED(_obj, _rl, _time, ...) \
    AIM_LOG_OBJ_RL_CUSTOM(_obj, ONLP_PLATFORM_DEFAULTS_LOG_FLAG_CALLED, "CALLED", _rl, _time, __VA_ARGS__)

/******************************************************************************
 *
 * Default Macro Mappings
 *
 *****************************************************************************/
#ifdef AIM_LOG_OBJ_DEFAULT

/** CALLED -> OBJ_CALLED */
#define ONLP_PLATFORM_DEFAULTS_LOG_CALLED ONLP_PLATFORM_DEFAULTS_LOG_OBJ_CALLED
/** RL_CALLED -> OBJ_RL_CALLED */
#define ONLP_PLATFORM_DEFAULTS_LOG_RL_CALLED ONLP_PLATFORM_DEFAULTS_LOG_RL_OBJ_CALLED


#else

/** CALLED -> MOD_CALLED */
#define ONLP_PLATFORM_DEFAULTS_LOG_CALLED ONLP_PLATFORM_DEFAULTS_LOG_MOD_CALLED
/** RL_CALLED -> MOD_RL_CALLED */
#define ONLP_PLATFORM_DEFAULTS_LOG_RL_CALLED ONLP_PLATFORM_DEFAULTS_LOG_MOD_RL_CALLED

#endif
/* <auto.end.aim_custom_log_macro(ALL).header> */

#endif /* __ONLP_PLATFORM_DEFAULTS_LOG_H__ */
