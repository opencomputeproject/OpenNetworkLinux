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

#include "x86_64_accton_minipack_log.h"

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
#define DEBUG_PRINT(fmt, args...)                                        \
        printf("%s#%d: " fmt "\r\n", __FUNCTION__, __LINE__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

#define BIT(i)            (1 << (i))

#define CHASSIS_FAN_COUNT     8
#define CHASSIS_THERMAL_COUNT 8
#define CHASSIS_LED_COUNT     2
#define CHASSIS_PSU_COUNT     4

#define IDPROM_PATH "/sys/bus/i2c/devices/1-0057/eeprom"
#define PLATFOTM_H_TTY_RETRY  (5)

#define MAXIMUM_TTY_BUFFER_LENGTH       1024
#define MAXIMUM_TTY_STRING_LENGTH       (MAXIMUM_TTY_BUFFER_LENGTH - 1)

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_6_ON_MAIN_BROAD,
    THERMAL_7_ON_MAIN_BROAD,
};

int bmc_reply_pure(char *cmd, unsigned long udelay, char *resp, int max_size);
int bmc_reply(char *cmd, char *resp, int max_size);
int bmc_file_read_int(int* value, char *file, int base);
int bmc_i2c_readb(uint8_t bus, uint8_t devaddr, uint8_t addr);
int bmc_i2c_writeb(uint8_t bus, uint8_t devaddr, uint8_t addr, uint8_t value);
int bmc_i2c_readw(uint8_t bus, uint8_t devaddr, uint8_t addr, uint16_t *data);
int bmc_i2c_readraw(uint8_t bus, uint8_t devaddr, uint8_t addr, char* data, int data_size);
#endif  /* __PLATFORM_LIB_H__ */


