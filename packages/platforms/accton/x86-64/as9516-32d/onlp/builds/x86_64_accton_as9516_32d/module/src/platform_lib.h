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


#include "x86_64_accton_as9516_32d_log.h"
#include "bf_pltfm_fpga.h"
#include "bf_fpga_ioctl.h"
#include <curl/curl.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <onlplib/file.h>
#include <onlp/onlp.h>

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(fmt, args...)                                        \
        printf("%s:%s[%d]: " fmt "\r\n", __FILE__, __FUNCTION__, __LINE__, ##args)
#else
    #define DEBUG_PRINT(fmt, args...)
#endif

#define CHASSIS_FAN_COUNT     6
#define CHASSIS_THERMAL_COUNT 7
#define CHASSIS_LED_COUNT     2
#define CHASSIS_PSU_COUNT     2

#define PSU1_ID 1
#define PSU2_ID 2
#define PSU_PRESENT true
#define PSU_ABSCENT false
#define MAX_PSU_FAN_SPEED 23000
        
#define PS_DESC_LEN 32
/* CURL data status */
#define curl_data_normal                        0
#define curl_data_fan_present                   1
#define curl_data_fan_absent                    0
/* CURL data location - Thermal */
enum curl_loc_data_thermal
{
    curl_data_loc_thermal_status = 0,
    curl_data_loc_thermal_3_48,
    curl_data_loc_thermal_3_49,
    curl_data_loc_thermal_3_4a,
    curl_data_loc_thermal_3_4b,
    curl_data_loc_thermal_3_4c_max6658,
    curl_data_loc_thermal_3_4d,
    curl_data_thermal_num
};
/* CURL data location - PSU */
enum curl_loc_data_psu
{
    curl_data_loc_psu_status = 0,
    curl_data_loc_psu_vin,
    curl_data_loc_psu_vout,
    curl_data_loc_psu_iin,
    curl_data_loc_psu_pin,
    curl_data_loc_psu_fan,
    curl_data_loc_psu_fan_status,
    curl_data_loc_psu_present,
    curl_data_loc_psu_load_share,
    curl_data_loc_psu_model_name,
    curl_data_loc_psu_mode_serial,
    curl_data_loc_psu_model_ver,
    curl_data_psu_num
};
/* CURL data location - FAN */
enum curl_loc_data_fan
{
    curl_data_loc_fan_status = 0,
    curl_data_loc_fan_id,
    curl_data_loc_fan_front_rpm,
    curl_data_loc_fan_rear_rpm,
    curl_data_loc_fan_pwm,
    curl_data_loc_fan_present,
    curl_data_fan_num
};

#define BMC_CURL_PREFIX "http://[fe80::ff:fe00:1%usb0]:8080/api/sys/bmc/"
#define BMC_CPLD_CURL_PREFIX "http://[fe80::ff:fe00:1%usb0]:8080/api/sys/cpldget/"
#define BMC_FAN_CPLD_CURL_PREFIX "http://[fe80::ff:fe00:1%usb0]:8080/api/sys/fancpldget/"
#define CURL_IGNORE_OFFSET 17

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_6_ON_MAIN_BROAD
};

enum fan_id {
    FAN_1_ON_FAN_BOARD = 1,
    FAN_2_ON_FAN_BOARD,
    FAN_3_ON_FAN_BOARD,
    FAN_4_ON_FAN_BOARD,
    FAN_5_ON_FAN_BOARD,
    FAN_6_ON_FAN_BOARD,
    FAN_ON_PSU_1,
    FAN_ON_PSU_2
};

#define HANDLECOUNT 14
enum curl_id
{
    CURL_THERMAL = 0,
    CURL_PSU_STATUS_1,
    CURL_PSU_STATUS_2,
    CURL_FAN_STATUS_1,
    CURL_FAN_STATUS_2,
    CURL_FAN_STATUS_3,
    CURL_FAN_STATUS_4,
    CURL_FAN_STATUS_5,
    CURL_FAN_STATUS_6,
    CURL_PSU_1_FAN,
    CURL_PSU_2_FAN,
    CURL_SYS_CPLD_VER,
    CURL_SYS_CPLD_SUBVER,
    CURL_FAN_CPLD
};
CURL *curl[HANDLECOUNT];
CURLM *multi_curl;
CURLMsg *msg;

int bmc_curl_init(void);
int bmc_curl_deinit(void);

/* fpga proc i2c read/write/addr read */
int fpga_proc_i2c_read(int fpga_id, uint8_t bus, uint8_t mux_i2c_addr, 
                        uint8_t mux_chn, uint8_t i2c_addr, int rd_size, uint8_t byte_buf[]);
int fpga_proc_i2c_write(int fpga_id, uint8_t bus, uint8_t mux_i2c_addr, 
                        uint8_t mux_chn, uint8_t i2c_addr, int wr_size, uint8_t byte_buf[]);
int fpga_proc_i2c_addr_read(int fpga_id, uint8_t bus, uint8_t mux_i2c_addr, uint8_t mux_chn, 
                            uint8_t i2c_addr, int rd_size, int wr_size, uint8_t byte_buf[]);
int fpga_pltfm_init(int fpga_id);
void fpga_pltfm_deinit(int fpga_id);
#endif  /* __PLATFORM_LIB_H__ */
