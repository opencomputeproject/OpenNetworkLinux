/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2018 Alpha Networks Incorporation.
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
#include <onlp/platformi/psui.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>

#include "x86_64_lenovo_ne10032_int.h"
#include "x86_64_lenovo_ne10032_log.h"
#include "platform_lib.h"
#include <fcntl.h>
#include <unistd.h>

#define DEBUG 0

#define PSU_NUM_ON_MAIN_BROAD         2
#define FAN_NUM_ON_MAIN_BROAD         8
#define LED_NUM_ON_MAIN_BROAD         15
#define THERMAL_NUM_ON_MAIN_BROAD     2

#define SYSTEM_CPLD_I2C_BUS_ID            8
#define SYSTEM_CPLD_I2C_ADDR              0x5F  /* System CPLD Physical Address in the I2C */
#define SYSTEM_CPLD_REVISION_ADDR_OFFSET  0x00

#define ONIE_EEPROM_BUS_ID              0
#define ONIE_EEPROM_ADDR                0x56



#define PLATFORM_STRING "x86-64-lenovo-ne10032-r0"

const char*
onlp_sysi_platform_get(void)
{
    DIAG_PRINT("%s, platform string: %s", __FUNCTION__, PLATFORM_STRING);
    return PLATFORM_STRING;
}

#if 0
int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);

    if (onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK)
    {
        if(*size == 256)
        {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }

    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_INTERNAL;
}

#else  // for EEPROM should be read by "i2cdump -y 0 0x56 c"(consecutive byte)
int
onlp_sysi_onie_data_get(uint8_t **data, int *size)
{
    DIAG_PRINT("%s", __FUNCTION__);
    int ret = 0;
    uint8_t *rdata = aim_zmalloc(256);

    ret = i2c_sequential_read(ONIE_EEPROM_BUS_ID, ONIE_EEPROM_ADDR, 0, 256, (char *)rdata);
    if (ret >= 0)
    {
        *data = rdata;

#if 0//debug
        int i = 0;
        for (i=0;
             i<256;
             i++)
        {
            if ( i%8 == 0)
            {
                AIM_LOG_INFO("\n",rdata[i]);
            }
            AIM_LOG_INFO("0x%2X [%c]",rdata[i],(rdata[i]<=122 && rdata[i] >=65)?rdata[i]:' ');

        }
        AIM_LOG_INFO("\n",rdata[i]);
#endif
        return ONLP_STATUS_OK;
    }

    aim_free(rdata);
    *size = 0;

    return ONLP_STATUS_E_INTERNAL;
}

#endif

void onlp_sysi_onie_data_free(uint8_t *data)
{
    DIAG_PRINT("%s", __FUNCTION__);
    if (data)
        aim_free(data);
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t *pi)
{
    DIAG_PRINT("%s", __FUNCTION__);
    int ret = 0;
    char data = 0;

    ret = i2c_read_byte(SYSTEM_CPLD_I2C_BUS_ID, SYSTEM_CPLD_I2C_ADDR, SYSTEM_CPLD_REVISION_ADDR_OFFSET, &data);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    pi->cpld_versions = aim_fstrdup("%d", (int)data);

    DIAG_PRINT("%s, cpld_versions:%d", __FUNCTION__, data);
    return 0;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t *pi)
{
    DIAG_PRINT("%s", __FUNCTION__);
    aim_free(pi->cpld_versions);
}


int
onlp_sysi_oids_get(onlp_oid_t *table, int max)
{
    DIAG_PRINT("%s, max:%d", __FUNCTION__, max);
    onlp_oid_t *e = table;
    memset(table, 0, max * sizeof(onlp_oid_t));
    int i;

    uint32_t oid = 0;

    /* PSUs */
    for (i = 1; i <= PSU_NUM_ON_MAIN_BROAD; i++)
    {
        oid = ONLP_PSU_ID_CREATE(i);
        *e++ = oid;
        DIAG_PRINT("PSU#%d oid:%d", i, oid);
    }

    /* LEDs */
    for (i = 1; i <= LED_NUM_ON_MAIN_BROAD; i++)
    {
        oid = ONLP_LED_ID_CREATE(i);
        *e++ = oid;
        DIAG_PRINT("LED#%d oid:%d", i, oid);
    }

    /* Thermal sensors */
    for (i = 1; i <= THERMAL_NUM_ON_MAIN_BROAD; i++)
    {
        oid = ONLP_THERMAL_ID_CREATE(i);
        *e++ = oid;
        DIAG_PRINT("THERMAL#%d oid:%d", i, oid);
    }

    /* Fans */
    for (i = 1; i <= FAN_NUM_ON_MAIN_BROAD; i++)
    {
        oid = ONLP_FAN_ID_CREATE(i);
        *e++ = oid;
        DIAG_PRINT("FAN#%d oid:%d", i, oid);
    }

    return 0;
}


int onlp_sysi_platform_manage_init(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    return 0;
}

/*
 * 1. If any FAN failed, set all the other fans as full speed (100%)
 * 2. When (LM75-1 + LM75-2)/2 >= 49.5 C, set fan speed from 50% to 65%.
 * 3. When (LM75-1 + LM75-2)/2 >= 53C, set fan speed from 65% to 80%
 * 4. When (LM75-1 + LM75-2)/2 >= 57.5C, set fan speed from 80% to 100%

 * 5. When (LM75-1 + LM75-2)/2 <= 52.7C, set fan speed from 100% to 80%
 * 6. When (LM75-1 + LM75-2)/2 <= 47.7C, set fan speed from 80% to 65%
 * 7. When (LM75-1 + LM75-2)/2 <= 42.7C, set fan speed from 65% to 50%
 * 8. The default FAN speed is 50%
 */
int
onlp_sysi_platform_manage_fans(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
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
#define LEV4_SPEED_PERC 50

    int rc, i;
    int is_up;
    int new_temp, temp1, temp2, diff;
    static int new_perc = 0, ori_perc = 0;
    static int ori_temp = 0;
    static int fan_failed = 0;
    onlp_thermal_info_t thermal_info;
    onlp_fan_info_t fan_info;

    if (diag_debug_pause_platform_manage_check() == 1) //diag test mode
    {
        return ONLP_STATUS_OK;
    }

    /* get new temperature */
    if ((rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(1), &thermal_info)) != ONLP_STATUS_OK)
        goto _EXIT;

    temp1 = thermal_info.mcelsius;

    if ((rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(2), &thermal_info)) != ONLP_STATUS_OK)
        goto _EXIT;

    temp2 = thermal_info.mcelsius;

    new_temp = (temp1 + temp2) / 2;

    /* check fan status */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
    {
        if ((rc = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info)) != ONLP_STATUS_OK)
            goto _EXIT;

        if (fan_info.status & ONLP_FAN_STATUS_FAILED)
        {
            new_perc = LEV1_SPEED_PERC;
            fan_failed = 1;
            goto _CTRL;
        }

#if 0
#define UNKNOW_SPEED_PERC 0
        if( fan_info.rpm < 8800) //to prevent fan speed set too low by user , 8800 = lower bound speed for 50%
        {
            ori_perc = UNKNOW_SPEED_PERC;
            new_perc = LEV4_SPEED_PERC;
            fan_failed = 1;
            goto _CTRL;
        }
#endif

    }

    if (ori_perc == LEV1_SPEED_PERC && fan_failed == 1)
    {
        fan_failed = 0;
        new_perc = LEV4_SPEED_PERC;
        goto _CTRL;
    }

    diff = new_temp - ori_temp;

    if (diff == 0)
        goto _EXIT;
    else
        is_up = (diff > 0) ? 1 : 0;

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

    if (DEBUG)
        printf("\n[%s][%d]{ori:temp=%d, perc=%d} {new:temp=%d, perc=%d}\n", __FUNCTION__, __LINE__,
               ori_temp, ori_perc, new_temp, new_perc);

    if (ori_perc == new_perc)
        goto _EXIT;

    /* ctrl fans */
#if 1 //CPLD control all fans by one register.
    if ((rc = onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1), new_perc)) != ONLP_STATUS_OK)
        goto _EXIT;

    AIM_LOG_INFO("Fan%d Speeds are now at %d%%", i, new_perc);
#else
    for (i = FAN_1;
         i <= FAN_6;
         i++)
    {
        if ((rc = onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), new_perc)) != ONLP_STATUS_OK)
        goto _EXIT;

        AIM_LOG_INFO("Fan%d Speeds are now at %d%%", i, new_perc);
    }
#endif

    /* update om */
    ori_perc = new_perc;
    ori_temp = new_temp;

_EXIT :

    return rc;
}

int
onlp_sysi_platform_manage_leds(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    int i = 0, j = 0;
    onlp_fan_info_t fan_info;
    onlp_led_mode_t led_mode = ONLP_LED_MODE_OFF;
    onlp_led_mode_t status_led_mode = ONLP_LED_MODE_BLUE_BLINKING;
    onlp_psu_info_t psu1_info, psu2_info;

    int fan_ok_count = 0;
    int psu_ok_count = 0;

    if (diag_debug_pause_platform_manage_check() == 1) //diag test mode
    {
        return ONLP_STATUS_OK;
    }

    /*
     * FAN Indicators
     *
     *     Green - Good
     *     Amber - Present but failed
     *     Off   - Not present
     */
    for (i = FAN_1, j = LED_FAN1; i <= FAN_6; i++, j++)
    {
        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) != ONLP_STATUS_OK)
        {
            /* Get fan status fail */
            led_mode = ONLP_LED_MODE_GREEN_BLINKING;
            //status_led_mode = ONLP_LED_MODE_BLUE_BLINKING;
        }
        else if ((fan_info.status & ONLP_FAN_STATUS_PRESENT) == 0)
        {
            /* Not present -- Off */
            led_mode = ONLP_LED_MODE_GREEN_BLINKING;
        }
        else if (fan_info.status & ONLP_FAN_STATUS_FAILED)
        {
            /* Present but Failed */
            led_mode = ONLP_LED_MODE_GREEN_BLINKING;
            //status_led_mode = ONLP_LED_MODE_BLUE_BLINKING;
        }
        else
        {
            led_mode = ONLP_LED_MODE_GREEN;
            fan_ok_count++;
        }

        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(j), led_mode);
    }

    /*
     * PSU Indicators, only one LED indicator for 2 PSUs. 
     */

    if ((onlp_psui_info_get(ONLP_PSU_ID_CREATE(PSU1_ID), &psu1_info) != ONLP_STATUS_OK) ||
        (onlp_psui_info_get(ONLP_PSU_ID_CREATE(PSU2_ID), &psu2_info) != ONLP_STATUS_OK))
    {
        /* Get PSU status fail */
        led_mode = ONLP_LED_MODE_GREEN_BLINKING;
        //status_led_mode = ONLP_LED_MODE_BLUE_BLINKING;
        printf("%s:%d psu1_info.status:0x%08x, psu2_info.status:0x%08x\n", __FUNCTION__, __LINE__, psu1_info.status, psu2_info.status);
    }
    else if (((psu1_info.status & ONLP_PSU_STATUS_PRESENT) == 0) &&
             ((psu2_info.status & ONLP_PSU_STATUS_PRESENT) == 0))
    {
        /* Not present */
        led_mode = ONLP_LED_MODE_OFF;
    }
    else if ((psu1_info.status & ONLP_PSU_STATUS_FAILED) ||
             (psu2_info.status & ONLP_PSU_STATUS_FAILED))
    {
        /* Present but Failed */
        led_mode = ONLP_LED_MODE_GREEN_BLINKING;
        //status_led_mode = ONLP_LED_MODE_BLUE_BLINKING;
        printf("%s:%d psu1_info.status:0x%08x, psu2_info.status:0x%08x\n", __FUNCTION__, __LINE__, psu1_info.status, psu2_info.status);
    }
    else if ((psu1_info.status & ONLP_PSU_STATUS_UNPLUGGED) &&
             (psu2_info.status & ONLP_PSU_STATUS_UNPLUGGED))
    {
        led_mode = ONLP_LED_MODE_GREEN_BLINKING;
    }
    else
    {
        led_mode = ONLP_LED_MODE_GREEN;
        psu_ok_count = 2;
    }

    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PWR), led_mode);


    if (psu_ok_count == 2 && fan_ok_count == 6)
    {
        status_led_mode = ONLP_LED_MODE_OFF;
    }

    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_SERVICE), status_led_mode);

    return ONLP_STATUS_OK;
}

int
onlp_sysi_init(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    return ONLP_STATUS_OK;
}
int onlp_sysi_debug_diag_fan_status(void)
{
    int oid = 0;
    int i = 0;
    uint32_t status = 0;
    for (i = 1; i <= FAN_NUM_ON_MAIN_BROAD; i++)
    {
        oid = ONLP_FAN_ID_CREATE(i);
        onlp_fani_status_get(oid, &status);
        printf("FAN#%d oid:%d\n", i, oid);
        printf("Status: 0x%x [%s %s]\n", status,
               (status & ONLP_FAN_STATUS_PRESENT) ? "PRESENT" : "",
               (status & ONLP_FAN_STATUS_FAILED) ? "FAILED" : "");
    }
    return 0;

}

int onlp_sysi_debug_diag_fan(void)
{
    onlp_fan_info_t fan_info;

    printf("[Set fan speed to 10%% ...]\n");
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1), 10);
    sleep(2);
    onlp_fani_info_get(ONLP_FAN_ID_CREATE(FAN_1), &fan_info);
    printf("FAN_1 fan_info.percentage = %d\n", fan_info.percentage);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set fan speed to 30%% ...]\n");
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1), 30);
    sleep(2);
    onlp_fani_info_get(ONLP_FAN_ID_CREATE(FAN_1), &fan_info);
    printf("FAN_1 fan_info.percentage = %d\n", fan_info.percentage);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set fan speed to 50%% ...]\n");
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1), 50);
    sleep(2);
    onlp_fani_info_get(ONLP_FAN_ID_CREATE(FAN_1), &fan_info);
    printf("FAN_1 fan_info.percentage = %d\n", fan_info.percentage);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set fan speed to 70%% ...]\n");
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1), 70);
    sleep(2);
    onlp_fani_info_get(ONLP_FAN_ID_CREATE(FAN_1), &fan_info);
    printf("FAN_1 fan_info.percentage = %d\n", fan_info.percentage);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set fan speed to 100%% ...]\n");
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1), 100);
    sleep(2);
    onlp_fani_info_get(ONLP_FAN_ID_CREATE(FAN_1), &fan_info);
    printf("FAN_1 fan_info.percentage = %d\n", fan_info.percentage);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("set fan speed to default(50%%) by onlp_fani_init()\n");
    onlp_fani_init();
    return 0;
}

int onlp_sysi_debug_diag_led(void)
{


    printf("           STACKING  o     o PSU  \n");
    printf("           FAN1~6  -----------    \n");
    printf("                   | o     o |    \n");
    printf("                   | o     o |    \n");
    printf("                   | o     o |    \n");
    printf("  SERVICE o        -----------    \n");


    printf("[Stop platform manage ...]\n");
#if 1
    diag_debug_pause_platform_manage_on();
#else
    onlp_sys_platform_manage_stop(0);
#endif
    sleep(1);

    printf("[Set All LED to OFF ...]\n");
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_SERVICE), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_STACKING), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_PWR), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN1), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN2), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN3), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN4), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN5), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN6), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN1), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN2), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN3), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN4), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN5), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN6), ONLP_LED_MODE_OFF);
    printf("<Press Any Key to Continue>\n");
    getchar();


    printf("[Set SERVICE LED to ONLP_LED_MODE_BLUE ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_SERVICE), ONLP_LED_MODE_BLUE);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set SERVICE LED to ONLP_LED_MODE_BLUE_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_SERVICE), ONLP_LED_MODE_BLUE_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set STACKING LED to ONLP_LED_MODE_ORANGE ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_STACKING), ONLP_LED_MODE_ORANGE);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set STACKING LED to ONLP_LED_MODE_ORANGE_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_STACKING), ONLP_LED_MODE_ORANGE_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set STACKING LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_STACKING), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set STACKING LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_STACKING), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set PSU LED to ONLP_LED_MODE_ORANGE ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PWR), ONLP_LED_MODE_ORANGE);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set PSU LED to ONLP_LED_MODE_ORANGE_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PWR), ONLP_LED_MODE_ORANGE_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set PSU LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PWR), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set PSU LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PWR), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set FAN1 LED to ONLP_LED_MODE_ORANGE ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN1), ONLP_LED_MODE_ORANGE);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN1 LED to ONLP_LED_MODE_ORANGE_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN1), ONLP_LED_MODE_ORANGE_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN1 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN1), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN1 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN1), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set FAN2 LED to ONLP_LED_MODE_ORANGE ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN2), ONLP_LED_MODE_ORANGE);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN2 LED to ONLP_LED_MODE_ORANGE_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN2), ONLP_LED_MODE_ORANGE_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN2 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN2), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN2 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN2), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set FAN3 LED to ONLP_LED_MODE_ORANGE ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN3), ONLP_LED_MODE_ORANGE);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN3 LED to ONLP_LED_MODE_ORANGE_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN3), ONLP_LED_MODE_ORANGE_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN3 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN3), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN3 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN3), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set FAN4 LED to ONLP_LED_MODE_ORANGE ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN4), ONLP_LED_MODE_ORANGE);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN4 LED to ONLP_LED_MODE_ORANGE_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN4), ONLP_LED_MODE_ORANGE_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN4 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN4), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN4 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN4), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set FAN5 LED to ONLP_LED_MODE_ORANGE ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN5), ONLP_LED_MODE_ORANGE);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN5 LED to ONLP_LED_MODE_ORANGE_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN5), ONLP_LED_MODE_ORANGE_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN5 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN5), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN5 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN5), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set FAN6 LED to ONLP_LED_MODE_ORANGE ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN6), ONLP_LED_MODE_ORANGE);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN6 LED to ONLP_LED_MODE_ORANGE_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN6), ONLP_LED_MODE_ORANGE_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN6 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN6), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set FAN6 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN6), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set REAR FAN1 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN1), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set REAR FAN1 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN1), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set REAR FAN2 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN2), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set REAR FAN2 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN2), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set REAR FAN3 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN3), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set REAR FAN3 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN3), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set REAR FAN4 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN4), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set REAR FAN4 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN4), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set REAR FAN5 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN5), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set REAR FAN5 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN5), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set REAR FAN6 LED to ONLP_LED_MODE_GREEN ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN6), ONLP_LED_MODE_GREEN);
    printf("<Press Any Key to Continue>\n");
    getchar();
    printf("[Set REAR FAN6 LED to ONLP_LED_MODE_GREEN_BLINKING ...]\n");
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN6), ONLP_LED_MODE_GREEN_BLINKING);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set All LED to OFF ...]\n");
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_SERVICE), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_STACKING), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_PWR), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN1), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN2), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN3), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN4), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN5), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FAN6), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN1), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN2), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN3), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN4), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN5), ONLP_LED_MODE_OFF);
    onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN6), ONLP_LED_MODE_OFF);

    printf("[Restart platform manage ...]\n");
    onlp_ledi_init();
#if 1
    diag_debug_pause_platform_manage_off();
#else
    onlp_sys_platform_manage_start(0);
#endif
    return 0;
}

#define SFP_DIAG_ADDR 114 //the reserved bytes 114~118 support r/w in SFF-8636 standard
#define SFP_DIAG_PATTEN_B 0xAA
#define SFP_DIAG_PATTEN_W 0xABCD

int onlp_sysi_debug_diag_sfp(int index)
{
    uint8_t *data = NULL;
    int rv = 0;

    uint8_t org_b = 0;
    uint16_t org_w = 0;
    uint8_t temp_b = 0;
    uint16_t temp_w = 0;

    data = aim_zmalloc(256);
    if ((rv = onlp_sfpi_eeprom_read(index, data)) < 0)
    {

        aim_printf(&aim_pvs_stdout, "Error reading eeprom: %{onlp_status}\n");
    }
    else
    {
        aim_printf(&aim_pvs_stdout, "dump eeprom:\n%{data}\n", data, 256);
    }
    aim_free(data);
    data = NULL;

//BYTE
    printf("Read/Write byte test...\n");
    org_b = onlp_sfpi_dev_readb(index, QSFP28_EEPROM_I2C_ADDR, SFP_DIAG_ADDR);
    if (org_b < 0)
    {
        printf("Error, read failed!");
        goto DONE;
    }
    rv = onlp_sfpi_dev_writeb(index, QSFP28_EEPROM_I2C_ADDR, SFP_DIAG_ADDR, SFP_DIAG_PATTEN_B);
    if (rv < 0)
    {
        printf("Error, write failed!");
        goto DONE;
    }
    sleep(2);
    temp_b = onlp_sfpi_dev_readb(index, QSFP28_EEPROM_I2C_ADDR, SFP_DIAG_ADDR);
    if (temp_b < 0)
    {
        printf("Error, read failed!");
        goto DONE;
    }
    if (temp_b != SFP_DIAG_PATTEN_B)
    {
        printf("Error, mismatch!");
        goto DONE;
    }
    rv = onlp_sfpi_dev_writeb(index, QSFP28_EEPROM_I2C_ADDR, SFP_DIAG_ADDR, org_b);
    if (rv < 0)
    {
        printf("Error, write failed!");
        goto DONE;
    }
    sleep(2);
//WORD
    printf("Read/Write word test...\n");
    org_w = onlp_sfpi_dev_readw(index, QSFP28_EEPROM_I2C_ADDR, SFP_DIAG_ADDR);
    if (org_w < 0)
    {
        printf("Error, read failed!");
        goto DONE;
    }
    rv = onlp_sfpi_dev_writew(index, QSFP28_EEPROM_I2C_ADDR, SFP_DIAG_ADDR, SFP_DIAG_PATTEN_W);
    if (rv < 0)
    {
        printf("Error, write failed!");
        goto DONE;
    }
    sleep(2);
    temp_w = onlp_sfpi_dev_readw(index, QSFP28_EEPROM_I2C_ADDR, SFP_DIAG_ADDR);
    if (temp_w < 0)
    {
        printf("Error, read failed!");
        goto DONE;
    }
    if (temp_w != SFP_DIAG_PATTEN_W)
    {
        printf("Error, mismatch!");
        goto DONE;
    }
    rv = onlp_sfpi_dev_writew(index, QSFP28_EEPROM_I2C_ADDR, SFP_DIAG_ADDR, org_w);
    if (rv < 0)
    {
        printf("Error, write failed!");
        goto DONE;
    }

DONE:
    return 0;
}

int onlp_sysi_debug_diag_sfp_dom(int index)
{
    uint8_t *data = NULL;
    int rv = 0;

    data = aim_zmalloc(256);
    if ((rv = onlp_sfpi_dom_read(index, data)) < 0)
    {

        aim_printf(&aim_pvs_stdout, "Error reading dom eeprom: %{onlp_status}\n");
    }
    else
    {
        aim_printf(&aim_pvs_stdout, "dump DOM eeprom:\n%{data}\n", data, 256);
    }
    aim_free(data);
    data = NULL;

    return 0;
}

int onlp_sysi_debug_diag_sfp_ctrl(int index)
{
    int val = 0;

    printf("[Option: %d(%s)... Set]\n", ONLP_SFP_CONTROL_RESET, sfp_control_to_str(ONLP_SFP_CONTROL_RESET));
    printf("[Set %s...]\n", sfp_control_to_str(ONLP_SFP_CONTROL_RESET));
    onlp_sfpi_control_set(index, ONLP_SFP_CONTROL_RESET, 1);
    sleep(1);
    printf("<Press Any Key to Continue>\n");
    getchar();


    printf("[Option: %d(%s)...Set/Get]\n", ONLP_SFP_CONTROL_RESET_STATE, sfp_control_to_str(ONLP_SFP_CONTROL_RESET_STATE));
    printf("[Set %s... to 1]\n", sfp_control_to_str(ONLP_SFP_CONTROL_RESET_STATE));
    onlp_sfpi_control_set(index, ONLP_SFP_CONTROL_RESET_STATE, 1);
    sleep(1);
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_RESET_STATE));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_RESET_STATE, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set %s... to 0]\n", sfp_control_to_str(ONLP_SFP_CONTROL_RESET_STATE));
    onlp_sfpi_control_set(index, ONLP_SFP_CONTROL_RESET_STATE, 0);
    sleep(1);
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_RESET_STATE));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_RESET_STATE, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Option: %d(%s)...Get]\n", ONLP_SFP_CONTROL_RX_LOS, sfp_control_to_str(ONLP_SFP_CONTROL_RX_LOS));
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_RX_LOS));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_RX_LOS, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Option: %d(%s)...Get]\n", ONLP_SFP_CONTROL_TX_FAULT, sfp_control_to_str(ONLP_SFP_CONTROL_TX_FAULT));
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_TX_FAULT));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_TX_FAULT, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Option: %d(%s)...Set/Get]\n", ONLP_SFP_CONTROL_TX_DISABLE, sfp_control_to_str(ONLP_SFP_CONTROL_TX_DISABLE));
    printf("[Set %s... to 1]\n", sfp_control_to_str(ONLP_SFP_CONTROL_TX_DISABLE));
    onlp_sfpi_control_set(index, ONLP_SFP_CONTROL_TX_DISABLE, 1);
    sleep(1);
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_TX_DISABLE));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_TX_DISABLE, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set %s... to 0]\n", sfp_control_to_str(ONLP_SFP_CONTROL_TX_DISABLE));
    onlp_sfpi_control_set(index, ONLP_SFP_CONTROL_TX_DISABLE, 0);
    sleep(1);
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_TX_DISABLE));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_TX_DISABLE, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Option: %d(%s)...Set/Get]\n", ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL, sfp_control_to_str(ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL));
    printf("[Set %s... to 0x05]\n", sfp_control_to_str(ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL));
    onlp_sfpi_control_set(index, ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL, 0x05);
    sleep(1);
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set %s... to 0]\n", sfp_control_to_str(ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL));
    onlp_sfpi_control_set(index, ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL, 0);
    sleep(1);
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Option: %d(%s)...Set/Get]\n", ONLP_SFP_CONTROL_LP_MODE, sfp_control_to_str(ONLP_SFP_CONTROL_LP_MODE));
    printf("[Set %s... to 1]\n", sfp_control_to_str(ONLP_SFP_CONTROL_LP_MODE));
    onlp_sfpi_control_set(index, ONLP_SFP_CONTROL_LP_MODE, 1);
    sleep(1);
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_LP_MODE));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_LP_MODE, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set %s... to 0]\n", sfp_control_to_str(ONLP_SFP_CONTROL_LP_MODE));
    onlp_sfpi_control_set(index, ONLP_SFP_CONTROL_LP_MODE, 0);
    sleep(1);
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_LP_MODE));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_LP_MODE, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Option: %d(%s)...Set/Get]\n", ONLP_SFP_CONTROL_POWER_OVERRIDE, sfp_control_to_str(ONLP_SFP_CONTROL_POWER_OVERRIDE));
    printf("[Set %s... to 1]\n", sfp_control_to_str(ONLP_SFP_CONTROL_POWER_OVERRIDE));
    onlp_sfpi_control_set(index, ONLP_SFP_CONTROL_POWER_OVERRIDE, 1);
    sleep(1);
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_POWER_OVERRIDE));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_POWER_OVERRIDE, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    printf("[Set %s... to 0]\n", sfp_control_to_str(ONLP_SFP_CONTROL_POWER_OVERRIDE));
    onlp_sfpi_control_set(index, ONLP_SFP_CONTROL_POWER_OVERRIDE, 0);
    sleep(1);
    printf("[Get %s... ]\n", sfp_control_to_str(ONLP_SFP_CONTROL_POWER_OVERRIDE));
    onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_POWER_OVERRIDE, &val);
    printf("<Press Any Key to Continue>\n");
    getchar();

    return 0;
}

int
onlp_sysi_debug(aim_pvs_t *pvs, int argc, char *argv[])
{
    int ret = 0;

    /* ONLPI driver APIs debug */

    if (argc > 0 && !strcmp(argv[0], "sys"))
    {
        diag_flag_set(DIAG_FLAG_ON);
        printf("DIAG for SYS: \n");
        printf("Platform : %s\n", onlp_sysi_platform_get());
        onlp_sysi_init();
        onlp_sysi_platform_manage_init();
        diag_flag_set(DIAG_FLAG_OFF);
    }
    else if (argc > 0 && !strcmp(argv[0], "fan_rpm"))
    {
        onlp_fan_info_t fan_info;
        int i = 0;
        diag_flag_set(DIAG_FLAG_ON);
        printf("DIAG for FAN rpm: \n");

        int rpm = 0;
        if (argc != 2)
        {
            printf("Parameter error, format: onlpdump debugi fan_rpm [RPM]\n");
            return -1;
        }
        rpm = atoi(argv[1]);
        onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(FAN_1), rpm);

        sleep(5);
        for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
        {
            onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info);
            printf("FAN#%d RPM:%d\n", i, fan_info.rpm);
        }

        diag_flag_set(DIAG_FLAG_OFF);
    }
    else if (argc > 0 && !strcmp(argv[0], "fan_status"))
    {
        diag_flag_set(DIAG_FLAG_ON);
        printf("DIAG for FAN status: \n");
        onlp_sysi_debug_diag_fan_status();
        diag_flag_set(DIAG_FLAG_OFF);
    }
    else if (argc > 0 && !strcmp(argv[0], "fan"))
    {
        diag_flag_set(DIAG_FLAG_ON);
        onlp_sysi_debug_diag_fan();
        diag_flag_set(DIAG_FLAG_OFF);
    }
    else if (argc > 0 && !strcmp(argv[0], "psu"))
    {
        printf("DIAG for PSU: \n");
        diag_flag_set(DIAG_FLAG_ON);
        onlp_psui_init();
        diag_flag_set(DIAG_FLAG_OFF);

    }
    else if (argc > 0 && !strcmp(argv[0], "led"))
    {
        printf("DIAG for LED: \n");
        diag_flag_set(DIAG_FLAG_ON);
        onlp_sysi_debug_diag_led();
        diag_flag_set(DIAG_FLAG_OFF);
    }
    else if (argc > 0 && !strcmp(argv[0], "sfp_dom"))
    {
        int port_index = 0;
        if (argc != 2)
        {
            printf("Parameter error, format: onlpdump debugi sfp_dom [PORT]\n");
            return -1;
        }
        port_index = atoi(argv[1]);
        if (port_index <= 0 || port_index > NUM_OF_SFP_PORT)
        {
            printf("Parameter error, PORT out of range.\n");
            return -1;
        }
        printf("DIAG for SFP DOM #%d: \n", port_index - 1);
        diag_flag_set(DIAG_FLAG_ON);
        onlp_sysi_debug_diag_sfp_dom(port_index - 1);
        diag_flag_set(DIAG_FLAG_OFF);
    }
    else if (argc > 0 && !strcmp(argv[0], "sfp_ctrl_set"))
    {
        int port_index = 0, ctrl = 0, val = 0;
        if (argc != 4)
        {
            printf("Parameter error, format: onlpdump debugi sfp_ctrl_set [PORT] [CTRL] [VALUE]\n");
            return -1;
        }
        port_index = atoi(argv[1]);
        if (port_index <= 0 || port_index > NUM_OF_SFP_PORT)
        {
            printf("Parameter error, PORT out of range.\n");
            return -1;
        }
        ctrl = atoi(argv[2]);
        val = atoi(argv[3]);
        diag_flag_set(DIAG_FLAG_ON);
        onlp_sfpi_control_set(port_index - 1, ctrl, val);
        diag_flag_set(DIAG_FLAG_OFF);

    }
    else if (argc > 0 && !strcmp(argv[0], "sfp_ctrl_get"))
    {
        int port_index = 0, ctrl = 0, val = 0;
        if (argc != 3)
        {
            printf("Parameter error, format: onlpdump debugi sfp_ctrl_get [PORT] [CTRL] \n");
            return -1;
        }
        port_index = atoi(argv[1]);
        if (port_index <= 0 || port_index > NUM_OF_SFP_PORT)
        {
            printf("Parameter error, PORT out of range.\n");
            return -1;
        }
        ctrl = atoi(argv[2]);
        diag_flag_set(DIAG_FLAG_ON);
        onlp_sfpi_control_get(port_index - 1, ctrl, &val);
        printf("Value = %d(0x%X)\n", val, val);
        diag_flag_set(DIAG_FLAG_OFF);

    }
    else if (argc > 0 && !strcmp(argv[0], "sfp_ctrl"))
    {
        int port_index = 0;
        if (argc != 2)
        {
            printf("Parameter error, format: onlpdump debugi sfp_ctrl [PORT]\n");
            return -1;
        }
        port_index = atoi(argv[1]);
        if (port_index <= 0 || port_index > NUM_OF_SFP_PORT)
        {
            printf("Parameter error, PORT out of range.\n");
            return -1;
        }

        printf("DIAG for SFP Control #%d: \n", port_index - 1);
        diag_flag_set(DIAG_FLAG_ON);
        onlp_sysi_debug_diag_sfp_ctrl(port_index - 1);
        diag_flag_set(DIAG_FLAG_OFF);
    }
    else if (argc > 0 && !strcmp(argv[0], "sfp"))
    {
        if (argc > 1)
        {
            int port_index = atoi(argv[1]);
            if (port_index <= 0 || port_index > NUM_OF_SFP_PORT)
            {
                printf("Parameter error, PORT out of range.\n");
                return -1;
            }
            printf("DIAG for SFP#%d: \n", port_index - 1);
            diag_flag_set(DIAG_FLAG_ON);
            onlp_sysi_debug_diag_sfp(port_index - 1);
            diag_flag_set(DIAG_FLAG_OFF);
        }
        else
        {
            printf("DIAG for SFP: \n");
            onlp_sfp_bitmap_t bmap;
            diag_flag_set(DIAG_FLAG_ON);

            onlp_sfpi_denit();
            onlp_sfpi_init();

            onlp_sfp_bitmap_t_init(&bmap);
            ret = onlp_sfpi_bitmap_get(&bmap);
            if (ret < 0)
            {
                printf("Error, onlp_sfpi_bitmap_get failed!\n");
            }
            else
            {
                aim_printf(&aim_pvs_stdout, "sfp_bitmap:\n  %{aim_bitmap}\n", &bmap);
            }
            diag_flag_set(DIAG_FLAG_OFF);

            return 0;
        }
    }
    else if (argc > 0 && !strcmp(argv[0], "sfpwb")) //write byte
    {
        int port;
        uint8_t addr, value;

        if (argc == 4)
        {
            port = atoi(argv[1]);
            addr = (uint8_t)atoi(argv[2]);
            value = (uint8_t)atoi(argv[3]);

            if (port <= 0 || port > NUM_OF_SFP_PORT)
            {
                printf("Parameter error, PORT out of range.\n");
                return -1;
            }

            diag_flag_set(DIAG_FLAG_ON);
            onlp_sfpi_dev_writeb(port - 1, QSFP28_EEPROM_I2C_ADDR, addr, value);
            diag_flag_set(DIAG_FLAG_OFF);
        }
        else
        {
            printf("Parameter error, format: onlpdump debugi sfpwb [PORT] [ADDR] [VALUE]\n");
            return -1;
        }

    }
    else if (argc > 0 && !strcmp(argv[0], "sfprb")) //read byte
    {
        int port;
        uint8_t addr;
        if (argc == 3)
        {
            port = atoi(argv[1]);
            addr = (uint8_t)atoi(argv[2]);

            if (port <= 0 || port > NUM_OF_SFP_PORT)
            {
                printf("Parameter error, PORT out of range.\n");
                return -1;
            }

            diag_flag_set(DIAG_FLAG_ON);
            onlp_sfpi_dev_readb(port - 1, QSFP28_EEPROM_I2C_ADDR, addr);
            diag_flag_set(DIAG_FLAG_OFF);
        }
        else
        {
            printf("Parameter error, format: onlpdump debugi sfprb [PORT] [ADDR]\n");
            return -1;
        }
    }
    else if (argc > 0 && !strcmp(argv[0], "sfpww")) //write word
    {
        int port;
        uint16_t value;
        uint8_t addr;

        if (argc == 4)
        {
            port = atoi(argv[1]);
            addr = (uint8_t)atoi(argv[2]);
            value = (uint16_t)atoi(argv[3]);

            if (port <= 0 || port > NUM_OF_SFP_PORT)
            {
                printf("Parameter error, PORT out of range.\n");
                return -1;
            }

            diag_flag_set(DIAG_FLAG_ON);
            onlp_sfpi_dev_writew(port - 1, QSFP28_EEPROM_I2C_ADDR, addr, value);
            diag_flag_set(DIAG_FLAG_OFF);
        }
        else
        {
            printf("Parameter error, format: onlpdump debugi sfpwb [PORT] [ADDR] [VALUE]\n");
            return -1;
        }
    }
    else if (argc > 0 && !strcmp(argv[0], "sfprw")) //read word
    {
        int port;
        uint8_t addr;
        if (argc == 3)
        {
            port = atoi(argv[1]);
            addr = (uint8_t)atoi(argv[2]);

            if (port <= 0 || port > NUM_OF_SFP_PORT)
            {
                printf("Parameter error, PORT out of range.\n");
                return -1;
            }

            diag_flag_set(DIAG_FLAG_ON);
            onlp_sfpi_dev_readw(port - 1, QSFP28_EEPROM_I2C_ADDR, addr);
            diag_flag_set(DIAG_FLAG_OFF);
        }
        else
        {
            printf("Parameter error, format: onlpdump debugi sfprb [PORT] [ADDR]\n");
            return -1;
        }
    }
    else if (argc > 0 && !strcmp(argv[0], "thermal"))
    {
        printf("DIAG for Thermal: \n");
        diag_flag_set(DIAG_FLAG_ON);
        onlp_thermali_init();
        diag_flag_set(DIAG_FLAG_OFF);
    }
    else if (argc > 0 && !strcmp(argv[0], "trace_on"))
    {
        diag_debug_trace_on();
        DIAG_PRINT("%s, ONLPI TRACE: ON", __FUNCTION__);
    }
    else if (argc > 0 && !strcmp(argv[0], "trace_off"))
    {
        diag_debug_trace_off();
        DIAG_PRINT("%s, ONLPI TRACE: OFF", __FUNCTION__);
    }
    else if (argc > 0 && !strcmp(argv[0], "help"))
    {
        printf("\nUsage: onlpdump debugi [OPTION]\n");
        printf("    help                : this message.\n");
        printf("    trace_on            : turn on ONLPI debug trace message output on screen.\n");
        printf("    trace_off           : turn off ONLPI debug trace message output on screen.\n");
        printf("    sys                 : run system ONLPI diagnostic function.\n");
        printf("    fan                 : run fan ONLPI diagnostic function.\n");
        printf("    fan_status          : run fan status ONLPI diagnostic function.\n");
        printf("    fan_rpm             : run fan RPM ONLPI diagnostic function.\n");
        printf("    led                 : run LED ONLPI diagnostic function.\n");
        printf("    psu                 : run psu ONLPI diagnostic function.\n");
        printf("    thermal             : run thermal ONLPI diagnostic function.\n");
        printf("    sfp                 : run sfp ONLPI diagnostic function.\n");
        printf("    sfp [PORT]          : run sfp ONLPI diagnostic function.\n");
        printf("    sfp_dom [PORT]      : run sfp dom ONLPI diagnostic function.\n");
        printf("    sfp_ctrl [PORT]     : run sfp control ONLPI diagnostic function.\n");

        printf("    (Warning! Please be careful to write a value to SFP,\n");
        printf("     you should keep the original value to prevent lose it forever.)\n");
        printf("    sfprb [PORT] [ADDR] : read a byte from sfp transeciver.\n");
        printf("    sfprw [PORT] [ADDR] : read a word from sfp transeciver.\n");
        printf("    sfpwb [PORT] [ADDR] [VALUE] : write a byte to sfp transeciver.\n");
        printf("    sfpww [PORT] [ADDR] [VALUE] : write a word to sfp transeciver.\n");

        printf("                        [PORT] is the port index start from 0.\n");
        printf("                        [ADDR] is the address to read/write.\n");
        printf("                        [VALUE] is the value to read/write.\n");


    }
    else if (argc > 0 && !strcmp(argv[0], "test")) //for RD debug test
    {
        diag_flag_set(DIAG_FLAG_ON);
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_SERVICE), ONLP_LED_MODE_BLUE_BLINKING);
    }
    else
    {}

    return 0;
}





