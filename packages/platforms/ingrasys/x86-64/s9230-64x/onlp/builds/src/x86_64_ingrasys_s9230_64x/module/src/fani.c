/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
 * Fan Platform Implementation Defaults.
 *
 ***********************************************************/
#include <onlp/platformi/fani.h>
#include "x86_64_ingrasys_s9230_64x_int.h"
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include "platform_lib.h"

onlp_fan_info_t fan_info[] = {
    { }, /* Not used */
    {
        { FAN_OID_FAN1, "FANTRAY 1", 0 },
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
        0,
        0,
        ONLP_FAN_MODE_INVALID,
    },
    {
        { FAN_OID_FAN2, "FANTRAY 2", 0 },
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
        0,
        0,
        ONLP_FAN_MODE_INVALID,
    },
    {
        { FAN_OID_FAN3, "FANTRAY 3", 0 },
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
        0,
        0,
        ONLP_FAN_MODE_INVALID,
    },
    {
        { FAN_OID_FAN4, "FANTRAY 4", 0 },
        ONLP_FAN_STATUS_PRESENT,
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
        0,
        0,
        ONLP_FAN_MODE_INVALID,
    },
    {
        { FAN_OID_PSU_FAN1, "PSU-1 FAN", 0 },
        ONLP_FAN_STATUS_PRESENT,
    },
    {
        { FAN_OID_PSU_FAN2, "PSU-2 FAN", 0 },
        ONLP_FAN_STATUS_PRESENT,
    }    
};

/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}

int sys_fan_present_get(onlp_fan_info_t* info, int id)
{
    int rv, fan_presence, i2c_bus, offset, fan_reg_mask;

    /* get fan presence*/
    i2c_bus = I2C_BUS_FANTRAY_LED; 
    switch (id)
    {
        case FAN_ID_FAN1:
            offset = 0;
            fan_reg_mask = FAN_1_PRESENT_MASK;
            break;
        case FAN_ID_FAN2:
            offset = 0;
            fan_reg_mask = FAN_2_PRESENT_MASK;
            break;
        case FAN_ID_FAN3:
            offset = 1;
            fan_reg_mask = FAN_3_PRESENT_MASK;
            break;
        case FAN_ID_FAN4:
            offset = 1;
            fan_reg_mask = FAN_4_PRESENT_MASK;
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }

    rv = onlp_i2c_readb(i2c_bus, FAN_GPIO_ADDR, offset, ONLP_I2C_F_FORCE);
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    fan_presence = (rv & fan_reg_mask) ? 0 : 1;

    if (!fan_presence) {
        info->status &= ~ONLP_FAN_STATUS_PRESENT;

    } else {
        info->status |= ONLP_FAN_STATUS_PRESENT;
    }

    return ONLP_STATUS_OK;
}

int 
sys_fan_info_get(onlp_fan_info_t* info, int id)
{
    int rv, fan_status, fan_rpm, perc_val, percentage;
    int max_fan_speed = 10000;
    fan_status = 0;
    fan_rpm = 0;       

    rv = sys_fan_present_get(info, id);
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
 
    rv = onlp_file_read_int(&fan_status, SYS_FAN_PREFIX "fan%d_alarm", 2 * id - 1);
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* fan status > 1, means failure */
    if (fan_status > 0) {
        info->status |= ONLP_FAN_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }
        
    rv = onlp_file_read_int(&fan_rpm, SYS_FAN_PREFIX "fan%d_input", 2 * id - 1);
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }    
    info->rpm = fan_rpm;
    
    /* get speed percentage*/
    switch (id)
	{
        case FAN_ID_FAN1:    
        case FAN_ID_FAN2:
        case FAN_ID_FAN3:
        case FAN_ID_FAN4:
            rv = onlp_file_read_int(&perc_val, SYS_FAN_PREFIX "pwm%d",
                                    FAN_CTRL_SET2);
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }   
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
   
    percentage = (info->rpm*100)/max_fan_speed; 
    info->percentage = percentage;
    
    return ONLP_STATUS_OK;
}

int
sys_fan_rpm_percent_set(int perc)
{  
    int rc;
    
    rc = onlp_file_write_int(perc, SYS_FAN_PREFIX "pwm%d", FAN_CTRL_SET2);
    if (rc < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
       
    return ONLP_STATUS_OK;
}

int
onlp_fani_rpm_set(onlp_oid_t id, int rpm)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function sets the fan speed of the given OID as a percentage.
 *
 * This will only be called if the OID has the PERCENTAGE_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_percentage_set(onlp_oid_t id, int percentage)
{
    int  fid, perc_val, rc;
    fid = ONLP_OID_ID_GET(id);

    /* 
     * Set fan speed 
     * Driver accept value in range between 128 and 255.
     * Value 128 is 50%.
     * Value 200 is 80%.
     * Value 255 is 100%.
     */
    if (percentage == 100) {
        perc_val = 255;
    } else if (percentage == 80) {
        perc_val = 200;
    } else if (percentage == 50) {
        perc_val = 128;
    } else {
         return ONLP_STATUS_E_INVALID;
    }

    switch (fid)
	{
        case FAN_ID_FAN1:    
        case FAN_ID_FAN2:
        case FAN_ID_FAN3:
        case FAN_ID_FAN4:
            rc = sys_fan_rpm_percent_set(perc_val);
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }
	return rc;   
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* rv)
{
    int fan_id ,rc;
    
    fan_id = ONLP_OID_ID_GET(id);
    *rv = fan_info[fan_id];
    rv->caps |= ONLP_FAN_CAPS_GET_RPM;
       
    switch (fan_id) {
        case FAN_ID_FAN1:
        case FAN_ID_FAN2:
        case FAN_ID_FAN3:
        case FAN_ID_FAN4:
            rc = sys_fan_info_get(rv, fan_id);
            break;
        case FAN_ID_PSU_FAN1:
        case FAN_ID_PSU_FAN2:
            rc = psu_fan_info_get(rv, fan_id);
            break;
        default:            
            return ONLP_STATUS_E_INTERNAL;
            break;
    }

    return rc;
}
