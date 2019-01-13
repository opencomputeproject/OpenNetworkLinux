/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <onlplib/file.h>
#include <fcntl.h>
#include <unistd.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)




typedef struct fani_info_s {
    int slow_pwm;
    int normal_pwm;
    int max_pwm;
    int step_size;
    char input_file[ONLP_CONFIG_INFO_STR_MAX];
    char pwm_file[ONLP_CONFIG_INFO_STR_MAX];
} fani_info_t;


static fani_info_t __info_list[ONLP_FAN_COUNT] = {
    {},
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/fan1_input",
        "/sys/class/hwmon/hwmon1/device/pwm1"
    },
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/fan3_input",
        "/sys/class/hwmon/hwmon1/device/pwm2"
    },
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/fan5_input",
        "/sys/class/hwmon/hwmon1/device/pwm3"
    },
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/fan7_input",
        "/sys/class/hwmon/hwmon1/device/pwm4"
    },
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/fan9_input",
        "/sys/class/hwmon/hwmon1/device/pwm5" /*sync with pwm1*/
    },
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/fan2_input",
        "/sys/class/hwmon/hwmon1/device/pwm1"
    },
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/fan4_input",
        "/sys/class/hwmon/hwmon1/device/pwm2"
    },
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/fan6_input",
        "/sys/class/hwmon/hwmon1/device/pwm3"
    },
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/fan8_input",
        "/sys/class/hwmon/hwmon1/device/pwm4"
    },
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/fan10_input",
        "/sys/class/hwmon/hwmon1/device/pwm5" /*sync with pwm1*/
    },
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/rpm_psu1",
        "/sys/class/hwmon/hwmon1/device/pwm_psu1"
    },
    {
        100,175,255,100,
        "/sys/class/hwmon/hwmon1/device/rpm_psu2",
        "/sys/class/hwmon/hwmon1/device/pwm_psu2"
    }
};

/* Static values */
static onlp_fan_info_t __onlp_fan_info[ONLP_FAN_COUNT] = {
    { }, /* Not used */
    {   {
            ONLP_FAN_ID_CREATE(ONLP_FAN_1), "Fan 1", 0,
            {
                ONLP_FAN_ID_CREATE(ONLP_FAN_1_WEAK),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN1_GREEN),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN1_RED)
            }
        },
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },
    {   {
            ONLP_FAN_ID_CREATE(ONLP_FAN_2), "Fan 2", 0,
            {
                ONLP_FAN_ID_CREATE(ONLP_FAN_2_WEAK),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN2_GREEN),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN2_RED)
            }
        },
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },
    {   {
            ONLP_FAN_ID_CREATE(ONLP_FAN_3), "Fan 3", 0,
            {
                ONLP_FAN_ID_CREATE(ONLP_FAN_3_WEAK),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN3_GREEN),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN3_RED)
            }
        },
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },
    {   {
            ONLP_FAN_ID_CREATE(ONLP_FAN_4), "Fan 4", 0,
            {
                ONLP_FAN_ID_CREATE(ONLP_FAN_4_WEAK),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN4_GREEN),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN4_RED)
            }
        },
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },
    {   {
            ONLP_FAN_ID_CREATE(ONLP_FAN_5), "Fan 5", 0,
            {
                ONLP_FAN_ID_CREATE(ONLP_FAN_5_WEAK),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN5_GREEN),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN5_RED)
            }
        },
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },
    {   { ONLP_FAN_ID_CREATE(ONLP_FAN_1_WEAK), "Fan 1 WEAK", ONLP_FAN_ID_CREATE(ONLP_FAN_1)},
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },

    {   { ONLP_FAN_ID_CREATE(ONLP_FAN_2_WEAK), "Fan 2 WEAK", ONLP_FAN_ID_CREATE(ONLP_FAN_2)},
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },
    {   { ONLP_FAN_ID_CREATE(ONLP_FAN_3_WEAK), "Fan 3 WEAK", ONLP_FAN_ID_CREATE(ONLP_FAN_3)},
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },
    {   { ONLP_FAN_ID_CREATE(ONLP_FAN_4_WEAK), "Fan 4 WEAK", ONLP_FAN_ID_CREATE(ONLP_FAN_4)},
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },
    {   { ONLP_FAN_ID_CREATE(ONLP_FAN_5_WEAK), "Fan 5 WEAK", ONLP_FAN_ID_CREATE(ONLP_FAN_5)},
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },
    {   { ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_1), "PSU-1 Fan", ONLP_PSU_ID_CREATE(ONLP_PSU_1)},
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },
    {   { ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_2), "PSU-2 Fan", ONLP_PSU_ID_CREATE(ONLP_PSU_2)},
        ONLP_FAN_STATUS_PRESENT|ONLP_FAN_STATUS_F2B,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    }
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
    int rv = ONLP_STATUS_OK;
    int local_id;
    int pwm;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    if(local_id >= ONLP_FAN_MAX) {
        rv = ONLP_STATUS_E_INVALID;
    }

    if( ONLP_STATUS_OK == rv) {
        *info = __onlp_fan_info[local_id];
        rv = onlp_file_read_int(&info->rpm, __info_list[local_id].input_file);
    }

    if(ONLP_STATUS_OK == rv) {
        rv = onlp_file_read_int(&pwm, __info_list[local_id].pwm_file);
    }

    if( ONLP_STATUS_OK == rv) {
        if(0 == info->rpm) {
            info->mode = ONLP_FAN_MODE_OFF;
        } else if(pwm < __info_list[local_id].slow_pwm) {
            info->mode = ONLP_FAN_MODE_SLOW;
        } else if(pwm < __info_list[local_id].normal_pwm) {
            info->mode = ONLP_FAN_MODE_NORMAL;
        } else if(pwm < __info_list[local_id].max_pwm) {
            info->mode = ONLP_FAN_MODE_FAST;
        } else {
            info->mode = ONLP_FAN_MODE_MAX;
        }

        info->percentage = (pwm*100)/__info_list[local_id].max_pwm;
        snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "NA");
        snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "NA");
    }
    return rv;
}

/**
 * @brief Retrieve the fan's operational status.
 * @param id The fan OID.
 * @param rv [out] Receives the fan's operations status flags.
 * @notes Only operational state needs to be returned -
 *        PRESENT/FAILED
 */
int onlp_fani_status_get(onlp_oid_t id, uint32_t* rv)
{
    int result = ONLP_STATUS_OK;
    onlp_fan_info_t* info;
    int local_id;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    if(local_id >= ONLP_FAN_MAX) {
        result = ONLP_STATUS_E_INVALID;
    } else {
        info = &__onlp_fan_info[local_id];
        *rv = info->status;
    }
    return result;
}

/**
 * @brief Retrieve the fan's OID hdr.
 * @param id The fan OID.
 * @param rv [out] Receives the OID header.
 */
int onlp_fani_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    int result = ONLP_STATUS_OK;
    onlp_fan_info_t* info;
    int local_id;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    if(local_id >= ONLP_FAN_MAX) {
        result = ONLP_STATUS_E_INVALID;
    } else {
        info = &__onlp_fan_info[local_id];
        *hdr = info->hdr;
    }
    return result;
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

