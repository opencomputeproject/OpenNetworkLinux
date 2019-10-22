/************************************************************
 * platform_lib.c
 *
 *           Copyright 2018 Inventec Corporation.
 *
 ************************************************************
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
#include <onlp/onlp.h>
#include "platform_lib.h"

#define PSU_NODE_MAX_PATH_LEN 64

int onlp_file_read_binary(char *filename, char *buffer, int buf_size, int data_len)
{
    if ((buffer == NULL) || (buf_size < 0)) {
        return -1;
    }

    return onlp_file_read((uint8_t*)buffer, buf_size, &data_len, "%s", filename);
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

#define I2C_PSU_MODEL_NAME_LEN 32
#define I2C_PSU_FAN_DIR_LEN    8
#include <ctype.h>

int psu_pmbus_info_get(int id, char *node, int *value)
{
    int  ret = 0;
    *value = 0;

    if (PSU1_ID == id) {
        ret = onlp_file_read_int(value, "%s%s%d", PSU1_AC_PMBUS_PREFIX, node, PSU1_ID);
    } else if (PSU2_ID == id) {
        ret = onlp_file_read_int(value, "%s%s%d", PSU2_AC_PMBUS_PREFIX, node, PSU2_ID);
    } else {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ret;
}

int psu_pmbus_info_set(int id, char *node, int value)
{
    char path[PSU_NODE_MAX_PATH_LEN] = {0};

    switch (id) {
    case PSU1_ID:
        sprintf(path, "%s%s%d", PSU1_AC_PMBUS_PREFIX, node, PSU1_ID);
        break;
    case PSU2_ID:
        sprintf(path, "%s%s%d", PSU2_AC_PMBUS_PREFIX, node, PSU2_ID);
        break;
    default:
        return ONLP_STATUS_E_UNSUPPORTED;
    };

    if (onlp_file_write_int(value, path, NULL) != 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
