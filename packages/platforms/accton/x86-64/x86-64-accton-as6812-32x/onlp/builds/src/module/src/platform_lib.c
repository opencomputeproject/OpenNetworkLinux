/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
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
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <AIM/aim.h>
#include "platform_lib.h"

int deviceNodeWrite(char *filename, char *buffer, int buf_size, int data_len)
{
    int fd;
    int len;

    if ((buffer == NULL) || (buf_size < 0)) {
        return -1;
    }

    if ((fd = open(filename, O_WRONLY, S_IWUSR)) == -1) {
        return -1;
    }

    if ((len = write(fd, buffer, buf_size)) < 0) {
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

int deviceNodeWriteInt(char *filename, int value, int data_len)
{
    char buf[8] = {0};
    sprintf(buf, "%d", value);

    return deviceNodeWrite(filename, buf, sizeof(buf)-1, data_len);
}

int deviceNodeReadBinary(char *filename, char *buffer, int buf_size, int data_len)
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

int deviceNodeReadString(char *filename, char *buffer, int buf_size, int data_len)
{
    int ret;

    if (data_len >= buf_size || data_len < 0) {
	    return -1;
	}

	ret = deviceNodeReadBinary(filename, buffer, buf_size-1, data_len);

    if (ret == 0) {
        if (data_len) {
            buffer[data_len] = '\0';
        }
        else {
            buffer[buf_size-1] = '\0';
        }
    }
    return ret;
}

#define I2C_PSU_MODEL_NAME_LEN 13
#define STRLEN(x) (sizeof(x) - 1)

psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
    char *node = NULL;
    char model_name[I2C_PSU_MODEL_NAME_LEN + 1] = {0};

    /* Check AC model name */
    node = (id == PSU1_ID) ? PSU1_AC_EEPROM_NODE(psu_model_name) : PSU2_AC_EEPROM_NODE(psu_model_name);

    if (deviceNodeReadString(node, model_name, sizeof(model_name), 0) == 0) {
        if (strncmp(model_name, "CPR-4011-4M11", STRLEN("CPR-4011-4M11")) == 0) {
            if (modelname) {
            strncpy(modelname, model_name, sizeof(model_name));
            }
            return PSU_TYPE_AC_COMPUWARE_F2B;
        }
        else if (strncmp(model_name, "CPR-4011-4M21", STRLEN("CPR-4011-4M21")) == 0) {
            if (modelname) {
            strncpy(modelname, model_name, sizeof(model_name));
            }
            return PSU_TYPE_AC_COMPUWARE_B2F;
        }
        else if (strncmp(model_name, "CPR-6011-2M11", STRLEN("CPR-6011-2M11")) == 0) {
            if (modelname) {
                strncpy(modelname, model_name, sizeof(model_name));
            }
            return PSU_TYPE_AC_COMPUWARE_F2B;
        }
        else if (strncmp(model_name, "CPR-6011-2M21", STRLEN("CPR-6011-2M21")) == 0) {
            if (modelname) {
                strncpy(modelname, model_name, sizeof(model_name));
            }
            return PSU_TYPE_AC_COMPUWARE_B2F;
        }
    }

    /* Check 3Y-Power AC model name */
    memset(model_name, 0, sizeof(model_name));
    node = (id == PSU1_ID) ? PSU1_AC_3YPOWER_EEPROM_NODE(psu_model_name) : PSU2_AC_3YPOWER_EEPROM_NODE(psu_model_name);	

    if (deviceNodeReadString(node, model_name, sizeof(model_name), 0) == 0) {
        if (strncmp(model_name, "YM-2401JCR", STRLEN("YM-2401JCR")) == 0) {
            if (modelname) {
            model_name[STRLEN("YM-2401JCR")] = 0;
            strncpy(modelname, model_name, 11);
            }
            return PSU_TYPE_AC_3YPOWER_F2B;
        }
        else if (strncmp(model_name, "YM-2401JDR", STRLEN("YM-2401JDR")) == 0) {
            if (modelname) {
            model_name[STRLEN("YM-2401JDR")] = 0;
            strncpy(modelname, model_name, 11);
            }
            return PSU_TYPE_AC_3YPOWER_B2F;
        }
    }

    /* Check DC model name */
    memset(model_name, 0, sizeof(model_name));
    node = (id == PSU1_ID) ? PSU1_DC_EEPROM_NODE(psu_model_name) : PSU2_DC_EEPROM_NODE(psu_model_name);

    if (deviceNodeReadString(node, model_name, sizeof(model_name), 0) == 0) {
        if (strncmp(model_name, "um400d01G", STRLEN("um400d01G")) == 0) {
            if (modelname) {
            model_name[STRLEN("um400d01G")] = 0;
            strncpy(modelname, model_name, 10);
            }
            return PSU_TYPE_DC_48V_B2F;
        }
        else if (strncmp(model_name, "um400d01-01G", STRLEN("um400d01-01G")) == 0) {
            if (modelname) {
            model_name[STRLEN("um400d01-01G")] = 0;
            strncpy(modelname, model_name, 13);
            }
            return PSU_TYPE_DC_48V_F2B;
        }
    }

    return PSU_TYPE_UNKNOWN;
}

int psu_ym2401_pmbus_info_get(int id, char *node, int *value)
{
    int  ret = 0;
    char path[64] = {0};
    
    *value = 0;

    if (PSU1_ID == id) {
        sprintf(path, "%s%s", PSU1_AC_3YPOWER_PMBUS_PREFIX, node);
    }
    else {
        sprintf(path, "%s%s", PSU2_AC_3YPOWER_PMBUS_PREFIX, node);
    }

    if (onlp_file_read_int(value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ret;
}

int psu_ym2401_pmbus_info_set(int id, char *node, int value)
{
    char path[64] = {0};

    switch (id) {
    case PSU1_ID:
        sprintf(path, "%s%s", PSU1_AC_3YPOWER_PMBUS_PREFIX, node);
        break;
    case PSU2_ID:
        sprintf(path, "%s%s", PSU2_AC_3YPOWER_PMBUS_PREFIX, node);
        break;
    default:
        return ONLP_STATUS_E_UNSUPPORTED;
    };

    if (deviceNodeWriteInt(path, value, 0) < 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

#define PSU_SERIAL_NUMBER_LEN	18

int psu_serial_number_get(int id, int is_ac, char *serial, int serial_len)
{
	int   size = 0;
	int   ret  = ONLP_STATUS_OK; 
	char *prefix = NULL;

	if (serial == NULL || serial_len < PSU_SERIAL_NUMBER_LEN) {
		return ONLP_STATUS_E_PARAM;
	}
    
    memset((void *)serial, 0x0, serial_len);
    if(is_ac)
	    prefix = (id == PSU1_ID) ? PSU1_AC_EEPROM_PREFIX : PSU2_AC_EEPROM_PREFIX;
	else
        prefix = (id == PSU1_ID) ? PSU1_DC_EEPROM_PREFIX : PSU2_DC_EEPROM_PREFIX;
	ret = onlp_file_read((uint8_t*)serial, PSU_SERIAL_NUMBER_LEN, &size, "%s%s", prefix, "psu_serial");
    if (ret != ONLP_STATUS_OK || size != PSU_SERIAL_NUMBER_LEN) {
		return ONLP_STATUS_E_INTERNAL;

    }

	serial[PSU_SERIAL_NUMBER_LEN] = '\0';
	return ONLP_STATUS_OK;
}