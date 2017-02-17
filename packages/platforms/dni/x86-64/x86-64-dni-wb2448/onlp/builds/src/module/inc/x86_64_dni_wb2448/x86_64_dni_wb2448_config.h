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

/**************************************************************************//**
 *
 * @file
 * @brief x86_64_dni_wb2448 Configuration Header
 *
 * @addtogroup x86_64_dni_wb2448-config
 * @{
 *
 *****************************************************************************/
#ifndef __x86_64_dni_wb2448_CONFIG_H__
#define __x86_64_dni_wb2448_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef x86_64_dni_wb2448_INCLUDE_CUSTOM_CONFIG
#include <x86_64_dni_wb2448_custom_config.h>
#endif

/* <auto.start.cdefs(x86_64_dni_wb2448_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * x86_64_dni_wb2448_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef x86_64_dni_wb2448_CONFIG_INCLUDE_LOGGING
#define x86_64_dni_wb2448_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * x86_64_dni_wb2448_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef x86_64_dni_wb2448_CONFIG_LOG_OPTIONS_DEFAULT
#define x86_64_dni_wb2448_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * x86_64_dni_wb2448_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef x86_64_dni_wb2448_CONFIG_LOG_BITS_DEFAULT
#define x86_64_dni_wb2448_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * x86_64_dni_wb2448_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef x86_64_dni_wb2448_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define x86_64_dni_wb2448_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * x86_64_dni_wb2448_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef x86_64_dni_wb2448_CONFIG_PORTING_STDLIB
#define x86_64_dni_wb2448_CONFIG_PORTING_STDLIB 1
#endif

/**
 * x86_64_dni_wb2448_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef x86_64_dni_wb2448_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define x86_64_dni_wb2448_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS x86_64_dni_wb2448_CONFIG_PORTING_STDLIB
#endif

/**
 * x86_64_dni_wb2448_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef x86_64_dni_wb2448_CONFIG_INCLUDE_UCLI
#define x86_64_dni_wb2448_CONFIG_INCLUDE_UCLI 0
#endif

/**
 * x86_64_dni_wb2448_CONFIG_INCLUDE_DEFAULT_FAN_DIRECTION
 *
 * Assume chassis fan direction is the same as the PSU fan direction. */


#ifndef x86_64_dni_wb2448_CONFIG_INCLUDE_DEFAULT_FAN_DIRECTION
#define x86_64_dni_wb2448_CONFIG_INCLUDE_DEFAULT_FAN_DIRECTION 0
#endif

#ifndef x86_64_dni_wb2448_CONFIG_SFP_COUNT
#define x86_64_dni_wb2448_CONFIG_SFP_COUNT 4
#endif


/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct x86_64_dni_wb2448_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} x86_64_dni_wb2448_config_settings_t;

/** Configuration settings table. */
/** x86_64_dni_wb2448_config_settings table. */
extern x86_64_dni_wb2448_config_settings_t x86_64_dni_wb2448_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* x86_64_dni_wb2448_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int x86_64_dni_wb2448_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(x86_64_dni_wb2448_CONFIG_HEADER).header> */

#include "x86_64_dni_wb2448_porting.h"

#endif /* __x86_64_dni_wb2448_CONFIG_H__ */
/* @} */
