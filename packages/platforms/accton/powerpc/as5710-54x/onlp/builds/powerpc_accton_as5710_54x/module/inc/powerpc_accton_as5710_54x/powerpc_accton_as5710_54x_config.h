/**************************************************************************//**
 *
 * @file
 * @brief powerpc_accton_as5710_54x Configuration Header
 *
 * @addtogroup powerpc_accton_as5710_54x-config
 * @{
 *
 *****************************************************************************/
#ifndef __POWERPC_ACCTON_AS5710_54X_CONFIG_H__
#define __POWERPC_ACCTON_AS5710_54X_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef POWERPC_ACCTON_AS5710_54X_INCLUDE_CUSTOM_CONFIG
#include <powerpc_accton_as5710_54x_custom_config.h>
#endif

/* <auto.start.cdefs(POWERPC_ACCTON_AS5710_54X_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * POWERPC_ACCTON_AS5710_54X_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef POWERPC_ACCTON_AS5710_54X_CONFIG_INCLUDE_LOGGING
#define POWERPC_ACCTON_AS5710_54X_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * POWERPC_ACCTON_AS5710_54X_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef POWERPC_ACCTON_AS5710_54X_CONFIG_LOG_OPTIONS_DEFAULT
#define POWERPC_ACCTON_AS5710_54X_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * POWERPC_ACCTON_AS5710_54X_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef POWERPC_ACCTON_AS5710_54X_CONFIG_LOG_BITS_DEFAULT
#define POWERPC_ACCTON_AS5710_54X_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * POWERPC_ACCTON_AS5710_54X_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef POWERPC_ACCTON_AS5710_54X_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define POWERPC_ACCTON_AS5710_54X_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * POWERPC_ACCTON_AS5710_54X_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef POWERPC_ACCTON_AS5710_54X_CONFIG_PORTING_STDLIB
#define POWERPC_ACCTON_AS5710_54X_CONFIG_PORTING_STDLIB 1
#endif

/**
 * POWERPC_ACCTON_AS5710_54X_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef POWERPC_ACCTON_AS5710_54X_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define POWERPC_ACCTON_AS5710_54X_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS POWERPC_ACCTON_AS5710_54X_CONFIG_PORTING_STDLIB
#endif

/**
 * POWERPC_ACCTON_AS5710_54X_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef POWERPC_ACCTON_AS5710_54X_CONFIG_INCLUDE_UCLI
#define POWERPC_ACCTON_AS5710_54X_CONFIG_INCLUDE_UCLI 0
#endif

/**
 * POWERPC_ACCTON_AS5710_54X_CONFIG_INCLUDE_DEBUG
 *
 * Include debug tool. */


#ifndef POWERPC_ACCTON_AS5710_54X_CONFIG_INCLUDE_DEBUG
#define POWERPC_ACCTON_AS5710_54X_CONFIG_INCLUDE_DEBUG 0
#endif

/**
 * POWERPC_ACCTON_AS5710_54X_CONFIG_SYS_FAN_FRONT_RPM_MAX
 *
 * Maximum system fan(Front) rpm. */


#ifndef POWERPC_ACCTON_AS5710_54X_CONFIG_SYS_FAN_FRONT_RPM_MAX
#define POWERPC_ACCTON_AS5710_54X_CONFIG_SYS_FAN_FRONT_RPM_MAX 19725
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct powerpc_accton_as5710_54x_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} powerpc_accton_as5710_54x_config_settings_t;

/** Configuration settings table. */
/** powerpc_accton_as5710_54x_config_settings table. */
extern powerpc_accton_as5710_54x_config_settings_t powerpc_accton_as5710_54x_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* powerpc_accton_as5710_54x_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int powerpc_accton_as5710_54x_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(POWERPC_ACCTON_AS5710_54X_CONFIG_HEADER).header> */

#include "powerpc_accton_as5710_54x_porting.h"

#endif /* __POWERPC_ACCTON_AS5710_54X_CONFIG_H__ */
/* @} */
