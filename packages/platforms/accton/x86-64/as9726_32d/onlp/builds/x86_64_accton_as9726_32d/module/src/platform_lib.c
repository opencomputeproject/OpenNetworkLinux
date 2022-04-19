#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <unistd.h>
#include <fcntl.h>
#include "platform_lib.h"
#include <onlp/platformi/sfpi.h>
#include "x86_64_accton_as9726_32d_log.h"

#define I2C_PSU_MODEL_NAME_LEN 15
#define I2C_PSU_FAN_DIR_LEN    3

#define AIM_FREE_IF_PTR(p) \
do \
{ \
	if (p) { \
        	aim_free(p); \
        	p = NULL; \
	} \
} while (0)

psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
	int ret = 0;
	char *node = NULL;
	char *mn = NULL;

	/* Check AC model name */
	node = (id == PSU1_ID) ? PSU1_AC_PMBUS_NODE(psu_mfr_model)
			       : PSU2_AC_PMBUS_NODE(psu_mfr_model);

	ret = onlp_file_read_str(&mn, node);

	if (ret <= 0 || ret > I2C_PSU_MODEL_NAME_LEN || mn == NULL) {
		AIM_FREE_IF_PTR(mn);
		return PSU_TYPE_UNKNOWN;
	}

	if (!strncmp(mn, "FSH082", strlen("FSH082"))) {
		if (modelname)
			aim_strlcpy(modelname, mn, strlen("FSH082") < 
				(modelname_len-1) ? (strlen("FSH082")+1) : 
				(modelname_len-1));
	
		return PSU_TYPE_ACBEL;
	}

	if (!strncmp(mn, "YESM1300AM", strlen("YESM1300AM"))) {
		if (modelname)
			aim_strlcpy(modelname, mn, 
				strlen("YESM1300AM") <
				(modelname_len-1) ? (strlen("YESM1300AM")+1) : 
				(modelname_len-1));

		return PSU_TYPE_YESM1300;
	}

	if (!strncmp(mn, "YM-2651Y", strlen("YM-2651Y"))) {
		if (modelname)
			aim_strlcpy(modelname, mn, modelname_len-1);

		return PSU_TYPE_YM2651Y;
	}

	if (!strncmp(mn, "FSF019", strlen("FSF019"))) {
		if (modelname)
			aim_strlcpy(modelname, mn, strlen("FSF019") < 
				(modelname_len-1) ? (strlen("FSF019")+1) : 
				(modelname_len-1));
	
		return PSU_TYPE_ACBEL;
	}

	if (!strncmp(mn, "FSJ001", strlen("FSJ001"))) {
		if (modelname)
			aim_strlcpy(modelname, mn, strlen("FSJ001") < 
				(modelname_len-1) ? (strlen("FSJ001")+1) : 
				(modelname_len-1));
	
		return PSU_TYPE_ACBEL;
	}

	return PSU_TYPE_UNKNOWN;
}

int psu_pmbus_info_get(int id, char *node, int *value)
{
	int  ret = 0;
	char path[PSU_NODE_MAX_PATH_LEN] = {0};

	*value = 0;

	if (PSU1_ID == id) {
		sprintf(path, "%s%s", PSU1_AC_PMBUS_PREFIX, node);
	} else {
		sprintf(path, "%s%s", PSU2_AC_PMBUS_PREFIX, node);
	}

	if (onlp_file_read_int(value, path) < 0) {
		AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", path);
		return ONLP_STATUS_E_INTERNAL;
	}

	return ret;
}

int psu_ym2651y_pmbus_info_get(int id, char *node, int *value)
{
	int  ret = 0;
	char path[PSU_NODE_MAX_PATH_LEN] = {0};

	*value = 0;

	if (PSU1_ID == id) {
		sprintf(path, "%s%s", PSU1_AC_PMBUS_PREFIX, node);
	} else {
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

	if (onlp_file_write_int(value, path) < 0) {
		AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
		return ONLP_STATUS_E_INTERNAL;
	}

	return ONLP_STATUS_OK;
}



#define PSU_SERIAL_NUMBER_LEN	18

int psu_serial_number_get(int id, char *serial, int serial_len)
{
	int   size = 0;
	int   ret  = ONLP_STATUS_OK;
	char *prefix = NULL;

	if (serial == NULL || serial_len < PSU_SERIAL_NUMBER_LEN)
		return ONLP_STATUS_E_PARAM;

	prefix = (id == PSU1_ID) ? PSU1_AC_PMBUS_PREFIX : PSU2_AC_PMBUS_PREFIX;

	ret = onlp_file_read((uint8_t*)serial, PSU_SERIAL_NUMBER_LEN, &size,
			     "%s%s", prefix, "psu_mfr_serial");

	if (ret != ONLP_STATUS_OK || size != PSU_SERIAL_NUMBER_LEN)
		return ONLP_STATUS_E_INTERNAL;


	serial[PSU_SERIAL_NUMBER_LEN] = '\0';
	return ONLP_STATUS_OK;
}
