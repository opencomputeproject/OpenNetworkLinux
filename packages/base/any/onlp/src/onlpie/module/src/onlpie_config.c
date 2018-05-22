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

#include <onlpie/onlpie_config.h>

/* <auto.start.cdefs(ONLPIE_CONFIG_HEADER).source> */
#define __onlpie_config_STRINGIFY_NAME(_x) #_x
#define __onlpie_config_STRINGIFY_VALUE(_x) __onlpie_config_STRINGIFY_NAME(_x)
onlpie_config_settings_t onlpie_config_settings[] =
{
#ifdef ONLPIE_CONFIG_INCLUDE_LOGGING
    { __onlpie_config_STRINGIFY_NAME(ONLPIE_CONFIG_INCLUDE_LOGGING), __onlpie_config_STRINGIFY_VALUE(ONLPIE_CONFIG_INCLUDE_LOGGING) },
#else
{ ONLPIE_CONFIG_INCLUDE_LOGGING(__onlpie_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPIE_CONFIG_LOG_OPTIONS_DEFAULT
    { __onlpie_config_STRINGIFY_NAME(ONLPIE_CONFIG_LOG_OPTIONS_DEFAULT), __onlpie_config_STRINGIFY_VALUE(ONLPIE_CONFIG_LOG_OPTIONS_DEFAULT) },
#else
{ ONLPIE_CONFIG_LOG_OPTIONS_DEFAULT(__onlpie_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPIE_CONFIG_LOG_BITS_DEFAULT
    { __onlpie_config_STRINGIFY_NAME(ONLPIE_CONFIG_LOG_BITS_DEFAULT), __onlpie_config_STRINGIFY_VALUE(ONLPIE_CONFIG_LOG_BITS_DEFAULT) },
#else
{ ONLPIE_CONFIG_LOG_BITS_DEFAULT(__onlpie_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPIE_CONFIG_LOG_CUSTOM_BITS_DEFAULT
    { __onlpie_config_STRINGIFY_NAME(ONLPIE_CONFIG_LOG_CUSTOM_BITS_DEFAULT), __onlpie_config_STRINGIFY_VALUE(ONLPIE_CONFIG_LOG_CUSTOM_BITS_DEFAULT) },
#else
{ ONLPIE_CONFIG_LOG_CUSTOM_BITS_DEFAULT(__onlpie_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPIE_CONFIG_PORTING_STDLIB
    { __onlpie_config_STRINGIFY_NAME(ONLPIE_CONFIG_PORTING_STDLIB), __onlpie_config_STRINGIFY_VALUE(ONLPIE_CONFIG_PORTING_STDLIB) },
#else
{ ONLPIE_CONFIG_PORTING_STDLIB(__onlpie_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPIE_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
    { __onlpie_config_STRINGIFY_NAME(ONLPIE_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS), __onlpie_config_STRINGIFY_VALUE(ONLPIE_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS) },
#else
{ ONLPIE_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS(__onlpie_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPIE_CONFIG_INCLUDE_UCLI
    { __onlpie_config_STRINGIFY_NAME(ONLPIE_CONFIG_INCLUDE_UCLI), __onlpie_config_STRINGIFY_VALUE(ONLPIE_CONFIG_INCLUDE_UCLI) },
#else
{ ONLPIE_CONFIG_INCLUDE_UCLI(__onlpie_config_STRINGIFY_NAME), "__undefined__" },
#endif
    { NULL, NULL }
};
#undef __onlpie_config_STRINGIFY_VALUE
#undef __onlpie_config_STRINGIFY_NAME

const char*
onlpie_config_lookup(const char* setting)
{
    int i;
    for(i = 0; onlpie_config_settings[i].name; i++) {
        if(!strcmp(onlpie_config_settings[i].name, setting)) {
            return onlpie_config_settings[i].value;
        }
    }
    return NULL;
}

int
onlpie_config_show(struct aim_pvs_s* pvs)
{
    int i;
    for(i = 0; onlpie_config_settings[i].name; i++) {
        aim_printf(pvs, "%s = %s\n", onlpie_config_settings[i].name, onlpie_config_settings[i].value);
    }
    return i;
}

/* <auto.end.cdefs(ONLPIE_CONFIG_HEADER).source> */

