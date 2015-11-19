/**************************************************************************//**
 *
 * @file
 * @brief powerpc_quanta_ly2 Porting Macros.
 *
 * @addtogroup powerpc_quanta_ly2-porting
 * @{
 *
 *****************************************************************************/
#ifndef __POWERPC_QUANTA_LY2_R0_PORTING_H__
#define __POWERPC_QUANTA_LY2_R0_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef POWERPC_QUANTA_LY2_R0_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define POWERPC_QUANTA_LY2_R0_MEMSET GLOBAL_MEMSET
    #elif POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_QUANTA_LY2_R0_MEMSET memset
    #else
        #error The macro POWERPC_QUANTA_LY2_R0_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_QUANTA_LY2_R0_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define POWERPC_QUANTA_LY2_R0_MEMCPY GLOBAL_MEMCPY
    #elif POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_QUANTA_LY2_R0_MEMCPY memcpy
    #else
        #error The macro POWERPC_QUANTA_LY2_R0_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_QUANTA_LY2_R0_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define POWERPC_QUANTA_LY2_R0_STRNCPY GLOBAL_STRNCPY
    #elif POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_QUANTA_LY2_R0_STRNCPY strncpy
    #else
        #error The macro POWERPC_QUANTA_LY2_R0_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_QUANTA_LY2_R0_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define POWERPC_QUANTA_LY2_R0_VSNPRINTF GLOBAL_VSNPRINTF
    #elif POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_QUANTA_LY2_R0_VSNPRINTF vsnprintf
    #else
        #error The macro POWERPC_QUANTA_LY2_R0_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_QUANTA_LY2_R0_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define POWERPC_QUANTA_LY2_R0_SNPRINTF GLOBAL_SNPRINTF
    #elif POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_QUANTA_LY2_R0_SNPRINTF snprintf
    #else
        #error The macro POWERPC_QUANTA_LY2_R0_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_QUANTA_LY2_R0_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define POWERPC_QUANTA_LY2_R0_STRLEN GLOBAL_STRLEN
    #elif POWERPC_QUANTA_LY2_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_QUANTA_LY2_R0_STRLEN strlen
    #else
        #error The macro POWERPC_QUANTA_LY2_R0_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __POWERPC_QUANTA_LY2_R0_PORTING_H__ */
/* @} */
