/**************************************************************************//**
 *
 * @file
 * @brief x86_64_mitac_ly1200_b32h0_c3 Porting Macros.
 *
 * @addtogroup x86_64_mitac_ly1200_b32h0_c3-porting
 * @{
 *
 *****************************************************************************/
#ifndef __x86_64_mitac_ly1200_b32h0_c3_PORTING_H__
#define __x86_64_mitac_ly1200_b32h0_c3_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_MITAC_LY1200_B32H0_C3_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef X86_64_MITAC_LY1200_B32H0_C3_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define X86_64_MITAC_LY1200_B32H0_C3_MALLOC GLOBAL_MALLOC
    #elif X86_64_MITAC_LY1200_B32H0_C3_CONFIG_PORTING_STDLIB == 1
        #define X86_64_MITAC_LY1200_B32H0_C3_MALLOC malloc
    #else
        #error The macro X86_64_MITAC_LY1200_B32H0_C3_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_MITAC_LY1200_B32H0_C3_FREE
    #if defined(GLOBAL_FREE)
        #define X86_64_MITAC_LY1200_B32H0_C3_FREE GLOBAL_FREE
    #elif X86_64_MITAC_LY1200_B32H0_C3_CONFIG_PORTING_STDLIB == 1
        #define X86_64_MITAC_LY1200_B32H0_C3_FREE free
    #else
        #error The macro X86_64_MITAC_LY1200_B32H0_C3_FREE is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_MITAC_LY1200_B32H0_C3_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define X86_64_MITAC_LY1200_B32H0_C3_MEMSET GLOBAL_MEMSET
    #elif X86_64_MITAC_LY1200_B32H0_C3_CONFIG_PORTING_STDLIB == 1
        #define X86_64_MITAC_LY1200_B32H0_C3_MEMSET memset
    #else
        #error The macro X86_64_MITAC_LY1200_B32H0_C3_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_MITAC_LY1200_B32H0_C3_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define X86_64_MITAC_LY1200_B32H0_C3_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_MITAC_LY1200_B32H0_C3_CONFIG_PORTING_STDLIB == 1
        #define X86_64_MITAC_LY1200_B32H0_C3_MEMCPY memcpy
    #else
        #error The macro X86_64_MITAC_LY1200_B32H0_C3_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_MITAC_LY1200_B32H0_C3_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define X86_64_MITAC_LY1200_B32H0_C3_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_MITAC_LY1200_B32H0_C3_CONFIG_PORTING_STDLIB == 1
        #define X86_64_MITAC_LY1200_B32H0_C3_VSNPRINTF vsnprintf
    #else
        #error The macro X86_64_MITAC_LY1200_B32H0_C3_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_MITAC_LY1200_B32H0_C3_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define X86_64_MITAC_LY1200_B32H0_C3_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_MITAC_LY1200_B32H0_C3_CONFIG_PORTING_STDLIB == 1
        #define X86_64_MITAC_LY1200_B32H0_C3_SNPRINTF snprintf
    #else
        #error The macro X86_64_MITAC_LY1200_B32H0_C3_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_MITAC_LY1200_B32H0_C3_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define X86_64_MITAC_LY1200_B32H0_C3_STRLEN GLOBAL_STRLEN
    #elif X86_64_MITAC_LY1200_B32H0_C3_CONFIG_PORTING_STDLIB == 1
        #define X86_64_MITAC_LY1200_B32H0_C3_STRLEN strlen
    #else
        #error The macro X86_64_MITAC_LY1200_B32H0_C3_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __x86_64_mitac_ly1200_b32h0_c3_PORTING_H__ */
/* @} */
