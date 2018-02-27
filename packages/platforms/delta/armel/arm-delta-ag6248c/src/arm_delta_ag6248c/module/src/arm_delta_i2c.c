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
 
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "arm_delta_ag6248c_log.h"
#include "arm_delta_i2c.h"
#include <onlplib/i2c.h>

struct i2c_device_info i2c_device_list[]={
	{"RTC",0X0,0X68},
	{"TMP1_CLOSE_TO_MAC",0X0,0X49},
 	{"TMP2_CLOSE_TO_PHY",0X0,0X4a},
	{"CPLD",0X0,0X28},
 	{"FAN_ON_BOARD",0X1,0X2C},
	{"CURT_MONTOR",0X1,0X40},
    {"SFP1",0X2,0X50},
    {"SFP2",0X3,0X50},
//	-------------------------
    {"PSU1_PMBUS",0X4,0X58},
    {"PSU2_PMBUS",0X5,0X59},
	{"PSU1_EEPROM",0X4,0X50},
	{"PSU2_EEPROM",0X5,0X51},
//	-------------------------
	{"PSU1_PMBUS_POE",0X4,0X58},
    {"PSU2_PMBUS_POE",0X5,0X58},
	{"PSU1_EEPROM_POE",0X4,0X52},
	{"PSU2_EEPROM_POE",0X5,0X52},
    {NULL,  -1,-1},
};


uint32_t i2c_flag=ONLP_I2C_F_FORCE; 


i2c_device_info_t *i2c_dev_find_by_name (char *name)
{
	i2c_device_info_t *i2c_dev = i2c_device_list;

	if (name == NULL) return NULL;

	while (i2c_dev->name) {
		if (strcmp (name, i2c_dev->name) == 0) break;
		++ i2c_dev;
	}
	if (i2c_dev->name == NULL) return NULL;

	return i2c_dev;
}

int i2c_devname_read_byte  (char *name, int reg)
{	
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);

	if(i2c_dev==NULL) return -1;
	
	ret=onlp_i2c_readb (i2c_dev->i2cbus, i2c_dev->addr, reg, i2c_flag);	

	return ret;
}

int i2c_devname_write_byte (char *name, int reg, int value)
{
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);
	
	 if(i2c_dev==NULL) return -1;
	
	ret=onlp_i2c_writeb (i2c_dev->i2cbus, i2c_dev->addr, reg, value, i2c_flag);


	return ret;
}

int i2c_devname_read_word  (char *name, int reg)
{	
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);

	if(i2c_dev==NULL) return -1;
	
	ret=onlp_i2c_readw (i2c_dev->i2cbus, i2c_dev->addr, reg,  i2c_flag);	

	return ret;
}

int i2c_devname_write_word (char *name, int reg, int value)
{
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);
	
	if(i2c_dev==NULL) return -1;
	
	ret=onlp_i2c_writew (i2c_dev->i2cbus, i2c_dev->addr, reg, value, i2c_flag);
	
    return ret;
}

int i2c_devname_read_block (char *name, int reg, uint8_t *buff, int buff_size)
{	
	int ret = -1;
	
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);
		
	if(i2c_dev==NULL) return -1;

	ret = onlp_i2c_block_read (i2c_dev->i2cbus, i2c_dev->addr, reg, buff_size, buff, i2c_flag);

	return ret;

}

