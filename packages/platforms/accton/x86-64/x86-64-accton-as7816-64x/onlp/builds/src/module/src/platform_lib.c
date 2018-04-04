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

#define PSU_MODEL_NAME_LEN 		8
#define PSU_SERIAL_NUMBER_LEN	18
#define PSU_NODE_MAX_PATH_LEN   64

int psu_serial_number_get(int id, char *serial, int serial_len)
{
	int   size = 0;
	int   ret  = ONLP_STATUS_OK; 
	char *prefix = NULL;

	if (serial == NULL || serial_len < PSU_SERIAL_NUMBER_LEN) {
		return ONLP_STATUS_E_PARAM;
	}

	prefix = (id == PSU1_ID) ? PSU1_AC_PMBUS_PREFIX : PSU2_AC_PMBUS_PREFIX;

	ret = onlp_file_read((uint8_t*)serial, PSU_SERIAL_NUMBER_LEN, &size, "%s%s", prefix, "psu_mfr_serial");
    if (ret != ONLP_STATUS_OK || size != PSU_SERIAL_NUMBER_LEN) {
		return ONLP_STATUS_E_INTERNAL;

    }

	serial[PSU_SERIAL_NUMBER_LEN] = '\0';
	return ONLP_STATUS_OK;
}

psu_type_t psu_type_get(int id, char* modelname, int modelname_len)
{
	int   value = 0;
	int   ret   = ONLP_STATUS_OK; 
	char  model[PSU_MODEL_NAME_LEN + 1] = {0};
	char *prefix = NULL;

	if (modelname && modelname_len < PSU_MODEL_NAME_LEN) {
		return PSU_TYPE_UNKNOWN;
	}

	/* Check if the psu is power good
	 */
    if (onlp_file_read_int(&value, PSU_POWERGOOD_FORMAT, id) < 0) {
        AIM_LOG_ERROR("Unable to read present status from PSU(%d)\r\n", index);
        return ONLP_STATUS_E_INTERNAL;
    }

	if (!value) {
		return PSU_TYPE_UNKNOWN;
	}

	/* Read mode name */
	prefix = (id == PSU1_ID) ? PSU1_AC_PMBUS_PREFIX : PSU2_AC_PMBUS_PREFIX;
	ret = onlp_file_read((uint8_t*)model, PSU_MODEL_NAME_LEN, &value, "%s%s", prefix, "psu_mfr_model");
    if (ret != ONLP_STATUS_OK || value != PSU_MODEL_NAME_LEN) {
		return PSU_TYPE_UNKNOWN;

    }

    if (modelname) {
		memcpy(modelname, model, sizeof(model));
    }

    if (strncmp(model, "YM-2851F", strlen("YM-2851F")) == 0) {
        return PSU_TYPE_AC_YM2851_F2B;
    }

    return PSU_TYPE_UNKNOWN;
}

int psu_ym2651y_pmbus_info_get(int id, char *node, int *value)
{
	char *prefix = NULL;
    *value = 0;

	prefix = (id == PSU1_ID) ? PSU1_AC_PMBUS_PREFIX : PSU2_AC_PMBUS_PREFIX;
    if (onlp_file_read_int(value, "%s%s", prefix, node) < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s%s)\r\n", prefix, node);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int psu_ym2651y_pmbus_info_set(int id, char *node, int value)
{
	char *prefix = NULL;

	prefix = (id == PSU1_ID) ? PSU1_AC_PMBUS_PREFIX : PSU2_AC_PMBUS_PREFIX;
    if (onlp_file_write_int(value, "%s%s", prefix, node) < 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s%s)\r\n", prefix, node);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

