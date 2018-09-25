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
 *
 *
 ***********************************************************/
#include <x86_64_netberg_aurora_420_rangeley/x86_64_netberg_aurora_420_rangeley_config.h>
#include <onlp/platformi/fani.h>

#include "x86_64_netberg_aurora_420_rangeley_int.h"
#include "x86_64_netberg_aurora_420_rangeley_log.h"

#include <onlplib/file.h>


#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int
sys_fan_info_get__(onlp_fan_info_t* info, int id)
{
    int value = 0;
    int rv;

    rv = onlp_file_read_int(&value, SYS_HWMON2_PREFIX "/fan%d_abs", ((id/2)+1));
    if (rv != ONLP_STATUS_OK)
        return rv;

    if (value == 0)
    {
        info->status = ONLP_FAN_STATUS_FAILED;
    }
    else
    {
        info->status = ONLP_FAN_STATUS_PRESENT;

        rv = onlp_file_read_int(&value, SYS_HWMON2_PREFIX "/fan%d_dir", ((id/2)+1));
        if (rv != ONLP_STATUS_OK)
            return rv;

        if (value == 1)
        {
            info->status |= ONLP_FAN_STATUS_B2F;
            info->caps |= ONLP_FAN_CAPS_B2F;
        }
        else
        {
            info->status |= ONLP_FAN_STATUS_F2B;
            info->caps |= ONLP_FAN_CAPS_F2B;
        }

        rv = onlp_file_read_int(&(info->rpm), SYS_HWMON1_PREFIX "/fan%d_rpm", (id+1));
        if (rv == ONLP_STATUS_E_INTERNAL)
            return rv;

        if (rv == ONLP_STATUS_E_MISSING)
        {
            info->status &= ~1;
            return 0;
        }

        if (info->rpm <= X86_64_NETBERG_AURORA_420_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD)
            info->status |= ONLP_FAN_STATUS_FAILED;


        rv = onlp_file_read_int(&(info->percentage), SYS_HWMON1_PREFIX "/fan%d_duty", (id+1));
        if (rv == ONLP_STATUS_E_INTERNAL)
            return rv;

        if (rv == ONLP_STATUS_E_MISSING)
        {
            info->status &= ~1;
            return 0;
        }
    }
    return 0;
}

static int
psu_fan_info_get__(onlp_fan_info_t* info, int id)
{
    return onlp_file_read_int(&(info->rpm), SYS_HWMON2_PREFIX "/psu%d_fan_speed", id);
}

/* Onboard Fans */
static onlp_fan_info_t fans__[] = {
    { }, /* Not used */
    { { FAN_OID_FAN1,  "Fan1_rotor1", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN2,  "Fan1_rotor2", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN3,  "Fan2_rotor1", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN4,  "Fan2_rotor2", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN5,  "Fan3_rotor1", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN6,  "Fan3_rotor2", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN7,  "Fan4_rotor1", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN8,  "Fan4_rotor2", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN9,  "PSU-1 Fan", 0 }, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN10, "PSU-2 Fan", 0 }, ONLP_FAN_STATUS_PRESENT },
};

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
    int fid;

    VALIDATE(id);

    memset(info, 0, sizeof(onlp_fan_info_t));
    fid = ONLP_OID_ID_GET(id);
    *info = fans__[fid];

    info->caps |= ONLP_FAN_CAPS_GET_RPM;

    switch(fid)
    {
        case FAN_ID_FAN1:
        case FAN_ID_FAN2:
        case FAN_ID_FAN3:
        case FAN_ID_FAN4:
        case FAN_ID_FAN5:
        case FAN_ID_FAN6:
        case FAN_ID_FAN7:
        case FAN_ID_FAN8:
          return sys_fan_info_get__(info, (fid - 1));
          break;

        case FAN_ID_FAN9:
        case FAN_ID_FAN10:
          return psu_fan_info_get__(info, (fid - FAN_ID_FAN9 + 1));
          break;

        default:
          return ONLP_STATUS_E_INVALID;
          break;
  }

  return ONLP_STATUS_E_INVALID;
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

