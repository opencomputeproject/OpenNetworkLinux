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
#include <unistd.h>
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_accton_as7712_32x_int.h"
#include "x86_64_accton_as7712_32x_log.h"

#include "platform_lib.h"

#define NUM_OF_THERMAL_ON_MAIN_BROAD  CHASSIS_THERMAL_COUNT
#define NUM_OF_FAN_ON_MAIN_BROAD      CHASSIS_FAN_COUNT
#define NUM_OF_PSU_ON_MAIN_BROAD      2
#define NUM_OF_LED_ON_MAIN_BROAD      5

#define PREFIX_PATH_ON_CPLD_DEV          "/sys/bus/i2c/devices/"
#define NUM_OF_CPLD                      3
static char arr_cplddev_name[NUM_OF_CPLD][10] =
{
 "4-0060",
 "5-0062",
 "6-0064"
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as7712-32x-r0";
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);
    if(onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) {
        if(*size == 256) {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }

    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_INTERNAL;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int   i, v[NUM_OF_CPLD]={0};
    for (i=0; i < NUM_OF_CPLD; i++) {
        v[i] = 0;
        if(onlp_file_read_int(v+i, "%s%s/version", PREFIX_PATH_ON_CPLD_DEV, arr_cplddev_name[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    pi->cpld_versions = aim_fstrdup("%d.%d.%d", v[0], v[1], v[2]);
    return 0;
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

    /* 4 Thermal sensors on the chassis */
    for (i = 1; i <= NUM_OF_THERMAL_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 5 LEDs on the chassis */
    for (i = 1; i <= NUM_OF_LED_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= NUM_OF_PSU_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 4 Fans on the chassis */
    for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

typedef struct fan_ctrl_policy {
   int duty_cycle;
   int temp_down_adjust; /* The boundary temperature to down adjust fan speed */
   int temp_up_adjust;   /* The boundary temperature to up adjust fan speed */
} fan_ctrl_policy_t;

fan_ctrl_policy_t  fan_ctrl_policy_f2b[] = {
{32,      0, 174000},
{38, 170000, 182000},
{50, 178000, 190000},
{63, 186000,      0}
};

fan_ctrl_policy_t  fan_ctrl_policy_b2f[] = {
{32,     0,  140000},
{38, 135000, 150000},
{50, 145000, 160000},
{69, 155000,      0}
};

#define FAN_DUTY_CYCLE_MAX  100
#define FAN_SPEED_CTRL_PATH "/sys/bus/i2c/devices/2-0066/fan_duty_cycle_percentage"

/*
 * For AC power Front to Back :
 *	* If any fan fail, please fan speed register to 15
 *	* The max value of Fan speed register is 9
 *		[LM75(48) + LM75(49) + LM75(4A)] > 174  => set Fan speed value from 4 to 5
 *		[LM75(48) + LM75(49) + LM75(4A)] > 182  => set Fan speed value from 5 to 7
 *		[LM75(48) + LM75(49) + LM75(4A)] > 190  => set Fan speed value from 7 to 9
 *
 *		[LM75(48) + LM75(49) + LM75(4A)] < 170  => set Fan speed value from 5 to 4
 *		[LM75(48) + LM75(49) + LM75(4A)] < 178  => set Fan speed value from 7 to 5
 *		[LM75(48) + LM75(49) + LM75(4A)] < 186  => set Fan speed value from 9 to 7
 *
 *
 * For  AC power Back to Front :
 *	* If any fan fail, please fan speed register to 15
 *	* The max value of Fan speed register is 10
 *		[LM75(48) + LM75(49) + LM75(4A)] > 140  => set Fan speed value from 4 to 5
 *		[LM75(48) + LM75(49) + LM75(4A)] > 150  => set Fan speed value from 5 to 7
 *		[LM75(48) + LM75(49) + LM75(4A)] > 160  => set Fan speed value from 7 to 10
 *
 *		[LM75(48) + LM75(49) + LM75(4A)] < 135  => set Fan speed value from 5 to 4
 *		[LM75(48) + LM75(49) + LM75(4A)] < 145  => set Fan speed value from 7 to 5
 *		[LM75(48) + LM75(49) + LM75(4A)] < 155  => set Fan speed value from 10 to 7
 */
int
onlp_sysi_platform_manage_fans(void)
{
    int i = 0, arr_size, temp;
    fan_ctrl_policy_t *policy;
    int cur_duty_cycle, new_duty_cycle;
    onlp_thermal_info_t thermal_1, thermal_2, thermal_3;

    int  fd, len;
    char  buf[10] = {0};

    /* Get each fan status
     */
    for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BROAD; i++)
    {
        onlp_fan_info_t fan_info;

        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get fan(%d) status\r\n", i);
            return ONLP_STATUS_E_INTERNAL;
        }

        /* Decision 1: Set fan as full speed if any fan is failed.
         */
        if (fan_info.status & ONLP_FAN_STATUS_FAILED) {
            AIM_LOG_ERROR("Fan(%d) is not working, set the other fans as full speed\r\n", i);
            return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
        }

        /* Decision 1.1: Set fan as full speed if any fan is not present.
         */
        if (!(fan_info.status & ONLP_FAN_STATUS_PRESENT)) {
            AIM_LOG_ERROR("Fan(%d) is not present, set the other fans as full speed\r\n", i);
            return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
        }

        /* Get fan direction (Only get the first one since all fan direction are the same)
         */
        if (i == 1) {
            if (fan_info.status & ONLP_FAN_STATUS_F2B) {
                policy   = fan_ctrl_policy_f2b;
                arr_size = AIM_ARRAYSIZE(fan_ctrl_policy_f2b);
            }
            else {
                policy   = fan_ctrl_policy_b2f;
                arr_size = AIM_ARRAYSIZE(fan_ctrl_policy_b2f);
            }
        }
    }

    /* Get current fan speed
     */
    fd = open(FAN_SPEED_CTRL_PATH, O_RDONLY);
    if (fd == -1){
        AIM_LOG_ERROR("Unable to open fan speed control node (%s)", FAN_SPEED_CTRL_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }

    len = read(fd, buf, sizeof(buf));
    close(fd);
    if (len <= 0) {
        AIM_LOG_ERROR("Unable to read fan speed from (%s)", FAN_SPEED_CTRL_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }
    cur_duty_cycle = atoi(buf);


    /* Decision 2: If no matched fan speed is found from the policy,
     *             use FAN_DUTY_CYCLE_MIN as default speed
     */
	for (i = 0; i < arr_size; i++) {
	    if (policy[i].duty_cycle != cur_duty_cycle)
		    continue;

		break;
	}

	if (i == arr_size) {
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), policy[0].duty_cycle);
	}

    /* Get current temperature
     */
    if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(2), &thermal_1) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(3), &thermal_2) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(4), &thermal_3) != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read thermal status");
        return ONLP_STATUS_E_INTERNAL;
    }
    temp = thermal_1.mcelsius + thermal_2.mcelsius + thermal_3.mcelsius;


    /* Decision 3: Decide new fan speed depend on fan direction/current fan speed/temperature
     */
    new_duty_cycle = cur_duty_cycle;

    if ((temp >= policy[i].temp_up_adjust) && (i != (arr_size-1))) {
	    new_duty_cycle = policy[i+1].duty_cycle;
	}
	else if ((temp <= policy[i].temp_down_adjust) && (i != 0)) {
	    new_duty_cycle = policy[i-1].duty_cycle;
	}

	if (new_duty_cycle == cur_duty_cycle) {
        /* Duty cycle does not change, just return */
	    return ONLP_STATUS_OK;
	}

    return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), new_duty_cycle);
}

int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

