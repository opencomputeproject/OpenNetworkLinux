/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
#include <onlp/sfp.h>
#include <onlp/platformi/sfpi.h>
#include "onlp_log.h"
#include "onlp_locks.h"

/**
 * All port numbers will be validated before calling the SFP driver.
 */
static onlp_sfp_bitmap_t sfpi_bitmap__;

void
onlp_sfp_bitmap_t_init(onlp_sfp_bitmap_t* bmap)
{
    AIM_BITMAP_INIT(bmap, 255);
    AIM_BITMAP_CLR_ALL(bmap);
}

static int
onlp_sfp_init_locked__(void)
{
    onlp_sfp_bitmap_t_init(&sfpi_bitmap__);

    int rv = onlp_sfpi_init();
    if(rv < 0) {
        if(rv == ONLP_STATUS_E_UNSUPPORTED) {
            /*
             * There are no SFPs on this platform.
             * Not necessarily an error condition.
             */
        }
        else {
            AIM_LOG_ERROR("Error initializing the SFPI driver: %{onlp_status}", rv);
        }
        return rv;
    }
    else {
        /* SFPI initialized. Get the bitmap */
        rv = onlp_sfpi_bitmap_get(&sfpi_bitmap__);
        if(rv < 0) {
            AIM_LOG_ERROR("onlp_sfpi_bitmap_get(): %{onlp_status}", rv);
            return rv;
        }
        return ONLP_STATUS_OK;
    }
}
ONLP_LOCKED_API0(onlp_sfp_init)



static int
onlp_sfp_bitmap_get_locked__(onlp_sfp_bitmap_t* bmap)
{
    AIM_BITMAP_ASSIGN(bmap, &sfpi_bitmap__);
    return ONLP_STATUS_OK;
}
ONLP_LOCKED_API1(onlp_sfp_bitmap_get, onlp_sfp_bitmap_t*, bmap);


static int
onlp_sfp_denit_locked__(void)
{
    return onlp_sfpi_denit();
}
ONLP_LOCKED_API0(onlp_sfp_denit);


#define ONLP_SFP_PORT_VALIDATE_AND_MAP(_port)            \
    do {                                                 \
        if(AIM_BITMAP_GET(&sfpi_bitmap__, _port) == 0) { \
            return -1;                                   \
        }                                                \
        int _rport;                                      \
        if(onlp_sfpi_port_map(_port, &_rport) >= 0) {  \
            _port = _rport;                              \
        }                                                \
    } while(0)

static int
onlp_sfp_is_present_locked__(int port)
{
    ONLP_SFP_PORT_VALIDATE_AND_MAP(port);
    return onlp_sfpi_is_present(port);
}
ONLP_LOCKED_API1(onlp_sfp_is_present, int, port);

static int
onlp_sfp_presence_bitmap_get_locked__(onlp_sfp_bitmap_t* dst)
{
    onlp_sfp_bitmap_t_init(dst);
    int rv = onlp_sfpi_presence_bitmap_get(dst);

    if(rv == ONLP_STATUS_E_UNSUPPORTED) {
        /* Generate from single-port API */
        int p;
        AIM_BITMAP_CLR_ALL(dst);
        AIM_BITMAP_ITER(&sfpi_bitmap__, p) {
            rv = onlp_sfp_is_present_locked__(p);
            if(rv < 0) {
                return rv;
            }
            if(rv > 0) {
                AIM_BITMAP_SET(dst, p);
            }
        }
        return 0;
    }

    return rv;
}
ONLP_LOCKED_API1(onlp_sfp_presence_bitmap_get, onlp_sfp_bitmap_t*, dst);

int
onlp_sfp_port_valid(int port)
{
    return AIM_BITMAP_GET(&sfpi_bitmap__, port);
}

static int
onlp_sfp_eeprom_read_locked__(int port, uint8_t** datap)
{
    int rv;
    uint8_t* data;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(port);

    data = aim_zmalloc(256);
    if((rv = onlp_sfpi_eeprom_read(port, data)) < 0) {
        aim_free(data);
        data = NULL;
    }
    *datap = data;
    return rv;
}
ONLP_LOCKED_API2(onlp_sfp_eeprom_read, int, port, uint8_t**, rv);

static int
onlp_sfp_dom_read_locked__(int port, uint8_t** datap)
{
    int rv;
    uint8_t* data;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(port);

    data = aim_zmalloc(256);
    if((rv = onlp_sfpi_dom_read(port, data)) < 0) {
        aim_free(data);
        data = NULL;
    }
    *datap = data;
    return rv;
}
ONLP_LOCKED_API2(onlp_sfp_dom_read, int, port, uint8_t**, rv);

void
onlp_sfp_dump(aim_pvs_t* pvs)
{
    int p;
    int rv;

    if(AIM_BITMAP_COUNT(&sfpi_bitmap__) == 0) {
        aim_printf(pvs, "There are no SFP capable ports.\n");
        return;
    }

    onlp_sfp_bitmap_t bmap;
    onlp_sfp_bitmap_t_init(&bmap);
    rv = onlp_sfp_presence_bitmap_get(&bmap);
    aim_printf(pvs, "  Presence Bitmap: ");
    if(rv == 0) {
        aim_printf(pvs, "%{aim_bitmap}\n", &bmap);
    }
    else {
        aim_printf(pvs,"Error: %{onlp_status}\n", rv);
    }
    aim_printf(pvs, "  RX_LOS Bitmap: ");
    rv = onlp_sfp_rx_los_bitmap_get(&bmap);
    if(rv == 0) {
        aim_printf(pvs, "%{aim_bitmap}\n", &bmap);
    }
    else {
        aim_printf(pvs, "Error: %{onlp_status}\n", rv);
    }
    aim_printf(pvs, "\n");

    AIM_BITMAP_ITER(&sfpi_bitmap__, p) {
        rv = onlp_sfp_is_present(p);
        aim_printf(pvs, "Port %.2d: ", p);
        if(rv == 0) {
            /* Missing, OK */
            aim_printf(pvs, "Missing.\n");
        }
        else if(rv == 1) {
            /* Present, OK */
            int srv;
            uint32_t flags = 0;
            srv = onlp_sfp_control_flags_get(p, &flags);
            if(srv >= 0) {
                aim_printf(pvs, "Present, Status = %{onlp_sfp_control_flags}\n", flags);
            }
            else {
                aim_printf(pvs, "Present, Status Unavailable [ %{onlp_status} ]\n", srv);
            }
        }
        else {
            /* Error */
            aim_printf(pvs, "Error: %{onlp_status}\n", rv);
        }
        if(rv == 1) {
            uint8_t* idprom = NULL;
            rv = onlp_sfp_eeprom_read(p, &idprom);
            if(rv < 0) {
                aim_printf(pvs, "Error reading eeprom: %{onlp_status}\n");
            }
            else {
                aim_printf(pvs, "eeprom:\n%{data}\n", idprom, 256);
                aim_free(idprom);
            }
        }
    }
    return;
}

static int
onlp_sfp_post_insert_locked__(int port, sff_info_t* info)
{
    ONLP_SFP_PORT_VALIDATE_AND_MAP(port);
    return onlp_sfpi_post_insert(port, info);
}
ONLP_LOCKED_API2(onlp_sfp_post_insert, int, port, sff_info_t*, info);

static int
onlp_sfp_control_set_locked__(int port, onlp_sfp_control_t control, int value)
{
    int supported;

    ONLP_SFP_PORT_VALIDATE_AND_MAP(port);

    if(!ONLP_SFP_CONTROL_VALID(control)) {
        return ONLP_STATUS_E_PARAM;
    }

    /* Does the platform advertise support for this control? */
    if( (onlp_sfpi_control_supported(port, control, &supported) >= 0) &&
        !supported) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
        {

        case ONLP_SFP_CONTROL_RX_LOS:
        case ONLP_SFP_CONTROL_TX_FAULT:
            /** These are read-only. */
            return ONLP_STATUS_E_PARAM;

        default:
            break;
        }
    return onlp_sfpi_control_set(port, control, value);
}
ONLP_LOCKED_API3(onlp_sfp_control_set, int, port, onlp_sfp_control_t, control,
                 int, value);

static int
onlp_sfp_control_get_locked__(int port, onlp_sfp_control_t control, int* value)
{
    ONLP_SFP_PORT_VALIDATE_AND_MAP(port);

    if(!ONLP_SFP_CONTROL_VALID(control)) {
        return ONLP_STATUS_E_PARAM;
    }

    /* Does the platform advertise support for this control? */
    int supported;
    if( (onlp_sfpi_control_supported(port, control, &supported) >= 0) &&
        !supported) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
        {
        case ONLP_SFP_CONTROL_RESET:
            /* This is a write-only control. */
            return ONLP_STATUS_E_UNSUPPORTED;

        default:
            break;
        }

    return (value) ? onlp_sfpi_control_get(port, control, value) : ONLP_STATUS_E_PARAM;
}
ONLP_LOCKED_API3(onlp_sfp_control_get, int, port, onlp_sfp_control_t, control,
                 int*, value);



static int
onlp_sfp_rx_los_bitmap_get_locked__(onlp_sfp_bitmap_t* dst)
{
    int rv = onlp_sfpi_rx_los_bitmap_get(dst);

    if(rv == ONLP_STATUS_E_UNSUPPORTED) {
        /* Generate from control API */
        int p;
        AIM_BITMAP_CLR_ALL(dst);
        AIM_BITMAP_ITER(&sfpi_bitmap__, p) {
            int v;
            rv = onlp_sfp_control_get_locked__(p, ONLP_SFP_CONTROL_RX_LOS, &v);
            if(rv < 0) {
                return rv;
            }
            if(v) {
                AIM_BITMAP_SET(dst, p);
            }
        }
    }

    return rv;
}
ONLP_LOCKED_API1(onlp_sfp_rx_los_bitmap_get, onlp_sfp_bitmap_t*, dst);


int
onlp_sfp_control_flags_get(int port, uint32_t* flags)
{
    /**
     * These are the control bits queried and returned.
     */
    onlp_sfp_control_t controls[] =
        {
            ONLP_SFP_CONTROL_RESET_STATE,
            ONLP_SFP_CONTROL_RX_LOS,
            ONLP_SFP_CONTROL_TX_DISABLE,
            ONLP_SFP_CONTROL_LP_MODE
        };

    if(flags) {
        *flags = 0;
    }
    else {
        return ONLP_STATUS_E_PARAM;
    }

    int rv, i, v;

    for(i = 0; i < AIM_ARRAYSIZE(controls); i++) {
        rv = onlp_sfp_control_get(port, controls[i], &v);
        if(rv >= 0) {
            if(v) {
                *flags |= (1 << controls[i]);
            }
        }
        else {
            if(rv != ONLP_STATUS_E_UNSUPPORTED) {
                return rv;
            }
        }
    }
    return 0;
}

int
onlp_sfp_ioctl(int port, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, port);
    rv = onlp_sfp_vioctl(port, vargs);
    va_end(vargs);
    return rv;
}

int
onlp_sfp_vioctl_locked__(int port, va_list vargs)
{
    return onlp_sfpi_ioctl(port, vargs);
};
ONLP_LOCKED_API2(onlp_sfp_vioctl, int, port, va_list, vargs);


int
onlp_sfp_dev_readb_locked__(int port, uint8_t devaddr, uint8_t addr)
{
    ONLP_SFP_PORT_VALIDATE_AND_MAP(port);
    return onlp_sfpi_dev_readb(port, devaddr, addr);
}
ONLP_LOCKED_API3(onlp_sfp_dev_readb, int, port, uint8_t, devaddr, uint8_t, addr);

int
onlp_sfp_dev_writeb_locked__(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    ONLP_SFP_PORT_VALIDATE_AND_MAP(port);
    return onlp_sfpi_dev_writeb(port, devaddr, addr, value);
}
ONLP_LOCKED_API4(onlp_sfp_dev_writeb, int, port, uint8_t, devaddr, uint8_t, addr, uint8_t, value);

int
onlp_sfp_dev_readw_locked__(int port, uint8_t devaddr, uint8_t addr)
{
    ONLP_SFP_PORT_VALIDATE_AND_MAP(port);
    return onlp_sfpi_dev_readw(port, devaddr, addr);
}
ONLP_LOCKED_API3(onlp_sfp_dev_readw, int, port, uint8_t, devaddr, uint8_t, addr);

int
onlp_sfp_dev_writew_locked__(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    ONLP_SFP_PORT_VALIDATE_AND_MAP(port);
    return onlp_sfpi_dev_writew(port, devaddr, addr, value);
}
ONLP_LOCKED_API4(onlp_sfp_dev_writew, int, port, uint8_t, devaddr, uint8_t, addr, uint16_t, value);
