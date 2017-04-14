/**************************************************************************//**
 *
 * @file
 * @brief x86_64_quanta_ix1_rangeley Porting Macros.
 *
 * @addtogroup x86_64_quanta_ix1_rangeley-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_QUANTA_IX1_RANGELEY_PORTING_H__
#define __X86_64_QUANTA_IX1_RANGELEY_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_QUANTA_IX1_RANGELEY_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef X86_64_QUANTA_IX1_RANGELEY_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define X86_64_QUANTA_IX1_RANGELEY_MEMSET GLOBAL_MEMSET
    #elif X86_64_QUANTA_IX1_RANGELEY_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX1_RANGELEY_MEMSET memset
    #else
        #error The macro X86_64_QUANTA_IX1_RANGELEY_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_IX1_RANGELEY_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define X86_64_QUANTA_IX1_RANGELEY_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_QUANTA_IX1_RANGELEY_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX1_RANGELEY_MEMCPY memcpy
    #else
        #error The macro X86_64_QUANTA_IX1_RANGELEY_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_IX1_RANGELEY_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define X86_64_QUANTA_IX1_RANGELEY_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_QUANTA_IX1_RANGELEY_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX1_RANGELEY_STRNCPY strncpy
    #else
        #error The macro X86_64_QUANTA_IX1_RANGELEY_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_IX1_RANGELEY_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define X86_64_QUANTA_IX1_RANGELEY_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_QUANTA_IX1_RANGELEY_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX1_RANGELEY_VSNPRINTF vsnprintf
    #else
        #error The macro X86_64_QUANTA_IX1_RANGELEY_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_IX1_RANGELEY_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define X86_64_QUANTA_IX1_RANGELEY_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_QUANTA_IX1_RANGELEY_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX1_RANGELEY_SNPRINTF snprintf
    #else
        #error The macro X86_64_QUANTA_IX1_RANGELEY_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_IX1_RANGELEY_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define X86_64_QUANTA_IX1_RANGELEY_STRLEN GLOBAL_STRLEN
    #elif X86_64_QUANTA_IX1_RANGELEY_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX1_RANGELEY_STRLEN strlen
    #else
        #error The macro X86_64_QUANTA_IX1_RANGELEY_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __X86_64_QUANTA_IX1_RANGELEY_PORTING_H__ */
/* @} */
