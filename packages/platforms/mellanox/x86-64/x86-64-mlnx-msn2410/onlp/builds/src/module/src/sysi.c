/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <onlplib/file.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/psui.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/thermali.h>
#include "platform_lib.h"
#include "x86_64_mlnx_msn2410_int.h"
#include "x86_64_mlnx_msn2410_log.h"

#define NUM_OF_THERMAL_ON_MAIN_BROAD  CHASSIS_THERMAL_COUNT
#define NUM_OF_FAN_ON_MAIN_BROAD      CHASSIS_FAN_COUNT
#define NUM_OF_PSU_ON_MAIN_BROAD      2
#define NUM_OF_LED_ON_MAIN_BROAD      6

#define COMMAND_OUTPUT_BUFFER        256

#define PREFIX_PATH_ON_CPLD_DEV          "/bsp/cpld"
#define NUM_OF_CPLD                      3
static char arr_cplddev_name[NUM_OF_CPLD][30] =
{
    "cpld_brd_version",
    "cpld_mgmt_version",
    "cpld_port_version"
};

static void
_onlp_sysi_execute_command(char *command, char buffer[COMMAND_OUTPUT_BUFFER])
{
    FILE *fp = NULL;

    /* Open the command for reading. */
    fp = popen(command, "r");
    if (NULL == fp) {
        AIM_LOG_WARN("Failed to run command '%s'\n", command);
    }

    /* Read the output */
    if (fgets(buffer, COMMAND_OUTPUT_BUFFER-1, fp) == NULL) {
        AIM_LOG_WARN("Failed to read output of command '%s'\n", command);
        pclose(fp);
    }

    /* The last symbol is '\n', so remote it */
    buffer[strnlen(buffer, COMMAND_OUTPUT_BUFFER) - 1] = '\0';

    /* close */
    pclose(fp);
}

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-mlnx-msn2410-r0";
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int   i, v[NUM_OF_CPLD]={0};

    for (i=0; i < NUM_OF_CPLD; i++) {
        v[i] = 0;
        if(onlp_file_read_int(v+i, "%s/%s", PREFIX_PATH_ON_CPLD_DEV, arr_cplddev_name[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    pi->cpld_versions = aim_fstrdup("brd=%d, mgmt=%d, port=%d", v[0], v[1], v[2]);

    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}


int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 8 Thermal sensors on the chassis */
    for (i = 1; i <= NUM_OF_THERMAL_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 6 LEDs on the chassis */
    for (i = 1; i <= NUM_OF_LED_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= NUM_OF_PSU_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 8 Fans and 2 PSU fans on the chassis */
    for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

static int
_onlp_sysi_grep_output(char value[256], const char *attr, const char *tmp_file)
{
    int value_offset  = 30; /* value offset in onie-syseeprom */
    char command[256] = {0};
    char buffer[COMMAND_OUTPUT_BUFFER]  = {0};
    int v = 0;

    snprintf(command, sizeof(command), "cat '%s' | grep '%s'", tmp_file, attr);
    _onlp_sysi_execute_command(command, buffer);

    /* Reading value from buffer with command output */
    while (buffer[value_offset] != '\n' &&
           buffer[value_offset] != '\r' &&
           buffer[value_offset] != '\0') {
        value[v] = buffer[value_offset];
        v++;
        value_offset++;
    }
    value[v] = '\0';

    AIM_LOG_VERBOSE("Value for sytem attribute '%s' is '%s' \n", attr, value);

    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{

    const char onie_version_file[] = "/bsp/onie-version";
    const char onie_version_command[] = "onie-shell -c 'onie-sysinfo -v' > /bsp/onie-version";
    const char onie_syseeprom_file[] = "/bsp/onie-syseeprom";
    const char onie_syseeprom_command[] = "onie-shell -c onie-syseeprom > /bsp/onie-syseeprom";
    struct stat stat_buf;
    char value[256] = {0};
    char command[256] = {0};
    int rc = 0;
    int exit_status;

    /* We must initialize this otherwise crash occurs while free memory */
    list_init(&onie->vx_list);

    /* Check if cache file exist */
	rc = stat(onie_syseeprom_file, &stat_buf);
    if (-1 == rc) {
    	rc = system(onie_syseeprom_command);
    	if (-1 == rc) {
    		return rc;
    	}
		exit_status = WEXITSTATUS(rc);
		if (EXIT_SUCCESS != exit_status) {
			return ONLP_STATUS_E_GENERIC;
		}
    }

    rc = _onlp_sysi_grep_output(value, "Product Name", onie_syseeprom_file);
    if (ONLP_STATUS_OK != rc) {
        return rc;
    }
    onie->product_name = aim_strdup(value);
    rc = _onlp_sysi_grep_output(value, "Part Number", onie_syseeprom_file);
    if (ONLP_STATUS_OK != rc) {
        return rc;
    }
    onie->part_number = aim_strdup(value);
    rc = _onlp_sysi_grep_output(value, "Serial Number", onie_syseeprom_file);
    if (ONLP_STATUS_OK != rc) {
        return rc;
    }
    onie->serial_number = aim_strdup(value);
    rc = _onlp_sysi_grep_output(value, "Base MAC Address", onie_syseeprom_file);
    if (ONLP_STATUS_OK != rc) {
        return rc;
    }
    strncpy((char*)onie->mac, value, sizeof(onie->mac));
    rc = _onlp_sysi_grep_output(value, "Manufacture Date", onie_syseeprom_file);
    if (ONLP_STATUS_OK != rc) {
        return rc;
    }
    onie->manufacture_date = aim_strdup(value);
    rc = _onlp_sysi_grep_output(value, "Device Version", onie_syseeprom_file);
    if (ONLP_STATUS_OK != rc) {
        return rc;
    }
    onie->device_version = atoi(value);
    rc = _onlp_sysi_grep_output(value, "Manufacturer", onie_syseeprom_file);
    if (ONLP_STATUS_OK != rc) {
        return rc;
    }
    onie->manufacturer = aim_strdup(value);
    rc = _onlp_sysi_grep_output(value, "Manufacturer", onie_syseeprom_file);
    if (ONLP_STATUS_OK != rc) {
        return rc;
    }
    onie->manufacturer = aim_strdup(value);
    onie->vendor = aim_strdup(value);
    rc = _onlp_sysi_grep_output(value, "MAC Addresses", onie_syseeprom_file);
    if (ONLP_STATUS_OK != rc) {
        return rc;
    }
    onie->mac_range = atoi(value);
    /* Check if onie version first run and cache file exist */
    rc = stat(onie_version_file, &stat_buf);
    if (-1 == rc)
    {
       rc = system(onie_version_command);
           if (-1 == rc) {
               return rc;
    }
    exit_status = WEXITSTATUS(rc);
    if (EXIT_SUCCESS != exit_status) {
       return ONLP_STATUS_E_GENERIC;
    }}
    snprintf(command, sizeof(command), "cat '%s'", onie_version_file);
    _onlp_sysi_execute_command(command, value);
    /* ONIE version */
    onie->onie_version = aim_strdup(value);

    /* Platform name */
    onie->platform_name = aim_strdup("x86_64-mlnx_msn2410-r0");

    return ONLP_STATUS_OK;
}

