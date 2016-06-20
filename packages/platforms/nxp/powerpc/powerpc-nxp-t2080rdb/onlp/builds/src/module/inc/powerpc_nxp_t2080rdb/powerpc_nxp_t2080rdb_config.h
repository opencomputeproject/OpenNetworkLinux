/****************************************************************************
 *
 * @file
 * @brief powerpc_nxp_t2080rdb Configuration Header
 *
 * @addtogroup powerpc_nxp_t2080rdb-config
 * @{
 *
 *****************************************************************************/
#ifndef __powerpc_nxp_T2080RDB_CONFIG_H__
#define __powerpc_nxp_T2080RDB_CONFIG_H__

#ifdef GLOBAL_INCLUDE_CUSTOM_CONFIG
#include <global_custom_config.h>
#endif
#ifdef powerpc_nxp_T2080RDB_INCLUDE_CUSTOM_CONFIG
#include <powerpc_nxp_t2080rdb_custom_config.h>
#endif

/* <auto.start.cdefs(powerpc_nxp_T2080RDB_CONFIG_HEADER).header> */
#include <AIM/aim.h>
/**
 * powerpc_nxp_T2080RDB_CONFIG_INCLUDE_LOGGING
 *
 * Include or exclude logging. */


#ifndef powerpc_nxp_T2080RDB_CONFIG_INCLUDE_LOGGING
#define powerpc_nxp_T2080RDB_CONFIG_INCLUDE_LOGGING 1
#endif

/**
 * powerpc_nxp_T2080RDB_CONFIG_LOG_OPTIONS_DEFAULT
 *
 * Default enabled log options. */


#ifndef powerpc_nxp_T2080RDB_CONFIG_LOG_OPTIONS_DEFAULT
#define powerpc_nxp_T2080RDB_CONFIG_LOG_OPTIONS_DEFAULT AIM_LOG_OPTIONS_DEFAULT
#endif

/**
 * powerpc_nxp_T2080RDB_CONFIG_LOG_BITS_DEFAULT
 *
 * Default enabled log bits. */


#ifndef powerpc_nxp_T2080RDB_CONFIG_LOG_BITS_DEFAULT
#define powerpc_nxp_T2080RDB_CONFIG_LOG_BITS_DEFAULT AIM_LOG_BITS_DEFAULT
#endif

/**
 * powerpc_nxp_T2080RDB_CONFIG_LOG_CUSTOM_BITS_DEFAULT
 *
 * Default enabled custom log bits. */


#ifndef powerpc_nxp_T2080RDB_CONFIG_LOG_CUSTOM_BITS_DEFAULT
#define powerpc_nxp_T2080RDB_CONFIG_LOG_CUSTOM_BITS_DEFAULT 0
#endif

/**
 * powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB
 *
 * Default all porting macros to use the C standard libraries. */


#ifndef powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB
#define powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB 1
#endif

/**
 * powerpc_nxp_T2080RDB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
 *
 * Include standard library headers for stdlib porting macros. */


#ifndef powerpc_nxp_T2080RDB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
#define powerpc_nxp_T2080RDB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB
#endif

/**
 * powerpc_nxp_T2080RDB_CONFIG_INCLUDE_UCLI
 *
 * Include generic uCli support. */


#ifndef powerpc_nxp_T2080RDB_CONFIG_INCLUDE_UCLI
#define powerpc_nxp_T2080RDB_CONFIG_INCLUDE_UCLI 0
#endif

/**
 * powerpc_nxp_T2080RDB_CONFIG_INCLUDE_DEBUG
 *
 * Include debug tool. */


#ifndef powerpc_nxp_T2080RDB_CONFIG_INCLUDE_DEBUG
#define powerpc_nxp_T2080RDB_CONFIG_INCLUDE_DEBUG 0
#endif



/**
 * All compile time options can be queried or displayed
 */

/** Configuration settings structure. */
typedef struct powerpc_nxp_t2080rdb_config_settings_s {
    /** name */
    const char* name;
    /** value */
    const char* value;
} powerpc_nxp_t2080rdb_config_settings_t;

/** Configuration settings table. */
/** powerpc_nxp_t2080rdb_config_settings table. */
extern powerpc_nxp_t2080rdb_config_settings_t powerpc_nxp_t2080rdb_config_settings[];

/**
 * @brief Lookup a configuration setting.
 * @param setting The name of the configuration option to lookup.
 */
const char* powerpc_nxp_t2080rdb_config_lookup(const char* setting);

/**
 * @brief Show the compile-time configuration.
 * @param pvs The output stream.
 */
int powerpc_nxp_t2080rdb_config_show(struct aim_pvs_s* pvs);

/* <auto.end.cdefs(powerpc_nxp_T2080RDB_CONFIG_HEADER).header> */

#include "powerpc_nxp_t2080rdb_porting.h"

#endif /* __powerpc_nxp_T2080RDB_CONFIG_H__ */
/* @} */
