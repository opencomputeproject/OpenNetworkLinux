/**************************************************************************//**
 *
 * @file
 * @brief x86_64_delta_ag9032v1 Porting Macros.
 *
 * @addtogroup x86_64_delta_ag9032v1-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_DELTA_AG9032V1_PORTING_H__
#define __X86_64_DELTA_AG9032V1_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_DELTA_AG9032V1_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_delta_ag9032v1_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_delta_ag9032v1_MALLOC GLOBAL_MALLOC
    #elif X86_64_DELTA_AG9032V1_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v1_MALLOC malloc
    #else
        #error The macro x86_64_delta_ag9032v1_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v1_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_delta_ag9032v1_FREE GLOBAL_FREE
    #elif X86_64_DELTA_AG9032V1_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v1_FREE free
    #else
        #error The macro x86_64_delta_ag9032v1_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v1_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_delta_ag9032v1_MEMSET GLOBAL_MEMSET
    #elif X86_64_DELTA_AG9032V1_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v1_MEMSET memset
    #else
        #error The macro x86_64_delta_ag9032v1_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v1_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_delta_ag9032v1_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_DELTA_AG9032V1_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v1_MEMCPY memcpy
    #else
        #error The macro x86_64_delta_ag9032v1_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v1_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_delta_ag9032v1_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_DELTA_AG9032V1_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v1_STRNCPY strncpy
    #else
        #error The macro x86_64_delta_ag9032v1_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v1_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_delta_ag9032v1_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_DELTA_AG9032V1_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v1_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_delta_ag9032v1_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v1_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_delta_ag9032v1_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_DELTA_AG9032V1_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v1_SNPRINTF snprintf
    #else
        #error The macro x86_64_delta_ag9032v1_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag9032v1_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_delta_ag9032v1_STRLEN GLOBAL_STRLEN
    #elif X86_64_DELTA_AG9032V1_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag9032v1_STRLEN strlen
    #else
        #error The macro x86_64_delta_ag9032v1_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* _X86_64_DELTA_AG9032V1_PORTING_H__ */
/* @} */
