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

#include <onlp/onlp_config.h>

/* <auto.start.cdefs(ONLP_CONFIG_HEADER).source> */
#define __onlp_config_STRINGIFY_NAME(_x) #_x
#define __onlp_config_STRINGIFY_VALUE(_x) __onlp_config_STRINGIFY_NAME(_x)
onlp_config_settings_t onlp_config_settings[] =
{
#ifdef ONLP_CONFIG_INCLUDE_LOGGING
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_INCLUDE_LOGGING), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_INCLUDE_LOGGING) },
#else
{ ONLP_CONFIG_INCLUDE_LOGGING(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_LOG_OPTIONS_DEFAULT
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_LOG_OPTIONS_DEFAULT), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_LOG_OPTIONS_DEFAULT) },
#else
{ ONLP_CONFIG_LOG_OPTIONS_DEFAULT(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_LOG_BITS_DEFAULT
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_LOG_BITS_DEFAULT), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_LOG_BITS_DEFAULT) },
#else
{ ONLP_CONFIG_LOG_BITS_DEFAULT(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_LOG_CUSTOM_BITS_DEFAULT
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_LOG_CUSTOM_BITS_DEFAULT), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_LOG_CUSTOM_BITS_DEFAULT) },
#else
{ ONLP_CONFIG_LOG_CUSTOM_BITS_DEFAULT(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_PORTING_STDLIB
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_PORTING_STDLIB), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_PORTING_STDLIB) },
#else
{ ONLP_CONFIG_PORTING_STDLIB(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS) },
#else
{ ONLP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_INCLUDE_UCLI
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_INCLUDE_UCLI), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_INCLUDE_UCLI) },
#else
{ ONLP_CONFIG_INCLUDE_UCLI(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_INCLUDE_PLATFORM_ERROR_CHECK
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_INCLUDE_PLATFORM_ERROR_CHECK), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_INCLUDE_PLATFORM_ERROR_CHECK) },
#else
{ ONLP_CONFIG_INCLUDE_PLATFORM_ERROR_CHECK(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_INCLUDE_SHLOCK_GLOBAL_INIT
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_INCLUDE_SHLOCK_GLOBAL_INIT), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_INCLUDE_SHLOCK_GLOBAL_INIT) },
#else
{ ONLP_CONFIG_INCLUDE_SHLOCK_GLOBAL_INIT(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_INCLUDE_PLATFORM_STATIC
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_INCLUDE_PLATFORM_STATIC), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_INCLUDE_PLATFORM_STATIC) },
#else
{ ONLP_CONFIG_INCLUDE_PLATFORM_STATIC(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_PLATFORM_STATIC
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_PLATFORM_STATIC), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_PLATFORM_STATIC) },
#else
{ ONLP_CONFIG_PLATFORM_STATIC(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_PLATFORM_FILENAME
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_PLATFORM_FILENAME), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_PLATFORM_FILENAME) },
#else
{ ONLP_CONFIG_PLATFORM_FILENAME(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_INCLUDE_PLATFORM_OVERRIDES
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_INCLUDE_PLATFORM_OVERRIDES), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_INCLUDE_PLATFORM_OVERRIDES) },
#else
{ ONLP_CONFIG_INCLUDE_PLATFORM_OVERRIDES(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_CONFIGURATION_FILENAME
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_CONFIGURATION_FILENAME), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_CONFIGURATION_FILENAME) },
#else
{ ONLP_CONFIG_CONFIGURATION_FILENAME(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_CONFIGURATION_ENV
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_CONFIGURATION_ENV), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_CONFIGURATION_ENV) },
#else
{ ONLP_CONFIG_CONFIGURATION_ENV(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_INCLUDE_API_LOCK
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_INCLUDE_API_LOCK), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_INCLUDE_API_LOCK) },
#else
{ ONLP_CONFIG_INCLUDE_API_LOCK(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_API_LOCK_GLOBAL_SHARED
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_API_LOCK_GLOBAL_SHARED), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_API_LOCK_GLOBAL_SHARED) },
#else
{ ONLP_CONFIG_API_LOCK_GLOBAL_SHARED(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_API_LOCK_TIMEOUT
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_API_LOCK_TIMEOUT), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_API_LOCK_TIMEOUT) },
#else
{ ONLP_CONFIG_API_LOCK_TIMEOUT(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_INFO_STR_MAX
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_INFO_STR_MAX), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_INFO_STR_MAX) },
#else
{ ONLP_CONFIG_INFO_STR_MAX(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_INCLUDE_THERMAL_THRESHOLDS
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_INCLUDE_THERMAL_THRESHOLDS), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_INCLUDE_THERMAL_THRESHOLDS) },
#else
{ ONLP_CONFIG_INCLUDE_THERMAL_THRESHOLDS(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_CONFIG_INCLUDE_API_PROFILING
    { __onlp_config_STRINGIFY_NAME(ONLP_CONFIG_INCLUDE_API_PROFILING), __onlp_config_STRINGIFY_VALUE(ONLP_CONFIG_INCLUDE_API_PROFILING) },
#else
{ ONLP_CONFIG_INCLUDE_API_PROFILING(__onlp_config_STRINGIFY_NAME), "__undefined__" },
#endif
    { NULL, NULL }
};
#undef __onlp_config_STRINGIFY_VALUE
#undef __onlp_config_STRINGIFY_NAME

const char*
onlp_config_lookup(const char* setting)
{
    int i;
    for(i = 0; onlp_config_settings[i].name; i++) {
        if(strcmp(onlp_config_settings[i].name, setting)) {
            return onlp_config_settings[i].value;
        }
    }
    return NULL;
}

int
onlp_config_show(struct aim_pvs_s* pvs)
{
    int i;
    for(i = 0; onlp_config_settings[i].name; i++) {
        aim_printf(pvs, "%s = %s\n", onlp_config_settings[i].name, onlp_config_settings[i].value);
    }
    return i;
}

/* <auto.end.cdefs(ONLP_CONFIG_HEADER).source> */

