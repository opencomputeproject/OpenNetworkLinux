/**************************************************************************//**
 *
 * @file
 * @brief x86_64_wnc_rseb_w1_32 Porting Macros.
 *
 * @addtogroup x86_64_wnc_rseb_w1_32-porting
 * @{
 *
 *****************************************************************************/
#ifndef __x86_64_wnc_rseb_w1_32_PORTING_H__
#define __x86_64_wnc_rseb_w1_32_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_WNC_RSEB_W1_32_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef X86_64_WNC_RSEB_W1_32_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define X86_64_WNC_RSEB_W1_32_MALLOC GLOBAL_MALLOC
    #elif X86_64_WNC_RSEB_W1_32_CONFIG_PORTING_STDLIB == 1
        #define X86_64_WNC_RSEB_W1_32_MALLOC malloc
    #else
        #error The macro X86_64_WNC_RSEB_W1_32_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_WNC_RSEB_W1_32_FREE
    #if defined(GLOBAL_FREE)
        #define X86_64_WNC_RSEB_W1_32_FREE GLOBAL_FREE
    #elif X86_64_WNC_RSEB_W1_32_CONFIG_PORTING_STDLIB == 1
        #define X86_64_WNC_RSEB_W1_32_FREE free
    #else
        #error The macro X86_64_WNC_RSEB_W1_32_FREE is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_WNC_RSEB_W1_32_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define X86_64_WNC_RSEB_W1_32_MEMSET GLOBAL_MEMSET
    #elif X86_64_WNC_RSEB_W1_32_CONFIG_PORTING_STDLIB == 1
        #define X86_64_WNC_RSEB_W1_32_MEMSET memset
    #else
        #error The macro X86_64_WNC_RSEB_W1_32_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_WNC_RSEB_W1_32_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define X86_64_WNC_RSEB_W1_32_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_WNC_RSEB_W1_32_CONFIG_PORTING_STDLIB == 1
        #define X86_64_WNC_RSEB_W1_32_MEMCPY memcpy
    #else
        #error The macro X86_64_WNC_RSEB_W1_32_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_WNC_RSEB_W1_32_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define X86_64_WNC_RSEB_W1_32_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_WNC_RSEB_W1_32_CONFIG_PORTING_STDLIB == 1
        #define X86_64_WNC_RSEB_W1_32_STRNCPY strncpy
    #else
        #error The macro X86_64_WNC_RSEB_W1_32_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_WNC_RSEB_W1_32_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define X86_64_WNC_RSEB_W1_32_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_WNC_RSEB_W1_32_CONFIG_PORTING_STDLIB == 1
        #define X86_64_WNC_RSEB_W1_32_VSNPRINTF vsnprintf
    #else
        #error The macro X86_64_WNC_RSEB_W1_32_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_wnc_rseb_w1_32_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_wnc_rseb_w1_32_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_WNC_RSEB_W1_32_CONFIG_PORTING_STDLIB == 1
        #define x86_64_wnc_rseb_w1_32_SNPRINTF snprintf
    #else
        #error The macro x86_64_wnc_rseb_w1_32_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_WNC_RSEB_W1_32_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define X86_64_WNC_RSEB_W1_32_STRLEN GLOBAL_STRLEN
    #elif X86_64_WNC_RSEB_W1_32_CONFIG_PORTING_STDLIB == 1
        #define X86_64_WNC_RSEB_W1_32_STRLEN strlen
    #else
        #error The macro X86_64_WNC_RSEB_W1_32_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __x86_64_wnc_rseb_w1_32_PORTING_H__ */
/* @} */
