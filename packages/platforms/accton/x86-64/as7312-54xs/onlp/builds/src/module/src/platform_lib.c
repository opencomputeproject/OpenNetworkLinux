#include <unistd.h>
#include <fcntl.h>
#include "platform_lib.h"
#include <onlp/platformi/sfpi.h>
#include "x86_64_accton_as7312_54xs_log.h"


static int _onlp_file_write(char *filename, char *buffer, int buf_size, int data_len)
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

int onlp_file_write_integer(char *filename, int value)
{
    char buf[8] = {0};
    sprintf(buf, "%d", value);

    return _onlp_file_write(filename, buf, (int)strlen(buf), 0);
}

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

#define I2C_PSU_MODEL_NAME_LEN 9
#define I2C_PSU_FAN_DIR_LEN    3

psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
    char *node = NULL;
    char  model_name[I2C_PSU_MODEL_NAME_LEN + 1] = {0};
    char  fan_dir[I2C_PSU_FAN_DIR_LEN + 1] = {0};
    

    /* Check AC model name */
    node = (id == PSU1_ID) ? PSU1_AC_PMBUS_NODE(psu_mfr_model) : PSU2_AC_PMBUS_NODE(psu_mfr_model);
    if (onlp_file_read_string(node, model_name, sizeof(model_name), 0) != 0) {
        return PSU_TYPE_UNKNOWN;
    }
	
    if (strncmp(model_name, "YM-2651Y", strlen("YM-2651Y")) != 0) {
        return PSU_TYPE_UNKNOWN;
    }

    if (modelname) {
        strncpy(modelname, model_name, modelname_len-1);
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

    return PSU_TYPE_UNKNOWN;
}

int psu_ym2651y_pmbus_info_get(int id, char *node, int *value)
{
    int  ret = 0;
    char path[PSU_NODE_MAX_PATH_LEN] = {0};
    
    *value = 0;

    if (PSU1_ID == id) {
        sprintf(path, "%s%s", PSU1_AC_PMBUS_PREFIX, node);
    }
    else {
        sprintf(path, "%s%s", PSU2_AC_PMBUS_PREFIX, node);
    }

    if (onlp_file_read_int(value, path) < 0) {
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

    if (onlp_file_write_integer(path, value) < 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
