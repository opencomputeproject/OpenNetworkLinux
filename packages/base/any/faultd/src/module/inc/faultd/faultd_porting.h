/**************************************************************************//**
 * 
 * <bsn.cl fy=2013 v=onl>
 * 
 *        Copyright 2013, 2014 BigSwitch Networks, Inc.        
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
 *
 * @file
 * @brief faultd Porting Macros.
 * 
 * @addtogroup faultd_porting
 * @{
 * 
 *****************************************************************************/
#ifndef __FAULTD_PORTING_H__
#define __FAULTD_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if FAULTD_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef FAULTD_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define FAULTD_MEMSET GLOBAL_MEMSET
    #elif FAULTD_CONFIG_PORTING_STDLIB == 1
        #define FAULTD_MEMSET memset
    #else
        #error The macro FAULTD_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef FAULTD_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define FAULTD_MEMCPY GLOBAL_MEMCPY
    #elif FAULTD_CONFIG_PORTING_STDLIB == 1
        #define FAULTD_MEMCPY memcpy
    #else
        #error The macro FAULTD_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef FAULTD_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define FAULTD_STRNCPY GLOBAL_STRNCPY
    #elif FAULTD_CONFIG_PORTING_STDLIB == 1
        #define FAULTD_STRNCPY strncpy
    #else
        #error The macro FAULTD_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef FAULTD_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define FAULTD_STRLEN GLOBAL_STRLEN
    #elif FAULTD_CONFIG_PORTING_STDLIB == 1
        #define FAULTD_STRLEN strlen
    #else
        #error The macro FAULTD_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __FAULTD_PORTING_H__ */
/* @} */
