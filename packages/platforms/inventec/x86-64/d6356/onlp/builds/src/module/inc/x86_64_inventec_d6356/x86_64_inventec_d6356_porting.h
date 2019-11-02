/**************************************************************************//**
 *
 * @file
 * @brief x86_64_inventec_d6356 Porting Macros.
 *
 * @addtogroup x86_64_inventec_d6356-porting
 * @{
 *
 *****************************************************************************/
#ifndef __x86_64_inventec_d6356_PORTING_H__
#define __x86_64_inventec_d6356_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if x86_64_inventec_d6356_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_inventec_d6356_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_inventec_d6356_MALLOC GLOBAL_MALLOC
    #elif x86_64_inventec_d6356_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6356_MALLOC malloc
    #else
        #error The macro x86_64_inventec_d6356_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6356_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_inventec_d6356_FREE GLOBAL_FREE
    #elif x86_64_inventec_d6356_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6356_FREE free
    #else
        #error The macro x86_64_inventec_d6356_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6356_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_inventec_d6356_MEMSET GLOBAL_MEMSET
    #elif x86_64_inventec_d6356_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6356_MEMSET memset
    #else
        #error The macro x86_64_inventec_d6356_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6356_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_inventec_d6356_MEMCPY GLOBAL_MEMCPY
    #elif x86_64_inventec_d6356_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6356_MEMCPY memcpy
    #else
        #error The macro x86_64_inventec_d6356_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6356_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_inventec_d6356_STRNCPY GLOBAL_STRNCPY
    #elif x86_64_inventec_d6356_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6356_STRNCPY strncpy
    #else
        #error The macro x86_64_inventec_d6356_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6356_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_inventec_d6356_VSNPRINTF GLOBAL_VSNPRINTF
    #elif x86_64_inventec_d6356_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6356_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_inventec_d6356_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6356_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_inventec_d6356_SNPRINTF GLOBAL_SNPRINTF
    #elif x86_64_inventec_d6356_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6356_SNPRINTF snprintf
    #else
        #error The macro x86_64_inventec_d6356_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6356_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_inventec_d6356_STRLEN GLOBAL_STRLEN
    #elif x86_64_inventec_d6356_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6356_STRLEN strlen
    #else
        #error The macro x86_64_inventec_d6356_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __x86_64_inventec_d6356_PORTING_H__ */
/* @} */
