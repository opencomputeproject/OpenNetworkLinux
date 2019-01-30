/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
 * SFPI Interface for the Aurora 420 Platform
 *
 ***********************************************************/
#include <x86_64_netberg_aurora_420_rangeley/x86_64_netberg_aurora_420_rangeley_config.h>
#include <onlp/oids.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/file.h>
#include <onlplib/sfp.h>
#include "x86_64_netberg_aurora_420_rangeley_int.h"
#include "x86_64_netberg_aurora_420_rangeley_log.h"

#include <unistd.h>
#include <fcntl.h>

/* Model ID Definition */
typedef enum
{
    HURACAN_WITH_BMC = 0x0,
    HURACAN_WITHOUT_BMC,
    CABRERAIII_WITH_BMC,
    CABRERAIII_WITHOUT_BMC,
    SESTO_WITH_BMC,
    SESTO_WITHOUT_BMC,
    NCIIX_WITH_BMC,
    NCIIX_WITHOUT_BMC,
    ASTERION_WITH_BMC,
    ASTERION_WITHOUT_BMC,
    HURACAN_A_WITH_BMC,
    HURACAN_A_WITHOUT_BMC,

    MODEL_ID_LAST
} modelId_t;

static int
onlp_board_model_id_get(void)
{
    static int board_model_id = MODEL_ID_LAST;

    if (board_model_id == MODEL_ID_LAST)
    {
        if (onlp_file_read_int(&board_model_id, SYS_HWMON1_PREFIX "/board_model_id") != ONLP_STATUS_OK)
            return 0;
    }

    return board_model_id;
}

/*
 * This function will be called prior to all other onlp_sfpi_* functions.
 */
int
onlp_sfpi_init(void)
{
    return ONLP_STATUS_OK;
}

/*
 * This function should populate the give bitmap with
 * all valid, SFP-capable port numbers.
 *
 * Only port numbers in this bitmap will be queried by the the
 * ONLP framework.
 *
 * No SFPI functions will be called with ports that are
 * not in this bitmap. You can ignore all error checking
 * on the incoming ports defined in this interface.
 */
int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;
    int total_port = 0;
    int board_model_id = onlp_board_model_id_get();

    switch (board_model_id)
    {
        case HURACAN_WITH_BMC:
        case HURACAN_WITHOUT_BMC:
        case HURACAN_A_WITH_BMC:
        case HURACAN_A_WITHOUT_BMC:
            total_port = 32;
            break;

        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
            total_port = 54;
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
            total_port = 64;
            break;

        default:
            break;
    }

    AIM_BITMAP_CLR_ALL(bmap);
    for(p = 0; p < total_port; p++)
        AIM_BITMAP_SET(bmap, p);

    return ONLP_STATUS_OK;
}

/*
 * This function should return whether an SFP is inserted on the given
 * port.
 *
 * Returns 1 if the SFP is present.
 * Returns 0 if the SFP is not present.
 * Returns ONLP_E_* if there was an error determining the status.
 */
int
onlp_sfpi_is_present(int port)
{
    int value = 0;

    onlp_file_read_int(&value, SYS_HWMON2_PREFIX "/port_%d_abs", (port+1));
    return value;
}

int
onlp_sfpi_port_map(int port, int* rport)
{
    int board_model_id = onlp_board_model_id_get();

    switch (board_model_id)
    {
        case HURACAN_WITH_BMC:
        case HURACAN_WITHOUT_BMC:
        case HURACAN_A_WITH_BMC:
        case HURACAN_A_WITHOUT_BMC:
            /* odd <=> even */
            if (port & 0x1)
                *rport = (port - 1);
            else
                *rport = (port + 1);
            break;

        default:
            *rport = port; break;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;
    int total_port = 0;
    int board_model_id = onlp_board_model_id_get();

    switch (board_model_id)
    {
        case HURACAN_WITH_BMC:
        case HURACAN_WITHOUT_BMC:
        case HURACAN_A_WITH_BMC:
        case HURACAN_A_WITHOUT_BMC:
            total_port = 32;
            break;

        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
            total_port = 54;
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
            total_port = 64;
            break;

        default:
            break;
    }

    AIM_BITMAP_CLR_ALL(bmap);
    for(p = 0; p < total_port; p++)
        AIM_BITMAP_SET(bmap, p);

    return ONLP_STATUS_OK;
}

/*
 * This function reads the SFPs idrom and returns in
 * in the data buffer provided.
 */
int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    int rv = ONLP_STATUS_OK;
    char fname[128];

    memset(data, 0, 256);
    memset(fname, 0, sizeof(fname));
    sprintf(fname, SYS_HWMON2_PREFIX "/port_%d_data_a0", (port+1));
    rv = onlplib_sfp_eeprom_read_file(fname, data);
    if (rv != ONLP_STATUS_OK)
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);

    return rv;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    int rv = ONLP_STATUS_OK;
    char fname[128];

    memset(data, 0, 256);
    memset(fname, 0, sizeof(fname));
    sprintf(fname, SYS_HWMON2_PREFIX "/port_%d_data_a2", (port+1));
    rv = onlplib_sfp_eeprom_read_file(fname, data);
    if (rv != ONLP_STATUS_OK)
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);

    return rv;
}

/*
 * Manually enable or disable the given SFP.
 *
 */
int
onlp_sfpi_enable_set(int port, int enable)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * Returns whether the SFP is currently enabled or disabled.
 */
int
onlp_sfpi_enable_get(int port, int* enable)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * If the platform requires any setup or equalizer modifications
 * based on the actual SFP that was inserted then that custom
 * setup should be performed here.
 *
 * After a new SFP is detected by the ONLP framework this
 * function will be called to perform the (optional) setup.
 */
int
onlp_sfpi_post_insert(int port, sff_info_t* sff_info)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * Return the current status of the SFP.
 * See onlp_sfp_status_t;
 */
int
onlp_sfpi_status_get(int port, uint32_t* status)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* supported)
{
    if (supported == NULL)
        return ONLP_STATUS_E_PARAM;

    *supported = 0;
    switch (control)
    {
        case ONLP_SFP_CONTROL_TX_DISABLE:
        case ONLP_SFP_CONTROL_RX_LOS:
        case ONLP_SFP_CONTROL_TX_FAULT:
        {
            int board_model_id = onlp_board_model_id_get();

            switch (board_model_id)
            {
                case HURACAN_WITH_BMC:
                case HURACAN_WITHOUT_BMC:
                case HURACAN_A_WITH_BMC:
                case HURACAN_A_WITHOUT_BMC:
                case SESTO_WITH_BMC:
                case SESTO_WITHOUT_BMC:
                case NCIIX_WITH_BMC:
                case NCIIX_WITHOUT_BMC:
                case ASTERION_WITH_BMC:
                case ASTERION_WITHOUT_BMC:
                    *supported = 1;
                    break;

                default:
                    break;
            }
        }
            break;

        default:
            break;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv = ONLP_STATUS_OK;
    int supported = 0;

    if ((onlp_sfpi_control_supported(port, control, &supported) == ONLP_STATUS_OK) && (supported == 0))
        return ONLP_STATUS_E_UNSUPPORTED;

    switch (control)
    {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            rv = onlp_file_write_int(value, SYS_HWMON2_PREFIX "/port_%d_tx_disable", (port+1));
            break;

        default:
            rv = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }
    return rv;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int rv = ONLP_STATUS_OK;
    int supported = 0;

    if (value == NULL)
        return ONLP_STATUS_E_PARAM;

    if ((onlp_sfpi_control_supported(port, control, &supported) == ONLP_STATUS_OK) && (supported == 0))
        return ONLP_STATUS_E_UNSUPPORTED;

    *value = 0;
    switch (control)
    {
        case ONLP_SFP_CONTROL_RX_LOS:
            rv = onlp_file_read_int(value, SYS_HWMON2_PREFIX "/port_%d_rxlos", (port+1));
            break;

        case ONLP_SFP_CONTROL_TX_DISABLE:
            rv = onlp_file_read_int(value, SYS_HWMON2_PREFIX "/port_%d_tx_disable", (port+1));
            break;

        case ONLP_SFP_CONTROL_TX_FAULT:
            rv = onlp_file_read_int(value, SYS_HWMON2_PREFIX "/port_%d_tx_fault", (port+1));
            break;

        default:
            rv = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }
    return rv;
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int value = 0;
    char fname[128];
    char data[512];

    memset(data, 0, 512);
    memset(fname, 0, sizeof(fname));
    sprintf(fname, SYS_HWMON2_PREFIX "/port_%d_sfp_copper", (port+1));

    int fd = open(fname, O_RDONLY);
    if (fd < 0) {
        AIM_LOG_INFO("Unable to read devaddr(0xAC) from port(%d)\r\n", port);
        return value;
    }

    int nrd = read(fd, data, 512);
    close(fd);

    if (nrd != 512) {
        AIM_LOG_INTERNAL("Failed to read EEPROM file '%s'", fname);
        return value;
    }

    value = (((data[addr*2 + 1] & 0xff) << 8) | (data[addr*2] & 0xff)) & 0xffff;

    return value;
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int rv = ONLP_STATUS_OK;
    int data = 0;

    data = ((addr << 16) | (value & 0xffff)) & 0x00ffffff;
    rv = onlp_file_write_int(data, SYS_HWMON2_PREFIX "/port_%d_sfp_copper", (port+1));

    return rv;
}

/*
 * This is a generic ioctl interface.
 */
int
onlp_sfpi_ioctl(int port, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * De-initialize the SFPI subsystem.
 */
int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

