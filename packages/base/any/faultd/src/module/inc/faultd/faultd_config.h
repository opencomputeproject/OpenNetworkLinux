/**************************************************************************//**
 * 
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
 *
 * @file
 * @brief faultd Configuration Header
 * 
 * @addtogroup faultd-config
 * @{
 * 
 *****************************************************************************/
#ifndef __FAULTD_CONFIG_H__
#define __FAULTD_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef FAULTD_INCLUDE_CUSTOM_CONFIG
#include <faultd_custom_config.h>
#endif

/* <auto.start.cdefs(FAULTD_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * FAULTD_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef FAULTD_CONFIG_INCLUDE_LOGGING
#define FAULTD_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * FAULTD_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef FAULTD_CONFIG_LOG_OPTIONS_DEFAULT
#define FAULTD_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * FAULTD_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef FAULTD_CONFIG_LOG_BITS_DEFAULT
#define FAULTD_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * FAULTD_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef FAULTD_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define FAULTD_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * FAULTD_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef FAULTD_CONFIG_PORTING_STDLIB
#define FAULTD_CONFIG_PORTING_STDLIB 1
#endif

/**
 * FAULTD_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef FAULTD_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define FAULTD_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS FAULTD_CONFIG_PORTING_STDLIB
#endif

/**
 * FAULTD_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef FAULTD_CONFIG_INCLUDE_UCLI
#define FAULTD_CONFIG_INCLUDE_UCLI 0
#endif

/**
 * FAULTD_CONFIG_PIPE_NAME_DEFAULT
 *
 * Default named pipe. */


#ifndef FAULTD_CONFIG_PIPE_NAME_DEFAULT
#define FAULTD_CONFIG_PIPE_NAME_DEFAULT "/var/run/faultd.pipe"
#endif

/**
 * FAULTD_CONFIG_BINARY_SIZE
 *
 * Maximum binary name size. */


#ifndef FAULTD_CONFIG_BINARY_SIZE
#define FAULTD_CONFIG_BINARY_SIZE 255
#endif

/**
 * FAULTD_CONFIG_BACKTRACE_SIZE_MAX
 *
 * Maximum backtrace size. */


#ifndef FAULTD_CONFIG_BACKTRACE_SIZE_MAX
#define FAULTD_CONFIG_BACKTRACE_SIZE_MAX 32
#endif

/**
 * FAULTD_CONFIG_SERVICE_PIPES_MAX
 *
 * Maximum number of simulatanous service pipes. */


#ifndef FAULTD_CONFIG_SERVICE_PIPES_MAX
#define FAULTD_CONFIG_SERVICE_PIPES_MAX 8
#endif

/**
 * FAULTD_CONFIG_BACKTRACE_SYMBOLS_SIZE
 *
 * Maximum backtrace symbols size */


#ifndef FAULTD_CONFIG_BACKTRACE_SYMBOLS_SIZE
#define FAULTD_CONFIG_BACKTRACE_SYMBOLS_SIZE 4096
#endif

/**
 * FAULTD_CONFIG_INCLUDE_MAIN
 *
 * Include faultd_main() for standard faultd daemon build. */


#ifndef FAULTD_CONFIG_INCLUDE_MAIN
#define FAULTD_CONFIG_INCLUDE_MAIN 0
#endif

/**
 * FAULTD_CONFIG_INCLUDE_AIM_MAIN
 *
 * Include aim_main() as faultd_main(). */


#ifndef FAULTD_CONFIG_INCLUDE_AIM_MAIN
#define FAULTD_CONFIG_INCLUDE_AIM_MAIN FAULTD_CONFIG_INCLUDE_MAIN
#endif

/**
 * FAULTD_CONFIG_MAIN_PIPENAME
 *
 * Default pipename used by faultd_main() if included. */


#ifndef FAULTD_CONFIG_MAIN_PIPENAME
#define FAULTD_CONFIG_MAIN_PIPENAME "/var/run/faultd.fifo"
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct faultd_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} faultd_config_settings_t;

/** Configuration settings table. */
/** faultd_config_settings table. */
extern faultd_config_settings_t faultd_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* faultd_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int faultd_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(FAULTD_CONFIG_HEADER).header> */

#include "faultd_porting.h"

#endif /* __FAULTD_CONFIG_H__ */
/* @} */
