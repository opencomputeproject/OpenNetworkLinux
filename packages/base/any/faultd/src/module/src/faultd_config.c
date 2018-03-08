/**************************************************************************//**
 * <bsn.cl fy=2013 v=onl>
 * 
 *        Copyright 2013, 2014 BigSwitch Networks, Inc.        
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
 *****************************************************************************/
#include <faultd/faultd_config.h>

/* <auto.start.cdefs(FAULTD_CONFIG_HEADER).source> */
#define __faultd_config_STRINGIFY_NAME(_x) #_x
#define __faultd_config_STRINGIFY_VALUE(_x) __faultd_config_STRINGIFY_NAME(_x)
faultd_config_settings_t faultd_config_settings[] =
{
#ifdef FAULTD_CONFIG_INCLUDE_LOGGING
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_INCLUDE_LOGGING), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_INCLUDE_LOGGING) },
#else
{ FAULTD_CONFIG_INCLUDE_LOGGING(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_LOG_OPTIONS_DEFAULT
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_LOG_OPTIONS_DEFAULT), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_LOG_OPTIONS_DEFAULT) },
#else
{ FAULTD_CONFIG_LOG_OPTIONS_DEFAULT(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_LOG_BITS_DEFAULT
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_LOG_BITS_DEFAULT), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_LOG_BITS_DEFAULT) },
#else
{ FAULTD_CONFIG_LOG_BITS_DEFAULT(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_LOG_CUSTOM_BITS_DEFAULT
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_LOG_CUSTOM_BITS_DEFAULT), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_LOG_CUSTOM_BITS_DEFAULT) },
#else
{ FAULTD_CONFIG_LOG_CUSTOM_BITS_DEFAULT(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_PORTING_STDLIB
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_PORTING_STDLIB), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_PORTING_STDLIB) },
#else
{ FAULTD_CONFIG_PORTING_STDLIB(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS) },
#else
{ FAULTD_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_INCLUDE_UCLI
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_INCLUDE_UCLI), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_INCLUDE_UCLI) },
#else
{ FAULTD_CONFIG_INCLUDE_UCLI(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_PIPE_NAME_DEFAULT
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_PIPE_NAME_DEFAULT), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_PIPE_NAME_DEFAULT) },
#else
{ FAULTD_CONFIG_PIPE_NAME_DEFAULT(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_BINARY_SIZE
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_BINARY_SIZE), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_BINARY_SIZE) },
#else
{ FAULTD_CONFIG_BINARY_SIZE(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_BACKTRACE_SIZE_MAX
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_BACKTRACE_SIZE_MAX), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_BACKTRACE_SIZE_MAX) },
#else
{ FAULTD_CONFIG_BACKTRACE_SIZE_MAX(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_SERVICE_PIPES_MAX
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_SERVICE_PIPES_MAX), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_SERVICE_PIPES_MAX) },
#else
{ FAULTD_CONFIG_SERVICE_PIPES_MAX(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_BACKTRACE_SYMBOLS_SIZE
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_BACKTRACE_SYMBOLS_SIZE), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_BACKTRACE_SYMBOLS_SIZE) },
#else
{ FAULTD_CONFIG_BACKTRACE_SYMBOLS_SIZE(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_INCLUDE_MAIN
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_INCLUDE_MAIN), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_INCLUDE_MAIN) },
#else
{ FAULTD_CONFIG_INCLUDE_MAIN(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_INCLUDE_AIM_MAIN
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_INCLUDE_AIM_MAIN), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_INCLUDE_AIM_MAIN) },
#else
{ FAULTD_CONFIG_INCLUDE_AIM_MAIN(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef FAULTD_CONFIG_MAIN_PIPENAME
    { __faultd_config_STRINGIFY_NAME(FAULTD_CONFIG_MAIN_PIPENAME), __faultd_config_STRINGIFY_VALUE(FAULTD_CONFIG_MAIN_PIPENAME) },
#else
{ FAULTD_CONFIG_MAIN_PIPENAME(__faultd_config_STRINGIFY_NAME), "__undefined__" },
#endif
    { NULL, NULL }
};
#undef __faultd_config_STRINGIFY_VALUE
#undef __faultd_config_STRINGIFY_NAME

const char*
faultd_config_lookup(const char* setting)
{
    int i;
    for(i = 0; faultd_config_settings[i].name; i++) {
        if(!strcmp(faultd_config_settings[i].name, setting)) {
            return faultd_config_settings[i].value;
        }
    }
    return NULL;
}

int
faultd_config_show(struct aim_pvs_s* pvs)
{
    int i;
    for(i = 0; faultd_config_settings[i].name; i++) {
        aim_printf(pvs, "%s = %s\n", faultd_config_settings[i].name, faultd_config_settings[i].value);
    }
    return i;
}

/* <auto.end.cdefs(FAULTD_CONFIG_HEADER).source> */

