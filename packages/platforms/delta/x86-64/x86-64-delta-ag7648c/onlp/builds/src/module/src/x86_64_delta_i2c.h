/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *  Copyright 2018 Delta Technology Corporation.
 *  Copyright 2018 Delta Networks, Inc
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

#include "x86_64_delta_ag7648c_log.h"

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


extern int i2c_devname_read_block (char *name, int reg, uint8_t*buff, int buff_size);
//extern int i2c_devname_write_block(char *name, int reg, char *buff, int buff_size);

#endif 
