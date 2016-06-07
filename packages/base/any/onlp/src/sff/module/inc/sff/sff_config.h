/**************************************************************************//**
 *
 * @file
 * @brief sff Configuration Header
 *
 * @addtogroup sff-config
 * @{
 *
 *****************************************************************************/
#ifndef __SFF_CONFIG_H__
#define __SFF_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef SFF_INCLUDE_CUSTOM_CONFIG
#include <sff_custom_config.h>
#endif

/* <auto.start.cdefs(SFF_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * SFF_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef SFF_CONFIG_INCLUDE_LOGGING
#define SFF_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * SFF_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef SFF_CONFIG_LOG_OPTIONS_DEFAULT
#define SFF_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * SFF_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef SFF_CONFIG_LOG_BITS_DEFAULT
#define SFF_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * SFF_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef SFF_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define SFF_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * SFF_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef SFF_CONFIG_PORTING_STDLIB
#define SFF_CONFIG_PORTING_STDLIB 1
#endif

/**
 * SFF_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef SFF_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define SFF_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS SFF_CONFIG_PORTING_STDLIB
#endif

/**
 * SFF_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef SFF_CONFIG_INCLUDE_UCLI
#define SFF_CONFIG_INCLUDE_UCLI 0
#endif

/**
 * SFF_CONFIG_INCLUDE_SFF_TOOL
 *
 * Include the SFF tool main entry point. */


#ifndef SFF_CONFIG_INCLUDE_SFF_TOOL
#define SFF_CONFIG_INCLUDE_SFF_TOOL 0
#endif

/**
 * SFF_CONFIG_INCLUDE_EXT_CC_CHECK
 *
 * Include extended checksum verification. */


#ifndef SFF_CONFIG_INCLUDE_EXT_CC_CHECK
#define SFF_CONFIG_INCLUDE_EXT_CC_CHECK 0
#endif

/**
 * SFF_CONFIG_INCLUDE_DATABASE
 *
 * Include eeprom database. */


#ifndef SFF_CONFIG_INCLUDE_DATABASE
#define SFF_CONFIG_INCLUDE_DATABASE 1
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct sff_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} sff_config_settings_t;

/** Configuration settings table. */
/** sff_config_settings table. */
extern sff_config_settings_t sff_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* sff_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int sff_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(SFF_CONFIG_HEADER).header> */

#include "sff_porting.h"

#endif /* __SFF_CONFIG_H__ */
/* @} */
