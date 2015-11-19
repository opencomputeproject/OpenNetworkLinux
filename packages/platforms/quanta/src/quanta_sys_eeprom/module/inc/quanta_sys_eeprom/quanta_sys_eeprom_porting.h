/**************************************************************************//**
 *
 * @file
 * @brief quanta_sys_eeprom Porting Macros.
 *
 * @addtogroup quanta_sys_eeprom-porting
 * @{
 *
 *****************************************************************************/
#ifndef __QUANTA_SYS_EEPROM_PORTING_H__
#define __QUANTA_SYS_EEPROM_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if QUANTA_SYS_EEPROM_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef QUANTA_SYS_EEPROM_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define QUANTA_SYS_EEPROM_MALLOC GLOBAL_MALLOC
    #elif QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB == 1
        #define QUANTA_SYS_EEPROM_MALLOC malloc
    #else
        #error The macro QUANTA_SYS_EEPROM_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef QUANTA_SYS_EEPROM_FREE
    #if defined(GLOBAL_FREE)
        #define QUANTA_SYS_EEPROM_FREE GLOBAL_FREE
    #elif QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB == 1
        #define QUANTA_SYS_EEPROM_FREE free
    #else
        #error The macro QUANTA_SYS_EEPROM_FREE is required but cannot be defined.
    #endif
#endif

#ifndef QUANTA_SYS_EEPROM_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define QUANTA_SYS_EEPROM_MEMSET GLOBAL_MEMSET
    #elif QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB == 1
        #define QUANTA_SYS_EEPROM_MEMSET memset
    #else
        #error The macro QUANTA_SYS_EEPROM_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef QUANTA_SYS_EEPROM_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define QUANTA_SYS_EEPROM_MEMCPY GLOBAL_MEMCPY
    #elif QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB == 1
        #define QUANTA_SYS_EEPROM_MEMCPY memcpy
    #else
        #error The macro QUANTA_SYS_EEPROM_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef QUANTA_SYS_EEPROM_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define QUANTA_SYS_EEPROM_STRNCPY GLOBAL_STRNCPY
    #elif QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB == 1
        #define QUANTA_SYS_EEPROM_STRNCPY strncpy
    #else
        #error The macro QUANTA_SYS_EEPROM_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef QUANTA_SYS_EEPROM_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define QUANTA_SYS_EEPROM_VSNPRINTF GLOBAL_VSNPRINTF
    #elif QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB == 1
        #define QUANTA_SYS_EEPROM_VSNPRINTF vsnprintf
    #else
        #error The macro QUANTA_SYS_EEPROM_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef QUANTA_SYS_EEPROM_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define QUANTA_SYS_EEPROM_SNPRINTF GLOBAL_SNPRINTF
    #elif QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB == 1
        #define QUANTA_SYS_EEPROM_SNPRINTF snprintf
    #else
        #error The macro QUANTA_SYS_EEPROM_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef QUANTA_SYS_EEPROM_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define QUANTA_SYS_EEPROM_STRLEN GLOBAL_STRLEN
    #elif QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB == 1
        #define QUANTA_SYS_EEPROM_STRLEN strlen
    #else
        #error The macro QUANTA_SYS_EEPROM_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __QUANTA_SYS_EEPROM_PORTING_H__ */
/* @} */
