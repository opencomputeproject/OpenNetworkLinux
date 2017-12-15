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
#include <x86_64_delta_agc5648s/x86_64_delta_agc5648s_config.h>
#include "x86_64_delta_agc5648s_log.h"

#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlplib/mmap.h>
#include <onlplib/i2c.h>

#include "platform_lib.h"

#define reverseBits(b)  b = ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;

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
    {1, SWPLD_2_ADDR, SFP_1_TO_8_PRESENT_REG, 7, SFP_1_8_RX_LOS_REG, 7, SFP_1_8_TX_DISABLE_REG, 0},
    {2, SWPLD_2_ADDR, SFP_1_TO_8_PRESENT_REG, 6, SFP_1_8_RX_LOS_REG, 6, SFP_1_8_TX_DISABLE_REG, 1},
    {3, SWPLD_2_ADDR, SFP_1_TO_8_PRESENT_REG, 5, SFP_1_8_RX_LOS_REG, 5, SFP_1_8_TX_DISABLE_REG, 2},
    {4, SWPLD_2_ADDR, SFP_1_TO_8_PRESENT_REG, 4, SFP_1_8_RX_LOS_REG, 4, SFP_1_8_TX_DISABLE_REG, 3},
    {5, SWPLD_2_ADDR, SFP_1_TO_8_PRESENT_REG, 3, SFP_1_8_RX_LOS_REG, 3, SFP_1_8_TX_DISABLE_REG, 4},
    {6, SWPLD_2_ADDR, SFP_1_TO_8_PRESENT_REG, 2, SFP_1_8_RX_LOS_REG, 2, SFP_1_8_TX_DISABLE_REG, 5},
    {7, SWPLD_2_ADDR, SFP_1_TO_8_PRESENT_REG, 1, SFP_1_8_RX_LOS_REG, 1, SFP_1_8_TX_DISABLE_REG, 6},
    {8, SWPLD_2_ADDR, SFP_1_TO_8_PRESENT_REG, 0, SFP_1_8_RX_LOS_REG, 0, SFP_1_8_TX_DISABLE_REG, 7},
    
    {9,  SWPLD_2_ADDR, SFP_9_TO_16_PRESENT_REG, 7, SFP_9_16_RX_LOS_REG, 7, SFP_9_16_TX_DISABLE_REG, 0},
    {10, SWPLD_2_ADDR, SFP_9_TO_16_PRESENT_REG, 6, SFP_9_16_RX_LOS_REG, 6, SFP_9_16_TX_DISABLE_REG, 1},
    {11, SWPLD_2_ADDR, SFP_9_TO_16_PRESENT_REG, 5, SFP_9_16_RX_LOS_REG, 5, SFP_9_16_TX_DISABLE_REG, 2},
    {12, SWPLD_2_ADDR, SFP_9_TO_16_PRESENT_REG, 4, SFP_9_16_RX_LOS_REG, 4, SFP_9_16_TX_DISABLE_REG, 3},
    {13, SWPLD_2_ADDR, SFP_9_TO_16_PRESENT_REG, 3, SFP_9_16_RX_LOS_REG, 3, SFP_9_16_TX_DISABLE_REG, 4},
    {14, SWPLD_2_ADDR, SFP_9_TO_16_PRESENT_REG, 2, SFP_9_16_RX_LOS_REG, 2, SFP_9_16_TX_DISABLE_REG, 5},
    {15, SWPLD_2_ADDR, SFP_9_TO_16_PRESENT_REG, 1, SFP_9_16_RX_LOS_REG, 1, SFP_9_16_TX_DISABLE_REG, 6},
    {16, SWPLD_2_ADDR, SFP_9_TO_16_PRESENT_REG, 0, SFP_9_16_RX_LOS_REG, 0, SFP_9_16_TX_DISABLE_REG, 7},
    
    {17, SWPLD_2_ADDR, SFP_17_TO_24_PRESENT_REG, 7, SFP_17_24_RX_LOS_REG, 7, SFP_17_24_TX_DISABLE_REG, 0},
    {18, SWPLD_2_ADDR, SFP_17_TO_24_PRESENT_REG, 6, SFP_17_24_RX_LOS_REG, 6, SFP_17_24_TX_DISABLE_REG, 1},
    {19, SWPLD_2_ADDR, SFP_17_TO_24_PRESENT_REG, 5, SFP_17_24_RX_LOS_REG, 5, SFP_17_24_TX_DISABLE_REG, 2},
    {20, SWPLD_2_ADDR, SFP_17_TO_24_PRESENT_REG, 4, SFP_17_24_RX_LOS_REG, 4, SFP_17_24_TX_DISABLE_REG, 3},
    {21, SWPLD_2_ADDR, SFP_17_TO_24_PRESENT_REG, 3, SFP_17_24_RX_LOS_REG, 3, SFP_17_24_TX_DISABLE_REG, 4},
    {22, SWPLD_2_ADDR, SFP_17_TO_24_PRESENT_REG, 2, SFP_17_24_RX_LOS_REG, 2, SFP_17_24_TX_DISABLE_REG, 5},
    {23, SWPLD_2_ADDR, SFP_17_TO_24_PRESENT_REG, 1, SFP_17_24_RX_LOS_REG, 1, SFP_17_24_TX_DISABLE_REG, 6},
    {24, SWPLD_2_ADDR, SFP_17_TO_24_PRESENT_REG, 0, SFP_17_24_RX_LOS_REG, 0, SFP_17_24_TX_DISABLE_REG, 7},
    
    {25, SWPLD_2_ADDR, SFP_25_TO_32_PRESENT_REG, 7, SFP_25_32_RX_LOS_REG, 7, SFP_25_32_TX_DISABLE_REG, 0},
    {26, SWPLD_2_ADDR, SFP_25_TO_32_PRESENT_REG, 6, SFP_25_32_RX_LOS_REG, 6, SFP_25_32_TX_DISABLE_REG, 1},
    {27, SWPLD_2_ADDR, SFP_25_TO_32_PRESENT_REG, 5, SFP_25_32_RX_LOS_REG, 5, SFP_25_32_TX_DISABLE_REG, 2},
    {28, SWPLD_2_ADDR, SFP_25_TO_32_PRESENT_REG, 4, SFP_25_32_RX_LOS_REG, 4, SFP_25_32_TX_DISABLE_REG, 3},
    {29, SWPLD_2_ADDR, SFP_25_TO_32_PRESENT_REG, 3, SFP_25_32_RX_LOS_REG, 3, SFP_25_32_TX_DISABLE_REG, 4},
    {30, SWPLD_2_ADDR, SFP_25_TO_32_PRESENT_REG, 2, SFP_25_32_RX_LOS_REG, 2, SFP_25_32_TX_DISABLE_REG, 5},
    {31, SWPLD_2_ADDR, SFP_25_TO_32_PRESENT_REG, 1, SFP_25_32_RX_LOS_REG, 1, SFP_25_32_TX_DISABLE_REG, 6},
    {32, SWPLD_2_ADDR, SFP_25_TO_32_PRESENT_REG, 0, SFP_25_32_RX_LOS_REG, 0, SFP_25_32_TX_DISABLE_REG, 7},
    
    {33, SWPLD_2_ADDR, SFP_33_TO_36_PRESENT_REG, 7, SFP_33_36_RX_LOS_REG, 7, SFP_33_36_TX_DISABLE_REG, 0},
    {34, SWPLD_2_ADDR, SFP_33_TO_36_PRESENT_REG, 6, SFP_33_36_RX_LOS_REG, 6, SFP_33_36_TX_DISABLE_REG, 1},
    {35, SWPLD_2_ADDR, SFP_33_TO_36_PRESENT_REG, 5, SFP_33_36_RX_LOS_REG, 5, SFP_33_36_TX_DISABLE_REG, 2},
    {36, SWPLD_2_ADDR, SFP_33_TO_36_PRESENT_REG, 4, SFP_33_36_RX_LOS_REG, 4, SFP_33_36_TX_DISABLE_REG, 3},
    
    {37, SWPLD_3_ADDR, SFP_37_TO_44_PRESENT_REG, 7, SFP_37_44_RX_LOS_REG, 7, SFP_37_44_TX_DISABLE_REG, 0},
    {38, SWPLD_3_ADDR, SFP_37_TO_44_PRESENT_REG, 6, SFP_37_44_RX_LOS_REG, 6, SFP_37_44_TX_DISABLE_REG, 1},
    {39, SWPLD_3_ADDR, SFP_37_TO_44_PRESENT_REG, 5, SFP_37_44_RX_LOS_REG, 5, SFP_37_44_TX_DISABLE_REG, 2},
    {40, SWPLD_3_ADDR, SFP_37_TO_44_PRESENT_REG, 4, SFP_37_44_RX_LOS_REG, 4, SFP_37_44_TX_DISABLE_REG, 3},
    {41, SWPLD_3_ADDR, SFP_37_TO_44_PRESENT_REG, 3, SFP_37_44_RX_LOS_REG, 3, SFP_37_44_TX_DISABLE_REG, 4},
    {42, SWPLD_3_ADDR, SFP_37_TO_44_PRESENT_REG, 2, SFP_37_44_RX_LOS_REG, 2, SFP_37_44_TX_DISABLE_REG, 5},
    {43, SWPLD_3_ADDR, SFP_37_TO_44_PRESENT_REG, 1, SFP_37_44_RX_LOS_REG, 1, SFP_37_44_TX_DISABLE_REG, 6},
    {44, SWPLD_3_ADDR, SFP_37_TO_44_PRESENT_REG, 0, SFP_37_44_RX_LOS_REG, 0, SFP_37_44_TX_DISABLE_REG, 7},
    
    {45, SWPLD_3_ADDR, SFP_45_TO_48_PRESENT_REG, 7, SFP_45_48_RX_LOS_REG, 7, SFP_45_48_TX_DISABLE_REG, 0},
    {46, SWPLD_3_ADDR, SFP_45_TO_48_PRESENT_REG, 6, SFP_45_48_RX_LOS_REG, 6, SFP_45_48_TX_DISABLE_REG, 1},
    {47, SWPLD_3_ADDR, SFP_45_TO_48_PRESENT_REG, 5, SFP_45_48_RX_LOS_REG, 5, SFP_45_48_TX_DISABLE_REG, 2},
    {48, SWPLD_3_ADDR, SFP_45_TO_48_PRESENT_REG, 4, SFP_45_48_RX_LOS_REG, 4, SFP_45_48_TX_DISABLE_REG, 3},
    
    {49, SWPLD_3_ADDR, QSFP_49_TO_54_PRESENT_REG, 7, INVALID_REG, 7, INVALID_REG_BIT, 0},
    {50, SWPLD_3_ADDR, QSFP_49_TO_54_PRESENT_REG, 6, INVALID_REG, 6, INVALID_REG_BIT, 1},
    {51, SWPLD_3_ADDR, QSFP_49_TO_54_PRESENT_REG, 5, INVALID_REG, 5, INVALID_REG_BIT, 2},
    {52, SWPLD_3_ADDR, QSFP_49_TO_54_PRESENT_REG, 4, INVALID_REG, 4, INVALID_REG_BIT, 3},
    {53, SWPLD_3_ADDR, QSFP_49_TO_54_PRESENT_REG, 3, INVALID_REG, 3, INVALID_REG_BIT, 4},
    {54, SWPLD_3_ADDR, QSFP_49_TO_54_PRESENT_REG, 2, INVALID_REG, 2, INVALID_REG_BIT, 5},
    
    {0xFFFF, INVALID_ADDR , INVALID_REG, 0, INVALID_REG, 0, INVALID_REG_BIT, 0},
};

static int port_to_presence_all_bitmap(int portstart, int portend, uint64_t* presence_all)
{
    int i = 0, j =0;
    int  rv = ONLP_STATUS_OK;
    uint8_t present_bit = 0;
    
    for (i = portstart; i <= portend; i += 8)
    {
        rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, gPortCtrl[i].swpldAddr, gPortCtrl[i].presentReg, (uint32_t*)(&present_bit), DATA_LEN);
        
        if (rv != ONLP_STATUS_OK)
        {
            AIM_LOG_ERROR("Unable to read present status from port(%d). error code: %d\r\n", i, rv);
            return ONLP_STATUS_E_INTERNAL;
        }

        present_bit = ~(present_bit) & 0xFF;
        
        if(portend <= 32)
        {
            reverseBits(present_bit);
            *presence_all |= ((uint64_t)(present_bit)) << (((i - 1)/ 8) * 8);
        }
        else
        {
            for ( j = portstart; j <= portend ; j ++)
            {
                *presence_all |= ((uint64_t)((present_bit >> gPortCtrl[j].presentRegBit ) & 0x01) << (j-1));
            }
        }
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

    for(p = SFP_PLUS_MIN_PORT; p <= QSFP_MAX_PORT; p++)
    {
        AIM_BITMAP_SET(bmap, p);
    }
    
    return ONLP_STATUS_OK;
}

int onlp_sfpi_is_present(int port)
{
    int rv = ONLP_STATUS_OK;
    uint32_t present_bit = 0, IsPresent = 0;
    
    if( (port >= SFP_PLUS_MIN_PORT) && (port <= QSFP_MAX_PORT) )
    {
        rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_3, gPortCtrl[port].swpldAddr, gPortCtrl[port].presentReg, &present_bit, DATA_LEN);
        
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
    
    port_to_presence_all_bitmap(1, 32, &presence_all);
    port_to_presence_all_bitmap(33, 36, &presence_all);
    port_to_presence_all_bitmap(37, 44, &presence_all);
    port_to_presence_all_bitmap(45, 48, &presence_all);
    port_to_presence_all_bitmap(49, 54, &presence_all);
        
    /* Populate bitmap */
    for(i = SFP_PLUS_MIN_PORT; presence_all; i++) 
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
    uint32_t i = 0, MuxDevAddr[] = {0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77};
    uint8_t u4Data = 0, u4Addr = 0;
        
    /* Clear Mux */
    for(i = 0; i < (sizeof(MuxDevAddr)/sizeof(uint32_t)); i++)
    {
        if(onlp_i2c_read(I2C_BUS_2, MuxDevAddr[i], 0x0, 1, &u4Data, 0) != 0)
        {
            AIM_LOG_ERROR("ERROR: unable to write mux device (0x%2x)\n",  MuxDevAddr[i]);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    
    /* Set Mux */
    u4Addr = 1 << ((port-1) % 8);
    if(onlp_i2c_read(I2C_BUS_2, MuxDevAddr[(port-1)/8], u4Addr, 1, &u4Data, 0) != 0)
    {
        AIM_LOG_ERROR("ERROR: unable to write mux device (0x%2x)\n",  MuxDevAddr[(port-1)/8]);
        return ONLP_STATUS_E_INTERNAL;
    }
    
    memset(data, 0 ,256);
                
    /* Read eeprom information into data[] */
    if (onlp_i2c_read(I2C_BUS_2, SFP_EEPROM_ADDR, 0, 256, data, 0) != 0)
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

