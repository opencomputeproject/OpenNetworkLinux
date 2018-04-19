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
#include "x86_64_delta_ag9032v2_int.h"
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

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

int 
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int i;
    int local_id;
    UINT4 multiplier = 1000;
    UINT4 u4Data     = 0;

    char device_name[10]  = {0};
    char module_name[20]  = {0};
    char *module_name_ptr = module_name;
    
    VALIDATE(id);
    local_id = ONLP_OID_ID_GET(id);
    *info = pinfo[ONLP_OID_ID_GET(id)]; 

    /* Set the associated oid_table
     * Set PSU's fan and thermal to child OID
     */
    info->hdr.coids[0] = ONLP_FAN_ID_CREATE(local_id + CHASSIS_FAN_COUNT);
    info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(local_id + CHASSIS_THERMAL_COUNT);

    switch (local_id) {
        case PSU1_ID:
        case PSU2_ID:
            //check PSU is present or not 
            if ( dni_psui_eeprom_info_get(module_name, local_id, PSU_MODEL_REG) == ONLP_STATUS_E_INVALID ){ 
                info->status = ONLP_PSU_STATUS_FAILED;
                break;
            }
            //get psu Pin/Pout
            sprintf(device_name, "PSU%d_Pin",local_id);
            if(dni_get_bmc_data(device_name, &u4Data, multiplier) == 0){
                info->mpin   = u4Data ;
                info->status = ONLP_PSU_STATUS_PRESENT;
                info->caps |= ONLP_PSU_CAPS_PIN;
            }
            else{
                info->caps |= ONLP_PSU_STATUS_UNPLUGGED;
            }

            sprintf(device_name, "PSU%d_Pout",local_id);
            if(dni_get_bmc_data(device_name, &u4Data, multiplier) == 0){
                info->mpout  = u4Data ;
                info->status = ONLP_PSU_STATUS_PRESENT;
                info->caps |= ONLP_PSU_CAPS_POUT;
            }
            else{
                info->caps |= ONLP_PSU_STATUS_UNPLUGGED;
            }
            //get psu Iin/Iout
            sprintf(device_name, "PSU%d_Iin",local_id);
            if(dni_get_bmc_data(device_name, &u4Data, multiplier) == ONLP_STATUS_OK){
                info->miin   = u4Data ;
                info->status = ONLP_PSU_STATUS_PRESENT;
                info->caps |= ONLP_PSU_CAPS_IIN;
            }
            else{
                info->caps |= ONLP_PSU_STATUS_UNPLUGGED;
            }
            sprintf(device_name, "PSU%d_Iout",local_id);
            if(dni_get_bmc_data(device_name, &u4Data, multiplier) == ONLP_STATUS_OK){
                info->miout  = u4Data;
                info->status = ONLP_PSU_STATUS_PRESENT;
                info->caps |= ONLP_PSU_CAPS_IOUT;
            }
            else{
                info->caps |= ONLP_PSU_STATUS_UNPLUGGED;
            }
            //get psu Vin/Vout
            sprintf(device_name, "PSU%d_Vin",local_id);
            if(dni_get_bmc_data(device_name, &u4Data, multiplier) == ONLP_STATUS_OK){
                info->mvin   = u4Data;
                info->status = ONLP_PSU_STATUS_PRESENT;
                info->caps |= ONLP_PSU_CAPS_VIN;
            }
            else{
                info->caps |= ONLP_PSU_STATUS_UNPLUGGED;
            }

            sprintf(device_name, "PSU%d_Vout",local_id);
            if(dni_get_bmc_data(device_name, &u4Data, multiplier) == ONLP_STATUS_OK){
                info->mvout  = u4Data;
                info->status = ONLP_PSU_STATUS_PRESENT;
                info->caps |= ONLP_PSU_CAPS_VOUT;
            }
            else{
                info->caps |= ONLP_PSU_STATUS_UNPLUGGED;
            }
            //get psu model name
            if(dni_psui_eeprom_info_get(module_name, local_id, PSU_MODEL_REG) == ONLP_STATUS_OK){
                for(i = 0; i< PSU_NUM_LENGTH; i++){
                    module_name[i]=*(module_name_ptr + i + 1);
                }
                strcpy(info->model, module_name_ptr); 
            }
            else{
                strcpy(info->model, "ONLP_STATUS_E_UNSUPPORTED");
            }
            //get psu serial number
            module_name[1] = '\0';
            if(dni_psui_eeprom_info_get(module_name_ptr, local_id, PSU_SERIAL_REG) == ONLP_STATUS_OK){
                for(i = 0; i < PSU_NUM_LENGTH; i++){ 
                    module_name[i]=*(module_name_ptr + i + 1);
                }
                strcpy(info->serial, module_name_ptr); 
            }
            else{
                strcpy(info->serial, "ONLP_STATUS_E_UNSUPPORTED");
            }   
            break;

        default:
            break;
    }

    return ONLP_STATUS_OK;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
