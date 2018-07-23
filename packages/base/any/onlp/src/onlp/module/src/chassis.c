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
#include <onlp/chassis.h>
#include <onlp/platformi/chassisi.h>
#include <AIM/aim.h>
#include "onlp_log.h"
#include "onlp_int.h"
#include "onlp_locks.h"


static int
onlp_chassis_sw_init_locked__(void)
{
    return onlp_chassisi_sw_init();
}
ONLP_LOCKED_API0(onlp_chassis_sw_init);

static int
onlp_chassis_hw_init_locked__(uint32_t flags)
{
    return onlp_chassisi_hw_init(flags);
}
ONLP_LOCKED_API1(onlp_chassis_hw_init, uint32_t, flags);

static int
onlp_chassis_sw_denit_locked__(void)
{
    return onlp_chassisi_sw_denit();
}
ONLP_LOCKED_API0(onlp_chassis_sw_denit);

static int
onlp_chassis_hdr_get_locked__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    ONLP_OID_CHASSIS_VALIDATE(oid);
    ONLP_PTR_VALIDATE_ZERO(hdr);
    memset(hdr, 0, sizeof(*hdr));
    int rv = onlp_chassisi_hdr_get(oid, hdr);
    onlp_oid_hdr_sort(hdr);
    return rv;
}
ONLP_LOCKED_API2(onlp_chassis_hdr_get, onlp_oid_t, oid, onlp_oid_hdr_t*, hdr);


static int
onlp_chassis_info_get_locked__(onlp_oid_t oid, onlp_chassis_info_t* cip)
{
    ONLP_OID_CHASSIS_VALIDATE(oid);
    ONLP_PTR_VALIDATE_ZERO(cip);
    int rv = onlp_chassisi_info_get(oid, cip);
    onlp_oid_hdr_sort(&cip->hdr);
    return rv;
}
ONLP_LOCKED_API2(onlp_chassis_info_get,onlp_oid_t, oid,
                 onlp_chassis_info_t*, rv);

int
onlp_chassis_format(onlp_oid_t oid, onlp_oid_format_t format,
                    aim_pvs_t* pvs, uint32_t flags)
{
    return 0;
}

int
onlp_chassis_info_format(onlp_chassis_info_t* info, onlp_oid_format_t format,
                         aim_pvs_t* pvs, uint32_t flags)
{
    return 0;
}

int
onlp_chassis_info_to_user_json(onlp_chassis_info_t* info, cJSON** cjp, uint32_t flags)
{
    int rv;
    cJSON* object = NULL;
    if(ONLP_SUCCESS(rv = onlp_info_to_user_json_create(&info->hdr, &object, flags))) {
        rv = onlp_info_to_user_json_finish(&info->hdr, object, cjp, flags);
    }
    return rv;
}

int
onlp_chassis_info_to_json(onlp_chassis_info_t* info, cJSON** cjp, uint32_t flags)
{
    cJSON* cj = NULL;
    ONLP_IF_ERROR_RETURN(onlp_info_to_json_create(&info->hdr, &cj, flags));
    return onlp_info_to_json_finish(&info->hdr, cj, cjp, flags);
}

int
onlp_chassis_info_from_json(cJSON* cj, onlp_chassis_info_t* info)
{
    memset(info, 0, sizeof(*info));
    return onlp_oid_hdr_from_json(cj, &info->hdr);
}
