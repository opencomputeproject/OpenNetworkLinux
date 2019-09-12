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
 * @brief onlplib Configuration Header
 *
 * @addtogroup onlplib-config
 * @{
 *
 *****************************************************************************/
#ifndef __ONLPLIB_CONFIG_H__
#define __ONLPLIB_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef ONLPLIB_INCLUDE_CUSTOM_CONFIG
#include <onlplib_custom_config.h>
#endif

/* <auto.start.cdefs(ONLPLIB_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * ONLPLIB_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef ONLPLIB_CONFIG_INCLUDE_LOGGING
#define ONLPLIB_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * ONLPLIB_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef ONLPLIB_CONFIG_LOG_OPTIONS_DEFAULT
#define ONLPLIB_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * ONLPLIB_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef ONLPLIB_CONFIG_LOG_BITS_DEFAULT
#define ONLPLIB_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * ONLPLIB_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef ONLPLIB_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define ONLPLIB_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * ONLPLIB_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef ONLPLIB_CONFIG_PORTING_STDLIB
#define ONLPLIB_CONFIG_PORTING_STDLIB 1
#endif

/**
 * ONLPLIB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef ONLPLIB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define ONLPLIB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS ONLPLIB_CONFIG_PORTING_STDLIB
#endif

/**
 * ONLPLIB_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef ONLPLIB_CONFIG_INCLUDE_UCLI
#define ONLPLIB_CONFIG_INCLUDE_UCLI 0
#endif

/**
 * ONLPLIB_CONFIG_INCLUDE_I2C
 *
 * Include Userspace I2C support. */


#ifndef ONLPLIB_CONFIG_INCLUDE_I2C
#define ONLPLIB_CONFIG_INCLUDE_I2C 1
#endif

/**
 * ONLPLIB_CONFIG_I2C_BLOCK_SIZE
 *
 * Maximum read and write block size. */


#ifndef ONLPLIB_CONFIG_I2C_BLOCK_SIZE
#define ONLPLIB_CONFIG_I2C_BLOCK_SIZE 32
#endif

/**
 * ONLPLIB_CONFIG_I2C_READ_RETRY_COUNT
 *
 * The number of I2C read retry attempts (if enabled). */


#ifndef ONLPLIB_CONFIG_I2C_READ_RETRY_COUNT
#define ONLPLIB_CONFIG_I2C_READ_RETRY_COUNT 16
#endif

/**
 * ONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER
 *
 * Include the custom i2c header (include/linux/i2c-devices.h) to avoid conflicts with the kernel and i2c-dev packages. */


#ifndef ONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER
#define ONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER 1
#endif

/**
 * ONLPLIB_CONFIG_INCLUDE_I2C_SMBUS
 *
 * Include <i2c/smbus.h> */


#ifndef ONLPLIB_CONFIG_INCLUDE_I2C_SMBUS
#define ONLPLIB_CONFIG_INCLUDE_I2C_SMBUS 0
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct onlplib_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} onlplib_config_settings_t;

/** Configuration settings table. */
/** onlplib_config_settings table. */
extern onlplib_config_settings_t onlplib_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* onlplib_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int onlplib_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(ONLPLIB_CONFIG_HEADER).header> */

#include "onlplib_porting.h"

#endif /* __ONLPLIB_CONFIG_H__ */
/* @} */
