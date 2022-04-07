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
 ***********************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <onlplib/sfp.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/sfpi.h>
#include <x86_64_ufispace_s9500_22xst/x86_64_ufispace_s9500_22xst_config.h>
#include "x86_64_ufispace_s9500_22xst_log.h"
#include "platform_lib.h"

static int _sfpi_get_bus_by_port(int port)
{
    if (port < (RJ45_NUM)) {
        AIM_LOG_ERROR("Invalid port has no I2C bus, port=%d\n", port);
        return 999; /* invalid bus num */
    } else if (port < (RJ45_NUM + SFP_NUM)) { //SFP
        return port + (9 - RJ45_NUM);
    } else if (port < (RJ45_NUM + SFP_NUM + SFP28_NUM)) { //SFP28
        return port + (21 - (RJ45_NUM + SFP_NUM));
    } else if (port < (PORT_NUM - 1)) { //QSFP
        return 36;
    } else if (port < (PORT_NUM)) { //QSFP
        return 35;
    } else { //unknown ports
        AIM_LOG_ERROR("unknown ports, port=%d\n", port);
        return 999; /* invalid bus num */
    }
}

int
onlp_sfpi_init(void)
{
    lock_init();
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;
    for(p = RJ45_NUM; p < PORT_NUM; p++) {
        AIM_BITMAP_SET(bmap, p);
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    int status = 1;

    // SFP, SFP28, QSFP Ports
    if (port < RJ45_NUM) {
        AIM_LOG_ERROR("Presence is not supported, port=%d\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }else if (port < (RJ45_NUM + SFP_NUM + SFP28_NUM)) { //SFP, SFP28
        if (sfp_present_get(port, &status) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("sfp_presnet_get() failed, port=%d\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    } else if (port >= (RJ45_NUM + SFP_NUM + SFP28_NUM) && port < PORT_NUM) { //QSFP
        if (qsfp_present_get(port, &status) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("qsfp_present_get() failed, port=%d\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    } else { //unkonwn ports
        AIM_LOG_ERROR("unknown ports, port=%d\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    return status;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int p = 0;
    int rc = 0;

    AIM_BITMAP_CLR_ALL(dst);

    /* The first 4 port is RJ45 */
    for (p = RJ45_NUM; p < PORT_NUM; p++) {
        rc = onlp_sfpi_is_present(p);
        AIM_BITMAP_MOD(dst, p, rc);
    }

    return ONLP_STATUS_OK;
}

/*
 * This function reads the SFPs idrom and returns in
 * in the data buffer provided.
 */
int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    char eeprom_path[512];
    int size = 0;

    memset(eeprom_path, 0, sizeof(eeprom_path));    
    memset(data, 0, 256);

    //QSFP, QSFPDD, SFP real ports
    if (port < (RJ45_NUM)) {
        AIM_LOG_ERROR("eeprom is not supported, port=%d\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    } else if (port < (RJ45_NUM + SFP_NUM + SFP28_NUM)) { //SFP
        snprintf(eeprom_path, sizeof(eeprom_path), "/sys/bus/i2c/devices/%d-0050/eeprom", _sfpi_get_bus_by_port(port));
    } else if (port >= (RJ45_NUM + SFP_NUM + SFP28_NUM) && port < PORT_NUM) { //QSFP
        snprintf(eeprom_path, sizeof(eeprom_path), "/sys/bus/i2c/devices/%d-0050/eeprom", _sfpi_get_bus_by_port(port));
    } else { //unknown ports
        AIM_LOG_ERROR("unknown ports, port=%d\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if(onlp_file_read(data, 256, &size, eeprom_path) != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (size != 256) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d), size is different!\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int i=0, value=0, rc=0;
    int sfpx_num = (SFP_NUM + SFP28_NUM);

    AIM_BITMAP_CLR_ALL(dst);

    /* Populate bitmap - SFP+ and SFP28 ports*/
    for(i = RJ45_NUM; i < (RJ45_NUM + sfpx_num); i++) {
        if ((rc=onlp_sfpi_control_get(i, ONLP_SFP_CONTROL_RX_LOS, &value)) != ONLP_STATUS_OK) {
            return ONLP_STATUS_E_INTERNAL;
        } else {
            AIM_BITMAP_MOD(dst, i, value);
        }
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int rc, status;
    int sfpx_num = (SFP_NUM + SFP28_NUM);

    if (port >= RJ45_NUM && port < (RJ45_NUM + sfpx_num)) {
        switch(control) {
            case ONLP_SFP_CONTROL_RX_LOS:
                if (sfp_rx_los_get(port, &status) != ONLP_STATUS_OK) {
                    AIM_LOG_ERROR("sfp_rx_los_get() failed, port=%d\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
                *value = status;
                rc = ONLP_STATUS_OK;
                break;
            case ONLP_SFP_CONTROL_TX_FAULT:
                if (control == ONLP_SFP_CONTROL_TX_FAULT) {
                    if (sfp_tx_fault_get(port, &status) != ONLP_STATUS_OK) {
                        AIM_LOG_ERROR("sfp_tx_fault_get() failed, port=%d\n", port);
                        return ONLP_STATUS_E_INTERNAL;
                    }
                }
                *value = status;
                rc = ONLP_STATUS_OK;
                break;
            case ONLP_SFP_CONTROL_TX_DISABLE:
                if (control == ONLP_SFP_CONTROL_TX_DISABLE) {
                    if (sfp_tx_disable_get(port, &status) != ONLP_STATUS_OK) {
                        AIM_LOG_ERROR("sfp_tx_disable_get() failed, port=%d\n", port);
                        return ONLP_STATUS_E_INTERNAL;
                    }
                }
                *value = status;
                rc = ONLP_STATUS_OK;
                break;
            default:
                rc = ONLP_STATUS_E_UNSUPPORTED;
                break;
        }
    } else if (port >= (RJ45_NUM + sfpx_num) && port < PORT_NUM) {
        switch (control) {
            case ONLP_SFP_CONTROL_LP_MODE:
                if (qsfp_lp_mode_get(port, &status) != ONLP_STATUS_OK) {
                    AIM_LOG_ERROR("qsfp_lp_mode_get() failed, port=%d\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
                *value = status;
                rc = ONLP_STATUS_OK;
                break;
            case ONLP_SFP_CONTROL_RESET:
                if (qsfp_reset_get(port, &status) != ONLP_STATUS_OK) {
                    AIM_LOG_ERROR("qsfp_reset_get() failed, port=%d\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
                *value = status;
                rc = ONLP_STATUS_OK;
                break;
            default:
                rc = ONLP_STATUS_E_UNSUPPORTED;
                break;
        }
    } else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    return rc;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rc;
    int sfpx_num = (SFP_NUM + SFP28_NUM);

    if (port >= RJ45_NUM && port < (RJ45_NUM + sfpx_num)) {
        switch (control) {
            case ONLP_SFP_CONTROL_TX_DISABLE:
                if (sfp_tx_disable_set(port, value) != ONLP_STATUS_OK) {
                    AIM_LOG_ERROR("sfp_tx_disable_set() failed, port=%d\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
                rc = ONLP_STATUS_OK;
                break;
            default:
                return ONLP_STATUS_E_UNSUPPORTED;
        }
    } else if (port >= (RJ45_NUM + sfpx_num) && port < PORT_NUM) {
        switch (control) {
            case ONLP_SFP_CONTROL_LP_MODE:
                if (qsfp_lp_mode_set(port, value) != ONLP_STATUS_OK) {
                    AIM_LOG_ERROR("qsfp_lp_mode_set() failed, port=%d\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
                rc = ONLP_STATUS_OK;
                break;
            case ONLP_SFP_CONTROL_RESET:
                if (qsfp_reset_set(port, value) != ONLP_STATUS_OK) {
                    AIM_LOG_ERROR("qsfp_reset_set() failed, port=%d\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }
                rc = ONLP_STATUS_OK;
                break;
            default:
                rc = ONLP_STATUS_E_UNSUPPORTED;
                break;
        }
    } else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    return rc;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = _sfpi_get_bus_by_port(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int bus = _sfpi_get_bus_by_port(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = _sfpi_get_bus_by_port(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int bus = _sfpi_get_bus_by_port(port);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

/*
 * De-initialize the SFPI subsystem.
 */
int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
