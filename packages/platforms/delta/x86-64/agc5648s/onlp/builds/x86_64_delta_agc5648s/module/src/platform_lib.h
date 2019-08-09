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

#include "x86_64_delta_agc5648s_log.h"

#define IDPROM_PATH                     "/sys/class/i2c-adapter/i2c-0/0-0053/eeprom"
#define OS_MAX_MSG_SIZE                 100

#define I2C_BMC_BUS_1                   0x01
#define I2C_BMC_BUS_2                   0x02
#define I2C_BMC_BUS_3                   0x03
#define I2C_BMC_BUS_5                   0x05
#define I2C_BUS_1                       0x00
#define I2C_BUS_2                       0x01
#define SWPLD_1_ADDR                    0x31
#define SWPLD_2_ADDR                    0x32
#define SWPLD_3_ADDR                    0x33
    
#define NUM_OF_THERMAL_ON_MAIN_BROAD    6
#define NUM_OF_LED_ON_MAIN_BROAD        8
#define NUM_OF_PSU_ON_MAIN_BROAD        2
#define NUM_OF_FAN_ON_MAIN_BROAD        10
#define NUM_OF_FAN_ON_PSU_BROAD         2
#define NUM_OF_FAN                      NUM_OF_FAN_ON_MAIN_BROAD + NUM_OF_FAN_ON_PSU_BROAD
    
#define CPLD_VERSION_REGISTER           0x01
#define CPLD_VERSION_OFFSET             4
        
#define PSU_PRESENT_REGISTER            0x52
#define PSU_STATUS_REGISTER             0x14
#define PSU_POWER_GOOD_STATUS           0x01
#define PSU_PRESENT_STATUS              0x00
        
#define PSU_I2C_MUX_ADDR                0x50
#define PSU1_EEPORM_CHANNEL             0x00
#define PSU2_EEPORM_CHANNEL             0x02
        
#define PMBUS_FAN_SPEED                 0x90
#define FAN_ON_PSU1_ADDR                0x58
#define FAN_ON_PSU2_ADDR                0x58
        
#define LED_PSU_REGISTER                0x40
#define LED_SYS_REGISTER                0x40
#define LED_FAN_REGISTER                0x40
#define LED_FAN_TRAY_1_REGISTER         0x41
#define LED_FAN_TRAY_2_REGISTER         0x41
#define LED_FAN_TRAY_3_REGISTER         0x41
#define LED_FAN_TRAY_4_REGISTER         0x41
#define LED_FAN_TRAY_5_REGISTER         0x42
    
#define SFP_PLUS_MIN_PORT               1
#define SFP_PLUS_MAX_PORT               48
#define QSFP_MIN_PORT                   49
#define QSFP_MAX_PORT                   54
    
#define SFP_EEPROM_ADDR                 0x50
#define SFP_1_TO_8_PRESENT_REG          0x70
#define SFP_9_TO_16_PRESENT_REG         0x71
#define SFP_17_TO_24_PRESENT_REG        0x72
#define SFP_25_TO_32_PRESENT_REG        0x73
#define SFP_33_TO_36_PRESENT_REG        0x74
#define SFP_37_TO_44_PRESENT_REG        0x90
#define SFP_45_TO_48_PRESENT_REG        0x91
#define QSFP_49_TO_54_PRESENT_REG       0xB2
    
#define SFP_1_8_RX_LOS_REG              0x75
#define SFP_9_16_RX_LOS_REG             0x76
#define SFP_17_24_RX_LOS_REG            0x77
#define SFP_25_32_RX_LOS_REG            0x78
#define SFP_33_36_RX_LOS_REG            0x79
#define SFP_37_44_RX_LOS_REG            0x95
#define SFP_45_48_RX_LOS_REG            0x96
    
#define SFP_1_8_TX_DISABLE_REG          0x80
#define SFP_9_16_TX_DISABLE_REG         0x81
#define SFP_17_24_TX_DISABLE_REG        0x82
#define SFP_25_32_TX_DISABLE_REG        0x83
#define SFP_33_36_TX_DISABLE_REG        0x84
#define SFP_37_44_TX_DISABLE_REG        0xA0
#define SFP_45_48_TX_DISABLE_REG        0xA1
    
#define INVALID_ADDR                    0xFF
#define INVALID_REG                     0xFF
#define INVALID_REG_BIT                 0xFF
    
#define THERMAL_CPU_ADDR                0x4D
#define THERMAL_FAN_ADDR                0x4F
#define THERMAL_AMBI_ADDR               0x48
#define THERMAL_KBP1_ADDR               0x4E
#define THERMAL_KBP2_ADDR               0x4F
#define THERMAL_JER1_ADDR               0x4C
#define THERMAL_JER2_ADDR               0x4D
#define THERMAL_REGISTER                0x00
                
#define DATA_LEN                        0x01

enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_PSU,
    LED_SYS,
    LED_FAN,
    LED_FAN_TRAY_1,
    LED_FAN_TRAY_2,
    LED_FAN_TRAY_3,
    LED_FAN_TRAY_4,
    LED_FAN_TRAY_5
};

enum onlp_fan_id
{    
    FAN_RESERVED = 0,    
    FAN_1_ON_FAN_BOARD,    
    FAN_2_ON_FAN_BOARD, 
    FAN_3_ON_FAN_BOARD,
    FAN_4_ON_FAN_BOARD,
    FAN_5_ON_FAN_BOARD,
    FAN_6_ON_FAN_BOARD,    
    FAN_7_ON_FAN_BOARD, 
    FAN_8_ON_FAN_BOARD,
    FAN_9_ON_FAN_BOARD,
    FAN_10_ON_FAN_BOARD,
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
    THERMAL_AMBI_ON_MAIN_BOARD,
    THERMAL_KBP1_ON_MAIN_BOARD,
    THERMAL_KBP2_ON_MAIN_BOARD,
    THERMAL_JER1_ON_MAIN_BOARD,
    THERMAL_JER2_ON_MAIN_BOARD,
};

enum led_light_mode 
{
    LED_MODE_OFF    = 0,
    LED_MODE_GREEN  = 1,
    LED_MODE_RED    = 3,
    LED_MODE_AMBER  = 2,
    LED_MODE_YELLOW = 2,
    LED_MODE_GREEN_BLINK  = 2,
    LED_MODE_AMBER_BLINK  = 2,
    LED_MODE_YELLOW_BLINK = 3,
    LED_MODE_FAN_TRAY_RED = 2,
    LED_MODE_FAN_TRAY_GREEN = 1,
    LED_MODE_UNKNOWN
};

int ifnOS_LINUX_BmcI2CGet(uint8_t bus, uint8_t dev, uint32_t addr, uint32_t *data, uint8_t datalen);
int ifnOS_LINUX_BmcI2CSet(uint8_t bus, uint8_t dev, uint32_t addr, uint32_t data, uint8_t datalen);
int ifnOS_LINUX_BmcGetDataByName(char *FanName, uint32_t *data);
uint32_t xtoi(const char* str);

#endif  /* __PLATFORM_LIB_H__ */

