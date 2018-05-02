/**************************************************************************//**
 *
 * @file
 * @brief x86_64_delta_ak7448 Porting Macros.
 *
 * @addtogroup x86_64_delta_ak7448-porting
 * @{
 *
 *****************************************************************************/
#ifndef __x86_64_delta_ak7448_PORTING_H__
#define __x86_64_delta_ak7448_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_DELTA_AK7448_CONFIGPORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_delta_ak7448_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_delta_ak7448_MALLOC GLOBAL_MALLOC
    #elif X86_64_DELTA_AK7448_CONFIGPORTING_STDLIB == 1
        #define x86_64_delta_ak7448_MALLOC malloc
    #else
        #error The macro x86_64_delta_ak7448_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ak7448_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_delta_ak7448_FREE GLOBAL_FREE
    #elif X86_64_DELTA_AK7448_CONFIGPORTING_STDLIB == 1
        #define x86_64_delta_ak7448_FREE free
    #else
        #error The macro x86_64_delta_ak7448_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ak7448_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_delta_ak7448_MEMSET GLOBAL_MEMSET
    #elif X86_64_DELTA_AK7448_CONFIGPORTING_STDLIB == 1
        #define x86_64_delta_ak7448_MEMSET memset
    #else
        #error The macro x86_64_delta_ak7448_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ak7448_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_delta_ak7448_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_DELTA_AK7448_CONFIGPORTING_STDLIB == 1
        #define x86_64_delta_ak7448_MEMCPY memcpy
    #else
        #error The macro x86_64_delta_ak7448_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ak7448_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_delta_ak7448_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_DELTA_AK7448_CONFIGPORTING_STDLIB == 1
        #define x86_64_delta_ak7448_STRNCPY strncpy
    #else
        #error The macro x86_64_delta_ak7448_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ak7448_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_delta_ak7448_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_DELTA_AK7448_CONFIGPORTING_STDLIB == 1
        #define x86_64_delta_ak7448_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_delta_ak7448_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ak7448_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_delta_ak7448_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_DELTA_AK7448_CONFIGPORTING_STDLIB == 1
        #define x86_64_delta_ak7448_SNPRINTF snprintf
    #else
        #error The macro x86_64_delta_ak7448_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ak7448_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_delta_ak7448_STRLEN GLOBAL_STRLEN
    #elif X86_64_DELTA_AK7448_CONFIGPORTING_STDLIB == 1
        #define x86_64_delta_ak7448_STRLEN strlen
    #else
        #error The macro x86_64_delta_ak7448_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __x86_64_delta_ak7448_PORTING_H__ */
/* @} */
