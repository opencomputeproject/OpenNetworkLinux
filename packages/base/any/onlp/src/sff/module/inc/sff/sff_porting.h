/**************************************************************************//**
 *
 * @file
 * @brief sff Porting Macros.
 *
 * @addtogroup sff-porting
 * @{
 *
 *****************************************************************************/
#ifndef __SFF_PORTING_H__
#define __SFF_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if SFF_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef SFF_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define SFF_MEMSET GLOBAL_MEMSET
    #elif SFF_CONFIG_PORTING_STDLIB == 1
        #define SFF_MEMSET memset
    #else
        #error The macro SFF_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef SFF_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define SFF_MEMCPY GLOBAL_MEMCPY
    #elif SFF_CONFIG_PORTING_STDLIB == 1
        #define SFF_MEMCPY memcpy
    #else
        #error The macro SFF_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef SFF_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define SFF_STRNCPY GLOBAL_STRNCPY
    #elif SFF_CONFIG_PORTING_STDLIB == 1
        #define SFF_STRNCPY strncpy
    #else
        #error The macro SFF_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef SFF_STRNCMP
    #if defined(GLOBAL_STRNCMP)
        #define SFF_STRNCMP GLOBAL_STRNCMP
    #elif SFF_CONFIG_PORTING_STDLIB == 1
        #define SFF_STRNCMP strncmp
    #else
        #error The macro SFF_STRNCMP is required but cannot be defined.
    #endif
#endif

#ifndef SFF_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define SFF_STRLEN GLOBAL_STRLEN
    #elif SFF_CONFIG_PORTING_STDLIB == 1
        #define SFF_STRLEN strlen
    #else
        #error The macro SFF_STRLEN is required but cannot be defined.
    #endif
#endif

#ifndef SFF_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define SFF_SNPRINTF GLOBAL_SNPRINTF
    #elif SFF_CONFIG_PORTING_STDLIB == 1
        #define SFF_SNPRINTF snprintf
    #else
        #error The macro SFF_SNPRINTF is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __SFF_PORTING_H__ */
/* @} */
