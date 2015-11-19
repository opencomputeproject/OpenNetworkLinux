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
 * Fan Platform Implementation Defaults.
 *
 ***********************************************************/
#include <onlp/platformi/fani.h>
#include "onlpie_int.h"

/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}

/*
 * Get the fan information.
 */

/* Static fan information */
onlp_fan_info_t finfo[] = {
    { }, /* Not used */
    {
        { ONLP_FAN_ID_CREATE(1), "Chassis Fan 1", 0 },
        0x1,
        ONLP_FAN_CAPS_B2F,
        9000,
        100,
        ONLP_FAN_MODE_MAX,
        "FAN1Model",
        "FAN1SerialNumber",
    },
    {
        { ONLP_FAN_ID_CREATE(2), "Chassis Fan 2", 0 },
        0x0,
    },
    {
        { ONLP_FAN_ID_CREATE(3), "PSU-1 Fan 1", 0 },
        0x1,
        ONLP_FAN_CAPS_B2F,
        5004,
        50,
        ONLP_FAN_MODE_NORMAL,
    },
    {
        { ONLP_FAN_ID_CREATE(3), "PSU-1 Fan 2", 0 },
        0x1,
        ONLP_FAN_CAPS_B2F,
        5020,
        50,
        ONLP_FAN_MODE_NORMAL,
    },
};

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    *info = finfo[ONLP_OID_ID_GET(id)];
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
    return ONLP_STATUS_E_UNSUPPORTED;
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


