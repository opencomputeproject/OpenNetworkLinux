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
 ***********************************************************/
#include <onlp/platformi/modulei.h>
#include "onlp_platform_defaults_int.h"
#include "onlp_platform_defaults_log.h"

__ONLP_DEFAULTI_IMPLEMENTATION_OPTIONAL(onlp_modulei_sw_init(void));
__ONLP_DEFAULTI_IMPLEMENTATION_OPTIONAL(onlp_modulei_sw_denit(void));
__ONLP_DEFAULTI_IMPLEMENTATION_OPTIONAL(onlp_modulei_hw_init(uint32_t flags));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_modulei_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr));

/*
 * There are no fields defined in the module info structure. As a result
 * we provide a default implementation which populates the OID header.
 */
int __ONLP_DEFAULTI
onlp_modulei_info_get(onlp_oid_id_t id, onlp_module_info_t* info)
{
    return onlp_modulei_hdr_get(id, &info->hdr);
};
