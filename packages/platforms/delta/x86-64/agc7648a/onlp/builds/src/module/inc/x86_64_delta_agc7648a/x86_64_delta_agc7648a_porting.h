/**************************************************************************//**
 *
 * @file
 * @brief x86_64_delta_agc7648a Porting Macros.
 *
 * @addtogroup x86_64_delta_agc7648a-porting
 * @{
 *
 *****************************************************************************/
#ifndef __x86_64_delta_agc7648a_PORTING_H__
#define __x86_64_delta_agc7648a_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_DELTA_AGC7648A_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef X86_64_DELTA_AGC7648A_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define X86_64_DELTA_AGC7648A_MALLOC GLOBAL_MALLOC
    #elif X86_64_DELTA_AGC7648A_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7648A_MALLOC malloc
    #else
        #error The macro X86_64_DELTA_AGC7648A_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7648A_FREE
    #if defined(GLOBAL_FREE)
        #define X86_64_DELTA_AGC7648A_FREE GLOBAL_FREE
    #elif X86_64_DELTA_AGC7648A_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7648A_FREE free
    #else
        #error The macro X86_64_DELTA_AGC7648A_FREE is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7648A_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define X86_64_DELTA_AGC7648A_MEMSET GLOBAL_MEMSET
    #elif X86_64_DELTA_AGC7648A_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7648A_MEMSET memset
    #else
        #error The macro X86_64_DELTA_AGC7648A_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7648A_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define X86_64_DELTA_AGC7648A_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_DELTA_AGC7648A_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7648A_MEMCPY memcpy
    #else
        #error The macro X86_64_DELTA_AGC7648A_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7648A_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define X86_64_DELTA_AGC7648A_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_DELTA_AGC7648A_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7648A_STRNCPY strncpy
    #else
        #error The macro X86_64_DELTA_AGC7648A_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7648A_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define X86_64_DELTA_AGC7648A_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_DELTA_AGC7648A_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7648A_VSNPRINTF vsnprintf
    #else
        #error The macro X86_64_DELTA_AGC7648A_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7648A_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define X86_64_DELTA_AGC7648A_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_DELTA_AGC7648A_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7648A_SNPRINTF snprintf
    #else
        #error The macro X86_64_DELTA_AGC7648A_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7648A_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define X86_64_DELTA_AGC7648A_STRLEN GLOBAL_STRLEN
    #elif X86_64_DELTA_AGC7648A_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7648A_STRLEN strlen
    #else
        #error The macro X86_64_DELTA_AGC7648A_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __x86_64_delta_agc7648a_PORTING_H__ */
/* @} */
