/**************************************************************************//**
 *
 * @file
 * @brief x86_64_delta_ag5648 Porting Macros.
 *
 * @addtogroup x86_64_delta_ag5648-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_DELTA_AG5648_PORTING_H__
#define __X86_64_DELTA_AG5648_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_DELTA_AG5648_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_delta_ag5648_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_delta_ag5648_MALLOC GLOBAL_MALLOC
    #elif X86_64_DELTA_AG5648_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag5648_MALLOC malloc
    #else
        #error The macro x86_64_delta_ag5648_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag5648_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_delta_ag5648_FREE GLOBAL_FREE
    #elif X86_64_DELTA_AG5648_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag5648_FREE free
    #else
        #error The macro x86_64_delta_ag5648_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag5648_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_delta_ag5648_MEMSET GLOBAL_MEMSET
    #elif X86_64_DELTA_AG5648_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag5648_MEMSET memset
    #else
        #error The macro x86_64_delta_ag5648_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag5648_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_delta_ag5648_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_DELTA_AG5648_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag5648_MEMCPY memcpy
    #else
        #error The macro x86_64_delta_ag5648_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag5648_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_delta_ag5648_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_DELTA_AG5648_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag5648_STRNCPY strncpy
    #else
        #error The macro x86_64_delta_ag5648_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag5648_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_delta_ag5648_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_DELTA_AG5648_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag5648_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_delta_ag5648_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag5648_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_delta_ag5648_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_DELTA_AG5648_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag5648_SNPRINTF snprintf
    #else
        #error The macro x86_64_delta_ag5648_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag5648_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_delta_ag5648_STRLEN GLOBAL_STRLEN
    #elif X86_64_DELTA_AG5648_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag5648_STRLEN strlen
    #else
        #error The macro x86_64_delta_ag5648_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* _X86_64_DELTA_AG5648_PORTING_H__ */
/* @} */
