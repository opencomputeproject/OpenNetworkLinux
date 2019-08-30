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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <onlp/platformi/thermali.h>
#include "vendor_driver_pool.h"
#include "vendor_i2c_device_list.h"


#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_1_CPU_1,
    THERMAL_2_RF,
    THERMAL_3_LF,
    THERMAL_4_MAC_UP,
    THERMAL_5_MAC_LO,
    THERMAL_6_FAN
};


#define THERMAL_INFO_ENTRY_INIT(_id, _desc)                                                                                                   \
    {                                                                                                                                         \
        {                                                                                                                                     \
            .id = ONLP_THERMAL_ID_CREATE(_id),                                                                                                \
            .description = _desc,                                                                                                             \
            .poid = 0,                                                                                                                        \
        },                                                                                                                                    \
            .status = ONLP_THERMAL_STATUS_PRESENT,                                                                                            \
            .caps = (ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD | ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD), \
            .mcelsius = 0,                                                                                                                    \
            ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS                                                                                              \
    }

static onlp_thermal_info_t onlp_thermal_info[] = {
    {}, /* Not used */
    THERMAL_INFO_ENTRY_INIT(THERMAL_1_CPU_1, "Thermal Sensor (CPU)"),
    THERMAL_INFO_ENTRY_INIT(THERMAL_2_RF, "Thermal Sensor (RF)"),
    THERMAL_INFO_ENTRY_INIT(THERMAL_3_LF, "Thermal Sensor (LF)"),
    THERMAL_INFO_ENTRY_INIT(THERMAL_4_MAC_UP, "Thermal Sensor (MAC-Upper)"),
    THERMAL_INFO_ENTRY_INIT(THERMAL_5_MAC_LO, "Thermal Sensor (MAC-Lower)"),
    THERMAL_INFO_ENTRY_INIT(THERMAL_6_FAN, "Thermal Sensor (FANBD)"),
};

int
onlp_thermali_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    //AIM_LOG_ERROR("Function: %s, instance: %d \n", __FUNCTION__, ONLP_OID_ID_GET(id) - 1);

    *hdr = onlp_thermal_info[ONLP_OID_ID_GET(id)].hdr;
    return ONLP_STATUS_OK;
}

int
onlp_thermali_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* rv)
{
    //AIM_LOG_ERROR("Function: %s, instance: %d \n", __FUNCTION__, ONLP_OID_ID_GET(id) - 1);
    int nid = ONLP_OID_ID_GET(id) - 1, fail = 0;

    *rv = onlp_thermal_info[ONLP_OID_ID_GET(id)];

    void *busDrv = (void *)vendor_find_driver_by_name(thermal_dev_list[nid].bus_drv_name);
    thermal_dev_driver_t *thermal =
        (thermal_dev_driver_t *)vendor_find_driver_by_name(thermal_dev_list[nid].dev_drv_name);

    vendor_dev_do_oc(thermal_o_list[nid]);
    if(thermal->temp_get(
        busDrv,
        thermal_dev_list[nid].bus,
        thermal_dev_list[nid].addr,
        thermal_dev_list[nid].id,
        &rv->mcelsius) != ONLP_STATUS_OK)
    {
        rv->mcelsius = 0;
        fail = 1;
    }

    if(thermal->limit_get(
        busDrv,
        thermal_dev_list[nid].bus,
        thermal_dev_list[nid].addr,
        thermal_dev_list[nid].id,
        VENDOR_THERMAL_LOW_THRESHOLD,
        &rv->thresholds.warning) != ONLP_STATUS_OK)
    {
        rv->thresholds.warning = 0;
        fail = 1;
    }

    if(thermal->limit_get(
        busDrv,
        thermal_dev_list[nid].bus,
        thermal_dev_list[nid].addr,
        thermal_dev_list[nid].id,
        VENDOR_THERMAL_HIGH_THRESHOLD,
        &rv->thresholds.shutdown) != ONLP_STATUS_OK)
    {
        rv->thresholds.shutdown = 0;
        fail = 1;
    }
    vendor_dev_do_oc(thermal_c_list[nid]);

    if(fail == 1) return ONLP_STATUS_E_INVALID;

    return ONLP_STATUS_OK;
}

