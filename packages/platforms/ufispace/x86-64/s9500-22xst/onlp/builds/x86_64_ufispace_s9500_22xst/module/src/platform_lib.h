/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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

#include <onlp/fan.h>
#include <onlp/psu.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/sysi.h>
#include "x86_64_ufispace_s9500_22xst_int.h"
#include "x86_64_ufispace_s9500_22xst_log.h"

#include <x86_64_ufispace_s9500_22xst/x86_64_ufispace_s9500_22xst_config.h>
#define SYS_DEV                     "/sys/bus/i2c/devices/"
#define SYS_CPU_CORETEMP_PREFIX     "/sys/devices/platform/coretemp.0/hwmon/hwmon0/"
#define SYS_CPU_CORETEMP_PREFIX2    "/sys/devices/platform/coretemp.0/"
#define SYS_CORE_TEMP_PREFIX        "/sys/class/hwmon/hwmon2/"
#define SYS_CPU_BOARD_TEMP_PREFIX   "/sys/bus/i2c/devices/0-004f/hwmon/hwmon1/"
#define SYS_CPU_BOARD_TEMP_PREFIX2  "/sys/bus/i2c/devices/0-004f/"

#define SYS_FAN_PREFIX              "/sys/class/hwmon/hwmon1/device/"
#define SYS_EEPROM_PATH             "/sys/bus/i2c/devices/0-0057/eeprom"
#define SYS_EEPROM_SIZE 512
#define PSU1_EEPROM_PATH            "/sys/bus/i2c/devices/58-0050/eeprom"
#define PSU2_EEPROM_PATH            "/sys/bus/i2c/devices/57-0050/eeprom"
#define BMC_EN_FILE_PATH            "/etc/onl/bmc_en"
#define BMC_SENSOR_CACHE            "/tmp/bmc_sensor_cache"
#define CMD_BIOS_VER                "dmidecode -s bios-version | tail -1 | tr -d '\r\n'"
#define CMD_BMC_VER_1               "expr `ipmitool mc info | grep 'Firmware Revision' | cut -d':' -f2 | cut -d'.' -f1` + 0"
#define CMD_BMC_VER_2               "expr `ipmitool mc info | grep 'Firmware Revision' | cut -d':' -f2 | cut -d'.' -f2` + 0"
#define CMD_BMC_VER_3               "echo $((`ipmitool mc info | grep 'Aux Firmware Rev Info' -A 2 | sed -n '2p'`))"
#define CMD_BMC_SENSOR_CACHE        "ipmitool sdr -c get Temp_CPU Temp_MAC Temp_BMC Temp_100GCage Temp_DDR4 Temp_FANCARD1 Temp_FANCARD2 PSU0_Temp PSU1_Temp FAN_1 FAN_2 FAN_3 PSU0_FAN PSU1_FAN Fan1_Presence Fan2_Presence Fan3_Presence PSU0_Presence PSU1_Presence PSU0_POWEROK PSU1_POWEROK PSU0_VIN PSU0_VOUT PSU0_IIN PSU0_IOUT PSU1_VIN PSU1_VOUT PSU1_IIN PSU1_IOUT HWM_Temp_AMB HWM_Temp_PHY1 HWM_Temp_Heater > /tmp/bmc_sensor_cache"
#define CMD_UCD_VER                 "ipmitool raw 0x3c 0x12 0x0 0x34 0x6 0x9b"
#define CMD_BMC_SDR_GET             "ipmitool sdr -c get %s"
#define CMD_FRU_INFO_GET            "ipmitool fru print %d | grep '%s' | cut -d':' -f2 | awk '{$1=$1};1' | tr -d '\n'"
#define CMD_BMC_CACHE_GET           "cat "BMC_SENSOR_CACHE" | grep %s | awk -F',' '{print $%d}'"
#define PSU_STATUS_PRESENT          1
#define PSU_STATUS_POWER_GOOD       1
#define FAN_PRESENT                 0
#define FAN_CTRL_SET1               1
#define FAN_CTRL_SET2               2
#define MAX_SYS_FAN_NUM             5
#define BOARD_THERMAL_NUM           6
#define SYS_FAN_NUM                 5
#define QSFP_NUM                    2
#define SFP28_NUM                   8
#define SFP_NUM                     8
#define RJ45_NUM                    4
#define PORT_NUM                    22

#define THERMAL_NUM                 21
#define LED_NUM                     5
#define FAN_NUM                     3



#define THERMAL_SHUTDOWN_DEFAULT    105000

#define THERMAL_ERROR_DEFAULT       95000
#define THERMAL_ERROR_FAN_PERC      100

#define THERMAL_WARNING_DEFAULT     77000
#define THERMAL_WARNING_FAN_PERC    80

#define THERMAL_NORMAL_DEFAULT      72000
#define THERMAL_NORMAL_FAN_PERC     50

/* I2C bus */
#define I2C_BUS_0               0
#define I2C_BUS_1               1
#define I2C_BUS_2               2
#define I2C_BUS_3               3
#define I2C_BUS_4               4
#define I2C_BUS_5               5
#define I2C_BUS_6               6
#define I2C_BUS_7               7
#define I2C_BUS_8               8
#define I2C_BUS_9               9

/* PSU */

/* LED */
#define LED_CTRL_REG            0x718
#define LED_SYS_ONOFF_MASK      0x40
#define LED_SYNC_ONOFF_MASK     0x01
#define LED_GPS_ONOFF_MASK      0x04
#define LED_SYS_COLOR_MASK      0x80
#define LED_SYNC_COLOR_MASK     0x02
#define LED_GPS_COLOR_MASK      0x08

#define LED_BLINKING_REG        0x71A
#define LED_SYS_BLINK_MASK      0x08
#define LED_SYNC_BLINK_MASK     0x01
#define LED_GPS_BLINK_MASK      0x02

#define LED_COLOR_GREEN         1
#define LED_COLOR_YELLOW        0
#define LED_BLINKING            1
#define LED_STABLE              0

/* SYS */

/* QSFP */

/* FAN */

/* BMC CMD */
#define BMC_CACHE_EN            1
#define BMC_CACHE_CYCLE         30
#define BMC_CMD_SDR_SIZE        48
#define BMC_TOKEN_SIZE          20

#define FAN_CACHE_TIME          5
#define PSU_CACHE_TIME          5
#define THERMAL_CACHE_TIME      10

enum sensor
{
    FAN_SENSOR = 0,
    PSU_SENSOR,
    THERMAL_SENSOR,
};

typedef struct bmc_info_s
{
    char name[20];
    float data;
}bmc_info_t;

/** led_oid */
typedef enum led_oid_e {
    LED_OID_SYSTEM    = ONLP_LED_ID_CREATE(1),
    LED_OID_SYNC      = ONLP_LED_ID_CREATE(2),
    LED_OID_GPS       = ONLP_LED_ID_CREATE(3),
    LED_OID_FAN_TRAY1 = ONLP_LED_ID_CREATE(4),
    LED_OID_FAN_TRAY2 = ONLP_LED_ID_CREATE(5),
    LED_OID_FAN_TRAY3 = ONLP_LED_ID_CREATE(6)
} led_oid_t;

/** led_id */
typedef enum led_id_e {
    LED_ID_SYS_SYS   = 1,
    LED_ID_SYS_SYNC  = 2,
    LED_ID_SYS_GPS   = 3,
    LED_ID_FAN_TRAY1 = 4,
    LED_ID_FAN_TRAY2 = 5,
    LED_ID_FAN_TRAY3 = 6,
    LED_ID_MAX       = 7
} led_id_t;

/** Thermal_oid */
typedef enum thermal_oid_e {
    THERMAL_OID_CPU      = ONLP_THERMAL_ID_CREATE(1),
    THERMAL_OID_MAC      = ONLP_THERMAL_ID_CREATE(2),
    THERMAL_OID_BMC      = ONLP_THERMAL_ID_CREATE(3),
    THERMAL_OID_100G_CAGE= ONLP_THERMAL_ID_CREATE(4),
    THERMAL_OID_DDR4     = ONLP_THERMAL_ID_CREATE(5),
    THERMAL_OID_FANCARD1 = ONLP_THERMAL_ID_CREATE(6),
    THERMAL_OID_FANCARD2 = ONLP_THERMAL_ID_CREATE(7),
    THERMAL_OID_PSU0     = ONLP_THERMAL_ID_CREATE(8),
    THERMAL_OID_PSU1     = ONLP_THERMAL_ID_CREATE(9),
    THERMAL_OID_CPU_PKG  = ONLP_THERMAL_ID_CREATE(10),
    THERMAL_OID_CPU1     = ONLP_THERMAL_ID_CREATE(11),
    THERMAL_OID_CPU2     = ONLP_THERMAL_ID_CREATE(12),
    THERMAL_OID_CPU3     = ONLP_THERMAL_ID_CREATE(13),
    THERMAL_OID_CPU4     = ONLP_THERMAL_ID_CREATE(14),
    THERMAL_OID_CPU_BOARD= ONLP_THERMAL_ID_CREATE(15),
    THERMAL_OID_AMB      = ONLP_THERMAL_ID_CREATE(16),
    THERMAL_OID_PHY1     = ONLP_THERMAL_ID_CREATE(17),
    THERMAL_OID_HEATER   = ONLP_THERMAL_ID_CREATE(18),
} thermal_oid_t;

/** thermal_id */
typedef enum thermal_id_e {
    THERMAL_ID_CPU      = 1,
    THERMAL_ID_MAC      = 2,
    THERMAL_ID_BMC      = 3,
    THERMAL_ID_100G_CAGE= 4,
    THERMAL_ID_DDR4     = 5,
    THERMAL_ID_FANCARD1 = 6,
    THERMAL_ID_FANCARD2 = 7,
    THERMAL_ID_PSU0     = 8,
    THERMAL_ID_PSU1     = 9,
    THERMAL_ID_CPU_PKG  = 10,
    THERMAL_ID_CPU1     = 11,
    THERMAL_ID_CPU2     = 12,
    THERMAL_ID_CPU3     = 13,
    THERMAL_ID_CPU4     = 14,
    THERMAL_ID_CPU_BOARD= 15,
    THERMAL_ID_AMB      = 16,
    THERMAL_ID_PHY1     = 17,
    THERMAL_ID_HEATER   = 18,
    THERMAL_ID_MAX      = 19,
} thermal_id_t;

/* Shortcut for CPU thermal threshold value. */
#define THERMAL_THRESHOLD_INIT_DEFAULTS  \
    { THERMAL_WARNING_DEFAULT, \
      THERMAL_ERROR_DEFAULT,   \
      THERMAL_SHUTDOWN_DEFAULT }

/** Fan_oid */
typedef enum fan_oid_e {
    FAN_OID_FAN1     = ONLP_FAN_ID_CREATE(1),
    FAN_OID_FAN2     = ONLP_FAN_ID_CREATE(2),
    FAN_OID_FAN3     = ONLP_FAN_ID_CREATE(3),
    FAN_OID_PSU0_FAN = ONLP_FAN_ID_CREATE(4),
    FAN_OID_PSU1_FAN = ONLP_FAN_ID_CREATE(5),
} fan_oid_t;

/** fan_id */
typedef enum fan_id_e {
    FAN_ID_FAN1     = 1,
    FAN_ID_FAN2     = 2,
    FAN_ID_FAN3     = 3,
    FAN_ID_PSU0_FAN = 4,
    FAN_ID_PSU1_FAN = 5,
    FAN_ID_MAX      = 6,
} fan_id_t;

/** led_oid */
typedef enum psu_oid_e {
    PSU_OID_PSU0 = ONLP_PSU_ID_CREATE(1),
    PSU_OID_PSU1 = ONLP_PSU_ID_CREATE(2)
} psu_oid_t;

/** psu_id */
typedef enum psu_id_e {
    PSU_ID_PSU0      = 1,
    PSU_ID_PSU1      = 2,
    PSU_ID_PSU0_VIN  = 3,
    PSU_ID_PSU0_VOUT = 4,
    PSU_ID_PSU0_IIN  = 5,
    PSU_ID_PSU0_IOUT = 6,
    PSU_ID_PSU1_VIN  = 7,
    PSU_ID_PSU1_VOUT = 8,
    PSU_ID_PSU1_IIN  = 9,
    PSU_ID_PSU1_IOUT = 10,
    PSU_ID_MAX       = 11,
} psu_id_t;

int psu_thermal_get(onlp_thermal_info_t* info, int id);

int psu_fan_info_get(onlp_fan_info_t* info, int id);

int psu_vin_get(onlp_psu_info_t* info, int id);

int psu_vout_get(onlp_psu_info_t* info, int id);

int psu_iin_get(onlp_psu_info_t* info, int id);

int psu_iout_get(onlp_psu_info_t* info, int id);

int psu_pout_get(onlp_psu_info_t* info, int id);

int psu_pin_get(onlp_psu_info_t* info, int id);

int psu_eeprom_get(onlp_psu_info_t* info, int id);

int psu_present_get(int *pw_present, int id);

int psu_pwgood_get(int *pw_good, int id);

int psu2_led_set(onlp_led_mode_t mode);

int psu1_led_set(onlp_led_mode_t mode);

int fan_led_set(onlp_led_mode_t mode);

int system_led_set(onlp_led_mode_t mode);

int fan_tray_led_set(onlp_oid_t id, onlp_led_mode_t mode);

int sysi_platform_info_get(onlp_platform_info_t* pi);

int qsfp_present_get(int port, int *pres_val);

int sfp_present_get(int port, int *pres_val);

int sfp_rx_los_get(int port, int *ctrl_val);

int sfp_tx_fault_get(int port, int *ctrl_val);

int sfp_tx_disable_get(int port, int *ctrl_val);

int qsfp_lp_mode_get(int port, int *ctrl_val);

int qsfp_reset_get(int port, int *ctrl_val);

int sfp_tx_disable_set(int port, int ctrl_val);

int qsfp_lp_mode_set(int port, int ctrl_val);

int qsfp_reset_set(int port, int ctrl_val);

bool onlp_sysi_bmc_en_get(void);

int read_ioport(int addr, int *reg_val);

int write_ioport(int addr, int val);

int bmc_thermal_info_get(onlp_thermal_info_t* info, int id);

int bmc_fan_info_get(onlp_fan_info_t* info, int id);

int exec_cmd(char *cmd, char* out, int size);

int file_read_hex(int* value, const char* fmt, ...);

int file_vread_hex(int* value, const char* fmt, va_list vargs);

int parse_bmc_sdr_cmd(char *cmd_out, int cmd_out_size,
                  char *tokens[], int token_size,
                  const char *sensor_id_str, int *idx);

int sys_led_info_get(onlp_led_info_t* info, int id);

int sys_led_set(int id, int on_or_off);

int sys_led_mode_set(int id, int color, int blink);

int psu_fru_get(onlp_psu_info_t* info, int id);

void
lock_init();

int
bmc_sensor_read(int bmc_cache_index, int sensor_type, float *data);

extern bool bmc_enable;
#endif  /* __PLATFORM_LIB_H__ */
