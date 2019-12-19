/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 MiTAC Computing Technology Corporation.
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
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include "platform_lib.h"
#include <ctype.h>

#define I2C_PSU_MODEL_NAME_LEN 20
#define I2C_PSU_FAN_DIRCTION_LEN 8
#define HEX_STR_TO_INT_VALUE_MAX_STR_LEN 32

int psu_pmbus_info_get(int id, char *node, int *value)
{
    int  ret = ONLP_STATUS_OK;
    char path[SYSFS_NODE_PATH_MAX_LEN] = {0};

    *value = 0;

    if (PSU1_ID == id) {
        sprintf(path, "%s%s", PSU1_PMBUS_PREFIX, node);
    } else {
        sprintf(path, "%s%s", PSU2_PMBUS_PREFIX, node);
    }

    if (onlp_file_read_int(value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ret;
}

int psu_pmbus_info_set(int id, char *node, int value)
{
    char path[SYSFS_NODE_PATH_MAX_LEN] = {0};

    switch (id) 
    {
        case PSU1_ID:
            sprintf(path, "%s%s", PSU1_PMBUS_PREFIX, node);
            break;
        case PSU2_ID:
            sprintf(path, "%s%s", PSU2_PMBUS_PREFIX, node);
            break;
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (onlp_file_write_int(value, path) < 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
    char *node = NULL;
    char  model_name[I2C_PSU_MODEL_NAME_LEN + 1] = {0};
    char  fan_direction[I2C_PSU_FAN_DIRCTION_LEN] = {0};
    int   len=0;

    /* Check AC model name */
    node = (id == PSU1_ID) ? PSU1_PMBUS_NODE(mfr_model) : PSU2_PMBUS_NODE(mfr_model);

    if (onlp_file_read((uint8_t *)model_name, sizeof(model_name), &len, node) != 0) {
        AIM_LOG_ERROR("Unable to read PSU(%d) model name\r\n", id);
        return PSU_TYPE_UNKNOWN;
    }

    if(isspace(model_name[strlen(model_name)-1])) {
        model_name[strlen(model_name)-1] = 0;
    }

    if (modelname) {
        strncpy(modelname, model_name, modelname_len-1);
    }

    /* Get fan direction */
    node = (id == PSU1_ID) ? PSU1_PMBUS_NODE(mfr_fan) : PSU2_PMBUS_NODE(mfr_fan);

    if (onlp_file_read((uint8_t *)fan_direction, sizeof(fan_direction), &len, node) != 0) {
        AIM_LOG_ERROR("Unable to read PSU(%d) fan direction\r\n", id);
        return PSU_TYPE_UNKNOWN;
    }

    if (memcmp(model_name, "YM-2651Y", strlen("YM-2651Y")) == 0) {
        /* AC Type */
        if (memcmp(fan_direction, "F2B", strlen("F2B")) == 0)
            return PSU_TYPE_AC_F2B;
        if (memcmp(fan_direction, "B2F", strlen("B2F")) == 0)
            return PSU_TYPE_AC_B2F;
        return PSU_TYPE_UNKNOWN;
    } else if (memcmp(model_name, "YM-2651W", strlen("YM-2651W")) == 0) {
        /* DC Type */
        if (memcmp(fan_direction, "F2B", strlen("F2B")) == 0)
            return PSU_TYPE_DC_F2B;
        if (memcmp(fan_direction, "B2F", strlen("B2F")) == 0)
            return PSU_TYPE_DC_B2F;
        return PSU_TYPE_UNKNOWN;
    } else {
        return PSU_TYPE_UNKNOWN;
    }

    return PSU_TYPE_UNKNOWN;
}

/***********************************************************************
 *  Read data from file and transfer content from "0x" format to integer.
 ***********************************************************************/
int onlp_file_read_int_hex(int* value, const char* node_path)
{
    int len = 0;
    long int tmp_v = 0;
    char *pEnd;
    char buffer[HEX_STR_TO_INT_VALUE_MAX_STR_LEN] = {0};

    if (onlp_file_read((uint8_t *)buffer, sizeof(buffer), &len, node_path) != 0) {
        DEBUG_PRINT("%s(%d)\r\n", __FUNCTION__, __LINE__);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Transfer from string "0x1" to int "1" */
    tmp_v = strtol (buffer,&pEnd,16);
    *value = (int)tmp_v;

    return ONLP_STATUS_OK;
}
