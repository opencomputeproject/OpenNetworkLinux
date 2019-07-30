/************************************************************
 * <bsn.cl fy=2016 v=onl>
 * 
 *        Copyright 2016 NXP Semiconductor, Inc.
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

#include <arm64_nxp_layerscape/arm64_nxp_layerscape_config.h>
#include "arm64_nxp_layerscape_int.h"

/* <--auto.start.enum(ALL).source> */
/* <auto.end.enum(ALL).source> */

aim_map_si_t platform_id_map[] =
{
	{ "arm64-nxp-ls2088ardb-r0", PLATFORM_ID_ARM64_NXP_LS2088ARDB_R0 },
	{ "arm64-nxp-ls1043ardb-r0", PLATFORM_ID_ARM64_NXP_LS1043ARDB_R0 },
	{ "arm64-nxp-ls1046ardb-r0", PLATFORM_ID_ARM64_NXP_LS1046ARDB_R0 },
	{ "arm64-nxp-ls1088ardb-r0", PLATFORM_ID_ARM64_NXP_LS1088ARDB_R0 },
	{ NULL, 0 }
};

int
platform_id_value(const char* str, platform_id_t* e, int substr)
{
	int i;
	AIM_REFERENCE(substr);
	if(aim_map_si_s(&i, str, platform_id_map, 0)) {
		/* Enum Found */
		*e = i;
		return 0;
	}
	else{
		return -1;
	}

}
