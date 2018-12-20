/**************************************************************************//**
 *
 * @file
 * @brief x86_64_delta_ag7648c Porting Macros.
 *
 * @addtogroup x86_64_delta_ag7648c-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_DELTA_AG7648C_PORTING_H__
#define __X86_64_DELTA_AG7648C_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_DELTA_AG7648C_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_delta_ag7648c_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_delta_ag7648c_MALLOC GLOBAL_MALLOC
    #elif X86_64_DELTA_AG7648C_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag7648c_MALLOC malloc
    #else
        #error The macro x86_64_delta_ag7648c_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag7648c_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_delta_ag7648c_FREE GLOBAL_FREE
    #elif X86_64_DELTA_AG7648C_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag7648c_FREE free
    #else
        #error The macro x86_64_delta_ag7648c_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag7648c_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_delta_ag7648c_MEMSET GLOBAL_MEMSET
    #elif X86_64_DELTA_AG7648C_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag7648c_MEMSET memset
    #else
        #error The macro x86_64_delta_ag7648c_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag7648c_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_delta_ag7648c_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_DELTA_AG7648C_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag7648c_MEMCPY memcpy
    #else
        #error The macro x86_64_delta_ag7648c_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag7648c_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_delta_ag7648c_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_DELTA_AG7648C_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag7648c_STRNCPY strncpy
    #else
        #error The macro x86_64_delta_ag7648c_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag7648c_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_delta_ag7648c_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_DELTA_AG7648C_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag7648c_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_delta_ag7648c_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag7648c_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_delta_ag7648c_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_DELTA_AG7648C_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag7648c_SNPRINTF snprintf
    #else
        #error The macro x86_64_delta_ag7648c_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_delta_ag7648c_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_delta_ag7648c_STRLEN GLOBAL_STRLEN
    #elif X86_64_DELTA_AG7648C_CONFIG_PORTING_STDLIB == 1
        #define x86_64_delta_ag7648c_STRLEN strlen
    #else
        #error The macro x86_64_delta_ag7648c_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __X86_64_DELTA_AG7648C_PORTING_H__ */
/* @} */
