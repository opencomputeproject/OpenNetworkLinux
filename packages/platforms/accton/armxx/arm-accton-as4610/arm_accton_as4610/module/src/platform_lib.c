/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
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
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
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

    if (data_len >= buf_size) {
	    return -1;
	}

	ret = deviceNodeReadBinary(filename, buffer, buf_size-1, data_len);

    if (ret == 0) {
        buffer[buf_size-1] = '\0';
    }

    return ret;
}

#define I2C_PSU_MODEL_NAME_LEN 8

psu_type_t get_psu_type(int id, char *data_buf, int data_len)
{
    int   len    = 0;
    char *path[] = { PSU1_AC_EEPROM_PREFIX, PSU2_AC_EEPROM_PREFIX };
    char *str = NULL;

    /* Read attribute */
    len = onlp_file_read_str(&str, "%s%s", path[id-1], "psu_model_name");
    if (!str || len <= 0) {
        AIM_FREE_IF_PTR(str);
        return PSU_TYPE_UNKNOWN;
    }

    if (len > data_len) {
        AIM_FREE_IF_PTR(str);
        return PSU_TYPE_UNKNOWN;
    }

    aim_strlcpy(data_buf, str, len+1);
    AIM_FREE_IF_PTR(str);

    /* Check AC model name */
    if (strncmp(data_buf, "YM-1921A", strlen("YM-1921A")) == 0 ||
        strncmp(data_buf, "YM-1151D", strlen("YM-1151D")) == 0 ||
        strncmp(data_buf, "YM-1601A", strlen("YM-1601A")) == 0 ||
        strncmp(data_buf, "YM-1151F", strlen("YM-1151F")) == 0 ||
        strncmp(data_buf, "DPS-920AB B", strlen("DPS-920AB B")) == 0) {

<<<<<<< HEAD
        return PSU_TYPE_AC_F2B;
=======
    if (deviceNodeReadString(node, model_name, sizeof(model_name), 0) == 0) {
        if (strncmp(model_name, "YM-1921A", strlen("YM-1921A")) == 0 ||
            strncmp(model_name, "YM-1151D", strlen("YM-1151D")) == 0 ||
            strncmp(model_name, "YM-1151F", strlen("YM-1151F")) == 0 ||
            strncmp(model_name, "YM-1601A", strlen("YM-1601A")) == 0) {
            if (modelname) {
                aim_strlcpy(modelname, model_name, modelname_len-1);
            }
            return PSU_TYPE_AC_F2B;
        }
>>>>>>> 5b0f6e97976d5a4de1a7dd9e6f040bce47d1eba6
    }

    return PSU_TYPE_UNKNOWN;
}

int get_psu_serial(int id, char *data_buf, int data_len)
{
    int   len    = 0;
    char *path[] = { PSU1_AC_EEPROM_PREFIX, PSU2_AC_EEPROM_PREFIX };
    char *str = NULL;

    /* Read attribute */
    len = onlp_file_read_str(&str, "%s%s", path[id-1], "psu_serial_number");
    if (!str || len <= 0) {
        AIM_FREE_IF_PTR(str);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (len > data_len) {
        AIM_FREE_IF_PTR(str);
        return ONLP_STATUS_E_INVALID;
    }

    aim_strlcpy(data_buf, str, len+1);
    AIM_FREE_IF_PTR(str);
    return ONLP_STATUS_OK;
}

enum as4610_product_id get_product_id(void)
{
    char *node = "/sys/bus/i2c/devices/0-0030/product_id";
    char  buf[3] = {0};
    int   pid = PID_UNKNOWN;

    if (deviceNodeReadString(node, buf, sizeof(buf), 0) != 0) {
        return PID_UNKNOWN;
    }

    pid = atoi(buf);
    if (pid >= PID_UNKNOWN || pid < PID_AS4610_30T || pid == PID_RESERVED) {
        return PID_UNKNOWN;
    }

    return pid;
}

int chassis_fan_count(void)
{
    enum as4610_product_id pid = get_product_id();

	if (pid == PID_AS4610T_B) {
		return 2;
	}

    return (pid == PID_AS4610_30P || pid == PID_AS4610P) ? 1 : 0;
}

int chassis_led_count(void)
{
    enum as4610_product_id pid = get_product_id();

	if (pid == PID_AS4610T_B) {
		return 8;
	}
	
    return (pid == PID_AS4610_30P || pid == PID_AS4610P) ? 12 : 10;
}
