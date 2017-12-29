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
#include "x86_64_mlnx_msn2700_int.h"
#include "x86_64_mlnx_msn2700_log.h"

#define ONL_PLATFORM_NAME             "x86-64-mlnx-msn2700-r0"
#define ONIE_PLATFORM_NAME            "x86_64-mlnx_msn2700-r0"

#define NUM_OF_THERMAL_ON_MAIN_BROAD  CHASSIS_THERMAL_COUNT
#define NUM_OF_FAN_ON_MAIN_BROAD      CHASSIS_FAN_COUNT
#define NUM_OF_PSU_ON_MAIN_BROAD      2
#define NUM_OF_LED_ON_MAIN_BROAD      6

#define COMMAND_OUTPUT_BUFFER         256

#define PREFIX_PATH_ON_CPLD_DEV       "/bsp/cpld"
#define NUM_OF_CPLD                   3
static char arr_cplddev_name[NUM_OF_CPLD][30] =
{
    "cpld_brd_version",
    "cpld_mgmt_version",
    "cpld_port_version"
};

const char*
onlp_sysi_platform_get(void)
{
    return ONL_PLATFORM_NAME;
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

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{
    int rv = onlp_onie_read_json(onie,
                                 "/lib/platform-config/current/onl/etc/onie/eeprom.json");
    if(rv >= 0) {
        if(onie->platform_name) {
            aim_free(onie->platform_name);
        }
        onie->platform_name = aim_strdup(ONIE_PLATFORM_NAME);
    }

    return rv;
}

int
onlp_sysi_platform_manage_leds(void)
{
	int fan_number, psu_number;
	onlp_led_mode_t mode, system_mode;
	int min_fan_speed;
	enum onlp_led_id fan_led_id[4] = { LED_FAN1, LED_FAN2, LED_FAN3, LED_FAN4 };
	int fan_problem = 0;
	int psu_problem = 0;

	/*
	 * FAN Indicators
	 *
	 *     Green - Fan is operating
	 *     Red   - No power or Fan failure
	 *     Off   - No power
	 *
	 */
	for( fan_number = 1; fan_number <= CHASSIS_FAN_COUNT; fan_number+=2)
	{
		/* each 2 fans had same led_fan */
		onlp_fan_info_t fi;
		/* check fan i */
		mode = ONLP_LED_MODE_GREEN;
		if(onlp_fani_info_get(ONLP_FAN_ID_CREATE(fan_number), &fi) < 0) {
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		}
		else if( (fi.status & ONLP_FAN_STATUS_PRESENT) == 0) {
			/* Not present */
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		}
		else if(fi.status & ONLP_FAN_STATUS_FAILED) {
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		}
		else
		{
			min_fan_speed = onlp_fani_get_min_rpm(fan_number);
			if( fi.rpm < min_fan_speed)
			{
				mode = ONLP_LED_MODE_RED;
				fan_problem = 1;
			}
		}
		/* check fan i+1 */
		if(onlp_fani_info_get(ONLP_FAN_ID_CREATE(fan_number+1), &fi) < 0) {
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		}
		else if( (fi.status & 0x1) == 0) {
			/* Not present */
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		}
		else if(fi.status & ONLP_FAN_STATUS_FAILED) {
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		}
		else
		{
			min_fan_speed = onlp_fani_get_min_rpm(fan_number+1);
			if( fi.rpm < min_fan_speed)
			{
				mode = ONLP_LED_MODE_RED;
				fan_problem = 1;
			}
		}
		onlp_ledi_mode_set(ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_LED,fan_led_id[fan_number/2]), mode);
	}

	for (psu_number = 1; psu_number <= CHASSIS_PSU_COUNT; psu_number++)
	{
		onlp_psu_info_t pi;
		if(onlp_psui_info_get(ONLP_PSU_ID_CREATE(psu_number), &pi) < 0) {
			psu_problem = 1;
		}
		else if((pi.status & ONLP_PSU_STATUS_PRESENT) == 0) {
			/* Not present */
			psu_problem = 1;
		}
		else if(pi.status & ONLP_PSU_STATUS_UNPLUGGED) {
			psu_problem = 1;
		}
	}

	if (psu_problem)
		mode = ONLP_LED_MODE_RED;
	else
		mode = ONLP_LED_MODE_GREEN;
	onlp_ledi_mode_set(ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_LED, LED_PSU), mode);

	/* Set System status LED green if no problem in FANs or PSUs */
	if (fan_problem || psu_problem)
		system_mode = ONLP_LED_MODE_RED;
	else
		system_mode = ONLP_LED_MODE_GREEN;

	onlp_ledi_mode_set(ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_LED,LED_SYSTEM), system_mode);

	return ONLP_STATUS_OK;
}

