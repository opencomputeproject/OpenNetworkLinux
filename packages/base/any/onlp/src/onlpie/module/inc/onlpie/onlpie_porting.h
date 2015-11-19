/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * 
 *        Copyright 2014, 2015 Big Switch Networks, Inc.       
 * 
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * 
 *        http://www.eclipse.org/legal/epl-v10.html
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 * 
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/

/********************************************************//**
 *
 * @file
 * @brief onlpie Porting Macros.
 *
 * @addtogroup onlpie-porting
 * @{
 *
 ***********************************************************/
#ifndef __ONLPIE_PORTING_H__
#define __ONLPIE_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if ONLPIE_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef ONLPIE_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define ONLPIE_MALLOC GLOBAL_MALLOC
    #elif ONLPIE_CONFIG_PORTING_STDLIB == 1
        #define ONLPIE_MALLOC malloc
    #else
        #error The macro ONLPIE_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef ONLPIE_FREE
    #if defined(GLOBAL_FREE)
        #define ONLPIE_FREE GLOBAL_FREE
    #elif ONLPIE_CONFIG_PORTING_STDLIB == 1
        #define ONLPIE_FREE free
    #else
        #error The macro ONLPIE_FREE is required but cannot be defined.
    #endif
#endif

#ifndef ONLPIE_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define ONLPIE_MEMSET GLOBAL_MEMSET
    #elif ONLPIE_CONFIG_PORTING_STDLIB == 1
        #define ONLPIE_MEMSET memset
    #else
        #error The macro ONLPIE_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef ONLPIE_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define ONLPIE_MEMCPY GLOBAL_MEMCPY
    #elif ONLPIE_CONFIG_PORTING_STDLIB == 1
        #define ONLPIE_MEMCPY memcpy
    #else
        #error The macro ONLPIE_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef ONLPIE_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define ONLPIE_STRNCPY GLOBAL_STRNCPY
    #elif ONLPIE_CONFIG_PORTING_STDLIB == 1
        #define ONLPIE_STRNCPY strncpy
    #else
        #error The macro ONLPIE_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef ONLPIE_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define ONLPIE_VSNPRINTF GLOBAL_VSNPRINTF
    #elif ONLPIE_CONFIG_PORTING_STDLIB == 1
        #define ONLPIE_VSNPRINTF vsnprintf
    #else
        #error The macro ONLPIE_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ONLPIE_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define ONLPIE_SNPRINTF GLOBAL_SNPRINTF
    #elif ONLPIE_CONFIG_PORTING_STDLIB == 1
        #define ONLPIE_SNPRINTF snprintf
    #else
        #error The macro ONLPIE_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ONLPIE_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define ONLPIE_STRLEN GLOBAL_STRLEN
    #elif ONLPIE_CONFIG_PORTING_STDLIB == 1
        #define ONLPIE_STRLEN strlen
    #else
        #error The macro ONLPIE_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __ONLPIE_PORTING_H__ */
/* @} */
