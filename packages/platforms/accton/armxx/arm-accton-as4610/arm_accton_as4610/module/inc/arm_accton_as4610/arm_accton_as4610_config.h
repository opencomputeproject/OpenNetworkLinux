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
 * @brief arm_accton_as4610 Configuration Header
 *
 * @addtogroup arm_accton_as4610-config
 * @{
 *
 *****************************************************************************/
#ifndef __ARM_ACCTON_AS4610_CONFIG_H__
#define __ARM_ACCTON_AS4610_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef ARM_ACCTON_AS4610_INCLUDE_CUSTOM_CONFIG
#include <arm_accton_as4610_custom_config.h>
#endif

/* <auto.start.cdefs(ARM_ACCTON_AS4610_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * ARM_ACCTON_AS4610_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef ARM_ACCTON_AS4610_CONFIG_INCLUDE_LOGGING
#define ARM_ACCTON_AS4610_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * ARM_ACCTON_AS4610_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef ARM_ACCTON_AS4610_CONFIG_LOG_OPTIONS_DEFAULT
#define ARM_ACCTON_AS4610_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * ARM_ACCTON_AS4610_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef ARM_ACCTON_AS4610_CONFIG_LOG_BITS_DEFAULT
#define ARM_ACCTON_AS4610_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * ARM_ACCTON_AS4610_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef ARM_ACCTON_AS4610_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define ARM_ACCTON_AS4610_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * ARM_ACCTON_AS4610_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef ARM_ACCTON_AS4610_CONFIG_PORTING_STDLIB
#define ARM_ACCTON_AS4610_CONFIG_PORTING_STDLIB 1
#endif

/**
 * ARM_ACCTON_AS4610_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef ARM_ACCTON_AS4610_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define ARM_ACCTON_AS4610_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS ARM_ACCTON_AS4610_CONFIG_PORTING_STDLIB
#endif

/**
 * ARM_ACCTON_AS4610_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef ARM_ACCTON_AS4610_CONFIG_INCLUDE_UCLI
#define ARM_ACCTON_AS4610_CONFIG_INCLUDE_UCLI 0
#endif

/**
 * ARM_ACCTON_AS4610_CONFIG_SFP_COUNT
 *
 * SFP Count. */


#ifndef ARM_ACCTON_AS4610_CONFIG_SFP_COUNT
#define ARM_ACCTON_AS4610_CONFIG_SFP_COUNT 0
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct arm_accton_as4610_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} arm_accton_as4610_config_settings_t;

/** Configuration settings table. */
/** arm_accton_as4610_config_settings table. */
extern arm_accton_as4610_config_settings_t arm_accton_as4610_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* arm_accton_as4610_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int arm_accton_as4610_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(ARM_ACCTON_AS4610_CONFIG_HEADER).header> */

#include "arm_accton_as4610_porting.h"

#endif /* __ARM_ACCTON_AS4610_CONFIG_H__ */
/* @} */
