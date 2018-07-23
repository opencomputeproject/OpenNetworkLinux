/**************************************************************************//**
 *
 * @file
 * @brief onlp_platform_sim Configuration Header
 *
 * @addtogroup onlp_platform_sim-config
 * @{
 *
 *****************************************************************************/
#ifndef __ONLP_PLATFORM_SIM_CONFIG_H__
#define __ONLP_PLATFORM_SIM_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef ONLP_PLATFORM_SIM_INCLUDE_CUSTOM_CONFIG
#include <onlp_platform_sim_custom_config.h>
#endif

/* <auto.start.cdefs(ONLP_PLATFORM_SIM_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * ONLP_PLATFORM_SIM_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef ONLP_PLATFORM_SIM_CONFIG_INCLUDE_LOGGING
#define ONLP_PLATFORM_SIM_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * ONLP_PLATFORM_SIM_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef ONLP_PLATFORM_SIM_CONFIG_LOG_OPTIONS_DEFAULT
#define ONLP_PLATFORM_SIM_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * ONLP_PLATFORM_SIM_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef ONLP_PLATFORM_SIM_CONFIG_LOG_BITS_DEFAULT
#define ONLP_PLATFORM_SIM_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * ONLP_PLATFORM_SIM_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef ONLP_PLATFORM_SIM_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define ONLP_PLATFORM_SIM_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * ONLP_PLATFORM_SIM_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef ONLP_PLATFORM_SIM_CONFIG_PORTING_STDLIB
#define ONLP_PLATFORM_SIM_CONFIG_PORTING_STDLIB 1
#endif

/**
 * ONLP_PLATFORM_SIM_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef ONLP_PLATFORM_SIM_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define ONLP_PLATFORM_SIM_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS ONLP_PLATFORM_SIM_CONFIG_PORTING_STDLIB
#endif

/**
 * ONLP_PLATFORM_SIM_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef ONLP_PLATFORM_SIM_CONFIG_INCLUDE_UCLI
#define ONLP_PLATFORM_SIM_CONFIG_INCLUDE_UCLI 0
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct onlp_platform_sim_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} onlp_platform_sim_config_settings_t;

/** Configuration settings table. */
/** onlp_platform_sim_config_settings table. */
extern onlp_platform_sim_config_settings_t onlp_platform_sim_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* onlp_platform_sim_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int onlp_platform_sim_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(ONLP_PLATFORM_SIM_CONFIG_HEADER).header> */

#include "onlp_platform_sim_porting.h"

#endif /* __ONLP_PLATFORM_SIM_CONFIG_H__ */
/* @} */
