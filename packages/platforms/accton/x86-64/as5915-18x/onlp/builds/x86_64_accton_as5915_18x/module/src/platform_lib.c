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

#if 0

#define PSU_NODE_MAX_PATH_LEN 64
#define PSU_FAN_DIR_LEN         3
#define PSU_MODEL_NAME_LEN 		11
#define PSU_SERIAL_NUMBER_LEN	18



psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
    int   ret = 0;
    char *node = NULL;
    char *mn = aim_zmalloc(PSU_MODEL_NAME_LEN + 1);
    psu_type_t ptype = PSU_TYPE_UNKNOWN;

    /* Check AC model name */
    node = (id == PSU1_ID) ? PSU1_AC_HWMON_NODE(psu_model_name) : PSU2_AC_HWMON_NODE(psu_model_name);
    ret = onlp_file_read_str(&mn, node);
    if (ret <= 0 || ret > PSU_MODEL_NAME_LEN) {
        aim_free(mn);
        return PSU_TYPE_UNKNOWN;
    }

    if (modelname) {
        strncpy(modelname, mn, PSU_MODEL_NAME_LEN + 1);
    }

    if (strncmp(mn, "FSH082-610G", PSU_MODEL_NAME_LEN) == 0) {
        ptype = PSU_TYPE_AC_F2B;
    }

    aim_free(mn);
    return ptype;
}


#if 1
    int   i, len;
    char *string = NULL;
    char  v[NUM_OF_CPLD][CPLD_VER_MAX_STR_LEN]={{0}};

    for (i = 0; i < NUM_OF_CPLD; i++) {
        len = onlp_file_read_str(&string, "%s%s", PREFIX_PATH_ON_CPLD_DEV, arr_cplddev_name[i]);
        if (string && len) {
            strncpy(v[i], string, len);
            aim_free(string);
        } else {
            return ONLP_STATUS_E_INTERNAL;
        }
    }

#endif

#endif
