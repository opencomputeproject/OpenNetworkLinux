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
 * Fan Platform Implementation Defaults.
 *
 ***********************************************************/
#include <onlp/platformi/fani.h>
#include "vendor_driver_pool.h"
#include "vendor_i2c_device_list.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum onlp_fan_id
{
    FAN_RESERVED = 0,
    FAN1_IN_TRAY_1, // FAN1_IN_TRAY_1,
    FAN2_IN_TRAY_2, // FAN1_IN_TRAY_2,
    FAN3_IN_TRAY_1, // FAN2_IN_TRAY_1,
    FAN4_IN_TRAY_2, // FAN2_IN_TRAY_2,
    FAN5_IN_TRAY_1, // FAN3_IN_TRAY_1,
    FAN6_IN_TRAY_2, // FAN3_IN_TRAY_2,
    FAN7_IN_TRAY_1, // FAN4_IN_TRAY_1,
    FAN8_IN_TRAY_2, // FAN4_IN_TRAY_2,
    FAN9_IN_TRAY_1, // FAN5_IN_TRAY_1,
    FAN10_IN_TRAY_2, // FAN5_IN_TRAY_2,
    FAN11_IN_TRAY_1, // FAN6_IN_TRAY_1,
    FAN12_IN_TRAY_2, // FAN6_IN_TRAY_2,
    FAN13_PSU_1,
    FAN14_PSU_2
};


#define FAN_INFO_ENTRY_INIT(_id, _desc, _caps)    \
    {                                      \
        {                                  \
            .id = ONLP_FAN_ID_CREATE(_id), \
            .description = _desc,          \
            .poid = 0,                     \
        },                                 \
        .status = ONLP_FAN_STATUS_PRESENT, \
        .caps = _caps,                     \
    }

onlp_fan_info_t onlp_fan_info[] = {
    {}, /* Not used */
    FAN_INFO_ENTRY_INIT(FAN1_IN_TRAY_1, "Chassis Fan 1-1",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN2_IN_TRAY_2, "Chassis Fan 1-2",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN3_IN_TRAY_1, "Chassis Fan 2-1",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN4_IN_TRAY_2, "Chassis Fan 2-2",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN5_IN_TRAY_1, "Chassis Fan 3-1",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN6_IN_TRAY_2, "Chassis Fan 3-2",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN7_IN_TRAY_1, "Chassis Fan 4-1",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN8_IN_TRAY_2, "Chassis Fan 4-2",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN9_IN_TRAY_1, "Chassis Fan 5-1",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN10_IN_TRAY_2, "Chassis Fan 5-2",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN11_IN_TRAY_1, "Chassis Fan 6-1",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN12_IN_TRAY_2, "Chassis Fan 6-2",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN13_PSU_1, "PSU Fan 1",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
    FAN_INFO_ENTRY_INIT(FAN14_PSU_2, "PSU Fan 2",
                        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM)),
};

int onlp_fani_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    *hdr = onlp_fan_info[ONLP_OID_ID_GET(id)].hdr;
    return ONLP_STATUS_OK;
}

/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    int rv = 0, nid = ONLP_OID_ID_GET(id) - 1, fail = 0;
    int present = 1;

    *info = onlp_fan_info[ONLP_OID_ID_GET(id)];

    void *busDrv = (void *)vendor_find_driver_by_name(fan_dev_list[nid].bus_drv_name);
    fan_dev_driver_t *fan =
        (fan_dev_driver_t *)vendor_find_driver_by_name(fan_dev_list[nid].dev_drv_name);

    rv = vendor_get_present_status(&fan_present_list[nid], &present);
    if(rv < 0) return ONLP_STATUS_E_INVALID;

    if (present)
    {
        info->status = ONLP_FAN_STATUS_PRESENT;
    }
    else
    {
        info->status = ONLP_FAN_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }

    vendor_dev_do_oc(fan_o_list[nid]);
    if(fan->rpm_get(
        busDrv,
        fan_dev_list[nid].bus,
        fan_dev_list[nid].addr,
        fan_dev_list[nid].id,
        &info->rpm) != ONLP_STATUS_OK)
    {
        info->rpm = 0;
        fail = 1;
    }
    vendor_dev_do_oc(fan_c_list[nid]);

    if(fail == 1) return ONLP_STATUS_E_INVALID;

    return ONLP_STATUS_OK;
}

/*
 * This function sets the speed of the given fan in RPM.
 *
 * This function will only be called if the fan supprots the RPM_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_rpm_set(onlp_oid_t id, int rpm)
{
    int nid = ONLP_OID_ID_GET(id) - 1, fail = 0;

    void *busDrv = (void *)vendor_find_driver_by_name(fan_dev_list[nid].bus_drv_name);
    fan_dev_driver_t *fan =
        (fan_dev_driver_t *)vendor_find_driver_by_name(fan_dev_list[nid].dev_drv_name);

    vendor_dev_do_oc(fan_o_list[nid]);
    if(fan->rpm_set(
        busDrv,
        fan_dev_list[nid].bus,
        fan_dev_list[nid].addr,
        fan_dev_list[nid].id,
        rpm) != ONLP_STATUS_OK)
    {
        fail = 1;
    }
    vendor_dev_do_oc(fan_c_list[nid]);

    if(fail == 1) return ONLP_STATUS_E_INVALID;

    return ONLP_STATUS_OK;
}

/*
 * This function sets the fan speed of the given OID as a percentage.
 *
 * This will only be called if the OID has the PERCENTAGE_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_percentage_set(onlp_oid_t id, int p)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function sets the fan speed of the given OID as per
 * the predefined ONLP fan speed modes: off, slow, normal, fast, max.
 *
 * Interpretation of these modes is up to the platform.
 *
 */
int
onlp_fani_mode_set(onlp_oid_t id, onlp_fan_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function sets the fan direction of the given OID.
 *
 * This function is only relevant if the fan OID supports both direction
 * capabilities.
 *
 * This function is optional unless the functionality is available.
 */
int
onlp_fani_dir_set(onlp_oid_t id, onlp_fan_dir_t dir)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * Generic fan ioctl. Optional.
 */
int
onlp_fani_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

