/**************************************************************************//**
 *
 * @file
 * @brief x86_64_delta_ag7648 Porting Macros.
 *
 * @addtogroup x86_64_delta_ag7648-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_DELTA_AG7648_PORTING_H__
#define __X86_64_DELTA_AG7648_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_DELTA_AG7648_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef X86_64_DELTA_AG7648_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define X86_64_DELTA_AG7648_MALLOC GLOBAL_MALLOC
    #elif X86_64_DELTA_AG7648_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AG7648_MALLOC malloc
    #else
        #error The macro X86_64_DELTA_AG7648_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AG7648_FREE
    #if defined(GLOBAL_FREE)
        #define X86_64_DELTA_AG7648_FREE GLOBAL_FREE
    #elif X86_64_DELTA_AG7648_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AG7648_FREE free
    #else
        #error The macro X86_64_DELTA_AG7648_FREE is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AG7648_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define X86_64_DELTA_AG7648_MEMSET GLOBAL_MEMSET
    #elif X86_64_DELTA_AG7648_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AG7648_MEMSET memset
    #else
        #error The macro X86_64_DELTA_AG7648_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AG7648_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define X86_64_DELTA_AG7648_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_DELTA_AG7648_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AG7648_MEMCPY memcpy
    #else
        #error The macro X86_64_DELTA_AG7648_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AG7648_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define X86_64_DELTA_AG7648_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_DELTA_AG7648_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AG7648_STRNCPY strncpy
    #else
        #error The macro X86_64_DELTA_AG7648_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AG7648_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define X86_64_DELTA_AG7648_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_DELTA_AG7648_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AG7648_VSNPRINTF vsnprintf
    #else
        #error The macro X86_64_DELTA_AG7648_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AG7648_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define X86_64_DELTA_AG7648_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_DELTA_AG7648_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AG7648_SNPRINTF snprintf
    #else
        #error The macro X86_64_DELTA_AG7648_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AG7648_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define X86_64_DELTA_AG7648_STRLEN GLOBAL_STRLEN
    #elif X86_64_DELTA_AG7648_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AG7648_STRLEN strlen
    #else
        #error The macro X86_64_DELTA_AG7648_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __X86_64_DELTA_AG7648_PORTING_H__ */
/* @} */
