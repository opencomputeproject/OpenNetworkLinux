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
#include <onlp/onlp.h>
#include "onlp_log.h"
#include <onlplib/shlocks.h>
#include <onlp/oids.h>

static int
onlp_aim_ts__onlp_oid(aim_datatype_context_t* dtc, aim_va_list_t* vargs,
                      const char** rv)
{
    char str[64];
    onlp_oid_t oid = va_arg(vargs->val, onlp_oid_t);
    if(ONLP_SUCCESS(onlp_oid_to_str(oid, str))) {
        *rv = aim_strdup(str);
        return AIM_DATATYPE_OK;
    }
    *rv = NULL;
    return AIM_DATATYPE_ERROR;
}

static int
onlp_aim_fs__onlp_oid(aim_datatype_context_t* dtc,
                      const char* arg, aim_va_list_t* vargs)
{
    onlp_oid_t* oidp = va_arg(vargs->val, onlp_oid_t*);
    AIM_REFERENCE(dtc);

    if(ONLP_SUCCESS(onlp_oid_from_str((char*)arg, oidp))) {
        return AIM_DATATYPE_OK;
    }

    return AIM_DATATYPE_ERROR;
}

static int
onlp_aim_ts__onlp_oid_hdr(aim_datatype_context_t* dtc, aim_va_list_t* vargs,
                          const char** rv)
{
    onlp_oid_hdr_t* hdr = va_arg(vargs->val, onlp_oid_hdr_t*);
    int id = ONLP_OID_ID_GET(hdr->id);
    switch(ONLP_OID_TYPE_GET(hdr->id))
        {
#define         ONLP_OID_TYPE_ENTRY(_name, _value, _upper, _lower)      \
            case ONLP_OID_TYPE_##_name:                                 \
                *rv = aim_dfstrdup(#_lower" %d %s status=%{onlp_oid_status_flags}", \
                                   id, hdr->description, hdr->status);               \
                break;
#include <onlp/onlp.x>
        }
    return AIM_DATATYPE_OK;
}

static int
datatypes_init__(void)
{
#define ONLP_ENUMERATION_ENTRY(_enum_name, _desc)     AIM_DATATYPE_MAP_REGISTER(_enum_name, _enum_name##_map, _desc,                               AIM_LOG_INTERNAL);
#include <onlp/onlp.x>
    aim_datatype_register(0, "onlp_oid",
                          "ONLP OID",
                          onlp_aim_fs__onlp_oid,
                          onlp_aim_ts__onlp_oid, NULL);
    aim_datatype_register(0, "onlp_oid_hdr",
                          "ONLP OID Header",
                          NULL,
                          onlp_aim_ts__onlp_oid_hdr, NULL);


    /*
     * Register our flag maps.
     */
    AIM_DATATYPE_FMAP_REGISTER(onlp_oid_status_flags, onlp_oid_status_flag_map, "OID Status Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_sfp_control_flags, onlp_sfp_control_flag_map, "SFP Control Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_fan_caps_flags, onlp_fan_caps_map, "FAN Capability Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_thermal_caps_flags, onlp_thermal_caps_map, "Thermal Capability Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_led_caps_flags, onlp_led_caps_map, "LED Capability Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_psu_caps_flags, onlp_psu_caps_map, "PSU Capability Flags", AIM_LOG_INTERNAL);
    AIM_DATATYPE_FMAP_REGISTER(onlp_oid_type_flags, onlp_oid_type_flag_map, "ONLP OID Type Flags", AIM_LOG_INTERNAL);

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
