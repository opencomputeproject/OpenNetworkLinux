/************************************************************
 * <bsn.cl fy=2017 v=onl>
 *
 *        Copyright 2017 Big Switch Networks, Inc.
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
 * Attribute Platform Implementation Defaults.
 *
 ***********************************************************/
#include <onlp/platformi/attributei.h>
#include "onlp_platform_defaults_int.h"
#include "onlp_platform_defaults_log.h"

__ONLP_DEFAULTI_IMPLEMENTATION(onlp_attributei_sw_init(void));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_attributei_hw_init(uint32_t flags));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_attributei_sw_denit(void));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_attributei_supported(onlp_oid_t id, const char* attribute));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_attributei_set(onlp_oid_t id, const char* attribute, void* value));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_attributei_get(onlp_oid_t id, const char* attribute, void** value));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_attributei_free(onlp_oid_t id, const char* attribute, void* value));
