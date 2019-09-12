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
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>

#include "x86_64_accton_as5812_54x_int.h"
#include "x86_64_accton_as5812_54x_log.h"
#include "platform_lib.h"
#include <fcntl.h>
#include <unistd.h>

#define LOCAL_DEBUG 0

#define OPEN_READ_FILE(fd,fullpath,data,nbytes,len) \
    if (LOCAL_DEBUG)                            \
        printf("[Debug][%s][%d][openfile: %s]\n", __FUNCTION__, __LINE__, fullpath); \
    if ((fd = open(fullpath, O_RDONLY)) == -1)  \
       return ONLP_STATUS_E_INTERNAL;           \
    if ((len = read(fd, r_data, nbytes)) <= 0){ \
        close(fd);                              \
        return ONLP_STATUS_E_INTERNAL;}         \
    if (LOCAL_DEBUG)                            \
        printf("[Debug][%s][%d][read data: %s][len:%d]\n", __FUNCTION__, __LINE__, r_data, len); \
    if (close(fd) == -1)                        \
        return ONLP_STATUS_E_INTERNAL


#define PREFIX_PATH_ON_CPLD_DEV          "/sys/bus/i2c/devices/"
#define NUM_OF_CPLD                      3
static char arr_cplddev_name[NUM_OF_CPLD][10] =
{
 "0-0060",
 "0-0061",
 "0-0062"
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as5812-54x-r0";
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
    int   i, siz=NUM_OF_CPLD, v[NUM_OF_CPLD]={0};
    int   fd, len, nbytes = 10;
    char  r_data[10]   = {0};
    char  fullpath[65] = {0};

    for (i=0; i<siz; i++)
    {
        sprintf(fullpath, "%s%s/version", PREFIX_PATH_ON_CPLD_DEV, arr_cplddev_name[i]);
        OPEN_READ_FILE(fd,fullpath,r_data,nbytes,len);
        v[i]=atoi(r_data);
        memset(r_data, 0, len);
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
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));
    int i;
    
    /* 2 PSUs */
    *e++ = ONLP_PSU_ID_CREATE(1);
    *e++ = ONLP_PSU_ID_CREATE(2);

    /* LEDs Item */
    for (i=1; i<=CHASSIS_LED_COUNT; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* THERMALs Item */
    for (i=1; i<=CHASSIS_THERMAL_COUNT; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* Fans Item */
    for (i=1; i<=CHASSIS_FAN_COUNT; i++)
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


	int rc, i;
    int is_up;
    int new_temp, temp1, temp2, diff;
    static int new_perc = 0, ori_perc = 0;
    static int ori_temp = 0;
    onlp_thermal_info_t thermal_info;
    onlp_fan_info_t fan_info;

    /* get new temperature */
    if ((rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(1), &thermal_info)) != ONLP_STATUS_OK)
        goto _EXIT;

    temp1 = thermal_info.mcelsius;

    if ((rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(2), &thermal_info)) != ONLP_STATUS_OK)
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
