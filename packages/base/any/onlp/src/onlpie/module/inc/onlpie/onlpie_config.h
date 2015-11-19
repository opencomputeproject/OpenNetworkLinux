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
 * @brief onlpie Configuration Header
 *
 * @addtogroup onlpie-config
 * @{
 *
 *****************************************************************************/
#ifndef __ONLPIE_CONFIG_H__
#define __ONLPIE_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef ONLPIE_INCLUDE_CUSTOM_CONFIG
#include <onlpie_custom_config.h>
#endif

/* <auto.start.cdefs(ONLPIE_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * ONLPIE_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef ONLPIE_CONFIG_INCLUDE_LOGGING
#define ONLPIE_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * ONLPIE_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef ONLPIE_CONFIG_LOG_OPTIONS_DEFAULT
#define ONLPIE_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * ONLPIE_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef ONLPIE_CONFIG_LOG_BITS_DEFAULT
#define ONLPIE_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * ONLPIE_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef ONLPIE_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define ONLPIE_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * ONLPIE_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef ONLPIE_CONFIG_PORTING_STDLIB
#define ONLPIE_CONFIG_PORTING_STDLIB 1
#endif

/**
 * ONLPIE_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef ONLPIE_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define ONLPIE_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS ONLPIE_CONFIG_PORTING_STDLIB
#endif

/**
 * ONLPIE_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef ONLPIE_CONFIG_INCLUDE_UCLI
#define ONLPIE_CONFIG_INCLUDE_UCLI 0
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct onlpie_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} onlpie_config_settings_t;

/** Configuration settings table. */
/** onlpie_config_settings table. */
extern onlpie_config_settings_t onlpie_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* onlpie_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int onlpie_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(ONLPIE_CONFIG_HEADER).header> */

#include "onlpie_porting.h"

#endif /* __ONLPIE_CONFIG_H__ */
/* @} */
