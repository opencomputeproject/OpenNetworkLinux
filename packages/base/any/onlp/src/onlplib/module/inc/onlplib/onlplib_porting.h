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


/**************************************************************************//**
 *
 * @file
 * @brief onlplib Porting Macros.
 *
 * @addtogroup onlplib-porting
 * @{
 *
 *****************************************************************************/
#ifndef __ONLPLIB_PORTING_H__
#define __ONLPLIB_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if ONLPLIB_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef ONLPLIB_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define ONLPLIB_MEMSET GLOBAL_MEMSET
    #elif ONLPLIB_CONFIG_PORTING_STDLIB == 1
        #define ONLPLIB_MEMSET memset
    #else
        #error The macro ONLPLIB_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef ONLPLIB_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define ONLPLIB_MEMCPY GLOBAL_MEMCPY
    #elif ONLPLIB_CONFIG_PORTING_STDLIB == 1
        #define ONLPLIB_MEMCPY memcpy
    #else
        #error The macro ONLPLIB_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef ONLPLIB_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define ONLPLIB_STRNCPY GLOBAL_STRNCPY
    #elif ONLPLIB_CONFIG_PORTING_STDLIB == 1
        #define ONLPLIB_STRNCPY strncpy
    #else
        #error The macro ONLPLIB_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef ONLPLIB_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define ONLPLIB_VSNPRINTF GLOBAL_VSNPRINTF
    #elif ONLPLIB_CONFIG_PORTING_STDLIB == 1
        #define ONLPLIB_VSNPRINTF vsnprintf
    #else
        #error The macro ONLPLIB_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ONLPLIB_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define ONLPLIB_SNPRINTF GLOBAL_SNPRINTF
    #elif ONLPLIB_CONFIG_PORTING_STDLIB == 1
        #define ONLPLIB_SNPRINTF snprintf
    #else
        #error The macro ONLPLIB_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ONLPLIB_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define ONLPLIB_STRLEN GLOBAL_STRLEN
    #elif ONLPLIB_CONFIG_PORTING_STDLIB == 1
        #define ONLPLIB_STRLEN strlen
    #else
        #error The macro ONLPLIB_STRLEN is required but cannot be defined.
    #endif
#endif

#ifndef ONLPLIB_ATOI
    #if defined(GLOBAL_ATOI)
        #define ONLPLIB_ATOI GLOBAL_ATOI
    #elif ONLPLIB_CONFIG_PORTING_STDLIB == 1
        #define ONLPLIB_ATOI atoi
    #else
        #error The macro ONLPLIB_ATOI is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __ONLPLIB_PORTING_H__ */
/* @} */
