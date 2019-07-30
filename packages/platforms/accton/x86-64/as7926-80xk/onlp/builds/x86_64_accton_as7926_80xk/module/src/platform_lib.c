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
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <AIM/aim.h>
#include "platform_lib.h"
#include <linux/i2c-devices.h>
#include <unistd.h>
#include <fcntl.h>

#define PSU_MODEL_NAME_LEN    17
#define PSU_SERIAL_NUMBER_LEN    18
#define PSU_NODE_MAX_PATH_LEN    64
#define PSU_FAN_DIR_LEN    3

int onlp_file_read_binary(char *filename, char *buffer, int buf_size, int data_len)
{
    int fd;
    int len;

    if ((buffer == NULL) || (buf_size < 0)) {
        return -1;
    }

    if ((fd = open(filename, O_RDONLY)) == -1) {
        return -1;
    }

    if ((len = read(fd, buffer, buf_size)) < 0) {
        close(fd);
        return -1;
    }

    if ((close(fd) == -1)) {
        return -1;
    }

    if ((len > buf_size) || (data_len != 0 && len != data_len)) {
        return -1;
    }

    return 0;
}

int onlp_file_read_string(char *filename, char *buffer, int buf_size, int data_len)
{
    int ret;

    if (data_len >= buf_size) {
	    return -1;
	}

	ret = onlp_file_read_binary(filename, buffer, buf_size-1, data_len);

    if (ret == 0) {
        buffer[buf_size-1] = '\0';
    }

    return ret;
}

int psu_serial_number_get(int id, char *serial, int serial_len)
{
	int   size = 0;
	int   ret  = ONLP_STATUS_OK; 
	char *prefix = NULL;

	if (serial == NULL || serial_len < PSU_SERIAL_NUMBER_LEN) {
		return ONLP_STATUS_E_PARAM;
	}

    switch(id)
    {
        case PSU1_ID: prefix = PSU1_AC_PMBUS_PREFIX;
                      break;
        case PSU2_ID: prefix = PSU2_AC_PMBUS_PREFIX;
                      break;
        case PSU3_ID: prefix = PSU3_AC_PMBUS_PREFIX;
                      break;
        case PSU4_ID: prefix = PSU4_AC_PMBUS_PREFIX;
                      break;
        default: break;                
    }

	ret = onlp_file_read((uint8_t*)serial, PSU_SERIAL_NUMBER_LEN, &size, "%s%s", prefix, "psu_mfr_serial");
    if (ret != ONLP_STATUS_OK || size != PSU_SERIAL_NUMBER_LEN) {
		return ONLP_STATUS_E_INTERNAL;

    }

	serial[PSU_SERIAL_NUMBER_LEN] = '\0';
	return ONLP_STATUS_OK;
}



psu_type_t psu_type_get(int id, char* modelname, int modelname_len)
{
    char *node = NULL;
    char  model_name[PSU_MODEL_NAME_LEN + 1] = {0};
    char  fan_dir[PSU_FAN_DIR_LEN + 1] = {0};
    

    /* Check AC model name */
    switch(id)
    {
        case PSU1_ID: node = PSU1_AC_PMBUS_NODE(psu_mfr_model);
                      break;
        case PSU2_ID: node = PSU2_AC_PMBUS_NODE(psu_mfr_model);
                      break;
        case PSU3_ID: node = PSU3_AC_PMBUS_NODE(psu_mfr_model);
                      break;
        case PSU4_ID: node = PSU4_AC_PMBUS_NODE(psu_mfr_model);
                      break;
        default: break;                
    }
    
    if (onlp_file_read_string(node, model_name, sizeof(model_name), 0) != 0) {
        return PSU_TYPE_UNKNOWN;
    }
	
    if ((strncmp(model_name, "PTT1600", strlen("PTT1600")) != 0) &&
        (strncmp(model_name, "FSJ001", strlen("FSJ001")) != 0)) {
        return PSU_TYPE_UNKNOWN;
    }

    if (modelname) {
        aim_strlcpy(modelname, model_name, PSU_MODEL_NAME_LEN - 1);
    }

    switch(id)
    {
        case PSU1_ID: node = PSU1_AC_PMBUS_NODE(psu_fan_dir);
                      break;
        case PSU2_ID: node = PSU2_AC_PMBUS_NODE(psu_fan_dir);
                      break;
        case PSU3_ID: node = PSU3_AC_PMBUS_NODE(psu_fan_dir);
                      break;
        case PSU4_ID: node = PSU4_AC_PMBUS_NODE(psu_fan_dir);
                      break;
        default: break;                
    }
    
    if (onlp_file_read_string(node, fan_dir, sizeof(fan_dir), 0) != 0) {
        return PSU_TYPE_UNKNOWN;
    }

    if (strncmp(model_name, "PTT1600", strlen("PTT1600")) == 0) {
        if ((strncmp(fan_dir, "B2F", strlen("B2F")) == 0) ||
            (strncmp(fan_dir, "AFI", strlen("AFI")) == 0)) {
                return PSU_TYPE_AC_B2F;
        }
        else if ((strncmp(fan_dir, "F2B", strlen("F2B")) == 0) ||
                (strncmp(fan_dir, "AFO", strlen("AFO")) == 0)) {
                return PSU_TYPE_AC_F2B;
         }
    }
    
    return PSU_TYPE_AC_F2B;
}

int psu_ym2651y_pmbus_info_get(int id, char *node, int *value)
{
	char *prefix = NULL;
    *value = 0;

    switch(id)
    {
        case PSU1_ID: prefix = PSU1_AC_PMBUS_PREFIX;
                      break;
        case PSU2_ID: prefix = PSU2_AC_PMBUS_PREFIX;
                      break;
        case PSU3_ID: prefix = PSU3_AC_PMBUS_PREFIX;
                      break;
        case PSU4_ID: prefix = PSU4_AC_PMBUS_PREFIX;
                      break;
        default: break;                
    }
    if (onlp_file_read_int(value, "%s%s", prefix, node) < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s%s)\r\n", prefix, node);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int psu_ym2651y_pmbus_info_set(int id, char *node, int value)
{
	char *prefix = NULL;

    switch(id)
    {
        case PSU1_ID: prefix = PSU1_AC_PMBUS_PREFIX;
                      break;
        case PSU2_ID: prefix = PSU2_AC_PMBUS_PREFIX;
                      break;
        case PSU3_ID: prefix = PSU3_AC_PMBUS_PREFIX;
                      break;
        case PSU4_ID: prefix = PSU4_AC_PMBUS_PREFIX;
                      break;
        default: break;                
    }
    if (onlp_file_write_int(value, "%s%s", prefix, node) < 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s%s)\r\n", prefix, node);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int sfpi_i2c_read(int bus, uint8_t addr, uint8_t offset, int size,
              uint8_t* rdata, uint32_t flags)
{
    int i;
    int fd;

    fd = onlp_i2c_open(bus, addr, flags);

    if(fd < 0) {
       return fd;
    }

    for(i = 0; i < size; i++) {
        int rv = -1;
        int retries = (flags & ONLP_I2C_F_DISABLE_READ_RETRIES) ? 1: ONLPLIB_CONFIG_I2C_READ_RETRY_COUNT;

        while(retries-- && rv < 0) {
            rv = i2c_smbus_read_byte_data(fd, offset+i);
        }

        if(rv < 0) {
            goto error;
        }
        else {
            rdata[i] = rv;
        }
    }
    close(fd);
    return 0;

 error:
    close(fd);
    return ONLP_STATUS_E_I2C;
}

int sfpi_i2c_readb(int bus, uint8_t addr, uint8_t offset, uint32_t flags)
{
    uint8_t byte;
    int rv = sfpi_i2c_read(bus, addr, offset, 1, &byte, flags);
    return (rv < 0) ? rv : byte;
}
