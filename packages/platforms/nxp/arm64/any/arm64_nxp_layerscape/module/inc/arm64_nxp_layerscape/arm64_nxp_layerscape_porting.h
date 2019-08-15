/************************************************************
 * <bsn.cl fy=2016 v=onl>
 * 
 *        Copyright 2016 NXP Semiconductor, Inc.
 *
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
 * @brief arm64_nxp_layerscape Porting Macros.
 *
 * @addtogroup arm64_nxp_layerscape-porting
 * @{
 *
 ***********************************************************/
#ifndef __ARM64_NXP_LAYERSCAPE_PORTING_H__
#define __ARM64_NXP_LAYERSCAPE_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if ARM64_NXP_LAYERSCAPE_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef ARM64_NXP_LAYERSCAPE_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define ARM64_NXP_LAYERSCAPE_MALLOC GLOBAL_MALLOC
    #elif ARM64_NXP_LAYERSCAPE_CONFIG_PORTING_STDLIB == 1
        #define ARM64_NXP_LAYERSCAPE_MALLOC malloc
    #else
        #error The macro ARM64_NXP_LAYERSCAPE_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef ARM64_NXP_LAYERSCAPE_FREE
    #if defined(GLOBAL_FREE)
        #define ARM64_NXP_LAYERSCAPE_FREE GLOBAL_FREE
    #elif ARM64_NXP_LAYERSCAPE_CONFIG_PORTING_STDLIB == 1
        #define ARM64_NXP_LAYERSCAPE_FREE free
    #else
        #error The macro ARM64_NXP_LAYERSCAPE_FREE is required but cannot be defined.
    #endif
#endif

#ifndef ARM64_NXP_LAYERSCAPE_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define ARM64_NXP_LAYERSCAPE_MEMSET GLOBAL_MEMSET
    #elif ARM64_NXP_LAYERSCAPE_CONFIG_PORTING_STDLIB == 1
        #define ARM64_NXP_LAYERSCAPE_MEMSET memset
    #else
        #error The macro ARM64_NXP_LAYERSCAPE_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef ARM64_NXP_LAYERSCAPE_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define ARM64_NXP_LAYERSCAPE_MEMCPY GLOBAL_MEMCPY
    #elif ARM64_NXP_LAYERSCAPE_CONFIG_PORTING_STDLIB == 1
        #define ARM64_NXP_LAYERSCAPE_MEMCPY memcpy
    #else
        #error The macro ARM64_NXP_LAYERSCAPE_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef ARM64_NXP_LAYERSCAPE_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define ARM64_NXP_LAYERSCAPE_VSNPRINTF GLOBAL_VSNPRINTF
    #elif ARM64_NXP_LAYERSCAPE_CONFIG_PORTING_STDLIB == 1
        #define ARM64_NXP_LAYERSCAPE_VSNPRINTF vsnprintf
    #else
        #error The macro ARM64_NXP_LAYERSCAPE_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ARM64_NXP_LAYERSCAPE_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define ARM64_NXP_LAYERSCAPE_SNPRINTF GLOBAL_SNPRINTF
    #elif ARM64_NXP_LAYERSCAPE_CONFIG_PORTING_STDLIB == 1
        #define ARM64_NXP_LAYERSCAPE_SNPRINTF snprintf
    #else
        #error The macro ARM64_NXP_LAYERSCAPE_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ARM64_NXP_LAYERSCAPE_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define ARM64_NXP_LAYERSCAPE_STRLEN GLOBAL_STRLEN
    #elif ARM64_NXP_LAYERSCAPE_CONFIG_PORTING_STDLIB == 1
        #define ARM64_NXP_LAYERSCAPE_STRLEN strlen
    #else
        #error The macro ARM64_NXP_LAYERSCAPE_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __ARM64_NXP_LAYERSCAPE_PORTING_H__ */
/* @} */
