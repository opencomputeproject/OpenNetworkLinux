/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <onlplib/file.h>
#include <onlp/platformi/fani.h>

#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)


#define FAN_BOARD_PATH    "/sys/switch/fan/"
#define PSU_PATH    "/sys/switch/psu/"

#define FAN_NAME(fan_id) "Chassis Fan-" #fan_id
#define CHASSIS_FAN_ID(fid,mid) FAN_##fid##_MOTOR_##mid##_ON_FAN_BOARD
#define PSU_FAN_ID(psuid) FAN_ON_PSU##psuid
#define CHASSIS_FAN_INFO(_id,_desc)        \
    { \
        { ONLP_FAN_ID_CREATE(_id), _desc, 0 },\
        0x0,\
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,\
        0,\
        0,\
        ONLP_FAN_MODE_INVALID,\
    }
#define PSU_FAN_INFO(_id,_desc)        \
    { \
        { ONLP_FAN_ID_CREATE(_id), _desc, 0 },\
        0x0,\
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,\
        0,\
        0,\
        ONLP_FAN_MODE_INVALID,\
    }    

/* Static fan information */
onlp_fan_info_t finfo[] = {
    { }, /* Not used */
    CHASSIS_FAN_INFO(1,"Chassis Fan-1 Motor-1"),
    CHASSIS_FAN_INFO(2,"Chassis Fan-2 Motor-1"),
    CHASSIS_FAN_INFO(3,"Chassis Fan-3 Motor-1"),
    CHASSIS_FAN_INFO(4,"Chassis Fan-4 Motor-1"),
    CHASSIS_FAN_INFO(5,"Chassis Fan-5 Motor-1"),
    CHASSIS_FAN_INFO(6,"Chassis Fan-6 Motor-1"),
    CHASSIS_FAN_INFO(7,"Chassis Fan-1 Motor-2"),
    CHASSIS_FAN_INFO(8,"Chassis Fan-2 Motor-2"),
    CHASSIS_FAN_INFO(9,"Chassis Fan-3 Motor-2"),
    CHASSIS_FAN_INFO(10,"Chassis Fan-4 Motor-2"),
    CHASSIS_FAN_INFO(11,"Chassis Fan-5 Motor-2"),
    CHASSIS_FAN_INFO(12,"Chassis Fan-6 Motor-2"),
    PSU_FAN_INFO(FAN_ON_PSU1,"PSU 1 Fan"),
    PSU_FAN_INFO(FAN_ON_PSU2,"PSU 2 Fan"),
};


typedef struct fan_syspath_s {
    char * status;
	char * speed;
    char * percentage;
    char * direction;
    char * model_name;
    char * serial_number;
} fan_syspath_t;

fan_syspath_t syspath[]={
    { }, /* Not used */
	{  /*FAN_1_MOTOR_1_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan1/status",\
        FAN_BOARD_PATH"fan1/motor1/speed",\
        FAN_BOARD_PATH"fan1/motor1/ratio",\
        FAN_BOARD_PATH"fan1/motor1/direction",\
        FAN_BOARD_PATH"fan1/model_name" ,\
        FAN_BOARD_PATH"fan1/serial_number",\
    },
	{  /*FAN_2_MOTOR_1_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan2/status",\
        FAN_BOARD_PATH"fan2/motor1/speed",\
        FAN_BOARD_PATH"fan2/motor1/ratio",\
        FAN_BOARD_PATH"fan2/motor1/direction",\
        FAN_BOARD_PATH"fan2/model_name" ,\
        FAN_BOARD_PATH"fan2/serial_number",\
    },
	{  /*FAN_3_MOTOR_1_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan3/status",\
        FAN_BOARD_PATH"fan3/motor1/speed",\
        FAN_BOARD_PATH"fan3/motor1/ratio",\
        FAN_BOARD_PATH"fan3/motor1/direction",\
        FAN_BOARD_PATH"fan3/model_name" ,\
        FAN_BOARD_PATH"fan3/serial_number",\
    },    
	{  /*FAN_4_MOTOR_1_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan4/status",\
        FAN_BOARD_PATH"fan4/motor1/speed",\
        FAN_BOARD_PATH"fan4/motor1/ratio",\
        FAN_BOARD_PATH"fan4/motor1/direction",\
        FAN_BOARD_PATH"fan4/model_name" ,\
        FAN_BOARD_PATH"fan4/serial_number",\
    },     
	{  /*FAN_5_MOTOR_1_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan5/status",\
        FAN_BOARD_PATH"fan5/motor1/speed",\
        FAN_BOARD_PATH"fan5/motor1/ratio",\
        FAN_BOARD_PATH"fan5/motor1/direction",\
        FAN_BOARD_PATH"fan5/model_name" ,\
        FAN_BOARD_PATH"fan5/serial_number",\
    },  
	{  /*FAN_6_MOTOR_1_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan6/status",\
        FAN_BOARD_PATH"fan6/motor1/speed",\
        FAN_BOARD_PATH"fan6/motor1/ratio",\
        FAN_BOARD_PATH"fan6/motor1/direction",\
        FAN_BOARD_PATH"fan6/model_name" ,\
        FAN_BOARD_PATH"fan6/serial_number",\
    },  
	{  /*FAN_1_MOTOR_2_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan1/status",\
        FAN_BOARD_PATH"fan1/motor2/speed",\
        FAN_BOARD_PATH"fan1/motor2/ratio",\
        FAN_BOARD_PATH"fan1/motor2/direction",\
        FAN_BOARD_PATH"fan1/model_name" ,\
        FAN_BOARD_PATH"fan1/serial_number",\
    },     
	{  /*FAN_2_MOTOR_2_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan2/status",\
        FAN_BOARD_PATH"fan2/motor2/speed",\
        FAN_BOARD_PATH"fan2/motor2/ratio",\
        FAN_BOARD_PATH"fan2/motor2/direction",\
        FAN_BOARD_PATH"fan2/model_name" ,\
        FAN_BOARD_PATH"fan2/serial_number",\
    },  
	{  /*FAN_3_MOTOR_2_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan3/status",\
        FAN_BOARD_PATH"fan3/motor2/speed",\
        FAN_BOARD_PATH"fan3/motor2/ratio",\
        FAN_BOARD_PATH"fan3/motor2/direction",\
        FAN_BOARD_PATH"fan3/model_name" ,\
        FAN_BOARD_PATH"fan3/serial_number",\
    },       
	{  /*FAN_4_MOTOR_2_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan4/status",\
        FAN_BOARD_PATH"fan4/motor2/speed",\
        FAN_BOARD_PATH"fan4/motor2/ratio",\
        FAN_BOARD_PATH"fan4/motor2/direction",\
        FAN_BOARD_PATH"fan4/model_name" ,\
        FAN_BOARD_PATH"fan4/serial_number",\
    },       
	{  /*FAN_5_MOTOR_2_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan5/status",\
        FAN_BOARD_PATH"fan5/motor2/speed",\
        FAN_BOARD_PATH"fan5/motor2/ratio",\
        FAN_BOARD_PATH"fan5/motor2/direction",\
        FAN_BOARD_PATH"fan5/model_name" ,\
        FAN_BOARD_PATH"fan5/serial_number",\
    },  
	{  /*FAN_6_MOTOR_2_ON_FAN_BOARD*/  \
        FAN_BOARD_PATH"fan6/status",\
        FAN_BOARD_PATH"fan6/motor2/speed",\
        FAN_BOARD_PATH"fan6/motor2/ratio",\
        FAN_BOARD_PATH"fan6/motor2/direction",\
        FAN_BOARD_PATH"fan6/model_name" ,\
        FAN_BOARD_PATH"fan6/serial_number",\
    },     
	{  /*FAN_ON_PSU1*/  \
        PSU_PATH"psu1/present",\
        PSU_PATH"psu1/fan_speed",\
        PSU_PATH"psu1/fan_ratio",\
        0,\
        0,\
        0,\
    },   
	{  /*FAN_ON_PSU2*/  \
        PSU_PATH"psu2/present",\
        PSU_PATH"psu2/fan_speed",\
        PSU_PATH"psu2/fan_ratio",\
        0,\
        0,\
        0,\
    },                                          
};


/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    //get speed_max
    return ONLP_STATUS_OK;
}


int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    int  value = 0, fid,len;
    VALIDATE(id);

    fid = ONLP_OID_ID_GET(id);
    *info = finfo[fid];

    /* get fan present status
     */
    //  /sys/switch/fan/fan1/status :1(good)
    if(syspath[fid].status){
        if (0 > onlp_file_read_int(&value, syspath[fid].status ))
            return ONLP_STATUS_E_INTERNAL;
        if (value != 1) {
            return ONLP_STATUS_OK;  /* fan is not present */
        }
        info->status |= ONLP_FAN_STATUS_PRESENT;
        if(fid <= FAN_6_MOTOR_2_ON_FAN_BOARD){
           if (value==2) //2(present but fail)
                info->status |= ONLP_FAN_STATUS_FAILED;           
        }
    }


    /* get front fan rpm
     */
    if(syspath[fid].speed){
        if (0 > onlp_file_read_int(&value, syspath[fid].speed ))
            return ONLP_STATUS_E_INTERNAL;
         info->rpm = value;
    }

    /* set fan fail based on rpm
     */
    if(fid>FAN_6_MOTOR_2_ON_FAN_BOARD){
        if (!info->rpm) {
            info->status |= ONLP_FAN_STATUS_FAILED;
            return ONLP_STATUS_OK;
        }
    }

    /* get speed percentage 
     */
    if(syspath[fid].percentage){
        if (0 > onlp_file_read_int(&value, syspath[fid].percentage ))
            return ONLP_STATUS_E_INTERNAL;
         info->percentage = value;
    }

    /* get fan direction
     */
    value=0;
    if(syspath[fid].direction){
        if (0 > onlp_file_read_int(&value, syspath[fid].direction ))
            return ONLP_STATUS_E_INTERNAL;
    }
    if(value == 0)
        info->status |= ONLP_FAN_STATUS_F2B;
    else if (value == 1)
        info->status |= ONLP_FAN_STATUS_B2F;

    /* Get model name */
    if(syspath[fid].model_name){
        if (0 > onlp_file_read((uint8_t*)info->model,sizeof(info->model),&len, syspath[fid].model_name ))
            return ONLP_STATUS_E_INTERNAL;
    }

    /* Get serial*/
    if(syspath[fid].serial_number){
        if (0 > onlp_file_read((uint8_t*)info->serial,sizeof(info->model),&len, syspath[fid].serial_number ))
            return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}
    
/*
 * This function sets the speed of the given fan in RPM.
 *
 * This function will only be called if the fan supprots the RPM_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
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
onlp_fani_percentage_set(onlp_oid_t id, int p)
{
    int fid;
    fid = ONLP_OID_ID_GET(id);
    if(syspath[fid].percentage){
        if ( 0 > onlp_file_write_int(p, syspath[fid].percentage))
            return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}

/*
 * This function sets the fan speed of the given OID as per
 * the predefined ONLP fan speed modes: off, slow, normal, fast, max.
 *
 * Interpretation of these modes is up to the platform.
 *
 */
int
onlp_fani_mode_set(onlp_oid_t id, onlp_fan_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function sets the fan direction of the given OID.
 *
 * This function is only relevant if the fan OID supports both direction
 * capabilities.
 *
 * This function is optional unless the functionality is available.
 */
int
onlp_fani_dir_set(onlp_oid_t id, onlp_fan_dir_t dir)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * Generic fan ioctl. Optional.
 */
int
onlp_fani_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
