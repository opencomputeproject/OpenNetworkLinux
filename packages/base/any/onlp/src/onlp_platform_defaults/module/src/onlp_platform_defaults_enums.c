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

#include <onlp_platform_defaults/onlp_platform_defaults_config.h>
#include "onlp_platform_defaults_int.h"

/* <auto.start.enum(ALL).source> */
aim_map_si_t onlp_platform_defaults_log_flag_map[] =
{
    { "called", ONLP_PLATFORM_DEFAULTS_LOG_FLAG_CALLED },
    { NULL, 0 }
};

aim_map_si_t onlp_platform_defaults_log_flag_desc_map[] =
{
    { "None", ONLP_PLATFORM_DEFAULTS_LOG_FLAG_CALLED },
    { NULL, 0 }
};

const char*
onlp_platform_defaults_log_flag_name(onlp_platform_defaults_log_flag_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_platform_defaults_log_flag_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_platform_defaults_log_flag'";
    }
}

int
onlp_platform_defaults_log_flag_value(const char* str, onlp_platform_defaults_log_flag_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_platform_defaults_log_flag_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_platform_defaults_log_flag_desc(onlp_platform_defaults_log_flag_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_platform_defaults_log_flag_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_platform_defaults_log_flag'";
    }
}

/* <auto.end.enum(ALL).source> */

