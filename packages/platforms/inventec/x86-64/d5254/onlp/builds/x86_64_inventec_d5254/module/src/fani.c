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
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_1),
            .description = "Fan 1",
            .poid = ONLP_OID_CHASSIS,
            .coids = {
                ONLP_FAN_ID_CREATE(ONLP_FAN_1_WEAK),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN1_GREEN),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN1_RED)
            },
            .status = ONLP_OID_STATUS_FLAG_PRESENT
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_2),
            .description = "Fan 2",
            .poid = ONLP_OID_CHASSIS,
            .coids = {
                ONLP_FAN_ID_CREATE(ONLP_FAN_2_WEAK),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN2_GREEN),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN2_RED)
            },
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_3),
            .description = "Fan 3",
            .poid = ONLP_OID_CHASSIS,
            .coids = {
                ONLP_FAN_ID_CREATE(ONLP_FAN_3_WEAK),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN3_GREEN),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN3_RED)
            },
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_4),
            .description = "Fan 4",
            .poid = ONLP_OID_CHASSIS,
            .coids = {
                ONLP_FAN_ID_CREATE(ONLP_FAN_4_WEAK),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN4_GREEN),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN4_RED)
            },
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_5),
            .description = "Fan 5",
            .poid = ONLP_OID_CHASSIS,
            .coids = {
                ONLP_FAN_ID_CREATE(ONLP_FAN_5_WEAK),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN5_GREEN),
                ONLP_LED_ID_CREATE(ONLP_LED_FAN5_RED)
            },
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_1_WEAK),
            .description = "Fan 1 WEAK",
            .poid = ONLP_FAN_ID_CREATE(ONLP_FAN_1),
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_2_WEAK),
            .description = "Fan 2 WEAK",
            .poid = ONLP_FAN_ID_CREATE(ONLP_FAN_2),
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_3_WEAK),
            .description = "Fan 3 WEAK",
            .poid = ONLP_FAN_ID_CREATE(ONLP_FAN_3),
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_4_WEAK),
            .description = "Fan 4 WEAK",
            .poid = ONLP_FAN_ID_CREATE(ONLP_FAN_4),
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_5_WEAK),
            .description = "Fan 5 WEAK",
            .poid = ONLP_FAN_ID_CREATE(ONLP_FAN_5),
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_1),
            .description = "PSU-1 Fan",
            .poid = ONLP_PSU_ID_CREATE(ONLP_PSU_1),
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_2),
            .description = "PSU-2 Fan",
            .poid = ONLP_PSU_ID_CREATE(ONLP_PSU_2),
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .dir = ONLP_FAN_DIR_F2B,
        .caps = ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
};


int
onlp_fani_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, ONLP_FAN_MAX-1);
}

int
onlp_fani_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    /** No Failed state processing? */
    *hdr = __onlp_fan_info[id].hdr;
    return ONLP_STATUS_OK;
}

int
onlp_fani_info_get(onlp_oid_id_t id, onlp_fan_info_t* info)
{
    int pwm;

    ONLP_TRY(onlp_fani_hdr_get(id, &info->hdr));

    ONLP_TRY(onlp_file_read_int(&info->rpm, __info_list[id].input_file));
    ONLP_TRY(onlp_file_read_int(&pwm, __info_list[id].pwm_file));

    info->percentage = (pwm*100)/__info_list[id].max_pwm;
    snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "NA");
    snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "NA");

    return ONLP_STATUS_OK;
}
