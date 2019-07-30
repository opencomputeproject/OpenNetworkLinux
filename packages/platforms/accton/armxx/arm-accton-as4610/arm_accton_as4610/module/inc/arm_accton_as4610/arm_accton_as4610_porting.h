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
 * @brief arm_accton_as4610 Porting Macros.
 *
 * @addtogroup arm_accton_as4610-porting
 * @{
 *
 ***********************************************************/
#ifndef __ARM_ACCTON_AS4610_PORTING_H__
#define __ARM_ACCTON_AS4610_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if ARM_ACCTON_AS4610_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef ARM_ACCTON_AS4610_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define ARM_ACCTON_AS4610_MALLOC GLOBAL_MALLOC
    #elif ARM_ACCTON_AS4610_CONFIG_PORTING_STDLIB == 1
        #define ARM_ACCTON_AS4610_MALLOC malloc
    #else
        #error The macro ARM_ACCTON_AS4610_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef ARM_ACCTON_AS4610_FREE
    #if defined(GLOBAL_FREE)
        #define ARM_ACCTON_AS4610_FREE GLOBAL_FREE
    #elif ARM_ACCTON_AS4610_CONFIG_PORTING_STDLIB == 1
        #define ARM_ACCTON_AS4610_FREE free
    #else
        #error The macro ARM_ACCTON_AS4610_FREE is required but cannot be defined.
    #endif
#endif

#ifndef ARM_ACCTON_AS4610_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define ARM_ACCTON_AS4610_MEMSET GLOBAL_MEMSET
    #elif ARM_ACCTON_AS4610_CONFIG_PORTING_STDLIB == 1
        #define ARM_ACCTON_AS4610_MEMSET memset
    #else
        #error The macro ARM_ACCTON_AS4610_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef ARM_ACCTON_AS4610_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define ARM_ACCTON_AS4610_MEMCPY GLOBAL_MEMCPY
    #elif ARM_ACCTON_AS4610_CONFIG_PORTING_STDLIB == 1
        #define ARM_ACCTON_AS4610_MEMCPY memcpy
    #else
        #error The macro ARM_ACCTON_AS4610_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef ARM_ACCTON_AS4610_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define ARM_ACCTON_AS4610_VSNPRINTF GLOBAL_VSNPRINTF
    #elif ARM_ACCTON_AS4610_CONFIG_PORTING_STDLIB == 1
        #define ARM_ACCTON_AS4610_VSNPRINTF vsnprintf
    #else
        #error The macro ARM_ACCTON_AS4610_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ARM_ACCTON_AS4610_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define ARM_ACCTON_AS4610_SNPRINTF GLOBAL_SNPRINTF
    #elif ARM_ACCTON_AS4610_CONFIG_PORTING_STDLIB == 1
        #define ARM_ACCTON_AS4610_SNPRINTF snprintf
    #else
        #error The macro ARM_ACCTON_AS4610_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ARM_ACCTON_AS4610_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define ARM_ACCTON_AS4610_STRLEN GLOBAL_STRLEN
    #elif ARM_ACCTON_AS4610_CONFIG_PORTING_STDLIB == 1
        #define ARM_ACCTON_AS4610_STRLEN strlen
    #else
        #error The macro ARM_ACCTON_AS4610_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __ARM_ACCTON_AS4610_PORTING_H__ */
/* @} */
