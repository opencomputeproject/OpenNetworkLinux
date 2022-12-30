/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2017 Accton Technology Corporation.
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
 ********************************************************//**
 *
 *
 *
 ***********************************************************/
#include <onlp/platformi/chassisi.h>
#include "platform_lib.h"

int
onlp_chassisi_sw_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_chassisi_sw_denit(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_chassisi_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    int i;
    onlp_oid_t* e = hdr->coids;

    ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
    ONLP_OID_STATUS_FLAG_SET(hdr, OPERATIONAL);

    /* 11 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 4 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 3 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 12 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 55 Ports */
    for (i = 1; i <= CHASSIS_SFP_COUNT; i++) {
        *e++ = ONLP_SFP_ID_CREATE(i);
    }

    return ONLP_STATUS_OK;
}

int onlp_chassisi_info_get(onlp_oid_id_t id, onlp_chassis_info_t* info)
{
    onlp_chassisi_hdr_get(id, &info->hdr);
    return ONLP_STATUS_OK;
}
