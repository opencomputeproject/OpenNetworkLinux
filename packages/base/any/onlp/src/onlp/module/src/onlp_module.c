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

#include <onlp/onlp_config.h>

#include "onlp_log.h"
#include <onlplib/shlocks.h>

static int
datatypes_init__(void)
{
#define ONLP_ENUMERATION_ENTRY(_enum_name, _desc)     AIM_DATATYPE_MAP_REGISTER(_enum_name, _enum_name##_map, _desc,                               AIM_LOG_INTERNAL);
#include <onlp/onlp.x>


    /*
     * Register our flag maps.
     */
    AIM_DATATYPE_FMAP_REGISTER(onlp_sfp_control_flags, onlp_sfp_control_flag_map, "SFP Control Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_fan_caps_flags, onlp_fan_caps_map, "FAN Capability Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_fan_status_flags, onlp_fan_status_map, "FAN Status Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_thermal_status_flags, onlp_thermal_status_map, "Thermal Status Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_thermal_caps_flags, onlp_thermal_caps_map, "Thermal Capability Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_led_caps_flags, onlp_led_caps_map, "LED Capability Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_led_status_flags, onlp_led_status_map, "LED Status Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_psu_status_flags, onlp_psu_status_map, "PSU Status Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_psu_caps_flags, onlp_psu_caps_map, "PSU Capability Flags", AIM_LOG_INTERNAL);


    return 0;
}

void __onlp_module_init__(void)
{
    AIM_LOG_STRUCT_REGISTER();
    datatypes_init__();

#if ONLP_CONFIG_INCLUDE_SHLOCK_GLOBAL_INIT == 1
    onlp_shlock_global_init();
#endif

    {
        extern int __onlp_platform_version__;
        extern int __onlp_platform_version_default__;
        __onlp_platform_version_default__ = __onlp_platform_version__;
    }
}

