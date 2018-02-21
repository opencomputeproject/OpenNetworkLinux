/**************************************************************************//**
 *
 * @file
 * @brief x86_64_accton_wedge100bf_65x Porting Macros.
 *
 * @addtogroup x86_64_accton_wedge100bf_65x-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_ACCTON_WEDGE100BF_65X_PORTING_H__
#define __X86_64_ACCTON_WEDGE100BF_65X_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_ACCTON_WEDGE100BF_65X_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_accton_wedge100bf_65x_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_accton_wedge100bf_65x_MALLOC GLOBAL_MALLOC
    #elif X86_64_ACCTON_WEDGE100BF_65X_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_wedge100bf_65x_MALLOC malloc
    #else
        #error The macro x86_64_accton_wedge100bf_65x_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_wedge100bf_65x_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_accton_wedge100bf_65x_FREE GLOBAL_FREE
    #elif X86_64_ACCTON_WEDGE100BF_65X_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_wedge100bf_65x_FREE free
    #else
        #error The macro x86_64_accton_wedge100bf_65x_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_wedge100bf_65x_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_accton_wedge100bf_65x_MEMSET GLOBAL_MEMSET
    #elif X86_64_ACCTON_WEDGE100BF_65X_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_wedge100bf_65x_MEMSET memset
    #else
        #error The macro x86_64_accton_wedge100bf_65x_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_wedge100bf_65x_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_accton_wedge100bf_65x_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_ACCTON_WEDGE100BF_65X_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_wedge100bf_65x_MEMCPY memcpy
    #else
        #error The macro x86_64_accton_wedge100bf_65x_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_wedge100bf_65x_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_accton_wedge100bf_65x_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_ACCTON_WEDGE100BF_65X_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_wedge100bf_65x_STRNCPY strncpy
    #else
        #error The macro x86_64_accton_wedge100bf_65x_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_wedge100bf_65x_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_accton_wedge100bf_65x_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_ACCTON_WEDGE100BF_65X_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_wedge100bf_65x_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_accton_wedge100bf_65x_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_wedge100bf_65x_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_accton_wedge100bf_65x_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_ACCTON_WEDGE100BF_65X_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_wedge100bf_65x_SNPRINTF snprintf
    #else
        #error The macro x86_64_accton_wedge100bf_65x_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_wedge100bf_65x_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_accton_wedge100bf_65x_STRLEN GLOBAL_STRLEN
    #elif X86_64_ACCTON_WEDGE100BF_65X_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_wedge100bf_65x_STRLEN strlen
    #else
        #error The macro x86_64_accton_wedge100bf_65x_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __X86_64_ACCTON_WEDGE100BF_65X_PORTING_H__ */
/* @} */
