/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc
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
#include <unistd.h>
#include <fcntl.h>
#include "platform_lib.h"
#include "arm_delta_ag6248c_int.h"
#include "arm_delta_i2c.h"


#define MAX_FAN_SPEED       16000
#define MAX_PSU_FAN_SPEED   23000

#define FILE_NAME_LEN       80

/* The MAX6639 registers, valid channel numbers: 0, 1 */
#define MAX6639_REG_STATUS			     0x02
#define MAX6639_REG_FAN_CONFIG1(ch)	    (0x10 + 4*(ch-1))
#define MAX6639_REG_FAN_CNT(ch)			(0x20 + (ch-1))
#define MAX6639_REG_TARGET_CNT(ch)		(0x22 + (ch-1))

/*define the reg bit mask*/
#define MAX6639_REG_FAN_STATUS_BIT(ch)  (0X02>>(ch-1))
#define MAX6639_FAN_CONFIG1_RPM_RANGE    0x03
#define MAX6639_FAN_PRESENT_REG         (0x0c) 
#define MAX6639_FAN_PRESENT_BIT         (0x2)
#define MAX6639_FAN_GOOD_BIT            (0x1)
#define FAN_FROM_REG(val)	((480000.0) / (val))

static int fan_initd=0;

enum onlp_fan_id
{
    FAN_RESERVED = 0,
    FAN_1_ON_MAIN_BOARD,
    FAN_2_ON_MAIN_BOARD,
    FAN_1_ON_PSU1,
    FAN_1_ON_PSU2
};

#define MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##id##_ON_MAIN_BOARD), "Chassis Fan "#id, 0 }, \
        ONLP_FAN_STATUS_B2F | ONLP_FAN_STATUS_PRESENT, \
        (ONLP_FAN_CAPS_SET_PERCENTAGE |ONLP_FAN_CAPS_SET_RPM| ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id, fan_id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fan_id##_ON_PSU##psu_id), "Chassis PSU-"#psu_id " Fan "#fan_id, 0 }, \
        ONLP_FAN_STATUS_B2F | ONLP_FAN_STATUS_PRESENT, \
        (ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

/* Static fan information */
onlp_fan_info_t linfo[] = {
    { }, /* Not used */
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(2),
    MAKE_FAN_INFO_NODE_ON_PSU(1,1),
    MAKE_FAN_INFO_NODE_ON_PSU(2,1),
};

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int
 _onlp_psu_fan_val_to_rpm (int v)
{
       int lf = (v & 0xffff);
       int y, n;

       y = lf & 0x7ff;
       n = ((lf >> 11) & 0x1f);

       return (y * (1 << n));
}

static int
_onlp_fan_board_init(void)
{
	i2c_devname_write_byte("FAN_ON_BOARD", 0x03,0xfc);
	i2c_devname_write_byte("FAN_ON_BOARD", 0x04,0x30);
	
	i2c_devname_write_byte("FAN_ON_BOARD", 0x10,0x23);
	i2c_devname_write_byte("FAN_ON_BOARD", 0x11,0x00);
	i2c_devname_write_byte("FAN_ON_BOARD", 0x12,0x00);
	i2c_devname_write_byte("FAN_ON_BOARD", 0x13,0x21);
	i2c_devname_write_byte("FAN_ON_BOARD", 0x24,0xe8);
	
	i2c_devname_write_byte("FAN_ON_BOARD", 0x14,0x23);
	i2c_devname_write_byte("FAN_ON_BOARD", 0x15,0x00);
	i2c_devname_write_byte("FAN_ON_BOARD", 0x16,0x00);
	i2c_devname_write_byte("FAN_ON_BOARD", 0x17,0x21);
	i2c_devname_write_byte("FAN_ON_BOARD", 0x25,0xe8);
	
	fan_initd=1;
	
	return ONLP_STATUS_OK;
}

static int
_onlp_fani_info_get_fan(int local_id, onlp_fan_info_t* info)
{
    int   r_data,fan_good,fan_present,fan_fault;
	
    /* init the fan on the board*/
	if(fan_initd==0)
		_onlp_fan_board_init();
    /* get fan fault status (turn on when any one fails)*/
	r_data= i2c_devname_read_byte("CPLD",MAX6639_FAN_PRESENT_REG);
    
  	if(r_data<0)
		return ONLP_STATUS_E_INVALID;
    
    fan_present = r_data & MAX6639_FAN_PRESENT_BIT;
    
    if(!fan_present){
        
        info->status |= ONLP_FAN_STATUS_PRESENT;
        
        fan_good=r_data&MAX6639_FAN_GOOD_BIT;
        
        if(fan_good)
            info->status&=~ONLP_FAN_STATUS_FAILED;
        else{
            r_data = i2c_devname_read_byte("FAN_ON_BOARD", MAX6639_REG_STATUS);
        
            if(r_data<0)
                return ONLP_STATUS_E_INVALID;
       
            fan_fault=r_data & MAX6639_REG_FAN_STATUS_BIT(local_id);
            
            if(!fan_fault)
                info->status &=~ ONLP_FAN_STATUS_FAILED;
            else{ 
                info->status |=ONLP_FAN_STATUS_FAILED;
                info->rpm=0;
                info->percentage=0;
                goto mode;
            }
        }
    }
    else{
        info->status &= ~ONLP_FAN_STATUS_PRESENT;
        return ONLP_STATUS_E_UNSUPPORTED;
    }
    
    /* get fan speed */
    r_data = i2c_devname_read_byte("FAN_ON_BOARD", MAX6639_REG_FAN_CNT(local_id));

    if(r_data<0)
            return ONLP_STATUS_E_INVALID;

    info->rpm = FAN_FROM_REG(r_data);
    
    /* get speed percentage from rpm */
    info->percentage = (info->rpm * 100.0) / MAX_FAN_SPEED;
    
mode:	
	if(info->percentage>100)
		strcpy(info->model,"ONLP_FAN_MODE_LAST");
	else if(info->percentage==100)
		strcpy(info->model,"ONLP_FAN_MODE_MAX");
	else if(info->percentage>=75&&info->percentage<100)
		strcpy(info->model,"ONLP_FAN_MODE_FAST");
	else if(info->percentage>=35&&info->percentage<75)
		strcpy(info->model,"ONLP_FAN_MODE_NORMAL");
	else if(info->percentage>0&&info->percentage<35)
		strcpy(info->model,"ONLP_FAN_MODE_SLOW");
	else if(info->percentage<=0)
		strcpy(info->model,"ONLP_FAN_MODE_OFF");
	else{ }

    return ONLP_STATUS_OK;
}

static int
_onlp_fani_info_get_fan_on_psu(int local_id, onlp_fan_info_t* info)
{
    int   psu_id;
    int   r_data,fan_rpm;
	
    psu_type_t psu_type;

	enum ag6248c_product_id pid = get_product_id();
    /* get fan fault status
     */
    psu_id   = (local_id - FAN_1_ON_PSU1) + 1;
    DEBUG_PRINT("[Debug][%s][%d][psu_id: %d]\n", __FUNCTION__, __LINE__, psu_id);

    psu_type = get_psu_type(psu_id); /* psu_id = 1 , present PSU1. pus_id =2 , present PSU2 */
    DEBUG_PRINT("[Debug][%s][%d][psu_type: %d]\n", __FUNCTION__, __LINE__, psu_type);

    switch (psu_type) {
        case PSU_TYPE_AC_F2B:
            info->status |= (ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_F2B);
            break;
        case PSU_TYPE_AC_B2F:
            info->status |= (ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_B2F);
            break;
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
    }

    /* get fan speed*/
	if(pid == PID_AG6248C_48){
		if(psu_id==1)
			r_data=i2c_devname_read_word("PSU1_PMBUS", 0x90);
		else
			r_data=i2c_devname_read_word("PSU2_PMBUS", 0x90);
	}
	else{
		if(psu_id==1)
			r_data=i2c_devname_read_word("PSU1_PMBUS_POE", 0x90);
		else
			r_data=i2c_devname_read_word("PSU2_PMBUS_POE", 0x90);
	}
	
	if(r_data<0)
		 return ONLP_STATUS_E_INVALID;
	 
	fan_rpm=_onlp_psu_fan_val_to_rpm(r_data);
	
    info->rpm = fan_rpm;

    /* get speed percentage from rpm */
    info->percentage = (info->rpm * 100.0) / MAX_PSU_FAN_SPEED;
	
	if(info->percentage>100)
		strcpy(info->model,"ONLP_FAN_MODE_LAST");
	else if(info->percentage==100)
		strcpy(info->model,"ONLP_FAN_MODE_MAX");
	else if(info->percentage>=75&&info->percentage<100)
		strcpy(info->model,"ONLP_FAN_MODE_FAST");
	else if(info->percentage>=35&&info->percentage<75)
		strcpy(info->model,"ONLP_FAN_MODE_NORMAL");
	else if(info->percentage>0&&info->percentage<35)
		strcpy(info->model,"ONLP_FAN_MODE_SLOW");
	else if(info->percentage<=0)
		strcpy(info->model,"ONLP_FAN_MODE_OFF");
    else{}

    return ONLP_STATUS_OK;
}

/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{	
	int rc;
   	rc=_onlp_fan_board_init();
    return rc;
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    int rc = 0;
    int local_id;

    VALIDATE(id);
	
    local_id = ONLP_OID_ID_GET(id);
	
    if (chassis_fan_count() == 0) {
        local_id += 1;
    }
    
    *info = linfo[local_id];

    switch (local_id)
    {
        case FAN_1_ON_PSU1:
        case FAN_1_ON_PSU2:
            rc = _onlp_fani_info_get_fan_on_psu(local_id, info);
            break;
        case FAN_1_ON_MAIN_BOARD:
		case FAN_2_ON_MAIN_BOARD:
            rc =_onlp_fani_info_get_fan(local_id, info);
            break;
        default:
            rc = ONLP_STATUS_E_INVALID;
            break;
    }

    return rc;
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
{	/*
	the rpm is the actual rpm/1000. so 16 represents the 16000(max spd)
	*/
	int fan_set_rpm_cont,rc;
	int local_id;
	int actual_rpm=rpm;

	VALIDATE(id);
	
	local_id = ONLP_OID_ID_GET(id);

	if((local_id==FAN_1_ON_PSU1)||(local_id==FAN_1_ON_PSU2))
		return ONLP_STATUS_E_UNSUPPORTED;
	
	if (chassis_fan_count() == 0) {
        return ONLP_STATUS_E_INVALID;
    }
	   /* init the fan on the board*/
	if(fan_initd==0)
		_onlp_fan_board_init();

	/* reject rpm=0 (rpm=0, stop fan) */
	if (actual_rpm == 0)
        return ONLP_STATUS_E_INVALID;    

	/*get ret value for the speed set*/
	fan_set_rpm_cont=FAN_FROM_REG(actual_rpm);

	/*set the rpm speed */
	rc=i2c_devname_write_byte("FAN_ON_BOARD", MAX6639_REG_TARGET_CNT(local_id), fan_set_rpm_cont);
	
	if(rc<0)
		return ONLP_STATUS_E_INVALID;

    return ONLP_STATUS_OK;
}
/*set the percentage for the psu fan*/


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
	/*
	p is between 0 and 100 ,p=100 represents 16000(max spd)
	*/
	int rpm_val,fan_set_rpm_cont,rc;
	int local_id;
	
    VALIDATE(id);
	
	local_id = ONLP_OID_ID_GET(id);
	
	if((local_id==FAN_1_ON_PSU1)||(local_id==FAN_1_ON_PSU2))
		return ONLP_STATUS_E_UNSUPPORTED;
	
    if (chassis_fan_count() == 0) {
        return ONLP_STATUS_E_INVALID;
    }
    
	   /* init the fan on the board*/
	if(fan_initd==0)
		_onlp_fan_board_init();
	
    /* reject p=0 (p=0, stop fan) */
    if (p == 0){
        return ONLP_STATUS_E_INVALID;
    }
	
	rpm_val=p* MAX_PSU_FAN_SPEED/100;
	
	/*get ret value for the speed set*/
	fan_set_rpm_cont=FAN_FROM_REG(rpm_val);
		
	/*set the rpm speed */
	rc=i2c_devname_write_byte("FAN_ON_BOARD", MAX6639_REG_TARGET_CNT(id), fan_set_rpm_cont);
	
	if(rc<0)
		return ONLP_STATUS_E_INVALID;
	
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

