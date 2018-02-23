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
#include <onlp/platformi/sfpi.h>
#include "platform_lib.h"

#include <arm_delta_ag6248c/arm_delta_ag6248c_config.h>
#include "arm_delta_ag6248c_log.h"
#include "arm_delta_i2c.h"

#define SFP_PRESENT_REG				(0X14)
#define SFP_RX_LOS_REG				(0X11)
#define SFP_TX_DISABLE_REG			(0X17)
#define SFP_PRESENT_PORT47_BIT		(0X40)
#define SFP_PRESENT_PORT48_BIT		(0X80)
#define SFP_PRESENT_PORT47_OFFSET	(0X06)
#define SFP_PRESENT_PORT48_OFFSET	(0X07)


/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
int
onlp_sfpi_init(void)
{
    /* Called at initialization time */
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;
    int start_port, end_port;

    if((platform_id == PLATFORM_ID_POWERPC_DELTA_AG6248C_POE_R0)||
		(platform_id ==PLATFORM_ID_POWERPC_DELTA_AG6248C_R0))
    {
        start_port = 47;
        end_port   = 48;
    }
    else /*reserved*/
    {
		AIM_LOG_ERROR("The platform id %d is invalid \r\n", platform_id);
		return ONLP_STATUS_E_UNSUPPORTED;
    }

    for(p = start_port; p <=end_port; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */
    int present,r_data;
	
	if((port==47)||(port==48))
		r_data=i2c_devname_read_byte("CPLD", SFP_PRESENT_REG);
	else{
		AIM_LOG_ERROR("The port %d is invalid \r\n", port);
		return ONLP_STATUS_E_UNSUPPORTED;
	}
		
	if(r_data<0){
		AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
		return ONLP_STATUS_E_INTERNAL;
	}
		
	if(port==47){
		r_data&=SFP_PRESENT_PORT47_BIT;
		present=!(r_data>>SFP_PRESENT_PORT47_OFFSET);
	}
	else{
		r_data&=SFP_PRESENT_PORT48_BIT;
		present=!(r_data>>SFP_PRESENT_PORT48_OFFSET);
	}
		
    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int status;
	int port, i = 0;
	uint64_t presence_all=0;
		
    AIM_BITMAP_CLR_ALL(dst);
	
	 if((platform_id == PLATFORM_ID_POWERPC_DELTA_AG6248C_POE_R0)|| \
		(platform_id ==PLATFORM_ID_POWERPC_DELTA_AG6248C_R0)){
			
		port=47;
		
	}
	else{
        AIM_LOG_ERROR("The platform id %d is invalid \r\n", platform_id);
		return ONLP_STATUS_E_UNSUPPORTED;
    }
 
	status=i2c_devname_read_byte("CPLD", SFP_PRESENT_REG);
	
    if(status<0){
        AIM_LOG_ERROR("Unable to read the sfp_is_present_all value. \r\n");
        return ONLP_STATUS_E_INTERNAL;
    }
	status=~status;
	
	status>>=6;
	
	/* Convert to 64 bit integer in port order */
    
	presence_all = status & 0x3;
	
    presence_all <<= port;
	
    /* Populate bitmap */
    for(i = 0; presence_all; i++) {
        AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int status;
    int   port,i = 0;
	uint64_t rx_los_all = 0;

    AIM_BITMAP_CLR_ALL(dst);
	
	if((platform_id == PLATFORM_ID_POWERPC_DELTA_AG6248C_POE_R0)|| \
		(platform_id ==PLATFORM_ID_POWERPC_DELTA_AG6248C_R0)){
			
		port=47;
		
	}
	else{
        AIM_LOG_ERROR("The platform id %d is invalid \r\n", platform_id);
		return ONLP_STATUS_E_UNSUPPORTED;
    }
 
	status=i2c_devname_read_byte("CPLD", SFP_RX_LOS_REG);

    if(status<0){
        AIM_LOG_ERROR("Unable to read the rx loss reg value. \r\n");
        return ONLP_STATUS_E_INTERNAL;
    }
	
	status>>=6;

    /* Convert to 64 bit integer in port order */
    rx_los_all = status & 0x3;
	
    rx_los_all <<= port;

    /* Populate bitmap */
    for(i = 0; rx_los_all; i++) {
        AIM_BITMAP_MOD(dst, i, (rx_los_all & 1));
        rx_los_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
	 
	int i, r_data, re_cnt;
	char* sfp_name;
	
    memset(data, 0, 256);
	
	if(port==47)
		sfp_name="SFP1";
	else
		sfp_name="SFP2";
	

	for(i=0;i<256;i++){
		re_cnt=3;
		while(re_cnt){
			r_data=i2c_devname_read_byte(sfp_name,i);
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
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    
    return onlp_sfpi_eeprom_read( port, data);
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    /*value is 1 if the tx disable
	  value is 0 if the tx enable
	*/
	
	int rc,r_data,dis_value,present;
	
	present=onlp_sfpi_is_present(port);
	
	if(present==0){
		AIM_LOG_INFO("The port %d is not present and can not set tx disable\r\n",port);
		return ONLP_STATUS_E_UNSUPPORTED;
	}
			
	r_data=i2c_devname_read_byte("CPLD", SFP_TX_DISABLE_REG);

	if(r_data<0){
		AIM_LOG_INFO("Unable to read sfp tx disable reg value\r\n");
        return ONLP_STATUS_E_INTERNAL;
	}
		
	if(port==47){
		r_data&=~(0x1<<SFP_PRESENT_PORT47_OFFSET);
		dis_value=value<<SFP_PRESENT_PORT47_OFFSET;
	}
	else{
		r_data&=~(0x1<<SFP_PRESENT_PORT48_OFFSET);
		dis_value=value<<SFP_PRESENT_PORT48_OFFSET;
	}
		
	dis_value|=r_data;
		
    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                rc = i2c_devname_write_byte("CPLD", SFP_TX_DISABLE_REG, dis_value);

                if (rc<0) {
                    AIM_LOG_ERROR("Unable to set tx_disable status to port(%d)\r\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
                break;
            }

        default:
            return ONLP_STATUS_E_UNSUPPORTED;
        }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
	int r_data,present;
	
	present=onlp_sfpi_is_present(port);
	
	if(present==0){
		AIM_LOG_INFO("The port %d is not present\r\n",port);
		return ONLP_STATUS_E_UNSUPPORTED;
	}

    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            {
                r_data=i2c_devname_read_byte("CPLD", SFP_RX_LOS_REG);
		
				if (r_data<0) {
                    AIM_LOG_ERROR("Unable to read rx_los status from port(%d)\r\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }

                break;
            }

        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                r_data=i2c_devname_read_byte("CPLD", SFP_TX_DISABLE_REG);

                if (r_data<0) {
                    AIM_LOG_ERROR("Unable to read tx_disabled status from port(%d)\r\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }

                break;
            }

        default:
            return ONLP_STATUS_E_UNSUPPORTED;
        }
		
		if(port==47){
			r_data&=(0x1<<SFP_PRESENT_PORT47_OFFSET);
			*value=(r_data >>SFP_PRESENT_PORT47_OFFSET);
		}
		else{
			r_data&=(0x1<<SFP_PRESENT_PORT48_OFFSET);
			*value=(r_data >>SFP_PRESENT_PORT48_OFFSET);
		}
	
    return ONLP_STATUS_OK;
}


int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
