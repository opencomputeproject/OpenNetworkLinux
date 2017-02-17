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
 * @brief x86_64_dni_wb2448 Porting Macros.
 *
 * @addtogroup x86_64_dni_wb2448-porting
 * @{
 *
 ***********************************************************/
#ifndef __x86_64_dni_wb2448_PORTING_H__
#define __x86_64_dni_wb2448_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if x86_64_dni_wb2448_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_dni_wb2448_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_dni_wb2448_MALLOC GLOBAL_MALLOC
    #elif x86_64_dni_wb2448_CONFIG_PORTING_STDLIB == 1
        #define x86_64_dni_wb2448_MALLOC malloc
    #else
        #error The macro x86_64_dni_wb2448_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_dni_wb2448_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_dni_wb2448_FREE GLOBAL_FREE
    #elif x86_64_dni_wb2448_CONFIG_PORTING_STDLIB == 1
        #define x86_64_dni_wb2448_FREE free
    #else
        #error The macro x86_64_dni_wb2448_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_dni_wb2448_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_dni_wb2448_MEMSET GLOBAL_MEMSET
    #elif x86_64_dni_wb2448_CONFIG_PORTING_STDLIB == 1
        #define x86_64_dni_wb2448_MEMSET memset
    #else
        #error The macro x86_64_dni_wb2448_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_dni_wb2448_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_dni_wb2448_MEMCPY GLOBAL_MEMCPY
    #elif x86_64_dni_wb2448_CONFIG_PORTING_STDLIB == 1
        #define x86_64_dni_wb2448_MEMCPY memcpy
    #else
        #error The macro x86_64_dni_wb2448_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_dni_wb2448_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_dni_wb2448_STRNCPY GLOBAL_STRNCPY
    #elif x86_64_dni_wb2448_CONFIG_PORTING_STDLIB == 1
        #define x86_64_dni_wb2448_STRNCPY strncpy
    #else
        #error The macro x86_64_dni_wb2448_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_dni_wb2448_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_dni_wb2448_VSNPRINTF GLOBAL_VSNPRINTF
    #elif x86_64_dni_wb2448_CONFIG_PORTING_STDLIB == 1
        #define x86_64_dni_wb2448_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_dni_wb2448_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_dni_wb2448_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_dni_wb2448_SNPRINTF GLOBAL_SNPRINTF
    #elif x86_64_dni_wb2448_CONFIG_PORTING_STDLIB == 1
        #define x86_64_dni_wb2448_SNPRINTF snprintf
    #else
        #error The macro x86_64_dni_wb2448_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_dni_wb2448_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_dni_wb2448_STRLEN GLOBAL_STRLEN
    #elif x86_64_dni_wb2448_CONFIG_PORTING_STDLIB == 1
        #define x86_64_dni_wb2448_STRLEN strlen
    #else
        #error The macro x86_64_dni_wb2448_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __x86_64_dni_wb2448_PORTING_H__ */
/* @} */
