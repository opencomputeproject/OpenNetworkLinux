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
 *
 *
 ***********************************************************/
#include <onlp/platformi/psui.h>
#include <onlplib/mmap.h>
#include <string.h>
#include <sys/stat.h>
#include "platform_lib.h"

#define PSU_STATUS_PRESENT    1
#define PSU_STATUS_POWER_GOOD 1
#define PSU_MAX_LENGTH     256

#define PSU_IPMI_TMP_FILE_RM "rm -f /tmp/psu_bmc_info > /dev/null 2>&1"
#define PSU_IPMI_TMP_FILE_ONE_ENTRY_RM "rm -f /tmp/psu_bmc_info_one_entry > /dev/null 2>&1"
#define PSU_IPMI_TMP_FILE            "/tmp/psu_bmc_info"
#define PSU_IPMI_TMP_FILE_ONE_ENTRY  "/tmp/psu_bmc_info_one_entry"
#define PSU_IPMI_TMP_FILE_FIND_NO_READ  "No Reading"
#define PSU_IPMI_SDR_CMD_PSU  "ipmitool sdr type 0x08"


#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)


typedef enum onlp_psu_info_id_s
{
    PSU_INFO_VIN=0,
    PSU_INFO_VOUT,
    PSU_INFO_IIN,
    PSU_INFO_IOUT,
    PSU_INFO_PIN,
    PSU_INFO_POUT,
    PSU_INFO_TEMP,
    PSU_INFO_MAX,	
}onlp_psu_info_id_t;

typedef struct onlp_psu_dev_info_s
{	
	  onlp_psu_info_id_t  info_id;    
    char *tag;
    char *find;	
}onlp_psu_dev_info_t;


typedef struct onlp_psu_dev_s
{
    onlp_psu_dev_info_t  psu_dev_info_table[PSU_INFO_MAX];
}onlp_psu_dev_t;


onlp_psu_dev_t psu_sensor_table[]= 
{
	{ 
		{
            {PSU_INFO_VIN,  "PSU1_VIN",  "Volts"},
            {PSU_INFO_VOUT, "PSU1_VOUT", "Volts"},
            {PSU_INFO_IIN,  "PSU1_IIN",  "Amps"},
            {PSU_INFO_IOUT, "PSU1_IOUT", "Amps"},
            {PSU_INFO_PIN,  "PSU1_PIN",  "Watts"},    
            {PSU_INFO_POUT, "PSU1_POUT", "Watts"},    
            {PSU_INFO_TEMP, "PSU1_TEMP", "degrees"}
    },
		//"ipmitool sdr type 0x08",		
	},
	{ 
		{
            {PSU_INFO_VIN,  "PSU2_VIN",  "Volts"},
            {PSU_INFO_VOUT, "PSU2_VOUT", "Volts"},
            {PSU_INFO_IIN,  "PSU2_IIN",  "Amps"},
            {PSU_INFO_IOUT, "PSU2_IOUT", "Amps"},
            {PSU_INFO_PIN,  "PSU2_PIN",  "Watts"},    
            {PSU_INFO_POUT, "PSU2_POUT", "Watts"},    
            {PSU_INFO_TEMP, "PSU2_TEMP", "degrees"},
    },
		//"ipmitool sdr type 0x08",		
	}	
};





//ipmitool sdr type 0x08s

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

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
/* return code=0:no such file or the file content is invalid
 * return code=1:file content is valid
 */
static int onlp_psui_check_file_valid(char* const file_path)
{
    struct stat file_info;
    if(stat(file_path ,&file_info)==0)
    {
        if(file_info.st_size==0)
            return 0;
        return 1;
    }
    else
       return 0;   
}

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int ret   = ONLP_STATUS_OK;
    int index = ONLP_OID_ID_GET(id);    
    int size=0;   
    onlp_psu_info_id_t i;    
    char cmd[PSU_MAX_LENGTH/2]={0};
    char psu_val_str[4];
    int  psu_val_int=0;
	  uint8_t  data[PSU_MAX_LENGTH] = {0};
    char  /* *ipmi_cmd,*/ * tag, *find, *p;    

    VALIDATE(id);

    memset(info, 0, sizeof(onlp_psu_info_t));
    *info = pinfo[index]; /* Set the onlp_oid_hdr_t */
    snprintf(cmd , (PSU_MAX_LENGTH/2)-1, "%s > %s", PSU_IPMI_SDR_CMD_PSU, PSU_IPMI_TMP_FILE);
    system(cmd);
    for(i=PSU_INFO_VIN;i<PSU_INFO_MAX;i++)
    {
        system(PSU_IPMI_TMP_FILE_ONE_ENTRY_RM);
        memset(psu_val_str, 0x0, sizeof(psu_val_str));
        
        tag=psu_sensor_table[index-1].psu_dev_info_table[i].tag;
        find=psu_sensor_table[index-1].psu_dev_info_table[i].find;
        snprintf(cmd , (PSU_MAX_LENGTH/2)-1, "cat %s|grep %s > %s", PSU_IPMI_TMP_FILE, tag, PSU_IPMI_TMP_FILE_ONE_ENTRY);
        system(cmd);
        if(!onlp_psui_check_file_valid(PSU_IPMI_TMP_FILE_ONE_ENTRY))
        {
            continue;
        }
        if(onlp_file_read(data, PSU_MAX_LENGTH, &size, PSU_IPMI_TMP_FILE_ONE_ENTRY)!=ONLP_STATUS_OK)
        {
    	      return ONLP_STATUS_E_INTERNAL;
        }
        p=strstr((char*)data, find);
        if(p==NULL)
        {
        	 p=strstr((char*)data, PSU_IPMI_TMP_FILE_FIND_NO_READ);
        	 if(p==NULL)
    	         return ONLP_STATUS_E_INTERNAL;
    	     else
    	         continue;
        }
        if(((uint8_t*)p-data) < sizeof(psu_val_str)/sizeof(char))
        {
            return ONLP_STATUS_E_INTERNAL;
        }
        psu_val_str[0]=(data[(uint8_t*)p-data-4]=='|' || data[(uint8_t*)p-data-4]==' ')?'0':data[(uint8_t*)p-data-4];
        psu_val_str[1]=(data[(uint8_t*)p-data-3]=='|' || data[(uint8_t*)p-data-3]==' ')?'0':data[(uint8_t*)p-data-3];
        psu_val_str[2]=(data[(uint8_t*)p-data-2]=='|' || data[(uint8_t*)p-data-2]==' ')?'0':data[(uint8_t*)p-data-2];
        
        psu_val_int=ONLPLIB_ATOI(psu_val_str);
        if(!strcmp(tag, "PSU1_VIN") || !strcmp(tag, "PSU2_VIN"))
        {
            info->mvin= psu_val_int*100;
            info->caps |= ONLP_PSU_CAPS_VIN;
        }
        if(!strcmp(tag, "PSU1_VOUT") || !strcmp(tag, "PSU2_VOUT"))
        {
           info->mvout= psu_val_int*100;
           info->caps |= ONLP_PSU_CAPS_VOUT;
        }
        if(!strcmp(tag, "PSU1_IIN") || !strcmp(tag, "PSU2_IIN"))
        {
            info->miin= psu_val_int*100;
            info->caps |= ONLP_PSU_CAPS_IIN;
        }
        if(!strcmp(tag, "PSU1_IOUT") || !strcmp(tag, "PSU2_IOUT"))
        {
            info->miout= psu_val_int*100;
            info->caps |= ONLP_PSU_CAPS_IOUT;
        }
        if(!strcmp(tag, "PSU1_PIN") || !strcmp(tag, "PSU2_PIN"))
        {
            info->mpin= psu_val_int*100;
            info->caps |= ONLP_PSU_CAPS_PIN;
        }
        if(!strcmp(tag, "PSU1_POUT") || !strcmp(tag, "PSU2_POUT"))
        {
            info->mpout= psu_val_int*100;
            info->caps |= ONLP_PSU_CAPS_POUT;
        }
        
    }
    system(PSU_IPMI_TMP_FILE_RM);
    if(info->mvin==0 && info->mvout ==0 && info->miin==0)
    	  info->status &= ~ONLP_PSU_STATUS_PRESENT;
    else
    	  info->status |= ONLP_PSU_STATUS_PRESENT;

    if(info->mpout == 0)
    	  info->status |=  ONLP_PSU_STATUS_FAILED;
   
    ret = ONLP_STATUS_OK;	

    return ret;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

