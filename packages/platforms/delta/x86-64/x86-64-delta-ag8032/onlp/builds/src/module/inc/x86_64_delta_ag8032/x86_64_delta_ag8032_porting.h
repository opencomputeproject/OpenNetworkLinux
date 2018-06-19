/**************************************************************************//**
 *
 * @file
 * @brief x86_64_delta_ag8032 Porting Macros.
 *
 * @addtogroup x86_64_delta_ag8032-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_DELTA_AG8032_PORTING_H__
#define __X86_64_DELTA_AG8032_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_DELTA_AG8032_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_delta_ag8032_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_delta_ag8032_MALLOC GLOBAL_MALLOC
    #elif X86_64_DELTA_AG8032_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag8032_MALLOC malloc
    #else
        #error The macro x86_64_delta_ag8032_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag8032_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_delta_ag8032_FREE GLOBAL_FREE
    #elif X86_64_DELTA_AG8032_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag8032_FREE free
    #else
        #error The macro x86_64_delta_ag8032_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag8032_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_delta_ag8032_MEMSET GLOBAL_MEMSET
    #elif X86_64_DELTA_AG8032_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag8032_MEMSET memset
    #else
        #error The macro x86_64_delta_ag8032_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag8032_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_delta_ag8032_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_DELTA_AG8032_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag8032_MEMCPY memcpy
    #else
        #error The macro x86_64_delta_ag8032_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag8032_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_delta_ag8032_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_DELTA_AG8032_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag8032_STRNCPY strncpy
    #else
        #error The macro x86_64_delta_ag8032_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag8032_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_delta_ag8032_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_DELTA_AG8032_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag8032_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_delta_ag8032_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag8032_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_delta_ag8032_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_DELTA_AG8032_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag8032_SNPRINTF snprintf
    #else
        #error The macro x86_64_delta_ag8032_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag8032_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_delta_ag8032_STRLEN GLOBAL_STRLEN
    #elif X86_64_DELTA_AG8032_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag8032_STRLEN strlen
    #else
        #error The macro x86_64_delta_ag8032_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __X86_64_DELTA_AG8032_PORTING_H__ */
/* @} */
