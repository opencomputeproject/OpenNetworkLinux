/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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
#include "x86_64_amx3200_int.h"
#include "x86_64_amx3200_log.h"
#include "platform_lib.h"

#define VALIDATE_PORT(_p) \
    do { \
        if ((_p < 0) || (_p > 20)) \
            return ONLP_STATUS_E_UNSUPPORTED; \
    } while(0)

#define VALIDATE_QSFP(_p) \
    do { \
        if ((_p < 3) || (_p > 10 && _p < 13) || (_p > 20)) \
            return ONLP_STATUS_E_UNSUPPORTED; \
    } while(0)

#define VALIDATE_CFP(_p) \
    do { \
        if ((_p <= 0) || (_p >= 3 && _p <= 10) || (_p >= 13)) \
            return ONLP_STATUS_E_UNSUPPORTED; \
    } while(0)

#define MODULE_QSFP_EEPROM_FORMAT  "/sys/bus/i2c/devices/%d-0050/eeprom"
#define MODULE_CFP_EEPROM_FORMAT   "%s/module_eeprom_%d"
#define MODULE_PRESENT_FORMAT      "%s/module_present_%d"
#define MODULE_RESET_FORMAT        "%s/module_reset_%d"
#define MODULE_LPMODE_FORMAT       "%s/module_lpmode_%d"

#define MODULE_ATTRIBUTE_PATH_SLED1 "/sys/devices/platform/amx3200_sled1_sfp.0"
#define MODULE_ATTRIBUTE_PATH_SLED2 "/sys/devices/platform/amx3200_sled2_sfp.1"

#define NUM_OF_CFP_PORT  (4)
#define NUM_OF_QSFP_PORT (16)
#define NUM_OF_PORT (NUM_OF_CFP_PORT+NUM_OF_QSFP_PORT)

/* For the CFP port, it does not use the I2C bus for access */
static const int port_bus_index[NUM_OF_PORT] = {
    -1, -1, 14, 15, 16, 17, 18, 19, 20, 21, 
    -1, -1, 30, 31, 32, 33, 34, 35, 36, 37
};

#define PORT_BUS_INDEX(port) (port_bus_index[port-1])

/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
static int
onlp_sfpi_get_sled_id(int port)
{
    return ((int)((port-1) / 10) + 1);
}

static char*
onlp_sfpi_get_attribute_dir(int port)
{
    char *dir[] = { MODULE_ATTRIBUTE_PATH_SLED1, MODULE_ATTRIBUTE_PATH_SLED2 };
    return dir[onlp_sfpi_get_sled_id(port)-1]; /* SLED id is 1-based */
}

static int
onlp_is_cfp_port(int port)
{
    return ((port >= 1 && port <= 2) || (port >= 11 && port <= 12));
}

static int
onlp_cfp_eeprom_read(int port, uint8_t data[256])
{
    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    int size = 0;
    char *dir = NULL;

    VALIDATE_CFP(port);
    memset(data, 0, 256);

    if (!onlp_sled_board_is_ready(onlp_sfpi_get_sled_id(port))) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get CFP eeprom */
    dir = onlp_sfpi_get_attribute_dir(port);

    if(onlp_file_read(data, 256, &size, MODULE_CFP_EEPROM_FORMAT, dir, port) != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (size != 256) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d), size is different!\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

static int
onlp_cfp_control_set(int port, onlp_sfp_control_t control, int value)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

static int
onlp_cfp_control_get(int port, onlp_sfp_control_t control, int* value)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

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

    for(p = 1; p <= NUM_OF_PORT; p++) {
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
    int present;
    char *dir = NULL;
    VALIDATE_PORT(port);

    if (!onlp_sled_board_is_ready(onlp_sfpi_get_sled_id(port))) {
        return 0;
    }

    dir = onlp_sfpi_get_attribute_dir(port);

    /* Get presence status */
    if (onlp_file_read_int(&present, MODULE_PRESENT_FORMAT, dir, port) < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    AIM_BITMAP_CLR_ALL(dst);
    return ONLP_STATUS_OK;
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

    VALIDATE_PORT(port);
    memset(data, 0, 256);

    if (!onlp_sled_board_is_ready(onlp_sfpi_get_sled_id(port))) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get CFP eeprom */
    if (onlp_is_cfp_port(port)) {
        return onlp_cfp_eeprom_read(port, data);
    }

    /* Access the different transceiver eeprom path for the SLED1 off case */
    if (!onlp_sled_board_is_ready(1)) {
        /* Get QSFP eeprom */
        if(onlp_file_read(data, 256, &size, MODULE_QSFP_EEPROM_FORMAT, (PORT_BUS_INDEX(port)-16)) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else
    {
        /* Get QSFP eeprom */
        if(onlp_file_read(data, 256, &size, MODULE_QSFP_EEPROM_FORMAT, PORT_BUS_INDEX(port)) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    if (size != 256) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d), size is different!\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    FILE* fp;
    char file[64] = {0};

    VALIDATE_QSFP(port);

    if (!onlp_sled_board_is_ready(onlp_sfpi_get_sled_id(port))) {
        return ONLP_STATUS_E_INTERNAL;
    }

    sprintf(file, MODULE_QSFP_EEPROM_FORMAT, PORT_BUS_INDEX(port));
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
        AIM_LOG_ERROR("Unable to read the module_eeprom device file of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    VALIDATE_QSFP(port);

    if (!onlp_sled_board_is_ready(onlp_sfpi_get_sled_id(port))) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return onlp_i2c_readb(PORT_BUS_INDEX(port), devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    VALIDATE_QSFP(port);

    if (!onlp_sled_board_is_ready(onlp_sfpi_get_sled_id(port))) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return onlp_i2c_writeb(PORT_BUS_INDEX(port), devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    VALIDATE_QSFP(port);

    if (!onlp_sled_board_is_ready(onlp_sfpi_get_sled_id(port))) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return onlp_i2c_readw(PORT_BUS_INDEX(port), devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    VALIDATE_QSFP(port);

    if (!onlp_sled_board_is_ready(onlp_sfpi_get_sled_id(port))) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return onlp_i2c_writew(PORT_BUS_INDEX(port), devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    char *dir = NULL;

    if (!onlp_sled_board_is_ready(onlp_sfpi_get_sled_id(port))) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (onlp_is_cfp_port(port)) {
        return onlp_cfp_control_set(port, control, value);
    }

    dir = onlp_sfpi_get_attribute_dir(port);

    switch(control) {
    case ONLP_SFP_CONTROL_RESET_STATE: {
        VALIDATE_QSFP(port);

        if (onlp_file_write_int(value, MODULE_RESET_FORMAT, dir, port) < 0) {
            AIM_LOG_ERROR("Unable to write reset status to port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        return ONLP_STATUS_OK;
    }
    case ONLP_SFP_CONTROL_LP_MODE: {
        VALIDATE_QSFP(port);

        if (onlp_file_write_int(value, MODULE_LPMODE_FORMAT, dir, port) < 0) {
            AIM_LOG_ERROR("Unable to write lp mode status to port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        return ONLP_STATUS_OK;
    }
    default:
        break;
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    char *dir = NULL;

    if (!onlp_sled_board_is_ready(onlp_sfpi_get_sled_id(port))) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (onlp_is_cfp_port(port)) {
        return onlp_cfp_control_get(port, control, value);
    }

    dir = onlp_sfpi_get_attribute_dir(port);

    switch(control) {
    case ONLP_SFP_CONTROL_RESET_STATE: {
        VALIDATE_QSFP(port);

        if (onlp_file_read_int(value, MODULE_RESET_FORMAT, dir, port) < 0) {
            AIM_LOG_ERROR("Unable to read reset status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        return ONLP_STATUS_OK;
    }
    case ONLP_SFP_CONTROL_LP_MODE: {
        VALIDATE_QSFP(port);

        if (onlp_file_read_int(value, MODULE_LPMODE_FORMAT, dir, port) < 0) {
            AIM_LOG_ERROR("Unable to read lp mode status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        return ONLP_STATUS_OK;
    }
    default:
        break;
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
