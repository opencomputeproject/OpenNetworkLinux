/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
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

#include "x86_64_accton_as6812_32x_int.h"
#include "x86_64_accton_as6812_32x_log.h"

#include "platform_lib.h"

#define OPEN_READ_FILE(fd,fullpath,data,nbytes,len) \
    DEBUG_PRINT("[Debug][%s][%d][openfile: %s]\n", __FUNCTION__, __LINE__, fullpath); \
    if ((fd = open(fullpath, O_RDONLY)) == -1)  \
       return ONLP_STATUS_E_INTERNAL;           \
    if ((len = read(fd, r_data, nbytes)) <= 0){ \
        close(fd);                              \
        return ONLP_STATUS_E_INTERNAL;}         \
    DEBUG_PRINT("[Debug][%s][%d][read data: %s]\n", __FUNCTION__, __LINE__, r_data); \
    if (close(fd) == -1)                        \
        return ONLP_STATUS_E_INTERNAL

#define PREFIX_PATH_ON_CPLD_DEV          "/sys/bus/i2c/devices/"
#define NUM_OF_CPLD                      3
static char arr_cplddev_name[NUM_OF_CPLD][10] =
{
 "0-0060",
 "0-0062",
 "0-0064"
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as6812-32x-r0";
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
    int   i, siz=NUM_OF_CPLD, v[NUM_OF_CPLD];
    int   fd, len, nbytes = 10;
    char  r_data[10]   = {0};
    char  fullpath[65] = {0};

    for (i=0; i<siz; i++)
    {
        sprintf(fullpath, "%s%s/version", PREFIX_PATH_ON_CPLD_DEV, arr_cplddev_name[i]);
        OPEN_READ_FILE(fd,fullpath,r_data,nbytes,len);
        v[i]=atoi(r_data);
    }


    if(3==NUM_OF_CPLD)
        pi->cpld_versions = aim_fstrdup("%d.%d.%d", v[0], v[1], v[2]);
    else
        printf("This CPLD numbers are wrong !! \n");

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

    /* 5 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 10 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 5 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    return 0;
}

/* Fan speed control related data
 */
enum fan_duty_cycle {
    FAN_DUTY_CYCLE_MIN = 40,
	FAN_DUTY_CYCLE_65  = 65,
	FAN_DUTY_CYCLE_80  = 80,
	FAN_DUTY_CYCLE_MAX = 100
};

typedef struct fan_ctrl_policy {
   enum fan_duty_cycle duty_cycle;
   int temp_down_adjust; /* The boundary temperature to down adjust fan speed */
   int temp_up_adjust;   /* The boundary temperature to up adjust fan speed */
} fan_ctrl_policy_t;

fan_ctrl_policy_t  fan_ctrl_policy_f2b[] = {
{FAN_DUTY_CYCLE_MIN,     0,  47950},
{FAN_DUTY_CYCLE_65,  42050,  49650},
{FAN_DUTY_CYCLE_80,  47050,  54050},
{FAN_DUTY_CYCLE_MAX, 52050,      0}
};

fan_ctrl_policy_t  fan_ctrl_policy_b2f[] = {
{FAN_DUTY_CYCLE_MIN,     0,  40850},
{FAN_DUTY_CYCLE_65,  35400,  44850},
{FAN_DUTY_CYCLE_80,  40400,  47400},
{FAN_DUTY_CYCLE_MAX, 45400,      0}
};

#define FAN_SPEED_CTRL_PATH "/sys/devices/platform/as6812_32x_fan/fan1_duty_cycle_percentage"

/*
 * Front to Back
 * 1. When (LM75-1 + LM75-4)/2 >= 47.95 C, please set CPLD's registor 0xd from "0x08" to "0x0d". (40% to 65%).
 * 2. When (LM75-1 + LM75-4)/2 >= 49.65 C, please set CPLD's registor 0xd from "0x0d" to "0x10". (65% to 80%)
 * 3. When (LM75-1 + LM75-4)/2 >= 54.05C, please set CPLD's registor 0x0d from "0x10" to "0x14". (80% to 100%)
 * 4. When (LM75-1 + LM75-4)/2 <= 52.05C, please set CPLD's registor 0x0d from "0x14" to "0x10". (100% to 80%)
 * 5. When (LM75-1 + LM75-4)/2 <= 47.05C, please set CPLD's registor 0x0d from "0x10" to "0x0d". (80% to 65%)
 * 6. When (LM75-1 + LM75-4)/2 <= 42.05C, please set CPLD's registor 0x0d from "0x0d" to "0x08"  (65% to 40%).
 * 7. If CPLD's (0x60 address) registor 0x0C is not equal "0x0", for example "0x1~0x1f",
 *    please set  CPLD's registor 0x0d to "0x14" (direct  to 100%) <--- Due to FAN Failed occur.
 * 8. Default value for FAN speed is 40% which is CPLD's registor 0x0d value is "0x08".

 * Back to Front
 * 1. When (LM75-1 + LM75-4)/2 >= 40.85 C, please set CPLD's registor 0xd from "0x08" to "0x0d". (40% to 65%).
 * 2. When (LM75-1 + LM75-4)/2 >= 44.85 C, please set CPLD's registor 0xd from "0x0d" to "0x10". (65% to 80%)
 * 3. When (LM75-1 + LM75-4)/2 >= 47.4C, please set CPLD's registor 0x0d from "0x10" to "0x14".  (80% to 100%)
 * 4. When (LM75-1 + LM75-4)/2 <= 45.4C, please set CPLD's registor 0x0d from "0x14" to "0x10".  (100% to 80%)
 * 5. When (LM75-1 + LM75-4)/2 <= 40.4C, please set CPLD's registor 0x0d from "0x10" to "0x0d".  (80% to 65%)
 * 6. When (LM75-1 + LM75-4)/2 <= 35.4C, please set CPLD's registor 0x0d from "0x0d" to "0x08"   (65% to 40%).
 * 7. If CPLD's (0x60 address)  registor 0x0C is not equal "0x0", for example "0x1~0x1f",
 *    please set  CPLD's registor 0x0d to "0x14" (direct  to 100%) <--- Due to FAN Failed occur.
 * 8. Default value for FAN speed is 40% which is CPLD's registor 0x0d value is "0x08".
 */
int
onlp_sysi_platform_manage_fans(void)
{
    int i = 0, arr_size, avg_temp;
    fan_ctrl_policy_t *policy;
    int cur_duty_cycle, new_duty_cycle;
    onlp_thermal_info_t thermal_1, thermal_4;

    int  fd, len;
    char  buf[10] = {0};

    /* Get each fan status
     */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
    {
        onlp_fan_info_t fan_info;

        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get fan(%d) status", i);
            return ONLP_STATUS_E_INTERNAL;
        }

        /* Decision 1: Set fan as full speed if any fan is failed.
         */
        if (!(fan_info.status & ONLP_FAN_STATUS_PRESENT) || fan_info.status & ONLP_FAN_STATUS_FAILED) {
            AIM_LOG_MSG("Setting max");
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
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MIN);
	}

    /* Get current temperature
     */
    if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(1), &thermal_1) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(4), &thermal_4) != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read thermal status");
        return ONLP_STATUS_E_INTERNAL;
    }
    avg_temp = (thermal_1.mcelsius + thermal_4.mcelsius) / 2;


    /* Decision 3: Decide new fan speed depend on fan direction/current fan speed/temperature
     */
    new_duty_cycle = cur_duty_cycle;

    if ((avg_temp >= policy[i].temp_up_adjust) && (cur_duty_cycle != FAN_DUTY_CYCLE_MAX)) {
	    new_duty_cycle = policy[i+1].duty_cycle;
	}
	else if ((avg_temp <= policy[i].temp_down_adjust) && (cur_duty_cycle != FAN_DUTY_CYCLE_MIN)) {
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

