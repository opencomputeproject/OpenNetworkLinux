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
 * @brief arm_qemu_armv7a Porting Macros.
 *
 * @addtogroup arm_qemu_armv7a-porting
 * @{
 *
 ***********************************************************/
#ifndef __ONLPSIM_PORTING_H__
#define __ONLPSIM_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if ONLPSIM_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef ONLPSIM_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define ONLPSIM_MALLOC GLOBAL_MALLOC
    #elif ONLPSIM_CONFIG_PORTING_STDLIB == 1
        #define ONLPSIM_MALLOC malloc
    #else
        #error The macro ONLPSIM_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef ONLPSIM_FREE
    #if defined(GLOBAL_FREE)
        #define ONLPSIM_FREE GLOBAL_FREE
    #elif ONLPSIM_CONFIG_PORTING_STDLIB == 1
        #define ONLPSIM_FREE free
    #else
        #error The macro ONLPSIM_FREE is required but cannot be defined.
    #endif
#endif

#ifndef ONLPSIM_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define ONLPSIM_MEMSET GLOBAL_MEMSET
    #elif ONLPSIM_CONFIG_PORTING_STDLIB == 1
        #define ONLPSIM_MEMSET memset
    #else
        #error The macro ONLPSIM_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef ONLPSIM_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define ONLPSIM_MEMCPY GLOBAL_MEMCPY
    #elif ONLPSIM_CONFIG_PORTING_STDLIB == 1
        #define ONLPSIM_MEMCPY memcpy
    #else
        #error The macro ONLPSIM_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef ONLPSIM_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define ONLPSIM_VSNPRINTF GLOBAL_VSNPRINTF
    #elif ONLPSIM_CONFIG_PORTING_STDLIB == 1
        #define ONLPSIM_VSNPRINTF vsnprintf
    #else
        #error The macro ONLPSIM_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ONLPSIM_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define ONLPSIM_SNPRINTF GLOBAL_SNPRINTF
    #elif ONLPSIM_CONFIG_PORTING_STDLIB == 1
        #define ONLPSIM_SNPRINTF snprintf
    #else
        #error The macro ONLPSIM_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ONLPSIM_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define ONLPSIM_STRLEN GLOBAL_STRLEN
    #elif ONLPSIM_CONFIG_PORTING_STDLIB == 1
        #define ONLPSIM_STRLEN strlen
    #else
        #error The macro ONLPSIM_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __ONLPSIM_PORTING_H__ */
/* @} */
