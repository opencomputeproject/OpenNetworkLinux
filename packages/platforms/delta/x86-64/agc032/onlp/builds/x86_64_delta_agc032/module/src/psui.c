/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2019 Delta Electronics, Inc.
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
#include <onlp/platformi/psui.h>
#include "vendor_driver_pool.h"
#include "vendor_i2c_device_list.h"


#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)


enum onlp_psu_id
{
    PSU_RESERVED = 0,
    PSU_1,
    PSU_2,
};

#define PSU_INFO_ENTRY_INIT(_id, _desc)                                             \
    {                                                                               \
        {                                                                           \
            .id = ONLP_PSU_ID_CREATE(_id),                                          \
            .description = _desc,                                                   \
            .poid = 0,                                                              \
        },                                                                          \
            .caps = (ONLP_PSU_CAPS_VOUT | ONLP_PSU_CAPS_IOUT | ONLP_PSU_CAPS_POUT), \
    }

static onlp_psu_info_t onlp_psu_info[] = {
    {}, /* Not used */
    PSU_INFO_ENTRY_INIT(PSU_1, "PSU 1"),
    PSU_INFO_ENTRY_INIT(PSU_2, "PSU 2"),
};

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_psui_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    //AIM_LOG_ERROR("Function: %s, instance: %d \n", __FUNCTION__, ONLP_OID_ID_GET(id) - 1);

    *hdr = onlp_psu_info[ONLP_OID_ID_GET(id)].hdr;
    return ONLP_STATUS_OK;
}

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    //AIM_LOG_ERROR("Function: %s, instance: %d \n", __FUNCTION__, ONLP_OID_ID_GET(id) - 1);
    int rv = 0;
    int nid = ONLP_OID_ID_GET(id) - 1, fail = 0;
    int present = 0;
    int mvout = 0, miout = 0, mpout = 0;

    *info = onlp_psu_info[ONLP_OID_ID_GET(id)];

    void *busDrv = (void *)vendor_find_driver_by_name(psu_dev_list[nid].bus_drv_name);
    psu_dev_driver_t *psu =
        (psu_dev_driver_t *)vendor_find_driver_by_name(psu_dev_list[nid].dev_drv_name);

    rv = vendor_get_present_status(&psu_present_list[nid], &present);
    if(rv < 0) return ONLP_STATUS_E_INVALID;

    if (present)
    {
        info->status = ONLP_PSU_STATUS_PRESENT;
    }
    else
    {
        info->status = ONLP_PSU_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }

    vendor_dev_do_oc(psu_o_list[nid]);
    if(psu->model_get(
        busDrv,
        psu_dev_list[nid].bus,
        psu_dev_list[nid].addr,
        (char *) &info->model) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("psu->model_get failed.");
        fail = 1;
    }

    if(psu->serial_get(
        busDrv,
        psu_dev_list[nid].bus,
        psu_dev_list[nid].addr,
        (char *) &info->serial) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("psu->serial_get failed.");
        fail = 1;
    }

    if(psu->volt_get(
        busDrv,
        psu_dev_list[nid].bus,
        psu_dev_list[nid].addr,
        &mvout) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("psu->volt_get failed.");
        fail = 1;
    }

    info->mvout = mvout;

    if(psu->amp_get(
        busDrv,
        psu_dev_list[nid].bus,
        psu_dev_list[nid].addr,
        &miout) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("psu->amp_get failed.");
        fail = 1;
    }

    info->miout = miout;

    if(psu->watt_get(
        busDrv,
        psu_dev_list[nid].bus,
        psu_dev_list[nid].addr,
        &mpout) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("psu->watt_get failed.");
        fail = 1;
    }

    info->mpout = mpout;
    vendor_dev_do_oc(psu_o_list[nid]);

    if(fail == 1) return ONLP_STATUS_E_INVALID;

    return ONLP_STATUS_OK;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
