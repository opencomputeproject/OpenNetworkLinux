/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2018 Alpha Networks Incorporation.
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
#include <stdio.h>
#include <string.h>
#include "platform_lib.h"

#define PSUI_DEBUG_FLAG         0


#define PSU_STATUS_INTERRUPT    3
#define PSU_STATUS_PRESENT      2
#define PSU_STATUS_POWER_OK     1
#define PSU_STATUS_POWER_ON     0

#define PSU_NODE_MAX_INT_LEN    8
#define PSU_NODE_MAX_PATH_LEN   64
#define PSUI_CPLD_BUS_ID        0
#define PSUI_POWER_CPLD_ADDR    0x5E
#define RPS_PSU_EEPROM_ADDR     0x51
#define RPS_PSU_MICRO_P_ADDR    0x59

#define PSUI_MANUFACTURER_NAME_REG  0x0C
#define PSUI_PRODUCT_NAME_REG       0x12
#define PSUI_MODEL_NO_REG           0x1D
#define PSUI_PRODUCT_VER_NO_REG     0x1F
#define PSUI_PRODUCT_SER_NO_REG_F2B     0x23
#define PSUI_PRODUCT_SER_NO_REG_B2F     0x25
#define PSUI_PRODUCT_SER_NO_REG         PSUI_PRODUCT_SER_NO_REG_B2F
#define PSUI_RPS_STATUS_REG0        0x04 /* PSU Status Register for PSU#1 */
#define PSUI_RPS_STATUS_REG1        0x03 /* PSU Status Register for PSU#2 */

#define PSUI_PRODUCT_SER_NO_SIZE    14
#define PSUI_PRODUCT_SER_NO_LEN     (PSUI_PRODUCT_SER_NO_SIZE + 1)
#define PSUI_PRODUCT_NAME_SIZE      13
#define PSUI_PRODUCT_NAME_SIZE_F2B      11
#define PSUI_PRODUCT_NAME_SIZE_B2F      13

struct psui_reg_data_word
{
    uint8_t   reg;
    uint16_t value;
};

struct psui_reg_data_word regs_word[] = { {0x88, 0},  /* READ_Vin */
                                          {0x8b, 0},  /* READ_Vout */
                                          {0x89, 0},  /* READ_Iin */
                                          {0x8c, 0},  /* READ_Iout */
                                          {0x96, 0},  /* READ_Pout */
                                          {0x97, 0}   /* READ_Pin */
                                        };


#define GET_RPS_STATUS_BIT(x,n) (((x) >> (n)) & 1)
#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)


int 
psu_ne2572_linear_format(int exponent, int mantissa)
{
    int multiplier = 1000;/* Units are "milli-" */

    return (exponent >= 0) ? (mantissa << exponent) * multiplier :
                                   (mantissa * multiplier) / (1 << -exponent);
}

int
onlp_psui_init(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    return ONLP_STATUS_OK;
}


static int 
psu_info_get_product_name(int id, UI8_T *data)
{
    DIAG_PRINT("%s, id:%d", __FUNCTION__, id);
    int ret = 0;

    ret = i2c_read_block(id, RPS_PSU_EEPROM_ADDR, PSUI_PRODUCT_NAME_REG, data, PSUI_PRODUCT_NAME_SIZE);
    if (ret < 0 && PSUI_DEBUG_FLAG)
        printf("I2C command 0x%X Read Fail, id=%d\n", PSUI_PRODUCT_NAME_REG,id);

    return ret;
}

static int 
psu_info_get_product_ser(psu_type_t psu_type, int id, UI8_T *data)
{
    int ret = 0;
    int addr = 0 ;
    DIAG_PRINT("%s, id:%d", __FUNCTION__, id);
    if (psu_type == PSU_TYPE_AC_F2B)
    {
        addr = PSUI_PRODUCT_SER_NO_REG_F2B;
    }
    else
    {
        addr = PSUI_PRODUCT_SER_NO_REG_B2F;
    }

    ret = i2c_read_block(id, RPS_PSU_EEPROM_ADDR, addr, data, PSUI_PRODUCT_SER_NO_SIZE);
    if (ret < 0)
        printf("I2C command 0x%X Read Fail, id=%d\n", addr,id);
    return ret;
}

static int 
psu_info_get_status(int id, char *data)
{
    DIAG_PRINT("%s, id:%d", __FUNCTION__, id);
    int ret = 0;

    if (id == PSU1_ID)
    {
        ret = i2c_read_rps_status(PSUI_CPLD_BUS_ID, PSUI_POWER_CPLD_ADDR, PSUI_RPS_STATUS_REG0);
        if (ret < 0)
            printf("I2C command 0x%X Read Fail, id=%d\n", PSUI_RPS_STATUS_REG0, PSUI_CPLD_BUS_ID);
    }
    else if (id == PSU2_ID)
    {
        ret = i2c_read_rps_status(PSUI_CPLD_BUS_ID, PSUI_POWER_CPLD_ADDR, PSUI_RPS_STATUS_REG1);
        if (ret < 0)
            printf("I2C command 0x%X Read Fail, id=%d\n", PSUI_RPS_STATUS_REG1, PSUI_CPLD_BUS_ID);
    }
    sprintf (data, "%d\n", ret);
    return ret;
}


static int
psu_ne2572_info_get(int id, onlp_psu_info_t* info)
{
    DIAG_PRINT("%s, id:%d", __FUNCTION__, id);
    //int val   = 0;
    //int index = ONLP_OID_ID_GET(info->hdr.id);

    /* Set capability
     */
    info->caps = ONLP_PSU_CAPS_AC;
    
    if (info->status & ONLP_PSU_STATUS_FAILED)
    {
        return ONLP_STATUS_OK;
    }

    /* Set the associated oid_table */
    //info->hdr.coids[0] = ONLP_FAN_ID_CREATE(index + CHASSIS_FAN_COUNT);
    //info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(index + CHASSIS_THERMAL_COUNT);
    
#if 1   
    int i = 0;
    int status;
    int exponent, mantissa;
    for (i = 0; i < 6; i++)
    {
        //printf("I2C register 0x%X \n", regs_word[i].reg);
        status = i2c_read_word(id, RPS_PSU_MICRO_P_ADDR, regs_word[i].reg);
        if (status < 0)
            printf("I2C command 0x%X Read Fail\n", regs_word[i].reg);
        else
        {
            //printf("I2C command 0x%X Read Success: 0x%02x\n", regs_word[i].reg, status);
            regs_word[i].value = status;
        }
    }
    
    info->caps |= ONLP_PSU_CAPS_VOUT;
    info->caps |= ONLP_PSU_CAPS_VIN;
    info->caps |= ONLP_PSU_CAPS_IOUT;
    info->caps |= ONLP_PSU_CAPS_IIN;
    info->caps |= ONLP_PSU_CAPS_POUT;
    info->caps |= ONLP_PSU_CAPS_PIN;
    
    /* Linear_5s_11s */
    exponent = psu_two_complement_to_int(regs_word[0].value >> 11, 5, 0x1f);
    mantissa = psu_two_complement_to_int(regs_word[0].value & 0x7ff, 11, 0x7ff);
    info->mvin = psu_ne2572_linear_format(exponent, mantissa);

    /* Linear_16u */
    exponent = -12; 
    mantissa = regs_word[1].value;
    info->mvout = psu_ne2572_linear_format(exponent, mantissa);

    exponent = psu_two_complement_to_int(regs_word[2].value >> 11, 5, 0x1f);
    mantissa = psu_two_complement_to_int(regs_word[2].value & 0x7ff, 11, 0x7ff);
    info->miin = psu_ne2572_linear_format(exponent, mantissa);
    
    exponent = psu_two_complement_to_int(regs_word[3].value >> 11, 5, 0x1f);
    mantissa = psu_two_complement_to_int(regs_word[3].value & 0x7ff, 11, 0x7ff);
    info->miout = psu_ne2572_linear_format(exponent, mantissa);
    
    exponent = psu_two_complement_to_int(regs_word[4].value >> 11, 5, 0x1f);
    mantissa = psu_two_complement_to_int(regs_word[4].value & 0x7ff, 11, 0x7ff);
    info->mpout = psu_ne2572_linear_format(exponent, mantissa);
    
    exponent = psu_two_complement_to_int(regs_word[5].value >> 11, 5, 0x1f);
    mantissa = psu_two_complement_to_int(regs_word[5].value & 0x7ff, 11, 0x7ff);
    info->mpin = psu_ne2572_linear_format(exponent, mantissa);
#endif
    return ONLP_STATUS_OK;
}

int
psu_um400d_info_get(onlp_psu_info_t* info)
{
    DIAG_PRINT("%s", __FUNCTION__);
    int index = ONLP_OID_ID_GET(info->hdr.id);

    /* Set capability
     */
    info->caps = ONLP_PSU_CAPS_DC48;

    if (info->status & ONLP_PSU_STATUS_FAILED)
    {
        return ONLP_STATUS_OK;
    }

    /* Set the associated oid_table */
    info->hdr.coids[0] = ONLP_FAN_ID_CREATE(index + CHASSIS_FAN_COUNT);

    return ONLP_STATUS_OK;
}

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    {    /* PSU-1 is on i2c channel 10 for ne2572*/
        { ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0 },
    },
    {    /* PSU-2 is on i2c channel 11 for ne2572*/
        { ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0 },
    }
};

psu_type_t get_psu_type(int id, char* product_name, int productname_len)
{
    DIAG_PRINT("%s, id:%d", __FUNCTION__, id);
    UI8_T p_name[PSUI_PRODUCT_NAME_SIZE + 1] = {0};

    /* Check AC model name */
    if (psu_info_get_product_name(id, p_name) >= 0)
    {
        //printf("[psu_info_get_product_name] %s\n", p_name);
        if (strncmp((char*)p_name, "DPS-770GB E", strlen("DPS-770GB E")) == 0)
        {
            if (product_name)
                memcpy(product_name, p_name, PSUI_PRODUCT_NAME_SIZE_F2B);
            
            return PSU_TYPE_AC_F2B;
        }
        else if (strncmp((char*)p_name, "DPS-770GB-1 A", strlen("DPS-770GB-1 A")) == 0)
        {
            if (product_name)
                memcpy(product_name, p_name, PSUI_PRODUCT_NAME_SIZE_B2F);
            
            return PSU_TYPE_AC_B2F;
        }
        #if 0
        else if (strncmp((char*)p_name, "DPS-770GB", strlen("DPS-770GB")) == 0)
        {
            if (product_name)
                memcpy(product_name, p_name, productname_len-1);
            
            return PSU_TYPE_AC_B2F;
        }
        #endif
    }
    return PSU_TYPE_UNKNOWN;
}


int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    char                  name[ONLP_CONFIG_INFO_STR_MAX];
    char                  status;
    int                   ret = ONLP_STATUS_OK;
    int                   index = ONLP_OID_ID_GET(id);
    psu_type_t            psu_type; 
    
    UI8_T product_ser[PSUI_PRODUCT_SER_NO_LEN];
    
    UI8_T rps_status = 0, power_ok = 0, power_on = 0, power_present = 0;

    VALIDATE(id);

    memset(info, 0, sizeof(onlp_psu_info_t));
    memset(name, 0, sizeof(name));
    *info = pinfo[index]; /* Set the onlp_oid_hdr_t */

    /* Get PSU type and product name, bus ID=index+10*/
	if( index == 1)
    	psu_type = get_psu_type(PSU1_BUS_ID, info->model, sizeof(info->model));
	else if( index == 2)
		psu_type = get_psu_type(PSU2_BUS_ID, info->model, sizeof(info->model));
	 
    //debug
    DIAG_PRINT("%s, id:%d, index:%d, psu_type:%d\n",__FUNCTION__,id,index,psu_type);
    switch (psu_type)
    {
        case PSU_TYPE_AC_F2B:
        case PSU_TYPE_AC_B2F:
			if( index == 1)
            ret = psu_ne2572_info_get(PSU1_BUS_ID, info);/* Get PSU electric info from PMBus */
			else if (index == 2)
				ret = psu_ne2572_info_get(PSU2_BUS_ID, info);/* Get PSU electric info from PMBus */
            break;
        case PSU_TYPE_DC_48V_F2B:
        case PSU_TYPE_DC_48V_B2F:
            ret = psu_um400d_info_get(info);
            break;
        default:
            ret = ONLP_STATUS_E_UNSUPPORTED;
            return ret;
    }

    /* Get the product serial number, bus ID=index+10 */
	if(index == 1) 
	{
	    if (psu_info_get_product_ser(psu_type, PSU1_BUS_ID, product_ser) < 0) 
	    {
	        printf("Unable to read PSU(%d) item(serial number)\r\n", index);
	    }
	    else
	    {
	        memcpy(info->serial, product_ser, sizeof(product_ser));
	    }
	} 
	else if( index == 2) 
	{
	   if (psu_info_get_product_ser(psu_type, PSU2_BUS_ID, product_ser) < 0) 
	    {
	        printf("Unable to read PSU(%d) item(serial number)\r\n", index);
	    }
	    else
	    {
	        memcpy(info->serial, product_ser, sizeof(product_ser));
	    }
	}
    /* Get psu status from CPLD */
    if (psu_info_get_status(index, &status) < 0) 
    {
        printf("Unable to read PSU(%d) item(psu status)\r\n", index);
    }
    else
    {
        rps_status = (UI8_T)atoi(&status);
        power_present = GET_RPS_STATUS_BIT(rps_status, PSU_STATUS_PRESENT);
        power_ok = GET_RPS_STATUS_BIT(rps_status, PSU_STATUS_POWER_OK);
        power_on = GET_RPS_STATUS_BIT(rps_status, PSU_STATUS_POWER_ON);

        /* Empty */
        if (!power_present)
        {
            info->status |= ONLP_PSU_STATUS_UNPLUGGED;
        }

        if (!power_ok)
        {
            info->status |= ONLP_PSU_STATUS_FAILED;
        }

        if (power_on)
        {
            info->status |= ONLP_PSU_STATUS_PRESENT;
        }

        DIAG_PRINT("rps_status:0x%x ,info->status:0x%x\n", rps_status, info->status);
        DIAG_PRINT("present:%d, ok:%d, on:%d\n", power_present, power_ok, power_on);

    }



    return ret;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    DIAG_PRINT("%s, pid=%d", __FUNCTION__, pid);
    return ONLP_STATUS_E_UNSUPPORTED;
}

