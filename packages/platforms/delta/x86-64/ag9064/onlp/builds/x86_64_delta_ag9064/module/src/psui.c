/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2017 Delta Networks, Inc.
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
#include <onlp/platformi/psui.h>
#include "platform_lib.h"


#define VALIDATE(_id)                           \
    do                                          \
    {                                           \
        if(!ONLP_OID_IS_PSU(_id))               \
        {                                       \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t psu_info[] =
{
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0 },
    },
    {
        { ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0 },
    }
};

int onlp_psui_init(void)
{
    lockinit();
    return ONLP_STATUS_OK;
}

static int dni_psu_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int rv = ONLP_STATUS_OK;
    int local_id;
    char device_name[VENDOR_MAX_NAME_SIZE] = {0};
    UINT4 u4Data = 0;
    UINT4 multiplier = 1000;
    char model_name[VENDOR_MAX_NAME_SIZE] = {0};
    char serial_name[VENDOR_MAX_NAME_SIZE] = {0};
    int dev_list_index = FIRST_DEV_INDEX;

    local_id = ONLP_OID_ID_GET(info->hdr.id);

    /* Set the associated oid_table
     * Set PSU's fan and thermal to child OID */
    info->hdr.coids[0] = ONLP_FAN_ID_CREATE(local_id + CHASSIS_FAN_COUNT);
    info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(local_id + CHASSIS_THERMAL_COUNT);

    /* Get psu model name */
    if (dni_bmc_psueeprom_info_get(model_name,
                                   psu_eeprom_info_table[local_id-1].psu_eeprom_table[0].name,
                                   local_id) == ONLP_STATUS_OK)
    {
        strcpy(info->model, model_name);
    }
    else
        strcpy(info->model, "ONLP_STATUS_E_UNSUPPORTED");

    /* Get psu serial number */
    if (dni_bmc_psueeprom_info_get(serial_name,
                                   psu_eeprom_info_table[local_id-1].psu_eeprom_table[1].name,
                                   local_id) == ONLP_STATUS_OK)
    {
        strcpy(info->serial, serial_name);
    }
    else
        strcpy(info->serial, "ONLP_STATUS_E_UNSUPPORTED");

    /* Get psu Vin/Vout */
    sprintf(device_name, "PSU%d_Vin", local_id);
    while (strcmp(psu_dev_list[dev_list_index].dev_name, device_name) != 0)
        dev_list_index++;
    if (dni_bmc_sensor_read(psu_dev_list[dev_list_index].dev_name,
                            &u4Data,
                            multiplier,
                            psu_dev_list[dev_list_index].dev_type) == ONLP_STATUS_OK)
    {
        info->mvin = u4Data;
        info->status = ONLP_PSU_STATUS_PRESENT;
        info->caps |= ONLP_PSU_CAPS_VIN;
    }
    else
        info->caps |= ONLP_PSU_STATUS_UNPLUGGED;

    if (dni_bmc_sensor_read(psu_dev_list[dev_list_index+1].dev_name,
                            &u4Data,
                            multiplier,
                            psu_dev_list[dev_list_index+1].dev_type) == ONLP_STATUS_OK)
    {
        info->mvout = u4Data;
        info->status = ONLP_PSU_STATUS_PRESENT;
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }
    else
        info->caps |= ONLP_PSU_STATUS_UNPLUGGED;

    /* Get psu Iin/Iout */
    if (dni_bmc_sensor_read(psu_dev_list[dev_list_index+2].dev_name,
                            &u4Data,
                            multiplier,
                            psu_dev_list[dev_list_index+2].dev_type) == ONLP_STATUS_OK)
    {
        info->miin = u4Data;
        info->status = ONLP_PSU_STATUS_PRESENT;
        info->caps |= ONLP_PSU_CAPS_IIN;
    }
    else
        info->caps |= ONLP_PSU_STATUS_UNPLUGGED;

    if (dni_bmc_sensor_read(psu_dev_list[dev_list_index+3].dev_name,
                           &u4Data,
                           multiplier,
                           psu_dev_list[dev_list_index+3].dev_type) == ONLP_STATUS_OK)
    {
        info->miout = u4Data;
        info->status = ONLP_PSU_STATUS_PRESENT;
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }
    else
        info->caps |= ONLP_PSU_STATUS_UNPLUGGED;

    /* Get psu Pin/Pout */
    if (dni_bmc_sensor_read(psu_dev_list[dev_list_index+4].dev_name,
                            &u4Data,
                            multiplier,
                            psu_dev_list[dev_list_index+4].dev_type) == ONLP_STATUS_OK)
    {
        info->mpin = u4Data;
        info->status = ONLP_PSU_STATUS_PRESENT;
        info->caps |= ONLP_PSU_CAPS_PIN;
    }
    else
        info->caps |= ONLP_PSU_STATUS_UNPLUGGED;

    if (dni_bmc_sensor_read(psu_dev_list[dev_list_index+5].dev_name,
                            &u4Data,
                            multiplier,
                            psu_dev_list[dev_list_index+5].dev_type) == ONLP_STATUS_OK)
    {
        info->mpout = u4Data;
        info->status = ONLP_PSU_STATUS_PRESENT;
        info->caps |= ONLP_PSU_CAPS_POUT;
    }
    else
        info->caps |= ONLP_PSU_STATUS_UNPLUGGED;

    return rv;
}

int onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int rv = ONLP_STATUS_OK;
    char device_name[VENDOR_MAX_NAME_SIZE] = {0};
    UINT4 u4Data = 0;
    UINT4 multiplier = 1000;
    int index = ONLP_OID_ID_GET(id);
    int dev_list_index = FIRST_DEV_INDEX;

    VALIDATE(id);
    /* Set the onlp_oid_hdr_t */
    memset(info, 0, sizeof(onlp_psu_info_t));
    *info = psu_info[index];

    /* Check PSU have voltage input or not */
    sprintf(device_name, "PSU%d_Vin", index);
    while (strcmp(psu_dev_list[dev_list_index].dev_name, device_name) != 0)
        dev_list_index++;
    if (dni_bmc_sensor_read(psu_dev_list[dev_list_index].dev_name,
                            &u4Data,
                            multiplier,
                            psu_dev_list[dev_list_index].dev_type) == ONLP_STATUS_OK)
    {
        if (u4Data == 0)
        {
            info->status = ONLP_PSU_STATUS_FAILED;
            return rv;
        }
        info->mvin = u4Data;
        info->status = ONLP_PSU_STATUS_PRESENT;
        info->caps |= ONLP_PSU_CAPS_VIN;
    }
    rv = dni_psu_info_get(id, info);

    return rv;
}

int onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
