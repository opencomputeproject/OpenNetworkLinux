/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 (C) Delta Networks, Inc.
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
#include "x86_64_delta_agc7648sv1_int.h"
#include <onlplib/i2c.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0 },
    },
    {
        { ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0 },
    }
};

static int dni_psu_pmbus_info_get(int id, char *node, int *value)
{
    int ret = ONLP_STATUS_OK;
    char node_path[PSU_NODE_MAX_PATH_LEN] = {0};

    *value = 0;

    switch (id) {
        case PSU1_ID:
            sprintf(node_path, "%s%s", PSU1_AC_PMBUS_PREFIX, node);
            break;
        case PSU2_ID:
            sprintf(node_path, "%s%s", PSU2_AC_PMBUS_PREFIX, node);
            break;
        default:
            break;
    }
    /* Read attribute value */
    *value = dni_i2c_lock_read_attribute(NULL, node_path);

    return ret;
}

int onlp_psui_init(void)
{
    lockinit();
    return ONLP_STATUS_OK;
}

static int dni_psu_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int i = 0;
    int ret = ONLP_STATUS_OK;
    int local_id;
    char device_name[10] = {0};
    UINT4 u4Data = 0;
    UINT4 multiplier = 1000;
    char name[20] = {0};
    char name1[20] = {0};
    char *module_name = name;
    char *module_name1 = name1;
    int val = 0;
    char val_char[16] = {'\0'};
    char node_path[PSU_NODE_MAX_PATH_LEN] = {'\0'};

    local_id = ONLP_OID_ID_GET(info->hdr.id);

    /* Set the associated oid_table
     * Set PSU's fan and thermal to child OID */
    info->hdr.coids[0] = ONLP_FAN_ID_CREATE(local_id + CHASSIS_FAN_COUNT);
    info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(local_id + CHASSIS_THERMAL_COUNT);
    if(dni_bmc_check() == BMC_ON)
    {
        /* get psu model name */
        if(dni_psui_eeprom_info_get(name, "Product Name", local_id) == ONLP_STATUS_OK)
        {
            for(i = 0; i < PSU_NUM_LENGTH; i++)
                name[i] = *(module_name + i);
            strcpy(info->model, module_name);
        }
        else
            strcpy(info->model, "ONLP_STATUS_E_UNSUPPORTED");

        /* get psu serial number */
        if(dni_psui_eeprom_info_get(name1, "Product Serial", local_id) == ONLP_STATUS_OK)
        {
            for(i = 0; i < PSU_NUM_LENGTH; i++)
                name1[i] = *(module_name1 + i);
            strcpy(info->serial, module_name1);
        }
        else
            strcpy(info->serial, "ONLP_STATUS_E_UNSUPPORTED");

        /* get psu Vin/Vout */
        sprintf(device_name, "PSU%d_Vin", local_id);
        if(dni_bmc_sensor_read(device_name, &u4Data, multiplier) == ONLP_STATUS_OK)
        {
            info->mvin = u4Data;
            info->status = ONLP_PSU_STATUS_PRESENT;
            info->caps |= ONLP_PSU_CAPS_VIN;
        }
        else
            info->caps |= ONLP_PSU_STATUS_UNPLUGGED;

        sprintf(device_name, "PSU%d_Vout", local_id);
        if(dni_bmc_sensor_read(device_name, &u4Data, multiplier) == ONLP_STATUS_OK)
        {
            info->mvout = u4Data;
            info->status = ONLP_PSU_STATUS_PRESENT;
            info->caps |= ONLP_PSU_CAPS_VOUT;
        }
        else
            info->caps |= ONLP_PSU_STATUS_UNPLUGGED;

        /* get psu Iin/Iout */
        sprintf(device_name, "PSU%d_Iin", local_id);
        if(dni_bmc_sensor_read(device_name, &u4Data, multiplier) == ONLP_STATUS_OK)
        {
            info->miin = u4Data;
            info->status = ONLP_PSU_STATUS_PRESENT;
            info->caps |= ONLP_PSU_CAPS_IIN;
        }
        else
            info->caps |= ONLP_PSU_STATUS_UNPLUGGED;

        sprintf(device_name, "PSU%d_Iout", local_id);
        if(dni_bmc_sensor_read(device_name, &u4Data, multiplier) == ONLP_STATUS_OK)
        {
            info->miout = u4Data;
            info->status = ONLP_PSU_STATUS_PRESENT;
            info->caps |= ONLP_PSU_CAPS_IOUT;
        }
        else
            info->caps |= ONLP_PSU_STATUS_UNPLUGGED;

        /* get psu Pin/Pout */
        sprintf(device_name, "PSU%d_Pin", local_id);
        if(dni_bmc_sensor_read(device_name, &u4Data, multiplier) == ONLP_STATUS_OK)
        {
            info->mpin = u4Data;
            info->status = ONLP_PSU_STATUS_PRESENT;
            info->caps |= ONLP_PSU_CAPS_PIN;
        }
        else
            info->caps |= ONLP_PSU_STATUS_UNPLUGGED;

        sprintf(device_name, "PSU%d_Pout", local_id);
        if(dni_bmc_sensor_read(device_name, &u4Data, multiplier) == ONLP_STATUS_OK)
        {
            info->mpout = u4Data;
            info->status = ONLP_PSU_STATUS_PRESENT;
            info->caps |= ONLP_PSU_CAPS_POUT;
        }
        else
            info->caps |= ONLP_PSU_STATUS_UNPLUGGED;
    }    
    else
    {
        int index = ONLP_OID_ID_GET(info->hdr.id);

        /* Read PSU product name from attribute */
        sprintf(node_path, "%s%s", PSU1_AC_PMBUS_PREFIX, "psu_mfr_model");
        dni_i2c_read_attribute_string(node_path, val_char, sizeof(val_char), 0);
        strcpy(info->model, val_char);

        /* Read PSU serial number from attribute */
        sprintf(node_path, "%s%s", PSU1_AC_PMBUS_PREFIX, "psu_mfr_serial");
        dni_i2c_read_attribute_string(node_path, val_char, sizeof(val_char), 0);
        strcpy(info->serial, val_char);

        /* Read voltage, current and power */
        if (dni_psu_pmbus_info_get(index, "psu_v_in", &val) == 0)
        {
            info->mvin = val;
            info->caps |= ONLP_PSU_CAPS_VIN;
        }

        if (dni_psu_pmbus_info_get(index, "psu_v_out", &val) == 0)
        {
            info->mvout = val;
            info->caps |= ONLP_PSU_CAPS_VOUT;
        }

        if (dni_psu_pmbus_info_get(index, "psu_i_in", &val) == 0)
        {
            info->miin = val;
            info->caps |= ONLP_PSU_CAPS_IIN;
        }

        if (dni_psu_pmbus_info_get(index, "psu_i_out", &val) == 0)
        {
            info->miout = val;
            info->caps |= ONLP_PSU_CAPS_IOUT;
        }

        if (dni_psu_pmbus_info_get(index, "psu_p_in", &val) == 0)
        {
            info->mpin = val;
            info->caps |= ONLP_PSU_CAPS_PIN;
        }

        if (dni_psu_pmbus_info_get(index, "psu_p_out", &val) == 0)
        {
            info->mpout = val;
            info->caps |= ONLP_PSU_CAPS_POUT;
        }    
    }
    return ret;
}

int onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int val = 0;
    int ret = ONLP_STATUS_OK;
    int index = ONLP_OID_ID_GET(id);
    char device_name[10] = {0};
    UINT4 u4Data = 0;
    UINT4 multiplier = 1000;
    int psu_present = -1;

    VALIDATE(id);

    /* Set the onlp_oid_hdr_t */
    memset(info, 0, sizeof(onlp_psu_info_t));
    *info = pinfo[index];

    switch (index) {
        case PSU1_ID:
            psu_present = dni_i2c_lock_read_attribute(NULL, PSU1_PRESENT_PATH);
            break;
        case PSU2_ID:
            psu_present = dni_i2c_lock_read_attribute(NULL, PSU2_PRESENT_PATH);
            break;
        default:
            break;
    }

    if(dni_bmc_check() == BMC_ON)
    {
        /* Check PSU have voltage input or not */
        sprintf(device_name, "PSU%d_Vin", index);
        if(dni_bmc_sensor_read(device_name, &u4Data, multiplier) == ONLP_STATUS_OK)
        {
            if(u4Data == 0)
            {
                info->status = ONLP_PSU_STATUS_FAILED;
                return ret;
            }
            info->mpin = u4Data;
            info->status = ONLP_PSU_STATUS_PRESENT;
            info->caps |= ONLP_PSU_CAPS_VIN;
        }
    }
    else
    {
        /* Check PSU have voltage input or not */
        dni_psu_pmbus_info_get(index, "psu_v_in", &val);

        /* Check PSU is PRESENT or not */
        if(psu_present == 0)
            info->status |= ONLP_PSU_STATUS_PRESENT;
        else if(val == 0 && psu_present == 1)
        {
            /* PSU is not PRESENT */
            /* Able to read PSU VIN(psu_power_not_good) */
            info->status |= ONLP_PSU_STATUS_FAILED;
            return ret;
        }
        else
        {
            /* Unable to read PSU VIN(psu_power_good) */
            info->status |= ONLP_PSU_STATUS_UNPLUGGED;
        }
    }

    ret = dni_psu_info_get(id, info);
    return ret;
}

int onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
