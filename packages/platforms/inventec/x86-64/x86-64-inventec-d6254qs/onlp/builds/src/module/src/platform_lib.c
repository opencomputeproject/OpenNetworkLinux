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
psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
    char *node = NULL;
    char  model_name[I2C_PSU_MODEL_NAME_LEN + 1] = {0};
    char  fan_dir[I2C_PSU_FAN_DIR_LEN + 1] = {0};

    /* Check AC model name */
    node = (id == PSU1_ID) ? PSU1_AC_HWMON_NODE(psu_model_name) : PSU2_AC_HWMON_NODE(psu_model_name);

    if (onlp_file_read_string(node, model_name, sizeof(model_name), 0) != 0) {
        return PSU_TYPE_UNKNOWN;
    }

    if(isspace(model_name[strlen(model_name)-1])) {
        model_name[strlen(model_name)] = 0;
    }

    if (strncmp(model_name, "YM-2651Y", 8) == 0) {
	    if (modelname) {
			strncpy(modelname, model_name, 8);
	    }

	    node = (id == PSU1_ID) ? PSU1_AC_PMBUS_NODE(psu_fan_dir) : PSU2_AC_PMBUS_NODE(psu_fan_dir);
	    if (onlp_file_read_string(node, fan_dir, sizeof(fan_dir), 0) != 0) {
	        return PSU_TYPE_UNKNOWN;
	    }

	    if (strncmp(fan_dir, "F2B", strlen("F2B")) == 0) {
	        return PSU_TYPE_AC_F2B;
	    }

	    if (strncmp(fan_dir, "B2F", strlen("B2F")) == 0) {
	        return PSU_TYPE_AC_B2F;
	    }
    }

    if (strncmp(model_name, "YM-2651V", 8) == 0) {
	    if (modelname) {
			strncpy(modelname, model_name, 8);
	    }

	    node = (id == PSU1_ID) ? PSU1_AC_PMBUS_NODE(psu_fan_dir) : PSU2_AC_PMBUS_NODE(psu_fan_dir);
	    if (onlp_file_read_string(node, fan_dir, sizeof(fan_dir), 0) != 0) {
	        return PSU_TYPE_UNKNOWN;
	    }

	    if (strncmp(fan_dir, "F2B", strlen("F2B")) == 0) {
	        return PSU_TYPE_DC_48V_F2B;
	    }

	    if (strncmp(fan_dir, "B2F", strlen("B2F")) == 0) {
	        return PSU_TYPE_DC_48V_B2F;
	    }
    }

	if (strncmp(model_name, "PSU-12V-750", 11) == 0) {
	    if (modelname) {
			strncpy(modelname, model_name, 11);
	    }

	    node = (id == PSU1_ID) ? PSU1_AC_HWMON_NODE(psu_fan_dir) : PSU2_AC_HWMON_NODE(psu_fan_dir);
	    if (onlp_file_read_string(node, fan_dir, sizeof(fan_dir), 0) != 0) {
	        return PSU_TYPE_UNKNOWN;
	    }

	    if (strncmp(fan_dir, "F2B", 3) == 0) {
	        return PSU_TYPE_DC_12V_F2B;
	    }

	    if (strncmp(fan_dir, "B2F", 3) == 0) {
	        return PSU_TYPE_DC_12V_B2F;
	    }

	    if (strncmp(fan_dir, "NON", 3) == 0) {
	        return PSU_TYPE_DC_12V_FANLESS;
	    }
	}

	if (strncmp(model_name, "DPS-150AB-10", 12) == 0) {
	    if (modelname) {
			strncpy(modelname, model_name, 12);
	    }

	    return PSU_TYPE_DC_12V_F2B;
        }

    return PSU_TYPE_UNKNOWN;
}

int psu_pmbus_info_get(int id, char *node, int *value)
{
    int  ret = 0;
    *value = 0;

    if (PSU1_ID == id) {
        ret = onlp_file_read_int(value, "%s%s%d", PSU1_AC_PMBUS_PREFIX, node, PSU1_ID);
    }
    else
    if (PSU2_ID == id) {
        ret = onlp_file_read_int(value, "%s%s%d", PSU2_AC_PMBUS_PREFIX, node, PSU2_ID);
    }
    else {
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
