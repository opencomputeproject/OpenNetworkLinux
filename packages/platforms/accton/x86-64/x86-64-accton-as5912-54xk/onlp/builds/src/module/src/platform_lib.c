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
#include <unistd.h>
#include <fcntl.h>
#include <onlplib/file.h>
#include <onlp/onlp.h>
#include "platform_lib.h"

#define PSU_NODE_MAX_PATH_LEN 64
#define PSU_MODEL_NAME_LEN 9
#define PSU_FAN_DIR_LEN    3

psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
    int   ret = 0, value = 0;
    char  model[PSU_MODEL_NAME_LEN + 1] = {0};
    char  fan_dir[PSU_FAN_DIR_LEN + 1] = {0};
    char *path = NULL;

    if (modelname && modelname_len < PSU_MODEL_NAME_LEN) {
        return PSU_TYPE_UNKNOWN;
    }

    /* Check AC model name */
    path = (id == PSU1_ID) ? PSU1_AC_EEPROM_NODE(psu_model_name) : PSU2_AC_EEPROM_NODE(psu_model_name);
    ret = onlp_file_read((uint8_t*)model, PSU_MODEL_NAME_LEN, &value, path);
    if (ret != ONLP_STATUS_OK || value != PSU_MODEL_NAME_LEN) {
		return PSU_TYPE_UNKNOWN;

    }
	
    if (strncmp(model, "YM-2651Y", strlen("YM-2651Y")) != 0) {
        return PSU_TYPE_UNKNOWN;
    }

    if (modelname) {
        strncpy(modelname, model, modelname_len-1);
    }

    path = (id == PSU1_ID) ? PSU1_AC_PMBUS_NODE(psu_fan_dir) : PSU2_AC_PMBUS_NODE(psu_fan_dir);
    ret = onlp_file_read((uint8_t*)fan_dir, sizeof(fan_dir), &value, path);
    if (ret != ONLP_STATUS_OK) {
        return PSU_TYPE_UNKNOWN;
    }

    if (strncmp(fan_dir, "F2B", strlen("F2B")) == 0) {
        return PSU_TYPE_AC_F2B;
    }

    if (strncmp(fan_dir, "B2F", strlen("B2F")) == 0) {
        return PSU_TYPE_AC_B2F;
    }

    return PSU_TYPE_UNKNOWN;
}

int psu_ym2651y_pmbus_info_get(int id, char *node, int *value)
{
    int  ret = 0;
    char path[PSU_NODE_MAX_PATH_LEN] = {0};
    
    *value = 0;

    if (PSU1_ID == id) {
        ret = onlp_file_read_int(value, "%s%s", PSU1_AC_PMBUS_PREFIX, node);
    }
    else {
		ret = onlp_file_read_int(value, "%s%s", PSU2_AC_PMBUS_PREFIX, node);
    }

    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ret;
}

int psu_ym2651y_pmbus_info_set(int id, char *node, int value)
{
    char path[PSU_NODE_MAX_PATH_LEN] = {0};

	switch (id) {
	case PSU1_ID:
		sprintf(path, "%s%s", PSU1_AC_PMBUS_PREFIX, node);
		break;
	case PSU2_ID:
		sprintf(path, "%s%s", PSU2_AC_PMBUS_PREFIX, node);
		break;
	default:
		return ONLP_STATUS_E_UNSUPPORTED;
	};

    if (onlp_file_write_int(value, path) < 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

