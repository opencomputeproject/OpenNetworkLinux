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
 *
 *
 ***********************************************************/
#include <onlp/platformi/psui.h>
#include <onlplib/mmap.h>
#include <stdio.h>
#include <string.h>
#include "platform_lib.h"
#include "arm_delta_ag6248c_int.h"
#include "arm_delta_i2c.h"

#define PSU_STATUS_PRESENT    			1
#define PSU_STATUS_POWER_GOOD 			1
#define PSU_STATUS_REG					(0X08)
#define PSU_STATUS_PRESENT_BIT(ch)		(0x8<<4*(ch-1))
#define PSU_STATUS_GOOD_BIT(ch)			(0x4<<4*(ch-1))
#define PSU_STATUS_PRESENT_OFFSET(ch)	(4*ch-1)
#define PSU_STATUS_GOOD_OFFSET(ch)		(0x2+4*(ch-1))
#define PSU_PNBUS_VIN_REG				(0x88)
#define PSU_PNBUS_IIN_REG				(0x89)
#define PSU_PNBUS_PIN_REG				(0x97)
#define PSU_PNBUS_VOUT_REG				(0x8b)
#define PSU_PNBUS_IOUT_REG				(0x8c)
#define PSU_PNBUS_POUT_REG				(0x96)
#define PSU_PNBUS_SERIAL_REG			(0x39)
#define PSU_PNBUS_MODEL_REG				(0xc)

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static long psu_data_convert(unsigned int d, int mult)
{
   long X, Y, N, n;

    Y = d & 0x07FF;
    N = (d >> 11) & 0x0f;
    n = d & 0x8000 ? 1 : 0;

    if (n)
        X = (Y * mult) / ((1<<(((~N)&0xf)+1))) ;
    else
        X = (Y * mult) * (N=(1<<(N&0xf)));

    return X;
}

static long psu_data_convert_16(unsigned int d, int mult)
{
      long X;
	  X = (d * mult) / (1 << 9);
      return X;

}

		
static int 
psu_status_info_get(int id, char *node)
{
    int ret;
	int r_data;
	ret=i2c_devname_read_byte("CPLD",PSU_STATUS_REG);
	
	if(ret<0)
		return -1;
	
	if (PSU1_ID == id) {
		if(!strcmp("present",node))
			r_data=!((ret& PSU_STATUS_PRESENT_BIT(id))>> PSU_STATUS_PRESENT_OFFSET(id));
		else if(!strcmp("good",node))
			r_data=((ret& PSU_STATUS_GOOD_BIT(id))>> PSU_STATUS_GOOD_OFFSET(id));
		else
			r_data=-1;
					
    }
    else if (PSU2_ID == id) {
		
    	if(!strcmp("present",node))
			r_data=!((ret& PSU_STATUS_PRESENT_BIT(id))>> PSU_STATUS_PRESENT_OFFSET(id));
		else if(!strcmp("good",node))
			r_data=((ret& PSU_STATUS_GOOD_BIT(id))>> PSU_STATUS_GOOD_OFFSET(id));
		else
			r_data=-1;
	}
    else{
		r_data=-1;
	}
		
	return r_data;
}

static int 
psu_value_info_get(int id, char *type)
{
    int ret;
	char *dev_name;
	int reg_offset;
	
	enum ag6248c_product_id pid = get_product_id();
	
	if(pid == PID_AG6248C_48){
		if(PSU1_ID == id)
			dev_name="PSU1_PMBUS";
		else
			dev_name="PSU2_PMBUS";
	}
	else{
		if(PSU1_ID == id)
			dev_name="PSU1_PMBUS_POE";
		else
			dev_name="PSU2_PMBUS_POE";
	}
	
	if(!strcmp(type,"vin"))
		reg_offset=PSU_PNBUS_VIN_REG;
	else if(!strcmp(type,"iin"))
		reg_offset=PSU_PNBUS_IIN_REG;
	else if(!strcmp(type,"pin"))
		reg_offset=PSU_PNBUS_PIN_REG;
	else if(!strcmp(type,"vout"))
		reg_offset=PSU_PNBUS_VOUT_REG;
	else if(!strcmp(type,"iout"))
		reg_offset=PSU_PNBUS_IOUT_REG;
	else
		reg_offset=PSU_PNBUS_POUT_REG;
	
	ret=i2c_devname_read_word(dev_name,reg_offset);
	
	if(ret<0)
		return -1;
		
	return ret;
}


static int 
psu_serial_model_info_get(int id,char *type,char*data,int data_len)
{
    int i,r_data,re_cnt;
	char *dev_name;
	int reg_offset;
		
	enum ag6248c_product_id pid = get_product_id();
	
	if(pid == PID_AG6248C_48){
		if(PSU1_ID == id)
			dev_name="PSU1_EEPROM";
		else
			dev_name="PSU2_EEPROM";
	}
	else{
		if(PSU1_ID == id)
			dev_name="PSU1_EEPROM_POE";
		else
			dev_name="PSU2_EEPROM_POE";
	}
		
	if(!strcmp(type,"serial"))
		reg_offset=PSU_PNBUS_SERIAL_REG;
	else 
		reg_offset=PSU_PNBUS_MODEL_REG;

	for(i=0;i<data_len;i++){
		re_cnt=3;
		while(re_cnt){
			r_data=i2c_devname_read_byte(dev_name,reg_offset+i);
			if(r_data<0){
				re_cnt--;
				continue;
			}
			data[i]=r_data;
			break;
		}
		if(re_cnt==0){
			AIM_LOG_ERROR("Unable to read the %d reg \r\n",i);
			return ONLP_STATUS_E_INTERNAL;
		}
			
	}

	return ONLP_STATUS_OK;
}


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

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int val   = 0;
    int ret   = ONLP_STATUS_OK;
    int index = ONLP_OID_ID_GET(id);
    psu_type_t psu_type; 
	int r_data;
	char sn_data[15]={0};
	char model_data[17]={0};
	
    VALIDATE(id);

    memset(info, 0, sizeof(onlp_psu_info_t));
	
    *info = pinfo[index]; /* Set the onlp_oid_hdr_t */

    /* Get the present state */
	val=psu_status_info_get(index, "present");
	
    if (val<0) {
		AIM_LOG_INFO("Unable to read PSU %d present value)\r\n", index);
		return ONLP_STATUS_E_INVALID;
    }

	if (val != PSU_STATUS_PRESENT) {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    }
    info->status |= ONLP_PSU_STATUS_PRESENT;

    /* Get power good status */
	val=psu_status_info_get(index,"good");
	
	if (val<0) {
		AIM_LOG_INFO("Unable to read PSU %d good value)\r\n", index);
		return ONLP_STATUS_E_INVALID;
    }

    if (val != PSU_STATUS_POWER_GOOD) {
        info->status |=  ONLP_PSU_STATUS_FAILED;
    }
	
    /* Get PSU type
     */
    psu_type = get_psu_type(index);

    switch (psu_type) {
        case PSU_TYPE_AC_F2B:
        case PSU_TYPE_AC_B2F:
            info->caps = ONLP_PSU_CAPS_AC;
            ret = ONLP_STATUS_OK;
            break;
        case PSU_TYPE_UNKNOWN:  /* User insert a unknown PSU or unplugged.*/
            info->status |= ONLP_PSU_STATUS_UNPLUGGED;
            info->status &= ~ONLP_PSU_STATUS_FAILED;
            ret = ONLP_STATUS_OK;
            break;
        default:
            ret = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }

	/* Get PSU vin,vout*/
     	
	r_data=psu_value_info_get(index,"vin");
	
    if (r_data<0) {
		AIM_LOG_INFO("Unable to read PSU %d Vin value)\r\n", index);
		return ONLP_STATUS_E_INVALID;
    }
	
	info->mvin=psu_data_convert(r_data,1000);
	
	r_data=psu_value_info_get(index,"vout");
	
    if (r_data<0) {
		AIM_LOG_INFO("Unable to read PSU %d Vout value)\r\n", index);
		return ONLP_STATUS_E_INVALID;
    }
	
	info->mvout=psu_data_convert_16(r_data,1000);
	    /* Get PSU iin, iout
     */
	r_data=psu_value_info_get(index,"iin");
	
    if (r_data<0) {
		AIM_LOG_INFO("Unable to read PSU %d Iin value)\r\n", index);
		return ONLP_STATUS_E_INVALID;
    }
	
	info->miin=psu_data_convert(r_data,1000);
	
	r_data=psu_value_info_get(index,"iout");
	
    if (r_data<0) {
		AIM_LOG_INFO("Unable to read PSU %d Iout value)\r\n", index);
		return ONLP_STATUS_E_INVALID;
    }
	
	info->miout=psu_data_convert(r_data,1000);
	
	/* Get PSU pin, pout
     */
	r_data=psu_value_info_get(index,"pin");
	
    if (r_data<0) {
		AIM_LOG_INFO("Unable to read PSU %d Pin value)\r\n", index);
		return ONLP_STATUS_E_INVALID;
    }
	
	info->mpin=psu_data_convert(r_data,1000);
	
	r_data=psu_value_info_get(index,"pout");
	
    if (r_data<0) {
		AIM_LOG_INFO("Unable to read PSU %d Pout value)\r\n", index);
		return ONLP_STATUS_E_INVALID;
    }
	
	info->mpout=psu_data_convert(r_data,1000);
	/* Get PSU serial
     */

	ret=psu_serial_model_info_get(index,"serial",sn_data,14);
	if (ret!=ONLP_STATUS_OK) {
		AIM_LOG_INFO("Unable to read PSU %d SN value)\r\n", index);
		return ONLP_STATUS_E_INVALID;
    }
	
	strcpy(info->serial,sn_data);
	
	/* Get PSU model
     */
	ret=psu_serial_model_info_get(index,"model",model_data,16);
	if (ret!=ONLP_STATUS_OK) {
		AIM_LOG_INFO("Unable to read PSU %d model value)\r\n", index);
		return ONLP_STATUS_E_INVALID;
    }
	
	strcpy(info->model,model_data);

    return ONLP_STATUS_OK;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

