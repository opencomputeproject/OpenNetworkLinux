/**************************************************************************//**
 *
 * @file
 * @brief x86_64_quanta_ly4r Porting Macros.
 *
 * @addtogroup x86_64_quanta_ly4r-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_QUANTA_LY4R_PORTING_H__
#define __X86_64_QUANTA_LY4R_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_QUANTA_LY4R_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef X86_64_QUANTA_LY4R_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define X86_64_QUANTA_LY4R_MEMSET GLOBAL_MEMSET
    #elif X86_64_QUANTA_LY4R_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_LY4R_MEMSET memset
    #else
        #error The macro X86_64_QUANTA_LY4R_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_LY4R_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define X86_64_QUANTA_LY4R_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_QUANTA_LY4R_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_LY4R_MEMCPY memcpy
    #else
        #error The macro X86_64_QUANTA_LY4R_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_LY4R_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define X86_64_QUANTA_LY4R_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_QUANTA_LY4R_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_LY4R_STRNCPY strncpy
    #else
        #error The macro X86_64_QUANTA_LY4R_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_LY4R_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define X86_64_QUANTA_LY4R_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_QUANTA_LY4R_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_LY4R_VSNPRINTF vsnprintf
    #else
        #error The macro X86_64_QUANTA_LY4R_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_LY4R_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define X86_64_QUANTA_LY4R_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_QUANTA_LY4R_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_LY4R_SNPRINTF snprintf
    #else
        #error The macro X86_64_QUANTA_LY4R_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_QUANTA_LY4R_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define X86_64_QUANTA_LY4R_STRLEN GLOBAL_STRLEN
    #elif X86_64_QUANTA_LY4R_CONFIG_PORTING_STDLIB == 1
        #define X86_64_QUANTA_LY4R_STRLEN strlen
    #else
        #error The macro X86_64_QUANTA_LY4R_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __X86_64_QUANTA_LY4R_PORTING_H__ */
/* @} */
