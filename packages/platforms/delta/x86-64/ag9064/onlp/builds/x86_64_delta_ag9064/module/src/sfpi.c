/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2017 Delta Networks, Inc.
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
 ***********************************************************/
#include <onlp/platformi/sfpi.h>
#include <x86_64_delta_ag9064/x86_64_delta_ag9064_config.h>
#include "x86_64_delta_ag9064_log.h"

#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlplib/mmap.h>
#include <onlplib/i2c.h>

#include "platform_lib.h"

struct portCtrl
{
    int portId;
    int swpldAddr;
    int presentReg;
    int presentRegBit;
    int rxLosReg;
    int rxLosRegBit;
    int txDisableReg;
    int txDisableRegBit;
};

static struct portCtrl gPortCtrl[] = 
{
    {},
    {1, SWPLD_1_ADDR, QSFP_1_TO_8_PRESENT_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
    {2, SWPLD_1_ADDR, QSFP_1_TO_8_PRESENT_REG, 1, INVALID_REG, 0, INVALID_REG, 0},
    {3, SWPLD_1_ADDR, QSFP_1_TO_8_PRESENT_REG, 2, INVALID_REG, 0, INVALID_REG, 0},
    {4, SWPLD_1_ADDR, QSFP_1_TO_8_PRESENT_REG, 3, INVALID_REG, 0, INVALID_REG, 0},
    {5, SWPLD_1_ADDR, QSFP_1_TO_8_PRESENT_REG, 4, INVALID_REG, 0, INVALID_REG, 0},
    {6, SWPLD_1_ADDR, QSFP_1_TO_8_PRESENT_REG, 5, INVALID_REG, 0, INVALID_REG, 0},
    {7, SWPLD_1_ADDR, QSFP_1_TO_8_PRESENT_REG, 6, INVALID_REG, 0, INVALID_REG, 0},
    {8, SWPLD_1_ADDR, QSFP_1_TO_8_PRESENT_REG, 7, INVALID_REG, 0, INVALID_REG, 0},

    {9,  SWPLD_2_ADDR, QSFP_9_TO_16_PRESENT_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
    {10, SWPLD_2_ADDR, QSFP_9_TO_16_PRESENT_REG, 1, INVALID_REG, 0, INVALID_REG, 0},
    {11, SWPLD_2_ADDR, QSFP_9_TO_16_PRESENT_REG, 2, INVALID_REG, 0, INVALID_REG, 0},
    {12, SWPLD_2_ADDR, QSFP_9_TO_16_PRESENT_REG, 3, INVALID_REG, 0, INVALID_REG, 0},
    {13, SWPLD_2_ADDR, QSFP_9_TO_16_PRESENT_REG, 4, INVALID_REG, 0, INVALID_REG, 0},
    {14, SWPLD_2_ADDR, QSFP_9_TO_16_PRESENT_REG, 5, INVALID_REG, 0, INVALID_REG, 0},
    {15, SWPLD_2_ADDR, QSFP_9_TO_16_PRESENT_REG, 6, INVALID_REG, 0, INVALID_REG, 0},
    {16, SWPLD_2_ADDR, QSFP_9_TO_16_PRESENT_REG, 7, INVALID_REG, 0, INVALID_REG, 0},

    {17, SWPLD_4_ADDR, QSFP_17_TO_24_PRESENT_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
    {18, SWPLD_4_ADDR, QSFP_17_TO_24_PRESENT_REG, 1, INVALID_REG, 0, INVALID_REG, 0},
    {19, SWPLD_4_ADDR, QSFP_17_TO_24_PRESENT_REG, 2, INVALID_REG, 0, INVALID_REG, 0},
    {20, SWPLD_4_ADDR, QSFP_17_TO_24_PRESENT_REG, 3, INVALID_REG, 0, INVALID_REG, 0},
    {21, SWPLD_4_ADDR, QSFP_17_TO_24_PRESENT_REG, 4, INVALID_REG, 0, INVALID_REG, 0},
    {22, SWPLD_4_ADDR, QSFP_17_TO_24_PRESENT_REG, 5, INVALID_REG, 0, INVALID_REG, 0},
    {23, SWPLD_4_ADDR, QSFP_17_TO_24_PRESENT_REG, 6, INVALID_REG, 0, INVALID_REG, 0},
    {24, SWPLD_4_ADDR, QSFP_17_TO_24_PRESENT_REG, 7, INVALID_REG, 0, INVALID_REG, 0},
    
    {25, SWPLD_3_ADDR, QSFP_25_TO_32_PRESENT_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
    {26, SWPLD_3_ADDR, QSFP_25_TO_32_PRESENT_REG, 1, INVALID_REG, 0, INVALID_REG, 0},
    {27, SWPLD_3_ADDR, QSFP_25_TO_32_PRESENT_REG, 2, INVALID_REG, 0, INVALID_REG, 0},
    {28, SWPLD_3_ADDR, QSFP_25_TO_32_PRESENT_REG, 3, INVALID_REG, 0, INVALID_REG, 0},
    {29, SWPLD_3_ADDR, QSFP_25_TO_32_PRESENT_REG, 4, INVALID_REG, 0, INVALID_REG, 0},
    {30, SWPLD_3_ADDR, QSFP_25_TO_32_PRESENT_REG, 5, INVALID_REG, 0, INVALID_REG, 0},
    {31, SWPLD_3_ADDR, QSFP_25_TO_32_PRESENT_REG, 6, INVALID_REG, 0, INVALID_REG, 0},
    {32, SWPLD_3_ADDR, QSFP_25_TO_32_PRESENT_REG, 7, INVALID_REG, 0, INVALID_REG, 0},
    
    {33, SWPLD_1_ADDR, QSFP_33_TO_40_PRESENT_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
    {34, SWPLD_1_ADDR, QSFP_33_TO_40_PRESENT_REG, 1, INVALID_REG, 0, INVALID_REG, 0},
    {35, SWPLD_1_ADDR, QSFP_33_TO_40_PRESENT_REG, 2, INVALID_REG, 0, INVALID_REG, 0},
    {36, SWPLD_1_ADDR, QSFP_33_TO_40_PRESENT_REG, 3, INVALID_REG, 0, INVALID_REG, 0},
    {37, SWPLD_1_ADDR, QSFP_33_TO_40_PRESENT_REG, 4, INVALID_REG, 0, INVALID_REG, 0},
    {38, SWPLD_1_ADDR, QSFP_33_TO_40_PRESENT_REG, 5, INVALID_REG, 0, INVALID_REG, 0},
    {39, SWPLD_1_ADDR, QSFP_33_TO_40_PRESENT_REG, 6, INVALID_REG, 0, INVALID_REG, 0},
    {40, SWPLD_1_ADDR, QSFP_33_TO_40_PRESENT_REG, 7, INVALID_REG, 0, INVALID_REG, 0},
    
    {41, SWPLD_2_ADDR, QSFP_41_TO_48_PRESENT_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
    {42, SWPLD_2_ADDR, QSFP_41_TO_48_PRESENT_REG, 1, INVALID_REG, 0, INVALID_REG, 0},
    {43, SWPLD_2_ADDR, QSFP_41_TO_48_PRESENT_REG, 2, INVALID_REG, 0, INVALID_REG, 0},
    {44, SWPLD_2_ADDR, QSFP_41_TO_48_PRESENT_REG, 3, INVALID_REG, 0, INVALID_REG, 0},
    {45, SWPLD_2_ADDR, QSFP_41_TO_48_PRESENT_REG, 4, INVALID_REG, 0, INVALID_REG, 0},
    {46, SWPLD_2_ADDR, QSFP_41_TO_48_PRESENT_REG, 5, INVALID_REG, 0, INVALID_REG, 0},
    {47, SWPLD_2_ADDR, QSFP_41_TO_48_PRESENT_REG, 6, INVALID_REG, 0, INVALID_REG, 0},
    {48, SWPLD_2_ADDR, QSFP_41_TO_48_PRESENT_REG, 7, INVALID_REG, 0, INVALID_REG, 0},
    
    {49, SWPLD_4_ADDR, QSFP_49_TO_56_PRESENT_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
    {50, SWPLD_4_ADDR, QSFP_49_TO_56_PRESENT_REG, 1, INVALID_REG, 0, INVALID_REG, 0},
    {51, SWPLD_4_ADDR, QSFP_49_TO_56_PRESENT_REG, 2, INVALID_REG, 0, INVALID_REG, 0},
    {52, SWPLD_4_ADDR, QSFP_49_TO_56_PRESENT_REG, 3, INVALID_REG, 0, INVALID_REG, 0},
    {53, SWPLD_4_ADDR, QSFP_49_TO_56_PRESENT_REG, 4, INVALID_REG, 0, INVALID_REG, 0},
    {54, SWPLD_4_ADDR, QSFP_49_TO_56_PRESENT_REG, 5, INVALID_REG, 0, INVALID_REG, 0},
    {55, SWPLD_4_ADDR, QSFP_49_TO_56_PRESENT_REG, 6, INVALID_REG, 0, INVALID_REG, 0},
    {56, SWPLD_4_ADDR, QSFP_49_TO_56_PRESENT_REG, 7, INVALID_REG, 0, INVALID_REG, 0},
    
    {57, SWPLD_3_ADDR, QSFP_57_TO_64_PRESENT_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
    {58, SWPLD_3_ADDR, QSFP_57_TO_64_PRESENT_REG, 1, INVALID_REG, 0, INVALID_REG, 0},
    {59, SWPLD_3_ADDR, QSFP_57_TO_64_PRESENT_REG, 2, INVALID_REG, 0, INVALID_REG, 0},
    {60, SWPLD_3_ADDR, QSFP_57_TO_64_PRESENT_REG, 3, INVALID_REG, 0, INVALID_REG, 0},
    {61, SWPLD_3_ADDR, QSFP_57_TO_64_PRESENT_REG, 4, INVALID_REG, 0, INVALID_REG, 0},
    {62, SWPLD_3_ADDR, QSFP_57_TO_64_PRESENT_REG, 5, INVALID_REG, 0, INVALID_REG, 0},
    {63, SWPLD_3_ADDR, QSFP_57_TO_64_PRESENT_REG, 6, INVALID_REG, 0, INVALID_REG, 0},
    {64, SWPLD_3_ADDR, QSFP_57_TO_64_PRESENT_REG, 7, INVALID_REG, 0, INVALID_REG, 0},
    
    {0xFFFF, INVALID_ADDR , INVALID_REG, 0, INVALID_REG, 0, INVALID_REG, 0},
};

static int port_to_presence_all_bitmap(int portstart, int portend, uint64_t* presence_all)
{
    int i  = 0;
    int rv = ONLP_STATUS_OK;
    uint32_t present_bit = 0;
    
    for (i = portstart; i <= portend; i += 8)
    {
        rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, gPortCtrl[i].swpldAddr, gPortCtrl[i].presentReg, &present_bit, 1);
        
        if (rv != ONLP_STATUS_OK)
        {
            AIM_LOG_ERROR("Unable to read present status from port(%d). error code: %d\r\n", i, rv);
            return ONLP_STATUS_E_INTERNAL;
        }

        present_bit = ~(present_bit) & 0xFF;
        *presence_all |= ((uint64_t)(present_bit)) << (((i - 1)/ 8) * 8);
    }
    
    return 0;
}

int onlp_sfpi_init(void)
{
    return ONLP_STATUS_OK;
}

int onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;
    
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = QSFP_MIN_PORT; p <= QSFP_MAX_PORT; p++)
    {
        AIM_BITMAP_SET(bmap, p);
    }
    
    return ONLP_STATUS_OK;
}

int onlp_sfpi_is_present(int port)
{
    int rv = ONLP_STATUS_OK;
    uint32_t present_bit = 0, IsPresent = 0;
    
    if( (port >= QSFP_MIN_PORT) && (port <= QSFP_MAX_PORT) )
    {
        rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, gPortCtrl[port].swpldAddr, gPortCtrl[port].presentReg, &present_bit, 1);
        
        if (rv == ONLP_STATUS_OK)
        {
            present_bit = (present_bit >> gPortCtrl[port].presentRegBit ) & 0x01;
    
            /* From sfp_is_present value,
            * return 0 = The module is preset
            * return 1 = The module is NOT present
            */
            if(present_bit == 0) 
            {
                IsPresent = 1;
            } 
            else if (present_bit == 1) 
            {
                IsPresent = 0;
            } 
            else 
            {
                AIM_LOG_ERROR("Error to present status from port(%d)\r\n", port);
                IsPresent = -1;
            }
            
            return IsPresent;
        }
        else
        {
            AIM_LOG_ERROR("Unable to read present status from port(%d). error code: %d\r\n", port, rv);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else
    {
        AIM_LOG_ERROR("The port %d is invalid \r\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}

int onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int i = 0;
    uint64_t presence_all = 0;
        
    AIM_BITMAP_CLR_ALL(dst);
    
    port_to_presence_all_bitmap(1, 64, &presence_all);
        
    /* Populate bitmap */
    for(i = QSFP_MIN_PORT; presence_all; i++) 
    {
        AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    if(onlp_i2c_writeb(I2C_BUS_3, PCA9548_I2C_MUX_ADDR, 0x00, QSFP_CHAN_ON_PCA9548, 0) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("ERROR: unable to set QSFP channel on PCA9548\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    
    if(ifnOS_LINUX_BmcI2CSet(I2C_BMC_BUS_5, SWPLD_2_ADDR, QSFP_PORT_MUX_REG, port, 1) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("ERROR: unable to set QSFP port mux\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    
    memset(data, 0 ,256);
                
    /* Read eeprom information into data[] */
    if (onlp_i2c_read(I2C_BUS_3, QSFP_EEPROM_ADDR, 0x00, 256, data, 0) != 0)
    {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    
    return ONLP_STATUS_OK;
}

int onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    
    return onlp_sfpi_eeprom_read( port, data);
}

int onlp_sfpi_ioctl(int port, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

