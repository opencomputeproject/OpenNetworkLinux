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
 * Power Supply Management Implementation.
 *
 ***********************************************************/
#include <onlp/platformi/modulei.h>
#include "onlp_platform_defaults_int.h"
#include "onlp_platform_defaults_log.h"

__ONLP_DEFAULTI_IMPLEMENTATION(onlp_modulei_init(void));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_modulei_info_get(onlp_oid_t id, onlp_module_info_t* rv));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_modulei_status_get(onlp_oid_t id, uint32_t* rv));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_modulei_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv));
