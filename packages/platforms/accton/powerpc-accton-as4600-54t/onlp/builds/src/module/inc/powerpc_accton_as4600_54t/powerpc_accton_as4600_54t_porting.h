/**************************************************************************//**
 *
 * @file
 * @brief powerpc_accton_as4600_54t Porting Macros.
 *
 * @addtogroup powerpc_accton_as4600_54t-porting
 * @{
 *
 *****************************************************************************/
#ifndef __POWERPC_ACCTON_AS4600_54T_PORTING_H__
#define __POWERPC_ACCTON_AS4600_54T_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef POWERPC_ACCTON_AS4600_54T_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define POWERPC_ACCTON_AS4600_54T_MALLOC GLOBAL_MALLOC
    #elif POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS4600_54T_MALLOC malloc
    #else
        #error The macro POWERPC_ACCTON_AS4600_54T_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS4600_54T_FREE
    #if defined(GLOBAL_FREE)
        #define POWERPC_ACCTON_AS4600_54T_FREE GLOBAL_FREE
    #elif POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS4600_54T_FREE free
    #else
        #error The macro POWERPC_ACCTON_AS4600_54T_FREE is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS4600_54T_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define POWERPC_ACCTON_AS4600_54T_MEMSET GLOBAL_MEMSET
    #elif POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS4600_54T_MEMSET memset
    #else
        #error The macro POWERPC_ACCTON_AS4600_54T_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS4600_54T_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define POWERPC_ACCTON_AS4600_54T_MEMCPY GLOBAL_MEMCPY
    #elif POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS4600_54T_MEMCPY memcpy
    #else
        #error The macro POWERPC_ACCTON_AS4600_54T_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS4600_54T_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define POWERPC_ACCTON_AS4600_54T_STRNCPY GLOBAL_STRNCPY
    #elif POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS4600_54T_STRNCPY strncpy
    #else
        #error The macro POWERPC_ACCTON_AS4600_54T_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS4600_54T_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define POWERPC_ACCTON_AS4600_54T_VSNPRINTF GLOBAL_VSNPRINTF
    #elif POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS4600_54T_VSNPRINTF vsnprintf
    #else
        #error The macro POWERPC_ACCTON_AS4600_54T_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS4600_54T_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define POWERPC_ACCTON_AS4600_54T_SNPRINTF GLOBAL_SNPRINTF
    #elif POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS4600_54T_SNPRINTF snprintf
    #else
        #error The macro POWERPC_ACCTON_AS4600_54T_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS4600_54T_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define POWERPC_ACCTON_AS4600_54T_STRLEN GLOBAL_STRLEN
    #elif POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS4600_54T_STRLEN strlen
    #else
        #error The macro POWERPC_ACCTON_AS4600_54T_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __POWERPC_ACCTON_AS4600_54T_PORTING_H__ */
/* @} */
