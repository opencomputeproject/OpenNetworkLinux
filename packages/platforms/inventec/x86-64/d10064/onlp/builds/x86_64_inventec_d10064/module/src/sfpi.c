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


#define MUX_START_INDEX 17
#define QSFP_DEV_ADDR 0x50
#define NUM_OF_QSFP_PORT 64
#define TOTAL_PORT_NUM NUM_OF_QSFP_PORT

/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
enum onlp_sfp_port_type {
    ONLP_PORT_TYPE_QSFP=0,
    ONLP_PORT_TYPE_MAX
};

static int
onlp_sfpi_port_type(onlp_oid_id_t  port)
{
    if( (port>=0)&&(port<=TOTAL_PORT_NUM) ) { //There are only QSFP ports in Lavender
        return ONLP_PORT_TYPE_QSFP;
    } else {
        AIM_LOG_ERROR("Port out of range (%d)\r\n", port);
        return ONLP_STATUS_E_PARAM;
    }
}


int onlp_sfpi_port_map(onlp_oid_id_t  port, int* rport)
{
    int type=onlp_sfpi_port_type(port);
    if(type!=ONLP_PORT_TYPE_QSFP) { //There are only QSFP ports in Lavender
        return ONLP_STATUS_E_INVALID;
    }
    *rport=port;
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    AIM_BITMAP_CLR_ALL(bmap);
    int i=0;
    for(i = 0; i < TOTAL_PORT_NUM; i++) {
        AIM_BITMAP_SET(bmap, i);
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(onlp_oid_id_t port)
{
    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */
    int type=onlp_sfpi_port_type(port);
    if(type!=ONLP_PORT_TYPE_QSFP) { //There are only QSFP ports in Lavender
        return ONLP_STATUS_E_INVALID;
    }
    int present_data;
    if(onlp_file_read_int(&present_data, INV_SFP_PREFIX"port%d/present", port ) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }
    switch(present_data) {
    case 1:
    case 0:
        return !present_data;
    default:
        AIM_LOG_ERROR("Unvalid present status %d from port(%d)\r\n",present_data, port + 1 );
        return ONLP_STATUS_E_INTERNAL;
    }
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    AIM_BITMAP_CLR_ALL(dst);
    onlp_oid_id_t port;
    for(port = 0; port < TOTAL_PORT_NUM; port++) {
        int present=onlp_sfpi_is_present(port);
        switch(present) {
        case true:
            AIM_BITMAP_MOD(dst, port, 1);
            break;
        case false:
            AIM_BITMAP_MOD(dst, port, 0);
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    return ONLP_STATUS_OK;
}

static int
onlp_sfpi_is_rx_los(onlp_oid_id_t port)
{
    int rxlos;
    int rv;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX];
    onlp_oid_id_t id = port + 1;

    int p_type = onlp_sfpi_port_type(port);

    if(p_type == ONLP_PORT_TYPE_QSFP) { //There are only QSFP ports in Lavender
        if(onlp_file_read((uint8_t*)buf, ONLP_CONFIG_INFO_STR_MAX, &len, INV_SFP_PREFIX"port%d/soft_rx_los", port) != ONLP_STATUS_OK) {
            return ONLP_STATUS_E_INTERNAL;
        }
        if(sscanf( buf, "0x%x\n", &rxlos) != 1) {
            AIM_LOG_ERROR("Unable to read rxlos from port(%d)\r\n", id);
            return ONLP_STATUS_E_INTERNAL;
        }
        if(rxlos < 0 || rxlos > 0x0f) {
            AIM_LOG_ERROR("Unable to read rxlos from port(%d)\r\n", id);
            return ONLP_STATUS_E_INTERNAL;
        } else if(rxlos == 0) {
            rv = false;
        } else {
            rv = true;
        }
    } else {
        return ONLP_STATUS_E_INVALID;
    }

    return rv;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    AIM_BITMAP_CLR_ALL(dst);
    onlp_oid_id_t port;// port start from 0 , this is ONL bug
    int isrxlos;
    for(port = 0; port < TOTAL_PORT_NUM; port++) {
        if(onlp_sfpi_is_present(port) == true) {
            isrxlos = onlp_sfpi_is_rx_los(port);
            if(isrxlos == true) {
                AIM_BITMAP_MOD(dst, port, 1);
            } else if(isrxlos == false) {
                AIM_BITMAP_MOD(dst, port, 0);
            } else {
                return ONLP_STATUS_E_INTERNAL;
            }
        }
    }
    return ONLP_STATUS_OK;
}


int
onlp_sfpi_port2chan(int port)  // port start from 0
{
    return port +MUX_START_INDEX;
}

int
onlp_sfpi_dev_read(onlp_oid_id_t id, int devaddr, int addr,
                   uint8_t* dst, int len) // ONL  bug. id should be port ,port start from 0
{
    int bus = onlp_sfpi_port2chan(id);
    return onlp_i2c_read(bus, devaddr, addr, len, dst, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readb(onlp_oid_id_t id, int devaddr, int addr) //ONL  bug. id should be port ,port start from 0
{
    int bus = onlp_sfpi_port2chan(id);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(onlp_oid_id_t id, int devaddr, int addr, uint8_t value) //ONL  bug. id should be port ,port start from 0
{
    int bus = onlp_sfpi_port2chan(id);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(onlp_oid_id_t id, int devaddr, int addr)
{
    int bus = onlp_sfpi_port2chan(id);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(onlp_oid_id_t id, int devaddr, int addr, uint16_t value)
{
    int bus = onlp_sfpi_port2chan(id);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_control_supported(onlp_oid_id_t port, onlp_sfp_control_t control, int* rv)
{
    *rv = 0;
    int p_type = onlp_sfpi_port_type(port);
    if(p_type !=ONLP_PORT_TYPE_QSFP ) {  //There are only QSFP ports in Lavender
        return ONLP_STATUS_E_INVALID;
    }
    switch (control) {
    case ONLP_SFP_CONTROL_RESET_STATE:
    case ONLP_SFP_CONTROL_LP_MODE:
        *rv = 1;
        break;
    default:
        break;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_set(onlp_oid_id_t port, onlp_sfp_control_t control, int value) //ONL bug . port start from 0.
{
    int ret = ONLP_STATUS_E_UNSUPPORTED;
    int p_type = onlp_sfpi_port_type(port);
    if(p_type != ONLP_PORT_TYPE_QSFP ) { //There are only QSFP ports in Lavender
        return ONLP_STATUS_E_INVALID;
    }
    switch (control) {
    case ONLP_SFP_CONTROL_RESET_STATE:
        ret = onlp_file_write_int(value, INV_SFP_PREFIX"port%d/reset", port );
        break;
    case ONLP_SFP_CONTROL_LP_MODE:
        ret = onlp_file_write_int(value, INV_SFP_PREFIX"port%d/lpmod", port);
        break;
    default:
        break;
    }
    return ret;
}

int
onlp_sfpi_control_get(onlp_oid_id_t port, onlp_sfp_control_t control, int* value)
{
    int ret = ONLP_STATUS_E_UNSUPPORTED;
    int p_type = onlp_sfpi_port_type( port );

    if(p_type != ONLP_PORT_TYPE_QSFP) { //There are only QSFP ports in Lavender
        return ONLP_STATUS_E_INVALID;
    }
    if(onlp_sfpi_is_present(port)!=true){
        *value=0;
        return ONLP_STATUS_OK;
    }
    switch (control) {
    case ONLP_SFP_CONTROL_RESET_STATE:
        /*the value of /port(id)/reset
        0: in reset state; 1:not in reset state*/
        ret = onlp_file_read_int(value, INV_SFP_PREFIX"port%d/reset", port );
        *value=(!(*value));
        break;
    case ONLP_SFP_CONTROL_LP_MODE:
        ret = onlp_file_read_int(value, INV_SFP_PREFIX"port%d/lpmod", port);
        break;
    default:
        break;
    }

    return ret;
}
