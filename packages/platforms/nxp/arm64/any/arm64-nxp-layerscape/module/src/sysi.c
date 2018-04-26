/************************************************************
 * <bsn.cl fy=2016 v=onl>
 *
 *	  Copyright 2016 NXP Semiconductor, Inc.
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
#include <onlp/platformi/sysi.h>
#include <onlplib/crc32.h>
#include "arm64_nxp_layerscape_log.h"
#include "arm64_nxp_layerscape_int.h"


platform_id_t platform_id = PLATFORM_ID_INVALID;

const char*
onlp_sysi_platform_get(void)
{
    return "arm64-nxp-layerscape";
}

int
onlp_sysi_platform_set(const char* platform)
{
	if(platform_id_value(platform, &platform_id, 0) == 0) {
		/* Platform supported */
		return ONLP_STATUS_OK;
	}
	AIM_LOG_ERROR("No support for platform '%s'", platform);
	return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    //TODO
    return ONLP_STATUS_OK;
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{
    /*
     * We returned a static array in onlp_sysi_onie_data_get()
     * so no free operation is required.
     */
}

void
onlp_sysi_platform_manage(void)
{
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    memset(table, 0, max*sizeof(onlp_oid_t));
    return 0;
}
