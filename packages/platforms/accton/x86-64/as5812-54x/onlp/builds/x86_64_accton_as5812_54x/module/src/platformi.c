
#include <onlp/platformi/base.h>
#include "x86_64_accton_as5812_54x_log.h"

const char*
onlp_platformi_get(void)
{
    return "x86-64-accton-as5812-54x-r0";
}

int
onlp_platformi_sw_init(void)
{
    return ONLP_STATUS_OK;
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
onlp_platformi_manage_fans(void)
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
    if ((rc = onlp_thermal_info_get(ONLP_THERMAL_ID_CREATE(1), &thermal_info)) != ONLP_STATUS_OK)
        goto _EXIT;

    temp1 = thermal_info.mcelsius;

    if ((rc = onlp_thermal_info_get(ONLP_THERMAL_ID_CREATE(2), &thermal_info)) != ONLP_STATUS_OK)
        goto _EXIT;

    temp2 = thermal_info.mcelsius;

    new_temp = (temp1+temp2)/2;

    /* check fan status */
    for (i=1; i<=FAN_NUM_ON_MAIN_BROAD; i++)
    {
        if ((rc = onlp_fan_info_get(ONLP_FAN_ID_CREATE(i), &fan_info)) != ONLP_STATUS_OK)
            goto _EXIT;

        if (ONLP_OID_FAILED(&fan_info))
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

    if (ori_perc == new_perc)
        goto _EXIT;

    /* ctrl fans */
    AIM_LOG_INFO("Fan Speeds are now at %d%%", new_perc);

    if ((rc = onlp_fan_percentage_set(ONLP_FAN_ID_CREATE(1), new_perc)) != ONLP_STATUS_OK)
        goto _EXIT;

    /* update om */
    ori_perc = new_perc;
    ori_temp = new_temp;

_EXIT :

    return rc;
}
