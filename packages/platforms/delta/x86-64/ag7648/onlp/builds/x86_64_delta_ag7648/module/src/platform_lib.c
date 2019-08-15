/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc.
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
#include "platform_lib.h"

#define I2C_PSU_MODEL_NAME_LEN 13

psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
    DEBUG_PRINT("id %d, modelname %s, length %d\r\n", id, modelname, modelname_len);
#if 0
    char *node = NULL;
    char model_name[I2C_PSU_MODEL_NAME_LEN + 1] = {0};

    /* Check AC model name */
    node = (id == PSU1_ID) ? PSU1_AC_HWMON_NODE(psu_model_name) : PSU2_AC_HWMON_NODE(psu_model_name);

    if (deviceNodeReadString(node, model_name, sizeof(model_name), 0) == 0) {
        if (strncmp(model_name, "CPR-4011-4M11", strlen("CPR-4011-4M11")) == 0) {
		    if (modelname) {
            aim_strlcpy(modelname, model_name, modelname_len-1);
			}
            return PSU_TYPE_AC_F2B;
        }
        else if (strncmp(model_name, "CPR-4011-4M21", strlen("CPR-4011-4M21")) == 0) {
		    if (modelname) {
            aim_strlcpy(modelname, model_name, modelname_len-1);
			}
            return PSU_TYPE_AC_B2F;
        }
    }

    /* Check DC model name */
    memset(model_name, 0, sizeof(model_name));
    node = (id == PSU1_ID) ? PSU1_DC_HWMON_NODE(psu_model_name) : PSU2_DC_HWMON_NODE(psu_model_name);

    if (deviceNodeReadString(node, model_name, sizeof(model_name), 0) == 0) {
        if (strncmp(model_name, "um400d01G", strlen("um400d01G")) == 0) {
		    if (modelname) {
            aim_strlcpy(modelname, model_name, modelname_len-1);
			}
            return PSU_TYPE_DC_48V_B2F;
        }
        else if (strncmp(model_name, "um400d01-01G", strlen("um400d01-01G")) == 0) {
		    if (modelname) {
            aim_strlcpy(modelname, model_name, modelname_len-1);
			}
            return PSU_TYPE_DC_48V_F2B;
        }
    }
#endif
    return PSU_TYPE_UNKNOWN;
}

enum ag7648_product_id get_product_id(void)
{
    return PID_AG7648;
}

int chassis_fan_count(void)
{
    return 6 ;
}

int chassis_led_count(void)
{
    return 7;
}
