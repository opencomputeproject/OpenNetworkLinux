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

#include <x86_64_delta_ag7648c/x86_64_delta_ag7648c_config.h>
#include "x86_64_delta_ag7648c_log.h"
#include "x86_64_delta_i2c.h"

#define QSFP_MIN_PORT 49
#define QSFP_MAX_PORT 54

#define QSFP_49_54_PRESENT_REG                  (0xC)
#define QSFP_49_54_LP_MODE_REG                  (0xB)
#define QSFP_49_54_RESET_REG                    (0xD)
#define INVALID_REG                             (0xFF)
#define INVALID_REG_BIT                         (0xFF)


struct portCtrl{
    int portId;
    char  cpldName[32];
    int presentReg;
    int presentRegBit;
	int lpModeReg;
	int lpModeRegBit;
	int resetReg;
	int resetRegBit;

};

#define CPLD_NAME1 "SYSCPLD"
#define CPLD_NAME2 "MASTERCPLD"
#define CPLD_NAME3 "SLAVECPLD"

static struct portCtrl gPortCtrl[] = 
{
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},

 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},

 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},

 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},

 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},

 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
 {0xFFFF, "", INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},

 {49, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 0, QSFP_49_54_LP_MODE_REG, 1, QSFP_49_54_RESET_REG, 0},
 {50, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 1, QSFP_49_54_LP_MODE_REG, 0, QSFP_49_54_RESET_REG, 1},
 {51, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 2, QSFP_49_54_LP_MODE_REG, 3, QSFP_49_54_RESET_REG, 2},
 {52, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 3, QSFP_49_54_LP_MODE_REG, 2, QSFP_49_54_RESET_REG, 3},
 {53, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 4, QSFP_49_54_LP_MODE_REG, 5, QSFP_49_54_RESET_REG, 4},
 {54, CPLD_NAME2, QSFP_49_54_PRESENT_REG, 5, QSFP_49_54_LP_MODE_REG, 4, QSFP_49_54_RESET_REG, 5},

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

    if(platform_id == PLATFORM_ID_DELTA_AG7648C_R0)
    {
        start_port = QSFP_MIN_PORT;
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
	
	if((port >= QSFP_MIN_PORT) && (port <= QSFP_MAX_PORT)){
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
	
	 if(platform_id == PLATFORM_ID_DELTA_AG7648C_R0)
	 {		
		port = QSFP_MIN_PORT;
		
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
        presence_all |= (uint64_t)(status);

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
    return ONLP_STATUS_E_UNSUPPORTED;
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

	if (port < QSFP_MIN_PORT || port > QSFP_MAX_PORT)
    {
        AIM_LOG_ERROR("port %d is not invalid\r\n", port);
        return ONLP_STATUS_E_INVALID;
    }
    if (onlp_sfpi_is_present(port) <= 0)
    {
        AIM_LOG_WARN("port %d is note present or error\r\n", port);
        return ONLP_STATUS_E_MISSING;
    }

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

int onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
	*rv = 0;

	if (port < QSFP_MIN_PORT || port > QSFP_MAX_PORT)
    {
        AIM_LOG_ERROR("port %d is not invalid\r\n", port);
        return ONLP_STATUS_E_INVALID;
    }

	if (control > ONLP_SFP_CONTROL_LAST)
		return ONLP_STATUS_E_UNSUPPORTED;

	switch (control) {
	case ONLP_SFP_CONTROL_LP_MODE:
		*rv = 1;
		break;

	case ONLP_SFP_CONTROL_RESET_STATE:
	case ONLP_SFP_CONTROL_RESET:
		*rv = 1;
		break;
	default:
		break;
	}
	return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
	int r_data,dis_value,rc;
	
	if (port < QSFP_MIN_PORT || port > QSFP_MAX_PORT)
    {
        AIM_LOG_ERROR("port %d is not invalid\r\n", port);
        return ONLP_STATUS_E_INVALID;
    }

	if (control > ONLP_SFP_CONTROL_LAST)
		return ONLP_STATUS_E_UNSUPPORTED;
	
    switch(control)
        {
        case ONLP_SFP_CONTROL_LP_MODE:
		{
			r_data = i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].lpModeReg);
			if(r_data<0){
					AIM_LOG_INFO("Unable to read sfp lp mode reg value\r\n");
					return ONLP_STATUS_E_INTERNAL;
				}

			r_data &= ~(0x1 << gPortCtrl[port - 1].lpModeRegBit);
			dis_value = value << gPortCtrl[port - 1].lpModeRegBit;
			dis_value |= r_data;
			rc = i2c_devname_write_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].lpModeReg, dis_value);

			if (rc<0) {
                    AIM_LOG_ERROR("Unable to set lp_mode status to port(%d)\r\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
            break;
        }

		case ONLP_SFP_CONTROL_RESET:
		{
			r_data = i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].resetReg);
			if(r_data<0){
					AIM_LOG_INFO("Unable to read sfp reset reg value\r\n");
					return ONLP_STATUS_E_INTERNAL;
				}

			r_data &= ~(0x1 << gPortCtrl[port - 1].resetRegBit);
			dis_value = (~value) << gPortCtrl[port - 1].resetRegBit;
			dis_value |= r_data;
			rc = i2c_devname_write_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].resetReg, dis_value);

			if (rc<0) {
                    AIM_LOG_ERROR("Unable to reset port(%d)\r\n", port);
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
	int r_data;
	
	if (port < QSFP_MIN_PORT || port > QSFP_MAX_PORT)
    {
        AIM_LOG_ERROR("port %d is not invalid\r\n", port);
        return ONLP_STATUS_E_INVALID;
    }
	if (control > ONLP_SFP_CONTROL_LAST)
		return ONLP_STATUS_E_UNSUPPORTED;

    switch(control)
        {
        case ONLP_SFP_CONTROL_LP_MODE:
            {
                r_data=i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].lpModeReg);

				if (r_data<0) {
                    AIM_LOG_ERROR("Unable to read lp_mode status from port(%d)\r\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data &= (0x1 << gPortCtrl[port - 1].lpModeRegBit);
                *value = (r_data >> gPortCtrl[port - 1].lpModeRegBit);
                break;
            }

        case ONLP_SFP_CONTROL_RESET_STATE:
        case ONLP_SFP_CONTROL_RESET:
            {
                r_data=i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].resetReg);

                if (r_data<0) {
                    AIM_LOG_ERROR("Unable to read reset status from port(%d)\r\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
                r_data = (~r_data) & 0xFF;
                *value = (r_data >> gPortCtrl[port - 1].presentRegBit) & 0x1;
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
