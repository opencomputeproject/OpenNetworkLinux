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
 * @brief onlp Configuration Header
 *
 * @addtogroup onlp-config
 * @{
 *
 *****************************************************************************/
#ifndef __ONLP_CONFIG_H__
#define __ONLP_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef ONLP_INCLUDE_CUSTOM_CONFIG
#include <onlp_custom_config.h>
#endif

/* <auto.start.cdefs(ONLP_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * ONLP_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef ONLP_CONFIG_INCLUDE_LOGGING
#define ONLP_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * ONLP_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef ONLP_CONFIG_LOG_OPTIONS_DEFAULT
#define ONLP_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * ONLP_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef ONLP_CONFIG_LOG_BITS_DEFAULT
#define ONLP_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * ONLP_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef ONLP_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define ONLP_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * ONLP_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef ONLP_CONFIG_PORTING_STDLIB
#define ONLP_CONFIG_PORTING_STDLIB 1
#endif

/**
 * ONLP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef ONLP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define ONLP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS ONLP_CONFIG_PORTING_STDLIB
#endif

/**
 * ONLP_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef ONLP_CONFIG_INCLUDE_UCLI
#define ONLP_CONFIG_INCLUDE_UCLI 0
#endif

/**
 * ONLP_CONFIG_INCLUDE_PLATFORM_ERROR_CHECK
 *
 * Include platform error checking at initialization. */


#ifndef ONLP_CONFIG_INCLUDE_PLATFORM_ERROR_CHECK
#define ONLP_CONFIG_INCLUDE_PLATFORM_ERROR_CHECK 1
#endif

/**
 * ONLP_CONFIG_INCLUDE_SHLOCK_GLOBAL_INIT
 *
 * Include global shlock initialization at module init time. */


#ifndef ONLP_CONFIG_INCLUDE_SHLOCK_GLOBAL_INIT
#define ONLP_CONFIG_INCLUDE_SHLOCK_GLOBAL_INIT 0
#endif

/**
 * ONLP_CONFIG_INCLUDE_PLATFORM_STATIC
 *
 * Specify the platform name directly as a compile-time option. */


#ifndef ONLP_CONFIG_INCLUDE_PLATFORM_STATIC
#define ONLP_CONFIG_INCLUDE_PLATFORM_STATIC 0
#endif

/**
 * ONLP_CONFIG_PLATFORM_STATIC
 *
 * The name of the static platform if configured. */


#ifndef ONLP_CONFIG_PLATFORM_STATIC
#define ONLP_CONFIG_PLATFORM_STATIC "unknown"
#endif

/**
 * ONLP_CONFIG_PLATFORM_FILENAME
 *
 * The local filename containing the current platform identifier. */


#ifndef ONLP_CONFIG_PLATFORM_FILENAME
#define ONLP_CONFIG_PLATFORM_FILENAME "/etc/onl/platform"
#endif

/**
 * ONLP_CONFIG_INCLUDE_PLATFORM_OVERRIDES
 *
 * Allow support for local overrides of all platform OID values (testing). */


#ifndef ONLP_CONFIG_INCLUDE_PLATFORM_OVERRIDES
#define ONLP_CONFIG_INCLUDE_PLATFORM_OVERRIDES 1
#endif

/**
 * ONLP_CONFIG_CONFIGURATION_FILENAME
 *
 * The filename for the (optional) ONLP JSON configuration file. */


#ifndef ONLP_CONFIG_CONFIGURATION_FILENAME
#define ONLP_CONFIG_CONFIGURATION_FILENAME "/etc/onlp.conf"
#endif

/**
 * ONLP_CONFIG_CONFIGURATION_ENV
 *
 * Environment variable to check for configuration filenames. Overrides default. */


#ifndef ONLP_CONFIG_CONFIGURATION_ENV
#define ONLP_CONFIG_CONFIGURATION_ENV "ONLP_CONF"
#endif

/**
 * ONLP_CONFIG_INCLUDE_API_LOCK
 *
 * Include exclusive locking for all API calls. */


#ifndef ONLP_CONFIG_INCLUDE_API_LOCK
#define ONLP_CONFIG_INCLUDE_API_LOCK 1
#endif

/**
 * ONLP_CONFIG_API_LOCK_GLOBAL_SHARED
 *
 * If 0, the API lock is a simple semaphore. If 1, the API lock is a global shared mutex. */


#ifndef ONLP_CONFIG_API_LOCK_GLOBAL_SHARED
#define ONLP_CONFIG_API_LOCK_GLOBAL_SHARED 1
#endif

/**
 * ONLP_CONFIG_API_LOCK_TIMEOUT
 *
 * The maximum amount of time (in usecs) to wait while attempting to acquire the API lock. Failure to acquire is fatal. A value of zero disables this feature.  */


#ifndef ONLP_CONFIG_API_LOCK_TIMEOUT
#define ONLP_CONFIG_API_LOCK_TIMEOUT 60000000
#endif

/**
 * ONLP_CONFIG_INFO_STR_MAX
 *
 * The maximum size of static information string buffers. */


#ifndef ONLP_CONFIG_INFO_STR_MAX
#define ONLP_CONFIG_INFO_STR_MAX 64
#endif

/**
 * ONLP_CONFIG_INCLUDE_THERMAL_THRESHOLDS
 *
 * Include thermal threshold reporting. */


#ifndef ONLP_CONFIG_INCLUDE_THERMAL_THRESHOLDS
#define ONLP_CONFIG_INCLUDE_THERMAL_THRESHOLDS 0
#endif

/**
 * ONLP_CONFIG_INCLUDE_API_PROFILING
 *
 * Include API timing profiles. */


#ifndef ONLP_CONFIG_INCLUDE_API_PROFILING
#define ONLP_CONFIG_INCLUDE_API_PROFILING 0
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct onlp_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} onlp_config_settings_t;

/** Configuration settings table. */
/** onlp_config_settings table. */
extern onlp_config_settings_t onlp_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* onlp_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int onlp_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(ONLP_CONFIG_HEADER).header> */

#include "onlp_porting.h"

#endif /* __ONLP_CONFIG_H__ */
/* @} */
