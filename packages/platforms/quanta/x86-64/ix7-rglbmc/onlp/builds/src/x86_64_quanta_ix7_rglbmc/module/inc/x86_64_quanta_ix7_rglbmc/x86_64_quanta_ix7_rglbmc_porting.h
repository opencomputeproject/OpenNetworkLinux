/**************************************************************************//**
 *
 * @file
 * @brief x86_64_quanta_ix7_rglbmc Porting Macros.
 *
 * @addtogroup x86_64_quanta_ix7_rglbmc-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_QUANTA_IX7_RGLBMC_PORTING_H__
#define __X86_64_QUANTA_IX7_RGLBMC_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_QUANTA_IX7_RGLBMC_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef X86_64_QUANTA_IX7_RGLBMC_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define X86_64_QUANTA_IX7_RGLBMC_MEMSET GLOBAL_MEMSET
    #elif X86_64_QUANTA_IX7_RGLBMC_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX7_RGLBMC_MEMSET memset
    #else
        #error The macro X86_64_QUANTA_IX7_RGLBMC_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_IX7_RGLBMC_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define X86_64_QUANTA_IX7_RGLBMC_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_QUANTA_IX7_RGLBMC_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX7_RGLBMC_MEMCPY memcpy
    #else
        #error The macro X86_64_QUANTA_IX7_RGLBMC_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_IX7_RGLBMC_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define X86_64_QUANTA_IX7_RGLBMC_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_QUANTA_IX7_RGLBMC_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX7_RGLBMC_STRNCPY strncpy
    #else
        #error The macro X86_64_QUANTA_IX7_RGLBMC_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_IX7_RGLBMC_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define X86_64_QUANTA_IX7_RGLBMC_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_QUANTA_IX7_RGLBMC_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX7_RGLBMC_VSNPRINTF vsnprintf
    #else
        #error The macro X86_64_QUANTA_IX7_RGLBMC_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_IX7_RGLBMC_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define X86_64_QUANTA_IX7_RGLBMC_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_QUANTA_IX7_RGLBMC_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX7_RGLBMC_SNPRINTF snprintf
    #else
        #error The macro X86_64_QUANTA_IX7_RGLBMC_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_IX7_RGLBMC_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define X86_64_QUANTA_IX7_RGLBMC_STRLEN GLOBAL_STRLEN
    #elif X86_64_QUANTA_IX7_RGLBMC_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_IX7_RGLBMC_STRLEN strlen
    #else
        #error The macro X86_64_QUANTA_IX7_RGLBMC_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __X86_64_QUANTA_IX7_RGLBMC_PORTING_H__ */
/* @} */
