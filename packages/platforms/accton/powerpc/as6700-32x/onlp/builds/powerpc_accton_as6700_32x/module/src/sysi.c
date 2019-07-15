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
#include <onlp/platformi/sysi.h>
#include "powerpc_accton_as6700_32x_log.h"
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlplib/mmap.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#define HW_INFO_PATH "/dev/mtd3"
#define HW_INFO_LENGTH 65536

#define THERMAL_NUM_ON_MAIN_BROAD  8
#define FAN_NUM_ON_MAIN_BROAD      5

typedef enum
{
    HPC_THERMAL_POLICY_NONE = 0,
    HPC_THERMAL_POLICY_REDUNDANT_ISKU,
    HPC_THERMAL_POLICY_REDUNDANT_ESKU,
    HPC_THERMAL_POLICY_NONREDUNDANT_ISKU,
    HPC_THERMAL_POLICY_NONREDUNDANT_ESKU,
    HPC_THERMAL_POLICY_MISMATCH_ISKU,
    HPC_THERMAL_POLICY_MISMATCH_ESKU,
    HPC_THERMAL_POLICY_SHUTDOWAN,
} HPC_THERMAL_POLICY_t;

typedef struct thermalPolicyTemp
{
    HPC_THERMAL_POLICY_t policy;
    int  tempCritical; /* Tc */
    int  tempWarning;  /* Ta */
    int  tempNormal;   /* Tb */
    unsigned char fanLowSpeed;
    unsigned char fanHighSpeed;
} thermalPolicyTemp_t;

thermalPolicyTemp_t thermalPolicyTempData[] = {
    {HPC_THERMAL_POLICY_MISMATCH_ESKU, 79, 68, 62, 0x5, 0x7},
    {HPC_THERMAL_POLICY_MISMATCH_ISKU, 70, 63, 57, 0x5, 0x7},
    {HPC_THERMAL_POLICY_NONREDUNDANT_ESKU, 82, 71, 65, 0x5, 0x7},
    {HPC_THERMAL_POLICY_NONREDUNDANT_ISKU, 73, 66, 60, 0x5, 0x7},
    {HPC_THERMAL_POLICY_REDUNDANT_ESKU, 78, 67, 61, 0x4, 0x7},
    {HPC_THERMAL_POLICY_REDUNDANT_ISKU, 70, 60, 55, 0x4, 0x7}
};

typedef struct thermalPolicyFanCount
{
    int numofIntakeFan;
    int numofExhaustFan;
    int numofIntakePsu;
    int numofExhaustPsu;
    HPC_THERMAL_POLICY_t policy;
} thermalPolicyFanCount_t;

thermalPolicyFanCount_t thermalPolicyFanData[] = {
    {4, 0, 1, 0, HPC_THERMAL_POLICY_NONREDUNDANT_ISKU},
    {5, 0, 1, 0, HPC_THERMAL_POLICY_NONREDUNDANT_ISKU},
    {4, 0, 2, 0, HPC_THERMAL_POLICY_NONREDUNDANT_ISKU},
    {0, 4, 0, 1, HPC_THERMAL_POLICY_NONREDUNDANT_ESKU},
    {0, 5, 0, 1, HPC_THERMAL_POLICY_NONREDUNDANT_ESKU},
    {0, 4, 0, 2, HPC_THERMAL_POLICY_NONREDUNDANT_ESKU},
    {5, 0, 2, 0, HPC_THERMAL_POLICY_REDUNDANT_ISKU},
    {0, 5, 0, 2, HPC_THERMAL_POLICY_REDUNDANT_ESKU},
    {4, 1, 1, 0, HPC_THERMAL_POLICY_MISMATCH_ISKU},
    {1, 4, 0, 1, HPC_THERMAL_POLICY_MISMATCH_ESKU},
    {4, 1, 2, 0, HPC_THERMAL_POLICY_MISMATCH_ISKU},
    {1, 4, 0, 2, HPC_THERMAL_POLICY_MISMATCH_ESKU},
    {4, 0, 1, 1, HPC_THERMAL_POLICY_MISMATCH_ISKU},
    {0, 4, 1, 1, HPC_THERMAL_POLICY_MISMATCH_ESKU},
    {5, 0, 1, 1, HPC_THERMAL_POLICY_MISMATCH_ISKU},
    {0, 5, 1, 1, HPC_THERMAL_POLICY_MISMATCH_ESKU},
    {4, 1, 1, 1, HPC_THERMAL_POLICY_MISMATCH_ISKU},
    {1, 4, 1, 1, HPC_THERMAL_POLICY_MISMATCH_ESKU}
};

uint8_t hw_info[HW_INFO_LENGTH]={0};

const char*
onlp_sysi_platform_get(void)
{
    return "powerpc-accton-as6700-rX";
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
onlp_sysi_platform_set(const char* platform)
{
    if(strstr(platform, "powerpc-accton-as6700-32x-r")) {
        return ONLP_STATUS_OK;
    }
    if(strstr(platform, "powerpc-as6700-32x-r")) {
        return ONLP_STATUS_OK;
    }
    AIM_LOG_ERROR("No support for platform '%s'", platform);
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    int n_thermal = THERMAL_NUM_ON_MAIN_BROAD, n_fan = FAN_NUM_ON_MAIN_BROAD;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* THERMALs Item */
    /* Sensor status and temperature,
       the temp1 does not have temp_fault.*/
    for (i=1; i<= n_thermal; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    for (i=1; i<= n_fan; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 7 LEDs */
    *e++ = ONLP_LED_ID_CREATE(1);
    *e++ = ONLP_LED_ID_CREATE(2);
    *e++ = ONLP_LED_ID_CREATE(3);
    *e++ = ONLP_LED_ID_CREATE(4);
    *e++ = ONLP_LED_ID_CREATE(5);
    *e++ = ONLP_LED_ID_CREATE(6);
    *e++ = ONLP_LED_ID_CREATE(7);

    /* 2 PSUs */
    *e++ = ONLP_PSU_ID_CREATE(1);
    *e++ = ONLP_PSU_ID_CREATE(2);

    return 0;
}

int
onlp_sysi_platform_manage_fans(void)
{
#define PSU_NUM_ON_MAIN_BROAD  2
#define FAN_NUM_ON_MAIN_BROAD_AND_PSU FAN_NUM_ON_MAIN_BROAD + PSU_NUM_ON_MAIN_BROAD
#define SYS_TEMPERATURE_MULTIPLIER 1000
#define LOCAL_DEBUG     0

    unsigned char new_level;
    static int new_perc = 0;
    int rc, i;
    int num_of_i_fan_present = 0, num_of_e_fan_present = 0;
    int num_of_psu_i_fan_present = 0, num_of_psu_e_fan_present = 0;
    float max_temp = 0;
    static unsigned char ori_level = 0;
    onlp_thermal_info_t thermal_info;
    onlp_fan_info_t fan_info;
    HPC_THERMAL_POLICY_t policy = HPC_THERMAL_POLICY_NONE;

    /* check thermal status */
    for (i=1; i<= THERMAL_NUM_ON_MAIN_BROAD; i++)
    {
        if ((rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i), &thermal_info)) != ONLP_STATUS_OK)
          goto _EXIT;

        if (LOCAL_DEBUG)
            printf("[Debug][%s][%d][number of fan: %d]\n", __FUNCTION__, __LINE__, thermal_info.mcelsius);

        if (i == 1) /* TR1_CPU_TCASE = TR1_CPU_TJ -15C */
            max_temp = thermal_info.mcelsius - 15 * SYS_TEMPERATURE_MULTIPLIER;
        else if (max_temp < thermal_info.mcelsius)
            max_temp = thermal_info.mcelsius;
    }

    /* check fan status */
    for (i=1; i<=FAN_NUM_ON_MAIN_BROAD_AND_PSU; i++)
    {
        if ((rc = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info)) != ONLP_STATUS_OK)
            goto _EXIT;

        if ((i<= FAN_NUM_ON_MAIN_BROAD) && (fan_info.status&ONLP_FAN_STATUS_PRESENT))
        {
            if (fan_info.status&ONLP_FAN_STATUS_F2B)
            {
                num_of_i_fan_present++;
            }
            else if (fan_info.status&ONLP_FAN_STATUS_B2F)
            {
                num_of_e_fan_present++;
            }
        }
        else if ((FAN_NUM_ON_MAIN_BROAD < i) && (fan_info.status&ONLP_FAN_STATUS_PRESENT) && (0 < fan_info.rpm))
        {
            if (fan_info.status&ONLP_FAN_STATUS_F2B)
            {
                num_of_psu_i_fan_present++;
            }
            else if (fan_info.status&ONLP_FAN_STATUS_B2F)
            {
                num_of_psu_e_fan_present++;
            }
        }
    }

    if (LOCAL_DEBUG)
        printf("[Debug][%s][%d][number of fan: %d, %d, %d, %d, %f]\n", __FUNCTION__, __LINE__, num_of_i_fan_present, num_of_e_fan_present,
                                                                             num_of_psu_i_fan_present, num_of_psu_e_fan_present, max_temp);
    /* find policy */
    for (i=0; i < sizeof(thermalPolicyFanData) / sizeof(thermalPolicyFanCount_t); i++)
    {
        if ((thermalPolicyFanData[i].numofIntakeFan == num_of_i_fan_present) &&
            (thermalPolicyFanData[i].numofExhaustFan == num_of_e_fan_present) &&
            (thermalPolicyFanData[i].numofIntakePsu == num_of_psu_i_fan_present)&&
            (thermalPolicyFanData[i].numofExhaustPsu == num_of_psu_e_fan_present))
        {
            policy = thermalPolicyFanData[i].policy;
            break;
        }
    }

    /* not match policy rule in the FRU table, setting shutdown process*/
    if (policy == HPC_THERMAL_POLICY_NONE)
    {
        /* shutdown */
        policy = HPC_THERMAL_POLICY_REDUNDANT_ISKU;
    }

    /* find thermal profile */
    for (i=0; i < sizeof(thermalPolicyTempData) / sizeof(thermalPolicyTemp_t); i++)
    {
        if (policy == thermalPolicyTempData[i].policy)
        {
            if (max_temp <= thermalPolicyTempData[i].tempNormal*SYS_TEMPERATURE_MULTIPLIER)
            {
                new_level = thermalPolicyTempData[i].fanLowSpeed;
            }
            else if ((thermalPolicyTempData[i].tempWarning*SYS_TEMPERATURE_MULTIPLIER < max_temp) &&
                     (max_temp <= thermalPolicyTempData[i].tempCritical*SYS_TEMPERATURE_MULTIPLIER) )
            {
                new_level = thermalPolicyTempData[i].fanHighSpeed;
            }
            else if ( thermalPolicyTempData[i].tempCritical*SYS_TEMPERATURE_MULTIPLIER < max_temp)
            {
                /* shutdown */
                new_level = thermalPolicyTempData[i].fanHighSpeed;
            }
            else
            {
                goto _EXIT;
            }
            break;
        }
    }

    if (ori_level != new_level)
    {
        ori_level = new_level;
        switch (new_level)
        {
            case 3:
                new_perc = 37;
                break;
            case 4:
                new_perc = 50;
                break;
            case 5:
                new_perc = 63;
                break;
            case 6:
                new_perc = 75;
                break;
            case 7:
                new_perc = 100;
                break;
            default:
                break;
        }
    }
    else
    {
        goto _EXIT;
    }

    if (LOCAL_DEBUG)
        printf("\n[DEBUG][%s][%d]{perc=%d}\n", __FUNCTION__, __LINE__, new_perc);

    AIM_LOG_INFO("Fans are now at %d%%", new_perc);
    /* ctrl fans */
    if ((rc = onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), new_perc)) != ONLP_STATUS_OK)
        goto _EXIT;

_EXIT :

    return rc;
}


/*
 * 1. system led is control by CPLD, it shall always green after booting
 * 2. When FAN/PSU/THERMAL is fail , alarm led shall be blink red.
 */
int
onlp_sysi_platform_manage_leds(void)
{
#define PSU_NUM_ON_MAIN_BROAD  2
#define LOCAL_DEBUG     0

    int rc, i;
    onlp_led_info_t led_info;
    onlp_thermal_info_t thermal_info;
    onlp_fan_info_t fan_info;
    onlp_psu_info_t psu_info;
    static int ori_status=0;
    static int new_status=0;
    /*int led_mode[]={ONLP_LED_MODE_OFF,ONLP_LED_MODE_RED_BLINKING,ONLP_LED_MODE_GREEN};*/

    /* get led status */
    if ((rc = onlp_ledi_info_get(ONLP_LED_ID_CREATE(2), &led_info)) != ONLP_STATUS_OK)
        goto _EXIT;

    ori_status=led_info.mode;

    /* check thermal status */
    for (i=1; i<= THERMAL_NUM_ON_MAIN_BROAD; i++)
    {
        if ((rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i), &thermal_info)) != ONLP_STATUS_OK)
            goto _EXIT;

        if((thermal_info.status& ONLP_THERMAL_STATUS_FAILED)&&(thermal_info.status& ONLP_FAN_STATUS_PRESENT))
        {
            new_status = ONLP_LED_MODE_RED_BLINKING;
            goto _CTRL;
        }
    }


    /* check fan status */
    for (i=1; i<=FAN_NUM_ON_MAIN_BROAD; i++)
    {
        if ((rc = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info)) != ONLP_STATUS_OK)
            goto _EXIT;

        if ((fan_info.status & ONLP_FAN_STATUS_FAILED)&&(fan_info.status& ONLP_FAN_STATUS_PRESENT))
        {
            new_status = ONLP_LED_MODE_RED_BLINKING;
            goto _CTRL;
        }
    }

    /* check psu status */
    for (i=1; i<=PSU_NUM_ON_MAIN_BROAD; i++)
    {
        if ((rc = onlp_psui_info_get(ONLP_PSU_ID_CREATE(i), &psu_info)) != ONLP_STATUS_OK)
            goto _EXIT;

        if ((psu_info.status & ONLP_PSU_STATUS_FAILED)&&
             (psu_info.status& ONLP_PSU_STATUS_PRESENT)&&
             (psu_info.mvin>0))
        {
            new_status = ONLP_LED_MODE_RED_BLINKING;
            goto _CTRL;
        }
    }

    new_status = ONLP_LED_MODE_GREEN;

_CTRL :

    if (LOCAL_DEBUG)
        printf("\n[DEBUG][%s][%d]{ori:status=%d} {new:status=%d}\n", __FUNCTION__, __LINE__,
                ori_status, new_status);

    if (ori_status == new_status)
        goto _EXIT;

    /* ctrl alarm led */
    if ((rc = onlp_led_mode_set(ONLP_LED_ID_CREATE(2), new_status)) != ONLP_STATUS_OK)
        goto _EXIT;

    /* update om */
    ori_status = new_status;

_EXIT :

    return rc;
}

#include <onlplib/i2c.h>

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int v = onlp_i2c_readb(1, 0x31, 0, ONLP_I2C_F_FORCE);
    if(v >= 0) {
        pi->cpld_versions = aim_fstrdup("%d.%d", (v>>4), (v & 0xf));
    }
    return 0;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}


