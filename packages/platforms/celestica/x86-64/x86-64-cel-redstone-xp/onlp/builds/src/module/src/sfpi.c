#include <onlp/platformi/sfpi.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "cpld.h"
#include "x86_64_cel_redstone_xp_int.h"
#include "sfp_xfp.h"


static int
_get_sfp_state(int sfp)
{
    unsigned char val = 0;
    int reg;
    int ret = -1;

    if (!sfp)
        return sfp;

    if(sfp < 9) {
        reg = 0x259;
        ret = read_cpld(reg, &val);
        if(ret < 0)
            return ret;
        val &= 0x1 << (sfp - 1);
    } else if((sfp > 8) && (sfp < 17)) {
        reg = 0x25A;
        ret = read_cpld(reg, &val);
        if(ret < 0)
            return ret;
        val &= 0x1 << ((sfp - 1) % 8);
    } else if((sfp > 16) && (sfp < 19)) {
        reg = 0x25B;
        ret = read_cpld(reg, &val);
        if(ret < 0)
            return ret;
        val &= 0x1 << ((sfp - 1) % 8);
    } else if((sfp > 18) && (sfp < 29)) {
        reg = 0x2D9;
        ret = read_cpld(reg, &val);
        if(ret < 0)
            return ret;
        val &= 0x1 << ((sfp - 3) % 8);
    } else if((sfp > 28) && (sfp < 35)) {
        reg = 0x2DA;
        ret = read_cpld(reg, &val);
        if(ret < 0)
            return ret;
        val &= 0x1 << ((sfp - 3) % 8);
    } else if((sfp > 34) && (sfp < 37)) {
        reg = 0x2DB;
        ret = read_cpld(reg, &val);
        if(ret < 0)
            return ret;
        val &= 0x1 << ((sfp - 3) % 8);
    } else if((sfp > 36) && (sfp < 45)) {
        reg = 0x3D6;
        ret = read_cpld(reg, &val);
        if(ret < 0)
            return ret;
        val &= 0x1 << ((sfp - 5) % 8);
    } else if((sfp > 44) && (sfp < 49)) {
        reg = 0x3D7;
        ret = read_cpld(reg, &val);
        if(ret < 0)
            return ret;
        val &= 0x1 << ((sfp - 5) % 8);
    } else if((sfp > 48) && (sfp <= CEL_REDSTONE_MAX_PORT)) {
        reg = 0x3D7;
        ret = read_cpld(reg, &val);
        if(ret < 0)
            return ret;
        val &= 0x1 << ((sfp - 1) % 8);
    }
    return (ret = (val ? 0 : 1));
}

/*@ _read_sfp
 * return
 *      0 on success
 *      *data_p with result
 */
static int
_read_sfp(int port, uint8_t *data_p, uint32_t addr, uint32_t offset, uint32_t size)
{
    char byte = 0;

    read_sfp(port, 0xA0, 0x7F, &byte, 1);

    if (byte) {
        byte = 0;
        write_sfp(port, 0xA0, 0x7F, &byte, 1);
        usleep(40000);
    }
    return (read_sfp(port, (addr<<1), offset, (char *)data_p, size));
}

/* @_spf_rx_los
 *      return
 *          1 if rx_loss
 *          else 0
 *      Arg
 *          port
 */
static int
_sfp_rx_los(int port)
{
    unsigned char byte = 0;

    if (!_get_sfp_state(port))
        return 1;
        //return ONLP_STATUS_E_MISSING;

    if (port <= 48) {
        _read_sfp(port, &byte, ALL_SFP_DIAG_I2C_ADDRESS, SFP_XFP_LOS_ADDR, SFP_XFP_LOS_SIZE);
    } else if (port <= 54) {
        _read_sfp(port, &byte, ALL_SFP_DIAG_I2C_ADDRESS, QSFP_LOS_ADDR, SFP_XFP_LOS_SIZE);
    }

    if (SFP_LOS_MASK == (byte & SFP_LOS_MASK))
        return 1;

    return 0;
}

/* @_spf_tx_fault
 *      return
 *           if tx fault
 *          else 0
 *      Arg
 *          port
 */
static int
_sfp_tx_fault(int port)
{
    unsigned int option = 0;

    if (!_get_sfp_state(port))
        return -1;

    if (port <= 48) {
        _read_sfp(port, (uint8_t *)&option, ALL_SFP_DIAG_I2C_ADDRESS, SFP_OPTIONS_ADDR, SFP_OPTIONS_SIZE);
    } else if (port <= 54) {
        _read_sfp(port, (uint8_t *)&option, ALL_SFP_DIAG_I2C_ADDRESS, QSFP_TX_FAULT_ADDR, SFP_OPTIONS_SIZE);
    }

    if (SFP_TX_FAULT_MASK & option)
        return 1;

    return 0;
}


int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;

    for(p = 1; p <= CEL_REDSTONE_MAX_PORT; p++)
        AIM_BITMAP_SET(bmap, p);
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_init(void)
{
    return ONLP_STATUS_OK;
}

/*
* @onlp_sfpi_is_present
*    Return
*       1 if present.
*       0 if not present.
*       < 0 if error.
*/
int
onlp_sfpi_is_present(int port)
{
    return (_get_sfp_state(port));
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int rt;
    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            rt = _sfp_rx_los(port);
            if (rt < 0)
                return ONLP_STATUS_E_INTERNAL;

            *value = rt;
            return ONLP_STATUS_OK;

        case ONLP_SFP_CONTROL_TX_FAULT:
            rt = _sfp_tx_fault(port);
            if (rt < 0)
                return ONLP_STATUS_E_INTERNAL;

            *value = rt;
            return ONLP_STATUS_OK;

        default:
            return ONLP_STATUS_E_UNSUPPORTED;

        }
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    switch(control) {
      case ONLP_SFP_CONTROL_TX_DISABLE:
        //return _tx_enable_set__(port, !value);
        return ONLP_STATUS_E_UNSUPPORTED;
      default:
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}

int
onlp_sfpi_port_map(int port, int* rport)
{
    /* rport need to be port+1 as the face plate starts at 1 */

    *rport = port;
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    if (port > CEL_REDSTONE_MAX_PORT) {
        return ONLP_STATUS_E_MISSING;
    }

    if (!_get_sfp_state(port))
        return ONLP_STATUS_E_MISSING;

    memset(data, 0, 256);
    if (_read_sfp(port, data, ALL_SFP_I2C_ADDRESS, 0, 256) == -1)
        return ONLP_STATUS_OK;

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t i;
    int rt;

    AIM_BITMAP_CLR_ALL(dst);

    for(i=0; i< CEL_REDSTONE_MAX_PORT; i++) {
        rt = _get_sfp_state(i);
        if (rt < 0 ) {
            return ONLP_STATUS_E_INTERNAL;
        } else if (rt) {
            AIM_BITMAP_SET(dst, i);
        }
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t i;
    int rt;

    AIM_BITMAP_CLR_ALL(dst);
    for(i=1; i<= CEL_REDSTONE_MAX_PORT; i++) {
        rt = _sfp_rx_los(i);
        if (rt < 0 ) {
            return ONLP_STATUS_E_INTERNAL;
        } else if (rt) {
            AIM_BITMAP_SET(dst, i);
        }
    }
    return ONLP_STATUS_OK;
}
