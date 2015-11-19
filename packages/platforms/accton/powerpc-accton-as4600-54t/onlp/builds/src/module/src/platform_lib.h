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

typedef enum psu_module_type_e {
    PSU_MODULE_TYPE_UNKNOWN = 0,
    PSU_MODULE_TYPE_AC_CHICONY_F2B,
    PSU_MODULE_TYPE_AC_CHICONY_B2F,
    PSU_MODULE_TYPE_DC_UMEC_48V_F2B,
    PSU_MODULE_TYPE_DC_UMEC_48V_B2F
} psu_module_type_t;

int i2c_write(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned char *buf);

int i2c_read(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned char *buf);

int I2C_nRead(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned int size, unsigned char * buf);

int I2C_nWrite(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned int size, unsigned char * buf);

int two_complement_to_int(unsigned short data, unsigned char valid_bit, unsigned short mask);

int parse_literal_format(unsigned short val, int multiplier_value);

int int_to_pmbus_linear(int val);

void crc_calc(char data_in, int *rem);

int as4600_54t_get_psu_type(int pid, char* modelname, int modelname_len);

#endif  /* __PLATFORM_LIB_H__ */
