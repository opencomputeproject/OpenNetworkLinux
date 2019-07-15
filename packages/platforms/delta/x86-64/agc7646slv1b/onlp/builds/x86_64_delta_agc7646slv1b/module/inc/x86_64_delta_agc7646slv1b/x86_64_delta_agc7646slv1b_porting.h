/**************************************************************************//**
 *
 * @file
 * @brief x86_64_delta_agc7646slv1b Porting Macros.
 *
 * @addtogroup x86_64_delta_agc7646slv1b-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_DELTA_AGC7646SLV1B_PORTING_H__
#define __X86_64_DELTA_AGC7646SLV1B_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_DELTA_AGC7646SLV1B_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef X86_64_DELTA_AGC7646SLV1B_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define X86_64_DELTA_AGC7646SLV1B_MALLOC GLOBAL_MALLOC
    #elif X86_64_DELTA_AGC7646SLV1B_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7646SLV1B_MALLOC malloc
    #else
        #error The macro X86_64_DELTA_AGC7646SLV1B_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7646SLV1B_FREE
    #if defined(GLOBAL_FREE)
        #define X86_64_DELTA_AGC7646SLV1B_FREE GLOBAL_FREE
    #elif X86_64_DELTA_AGC7646SLV1B_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7646SLV1B_FREE free
    #else
        #error The macro X86_64_DELTA_AGC7646SLV1B_FREE is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7646SLV1B_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define X86_64_DELTA_AGC7646SLV1B_MEMSET GLOBAL_MEMSET
    #elif X86_64_DELTA_AGC7646SLV1B_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7646SLV1B_MEMSET memset
    #else
        #error The macro X86_64_DELTA_AGC7646SLV1B_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7646SLV1B_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define X86_64_DELTA_AGC7646SLV1B_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_DELTA_AGC7646SLV1B_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7646SLV1B_MEMCPY memcpy
    #else
        #error The macro X86_64_DELTA_AGC7646SLV1B_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7646SLV1B_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define X86_64_DELTA_AGC7646SLV1B_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_DELTA_AGC7646SLV1B_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7646SLV1B_STRNCPY strncpy
    #else
        #error The macro X86_64_DELTA_AGC7646SLV1B_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7646SLV1B_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define X86_64_DELTA_AGC7646SLV1B_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_DELTA_AGC7646SLV1B_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7646SLV1B_VSNPRINTF vsnprintf
    #else
        #error The macro X86_64_DELTA_AGC7646SLV1B_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7646SLV1B_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define X86_64_DELTA_AGC7646SLV1B_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_DELTA_AGC7646SLV1B_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7646SLV1B_SNPRINTF snprintf
    #else
        #error The macro X86_64_DELTA_AGC7646SLV1B_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_DELTA_AGC7646SLV1B_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define X86_64_DELTA_AGC7646SLV1B_STRLEN GLOBAL_STRLEN
    #elif X86_64_DELTA_AGC7646SLV1B_CONFIG_PORTING_STDLIB == 1
        #define X86_64_DELTA_AGC7646SLV1B_STRLEN strlen
    #else
        #error The macro X86_64_DELTA_AGC7646SLV1B_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* _X86_64_DELTA_AGC7646SLV1B_PORTING_H__ */
/* @} */
