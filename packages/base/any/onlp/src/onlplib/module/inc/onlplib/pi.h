/**************************************************************
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
 **************************************************************
 *
 * Platform Information
 *
 ************************************************************/
#ifndef __ONLPLIB_PI_H__
#define __ONLPLIB_PI_H__

#include <onlplib/onlplib_config.h>

/**
 * Platform Information
 */
typedef struct onlp_platform_info_s {

    /**
     * CPLD Versions
     *
     * Describes the internal CPLD version numbers, if applicable.
     *
     */
    char* cpld_versions;

    /**
     * Additional version or platform information.
     */
    char* other_versions;

} onlp_platform_info_t;

void onlp_platform_info_show_json(onlp_platform_info_t* pi, aim_pvs_t* pvs);

void onlp_platform_info_show(onlp_platform_info_t* pi, aim_pvs_t* pvs);

#endif /* __ONLPLIB_PI_H__ */
