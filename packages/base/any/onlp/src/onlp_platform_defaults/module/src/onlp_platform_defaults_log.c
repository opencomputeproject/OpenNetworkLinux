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
#include "onlp_platform_defaults_log.h"

/*
 * onlp_platform_defaults log struct.
 */
AIM_LOG_STRUCT_DEFINE(
                      ONLP_PLATFORM_DEFAULTS_CONFIG_LOG_OPTIONS_DEFAULT,
                      ONLP_PLATFORM_DEFAULTS_CONFIG_LOG_BITS_DEFAULT,
                      onlp_platform_defaults_log_flag_map,
                      ONLP_PLATFORM_DEFAULTS_CONFIG_LOG_CUSTOM_BITS_DEFAULT
                     );

