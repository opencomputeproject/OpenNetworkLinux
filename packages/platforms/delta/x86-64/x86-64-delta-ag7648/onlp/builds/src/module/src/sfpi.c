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

#include <x86_64_delta_ag7648/x86_64_delta_ag7648_config.h>
#include "x86_64_delta_ag7648_log.h"
#include "x86_64_delta_i2c.h"

#define SFP_PLUS_MIN_PORT 1
#define SFP_PLUS_MAX_PORT 48
#define QSFP_MIN_PORT 49
#define QSFP_MAX_PORT 54

#define SFP_PLUS_1_8_PRESENT_REG				(0X2)
#define SFP_PLUS_9_16_PRESENT_REG				(0X3)
#define SFP_PLUS_17_24_PRESENT_REG				(0X4)
#define SFP_PLUS_25_32_PRESENT_REG				(0X5)
#define SFP_PLUS_33_40_PRESENT_REG				(0X6)
#define SFP_PLUS_41_48_PRESENT_REG				(0X7)

#define SFP_PLUS_1_8_RX_LOS_REG				    (0XE)
#define SFP_PLUS_9_16_RX_LOS_REG				(0XF)
#define SFP_PLUS_17_24_RX_LOS_REG				(0X10)
#define SFP_PLUS_25_32_RX_LOS_REG				(0X11)
#define SFP_PLUS_33_40_RX_LOS_REG				(0X12)
#define SFP_PLUS_41_48_RX_LOS_REG				(0X13)

#define SFP_PLUS_1_8_TX_DISABLE_REG			    (0X8)
#define SFP_PLUS_9_16_TX_DISABLE_REG			(0X9)
#define SFP_PLUS_17_24_TX_DISABLE_REG			(0XA)
#define SFP_PLUS_25_32_TX_DISABLE_REG			(0XB)
#define SFP_PLUS_33_40_TX_DISABLE_REG			(0XC)
#define SFP_PLUS_41_48_TX_DISABLE_REG			(0XD)

#define QSFP_49_54_PRESENT_REG                  (0xC)
#define INVALID_REG                             (0xFF)
#define INVALID_REG_BIT                         (0xFF)


struct portCtrl{
    int portId;
    char  cpldName[32];
    int presentReg;
    int presentRegBit;
    int rxLosReg;
    int rxLosRegBit;
    int txDisableReg;
    int txDisableRegBit;
    
};

#define CPLD_NAME1 "SYSCPLD"
#define CPLD_NAME2 "MASTERCPLD"
#define CPLD_NAME3 "SLAVECPLD"

static struct portCtrl gPortCtrl[] = 
{
 {1, CPLD_NAME3, SFP_PLUS_1_8_PRESENT_REG, 0, SFP_PLUS_1_8_RX_LOS_REG, 0, SFP_PLUS_1_8_TX_DISABLE_REG, 0},
 {2, CPLD_NAME3, SFP_PLUS_1_8_PRESENT_REG, 1, SFP_PLUS_1_8_RX_LOS_REG, 1, SFP_PLUS_1_8_TX_DISABLE_REG, 1},
 {3, CPLD_NAME3, SFP_PLUS_1_8_PRESENT_REG, 2, SFP_PLUS_1_8_RX_LOS_REG, 1, SFP_PLUS_1_8_TX_DISABLE_REG, 2},
 {4, CPLD_NAME3, SFP_PLUS_1_8_PRESENT_REG, 3, SFP_PLUS_1_8_RX_LOS_REG, 2, SFP_PLUS_1_8_TX_DISABLE_REG, 3},
 {5, CPLD_NAME3, SFP_PLUS_1_8_PRESENT_REG, 4, SFP_PLUS_1_8_RX_LOS_REG, 3, SFP_PLUS_1_8_TX_DISABLE_REG, 4},
 {6, CPLD_NAME3, SFP_PLUS_1_8_PRESENT_REG, 5, SFP_PLUS_1_8_RX_LOS_REG, 4, SFP_PLUS_1_8_TX_DISABLE_REG, 5},
 {7, CPLD_NAME3, SFP_PLUS_1_8_PRESENT_REG, 6, SFP_PLUS_1_8_RX_LOS_REG, 5, SFP_PLUS_1_8_TX_DISABLE_REG, 6},
 {8, CPLD_NAME3, SFP_PLUS_1_8_PRESENT_REG, 7, SFP_PLUS_1_8_RX_LOS_REG, 6, SFP_PLUS_1_8_TX_DISABLE_REG, 7},

 {9, CPLD_NAME3, SFP_PLUS_9_16_PRESENT_REG, 0, SFP_PLUS_9_16_RX_LOS_REG, 0, SFP_PLUS_9_16_TX_DISABLE_REG, 0},
 {10, CPLD_NAME3, SFP_PLUS_9_16_PRESENT_REG, 1, SFP_PLUS_9_16_RX_LOS_REG, 1, SFP_PLUS_9_16_TX_DISABLE_REG, 1},
 {11, CPLD_NAME3, SFP_PLUS_9_16_PRESENT_REG, 2, SFP_PLUS_9_16_RX_LOS_REG, 1, SFP_PLUS_9_16_TX_DISABLE_REG, 2},
 {12, CPLD_NAME3, SFP_PLUS_9_16_PRESENT_REG, 3, SFP_PLUS_9_16_RX_LOS_REG, 2, SFP_PLUS_9_16_TX_DISABLE_REG, 3},
 {13, CPLD_NAME3, SFP_PLUS_9_16_PRESENT_REG, 4, SFP_PLUS_9_16_RX_LOS_REG, 3, SFP_PLUS_9_16_TX_DISABLE_REG, 4},
 {14, CPLD_NAME3, SFP_PLUS_9_16_PRESENT_REG, 5, SFP_PLUS_9_16_RX_LOS_REG, 4, SFP_PLUS_9_16_TX_DISABLE_REG, 5},
 {15, CPLD_NAME3, SFP_PLUS_9_16_PRESENT_REG, 6, SFP_PLUS_9_16_RX_LOS_REG, 5, SFP_PLUS_9_16_TX_DISABLE_REG, 6},
 {16, CPLD_NAME3, SFP_PLUS_9_16_PRESENT_REG, 7, SFP_PLUS_9_16_RX_LOS_REG, 6, SFP_PLUS_9_16_TX_DISABLE_REG, 7},

 {17, CPLD_NAME3, SFP_PLUS_17_24_PRESENT_REG, 0, SFP_PLUS_17_24_RX_LOS_REG, 0, SFP_PLUS_17_24_TX_DISABLE_REG, 0},
 {18, CPLD_NAME3, SFP_PLUS_17_24_PRESENT_REG, 1, SFP_PLUS_17_24_RX_LOS_REG, 1, SFP_PLUS_17_24_TX_DISABLE_REG, 1},
 {19, CPLD_NAME3, SFP_PLUS_17_24_PRESENT_REG, 2, SFP_PLUS_17_24_RX_LOS_REG, 1, SFP_PLUS_17_24_TX_DISABLE_REG, 2},
 {20, CPLD_NAME3, SFP_PLUS_17_24_PRESENT_REG, 3, SFP_PLUS_17_24_RX_LOS_REG, 2, SFP_PLUS_17_24_TX_DISABLE_REG, 3},
 {21, CPLD_NAME3, SFP_PLUS_17_24_PRESENT_REG, 4, SFP_PLUS_17_24_RX_LOS_REG, 3, SFP_PLUS_17_24_TX_DISABLE_REG, 4},
 {22, CPLD_NAME3, SFP_PLUS_17_24_PRESENT_REG, 5, SFP_PLUS_17_24_RX_LOS_REG, 4, SFP_PLUS_17_24_TX_DISABLE_REG, 5},
 {23, CPLD_NAME3, SFP_PLUS_17_24_PRESENT_REG, 6, SFP_PLUS_17_24_RX_LOS_REG, 5, SFP_PLUS_17_24_TX_DISABLE_REG, 6},
 {24, CPLD_NAME3, SFP_PLUS_17_24_PRESENT_REG, 7, SFP_PLUS_17_24_RX_LOS_REG, 6, SFP_PLUS_17_24_TX_DISABLE_REG, 7},

 {25, CPLD_NAME3, SFP_PLUS_25_32_PRESENT_REG, 0, SFP_PLUS_25_32_RX_LOS_REG, 0, SFP_PLUS_25_32_TX_DISABLE_REG, 0},
 {26, CPLD_NAME3, SFP_PLUS_25_32_PRESENT_REG, 1, SFP_PLUS_25_32_RX_LOS_REG, 1, SFP_PLUS_25_32_TX_DISABLE_REG, 1},
 {27, CPLD_NAME3, SFP_PLUS_25_32_PRESENT_REG, 2, SFP_PLUS_25_32_RX_LOS_REG, 1, SFP_PLUS_25_32_TX_DISABLE_REG, 2},
 {28, CPLD_NAME3, SFP_PLUS_25_32_PRESENT_REG, 3, SFP_PLUS_25_32_RX_LOS_REG, 2, SFP_PLUS_25_32_TX_DISABLE_REG, 3},
 {29, CPLD_NAME3, SFP_PLUS_25_32_PRESENT_REG, 4, SFP_PLUS_25_32_RX_LOS_REG, 3, SFP_PLUS_25_32_TX_DISABLE_REG, 4},
 {30, CPLD_NAME3, SFP_PLUS_25_32_PRESENT_REG, 5, SFP_PLUS_25_32_RX_LOS_REG, 4, SFP_PLUS_25_32_TX_DISABLE_REG, 5},
 {31, CPLD_NAME3, SFP_PLUS_25_32_PRESENT_REG, 6, SFP_PLUS_25_32_RX_LOS_REG, 5, SFP_PLUS_25_32_TX_DISABLE_REG, 6},
 {32, CPLD_NAME3, SFP_PLUS_25_32_PRESENT_REG, 7, SFP_PLUS_25_32_RX_LOS_REG, 6, SFP_PLUS_25_32_TX_DISABLE_REG, 7},

 {33, CPLD_NAME3, SFP_PLUS_33_40_PRESENT_REG, 0, SFP_PLUS_33_40_RX_LOS_REG, 0, SFP_PLUS_33_40_TX_DISABLE_REG, 0},
 {34, CPLD_NAME3, SFP_PLUS_33_40_PRESENT_REG, 1, SFP_PLUS_33_40_RX_LOS_REG, 1, SFP_PLUS_33_40_TX_DISABLE_REG, 1},
 {35, CPLD_NAME3, SFP_PLUS_33_40_PRESENT_REG, 2, SFP_PLUS_33_40_RX_LOS_REG, 1, SFP_PLUS_33_40_TX_DISABLE_REG, 2},
 {36, CPLD_NAME3, SFP_PLUS_33_40_PRESENT_REG, 3, SFP_PLUS_33_40_RX_LOS_REG, 2, SFP_PLUS_33_40_TX_DISABLE_REG, 3},
 {37, CPLD_NAME3, SFP_PLUS_33_40_PRESENT_REG, 4, SFP_PLUS_33_40_RX_LOS_REG, 3, SFP_PLUS_33_40_TX_DISABLE_REG, 4},
 {38, CPLD_NAME3, SFP_PLUS_33_40_PRESENT_REG, 5, SFP_PLUS_33_40_RX_LOS_REG, 4, SFP_PLUS_33_40_TX_DISABLE_REG, 5},
 {39, CPLD_NAME3, SFP_PLUS_33_40_PRESENT_REG, 6, SFP_PLUS_33_40_RX_LOS_REG, 5, SFP_PLUS_33_40_TX_DISABLE_REG, 6},
 {40, CPLD_NAME3, SFP_PLUS_33_40_PRESENT_REG, 7, SFP_PLUS_33_40_RX_LOS_REG, 6, SFP_PLUS_33_40_TX_DISABLE_REG, 7},

 {41, CPLD_NAME3, SFP_PLUS_41_48_PRESENT_REG, 0, SFP_PLUS_41_48_RX_LOS_REG, 0, SFP_PLUS_41_48_TX_DISABLE_REG, 0},
 {42, CPLD_NAME3, SFP_PLUS_41_48_PRESENT_REG, 1, SFP_PLUS_41_48_RX_LOS_REG, 1, SFP_PLUS_41_48_TX_DISABLE_REG, 1},
 {43, CPLD_NAME3, SFP_PLUS_41_48_PRESENT_REG, 2, SFP_PLUS_41_48_RX_LOS_REG, 1, SFP_PLUS_41_48_TX_DISABLE_REG, 2},
 {44, CPLD_NAME3, SFP_PLUS_41_48_PRESENT_REG, 3, SFP_PLUS_41_48_RX_LOS_REG, 2, SFP_PLUS_41_48_TX_DISABLE_REG, 3},
 {45, CPLD_NAME3, SFP_PLUS_41_48_PRESENT_REG, 4, SFP_PLUS_41_48_RX_LOS_REG, 3, SFP_PLUS_41_48_TX_DISABLE_REG, 4},
 {46, CPLD_NAME3, SFP_PLUS_41_48_PRESENT_REG, 5, SFP_PLUS_41_48_RX_LOS_REG, 4, SFP_PLUS_41_48_TX_DISABLE_REG, 5},
 {47, CPLD_NAME3, SFP_PLUS_41_48_PRESENT_REG, 6, SFP_PLUS_41_48_RX_LOS_REG, 5, SFP_PLUS_41_48_TX_DISABLE_REG, 6},
 {48, CPLD_NAME3, SFP_PLUS_41_48_PRESENT_REG, 7, SFP_PLUS_41_48_RX_LOS_REG, 6, SFP_PLUS_41_48_TX_DISABLE_REG, 7},

 {49, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 0, INVALID_REG, 0, INVALID_REG_BIT, 0},
 {50, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 1, INVALID_REG, 1, INVALID_REG_BIT, 1},
 {51, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 2, INVALID_REG, 1, INVALID_REG_BIT, 2},
 {52, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 3, INVALID_REG, 2, INVALID_REG_BIT, 3},
 {53, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 4, INVALID_REG, 3, INVALID_REG_BIT, 4},
 {54, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 5, INVALID_REG, 4, INVALID_REG_BIT, 5},

 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG_BIT, 0},


};

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

    if(platform_id == PLATFORM_ID_DELTA_AG7648_R0)
    {
        start_port = SFP_PLUS_MIN_PORT;
        end_port   = QSFP_MAX_PORT;
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
	
	if((port >= SFP_PLUS_MIN_PORT) && (port <= QSFP_MAX_PORT)){
		r_data=i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].presentReg);
    }
	else{
		AIM_LOG_ERROR("The port %d is invalid \r\n", port);
		return ONLP_STATUS_E_UNSUPPORTED;
	}
		
	if(r_data<0){
		AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
		return ONLP_STATUS_E_INTERNAL;
	}
    r_data = (~r_data) & 0xFF;

	present = (r_data >> gPortCtrl[port - 1].presentRegBit) & 0x1;

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int status;
	int port, i = 0;
	uint64_t presence_all=0;

    AIM_BITMAP_CLR_ALL(dst);
	
	 if(platform_id == PLATFORM_ID_DELTA_AG7648_R0)
	 {		
		port = 1;
		
	}
	else{
        AIM_LOG_ERROR("The platform id %d is invalid \r\n", platform_id);
		return ONLP_STATUS_E_UNSUPPORTED;
    }
 
    /*read 8 ports present status once*/
    for (i = port; i <= QSFP_MAX_PORT;)
    {
        /*
        AIM_LOG_ERROR("port %d, cpldname %s, reg %d\r\n", i, gPortCtrl[i - 1].cpldName, \
               gPortCtrl[i - 1].presentReg);
        */
        status = i2c_devname_read_byte(gPortCtrl[i - 1].cpldName, gPortCtrl[i - 1].presentReg);

        if(status<0){
            AIM_LOG_ERROR("Unable to read presence from the port %d to %d value(status %d) \r\n", i, i + 8, status);
            return ONLP_STATUS_E_INTERNAL;
        }
        status = ~(status) & 0xFF;
        presence_all |= ((uint64_t)(status)) << (((i - 1)/ 8) * 8);

        i += 8;
    }

    /* Populate bitmap */
    for(i = port; presence_all; i++) {
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
	uint64_t rx_los_all;		
	
 
	if(platform_id == PLATFORM_ID_DELTA_AG7648_R0)
	{		
		port = 1;
		
	}
	else{
        AIM_LOG_ERROR("The platform id %d is invalid \r\n", platform_id);
		return ONLP_STATUS_E_UNSUPPORTED;
    }

    /*read 8 ports present status once*/
    for (i = port; i <= QSFP_MAX_PORT;)
    {
        status = i2c_devname_read_byte(gPortCtrl[i - 1].cpldName, gPortCtrl[i - 1].rxLosReg);

        if(status<0){
            AIM_LOG_ERROR("Unable to read rx los from the port %d to %d value. \r\n", i, i + 8);
            return ONLP_STATUS_E_INTERNAL;
        }
        status = ~(status) & 0xFF;
        rx_los_all |= status << (((i - 1)/ 8) * 8);

        i += 8;
    }

    /* Populate bitmap */
    for(i = port; rx_los_all; i++) {
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
	 
	int i;//,r_data,re_cnt;
	char sfp_name[32];
   
	//int i,re_cnt;uint8_t r_data;
    memset(data, 0, 256);
    memset(sfp_name, 0x0, sizeof(sfp_name));

	if (port < SFP_PLUS_MIN_PORT || port > QSFP_MAX_PORT)
    {
        AIM_LOG_ERROR("port %d is not invalid\r\n", port);
        return ONLP_STATUS_E_INVALID;
    }
    if (onlp_sfpi_is_present(port) <= 0)
    {
        AIM_LOG_WARN("port %d is note present or error\r\n", port);
        return ONLP_STATUS_E_MISSING;
    }

    if (port <= SFP_PLUS_MAX_PORT)
        sprintf(sfp_name, "SFP%d", port);
    else
        sprintf(sfp_name, "QSFP%d", port);
    for(i=0;i<8;i++){
        if (i2c_devname_read_block(sfp_name, (32*i), (uint8_t*)(data+32*i), 32) < 0)
        {
            AIM_LOG_ERROR("Unable to read the port %d eeprom\r\n", port);
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
	
	if (port < SFP_PLUS_MIN_PORT || port > QSFP_MAX_PORT)
    {
        AIM_LOG_ERROR("port %d is not invalid\r\n", port);
        return ONLP_STATUS_E_INVALID;
    }
	present=onlp_sfpi_is_present(port);
	
	if(present <= 0){
		AIM_LOG_INFO("The port %d is not present and can not set tx disable\r\n",port);
		return ONLP_STATUS_E_UNSUPPORTED;
	}
	r_data = i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].txDisableReg);	

	if(r_data<0){
		AIM_LOG_INFO("Unable to read sfp tx disable reg value\r\n");
        return ONLP_STATUS_E_INTERNAL;
	}
	
    r_data &= ~(0x1 << gPortCtrl[port - 1].txDisableReg);
    dis_value = value << gPortCtrl[port - 1].txDisableReg;
    dis_value |= r_data;

		
    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                rc = i2c_devname_write_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].txDisableReg, dis_value);

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
	
	if (port < SFP_PLUS_MIN_PORT || port > QSFP_MAX_PORT)
    {
        AIM_LOG_ERROR("port %d is not invalid\r\n", port);
        return ONLP_STATUS_E_INVALID;
    }

	present=onlp_sfpi_is_present(port);
	
	if(present <= 0){
		AIM_LOG_INFO("The port %d is not present\r\n",port);
		return ONLP_STATUS_E_UNSUPPORTED;
	}

    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            {
                r_data=i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].rxLosReg);
		
				if (r_data<0) {
                    AIM_LOG_ERROR("Unable to read rx_los status from port(%d)\r\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data &= (0x1 << gPortCtrl[port - 1].rxLosRegBit);
                *value = (r_data >> gPortCtrl[port - 1].rxLosRegBit);
                break;
            }

        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                r_data=i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].txDisableReg);

                if (r_data<0) {
                    AIM_LOG_ERROR("Unable to read tx_disabled status from port(%d)\r\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data &= (0x1 << gPortCtrl[port - 1].txDisableRegBit);
                *value = (r_data >>  gPortCtrl[port - 1].txDisableRegBit);
                break;
            }

        default:
            return ONLP_STATUS_E_UNSUPPORTED;
        }
		
	
    return ONLP_STATUS_OK;
}


int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
