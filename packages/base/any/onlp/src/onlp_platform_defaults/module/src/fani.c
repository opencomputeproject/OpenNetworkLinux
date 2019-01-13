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
 * Fan Platform Implementation Defaults.
 *
 ***********************************************************/
#include <onlp/platformi/fani.h>
#include "onlp_platform_defaults_int.h"
#include "onlp_platform_defaults_log.h"

/**
 * These are the default implementations for all currently
 * defined interface functions.
 */
__ONLP_DEFAULTI_IMPLEMENTATION_OPTIONAL(onlp_fani_sw_init(void));
__ONLP_DEFAULTI_IMPLEMENTATION_OPTIONAL(onlp_fani_hw_init(uint32_t flags));
__ONLP_DEFAULTI_IMPLEMENTATION_OPTIONAL(onlp_fani_sw_denit(void));
__ONLP_DEFAULTI_IMPLEMENTATION_OPTIONAL(onlp_fani_id_validate(onlp_oid_id_t id));

__ONLP_DEFAULTI_IMPLEMENTATION(onlp_fani_info_get(onlp_oid_id_t id, onlp_fan_info_t* info));

/**
 * Simulate hdr_get for older platforms which don't support it.
 * This is inefficient.
 */
int __ONLP_DEFAULTI
onlp_fani_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    onlp_fan_info_t info;
    ONLP_TRY(onlp_fani_info_get(id, &info));
    *hdr = info.hdr;
    return ONLP_STATUS_OK;
}

/**
 * Simulate caps_get for older platforms which don't support it.
 * This is inefficient.
 */
int __ONLP_DEFAULTI
onlp_fani_caps_get(onlp_oid_id_t id, uint32_t* rv)
{
    onlp_fan_info_t info;
    ONLP_TRY(onlp_fani_info_get(id, &info));
    *rv = info.caps;
    return ONLP_STATUS_OK;
}

__ONLP_DEFAULTI_IMPLEMENTATION(onlp_fani_rpm_set(onlp_oid_id_t id, int rpm));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_fani_percentage_set(onlp_oid_id_t id, int p));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_fani_dir_set(onlp_oid_id_t id, onlp_fan_dir_t dir));
