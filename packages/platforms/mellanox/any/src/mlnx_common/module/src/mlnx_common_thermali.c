/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
 * Thermal Platform Implementation Defaults.
 *
 ***********************************************************/
#include <fcntl.h>
#include <unistd.h>
#include <AIM/aim_log.h>
#include <onlplib/file.h>
#include <onlplib/mmap.h>
#include <onlp/platformi/thermali.h>
#include "mlnx_common/mlnx_common.h"

#define prefix_path "/bsp/thermal"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/*
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */
int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    int   rv, temp_base=1, local_id = 0;
    int   r_val;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    mlnx_platform_info_t* mlnx_platform_info = get_platform_info();

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = mlnx_platform_info->tinfo[local_id];

    rv = onlp_file_read_int(&r_val, "%s/%s", prefix_path, mlnx_platform_info->thermal_fnames[local_id]);
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    info->mcelsius = r_val / temp_base;

    return ONLP_STATUS_OK;
}
