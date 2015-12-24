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
#include "assert.h"
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/thermali.h>
#include "powerpc_accton_as5710_54x_log.h"
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlplib/mmap.h>
#include "platform_lib.h"

#define HW_INFO_PATH "/dev/mtd1"
#define HW_INFO_LENGTH 2*65536
uint8_t hw_info[HW_INFO_LENGTH]={0};

platform_id_t platform_id = PLATFORM_ID_UNKNOWN;

const char*
onlp_sysi_platform_get(void)
{
    return "powerpc-accton-as5710-54x-rX";
}

int
onlp_sysi_platform_set(const char* platform)
{
    if(strstr(platform, "powerpc-accton-as5710-54x-r")) {
        platform_id = PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_RX;
        return ONLP_STATUS_OK;
    }
    if(strstr(platform, "powerpc-as5710-54x-r0b")) {
        platform_id = PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_R0;
        return ONLP_STATUS_OK;
    }
    AIM_LOG_ERROR("No support for platform '%s'", platform);
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_onie_data_phys_addr_get(void** physaddr)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    int  fd, len, nbytes=HW_INFO_LENGTH;
    /* get hw info */
    if ((fd = open(HW_INFO_PATH, O_RDONLY)) == -1)
    {
        return ONLP_STATUS_E_INTERNAL;
    }

    if ((len = read(fd, hw_info, nbytes)) < 0)
    {
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* If the read byte count is less, the format is different and calc will be wrong*/
    if ((close(fd) == -1) || (len != nbytes))
    {
        return ONLP_STATUS_E_INTERNAL;
    }
    *data = hw_info;
    return 0;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{

    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));
    int i;
    int n_led=10, n_thermal=5, n_fan=5;

    assert(max > (n_led+n_thermal+n_fan));

    /* 2 PSUs */
    *e++ = ONLP_PSU_ID_CREATE(1);
    *e++ = ONLP_PSU_ID_CREATE(2);

    /* LEDs Item */
    for (i=1; i<=n_led; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* THERMALs Item */
    for (i=1; i<=n_thermal; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* Fans Item */
    for (i=1; i<=n_fan; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

/*
 * 1. If any FAN failed, set all the other fans as full speed (100%)
 * 2. When (LM75-1 + LM75-2)/2 >= 49.5 C, set fan speed from 40% to 65%.
 * 3. When (LM75-1 + LM75-2)/2 >= 53C, set fan speed from 65% to 80%
 * 4. When (LM75-1 + LM75-2)/2 >= 57.7C, set fan speed from 80% to 100%

 * 5. When (LM75-1 + LM75-2)/2 <= 52.7C, set fan speed from 100% to 80%
 * 6. When (LM75-1 + LM75-2)/2 <= 47.7C, set fan speed from 80% to 65%
 * 7. When (LM75-1 + LM75-2)/2 <= 42.7C, set fan speed from 65% to 40%
 * 8. The default FAN speed is 40%
 */
int
onlp_sysi_platform_manage_fans(void)
{
#define LEV1_UP_TEMP    57500  /*temperature*/
#define LEV1_DOWN_TEMP  NULL  /* unused */
#define LEV1_SPEED_PERC 100   /*percentage*/

#define LEV2_UP_TEMP    53000
#define LEV2_DOWN_TEMP  52700
#define LEV2_SPEED_PERC 80

#define LEV3_UP_TEMP    49500
#define LEV3_DOWN_TEMP  47700
#define LEV3_SPEED_PERC 65

#define LEV4_UP_TEMP    NULL  /* unused */
#define LEV4_DOWN_TEMP  42700
#define LEV4_SPEED_PERC 40

#define FAN_NUM_ON_MAIN_BROAD  5

#define LOCAL_DEBUG     0

	int rc, i;
    int is_up;
    int new_temp, temp1, temp2, diff;
    static int new_perc = 0, ori_perc = 0;
    static int ori_temp = 0;
    onlp_thermal_info_t thermal_info;
    onlp_fan_info_t fan_info;

    /* get new temperature */
    if ((rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(3), &thermal_info)) != ONLP_STATUS_OK)
        goto _EXIT;

    temp1 = thermal_info.mcelsius;

    if ((rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(4), &thermal_info)) != ONLP_STATUS_OK)
        goto _EXIT;

    temp2 = thermal_info.mcelsius;

    new_temp = (temp1+temp2)/2;

    /* check fan status */
    for (i=1; i<=FAN_NUM_ON_MAIN_BROAD; i++)
    {
        if ((rc = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info)) != ONLP_STATUS_OK)
            goto _EXIT;

        if (fan_info.status & ONLP_FAN_STATUS_FAILED)
        {
            new_perc = LEV1_SPEED_PERC;
            goto _CTRL;
        }
    }

    diff = new_temp - ori_temp;

    if (diff == 0)
        goto _EXIT;
    else
        is_up = (diff > 0 ? 1 : 0);

    if (is_up)
    {
        if (new_temp >= LEV1_UP_TEMP)
            new_perc = LEV1_SPEED_PERC;
        else if (new_temp >= LEV2_UP_TEMP)
            new_perc = LEV2_SPEED_PERC;
        else if (new_temp >= LEV3_UP_TEMP)
            new_perc = LEV3_SPEED_PERC;
        else
            new_perc = LEV4_SPEED_PERC;
    }
    else
    {
        if (new_temp <= LEV4_DOWN_TEMP)
            new_perc = LEV4_SPEED_PERC;
        else if (new_temp <= LEV3_DOWN_TEMP)
            new_perc = LEV3_SPEED_PERC;
        else if (new_temp <= LEV2_DOWN_TEMP)
            new_perc = LEV2_SPEED_PERC;
        else
            new_perc = LEV1_SPEED_PERC;
    }

_CTRL :

    if (LOCAL_DEBUG)
        printf("\n[DEBUG][%s][%d]{ori:temp=%d, perc=%d} {new:temp=%d, perc=%d}\n", __FUNCTION__, __LINE__,
                ori_temp, ori_perc, new_temp, new_perc);

    if (ori_perc == new_perc)
        goto _EXIT;

    /* ctrl fans */
    AIM_LOG_INFO("Fan Speeds are now at %d%%", new_perc);

    if ((rc = onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), new_perc)) != ONLP_STATUS_OK)
        goto _EXIT;

    /* update om */
    ori_perc = new_perc;
    ori_temp = new_temp;

_EXIT :

    return rc;
}

int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

#include <onlplib/i2c.h>

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int v1, v2, v3;

    v1 = onlp_i2c_readb(3, 0x60, 1, ONLP_I2C_F_FORCE);
    v2 = onlp_i2c_readb(3, 0x61, 1, ONLP_I2C_F_FORCE);
    v3 = onlp_i2c_readb(3, 0x62, 1, ONLP_I2C_F_FORCE);
    pi->cpld_versions = aim_fstrdup("%d.%d.%d", v1, v2, v3);

    return 0;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}
