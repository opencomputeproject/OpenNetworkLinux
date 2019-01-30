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
#include "mlnx_common_log.h"
#include "mlnx_common_int.h"
#include "mlnx_common/mlnx_common.h"

mlnx_platform_info_t mlnx_platform_info;

static char arr_cplddev_name[MAX_NUM_OF_CPLD][30] =
{
    "cpld_brd_version",
    "cpld_mgmt_version",
    "cpld_port_version"
};

mlnx_platform_info_t* get_platform_info()
{
	return &mlnx_platform_info;
}

const char* onlp_sysi_platform_get()
{
	if (mc_get_platform_info(&mlnx_platform_info) < 0) {
		AIM_LOG_ERROR("Unable to get paltform info!\n");
		return NULL;
	}
	else
		return mlnx_platform_info.onl_platform_name;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int   i, v[MAX_NUM_OF_CPLD]={0};
    mlnx_platform_info_t* platform_info = get_platform_info();

    for (i=0; i < platform_info->cpld_num; i++) {
        v[i] = 0;
        if(onlp_file_read_int(v+i, "%s/%s", PREFIX_PATH_ON_CPLD_DEV, arr_cplddev_name[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    switch (platform_info->cpld_num) {
    case 1:
        pi->cpld_versions = aim_fstrdup("unified=%d", v[0]); /* TBD Currently not exist */
        break;
    case 2:
        pi->cpld_versions = aim_fstrdup("brd=%d, mgmt=%d", v[0], v[1]);
        break;
    case 3:
        pi->cpld_versions = aim_fstrdup("brd=%d, mgmt=%d, port=%d", v[0], v[1], v[2]);
        break;
    case 0:
    default:
        AIM_LOG_ERROR("Incorrect CPLD Number %d\n", platform_info->cpld_num);
        return ONLP_STATUS_E_INTERNAL;
    }

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

    for (i = 1; i <= mlnx_platform_info.thermal_num; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    for (i = 1; i <= mlnx_platform_info.led_num; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    for (i = 1; i <= mlnx_platform_info.psu_num; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    for (i = 1; i <= mlnx_platform_info.fan_num; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return ONLP_STATUS_OK;
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
        onie->platform_name = aim_strdup(mlnx_platform_info.onl_platform_name);
    }

    return rv;
}

int
onlp_sysi_platform_manage_leds_type1(void)
{
	int fan_number, psu_number;
	onlp_led_mode_t mode, system_mode;
	int min_fan_speed;
	int psu_led_id[2] = { LED_PSU1, LED_PSU2 };
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
	mode = ONLP_LED_MODE_GREEN;
	for( fan_number = 1; fan_number<= mlnx_platform_info.fan_num; fan_number+=2)
	{
		/* each 2 fans had same led_fan */
		onlp_fan_info_t fi;
		/* check fans */
		if(onlp_fani_info_get(ONLP_FAN_ID_CREATE(fan_number), &fi) < 0) {
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		}
		else if( (fi.status & ONLP_FAN_STATUS_PRESENT) == 0) {
		    if(mlnx_platform_info.fan_fixed == false) {
			/* Not present */
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		    }
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
		    if(mlnx_platform_info.fan_fixed == false) {
			/* Not present */
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		    }
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
	}
	onlp_ledi_mode_set(ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_LED, LED_FAN), mode);

	for (psu_number = 1; psu_number <= mlnx_platform_info.psu_num; psu_number++)
	{
		onlp_psu_info_t pi;
		mode = ONLP_LED_MODE_GREEN;
		if(onlp_psui_info_get(ONLP_PSU_ID_CREATE(psu_number), &pi) < 0) {
			mode = ONLP_LED_MODE_RED;
			psu_problem = 1;
		}
		else {
		    if(mlnx_platform_info.psu_fixed) {
			/* Fixed system, PSU always in. Check only cable plugged. */
		      if(pi.status & ONLP_PSU_STATUS_UNPLUGGED) {
			    mode = ONLP_LED_MODE_RED;
			    psu_problem = 1;
		      }
		    }
		    else {
			if((pi.status & ONLP_PSU_STATUS_PRESENT) == 0) {
				/* Not present */
				psu_problem = 1;
			}
			else if(pi.status & ONLP_PSU_STATUS_UNPLUGGED) {
				psu_problem = 1;
			}
		    }
		}
		onlp_ledi_mode_set(ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_LED, psu_led_id[(psu_number-1)]), mode);
	}

	/* Set System status LED green if no problem in FANs or PSUs */
	if (fan_problem || psu_problem)
		system_mode = ONLP_LED_MODE_RED;
	else
		system_mode = ONLP_LED_MODE_GREEN;

	onlp_ledi_mode_set(ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_LED, LED_SYSTEM), system_mode);

	return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds_type2(void)
{
	int fan_number, psu_number;
	onlp_led_mode_t mode, system_mode;
	int min_fan_speed;

	int fan_led_id[4] = { LED_FAN1, LED_FAN2, LED_FAN3, LED_FAN4 };

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
	for( fan_number = 1; fan_number <= mlnx_platform_info.fan_num; fan_number+=2)
	{
		/* each 2 fans had same led_fan */
		onlp_fan_info_t fi;
		/* check fans */
		mode = ONLP_LED_MODE_GREEN;
		if(onlp_fani_info_get(ONLP_FAN_ID_CREATE(fan_number), &fi) < 0) {
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		}
		else if( (fi.status & ONLP_FAN_STATUS_PRESENT) == 0) {
		    if(mlnx_platform_info.fan_fixed == false) {
			/* Not present */
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		    }
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
		    if(mlnx_platform_info.fan_fixed == false) {
			/* Not present */
			mode = ONLP_LED_MODE_RED;
			fan_problem = 1;
		    }
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

	for (psu_number = 1; psu_number <= mlnx_platform_info.psu_num; psu_number++)
	{
		onlp_psu_info_t pi;
		if(onlp_psui_info_get(ONLP_PSU_ID_CREATE(psu_number), &pi) < 0) {
			psu_problem = 1;
		}
		else  {
		    if(mlnx_platform_info.psu_fixed) {
			/* Fixed system, PSU always in. Check only cable plugged. */
		      if(pi.status & ONLP_PSU_STATUS_UNPLUGGED) {
			    mode = ONLP_LED_MODE_RED;
			    psu_problem = 1;
		      }
		    }
		    else {
			if((pi.status & ONLP_PSU_STATUS_PRESENT) == 0) {
				/* Not present */
				psu_problem = 1;
			}
			else if(pi.status & ONLP_PSU_STATUS_UNPLUGGED) {
				psu_problem = 1;
			}
		    }
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

int
onlp_sysi_platform_manage_leds_type3(void)
{
    int fan_number, psu_number;
    onlp_led_mode_t mode, system_mode;
    int min_fan_speed;

    int fan_led_id[6] = { LED_FAN1, LED_FAN2, LED_FAN3, LED_FAN4, LED_FAN5, LED_FAN6 };

    int fan_problem = 0;
    int psu_problem = 0;

    /*
     * FAN Indicators
     *
     *     Green - Fan is operating
     *     Orange   - No power or Fan failure
     *     Off   - No power
     *
     */
    for (fan_number = 1; fan_number <= mlnx_platform_info.fan_num; fan_number += 2)
    {
        /* each 2 fans had same led_fan */
        onlp_fan_info_t fi;
        /* check fans */
        mode = ONLP_LED_MODE_GREEN;
        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(fan_number), &fi) < 0)
        {
            mode = ONLP_LED_MODE_ORANGE;
            fan_problem = 1;
        } else if ((fi.status & ONLP_FAN_STATUS_PRESENT) == 0) {
            if (mlnx_platform_info.fan_fixed == false)
            {
                /* Not present */
                mode = ONLP_LED_MODE_ORANGE;
                fan_problem = 1;
            }
        } else if (fi.status & ONLP_FAN_STATUS_FAILED) {
            mode = ONLP_LED_MODE_ORANGE;
            fan_problem = 1;
        } else {
            min_fan_speed = onlp_fani_get_min_rpm(fan_number);
            if (fi.rpm < min_fan_speed)
            {
                mode = ONLP_LED_MODE_ORANGE;
                fan_problem = 1;
            }
        }
        /* check fan i+1 */
        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(fan_number + 1), &fi) < 0)
        {
            mode = ONLP_LED_MODE_ORANGE;
            fan_problem = 1;
        } else if ((fi.status & 0x1) == 0) {
            if (mlnx_platform_info.fan_fixed == false)
            {
                /* Not present */
                mode = ONLP_LED_MODE_ORANGE;
                fan_problem = 1;
            }
        } else if (fi.status & ONLP_FAN_STATUS_FAILED)
        {
            mode = ONLP_LED_MODE_ORANGE;
            fan_problem = 1;
        } else {
            min_fan_speed = onlp_fani_get_min_rpm(fan_number + 1);
            if (fi.rpm < min_fan_speed)
            {
                mode = ONLP_LED_MODE_ORANGE;
                fan_problem = 1;
            }
        }
        onlp_ledi_mode_set( ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_LED, fan_led_id[fan_number / 2]), mode);
    }

    for (psu_number = 1; psu_number <= mlnx_platform_info.psu_num; psu_number++)
    {
        onlp_psu_info_t pi;
        if (onlp_psui_info_get(ONLP_PSU_ID_CREATE(psu_number), &pi) < 0)
        {
            psu_problem = 1;
        } else {
            if (mlnx_platform_info.psu_fixed)
            {
                /* Fixed system, PSU always in. Check only cable plugged. */
                if (pi.status & ONLP_PSU_STATUS_UNPLUGGED)
                {
                    mode = ONLP_LED_MODE_ORANGE;
                    psu_problem = 1;
                }
            } else {
                if ((pi.status & ONLP_PSU_STATUS_PRESENT) == 0)
                {
                    /* Not present */
                    psu_problem = 1;
                } else if (pi.status & ONLP_PSU_STATUS_UNPLUGGED) {
                    psu_problem = 1;
                }
            }
        }
    }

    if (psu_problem)
        mode = ONLP_LED_MODE_ORANGE;
    else
        mode = ONLP_LED_MODE_GREEN;
    onlp_ledi_mode_set(ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_LED, LED_PSU), mode);

    /* Set System status LED green if no problem in FANs or PSUs */
    if (fan_problem || psu_problem)
        system_mode = ONLP_LED_MODE_ORANGE;
    else
        system_mode = ONLP_LED_MODE_GREEN;

    onlp_ledi_mode_set(ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_LED, LED_SYSTEM), system_mode);

    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    int res;
    switch (mlnx_platform_info.led_type) {
        case LED_TYPE_1:
            res=onlp_sysi_platform_manage_leds_type1();
            break;

        case LED_TYPE_2:
            res=onlp_sysi_platform_manage_leds_type2();
            break;

        case LED_TYPE_3:
            res=onlp_sysi_platform_manage_leds_type3();
            break;
        default:
            res = ONLP_STATUS_E_INVALID;
    }
    return res;
}
