/**************************************************************************//**
 *
 * @file
 * @brief onlp_snmp Configuration Header
 *
 * @addtogroup onlp_snmp-config
 * @{
 *
 *****************************************************************************/
#ifndef __ONLP_SNMP_CONFIG_H__
#define __ONLP_SNMP_CONFIG_H__

#include <onlp/oids.h>

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef ONLP_SNMP_INCLUDE_CUSTOM_CONFIG
#include <onlp_snmp_custom_config.h>
#endif

/* <auto.start.cdefs(ONLP_SNMP_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * ONLP_SNMP_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef ONLP_SNMP_CONFIG_INCLUDE_LOGGING
#define ONLP_SNMP_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * ONLP_SNMP_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef ONLP_SNMP_CONFIG_LOG_OPTIONS_DEFAULT
#define ONLP_SNMP_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * ONLP_SNMP_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef ONLP_SNMP_CONFIG_LOG_BITS_DEFAULT
#define ONLP_SNMP_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * ONLP_SNMP_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef ONLP_SNMP_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define ONLP_SNMP_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * ONLP_SNMP_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef ONLP_SNMP_CONFIG_PORTING_STDLIB
#define ONLP_SNMP_CONFIG_PORTING_STDLIB 1
#endif

/**
 * ONLP_SNMP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef ONLP_SNMP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define ONLP_SNMP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS ONLP_SNMP_CONFIG_PORTING_STDLIB
#endif

/**
 * ONLP_SNMP_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef ONLP_SNMP_CONFIG_INCLUDE_UCLI
#define ONLP_SNMP_CONFIG_INCLUDE_UCLI 0
#endif

/**
 * ONLP_SNMP_CONFIG_MAX_NAME_LENGTH
 *
 * Maximum object name length. */


#ifndef ONLP_SNMP_CONFIG_MAX_NAME_LENGTH
#define ONLP_SNMP_CONFIG_MAX_NAME_LENGTH 32
#endif

/**
 * ONLP_SNMP_CONFIG_MAX_DESC_LENGTH
 *
 * Maximum object description length. */


#ifndef ONLP_SNMP_CONFIG_MAX_DESC_LENGTH
#define ONLP_SNMP_CONFIG_MAX_DESC_LENGTH ONLP_OID_DESC_SIZE
#endif

/**
 * ONLP_SNMP_CONFIG_UPDATE_PERIOD
 *
 * Default update period in seconds. */


#ifndef ONLP_SNMP_CONFIG_UPDATE_PERIOD
#define ONLP_SNMP_CONFIG_UPDATE_PERIOD 5
#endif

/**
 * ONLP_SNMP_CONFIG_DEV_BASE_INDEX
 *
 * Base index. */


#ifndef ONLP_SNMP_CONFIG_DEV_BASE_INDEX
#define ONLP_SNMP_CONFIG_DEV_BASE_INDEX 1
#endif

/**
 * ONLP_SNMP_CONFIG_DEV_MAX_INDEX
 *
 * Maximum index. */


#ifndef ONLP_SNMP_CONFIG_DEV_MAX_INDEX
#define ONLP_SNMP_CONFIG_DEV_MAX_INDEX 100
#endif

/**
 * ONLP_SNMP_CONFIG_INCLUDE_THERMALS
 *
 * Include Thermals. */


#ifndef ONLP_SNMP_CONFIG_INCLUDE_THERMALS
#define ONLP_SNMP_CONFIG_INCLUDE_THERMALS 1
#endif

/**
 * ONLP_SNMP_CONFIG_INCLUDE_FANS
 *
 * Include Fans. */


#ifndef ONLP_SNMP_CONFIG_INCLUDE_FANS
#define ONLP_SNMP_CONFIG_INCLUDE_FANS 1
#endif

/**
 * ONLP_SNMP_CONFIG_INCLUDE_PSUS
 *
 * Include PSUS. */


#ifndef ONLP_SNMP_CONFIG_INCLUDE_PSUS
#define ONLP_SNMP_CONFIG_INCLUDE_PSUS 1
#endif

/**
 * ONLP_SNMP_CONFIG_INCLUDE_LEDS
 *
 * Include LEDs. */


#ifndef ONLP_SNMP_CONFIG_INCLUDE_LEDS
#define ONLP_SNMP_CONFIG_INCLUDE_LEDS 1
#endif

/**
 * ONLP_SNMP_CONFIG_INCLUDE_PLATFORM
 *
 * Include ONLP Platform MIB */


#ifndef ONLP_SNMP_CONFIG_INCLUDE_PLATFORM
#define ONLP_SNMP_CONFIG_INCLUDE_PLATFORM 1
#endif

/**
 * ONLP_SNMP_CONFIG_AS_SUBAGENT
 *
 * Configure as an snmp_subagent client. */


#ifndef ONLP_SNMP_CONFIG_AS_SUBAGENT
#define ONLP_SNMP_CONFIG_AS_SUBAGENT 0
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct onlp_snmp_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} onlp_snmp_config_settings_t;

/** Configuration settings table. */
/** onlp_snmp_config_settings table. */
extern onlp_snmp_config_settings_t onlp_snmp_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* onlp_snmp_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int onlp_snmp_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(ONLP_SNMP_CONFIG_HEADER).header> */

#include "onlp_snmp_porting.h"

#endif /* __ONLP_SNMP_CONFIG_H__ */
/* @} */
