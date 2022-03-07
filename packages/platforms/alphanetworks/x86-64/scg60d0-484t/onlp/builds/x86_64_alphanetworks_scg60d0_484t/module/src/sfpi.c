/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 Accton Technology Corporation.
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
#include <onlp/platformi/sfpi.h>
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include <onlplib/gpio.h>
#include <x86_64_alphanetworks_scg60d0_484t/x86_64_alphanetworks_scg60d0_484t_gpio_table.h>
#include "platform_lib.h"

/**
 * This table maps the presence gpio, tx_disable gpio, tx_fault gpio, lp_mode gpio, and reset gpio
 * for each SFP port.
 */
typedef struct sfpmap_s {
    int port;
    int tx_dis_gpio;
    int tx_fault_gpio;
    int present_gpio;
    int rx_los_gpio;
} sfpmap_t;

static sfpmap_t sfpmap__[] =
{
    { SFP0_PORT_INDEX, SCG60D0_484T_PCA9539_GPIO_SFP_1_TX_DIS_N, SCG60D0_484T_PCA9539_GPIO_SFP_1_TX_FAULT_N, SCG60D0_484T_PCA9539_GPIO_SFP_1_PRSNT_N, SCG60D0_484T_PCA9539_GPIO_SFP_1_RXLOS_N},
    { SFP1_PORT_INDEX, SCG60D0_484T_PCA9539_GPIO_SFP_2_TX_DIS_N, SCG60D0_484T_PCA9539_GPIO_SFP_2_TX_FAULT_N, SCG60D0_484T_PCA9539_GPIO_SFP_2_PRSNT_N, SCG60D0_484T_PCA9539_GPIO_SFP_2_RXLOS_N},
    { SFP2_PORT_INDEX, SCG60D0_484T_PCA9539_GPIO_SFP_3_TX_DIS_N, SCG60D0_484T_PCA9539_GPIO_SFP_3_TX_FAULT_N, SCG60D0_484T_PCA9539_GPIO_SFP_3_PRSNT_N, SCG60D0_484T_PCA9539_GPIO_SFP_3_RXLOS_N},
    { SFP3_PORT_INDEX, SCG60D0_484T_PCA9539_GPIO_SFP_4_TX_DIS_N, SCG60D0_484T_PCA9539_GPIO_SFP_4_TX_FAULT_N, SCG60D0_484T_PCA9539_GPIO_SFP_4_PRSNT_N, SCG60D0_484T_PCA9539_GPIO_SFP_4_RXLOS_N},
};

#define SFP_GET(_port) (sfpmap__ + (_port - SFP_START_INDEX))

static const int port_bus_index[NUM_OF_SFP_PORT] = 
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    9, 10, 11, 12
};

#define PORT_BUS_INDEX(port)                (port_bus_index[port])
/* SFP */
#define MODULE_EEPROM_SFP_FORMAT            "/sys/bus/i2c/devices/%d-0050/eeprom"
#define MODULE_EEPROM_DOM_SFP_FORMAT        "/sys/bus/i2c/devices/%d-0051/eeprom"


/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
int
onlp_sfpi_init(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    /* Called at initialization time */
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = SFP_START_INDEX; p < NUM_OF_SFP_PORT; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    DIAG_PRINT("%s", __FUNCTION__);
    
    return ONLP_STATUS_OK;
}

int onlp_sfpi_is_present(int port)
{
    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */
    int present;

    if (IS_SFP_PORT(port)) 
    {
        sfpmap_t* sfp = SFP_GET(port);
        if(sfp->present_gpio > 0) 
        {

            if (onlp_gpio_get(sfp->present_gpio, &present) < 0) 
            {
                AIM_LOG_ERROR("Unable to read sfp present status from port(%d)\r\n", port);
                return ONLP_STATUS_E_INTERNAL;
            }

            DIAG_PRINT("%s, SFP Present value:0x%x, ", __FUNCTION__, present);

            present = !present;
            DIAG_PRINT("%s, SFP Port:%d Present value:0x%x, ", __FUNCTION__, port, present);
        }
        else
        {
            return ONLP_STATUS_E_INVALID;
        }
    }
    else
    {
        return ONLP_STATUS_E_INVALID;
    }
    
    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    /* auto generate from sfp.c */
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    /* auto generate from sfp.c */
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
    int size = 0;
    if(port < 0 || port >= NUM_OF_SFP_PORT)
        return ONLP_STATUS_E_INTERNAL;

    DIAG_PRINT("%s, port:%d, busid:%d", __FUNCTION__, port, PORT_BUS_INDEX(port));

    if (IS_SFP_PORT(port)) 
    {
        if(onlp_file_read(data, 256, &size, MODULE_EEPROM_SFP_FORMAT, 
                PORT_BUS_INDEX(port)) != ONLP_STATUS_OK) 
        {
            AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else
    {
        return ONLP_STATUS_E_INVALID;
    }

    if(size != 256) 
    {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    FILE* fp;
    char file[64] = {0};

    DIAG_PRINT("%s, port:%d, busid:%d", __FUNCTION__, port, PORT_BUS_INDEX(port));

    if (IS_SFP_PORT(port))
    {
        sprintf(file, MODULE_EEPROM_SFP_FORMAT, PORT_BUS_INDEX(port));
        fp = fopen(file, "r");
        if(fp == NULL) {
            AIM_LOG_ERROR("Unable to open the eeprom device file of port(%d)", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        if (fseek(fp, 256, SEEK_CUR) != 0) {
            fclose(fp);
            AIM_LOG_ERROR("Unable to set the file position indicator of port(%d)", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        int ret = fread(data, 1, 256, fp);
        fclose(fp);
        if (ret != 256) {
            AIM_LOG_ERROR("Unable to read the eeprom device file of port(%d)", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else
    {
        return ONLP_STATUS_E_INVALID;
    }
    
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int ret = 0;
    int bus = PORT_BUS_INDEX(port);

    ret = onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, ret:%d(0x%02X)", __FUNCTION__, port, devaddr, addr, ret, ret);
    return ret; 
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int ret = 0;
    int bus = PORT_BUS_INDEX(port);

    ret = onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, value:%d(0x%02X), ret:%d", __FUNCTION__, port, devaddr, addr, value, value, ret);
    return ret;
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int ret = 0;
    int bus = PORT_BUS_INDEX(port);

    ret = onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, ret:%d(0x%04X)", __FUNCTION__, port, devaddr, addr, ret, ret);
    return ret;
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int ret = 0;
    int bus = PORT_BUS_INDEX(port);

    ret = onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, value:%d(0x%04X), ret:%d", __FUNCTION__, port, devaddr, addr, value, value, ret);
    return ret;
}

int onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int *supported)
{
    if (supported == NULL)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_PARAM);
        return ONLP_STATUS_E_PARAM;
    }

    *supported = 0;
    switch (control)
    {
        case ONLP_SFP_CONTROL_RESET:
        case ONLP_SFP_CONTROL_LP_MODE:
            *supported = 0;
            break;
        case ONLP_SFP_CONTROL_RX_LOS:
        case ONLP_SFP_CONTROL_TX_FAULT:
        case ONLP_SFP_CONTROL_TX_DISABLE:
        {
            if(IS_SFP_PORT(port))
            {
                *supported = 1;
            }
            else
            {
                *supported = 0;
            }
        }
            break;

        default:
            *supported = 0;
            break;
    }

    DIAG_PRINT("%s, port:%d, control:%d(%s), supported:%d", __FUNCTION__, port, control, sfp_control_to_str(control), *supported);
    return ONLP_STATUS_OK;
}


int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;  
    int supported = 0;

    if ((onlp_sfpi_control_supported(port, control, &supported) == ONLP_STATUS_OK) && 
        (supported == 0))
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_UNSUPPORTED);
        return ONLP_STATUS_E_UNSUPPORTED;
    }
    
    switch(control)
    {
        case ONLP_SFP_CONTROL_TX_DISABLE:
        {
            if(IS_SFP_PORT(port)) 
            {
                sfpmap_t* sfp = SFP_GET(port);
                if(sfp->tx_dis_gpio > 0) 
                {
                    if (onlp_gpio_set(sfp->tx_dis_gpio, value) < 0) 
                    {
                        AIM_LOG_ERROR("Unable to set tx_disabled status to port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    else
                    {
                        rv = ONLP_STATUS_OK;
                    }

                    DIAG_PRINT("%s, Write SFP port:%d TXDISABLE value:0x%x, ", __FUNCTION__, port, value);
                }
                else
                {
                    rv = ONLP_STATUS_E_INVALID;
                }
            }
            else 
            {
                rv = ONLP_STATUS_E_UNSUPPORTED;
            }
            
            break;
        }

        default:
            rv = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }

    DIAG_PRINT("%s, port:%d, control:%d(%s), value:0x%x, rv:0x%x", __FUNCTION__, port, control, sfp_control_to_str(control), value, rv);

    if (rv == ONLP_STATUS_E_UNSUPPORTED)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_UNSUPPORTED);
    }

    return rv;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int rv;
    int gpio_value;
    int supported = 0;

    if (value == NULL)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_PARAM);
        return ONLP_STATUS_E_PARAM;
    }

    if ((onlp_sfpi_control_supported(port, control, &supported) == ONLP_STATUS_OK) && 
        (supported == 0))
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_UNSUPPORTED);
        return ONLP_STATUS_E_UNSUPPORTED;
    }
    *value = 0;
    
    /* ONLP_SFP_CONTROL_RX_LOS , ONLP_SFP_CONTROL_TX_FAULT are read-only. */
    switch(control)
    {
        case ONLP_SFP_CONTROL_RX_LOS:
        {
            if(IS_SFP_PORT(port))
            {
                sfpmap_t* sfp = SFP_GET(port);
                if(sfp->rx_los_gpio > 0) 
                {
                    if (onlp_gpio_get(sfp->rx_los_gpio, &gpio_value) < 0) 
                    {
                        AIM_LOG_ERROR("Unable to read sfp rx_los status from port(%d)\r\n", port);
                        return ONLP_STATUS_E_INTERNAL;
                    }
                    else
                    {
                        rv = ONLP_STATUS_OK;
                    }

                    DIAG_PRINT("%s, Read Current SFP port:%d RXLOS GPIO value:0x%x, ", __FUNCTION__, port, gpio_value);
                }
                else
                {
                    return ONLP_STATUS_E_INVALID;
                }

                *value = gpio_value;
            }
            else
            {
                rv = ONLP_STATUS_E_UNSUPPORTED;
            }
            break;
        }
        case ONLP_SFP_CONTROL_TX_FAULT:
        {
            if (IS_SFP_PORT(port)) 
            {
                sfpmap_t* sfp = SFP_GET(port);
                if(sfp->tx_fault_gpio > 0) 
                {
                    if (onlp_gpio_get(sfp->tx_fault_gpio, &gpio_value) < 0) 
                    {
                        AIM_LOG_ERROR("Unable to read sfp tx_fault status from port(%d)\r\n", port);
                        return ONLP_STATUS_E_INTERNAL;
                    }
                    else
                    {
                        rv = ONLP_STATUS_OK;
                    }

                    DIAG_PRINT("%s, Read Current SFP port:%d TXFAULT GPIO value:0x%x, ", __FUNCTION__, port, gpio_value);
                }
                else
                {
                    return ONLP_STATUS_E_INVALID;
                }

                *value = gpio_value;
            }
            else 
            {
                rv = ONLP_STATUS_E_UNSUPPORTED;
            }
            break;
        }
        case ONLP_SFP_CONTROL_TX_DISABLE:
        {
            if (IS_SFP_PORT(port))
            {
                sfpmap_t* sfp = SFP_GET(port);
                if(sfp->tx_dis_gpio > 0) 
                {
                    if (onlp_gpio_get(sfp->tx_dis_gpio, &gpio_value) < 0) 
                    {
                        AIM_LOG_ERROR("Unable to read sfp tx_disable status from port(%d)\r\n", port);
                        return ONLP_STATUS_E_INTERNAL;
                    }
                    else
                    {
                        rv = ONLP_STATUS_OK;
                    }

                    DIAG_PRINT("%s, Read Current SFP port:%d TXDISABLE GPIO value:0x%x, ", __FUNCTION__, port, gpio_value);
                }
                else
                {
                    return ONLP_STATUS_E_INVALID;
                }

                *value = gpio_value;
            }
            else {
                rv = ONLP_STATUS_E_UNSUPPORTED;
            }
            break;
        }

        default:
            rv = ONLP_STATUS_E_UNSUPPORTED;
    }

    DIAG_PRINT("%s, port:%d, control:%d(%s), value:0x%x, rv:0x%x", __FUNCTION__, port, control, sfp_control_to_str(control), *value, rv);

    if (rv == ONLP_STATUS_E_UNSUPPORTED)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_UNSUPPORTED);
    }

    return rv;
}

int
onlp_sfpi_denit(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    return ONLP_STATUS_OK;
}

