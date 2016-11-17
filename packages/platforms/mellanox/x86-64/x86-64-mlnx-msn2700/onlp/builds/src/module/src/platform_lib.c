/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include <onlplib/file.h>
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

    return deviceNodeWrite(filename, buf, (int)strlen(buf), data_len);
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
        buffer[strnlen(buffer, buf_size)-1] = '\0';
    }

    return ret;
}

int
psu_read_eeprom(int psu_index, onlp_psu_info_t* psu_info, onlp_fan_info_t* fan_info)
{
    char path[64] = {0};
    const char sanity_check[]   = "MLNX";
    const uint8_t serial_len    = 24;
    char data[256] = {0};
    bool sanity_found = false;
    int index = 0;
    int rv  = 0;
    int len = 0;

    snprintf(path, sizeof(path), IDPROM_PATH, "psu", psu_index);
    rv = onlp_file_read((uint8_t* )data, sizeof(data)-1, &len, path);
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Looking for sanity checker */
    while (index < sizeof(data) - sizeof(sanity_check) - 1) {
        if (!strncmp(&data[index], sanity_check, sizeof(sanity_check) - 1)) {
            sanity_found = true;
            break;
        }
        index++;
    }
    if (false == sanity_found) {
        return ONLP_STATUS_E_INVALID;
    }

    /* Serial number */
    index += strlen(sanity_check);
    if (psu_info) {
        strncpy(psu_info->serial, &data[index], sizeof(psu_info->serial));
    } else if (fan_info) {
        strncpy(fan_info->serial, &data[index], sizeof(fan_info->serial));
    }

    /* Part number */
    index += serial_len;
    if (psu_info) {
        strncpy(psu_info->model, &data[index], sizeof(psu_info->model));
    } else if (fan_info) {
        strncpy(fan_info->model, &data[index], sizeof(fan_info->model));
    }

    return ONLP_STATUS_OK;
}
