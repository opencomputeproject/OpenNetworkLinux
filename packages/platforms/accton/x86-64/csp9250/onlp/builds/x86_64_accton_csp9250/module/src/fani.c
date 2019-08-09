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
#include <sys/time.h>
#include <sys/stat.h>
#include <onlp/platformi/fani.h>
#include "platform_lib.h"

#define MAX_FAN_1_SPEED     21500
#define MAX_FAN_2_SPEED     18000

#define MAX_PSU_FAN_SPEED   25500

#define FAN_MAX_LENGTH     256
#define FAN_LEAVE_NUM      2

#define FAN_IPMI_TMP_FILE_RM_RPM "rm -f /tmp/fan_bmc_info_rpm > /dev/null 2>&1"
#define FAN_IPMI_TMP_FILE_RM_PERCENT "rm -f /tmp/fan_bmc_info_rpm_percent > /dev/null 2>&1"

#define FAN_IPMI_TMP_FILE_RPM            "/tmp/fan_bmc_info_rpm"
#define FAN_IPMI_TMP_FILE_PERCENT            "/tmp/fan_bmc_info_percent"

#define FAN_IPMI_TMP_FILE_FIND_NO_READ  "No Reading"
#define FAN_IPMI_TMP_FILE_FIND "RPM"
#define FAN_IPMI_TMP_FILE_FIND_PERCENT "02"

#define FAN_IPMI_SDR_CMD      "ipmitool sdr list"
#define FAN_IPMI_PWM_SET      "ipmitool raw 0x34 0xaa 0x5a 0x54 0x40 0x04 0x01 0xff"
#define FAN_IPMI_PWM_GET      "ipmitool raw 0x34 0xaa 0x5a 0x54 0x40 0x04 0x02 0x01"
#define FAN_IPMI_SDR_FILE     "/usr/bin/fan_bmc_sdr"
#define FAN_IPMI_SDR_FILE_RM  "rm -f /usr/bin/fan_bmc_sdr > /dev/null 2>&1"
#define FAN_CHECK_TIME        "/usr/bin/fan_check_time"
#define FAN_IPMI_PWM_FILE     "/usr/bin/fan_bmc_pwm"
#define FAN_IPMI_PWM_FILE_RM  "rm -f /usr/bin/fan_bmc_pwm > /dev/null 2>&1"

#define FAN_CAN_SET_IPMI_TIME    5

enum fan_id {
	FAN_1_ON_FAN_BOARD = 1,
	FAN_2_ON_FAN_BOARD,
	FAN_3_ON_FAN_BOARD,
	FAN_4_ON_FAN_BOARD,
	FAN_5_ON_FAN_BOARD,
};

#define CHASSIS_FAN_INFO(fid) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fid##_ON_FAN_BOARD), "Fan - "#fid, 0 },\
        0x0,\
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,\
        0,\
        0,\
        ONLP_FAN_MODE_INVALID,\
    }


#define PSU_FAN_INFO(pid, fid) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fid##_ON_PSU_##pid), "PSU "#pid" - Fan "#fid, 0 },\
        0x0,\
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,\
        0,\
        0,\
        ONLP_FAN_MODE_INVALID,\
    }

/* Static fan information */
onlp_fan_info_t finfo[] = {
    { }, /* Not used */
	CHASSIS_FAN_INFO(1),
	CHASSIS_FAN_INFO(2),
	CHASSIS_FAN_INFO(3),
	CHASSIS_FAN_INFO(4),
	CHASSIS_FAN_INFO(5)
};



typedef struct onlp_fan_dev_s
{
   int   fan_id;   
   char  *tag[FAN_LEAVE_NUM];   
    	
}onlp_fan_dev_t;


onlp_fan_dev_t fan_sensor_table[]=
{
    {0, {NULL, NULL}},
    {FAN_1_ON_FAN_BOARD, {"Fan1_1", "Fan1_2"}},
    {FAN_2_ON_FAN_BOARD, {"Fan2_1", "Fan2_2"}},
    {FAN_3_ON_FAN_BOARD, {"Fan3_1", "Fan3_2"}},
    {FAN_4_ON_FAN_BOARD, {"Fan4_1", "Fan4_2"}},
    {FAN_5_ON_FAN_BOARD, {"Fan5_1", "Fan5_2"}},
};



#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)
 
 
static int 
fani_time_exist(void)
{
    struct stat file_info;    
    if(stat(FAN_CHECK_TIME ,&file_info)==0)
    {
        if(file_info.st_size==0)
            return 0;
        return 1;      
    }
    else
       return 1;    
}
static int 
fani_sdr_file_exist(void)
{
    struct stat file_info;
    if(stat(FAN_IPMI_SDR_FILE ,&file_info)==0)
    {
        if(file_info.st_size==0)
            return 0;
        return 1;
    }
    else
       return 0;    
}

 
static int
_onlp_fani_info_get_fan(int fid, onlp_fan_info_t* info)
{
    int   i, size;
    char *tag, *p;
    char cmd[FAN_MAX_LENGTH/2]={0};
    char fan_val_str[6]={0};
    int  fan_val_int=0;
    uint8_t  data[FAN_MAX_LENGTH] = {0};
	struct  timeval    new_tv;
    long    last_time;
    int     get_data_by_ipmi=0;
	
	if(fid > FAN_5_ON_FAN_BOARD)
	    return ONLP_STATUS_E_INTERNAL;
	
	if(fani_time_exist())
    {
        if(onlp_file_read(data, FAN_MAX_LENGTH, &size, FAN_CHECK_TIME)!=ONLP_STATUS_OK)
        {
            last_time=0; 
        }
        else
           last_time = atol((char*)data);
    }
    else
        last_time=0;    
    
    gettimeofday(&new_tv,NULL);
         
    if(last_time==0) /* first time */
    {
        get_data_by_ipmi=1;
    }
    else
    {
         if(new_tv.tv_sec > last_time)
         {
             if((new_tv.tv_sec - last_time) > FAN_CAN_SET_IPMI_TIME)
             {
                get_data_by_ipmi=1;
             }
             else
                get_data_by_ipmi=0;
         }
         else if(new_tv.tv_sec == last_time)
            get_data_by_ipmi=0;
         else
            get_data_by_ipmi=1;
            
    }
	memset(data ,0x0, FAN_MAX_LENGTH);
    snprintf((char*)data, FAN_MAX_LENGTH-1, "%ld", new_tv.tv_sec);
    if(onlp_file_write_str((char*)data, FAN_CHECK_TIME)!=ONLP_STATUS_OK)
    {
        return ONLP_STATUS_E_INTERNAL;
    }
    
    if(get_data_by_ipmi || !fani_sdr_file_exist()) /* Set ipmitool cmd to get all data and save to file*/
    {
        /* set ipmi sdr cmd to get rpm */
        snprintf(cmd, (FAN_MAX_LENGTH/2) -1, "%s > %s ", FAN_IPMI_SDR_CMD, FAN_IPMI_SDR_FILE);
        system(cmd);
        /* set ipmi cmd to get pwm */        
        snprintf(cmd, (FAN_MAX_LENGTH/2) -1, "%s > %s ", FAN_IPMI_PWM_GET, FAN_IPMI_PWM_FILE);
        system(cmd);
        fflush(stdout);
    }    
	for(i=0; i<FAN_LEAVE_NUM; i++)
	{
        tag=fan_sensor_table[fid].tag[i];
        snprintf(cmd , (FAN_MAX_LENGTH/2)-1, "cat %s |grep %s > %s", FAN_IPMI_SDR_FILE, tag, FAN_IPMI_TMP_FILE_RPM);
        system(cmd);
        
        if(onlp_file_read(data, FAN_MAX_LENGTH, &size, FAN_IPMI_TMP_FILE_RPM)!=ONLP_STATUS_OK)
        {
            return ONLP_STATUS_E_INTERNAL;
        }    
        p=strstr((char*)data, FAN_IPMI_TMP_FILE_FIND);
        if(p==NULL)
        {
            return ONLP_STATUS_E_INTERNAL;
        }
        if(((uint8_t*)p-data) < sizeof(fan_val_str)/sizeof(char))
        {
            return ONLP_STATUS_E_INTERNAL;
        }
        fan_val_str[0]=(data[(uint8_t*)p-data-6]=='|' || data[(uint8_t*)p-data-6]==' ')?'0':data[(uint8_t*)p-data-6];
        fan_val_str[1]=(data[(uint8_t*)p-data-5]=='|' || data[(uint8_t*)p-data-5]==' ')?'0':data[(uint8_t*)p-data-5];
        fan_val_str[2]=(data[(uint8_t*)p-data-4]=='|' || data[(uint8_t*)p-data-4]==' ')?'0':data[(uint8_t*)p-data-4];        
        fan_val_str[3]=(data[(uint8_t*)p-data-3]=='|' || data[(uint8_t*)p-data-3]==' ')?'0':data[(uint8_t*)p-data-3];
        fan_val_str[4]=(data[(uint8_t*)p-data-2]=='|' || data[(uint8_t*)p-data-2]==' ')?'0':data[(uint8_t*)p-data-2];        
        /* take the min value from front/rear fan speed
	     */
        if(!fan_val_int)
        {
            if(fan_val_int < ONLPLIB_ATOI(fan_val_str))
            {
      	 	   fan_val_int=ONLPLIB_ATOI(fan_val_str);
            }
        }
        else
            fan_val_int=ONLPLIB_ATOI(fan_val_str);
    }
    
	if(fan_val_int==0)
    {
        info->status &= ~ONLP_FAN_STATUS_PRESENT;
        return ONLP_STATUS_OK;
	}
	info->rpm=fan_val_int;	
	info->percentage=0;
	snprintf(cmd , (FAN_MAX_LENGTH/2)-1, "cat %s > %s", FAN_IPMI_PWM_FILE, FAN_IPMI_TMP_FILE_PERCENT);
	system(cmd);
	
	if(onlp_file_read(data, FAN_MAX_LENGTH, &size, FAN_IPMI_TMP_FILE_PERCENT)!=ONLP_STATUS_OK)
    {
        return ONLP_STATUS_E_INTERNAL;
    }
    p=strstr((char*)data, FAN_IPMI_TMP_FILE_FIND_PERCENT);
    if(p==NULL)
    {
        return ONLP_STATUS_E_INTERNAL;
    }
    if(((uint8_t*)p-data) < sizeof(fan_val_str)/sizeof(char))
    {
        return ONLP_STATUS_E_INTERNAL;
    }
    memset(fan_val_str, 0x0, sizeof(fan_val_str));
    fan_val_str[0]=data[(uint8_t*)p-data+3];
    fan_val_str[1]=data[(uint8_t*)p-data+4];
    info->percentage = ONLPLIB_ATOI(fan_val_str);
    
    info->status |= ONLP_FAN_STATUS_PRESENT;
  
	return ONLP_STATUS_OK;
}

/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    char     cmd[FAN_MAX_LENGTH/2] = {0};
    
    snprintf(cmd, (FAN_MAX_LENGTH/2) -1, "echo 0 >  %s ", FAN_CHECK_TIME);
    system(cmd);
    return ONLP_STATUS_OK;
   
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    int rc = 0;
    int fid;
    VALIDATE(id);

    fid = ONLP_OID_ID_GET(id);
    *info = finfo[fid];

    switch (fid)
    {
        case FAN_1_ON_FAN_BOARD:
        case FAN_2_ON_FAN_BOARD:
        case FAN_3_ON_FAN_BOARD:
        case FAN_4_ON_FAN_BOARD:
        case FAN_5_ON_FAN_BOARD:
            rc =_onlp_fani_info_get_fan(fid, info);						
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
    int  fid;
    char cmd[FAN_MAX_LENGTH/2]={0};
    
    VALIDATE(id);

    fid = ONLP_OID_ID_GET(id);
  

    switch (fid)
	{
        case FAN_1_ON_FAN_BOARD:
        case FAN_2_ON_FAN_BOARD:
        case FAN_3_ON_FAN_BOARD:
        case FAN_4_ON_FAN_BOARD:
        case FAN_5_ON_FAN_BOARD:			
			break;
        default:
            return ONLP_STATUS_E_INVALID;
    }
	
    snprintf(cmd,FAN_MAX_LENGTH/2,  "%s %d > /dev/null 2>&1", FAN_IPMI_PWM_SET, p);
    system(cmd);
    
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

