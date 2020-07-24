/**************************************************************************//**
 *
 * @file
 * @brief x86_64_cel_silverstone Porting Macros.
 *
 * @addtogroup x86_64_cel_silverstone-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_CEL_SILVERSTONE_PORTING_H__
#define __X86_64_CEL_SILVERSTONE_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_CEL_SILVERSTONE_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef X86_64_CEL_SILVERSTONE_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define X86_64_CEL_SILVERSTONE_MALLOC GLOBAL_MALLOC
    #elif X86_64_CEL_SILVERSTONE_CONFIG_PORTING_STDLIB == 1
        #define X86_64_CEL_SILVERSTONE_MALLOC malloc
    #else
        #error The macro X86_64_CEL_SILVERSTONE_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_CEL_SILVERSTONE_FREE
    #if defined(GLOBAL_FREE)
        #define X86_64_CEL_SILVERSTONE_FREE GLOBAL_FREE
    #elif X86_64_CEL_SILVERSTONE_CONFIG_PORTING_STDLIB == 1
        #define X86_64_CEL_SILVERSTONE_FREE free
    #else
        #error The macro X86_64_CEL_SILVERSTONE_FREE is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_CEL_SILVERSTONE_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define X86_64_CEL_SILVERSTONE_MEMSET GLOBAL_MEMSET
    #elif X86_64_CEL_SILVERSTONE_CONFIG_PORTING_STDLIB == 1
        #define X86_64_CEL_SILVERSTONE_MEMSET memset
    #else
        #error The macro X86_64_CEL_SILVERSTONE_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_CEL_SILVERSTONE_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define X86_64_CEL_SILVERSTONE_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_CEL_SILVERSTONE_CONFIG_PORTING_STDLIB == 1
        #define X86_64_CEL_SILVERSTONE_MEMCPY memcpy
    #else
        #error The macro X86_64_CEL_SILVERSTONE_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_CEL_SILVERSTONE_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define X86_64_CEL_SILVERSTONE_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_CEL_SILVERSTONE_CONFIG_PORTING_STDLIB == 1
        #define X86_64_CEL_SILVERSTONE_STRNCPY strncpy
    #else
        #error The macro X86_64_CEL_SILVERSTONE_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_CEL_SILVERSTONE_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define X86_64_CEL_SILVERSTONE_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_CEL_SILVERSTONE_CONFIG_PORTING_STDLIB == 1
        #define X86_64_CEL_SILVERSTONE_VSNPRINTF vsnprintf
    #else
        #error The macro X86_64_CEL_SILVERSTONE_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_CEL_SILVERSTONE_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define X86_64_CEL_SILVERSTONE_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_CEL_SILVERSTONE_CONFIG_PORTING_STDLIB == 1
        #define X86_64_CEL_SILVERSTONE_SNPRINTF snprintf
    #else
        #error The macro X86_64_CEL_SILVERSTONE_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_CEL_SILVERSTONE_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define X86_64_CEL_SILVERSTONE_STRLEN GLOBAL_STRLEN
    #elif X86_64_CEL_SILVERSTONE_CONFIG_PORTING_STDLIB == 1
        #define X86_64_CEL_SILVERSTONE_STRLEN strlen
    #else
        #error The macro X86_64_CEL_SILVERSTONE_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __X86_64_CEL_SILVERSTONE_PORTING_H__ */
/* @} */
