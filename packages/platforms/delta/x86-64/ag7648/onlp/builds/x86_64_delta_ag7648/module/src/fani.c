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
#include "x86_64_delta_ag7648_int.h"
#include "x86_64_delta_i2c.h"


#define MAX_FAN_SPEED       19000
#define MAX_PSU1_FAN_SPEED   19000
#define MAX_PSU2_FAN_SPEED   18000

#define FILE_NAME_LEN       80

#define CPLD_FAN_NAME   "MASTERCPLD"

#define CPLD_FAN_TRAY0_PRESENT_REG          (0x8)
#define CPLD_FAN_TRAY0_PRESENT_REG_OFFSET   (0x6)
#define CPLD_FAN_TRAY1_PRESENT_REG          (0x8)
#define CPLD_FAN_TRAY1_PRESENT_REG_OFFSET   (0x7)
#define CPLD_FAN_TRAY2_PRESENT_REG          (0x9)
#define CPLD_FAN_TRAY2_PRESENT_REG_OFFSET   (0x0)


/* The MAX6620 registers, valid channel numbers: 0, 1 */
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
#define FAN_FROM_REG(d1, d2)   \
    {   \
        int tech = (d1 << 3) | ((d2 >> 5) & 0x07);\
        rpm = (491520 * 4) / (2 * tech);\
    }

#define FAN_TO_REG(rpm) \
{   \
    float ftech;    \
    uint32_t tech;  \
     ftech = (491520.0 * 4)/ (2.0 * rpm);       \
     ftech = ftech + 0.3;   \
     tech = (uint32_t) ftech;   \
     d1 = (uint8_t)(tech >> 3); \
     d2 = (uint8_t)((tech << 5) & 0xe0);\
}
static int fan_initd=0;

enum onlp_fan_id
{
    FAN_RESERVED = 0,
    FAN_1_ON_MAIN_BOARD,    /*fan tray 0*/
    FAN_2_ON_MAIN_BOARD,    /*fan tray 0*/
    FAN_3_ON_MAIN_BOARD,    /*fan tray 1*/
    FAN_4_ON_MAIN_BOARD,    /*fan tray 1*/
    FAN_5_ON_MAIN_BOARD,    /*fan tray 2*/
    FAN_6_ON_MAIN_BOARD,    /*fan tray 2*/
    FAN_1_ON_PSU1,
    FAN_1_ON_PSU2
};

enum onlp_fan_tray_id
{
    FAN_TRAY_0 = 0,
    FAN_TRAY_1 = 1,
    FAN_TRAY_2 = 2
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
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(3),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(4),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(5),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(6),
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
_onlp_get_fan_tray(int fanId)
{   
    int tray_id;
    if((fanId==5) || (fanId==6))
        tray_id=0;
    else if((fanId==3) || (fanId==4))
        tray_id=1;
    else
        tray_id=2;
    return tray_id;
}
#if 0
static int
 _onlp_psu_fan_val_to_rpm (int v)
{
       int lf = (v & 0xffff);
       int y, n;

       y = lf & 0x7ff;
       n = ((lf >> 11) & 0x1f);

       return (y * (1 << n));
}
#endif

static int
_onlp_fan_board_init(void)
{
    int i = 0;
    int d1,d2;
    int rpm = 8000;
	i2c_devname_write_byte("FANCTRL1", 0x00,0x10);
	i2c_devname_write_byte("FANCTRL2", 0x00,0x10);

	i2c_devname_write_byte("FANCTRL1", 0x01,0x00);
	i2c_devname_write_byte("FANCTRL2", 0x01,0x00);

    for (i = FAN_1_ON_MAIN_BOARD; i <= FAN_4_ON_MAIN_BOARD; i ++)
    {
        int offset = i - FAN_1_ON_MAIN_BOARD;

	    i2c_devname_write_byte("FANCTRL2", 0x02 + offset ,0xc0);
        
        FAN_TO_REG(rpm);

	    i2c_devname_write_byte("FANCTRL2", 0x20 + 2 * offset, d1);
	    i2c_devname_write_byte("FANCTRL2", 0x21 + 2 * offset, d2);

    }
    for (i = FAN_5_ON_MAIN_BOARD; i <= FAN_6_ON_MAIN_BOARD; i ++)
    {
        int offset = i - FAN_5_ON_MAIN_BOARD;

	    i2c_devname_write_byte("FANCTRL1", 0x02 + offset ,0xc0);
       
        FAN_TO_REG(rpm);

	    i2c_devname_write_byte("FANCTRL1", 0x20 + 2 * offset, d1);
	    i2c_devname_write_byte("FANCTRL1", 0x21 + 2 * offset, d2);
    }

	fan_initd=1;
	
	return ONLP_STATUS_OK;
}

static int
_onlp_fani_info_get_fan(int local_id, onlp_fan_info_t* info)
{
    int   r_data, fan_present;
	int fan_tray = 0;
    int reg, offset;
    int d1, d2;
    int rpm;

    /* init the fan on the board*/
	if(fan_initd==0)
		_onlp_fan_board_init();

    fan_tray = _onlp_get_fan_tray(local_id);
    if (fan_tray == 0)
    {
        reg = CPLD_FAN_TRAY0_PRESENT_REG;
        offset = CPLD_FAN_TRAY0_PRESENT_REG_OFFSET;
    }else if (fan_tray == 1)
    {
        reg = CPLD_FAN_TRAY1_PRESENT_REG;
        offset = CPLD_FAN_TRAY1_PRESENT_REG_OFFSET;
    }else if (fan_tray == 2)
    {
        reg = CPLD_FAN_TRAY2_PRESENT_REG;
        offset = CPLD_FAN_TRAY2_PRESENT_REG_OFFSET;
    }else
    {
        return ONLP_STATUS_E_INVALID;
    }

    /* get fan fault status (turn on when any one fails)*/
    r_data = i2c_devname_read_byte(CPLD_FAN_NAME, reg);
  	if(r_data<0)
		return ONLP_STATUS_E_INVALID;
    
    fan_present = (r_data >> offset ) & 0x1;
    
    if(!fan_present){
        
        info->status |= ONLP_FAN_STATUS_PRESENT;
     
        if (fan_tray == 0)
        {
            d1 = i2c_devname_read_byte("FANCTRL1", 0x10 + 2 * (local_id - FAN_5_ON_MAIN_BOARD));
            d2 = i2c_devname_read_byte("FANCTRL1", 0x11 + 2 * (local_id - FAN_5_ON_MAIN_BOARD));
        }else
        {
            d1 = i2c_devname_read_byte("FANCTRL2", 0x10 + 2 * (local_id - FAN_1_ON_MAIN_BOARD) );
            d2 = i2c_devname_read_byte("FANCTRL2", 0x11 + 2 * (local_id - FAN_1_ON_MAIN_BOARD) );
        }

        if (d1 < 0 || d2 < 0)
        {
            info->status |= ONLP_FAN_STATUS_FAILED;
            return ONLP_STATUS_E_INVALID;
        }

    }
    else{
        info->status &= ~ONLP_FAN_STATUS_PRESENT;
        return ONLP_STATUS_E_UNSUPPORTED;
    }
    DEBUG_PRINT("d1 %d, d2 %d\r\n", d1, d2);

    FAN_FROM_REG(d1,d2);
    
    info->rpm = rpm;
    
    DEBUG_PRINT("rpm %d\r\n", rpm);
    /* get speed percentage from rpm */
    info->percentage = (info->rpm * 100.0) / MAX_FAN_SPEED;
    
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
#if 0	
    int   psu_id;
    int   r_data,fan_rpm;
    psu_type_t psu_type;
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
	if(pid == PID_AG7648){
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
#endif
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
		case FAN_3_ON_MAIN_BOARD:
		case FAN_4_ON_MAIN_BOARD:
		case FAN_5_ON_MAIN_BOARD:
		case FAN_6_ON_MAIN_BOARD:
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
	int rc1, rc2;
	int local_id;
    int d1, d2;
    int fan_tray;

	VALIDATE(id);
	
	local_id = ONLP_OID_ID_GET(id);

    DEBUG_PRINT("local id %d, rpm %d\n", local_id, rpm);

	if((local_id==FAN_1_ON_PSU1)||(local_id==FAN_1_ON_PSU2))
		return ONLP_STATUS_E_UNSUPPORTED;
	
	if (chassis_fan_count() == 0) {
        return ONLP_STATUS_E_INVALID;
    }
	   /* init the fan on the board*/
	if(fan_initd==0)
		_onlp_fan_board_init();

	/* reject rpm=0 (rpm=0, stop fan) */
	if (rpm == 0)
        return ONLP_STATUS_E_INVALID;    


	/*get ret value for the speed set*/
    FAN_TO_REG(rpm);
    DEBUG_PRINT("local id %d, rpm %d(d1: %d, d2: %d)\n", local_id, rpm, d1, d2);
    //return ONLP_STATUS_OK;

	/*set the rpm speed */
	fan_tray = _onlp_get_fan_tray(local_id);
    if (fan_tray < 0 || fan_tray > 2)
        return ONLP_STATUS_E_INVALID;

    if (fan_tray == 0)
    {
        rc1 = i2c_devname_write_byte("FANCTRL1", 0x20 + 2 * (local_id - FAN_5_ON_MAIN_BOARD), d1);
        rc2 = i2c_devname_write_byte("FANCTRL1", 0x21 + 2 * (local_id - FAN_5_ON_MAIN_BOARD), d2);
    }else
    {
        rc1 = i2c_devname_write_byte("FANCTRL2", 0x20 + 2 * (local_id - FAN_1_ON_MAIN_BOARD), d1);
        rc2 = i2c_devname_write_byte("FANCTRL2", 0x21 + 2 * (local_id - FAN_1_ON_MAIN_BOARD), d2);
    }

    if (rc1 < 0 || rc2 < 0)
    {
        return ONLP_STATUS_E_INVALID;
    }

    
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
	int rpm_val;
	int local_id;
	int d1, d2;
    int rc1, rc2;
    int fan_tray;

    VALIDATE(id);
	
	local_id = ONLP_OID_ID_GET(id);
	
	DEBUG_PRINT("local_id %d, percentage %d", local_id, p);
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
	
	rpm_val=p* MAX_FAN_SPEED/100;


	/*get ret value for the speed set*/
    FAN_TO_REG(rpm_val);

	DEBUG_PRINT("local_id %d, p %d, rpm_val %d(d1:%d, d2:%d)", local_id, p, rpm_val, d1, d2);
    //return ONLP_STATUS_OK;
	/*set the rpm speed */
	fan_tray = _onlp_get_fan_tray(local_id);
    if (fan_tray < 0 || fan_tray > 2)
        return ONLP_STATUS_E_INVALID;

    if (fan_tray == 0)
    {
        rc1 = i2c_devname_write_byte("FANCTRL1", 0x20 + 2 * (local_id - FAN_5_ON_MAIN_BOARD), d1);
        rc2 = i2c_devname_write_byte("FANCTRL1", 0x21 + 2 * (local_id - FAN_5_ON_MAIN_BOARD), d2);
    }else
    {
        rc1 = i2c_devname_write_byte("FANCTRL2", 0x20 + 2 * (local_id - FAN_1_ON_MAIN_BOARD) , d1);
        rc2 = i2c_devname_write_byte("FANCTRL2", 0x21 + 2 * (local_id - FAN_1_ON_MAIN_BOARD) , d2);
    }

    if (rc1 < 0 || rc2 < 0)
    {
        return ONLP_STATUS_E_INVALID;
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

