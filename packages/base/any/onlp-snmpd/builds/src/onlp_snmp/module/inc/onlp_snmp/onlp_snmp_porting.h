/**************************************************************************//**
 *
 * @file
 * @brief onlp_snmp Porting Macros.
 *
 * @addtogroup onlp_snmp-porting
 * @{
 *
 *****************************************************************************/
#ifndef __ONLP_SNMP_PORTING_H__
#define __ONLP_SNMP_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if ONLP_SNMP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef ONLP_SNMP_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define ONLP_SNMP_MEMSET GLOBAL_MEMSET
    #elif ONLP_SNMP_CONFIG_PORTING_STDLIB == 1
        #define ONLP_SNMP_MEMSET memset
    #else
        #error The macro ONLP_SNMP_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef ONLP_SNMP_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define ONLP_SNMP_MEMCPY GLOBAL_MEMCPY
    #elif ONLP_SNMP_CONFIG_PORTING_STDLIB == 1
        #define ONLP_SNMP_MEMCPY memcpy
    #else
        #error The macro ONLP_SNMP_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef ONLP_SNMP_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define ONLP_SNMP_STRNCPY GLOBAL_STRNCPY
    #elif ONLP_SNMP_CONFIG_PORTING_STDLIB == 1
        #define ONLP_SNMP_STRNCPY strncpy
    #else
        #error The macro ONLP_SNMP_STRNCPY is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __ONLP_SNMP_PORTING_H__ */
/* @} */
