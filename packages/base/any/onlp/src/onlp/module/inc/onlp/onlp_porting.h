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
 * @brief onlp Porting Macros.
 *
 * @addtogroup onlp-porting
 * @{
 *
 *****************************************************************************/
#ifndef __ONLP_PORTING_H__
#define __ONLP_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if ONLP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef ONLP_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define ONLP_MEMSET GLOBAL_MEMSET
    #elif ONLP_CONFIG_PORTING_STDLIB == 1
        #define ONLP_MEMSET memset
    #else
        #error The macro ONLP_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef ONLP_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define ONLP_MEMCPY GLOBAL_MEMCPY
    #elif ONLP_CONFIG_PORTING_STDLIB == 1
        #define ONLP_MEMCPY memcpy
    #else
        #error The macro ONLP_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef ONLP_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define ONLP_STRNCPY GLOBAL_STRNCPY
    #elif ONLP_CONFIG_PORTING_STDLIB == 1
        #define ONLP_STRNCPY strncpy
    #else
        #error The macro ONLP_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef ONLP_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define ONLP_STRLEN GLOBAL_STRLEN
    #elif ONLP_CONFIG_PORTING_STDLIB == 1
        #define ONLP_STRLEN strlen
    #else
        #error The macro ONLP_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __ONLP_PORTING_H__ */
/* @} */
