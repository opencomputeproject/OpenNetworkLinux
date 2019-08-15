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

int cpld_read(unsigned int regOffset, unsigned char *val);

int cpld_write(unsigned int regOffset, unsigned char val);

int i2c_write(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned char *buf);

int i2c_read(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned char *buf);

int i2c_nRead(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned int size, unsigned char * buf);

int i2c_nWrite(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned int size, unsigned char * buf);
int i2c_nWriteForce(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned int size, unsigned char * buf, int force);

int two_complement_to_int(unsigned short data, unsigned char valid_bit, unsigned short mask);

int pmbus_parse_literal_format(unsigned short val);

int pmbus_read_literal_data(unsigned char bus, unsigned char i2c_addr, unsigned char reg, int *val);

int pmbus_parse_vout_format(unsigned char vout_mode, unsigned short val);

int pmbus_read_vout_data(unsigned char bus, unsigned char i2c_addr, int *val);

int int_to_pmbus_linear(int val);

int as5610_52x_i2c0_pca9548_channel_set(unsigned char channel);

typedef enum as5610_52x_psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F,
    PSU_TYPE_DC_48V_F2B,
    PSU_TYPE_DC_48V_B2F
} as5610_52x_psu_type_t;

as5610_52x_psu_type_t as5610_52x_get_psu_type(int id, char* modelname, int size);

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    NE1617A_LOCAL_SENSOR,
    NE1617A_REMOTE_SENSOR,
    MAX6581_LOCAL_SENSOR,
    MAX6581_REMOTE_SENSOR_1,
    MAX6581_REMOTE_SENSOR_2,
    MAX6581_REMOTE_SENSOR_3,
    MAX6581_REMOTE_SENSOR_4,
    MAX6581_REMOTE_SENSOR_5,
    MAX6581_REMOTE_SENSOR_6,
    MAX6581_REMOTE_SENSOR_7,
    BCM56846_LOCAL_SENSOR,
    PSU1_THERMAL_SENSOR_1,
    PSU2_THERMAL_SENSOR_1,
    NUM_OF_CHASSIS_THERMAL_SENSOR = BCM56846_LOCAL_SENSOR,
};

enum onlp_fan_duty_cycle_percentage
{
    FAN_PERCENTAGE_MIN = 40,
    FAN_PERCENTAGE_MID = 70,
    FAN_PERCENTAGE_MAX = 100
};

#define TEMPERATURE_MULTIPLIER 1000

#endif  /* __PLATFORM_LIB_H__ */
