/**************************************************************************//**
 *
 * @file
 * @brief powerpc_quanta_ly2 Configuration Header
 *
 * @addtogroup powerpc_quanta_ly2-config
 * @{
 *
 *****************************************************************************/
#ifndef __POWERPC_QUANTA_LY2_R0_CONFIG_H__
#define __POWERPC_QUANTA_LY2_R0_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef POWERPC_QUANTA_LY2_R0_INCLUDE_CUSTOM_CONFIG
#include <powerpc_quanta_ly2_custom_config.h>
#endif

/* <auto.start.cdefs(POWERPC_QUANTA_LY2_R0_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * POWERPC_QUANTA_LY2_R0_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef POWERPC_QUANTA_LY2_R0_CONFIG_INCLUDE_LOGGING
#define POWERPC_QUANTA_LY2_R0_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * POWERPC_QUANTA_LY2_R0_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef POWERPC_QUANTA_LY2_R0_CONFIG_LOG_OPTIONS_DEFAULT
#define POWERPC_QUANTA_LY2_R0_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * POWERPC_QUANTA_LY2_R0_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef POWERPC_QUANTA_LY2_R0_CONFIG_LOG_BITS_DEFAULT
#define POWERPC_QUANTA_LY2_R0_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * POWERPC_QUANTA_LY2_R0_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef POWERPC_QUANTA_LY2_R0_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define POWERPC_QUANTA_LY2_R0_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_STDLIB
#define POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_STDLIB 1
#endif

/**
 * POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_STDLIB
#endif

/**
 * POWERPC_QUANTA_LY2_R0_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef POWERPC_QUANTA_LY2_R0_CONFIG_INCLUDE_UCLI
#define POWERPC_QUANTA_LY2_R0_CONFIG_INCLUDE_UCLI 0
#endif

/**
 * POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD
 *
 * RPM Threshold at which the fan is considered to have failed. */


#ifndef POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD
#define POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD 100
#endif

/**
 * POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_F2B_RPM_MAX
 *
 * Maximum system front-to-back fan speed. */


#ifndef POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_F2B_RPM_MAX
#define POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_F2B_RPM_MAX 13000
#endif

/**
 * POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_B2F_RPM_MAX
 *
 * Maximum system back-to-front fan speed. */


#ifndef POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_B2F_RPM_MAX
#define POWERPC_QUANTA_LY2_R0_CONFIG_SYSFAN_B2F_RPM_MAX 24000
#endif

/**
 * POWERPC_QUANTA_LY2_R0_CONFIG_PHY_RESET_DELAY_MS
 *
 * Time to hold Phy GPIO in reset, in ms */


#ifndef POWERPC_QUANTA_LY2_R0_CONFIG_PHY_RESET_DELAY_MS
#define POWERPC_QUANTA_LY2_R0_CONFIG_PHY_RESET_DELAY_MS 100
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct powerpc_quanta_ly2_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} powerpc_quanta_ly2_config_settings_t;

/** Configuration settings table. */
/** powerpc_quanta_ly2_config_settings table. */
extern powerpc_quanta_ly2_config_settings_t powerpc_quanta_ly2_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* powerpc_quanta_ly2_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int powerpc_quanta_ly2_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(POWERPC_QUANTA_LY2_R0_CONFIG_HEADER).header> */

#include "powerpc_quanta_ly2_porting.h"

#endif /* __POWERPC_QUANTA_LY2_R0_CONFIG_H__ */
/* @} */
