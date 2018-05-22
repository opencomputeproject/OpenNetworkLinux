/**************************************************************************//**
 *
 * @file
 * @brief x86_64_delta_ag9032v2 Porting Macros.
 *
 * @addtogroup x86_64_delta_ag9032v2-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_DELTA_AG9032V2_PORTING_H__
#define __X86_64_DELTA_AG9032V2_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_DELTA_AG9032V2_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_delta_ag9032v2_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_delta_ag9032v2_MALLOC GLOBAL_MALLOC
    #elif X86_64_DELTA_AG9032V2_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v2_MALLOC malloc
    #else
        #error The macro x86_64_delta_ag9032v2_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v2_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_delta_ag9032v2_FREE GLOBAL_FREE
    #elif X86_64_DELTA_AG9032V2_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v2_FREE free
    #else
        #error The macro x86_64_delta_ag9032v2_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v2_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_delta_ag9032v2_MEMSET GLOBAL_MEMSET
    #elif X86_64_DELTA_AG9032V2_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v2_MEMSET memset
    #else
        #error The macro x86_64_delta_ag9032v2_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v2_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_delta_ag9032v2_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_DELTA_AG9032V2_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v2_MEMCPY memcpy
    #else
        #error The macro x86_64_delta_ag9032v2_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v2_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_delta_ag9032v2_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_DELTA_AG9032V2_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v2_STRNCPY strncpy
    #else
        #error The macro x86_64_delta_ag9032v2_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v2_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_delta_ag9032v2_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_DELTA_AG9032V2_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v2_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_delta_ag9032v2_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v2_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_delta_ag9032v2_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_DELTA_AG9032V2_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v2_SNPRINTF snprintf
    #else
        #error The macro x86_64_delta_ag9032v2_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v2_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_delta_ag9032v2_STRLEN GLOBAL_STRLEN
    #elif X86_64_DELTA_AG9032V2_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v2_STRLEN strlen
    #else
        #error The macro x86_64_delta_ag9032v2_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* _X86_64_DELTA_AG9032V2_PORTING_H__ */
/* @} */
