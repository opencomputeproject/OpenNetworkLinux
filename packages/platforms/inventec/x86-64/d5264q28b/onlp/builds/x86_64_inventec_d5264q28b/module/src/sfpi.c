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
 *
 *
 ***********************************************************/

#include <onlp/platformi/sfpi.h>
#include <fcntl.h> /* For O_RDWR && open */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <onlplib/i2c.h>
#include "platform_lib.h"
#include <dirent.h>
#include <onlplib/file.h>

#define MAX_SFP_PATH 128

#define MUX_START_INDEX 17
#define QSFP_DEV_ADDR 0x50
#define NUM_OF_QSFP_PORT 64
#define NUM_OF_ALL_PORT (NUM_OF_QSFP_PORT)

#define FRONT_PORT_TO_MUX_INDEX(port) (port+MUX_START_INDEX)
#define VALIDATE_PORT(p) { if ((p < 0) || (p >= NUM_OF_ALL_PORT)) return ONLP_STATUS_E_PARAM; }
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

int onlp_sfpi_port_map(int port, int* rport)
{
    VALIDATE_PORT(port);
    *rport = port;
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    /*
     * Ports {0, 63}
     */
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 0; p < NUM_OF_ALL_PORT; p++) {
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
    VALIDATE_PORT(port);
    int present;
    int rv;
    if(onlp_file_read_int(&present, INV_SFP_PREFIX"port%d/present", port) != ONLP_STATUS_OK){
        return ONLP_STATUS_E_INTERNAL;
    }
    if(present == 0){
        rv = 1;
    } else if (present == 1){
        rv = 0;
    }
    else {
        AIM_LOG_ERROR("Unvalid present status %d from port(%d)\r\n",present,port);
        return ONLP_STATUS_E_INTERNAL;
    }
    return rv;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    AIM_BITMAP_CLR_ALL(dst);
    int port;
    for(port = 0; port < NUM_OF_ALL_PORT; port++){
        if(onlp_sfpi_is_present(port) == 1){
            AIM_BITMAP_MOD(dst, port, 1);
        }else if(onlp_sfpi_is_present(port) == 0){
            AIM_BITMAP_MOD(dst, port, 0);
        }else{
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_rx_los(int port)
{
    /*rx los attribute of QSFP are not supported*/
    return ONLP_STATUS_E_UNSUPPORTED; 
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{   
    /*rx los attribute of QSFP are not supported*/
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_post_insert(int port, sff_info_t* info)
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
    memset(data, 0, 256);
    
    VALIDATE_PORT(port);
    int sts;
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    sts = onlp_i2c_block_read(bus, QSFP_DEV_ADDR, 0, 256, data, ONLP_I2C_F_FORCE);
    if(sts < 0){
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_MISSING;
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    VALIDATE_PORT(port);
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    VALIDATE_PORT(port);
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    VALIDATE_PORT(port);
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    VALIDATE_PORT(port);
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
    *rv = 0;
    if(port >= 0 && port < NUM_OF_QSFP_PORT){
        switch (control) {
            case ONLP_SFP_CONTROL_RESET_STATE:
            case ONLP_SFP_CONTROL_LP_MODE:
                *rv = 1;
                break;
            default:
                break;
        }
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int ret = ONLP_STATUS_E_UNSUPPORTED;
    if(port >= 0 && port < NUM_OF_QSFP_PORT){
        switch (control) {
            case ONLP_SFP_CONTROL_RESET_STATE:
                ret = onlp_file_write_int(value, INV_SFP_PREFIX"port%d/reset", port); 
                break;
            case ONLP_SFP_CONTROL_LP_MODE:
                ret = onlp_file_write_int(value, INV_SFP_PREFIX"port%d/lpmod", port);
                break;
            default:
                break;
        }
    }
    return ret;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int ret = ONLP_STATUS_E_UNSUPPORTED;
    if(port >= 0 && port < NUM_OF_QSFP_PORT){    
        switch (control) {
            case ONLP_SFP_CONTROL_RESET_STATE:
                /*the value of /port(id)/reset
                0: in reset state; 1:not in reset state*/            
                ret = onlp_file_read_int(value, INV_SFP_PREFIX"port%d/reset", port); 
                if(ret==ONLP_STATUS_OK){
                    *value=!(*value);
                }
                break;
            case ONLP_SFP_CONTROL_LP_MODE:
                ret = onlp_file_read_int(value, INV_SFP_PREFIX"port%d/lpmod", port); 
                break;
            default:
                break;
        }
    }
    return ret;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

void
onlp_sfpi_debug(int port, aim_pvs_t* pvs)
{
    aim_printf(pvs, "Debug data for port %d goes here.", port);
}

int
onlp_sfpi_ioctl(int port, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
