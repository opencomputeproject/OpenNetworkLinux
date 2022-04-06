/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2018 Alpha Networks Incorporation
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
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <onlplib/file.h>
#include <AIM/aim.h>
#include <onlp/platformi/sfpi.h>
#include "platform_lib.h"

#include <onlplib/i2c.h>

#define DEBUG_FLAG 0
#define PSU_MODEL_NAME_LEN          20  /* mapping to PSU driver */
#define PSU_SERIAL_NUMBER_LEN       26  /* mapping to PSU driver */

#define AIM_FREE_IF_PTR(p)          \
	do {						    \
		if (p) {				    \
			aim_free(p); 			\
			p = NULL;			    \
		} 					        \
	} while (0)
	
static char diag_flag=0;

char diag_flag_set(char d)
{
    diag_flag = d;
    return 0;
}

char diag_flag_get(void)
{
    return diag_flag;
}

char diag_debug_trace_on(void)
{
    system("echo 1 > /tmp/onlpi_dbg_trace");
    return 0;
}

char diag_debug_trace_off(void)
{
    system("echo 0 > /tmp/onlpi_dbg_trace");
    return 0;
}

char diag_debug_trace_check(void)
{
    char flag = 0;
    FILE* file = fopen ("/tmp/onlpi_dbg_trace", "r");
    if (file == NULL)
    {
        return 0;
    }
    flag = fgetc (file);
    fclose (file);

    return (flag == '1')?1:0;
}

char* sfp_control_to_str(int value)
{
    switch (value)
    {
        case ONLP_SFP_CONTROL_RESET:
            return "RESET";
        case ONLP_SFP_CONTROL_RESET_STATE:
            return "RESET_STATE";
        case ONLP_SFP_CONTROL_RX_LOS:
            return "RX_LOS";
        case ONLP_SFP_CONTROL_TX_FAULT:
            return "TX_FAULT";
        case ONLP_SFP_CONTROL_TX_DISABLE:
            return "TX_DISABLE";
        case ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL:
            return "TX_DISABLE_CHANNEL";
        case ONLP_SFP_CONTROL_LP_MODE:
            return "LP_MODE";
        case ONLP_SFP_CONTROL_POWER_OVERRIDE:
            return "POWER_OVERRIDE";

        default:
            return "UNKNOW";
    }
    return "";
}

char diag_debug_pause_platform_manage_on(void)
{
    system("echo 1 > /tmp/onlpi_dbg_pause_pm");
    return 0;
}

char diag_debug_pause_platform_manage_off(void)
{
    system("echo 0 > /tmp/onlpi_dbg_pause_pm");
    return 0;
}

char diag_debug_pause_platform_manage_check(void)
{
    char flag = 0;
    FILE* file = fopen ("/tmp/onlpi_dbg_pause_pm", "r");
    if (file == NULL)
    {
        return 0;
    }
    flag = fgetc (file);
    fclose (file);

    return (flag == '1')?1:0;
}

int psu_serial_number_get(int id, char *serial, int serial_len)
{
    char *node = NULL;
    int   len = 0;

    if (serial == NULL || serial_len < PSU_SERIAL_NUMBER_LEN + 1) 
    {
        return ONLP_STATUS_E_PARAM;
    }

    switch(id)
    {
        case PSU1_ID:
            node = PSU1_AC_PMBUS_NODE(psu_mfr_serial);
            break;
        case PSU2_ID:
            node = PSU2_AC_PMBUS_NODE(psu_mfr_serial);
            break;
        default:
            break;
    }

    /* Copy serial number from PSU PMBUS */
	char *string = NULL;
    len = onlp_file_read_str(&string, "%s", node);
    if (len <= 0 || len > PSU_SERIAL_NUMBER_LEN) 
    {
        AIM_LOG_ERROR("PSU Serial Number length %d is invalid\r\n", len);
        AIM_FREE_IF_PTR(string);
        return ONLP_STATUS_E_INTERNAL;
    }

    aim_strlcpy(serial, string, len+1);

    AIM_FREE_IF_PTR(string);
    return ONLP_STATUS_OK;
}

psu_type_t psu_type_get(int id, char* modelname, int modelname_len)
{
    char *node = NULL;
    int   len = 0;

    /* Check AC model name , AC can't be read from eeprom or model name */
    switch(id)
    {
        case PSU1_ID:
            node = PSU1_AC_PMBUS_NODE(psu_mfr_model);
            break;
        case PSU2_ID:
            node = PSU2_AC_PMBUS_NODE(psu_mfr_model);
            break;
        default:
            break;
    }

	/* Read model name */
	char *string = NULL;
    len = onlp_file_read_str(&string, "%s", node);
    if (len <= 0 || len > PSU_MODEL_NAME_LEN) 
    {
        AIM_LOG_ERROR("PSU Model Name length %d is invalid\r\n", len);
        AIM_FREE_IF_PTR(string);
        return PSU_TYPE_UNKNOWN;
    }

    if (modelname) 
    {
        aim_strlcpy(modelname, string, len+1);
    }

    /* Current PSU, can't read fan_dir*/

    AIM_FREE_IF_PTR(string);
    return PSU_TYPE_AC_F2B;
}

int psu_pmbus_info_get(int id, char *node, int *value)
{
    char *prefix = NULL;
    *value = 0;

    switch(id)
    {
        case PSU1_ID:
            prefix = PSU1_AC_PMBUS_PREFIX;
            break;
        case PSU2_ID:
            prefix = PSU2_AC_PMBUS_PREFIX;
            break;
        default:
            break;
    }
    if (onlp_file_read_int(value, "%s%s", prefix, node) < 0) 
    {
        AIM_LOG_ERROR("Unable to read status from file(%s%s)\r\n", prefix, node);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int psu_pmbus_info_set(int id, char *node, int value)
{
    char *prefix = NULL;

    switch(id)
    {
        case PSU1_ID:
            prefix = PSU1_AC_PMBUS_PREFIX;
            break;
        case PSU2_ID:
            prefix = PSU2_AC_PMBUS_PREFIX;
            break;
        default:
            break;
    }
    if (onlp_file_write_int(value, "%s%s", prefix, node) < 0) 
    {
        AIM_LOG_ERROR("Unable to write data to file (%s%s)\r\n", prefix, node);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
