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
 *
 *
 ***********************************************************/
#ifndef __PLATFORM_LIB_H__
#define __PLATFORM_LIB_H__

#include <unistd.h>
#include <pthread.h>
#include "x86_64_accton_as7927_50x_log.h"

#define CHASSIS_FAN_COUNT      14
#define CHASSIS_THERMAL_COUNT  17
#define CHASSIS_LED_COUNT      6
#define CHASSIS_PSU_COUNT      2
#define NUM_OF_THERMAL_PER_PSU 3

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_SYSFS_FORMAT   "/sys/devices/platform/as7927_50x_psu*psu%d_%s"
#define PSU_SYSFS_FORMAT_1 "/sys/devices/platform/as7927_50x_psu/hwmon/hwmon%d/%s"
#define FAN_SYSFS_FORMAT   "/sys/devices/platform/as7927_50x_fan*"
#define FAN_SYSFS_FORMAT_1 "/sys/devices/platform/as7927_50x_fan/hwmon/hwmon%d/%s"
#define SYS_LED_PATH   "/sys/devices/platform/as7927_50x_led/"
#define IDPROM_PATH "/sys/bus/platform/devices/as7927_50x_sys/eeprom"
#define BIOS_VER_PATH  "/sys/devices/virtual/dmi/id/bios_version"
#define BMC_VER1_PATH  "/sys/devices/platform/ipmi_bmc.0/firmware_revision"
#define BMC_VER2_PATH  "/sys/devices/platform/ipmi_bmc.0/aux_firmware_revision"
#define BMC_THERMAL_DATA_PATH   "/sys/devices/platform/as7927_50x_sys/bmc_thermal_data"

enum onlp_thermal_id {
    THERMAL_RESERVED = 0,
    THERMAL_1_CPU_CORE,
    THERMAL_2_CPU_CORE,
    THERMAL_3_CPU_CORE,
    THERMAL_4_CPU_CORE,
    THERMAL_5_CPU_CORE,
    THERMAL_1_ON_CARRIER_BROAD,
    THERMAL_2_ON_CARRIER_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_IO_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_6_ON_MAIN_BROAD,
    THERMAL_7_ON_MAC_BROAD,
    THERMAL_8_ON_MAC_BROAD,
    THERMAL_9_ON_MAC_BROAD,
    THERMAL_10_ON_MAC_BROAD,
    THERMAL_1_ON_FAN_BROAD,
    THERMAL_2_ON_FAN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_2_ON_PSU1,
    THERMAL_3_ON_PSU1,
    THERMAL_1_ON_PSU2,
    THERMAL_2_ON_PSU2,
    THERMAL_3_ON_PSU2,
    THERMAL_COUNT
};

enum onlp_led_id {
    LED_LOC = 1,
    LED_DIAG,
    LED_ALARM,
    LED_FAN,
    LED_PSU1,
    LED_PSU2
};

enum onlp_fan_dir {
    FAN_DIR_F2B,
    FAN_DIR_B2F,
    FAN_DIR_COUNT
};

enum onlp_psu_type {
    PSU_TYPE_DC,
    PSU_TYPE_AC,
    PSU_TYPE_COUNT
};

typedef enum as7927_50x_platform_id {
    as7927_50x,
    PID_UNKNOWN
} as7927_50x_platform_id_t;

#define PORT_NUM                50
#define LAST_OF_SFP_PORT        48

typedef struct port_thermal_data {
    int present;
    int temp;
    int high_alarm;
} port_thermal_data_t;

typedef struct temp_reader_data {
    port_thermal_data_t ports[PORT_NUM + 1];
} temp_reader_data_t;

enum onlp_fan_dir onlp_get_fan_dir(int fid);
enum onlp_psu_type onlp_get_psu_type(int pid);
int onlp_get_psu_hwmon_idx(int pid);
int onlp_get_fan_hwmon_idx(void);

int get_xcvr_presence(void);
int get_sff8472_temp(int port, int *temp);
int get_sff8472_temp_alarm(int port, int *alarm);
int get_sff8436_temp(int port, int *temp);
int get_sff8436_temp_alarm(int port, int *alarm);
int get_cmis_temp(int port, int *temp);
int get_cmis_temp_alarm(int port, int *alarm);
int get_xcvr_temp(temp_reader_data_t *temp);

#define AIM_FREE_IF_PTR(p) \
    do \
    { \
        if (p) { \
            aim_free(p); \
            p = NULL; \
        } \
    } while (0)

#endif  /* __PLATFORM_LIB_H__ */
