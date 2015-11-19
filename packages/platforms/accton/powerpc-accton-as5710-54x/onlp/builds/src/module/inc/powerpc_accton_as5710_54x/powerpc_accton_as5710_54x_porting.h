/**************************************************************************//**
 *
 * @file
 * @brief powerpc_accton_as5710_54x Porting Macros.
 *
 * @addtogroup powerpc_accton_as5710_54x-porting
 * @{
 *
 *****************************************************************************/
#ifndef __POWERPC_ACCTON_AS5710_54X_R0_PORTING_H__
#define __POWERPC_ACCTON_AS5710_54X_R0_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if POWERPC_ACCTON_AS5710_54X_R0_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef POWERPC_ACCTON_AS5710_54X_R0_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define POWERPC_ACCTON_AS5710_54X_R0_MALLOC GLOBAL_MALLOC
    #elif POWERPC_ACCTON_AS5710_54X_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS5710_54X_R0_MALLOC malloc
    #else
        #error The macro POWERPC_ACCTON_AS5710_54X_R0_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS5710_54X_R0_FREE
    #if defined(GLOBAL_FREE)
        #define POWERPC_ACCTON_AS5710_54X_R0_FREE GLOBAL_FREE
    #elif POWERPC_ACCTON_AS5710_54X_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS5710_54X_R0_FREE free
    #else
        #error The macro POWERPC_ACCTON_AS5710_54X_R0_FREE is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS5710_54X_R0_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define POWERPC_ACCTON_AS5710_54X_R0_MEMSET GLOBAL_MEMSET
    #elif POWERPC_ACCTON_AS5710_54X_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS5710_54X_R0_MEMSET memset
    #else
        #error The macro POWERPC_ACCTON_AS5710_54X_R0_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS5710_54X_R0_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define POWERPC_ACCTON_AS5710_54X_R0_MEMCPY GLOBAL_MEMCPY
    #elif POWERPC_ACCTON_AS5710_54X_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS5710_54X_R0_MEMCPY memcpy
    #else
        #error The macro POWERPC_ACCTON_AS5710_54X_R0_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS5710_54X_R0_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define POWERPC_ACCTON_AS5710_54X_R0_STRNCPY GLOBAL_STRNCPY
    #elif POWERPC_ACCTON_AS5710_54X_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS5710_54X_R0_STRNCPY strncpy
    #else
        #error The macro POWERPC_ACCTON_AS5710_54X_R0_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS5710_54X_R0_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define POWERPC_ACCTON_AS5710_54X_R0_VSNPRINTF GLOBAL_VSNPRINTF
    #elif POWERPC_ACCTON_AS5710_54X_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS5710_54X_R0_VSNPRINTF vsnprintf
    #else
        #error The macro POWERPC_ACCTON_AS5710_54X_R0_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS5710_54X_R0_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define POWERPC_ACCTON_AS5710_54X_R0_SNPRINTF GLOBAL_SNPRINTF
    #elif POWERPC_ACCTON_AS5710_54X_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS5710_54X_R0_SNPRINTF snprintf
    #else
        #error The macro POWERPC_ACCTON_AS5710_54X_R0_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef POWERPC_ACCTON_AS5710_54X_R0_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define POWERPC_ACCTON_AS5710_54X_R0_STRLEN GLOBAL_STRLEN
    #elif POWERPC_ACCTON_AS5710_54X_R0_CONFIG_PORTING_STDLIB == 1
        #define POWERPC_ACCTON_AS5710_54X_R0_STRLEN strlen
    #else
        #error The macro POWERPC_ACCTON_AS5710_54X_R0_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __POWERPC_ACCTON_AS5710_54X_R0_PORTING_H__ */
/* @} */
