/************************************************************
 * sfpi.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#include <onlp/platformi/base.h>
#include <onlp/platformi/sfpi.h>
#include <fcntl.h> /* For O_RDWR && open */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <onlplib/i2c.h>
#include <dirent.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#define NUM_OF_SFP_PORT		(CHASSIS_SFP_COUNT)
#define NUM_OF_QSFP_PORT	(CHASSIS_QSFP_COUNT)
#define NUM_OF_ALL_PORT		(NUM_OF_SFP_PORT+NUM_OF_QSFP_PORT)

enum onlp_sfp_port_type {
    ONLP_PORT_TYPE_SFP = 0,
    ONLP_PORT_TYPE_QSFP,
    ONLP_PORT_TYPE_MAX
};

static const int sfp_mux_index[NUM_OF_ALL_PORT] = {
    22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35, 36, 37,
    6,  7,  8,  9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20, 21
};

#define FRONT_PORT_TO_MUX_INDEX(port) (sfp_mux_index[port])

static int
onlp_sfpi_port_type(onlp_oid_id_t  port) //port, start from 0
{
#if 0
    if(port >= 0  && port < NUM_OF_SFP_PORT) {
        return ONLP_PORT_TYPE_SFP;
    } else if(port >=NUM_OF_SFP_PORT && port < NUM_OF_ALL_PORT) {
        return ONLP_PORT_TYPE_QSFP;
    }
#else
    if(port >= 0  && port < NUM_OF_ALL_PORT) {
        return ONLP_PORT_TYPE_QSFP;
    }
#endif

    AIM_LOG_ERROR("Invalid port(%d)\r\n", port);
    return ONLP_STATUS_E_PARAM;
}

int
onlp_sfpi_port_map(onlp_oid_id_t  port, int* rport)
{
    int p_type = onlp_sfpi_port_type(port);

    if (p_type == ONLP_STATUS_E_PARAM) {
        return ONLP_STATUS_E_INVALID;
    }
    *rport = port ;
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    /*
     * Ports {0, 53}
     */
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 0; p < NUM_OF_ALL_PORT; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(onlp_oid_id_t port) // port start from 0 , this is ONL bug
{
    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */
    int p_type = onlp_sfpi_port_type(port);
    if(p_type < 0) {
        return ONLP_STATUS_E_INVALID;
    }

    int present;
    int rv;
    if(onlp_file_read_int(&present, INV_SWPS_PREFIX"port%d/present", port ) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }
    if(present == 0) {
        rv = true;
    } else if (present == 1) {
        rv = false;
    } else {
        AIM_LOG_ERROR("Unvalid present status %d from port(%d)\r\n",present, port + 1 );
        return ONLP_STATUS_E_INTERNAL;
    }
    return rv;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    AIM_BITMAP_CLR_ALL(dst);
    onlp_oid_id_t port;
    for(port = 0; port < NUM_OF_ALL_PORT; port++) {
        if(onlp_sfpi_is_present(port) == true) {
            AIM_BITMAP_MOD(dst, port, 1);
        } else if(onlp_sfpi_is_present(port) == false) {
            AIM_BITMAP_MOD(dst, port, 0);
        } else {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    return ONLP_STATUS_OK;
}

static int
onlp_sfpi_is_rx_los(onlp_oid_id_t port) // port start from 0 , this is ONL bug
{
    int rxlos;
    int rv;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX];
    onlp_oid_id_t id = port + 1;

    int p_type = onlp_sfpi_port_type(port);

    if(p_type == ONLP_PORT_TYPE_SFP) {
        if(onlp_file_read_int(&rxlos, INV_SWPS_PREFIX"port%d/rxlos", port) != ONLP_STATUS_OK) {
            return ONLP_STATUS_E_INTERNAL;
        }
        if(rxlos == 0) {
            rv = true;
        } else {
            rv = false;
        }
    } else if(p_type == ONLP_PORT_TYPE_QSFP) {
        if(onlp_file_read((uint8_t*)buf, ONLP_CONFIG_INFO_STR_MAX, &len, INV_SWPS_PREFIX"port%d/soft_rx_los", port) != ONLP_STATUS_OK) {
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
    for(port = 0; port < NUM_OF_ALL_PORT; port++) {
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
    return FRONT_PORT_TO_MUX_INDEX(port);
}

int
onlp_sfpi_dev_read(onlp_oid_id_t id, int devaddr, int addr,
                   uint8_t* dst, int len) // ONL  bug. id should be port ,port start from 0
{
    int bus = onlp_sfpi_port2chan(id);
    /* Can this be block_read? */
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
onlp_sfpi_dev_readw(onlp_oid_id_t id, int devaddr, int addr) //ONL  bug. id should be port ,port start from 0
{
    int bus = onlp_sfpi_port2chan(id);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(onlp_oid_id_t id, int devaddr, int addr, uint16_t value) // ONL  bug. id should be port ,port start from 0
{
    int bus = onlp_sfpi_port2chan(id);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_control_supported(onlp_oid_id_t port, onlp_sfp_control_t control, int* rv) //ONL bug , port start from 0.
{
    *rv = 0;
    int p_type = onlp_sfpi_port_type(port);
    if(p_type < 0) {
        return ONLP_STATUS_E_INVALID;
    }
    switch (control) {
    case ONLP_SFP_CONTROL_RESET_STATE:
        if(p_type == ONLP_PORT_TYPE_QSFP) {
            *rv = 1;
        }
        break;
    case ONLP_SFP_CONTROL_TX_DISABLE:
        if(p_type == ONLP_PORT_TYPE_SFP) {
            *rv = 1;
        }
        break;
    case ONLP_SFP_CONTROL_LP_MODE:
        if(p_type == ONLP_PORT_TYPE_QSFP) {
            *rv = 1;
        }
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
    if(p_type < 0) {
        return ONLP_STATUS_E_INVALID;
    }
    switch (control) {
    case ONLP_SFP_CONTROL_RESET_STATE:
        if(p_type == ONLP_PORT_TYPE_QSFP) {
            ret = onlp_file_write_int(value, INV_SWPS_PREFIX"port%d/reset", port );
        }
        break;
    case ONLP_SFP_CONTROL_TX_DISABLE:
        if(p_type == ONLP_PORT_TYPE_SFP) {
            ret = onlp_file_write_int(value, INV_SWPS_PREFIX"port%d/tx_disable", port);
        }
        break;
    case ONLP_SFP_CONTROL_LP_MODE:
        if(p_type == ONLP_PORT_TYPE_QSFP) {
            ret = onlp_file_write_int(value, INV_SWPS_PREFIX"port%d/lpmod", port);
        }
        break;
    default:
        break;
    }

    return ret;
}

int
onlp_sfpi_control_get(onlp_oid_id_t port, onlp_sfp_control_t control, int* value) // port start from 0 , oid start from 1
{
    int ret = ONLP_STATUS_E_UNSUPPORTED;
    int p_type = onlp_sfpi_port_type( port );

    if(p_type != ONLP_PORT_TYPE_QSFP) { //There are only QSFP ports in D7032Q28B
        return ONLP_STATUS_E_INVALID;
    }
    if(onlp_sfpi_is_present(port)!=true) {
        *value=0;
        return ONLP_STATUS_OK;
    }
    switch (control) {
    case ONLP_SFP_CONTROL_RESET_STATE:
        /*the value of /port(id)/reset
        0: in reset state; 1:not in reset state*/
        ret = onlp_file_read_int(value, INV_SWPS_PREFIX"port%d/reset", port );
        *value=(!(*value));
        break;
    case ONLP_SFP_CONTROL_LP_MODE:
        ret = onlp_file_read_int(value, INV_SWPS_PREFIX"port%d/lpmod", port);
        break;
    default:
        break;
    }

    return ret;
}
