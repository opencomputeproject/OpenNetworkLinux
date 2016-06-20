/****************************************************************************
 *
 * @file
 * @brief powerpc_nxp_t2080rdb Porting Macros.
 *
 * @addtogroup powerpc_nxp_t2080rdb-porting
 * @{
 *
 *****************************************************************************/
#ifndef __powerpc_nxp_T2080RDB_PORTING_H__
#define __powerpc_nxp_T2080RDB_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if powerpc_nxp_T2080RDB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef powerpc_nxp_T2080RDB_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define powerpc_nxp_T2080RDB_MALLOC GLOBAL_MALLOC
    #elif powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB == 1
        #define powerpc_nxp_T2080RDB_MALLOC malloc
    #else
        #error The macro powerpc_nxp_T2080RDB_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef powerpc_nxp_T2080RDB_FREE
    #if defined(GLOBAL_FREE)
        #define powerpc_nxp_T2080RDB_FREE GLOBAL_FREE
    #elif powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB == 1
        #define powerpc_nxp_T2080RDB_FREE free
    #else
        #error The macro powerpc_nxp_T2080RDB_FREE is required but cannot be defined.
    #endif
#endif

#ifndef powerpc_nxp_T2080RDB_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define powerpc_nxp_T2080RDB_MEMSET GLOBAL_MEMSET
    #elif powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB == 1
        #define powerpc_nxp_T2080RDB_MEMSET memset
    #else
        #error The macro powerpc_nxp_T2080RDB_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef powerpc_nxp_T2080RDB_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define powerpc_nxp_T2080RDB_MEMCPY GLOBAL_MEMCPY
    #elif powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB == 1
        #define powerpc_nxp_T2080RDB_MEMCPY memcpy
    #else
        #error The macro powerpc_nxp_T2080RDB_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef powerpc_nxp_T2080RDB_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define powerpc_nxp_T2080RDB_STRNCPY GLOBAL_STRNCPY
    #elif powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB == 1
        #define powerpc_nxp_T2080RDB_STRNCPY strncpy
    #else
        #error The macro powerpc_nxp_T2080RDB_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef powerpc_nxp_T2080RDB_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define powerpc_nxp_T2080RDB_VSNPRINTF GLOBAL_VSNPRINTF
    #elif powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB == 1
        #define powerpc_nxp_T2080RDB_VSNPRINTF vsnprintf
    #else
        #error The macro powerpc_nxp_T2080RDB_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef powerpc_nxp_T2080RDB_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define powerpc_nxp_T2080RDB_SNPRINTF GLOBAL_SNPRINTF
    #elif powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB == 1
        #define powerpc_nxp_T2080RDB_SNPRINTF snprintf
    #else
        #error The macro powerpc_nxp_T2080RDB_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef powerpc_nxp_T2080RDB_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define powerpc_nxp_T2080RDB_STRLEN GLOBAL_STRLEN
    #elif powerpc_nxp_T2080RDB_CONFIG_PORTING_STDLIB == 1
        #define powerpc_nxp_T2080RDB_STRLEN strlen
    #else
        #error The macro powerpc_nxp_T2080RDB_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __powerpc_nxp_T2080RDB_PORTING_H__ */
/* @} */
