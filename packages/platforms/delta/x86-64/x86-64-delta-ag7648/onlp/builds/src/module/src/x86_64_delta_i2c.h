/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc
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
 ************************************************************/
/* the i2c struct header*/

#ifndef __X86_64_DELTA_I2C_H__
#define __X86_64_DELTA_I2C_H__

#include "x86_64_delta_ag7648_log.h"

struct i2c_device_info {
	/*i2c device name*/
	char *name;
	char  i2cbus;
	char  addr;
};


typedef struct i2c_device_info i2c_device_info_t;

extern struct i2c_device_info i2c_device_list[];


extern int i2c_devname_read_byte(char *name, int reg);

extern int i2c_devname_write_byte(char *name, int reg, int value);


extern int i2c_devname_read_word(char *name, int reg);

extern int i2c_devname_write_word(char *name, int reg, int value);


extern int i2c_devname_read_block (char *name, int reg, char *buff, int buff_size);
extern int i2c_devname_write_block(char *name, int reg, char *buff, int buff_size);

#endif 
