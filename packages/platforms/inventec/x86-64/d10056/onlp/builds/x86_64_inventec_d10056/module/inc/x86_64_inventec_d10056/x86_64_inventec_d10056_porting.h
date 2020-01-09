/**************************************************************************//**
 *
 * @file
 * @brief x86_64_inventec_d10056 Porting Macros.
 *
 * @addtogroup x86_64_inventec_d10056-porting
 * @{
 *
 *****************************************************************************/
#ifndef __x86_64_inventec_d10056_PORTING_H__
#define __x86_64_inventec_d10056_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if x86_64_inventec_d10056_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_inventec_d10056_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_inventec_d10056_MALLOC GLOBAL_MALLOC
    #elif x86_64_inventec_d10056_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d10056_MALLOC malloc
    #else
        #error The macro x86_64_inventec_d10056_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d10056_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_inventec_d10056_FREE GLOBAL_FREE
    #elif x86_64_inventec_d10056_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d10056_FREE free
    #else
        #error The macro x86_64_inventec_d10056_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d10056_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_inventec_d10056_MEMSET GLOBAL_MEMSET
    #elif x86_64_inventec_d10056_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d10056_MEMSET memset
    #else
        #error The macro x86_64_inventec_d10056_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d10056_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_inventec_d10056_MEMCPY GLOBAL_MEMCPY
    #elif x86_64_inventec_d10056_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d10056_MEMCPY memcpy
    #else
        #error The macro x86_64_inventec_d10056_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d10056_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_inventec_d10056_STRNCPY GLOBAL_STRNCPY
    #elif x86_64_inventec_d10056_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d10056_STRNCPY strncpy
    #else
        #error The macro x86_64_inventec_d10056_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d10056_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_inventec_d10056_VSNPRINTF GLOBAL_VSNPRINTF
    #elif x86_64_inventec_d10056_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d10056_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_inventec_d10056_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d10056_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_inventec_d10056_SNPRINTF GLOBAL_SNPRINTF
    #elif x86_64_inventec_d10056_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d10056_SNPRINTF snprintf
    #else
        #error The macro x86_64_inventec_d10056_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d10056_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_inventec_d10056_STRLEN GLOBAL_STRLEN
    #elif x86_64_inventec_d10056_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d10056_STRLEN strlen
    #else
        #error The macro x86_64_inventec_d10056_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __x86_64_inventec_d10056_PORTING_H__ */
/* @} */
