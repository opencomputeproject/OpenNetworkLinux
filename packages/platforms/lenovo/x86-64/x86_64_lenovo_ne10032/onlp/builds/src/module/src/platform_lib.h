/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2018 Alpha Networks Incorporation.
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

#include "x86_64_lenovo_ne10032_log.h"

//for ne10032 
#define IDPROM_PATH "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/0-0056/eeprom"
//#define IDPROM_PATH "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-6/i2c-10/10-0051/eeprom"

#define PSU1_ID 1
#define PSU2_ID 2
#define PSUI_BUS_ID_OFFSET      9
#define PSU1_BUS_ID             (PSUI_BUS_ID_OFFSET + PSU1_ID)
#define PSU2_BUS_ID             (PSUI_BUS_ID_OFFSET + PSU2_ID)

#define CHASSIS_THERMAL_COUNT 2
#define CHASSIS_FAN_COUNT     6

#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/57-003c/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/58-003f/"

#define PSU1_AC_HWMON_PREFIX "/sys/bus/i2c/devices/2-0050/"
#define PSU1_DC_HWMON_PREFIX "/sys/bus/i2c/devices/57-0050/"
#define PSU2_AC_HWMON_PREFIX "/sys/bus/i2c/devices/3-0050/"
#define PSU2_DC_HWMON_PREFIX "/sys/bus/i2c/devices/58-0053/"

#define PSU1_AC_HWMON_NODE(node) PSU1_AC_HWMON_PREFIX#node
#define PSU1_DC_HWMON_NODE(node) PSU1_DC_HWMON_PREFIX#node
#define PSU2_AC_HWMON_NODE(node) PSU2_AC_HWMON_PREFIX#node
#define PSU2_DC_HWMON_NODE(node) PSU2_DC_HWMON_PREFIX#node

#define NUM_OF_SFP_PORT 32
#define QSFP28_EEPROM_I2C_ADDR            0x50  /* QSFP28 EEPROM Physical Address in the I2C */

#define QSFP28_EEPROM_TXRX_LOS_OFFSET     3
#define QSFP28_EEPROM_TX_FAULT_OFFSET     4
#define QSFP28_EEPROM_TX_DISABLE_OFFSET   86
#define QSFP28_EEPROM_POWERSET_OFFSET     93
#define QSFP28_EEPROM_PAGE_SELECT_OFFSET  127

typedef long long               I64_T;      /* 64-bit signed   */
typedef unsigned long long      UI64_T;     /* 64-bit unsigned */
typedef long                    I32_T;      /* 32-bit signed   */
typedef unsigned long           UI32_T;     /* 32-bit unsigned */
typedef short int               I16_T;      /* 16-bit signed   */
typedef unsigned short int      UI16_T;     /* 16-bit unsigned */
typedef char                    I8_T;       /* 8-bit signed    */
typedef unsigned char           UI8_T;      /* 8-bit unsigned  */

/*----------------------------------------------------------*/
/*  BIT operation                                           */
/*----------------------------------------------------------*/
#define UTL_TEST_BITS(__type__,__var__,__pos__)                         \
        (__type__)((((__type__)(__var__)>>(__type__)(__pos__))&(__type__)1)?(__type__)1:(__type__)0)

#define UTL_LEFT_SHIFT_BITS(__type__,__range__,__var__,__sft_bits__)    \
        (((__type__)(__sft_bits__)>=(__range__))?0:((__type__)(__var__)<<(__type__)(__sft_bits__)))

#define UTL_RIGHT_SHIFT_BITS(__type__,__range__,__var__,__sft_bits__)   \
        (((__type__)__sft_bits__>=(__range__))?(__type__)(__var__):((__type__)(__var__)>>(__type__)(__sft_bits__)))

#define UTL_SET_BITS(__type__,__range__,__var__,__pos__)                \
        ((__type__)(__var__)|UTL_LEFT_SHIFT_BITS(__type__,__range__,(__type__)1U,(__type__)(__pos__)))

#define UTL_RESET_BITS(__type__,__range__,__var__,__pos__)              \
        ((__type__)(__var__)&~UTL_LEFT_SHIFT_BITS(__type__,__range__,(__type__)1U,(__type__)(__pos__)))

/*----------------------------------------------------------*/
/*  64 BIT operation                                        */
/*----------------------------------------------------------*/
#define UTL_TEST_BITS64(__var__,__pos__)     UTL_TEST_BITS(UI64_T,__var__,__pos__)

#define UTL_SET_BITS64(__var__,__pos__)      (__var__) = UTL_SET_BITS(UI64_T,64,__var__,__pos__)
#define UTL_RESET_BITS64(__var__,__pos__)    (__var__) = UTL_RESET_BITS(UI64_T,64,__var__,__pos__)

/*----------------------------------------------------------*/
/*  32 BIT operation                                        */
/*----------------------------------------------------------*/
/*   Usage: if( UTL_TEST_BITS32(val,2) )*/
/*   pos = 0~7, 0~15, 0~31              */
/*   0x1234 = 0001 0010 0011 0100       */
/*   UTL_TEST_BITS32( 0x1234, 2 ) ==> 1 */
/*   UTL_TEST_BITS32( 0x1234, 1 ) ==> 0 */
#define UTL_TEST_BITS32(__var__,__pos__)     UTL_TEST_BITS(UI32_T,__var__,__pos__)


/*   Usage: UTL_SET_BITS32( val, 6 )                    */
/*   pos = 0~7, 0~15, 0~31                                  */
/*   val = 0x0100 =               0000 0001 0000 0000       */
/*   UTL_SET_BITS32( val,6 )==>   0000 0001 0100 0000   */
/*   UTL_RESET_BITS32( val,8 )=>  0000 0000 0000 0000   */
#define UTL_SET_BITS32(__var__,__pos__)      (__var__) = UTL_SET_BITS(UI32_T,32,__var__,__pos__)
#define UTL_RESET_BITS32(__var__,__pos__)    (__var__) = UTL_RESET_BITS(UI32_T,32,__var__,__pos__)

/*----------------------------------------------------------*/
/*  16 BIT operation                                        */
/*----------------------------------------------------------*/
#define UTL_TEST_BITS16(__var__,__pos__)     UTL_TEST_BITS(UI16_T,__var__,__pos__)

#define UTL_SET_BITS16(__var__,__pos__)      (__var__) = UTL_SET_BITS(UI16_T,16,__var__,__pos__)
#define UTL_RESET_BITS16(__var__,__pos__)    (__var__) = UTL_RESET_BITS(UI16_T,16,__var__,__pos__)

/*----------------------------------------------------------*/
/*  8 BIT operation                                         */
/*----------------------------------------------------------*/
#define UTL_TEST_BITS8(__var__,__pos__)     UTL_TEST_BITS(UI8_T,__var__,__pos__)

#define UTL_SET_BITS8(__var__,__pos__)      (__var__) = UTL_SET_BITS(UI8_T,8,__var__,__pos__)
#define UTL_RESET_BITS8(__var__,__pos__)    (__var__) = UTL_RESET_BITS(UI8_T,16,__var__,__pos__)


int deviceNodeWriteInt(char *filename, int value, int data_len);
int deviceNodeReadBinary(char *filename, char *buffer, int buf_size, int data_len);
int deviceNodeReadString(char *filename, char *buffer, int buf_size, int data_len);

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F,
    PSU_TYPE_DC_48V_F2B,
    PSU_TYPE_DC_48V_B2F
} psu_type_t;

//psu_type_t get_psu_type(int id, char* modelname, int modelname_len);
int psu_two_complement_to_int(uint16_t data, uint8_t valid_bit, int mask);

/* FAN related data
 */
enum onlp_fan_id
{
    FAN_RESERVED = 0,
    FAN_1,
    FAN_2,
    FAN_3,
    FAN_4,
    FAN_5,
    FAN_6,
    FAN_1_ON_PSU1,
    FAN_1_ON_PSU2,
};

/* 
 * LED ID (need to sync with "enum onlp_led_id" defined in ledi.c)
 */
enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_SERVICE,
    LED_STACKING,
    LED_PWR,
    LED_FAN1,
    LED_FAN2,
    LED_FAN3,
    LED_FAN4,
    LED_FAN5,
    LED_FAN6,
    LED_REAR_FAN1,
    LED_REAR_FAN2,
    LED_REAR_FAN3,
    LED_REAR_FAN4,
    LED_REAR_FAN5,
    LED_REAR_FAN6,
};

/*
* TLV type code
*/
#define TLV_CODE_PRODUCT_NAME   0x21
#define TLV_CODE_PART_NUMBER    0x22
#define TLV_CODE_SERIAL_NUMBER  0x23
#define TLV_CODE_MAC_BASE       0x24
#define TLV_CODE_MANUF_DATE     0x25
#define TLV_CODE_DEVICE_VERSION 0x26
#define TLV_CODE_LABEL_REVISION 0x27
#define TLV_CODE_PLATFORM_NAME  0x28
#define TLV_CODE_ONIE_VERSION   0x29
#define TLV_CODE_MAC_SIZE       0x2A
#define TLV_CODE_MANUF_NAME     0x2B
#define TLV_CODE_MANUF_COUNTRY  0x2C
#define TLV_CODE_VENDOR_NAME    0x2D
#define TLV_CODE_DIAG_VERSION   0x2E
#define TLV_CODE_SERVICE_TAG    0x2F
#define TLV_CODE_VENDOR_EXT     0xFD
#define TLV_CODE_CRC_32         0xFE 


/*
i2c APIs: access i2c device by ioctl 
*/ 
#include <errno.h>
#include <linux/i2c-dev.h>

int i2c_read(int i2cbus, int addr, int offset, int length, char* data);
int i2c_sequential_read(int i2cbus, int addr, int offset, int length, char* data);
int i2c_read_byte(int i2cbus, int addr, int offset, char* data);
int i2c_read_word(int i2cbus, int addr, int command);
int i2c_read_block(int i2cbus, int addr, uint8_t offset, uint8_t *data, int length);
int i2c_read_i2c_block_dump(int i2cbus, int addr, uint8_t *data);
int i2c_write_byte(int i2cbus, int addr, int offset, char val); 
int i2c_write_bit(int i2cbus, int addr, int offset, int bit, char val); 
int i2c_read_rps_status(int i2cbus, int addr, int offset);

#define DIAG_FLAG_ON 1
#define DIAG_FLAG_OFF 0
char diag_flag_set(char d);
char diag_flag_get(void);

char diag_debug_trace_on(void);
char diag_debug_trace_off(void);
char diag_debug_trace_check(void);

char diag_debug_pause_platform_manage_on(void);
char diag_debug_pause_platform_manage_off(void);
char diag_debug_pause_platform_manage_check(void);
/*
* TLV parsering for specific type code
*/
int eeprom_tlv_read(uint8_t *rdata, char type, char *data);


#define DIAG_TRACE(fmt,args...) if(diag_debug_trace_check()) printf("\n[TRACE]"fmt"\n", args)
#define DIAG_PRINT(fmt,args...) DIAG_TRACE(fmt,args);else if(diag_flag_get()) printf("[DIAG]"fmt"\n", args) 

char* sfp_control_to_str(int value);

#endif  /* __PLATFORM_LIB_H__ */
