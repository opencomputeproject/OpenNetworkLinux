/************************************************************
 * <bsn.cl fy=2017 v=onl>
 *
 *        Copyright 2017 Delta Networks, Inc.
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

#include "x86_64_delta_ag9064_log.h"

#define IDPROM_PATH                     "/sys/class/i2c-adapter/i2c-0/0-0056/eeprom"
                                       
#define I2C_BMC_BUS_1                   0x00
#define I2C_BMC_BUS_3                   0x02
#define I2C_BMC_BUS_5                   0x04
#define I2C_BUS_1                       0x00
#define I2C_BUS_2                       0x01
#define I2C_BUS_3                       0x02
                                        
#define SWPLD_1_ADDR                    0x35
#define SWPLD_2_ADDR                    0x34
#define SWPLD_3_ADDR                    0x33
#define SWPLD_4_ADDR                    0x32
                                        
#define NUM_OF_THERMAL_ON_MAIN_BROAD    6
#define NUM_OF_LED_ON_MAIN_BROAD        8
#define NUM_OF_PSU_ON_MAIN_BROAD        2
#define NUM_OF_FAN_ON_MAIN_BROAD        8
#define NUM_OF_FAN_ON_PSU_BROAD         2
#define NUM_OF_FAN                      NUM_OF_FAN_ON_MAIN_BROAD + NUM_OF_FAN_ON_PSU_BROAD
                                       
#define CPLD_VERSION_REGISTER           0x00
#define CPLD_VERSION_OFFSET             4
                                        
#define LED_FAN_SYS_FRONT_REGISTER      0x02
#define LED_FAN_TRAY_REGISTER           0x1B
#define LED_FAN_FRONT_BIT               0x06
#define LED_SYS_FRONT_BIT               0x04
#define LED_FAN_TRAY_1_BIT              0x07
#define LED_FAN_TRAY_2_BIT              0x06
#define LED_FAN_TRAY_3_BIT              0x05
#define LED_FAN_TRAY_4_BIT              0x04
                                        
#define PSU_STATUS_REGISTER             0x02
#define PSU1_PRESENT_BIT                0x07
#define PSU1_POWER_GOOD_BIT             0x06
#define PSU1_INT_BIT                    0x05
#define PSU2_PRESENT_BIT                0x03
#define PSU2_POWER_GOOD_BIT             0x02
#define PSU2_INT_BIT                    0x01
#define PSU_INT_HAPPEN_STATUS           0x01
#define PSU_POWER_GOOD_STATUS           0x00
#define PSU_PRESENT_STATUS              0x00
                                        
#define FAN_SPEED_PMBUS                 0x90
#define FAN_ON_PSU1_ADDR                0x58
#define FAN_ON_PSU2_ADDR                0x59
                                        
#define THERMAL_CPU_ADDR                0x4D
#define THERMAL_FAN_ADDR                0x4F
#define THERMAL_1_ADDR                  0x4C
#define THERMAL_2_ADDR                  0x4E
#define THERMAL_3_ADDR                  0x4B
#define THERMAL_4_ADDR                  0x4A
#define THERMAL_REGISTER                0x00
                                        
#define QSFP_MIN_PORT                   1
#define QSFP_MAX_PORT                   64
                                        
#define PCA9548_I2C_MUX_ADDR            0x70
#define QSFP_CHAN_ON_PCA9548            0x04
#define QSFP_PORT_MUX_REG               0x13
#define QSFP_EEPROM_ADDR                0x50
                                        
#define QSFP_1_TO_8_PRESENT_REG         0x03
#define QSFP_9_TO_16_PRESENT_REG        0x03
#define QSFP_17_TO_24_PRESENT_REG       0x24
#define QSFP_25_TO_32_PRESENT_REG       0x24
#define QSFP_33_TO_40_PRESENT_REG       0x04
#define QSFP_41_TO_48_PRESENT_REG       0x04
#define QSFP_49_TO_56_PRESENT_REG       0x25
#define QSFP_57_TO_64_PRESENT_REG       0x25
                                        
#define OS_MAX_MSG_SIZE                 100

#define INVALID_ADDR                    0xFF
#define INVALID_REG                     0xFF
#define INVALID_REG_BIT                 0xFF

enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_FAN,
    LED_SYS,
    LED_PSU1,
    LED_PSU2,
    LED_FAN_TRAY_1,
    LED_FAN_TRAY_2,
    LED_FAN_TRAY_3,
    LED_FAN_TRAY_4
};

enum onlp_fan_id
{    
    FAN_RESERVED = 0,    
    FAN_1_ON_FAN_BOARD,    
    FAN_2_ON_FAN_BOARD, 
    FAN_3_ON_FAN_BOARD,
    FAN_4_ON_FAN_BOARD,
    FAN_6_ON_FAN_BOARD,
    FAN_7_ON_FAN_BOARD,
    FAN_8_ON_FAN_BOARD,
    FAN_9_ON_FAN_BOARD,
    FAN_ON_PSU1,
    FAN_ON_PSU2
};

enum onlp_psu_id
{    
    PSU_RESERVED = 0,    
    PSU_1,
    PSU_2,
};

enum onlp_thermal_id
{    
    THERMAL_RESERVED = 0,    
    THERMAL_CPU_CORE,    
    THERMAL_ON_FAN_BROAD,
    THERMAL_1_ON_MAIN_BOARD,
    THERMAL_2_ON_MAIN_BOARD,
    THERMAL_3_ON_MAIN_BOARD,
    THERMAL_4_ON_MAIN_BOARD,
};

enum led_light_mode 
{
    LED_MODE_OFF = 0,
    LED_MODE_GREEN = 1,
    LED_MODE_RED = 2,
    LED_MODE_FAN_TRAY_GREEN = 1,
    LED_MODE_SYS_RED = 3,
    LED_MODE_FAN_TRAY_AMBER = 0,
    LED_MODE_GREEN_BLINK = 2,
    LED_MODE_UNKNOWN
};

int ifnOS_LINUX_BmcI2CGet(uint8_t bus, uint8_t dev, uint32_t addr, uint32_t *data, uint8_t datalen);
int ifnOS_LINUX_BmcI2CSet(uint8_t bus, uint8_t dev, uint32_t addr, uint32_t data, uint8_t datalen);
int ifnOS_LINUX_BmcGetDataByName(char *FanName, uint32_t *data);
uint32_t xtoi(const char* str);

#endif  /* __PLATFORM_LIB_H__ */

