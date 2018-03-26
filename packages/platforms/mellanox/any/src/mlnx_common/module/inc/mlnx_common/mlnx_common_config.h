/**************************************************************************//**
 *
 * @file
 * @brief mlnx_common Configuration Header
 *
 * @addtogroup mlnx_common-config
 * @{
 *
 *****************************************************************************/
#ifndef __MLNX_COMMON_CONFIG_H__
#define __MLNX_COMMON_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef MLNX_COMMON_INCLUDE_CUSTOM_CONFIG
#include <mlnx_common_custom_config.h>
#endif

/* <auto.start.cdefs(MLNX_COMMON_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * MLNX_COMMON_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef MLNX_COMMON_CONFIG_INCLUDE_LOGGING
#define MLNX_COMMON_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * MLNX_COMMON_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef MLNX_COMMON_CONFIG_LOG_OPTIONS_DEFAULT
#define MLNX_COMMON_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * MLNX_COMMON_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef MLNX_COMMON_CONFIG_LOG_BITS_DEFAULT
#define MLNX_COMMON_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * MLNX_COMMON_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef MLNX_COMMON_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define MLNX_COMMON_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * MLNX_COMMON_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef MLNX_COMMON_CONFIG_PORTING_STDLIB
#define MLNX_COMMON_CONFIG_PORTING_STDLIB 1
#endif

/**
 * MLNX_COMMON_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef MLNX_COMMON_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define MLNX_COMMON_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS MLNX_COMMON_CONFIG_PORTING_STDLIB
#endif

/**
 * MLNX_COMMON_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef MLNX_COMMON_CONFIG_INCLUDE_UCLI
#define MLNX_COMMON_CONFIG_INCLUDE_UCLI 0
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct mlnx_common_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} mlnx_common_config_settings_t;

/** Configuration settings table. */
/** mlnx_common_config_settings table. */
extern mlnx_common_config_settings_t mlnx_common_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* mlnx_common_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int mlnx_common_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(MLNX_COMMON_CONFIG_HEADER).header> */

#include "mlnx_common_porting.h"

#endif /* __MLNX_COMMON_CONFIG_H__ */
/* @} */
