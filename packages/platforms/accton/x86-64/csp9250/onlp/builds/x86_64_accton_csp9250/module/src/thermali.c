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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
//#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include "platform_lib.h"

//#define PSU_THERMAL_PATH_FORMAT "/sys/bus/i2c/devices/%s/*psu_temp1_input"
#define THERMAL_IPMI_TMP_FILE "/tmp/thermal_bmc_info"
#define THERMAL_IPMI_TMP_FILE_RM "rm -f /tmp/thermal_bmc_info > /dev/null 2>&1"
#define THERMAL_IPMI_TMP_FILE_FIND "degrees"
#define THERMAL_CHECK_TIME       "/usr/bin/thermal_check_time"
#define THERMAL_IPMI_SDR_CMD         "ipmitool sdr list"
#define THERMAL_IPMI_SDR_FILE     "/usr/bin/thermal_bmc_sdr"
#define THERMAL_IPMI_SDR_FILE_RM     "rm -f /usr/bin/thermal_bmc_sdr > /dev/null 2>&1"

#define THERMAL_CAN_SET_IPMI_TIME    10
#define THERMAL_MAX_LENGTH     256
#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_SWITCH_BOARD,
    THERMAL_2_ON_SWITCH_BOARD,
    THERMAL_3_ON_SWITCH_BOARD,
    THERMAL_1_ON_SERVER_BOARD,
    THERMAL_2_ON_SERVER_BOARD,
    THERMAL_3_ON_SERVER_BOARD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

typedef struct onlp_thermal_dev_s
{
   int thermal_id;
   char *ipmi_cmd;
   char *tag;
    	
}onlp_thermal_dev_t;

onlp_thermal_dev_t thermal_sensor_table[]=
{
    {THERMAL_RESERVED, NULL, NULL},
    {THERMAL_CPU_CORE, NULL, NULL},
    {THERMAL_1_ON_SWITCH_BOARD, "ipmitool sdr list", "Temp_LM75_Power"},
    {THERMAL_2_ON_SWITCH_BOARD, "ipmitool sdr list", "Temp_LM75_LEFT"},
    {THERMAL_3_ON_SWITCH_BOARD, "ipmitool sdr list", "Temp_LM75_HS"},    
    {THERMAL_1_ON_SERVER_BOARD, "ipmitool sdr list", "Temp_LM75_CPU0"},
    {THERMAL_2_ON_SERVER_BOARD, "ipmitool sdr list", "Temp_LM75_CPU1"},
    {THERMAL_3_ON_SERVER_BOARD, "ipmitool sdr list", "Temp_LM75_PCH"},
    {THERMAL_1_ON_PSU1, "ipmitool sdr list", "PSU1_TEMP"},
    {THERMAL_1_ON_PSU2, "ipmitool sdr list", "PSU2_TEMP"}
};

char *onlp_find_thermal_sensor_cmd(unsigned int id)
{
	  int i;
	  for(i=0; i<sizeof(thermal_sensor_table)/sizeof(onlp_thermal_dev_t) ; i++)
	  {
	      if(thermal_sensor_table[i].thermal_id == id)
	      	return thermal_sensor_table[i].ipmi_cmd;
	  }
    return NULL;    
} 

char *onlp_find_thermal_sensor_tag(unsigned int id)
{
	  int i; 
	  
    for(i=0; i<sizeof(thermal_sensor_table)/sizeof(onlp_thermal_dev_t) ; i++)
	  {
	      if(thermal_sensor_table[i].thermal_id == id)
	      	return thermal_sensor_table[i].tag;
	  }
	  
	  return NULL;     
}

#if 0
static char* directory[] =  /* must map with onlp_thermal_id */
{
    NULL,
    NULL,                  /* CPU_CORE files */
    "3-0048",
    "3-0049",
    "3-004a",
    "3-004b",
    "11-0059",
    "10-0058",	
};
#endif

static char* cpu_coretemp_files[] =
    {
        "/sys/devices/platform/coretemp.0*temp2_input",
        "/sys/devices/platform/coretemp.0*temp3_input",
        "/sys/devices/platform/coretemp.0*temp4_input",
        "/sys/devices/platform/coretemp.0*temp5_input",
        NULL,
    };

/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },	
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_SWITCH_BOARD), "Temp_LM75_Power", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_SWITCH_BOARD), "Temp_LM75_LEFT", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_SWITCH_BOARD), " Temp_LM75_HS", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_SERVER_BOARD), "Temp_LM75_CPU0", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
 { { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_SERVER_BOARD), "Temp_LM75_CPU1", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
 { { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_SERVER_BOARD), "Temp_LM75_PCH", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID)}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID)}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        }
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    char     cmd[THERMAL_MAX_LENGTH/2] = {0};
    snprintf(cmd, (THERMAL_MAX_LENGTH/2) -1, "echo 0 >  %s ", THERMAL_CHECK_TIME);
    system(cmd);
    system("echo V0002 > /etc/onlp_drv_version");
    return ONLP_STATUS_OK;
}


static int 
thermali_time_exist(void)
{
    struct stat file_info;    
    if(stat(THERMAL_CHECK_TIME ,&file_info)==0)
    {
        if(file_info.st_size==0)
            return 0;
        return 1;      
    }
    else
       return 1;
           
}
static int 
thermali_sdr_file_exist(void)
{
    struct stat file_info;
    if(stat(THERMAL_IPMI_SDR_FILE ,&file_info)==0)
    {
        if(file_info.st_size==0)
            return 0;
        return 1;
    }
    else
       return 0;
          
}


/*
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */
int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    int   tid;
    int size;
    char     cmd[THERMAL_MAX_LENGTH/2] = {0};
    char     temp[4];
    uint8_t  data[THERMAL_MAX_LENGTH] = {0};
    //char  *ipmi_cmd;
    char  * tag, *p;
    struct  timeval    new_tv;
    long    last_time;
    int     get_data_by_ipmi=0;
       
    VALIDATE(id);
	
    tid = ONLP_OID_ID_GET(id);
	
    if(tid > THERMAL_1_ON_PSU2 || tid <= THERMAL_RESERVED)
        return ONLP_STATUS_E_INTERNAL;
    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[tid];
    
    if(tid == THERMAL_CPU_CORE) {    	 
        return onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
    }
    if(thermali_time_exist())
    {
        if(onlp_file_read(data, THERMAL_MAX_LENGTH, &size, THERMAL_CHECK_TIME)!=ONLP_STATUS_OK)
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
             if((new_tv.tv_sec - last_time) > THERMAL_CAN_SET_IPMI_TIME)
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
    memset(data ,0x0, THERMAL_MAX_LENGTH);
    snprintf((char*)data, THERMAL_MAX_LENGTH-1, "%ld", new_tv.tv_sec);
    if(onlp_file_write_str((char*)data, THERMAL_CHECK_TIME)!=ONLP_STATUS_OK)
    {
        return ONLP_STATUS_E_INTERNAL;
    }
    if(get_data_by_ipmi || !thermali_sdr_file_exist()) /* Set ipmitool cmd to get all data and save to file*/
    {
        /* set ipmi cmd */
        snprintf(cmd, (THERMAL_MAX_LENGTH/2) -1, "%s > %s ", THERMAL_IPMI_SDR_CMD, THERMAL_IPMI_SDR_FILE);
        system(cmd);
        fflush(stdout);
    }    
    tag= thermal_sensor_table[tid].tag;
    
    if(tag==NULL)
        return ONLP_STATUS_E_INTERNAL; 
    
    snprintf(cmd, (THERMAL_MAX_LENGTH/2) -1, "cat %s|grep %s > %s ", THERMAL_IPMI_SDR_FILE, tag, THERMAL_IPMI_TMP_FILE);
    system(cmd);
    
    if(onlp_file_read(data, THERMAL_MAX_LENGTH, &size, THERMAL_IPMI_TMP_FILE)!=ONLP_STATUS_OK)
    {
        return ONLP_STATUS_E_INTERNAL;
    }
    p=strstr((char*)data, THERMAL_IPMI_TMP_FILE_FIND);
    if(p==NULL)
    {
        return ONLP_STATUS_E_INTERNAL;
    }
    if(((uint8_t*)p-data) < sizeof(temp)/sizeof(char))
    {
       return ONLP_STATUS_E_INTERNAL;
    }
    
    temp[0]=data[(uint8_t*)p-data-4];
    temp[1]=data[(uint8_t*)p-data-3];
    temp[2]=data[(uint8_t*)p-data-2];
    info->mcelsius=ONLPLIB_ATOI(temp) * 1000;
       
    return ONLP_STATUS_OK;
}


