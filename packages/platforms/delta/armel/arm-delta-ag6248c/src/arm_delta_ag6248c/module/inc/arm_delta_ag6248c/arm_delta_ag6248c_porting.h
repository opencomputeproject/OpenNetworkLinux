/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * 
 * Copyright 2018, Delta Networks, Inc.       
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
 * @brief arm_delta_ag6248c Porting Macros.
 *
 * @addtogroup arm_delta_ag6248c-porting
 * @{
 *
 ***********************************************************/
#ifndef __ARM_DELTA_AG6248C_PORTING_H__
#define __ARM_DELTA_AG6248C_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if ARM_DELTA_AG6248C_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef ARM_DELTA_AG6248C_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define ARM_DELTA_AG6248C_MALLOC GLOBAL_MALLOC
    #elif ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB == 1
        #define ARM_DELTA_AG6248C_MALLOC malloc
    #else
        #error The macro ARM_DELTA_AG6248C_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef ARM_DELTA_AG6248C_FREE
    #if defined(GLOBAL_FREE)
        #define ARM_DELTA_AG6248C_FREE GLOBAL_FREE
    #elif ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB == 1
        #define ARM_DELTA_AG6248C_FREE free
    #else
        #error The macro ARM_DELTA_AG6248C_FREE is required but cannot be defined.
    #endif
#endif

#ifndef ARM_DELTA_AG6248C_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define ARM_DELTA_AG6248C_MEMSET GLOBAL_MEMSET
    #elif ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB == 1
        #define ARM_DELTA_AG6248C_MEMSET memset
    #else
        #error The macro ARM_DELTA_AG6248C_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef ARM_DELTA_AG6248C_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define ARM_DELTA_AG6248C_MEMCPY GLOBAL_MEMCPY
    #elif ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB == 1
        #define ARM_DELTA_AG6248C_MEMCPY memcpy
    #else
        #error The macro ARM_DELTA_AG6248C_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef ARM_DELTA_AG6248C_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define ARM_DELTA_AG6248C_STRNCPY GLOBAL_STRNCPY
    #elif ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB == 1
        #define ARM_DELTA_AG6248C_STRNCPY strncpy
    #else
        #error The macro ARM_DELTA_AG6248C_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef ARM_DELTA_AG6248C_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define ARM_DELTA_AG6248C_VSNPRINTF GLOBAL_VSNPRINTF
    #elif ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB == 1
        #define ARM_DELTA_AG6248C_VSNPRINTF vsnprintf
    #else
        #error The macro ARM_DELTA_AG6248C_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ARM_DELTA_AG6248C_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define ARM_DELTA_AG6248C_SNPRINTF GLOBAL_SNPRINTF
    #elif ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB == 1
        #define ARM_DELTA_AG6248C_SNPRINTF snprintf
    #else
        #error The macro ARM_DELTA_AG6248C_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef ARM_DELTA_AG6248C_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define ARM_DELTA_AG6248C_STRLEN GLOBAL_STRLEN
    #elif ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB == 1
        #define ARM_DELTA_AG6248C_STRLEN strlen
    #else
        #error The macro ARM_DELTA_AG6248C_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __ARM_DELTA_AG6248C_PORTING_H__ */
/* @} */
