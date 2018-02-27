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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <unistd.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <fcntl.h>
#include "platform_lib.h"
#include "x86_64_delta_ag7648_log.h"
#include <stdio.h>
#include "x86_64_delta_i2c.h"
#define prefix_path "/sys/bus/i2c/devices/"
#define LOCAL_DEBUG 0

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_1_CLOSE_TO_CPU,
    THERMAL_1_CLOSE_TO_MAC,
	THERMAL_2_CLOSE_TO_PHY_SFP_PLUS,
	THERMAL_3_CLOSE_TO_PHY_QSFP,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

static char* last_path[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    "2-004d/hwmon/hwmon1/temp1_input",
    "3-004c/hwmon/hwmon2/temp1_input",
    "3-004d/hwmon/hwmon3/temp1_input",
    "3-004e/hwmon/hwmon4/temp1_input",
};
	
/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_CLOSE_TO_CPU), "Thermal Sensor 1- close to cpu", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_CLOSE_TO_MAC), "Thermal Sensor 1- close to mac", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_CLOSE_TO_PHY_SFP_PLUS), "Thermal Sensor 2- close to sfp+ phy", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_CLOSE_TO_PHY_QSFP), "Thermal Sensor 2- close to qsfp phy", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(1)},
	    	ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(2)},
	    	ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }
};

/*thermali val to real tempeture*/
static int
_onlp_psu_thermali_val_to_temperature (int v,int mult)
{
    long X, Y, N, n;
    Y = v & 0x07FF;
    N = (v >> 11) & 0x0f;
    n = v & 0x8000 ? 1 : 0;
    if (n)
         X = (Y * mult) / ((1<<(((~N)&0xf)+1))) ;
    else
         X = Y * mult * (N=(1<<(N&0xf)));
    return X;
}

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{ 
    return ONLP_STATUS_OK;
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
_onlp_thermali_info_get(int id, onlp_thermal_info_t* info)
{
    int   len, nbytes = 10, temp_base=1, local_id;
    uint8_t r_data[10]={0};
    char  fullpath[50] = {0};

    local_id = id;
    
    DEBUG_PRINT("\n[Debug][%s][%d][local_id: %d]", __FUNCTION__, __LINE__, local_id);
    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];
    /* get fullpath */
    sprintf(fullpath, "%s%s", prefix_path, last_path[local_id]);

    //OPEN_READ_FILE(fd, fullpath, r_data, nbytes, len);
    onlp_file_read(r_data,nbytes,&len, fullpath);
    
    info->mcelsius =ONLPLIB_ATOI((char*)r_data) / temp_base;
    
    DEBUG_PRINT("\n[Debug][%s][%d][save data: %d]\n", __FUNCTION__, __LINE__, info->mcelsius);

    return ONLP_STATUS_OK;
}

/*psu temperture info get*/
static int
_onlp_thermali_psu_info_get(int id, onlp_thermal_info_t* info)
{   
    int psu_present,psu_good;
    int psu_id,local_id;
    int r_data,temperature_v;
    enum ag7648_product_id pid;
    
    local_id=id;
    
    DEBUG_PRINT("\n[Debug][%s][%d][local_id: %d]", __FUNCTION__, __LINE__, local_id);
    
    psu_id=(local_id-THERMAL_1_ON_PSU1)+1;
    pid=get_product_id();
    //if the psu is not, directly to return
    psu_present=psu_status_info_get(psu_id, "present");
    if(psu_present != 1){
        info->status &= ~ONLP_THERMAL_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    }
	psu_good= psu_status_info_get(psu_id,"good");
	if(psu_good != 0){
		info->status |= ONLP_THERMAL_STATUS_FAILED;
		return ONLP_STATUS_OK;
	}

    //read the pus temperture register value     
    if(pid == PID_AG7648){
        if(psu_id==1)
            r_data=i2c_devname_read_word("PSU1_PMBUS", 0x8d);
        else
            r_data=i2c_devname_read_word("PSU2_PMBUS", 0x8d);
    }
	else{
		DEBUG_PRINT("\n[Debug][%s][%d][unsupported board:%d]", __FUNCTION__, __LINE__, pid);
		return ONLP_STATUS_E_UNSUPPORTED;
	}
    if(r_data<0)
        return ONLP_STATUS_E_INVALID;
    //get the real temperture value
    temperature_v=_onlp_psu_thermali_val_to_temperature(r_data,1000);

    info->mcelsius=temperature_v;

    DEBUG_PRINT("\n[Debug][%s][%d][save data: %d]\n", __FUNCTION__, __LINE__, info->mcelsius);

    return ONLP_STATUS_OK;

}

int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{   
    int rc;
    int local_id;

    VALIDATE(id);

    local_id=ONLP_OID_ID_GET(id);
	
	if((local_id > THERMAL_1_ON_PSU2) || (local_id < THERMAL_1_CLOSE_TO_CPU)){
		DEBUG_PRINT("\n[Debug][%s][%d][outside addr:%d]", __FUNCTION__, __LINE__, local_id);
		return ONLP_STATUS_E_INVALID;
	}

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];

    if((local_id==THERMAL_1_ON_PSU1) || (local_id==THERMAL_1_ON_PSU2))
		rc= _onlp_thermali_psu_info_get(local_id,info);
	else
        rc= _onlp_thermali_info_get(local_id,info);
    
	return rc;
}
