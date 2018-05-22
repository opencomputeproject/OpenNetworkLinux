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

#include <onlplib/onlplib_config.h>

/* <auto.start.cdefs(ONLPLIB_CONFIG_HEADER).source> */
#define __onlplib_config_STRINGIFY_NAME(_x) #_x
#define __onlplib_config_STRINGIFY_VALUE(_x) __onlplib_config_STRINGIFY_NAME(_x)
onlplib_config_settings_t onlplib_config_settings[] =
{
#ifdef ONLPLIB_CONFIG_INCLUDE_LOGGING
    { __onlplib_config_STRINGIFY_NAME(ONLPLIB_CONFIG_INCLUDE_LOGGING), __onlplib_config_STRINGIFY_VALUE(ONLPLIB_CONFIG_INCLUDE_LOGGING) },
#else
{ ONLPLIB_CONFIG_INCLUDE_LOGGING(__onlplib_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPLIB_CONFIG_LOG_OPTIONS_DEFAULT
    { __onlplib_config_STRINGIFY_NAME(ONLPLIB_CONFIG_LOG_OPTIONS_DEFAULT), __onlplib_config_STRINGIFY_VALUE(ONLPLIB_CONFIG_LOG_OPTIONS_DEFAULT) },
#else
{ ONLPLIB_CONFIG_LOG_OPTIONS_DEFAULT(__onlplib_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPLIB_CONFIG_LOG_BITS_DEFAULT
    { __onlplib_config_STRINGIFY_NAME(ONLPLIB_CONFIG_LOG_BITS_DEFAULT), __onlplib_config_STRINGIFY_VALUE(ONLPLIB_CONFIG_LOG_BITS_DEFAULT) },
#else
{ ONLPLIB_CONFIG_LOG_BITS_DEFAULT(__onlplib_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPLIB_CONFIG_LOG_CUSTOM_BITS_DEFAULT
    { __onlplib_config_STRINGIFY_NAME(ONLPLIB_CONFIG_LOG_CUSTOM_BITS_DEFAULT), __onlplib_config_STRINGIFY_VALUE(ONLPLIB_CONFIG_LOG_CUSTOM_BITS_DEFAULT) },
#else
{ ONLPLIB_CONFIG_LOG_CUSTOM_BITS_DEFAULT(__onlplib_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPLIB_CONFIG_PORTING_STDLIB
    { __onlplib_config_STRINGIFY_NAME(ONLPLIB_CONFIG_PORTING_STDLIB), __onlplib_config_STRINGIFY_VALUE(ONLPLIB_CONFIG_PORTING_STDLIB) },
#else
{ ONLPLIB_CONFIG_PORTING_STDLIB(__onlplib_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPLIB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
    { __onlplib_config_STRINGIFY_NAME(ONLPLIB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS), __onlplib_config_STRINGIFY_VALUE(ONLPLIB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS) },
#else
{ ONLPLIB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS(__onlplib_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPLIB_CONFIG_INCLUDE_UCLI
    { __onlplib_config_STRINGIFY_NAME(ONLPLIB_CONFIG_INCLUDE_UCLI), __onlplib_config_STRINGIFY_VALUE(ONLPLIB_CONFIG_INCLUDE_UCLI) },
#else
{ ONLPLIB_CONFIG_INCLUDE_UCLI(__onlplib_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPLIB_CONFIG_INCLUDE_I2C
    { __onlplib_config_STRINGIFY_NAME(ONLPLIB_CONFIG_INCLUDE_I2C), __onlplib_config_STRINGIFY_VALUE(ONLPLIB_CONFIG_INCLUDE_I2C) },
#else
{ ONLPLIB_CONFIG_INCLUDE_I2C(__onlplib_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPLIB_CONFIG_I2C_BLOCK_SIZE
    { __onlplib_config_STRINGIFY_NAME(ONLPLIB_CONFIG_I2C_BLOCK_SIZE), __onlplib_config_STRINGIFY_VALUE(ONLPLIB_CONFIG_I2C_BLOCK_SIZE) },
#else
{ ONLPLIB_CONFIG_I2C_BLOCK_SIZE(__onlplib_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPLIB_CONFIG_I2C_READ_RETRY_COUNT
    { __onlplib_config_STRINGIFY_NAME(ONLPLIB_CONFIG_I2C_READ_RETRY_COUNT), __onlplib_config_STRINGIFY_VALUE(ONLPLIB_CONFIG_I2C_READ_RETRY_COUNT) },
#else
{ ONLPLIB_CONFIG_I2C_READ_RETRY_COUNT(__onlplib_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER
    { __onlplib_config_STRINGIFY_NAME(ONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER), __onlplib_config_STRINGIFY_VALUE(ONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER) },
#else
{ ONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER(__onlplib_config_STRINGIFY_NAME), "__undefined__" },
#endif
    { NULL, NULL }
};
#undef __onlplib_config_STRINGIFY_VALUE
#undef __onlplib_config_STRINGIFY_NAME

const char*
onlplib_config_lookup(const char* setting)
{
    int i;
    for(i = 0; onlplib_config_settings[i].name; i++) {
        if(!strcmp(onlplib_config_settings[i].name, setting)) {
            return onlplib_config_settings[i].value;
        }
    }
    return NULL;
}

int
onlplib_config_show(struct aim_pvs_s* pvs)
{
    int i;
    for(i = 0; onlplib_config_settings[i].name; i++) {
        aim_printf(pvs, "%s = %s\n", onlplib_config_settings[i].name, onlplib_config_settings[i].value);
    }
    return i;
}

/* <auto.end.cdefs(ONLPLIB_CONFIG_HEADER).source> */

