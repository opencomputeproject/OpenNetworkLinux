/**************************************************************************//**
 *
 * @file
 * @brief mlnx_common Porting Macros.
 *
 * @addtogroup mlnx_common-porting
 * @{
 *
 *****************************************************************************/
#ifndef __MLNX_COMMON_PORTING_H__
#define __MLNX_COMMON_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if MLNX_COMMON_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef MLNX_COMMON_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define MLNX_COMMON_MALLOC GLOBAL_MALLOC
    #elif MLNX_COMMON_CONFIG_PORTING_STDLIB == 1
        #define MLNX_COMMON_MALLOC malloc
    #else
        #error The macro MLNX_COMMON_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef MLNX_COMMON_FREE
    #if defined(GLOBAL_FREE)
        #define MLNX_COMMON_FREE GLOBAL_FREE
    #elif MLNX_COMMON_CONFIG_PORTING_STDLIB == 1
        #define MLNX_COMMON_FREE free
    #else
        #error The macro MLNX_COMMON_FREE is required but cannot be defined.
    #endif
#endif

#ifndef MLNX_COMMON_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define MLNX_COMMON_MEMSET GLOBAL_MEMSET
    #elif MLNX_COMMON_CONFIG_PORTING_STDLIB == 1
        #define MLNX_COMMON_MEMSET memset
    #else
        #error The macro MLNX_COMMON_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef MLNX_COMMON_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define MLNX_COMMON_MEMCPY GLOBAL_MEMCPY
    #elif MLNX_COMMON_CONFIG_PORTING_STDLIB == 1
        #define MLNX_COMMON_MEMCPY memcpy
    #else
        #error The macro MLNX_COMMON_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef MLNX_COMMON_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define MLNX_COMMON_STRNCPY GLOBAL_STRNCPY
    #elif MLNX_COMMON_CONFIG_PORTING_STDLIB == 1
        #define MLNX_COMMON_STRNCPY strncpy
    #else
        #error The macro MLNX_COMMON_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef MLNX_COMMON_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define MLNX_COMMON_VSNPRINTF GLOBAL_VSNPRINTF
    #elif MLNX_COMMON_CONFIG_PORTING_STDLIB == 1
        #define MLNX_COMMON_VSNPRINTF vsnprintf
    #else
        #error The macro MLNX_COMMON_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef MLNX_COMMON_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define MLNX_COMMON_SNPRINTF GLOBAL_SNPRINTF
    #elif MLNX_COMMON_CONFIG_PORTING_STDLIB == 1
        #define MLNX_COMMON_SNPRINTF snprintf
    #else
        #error The macro MLNX_COMMON_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef MLNX_COMMON_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define MLNX_COMMON_STRLEN GLOBAL_STRLEN
    #elif MLNX_COMMON_CONFIG_PORTING_STDLIB == 1
        #define MLNX_COMMON_STRLEN strlen
    #else
        #error The macro MLNX_COMMON_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __MLNX_COMMON_PORTING_H__ */
/* @} */
