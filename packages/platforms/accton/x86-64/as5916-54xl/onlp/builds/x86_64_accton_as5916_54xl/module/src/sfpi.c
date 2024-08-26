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
#include "x86_64_accton_as5916_54xl_int.h"
#include "x86_64_accton_as5916_54xl_log.h"

#define PHY_DATA_SIZE 64
#define PORT_PHY_REG_FORMAT              "/sys/devices/platform/as5916_54xl_sfp/module_phy_%d"
#define PORT_EEPROM_FORMAT              "/sys/devices/platform/as5916_54xl_sfp/module_eeprom_%d"
#define MODULE_PRESENT_FORMAT		    "/sys/devices/platform/as5916_54xl_sfp/module_present_%d"
#define MODULE_RXLOS_FORMAT             "/sys/devices/platform/as5916_54xl_sfp/module_rx_los_%d"
#define MODULE_TXFAULT_FORMAT           "/sys/devices/platform/as5916_54xl_sfp/module_tx_fault_%d"
#define MODULE_TXDISABLE_FORMAT         "/sys/devices/platform/as5916_54xl_sfp/module_tx_disable_%d"
#define MODULE_LPMODE_FORMAT            "/sys/devices/platform/as5916_54xl_sfp/module_lpmode_%d"
#define MODULE_RESET_FORMAT             "/sys/devices/platform/as5916_54xl_sfp/module_reset_%d"
#define MODULE_PRESENT_ALL_ATTR	        "/sys/devices/platform/as5916_54xl_sfp/module_present_all"
#define MODULE_RXLOS_ALL_ATTR           "/sys/devices/platform/as5916_54xl_sfp/module_rxlos_all"

#define VALIDATE_QSFP(_port) \
    do { \
        if (_port < 48 || _port > 53 ) \
            return ONLP_STATUS_E_UNSUPPORTED; \
    } while(0)

#define VALIDATE_SFP(_port) \
    do { \
        if (_port < 0 || _port > 47) \
            return ONLP_STATUS_E_UNSUPPORTED; \
    } while(0)

#define VALIDATE_PHY(_port, _devaddr, _addr, _size) \
    do { \
        if (_port < 0 || _port > 47) \
            return ONLP_STATUS_E_UNSUPPORTED; \
        if (_devaddr != 0x56) \
            return ONLP_STATUS_E_UNSUPPORTED; \
        if (_addr >= 0x20) \
            return ONLP_STATUS_E_PARAM; \
        if ((_size % 2) || (size <= 0)) \
            return ONLP_STATUS_E_PARAM; \
        if ((_addr*2 + _size) > PHY_DATA_SIZE) \
            return ONLP_STATUS_E_PARAM; \
    } while(0)

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

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    /*
     * Ports {0, 54}
     */
    int p;

    for(p = 0; p < 54; p++) {
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

	if (onlp_file_read_int(&present, MODULE_PRESENT_FORMAT, (port+1)) < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[7];
    FILE* fp;

    /* Read present status of port 0~54 */
    int count = 0;

    fp = fopen(MODULE_PRESENT_ALL_ATTR, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the module_present_all device file from (%s).", MODULE_PRESENT_ALL_ATTR);
        return ONLP_STATUS_E_INTERNAL;
    }

    count = fscanf(fp, "%x %x %x %x %x %x %x", bytes+0, bytes+1, bytes+2, bytes+3,
                                               bytes+4, bytes+5, bytes+6);
    fclose(fp);
    if(count != 7) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields the module_present_all device file from(%s).", MODULE_PRESENT_ALL_ATTR);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Mask out non-existant QSFP ports */
    bytes[6] &= 0x3F;

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t presence_all = 0 ;
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }

    /* Populate bitmap */
    for(i = 0; presence_all; i++) {
        AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[6];
    FILE* fp;

    /* Read present status of port 0~25 */
    int count = 0;

    fp = fopen(MODULE_RXLOS_ALL_ATTR, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the module_rxlos_all device file from (%s).", MODULE_RXLOS_ALL_ATTR);
        return ONLP_STATUS_E_INTERNAL;
    }

    count = fscanf(fp, "%x %x %x %x %x %x", bytes+0, bytes+1, bytes+2,
                                            bytes+3, bytes+4, bytes+5);
    fclose(fp);
    if(count != 6) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields the module_rxlos_all device file from(%s).", MODULE_RXLOS_ALL_ATTR);
        return ONLP_STATUS_E_INTERNAL;
    }    

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t rx_los_all = 0 ;
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        rx_los_all <<= 8;
        rx_los_all |= bytes[i];
    }

    /* Populate bitmap */
    for(i = 0; rx_los_all; i++) {
        AIM_BITMAP_MOD(dst, i, (rx_los_all & 1));
        rx_los_all >>= 1;
    }

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
    memset(data, 0, 256);

	if(onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT, (port+1)) != ONLP_STATUS_OK) {
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
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;

    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                if (onlp_file_write_int(value, MODULE_TXDISABLE_FORMAT, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to set tx_disable status to port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }
        case ONLP_SFP_CONTROL_RESET_STATE: 
            {
                VALIDATE_QSFP(port);

                if (onlp_file_write_int(value, MODULE_RESET_FORMAT, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to write reset status to port(%d)\r\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }

                return ONLP_STATUS_OK;
            }
        case ONLP_SFP_CONTROL_LP_MODE:
            {
                VALIDATE_QSFP(port);

                if (onlp_file_write_int(value, MODULE_LPMODE_FORMAT, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to write lpmode status to port(%d)\r\n", port);
                    return ONLP_STATUS_E_INTERNAL;
                }

                return ONLP_STATUS_OK;
            }
        default:
            rv = ONLP_STATUS_E_UNSUPPORTED;
            break;
        }

    return rv;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int rv;

    if (port < 0) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            {
                VALIDATE_SFP(port);
                
            	if (onlp_file_read_int(value, MODULE_RXLOS_FORMAT, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to read rx_loss status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }

        case ONLP_SFP_CONTROL_TX_FAULT:
            {
                VALIDATE_SFP(port);
                
            	if (onlp_file_read_int(value, MODULE_TXFAULT_FORMAT, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to read tx_fault status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }

        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
            	if (onlp_file_read_int(value, MODULE_TXDISABLE_FORMAT, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to read tx_disabled status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }
        case ONLP_SFP_CONTROL_RESET_STATE:
            {
                VALIDATE_QSFP(port);

                if (onlp_file_read_int(value, MODULE_RESET_FORMAT, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to read reset status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }
        case ONLP_SFP_CONTROL_LP_MODE:
            {
                VALIDATE_QSFP(port);

                if (onlp_file_read_int(value, MODULE_LPMODE_FORMAT, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to read lpmode status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }
        default:
            rv = ONLP_STATUS_E_UNSUPPORTED;
        }

    return rv;
}

#define SFP_PAGE_OFFSET      383
#define QSFP_PAGE_OFFSET     127
#define SFF_A0_ADDR          0x50
#define SFF_A2_ADDR          0x51
#define SFF_A0_DATA_SIZE     256
#define SFF_A2_NON_PAGE_SIZE 128
#define SFF_PAGE_DATA_SIZE   128
#define SFP_PORT_IDX_END     47
#define QSFP_PORT_IDX_BEGIN  48
#define QSFP_PORT_IDX_END    53

static int onlp_sfpi_dev_get_sysfs_off(int port, uint8_t devaddr, uint8_t addr, uint8_t page)
{
    if (port <= SFP_PORT_IDX_END) {
        if (devaddr == SFF_A0_ADDR) {
            return addr;
        }

        // SFF_A2_ADDR
        if (addr < SFF_A2_NON_PAGE_SIZE) {
            return SFF_A0_DATA_SIZE + addr;
        }

        return SFF_A0_DATA_SIZE + (page * SFF_PAGE_DATA_SIZE) + addr;
    }

    // QSFP
    return (addr < SFF_A2_NON_PAGE_SIZE) ? (addr) : (page * SFF_PAGE_DATA_SIZE) + addr;
}

static int onlp_sfpi_dev_read_write(int port, uint8_t devaddr, uint8_t addr, uint8_t* data, int size, int write_access)
{
    FILE* fp;
    char file[64] = {0};
    int  seek_off = 0;
    uint8_t page = 0;

    if (port < 0 || port > QSFP_PORT_IDX_END) {
        return ONLP_STATUS_E_PARAM;
    }

    if ((port <= SFP_PORT_IDX_END) && (devaddr != 0x50 && devaddr != 0x51)) {
        return ONLP_STATUS_E_PARAM;
    }

    if ((port >= QSFP_PORT_IDX_BEGIN) && (devaddr != 0x50)) {
        return ONLP_STATUS_E_PARAM;
    }

    if ((addr + size) > 256) {
        return ONLP_STATUS_E_PARAM;
    }

    sprintf(file, PORT_EEPROM_FORMAT, (port+1));
    fp = fopen(file, "r+");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the eeprom device file of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    // Disable buffering
	setvbuf(fp, NULL, _IONBF, 0);

    // Read out current page to decide the target offset
    seek_off = (port <= SFP_PORT_IDX_END) ? SFP_PAGE_OFFSET : QSFP_PAGE_OFFSET;
    if (fseek(fp, seek_off, SEEK_SET) != 0) {
        fclose(fp);
        AIM_LOG_ERROR("Unable to set the file position indicator of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    int ret = fread(&page, 1, 1, fp);
    if (ret != 1) {
        fclose(fp);
        AIM_LOG_ERROR("Unable to read the module_eeprom device file of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    seek_off = onlp_sfpi_dev_get_sysfs_off(port, devaddr, addr, page);
    if (fseek(fp, seek_off, SEEK_SET) != 0) {
        fclose(fp);
        AIM_LOG_ERROR("Unable to set the file position indicator of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (write_access) {
        int ret = fwrite(data, 1, size, fp);

        fclose(fp);
        if (ret != size) {
            AIM_LOG_ERROR("Unable to write the module_eeprom device file of port(%d)", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else {
        int ret = fread(data, 1, size, fp);

        fclose(fp);
        if (ret != size) {
            AIM_LOG_ERROR("Unable to read the module_eeprom device file of port(%d)", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    return ONLP_STATUS_OK;
}

static int onlp_sfpi_phy_dev_read_write(int port, uint8_t devaddr, uint8_t addr, uint8_t* data, int size, int write_access)
{
    int ret;
    FILE* fp;
    char file[64] = {0};
    VALIDATE_PHY(port, devaddr, addr, size);

    sprintf(file, PORT_PHY_REG_FORMAT, (port+1));
    fp = fopen(file, "r+");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the phy device file of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    setvbuf(fp, NULL, _IONBF, 0); // Disable buffering

    addr *= 2;
    if (fseek(fp, addr, SEEK_SET) != 0) {
        fclose(fp);
        AIM_LOG_ERROR("Unable to set the phy position indicator of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    ret = write_access ? fwrite(data, 1, size, fp) : fread(data, 1, size, fp);
    fclose(fp);

    if (ret != size) {
        AIM_LOG_ERROR("Unable to %s the module_phy device file of port(%d)", 
                      write_access ? "write" : "read", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    uint8_t data;
    int ret = onlp_sfpi_dev_read_write(port, devaddr, addr, &data, 1, 0);
    return (ret < 0) ? ret : data;
}

int onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    return onlp_sfpi_dev_read_write(port, devaddr, addr, &value, 1, 1);
}

int onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    uint8_t data[2];
    int ret = 0;

    if ((port <= SFP_PORT_IDX_END) && (devaddr == 0x56)) {
        ret = onlp_sfpi_phy_dev_read_write(port, devaddr, addr, data, 2, 0);
    }
    else {
        // Split into two byte operation if data cross the boundary
        if (addr == 0x7F) {
            int i = 0;

            for (i = 0; i < 2; i++) {
                ret = onlp_sfpi_dev_readb(port, devaddr, addr+i);
                if (ret < 0) {
                    break;
                }

                data[i] = ret;
            }
        }
        else {
            ret = onlp_sfpi_dev_read_write(port, devaddr, addr, data, 2, 0);
        }
    }

    return (ret < 0) ? ret : (data[0] | (data[1] << 8));
}

int onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int ret = 0;
    uint8_t data[2] = {value & 0xFF, (value >> 8) & 0xFF};

    if ((port <= SFP_PORT_IDX_END) && (devaddr == 0x56)) {
        return onlp_sfpi_phy_dev_read_write(port, devaddr, addr, data, 2, 1);
    }

    // Split into two byte operation if data cross the boundary
    if (addr == 0x7F) {
        int i = 0;

        for (i = 0; i < 2; i++) {
            ret = onlp_sfpi_dev_writeb(port, devaddr, addr+i, data[i]);
            if (ret < 0) {
                break;
            }
        }
    }
    else {
        ret = onlp_sfpi_dev_read_write(port, devaddr, addr, data, 2, 1);
    }

    return ret;
}

int onlp_sfpi_dev_read(int port, uint8_t devaddr, uint8_t addr, uint8_t* rdata, int size)
{
    int ret = 0;

    if ((port <= SFP_PORT_IDX_END) && (devaddr == 0x56)) {
        return onlp_sfpi_phy_dev_read_write(port, devaddr, addr, rdata, size, 0);
    }

    // Split into two byte operation if data cross the boundary
    if ((addr+size) > SFF_PAGE_DATA_SIZE) {
        ret = onlp_sfpi_dev_read_write(port, devaddr, addr, rdata, SFF_PAGE_DATA_SIZE-addr, 0);
        if (ret < 0) {
            return ret;
        }

        ret = onlp_sfpi_dev_read_write(port, devaddr, SFF_PAGE_DATA_SIZE, rdata + (SFF_PAGE_DATA_SIZE-addr), (addr+size-SFF_PAGE_DATA_SIZE), 0);
        if (ret < 0) {
            return ret;
        }
    }
    else {
        ret = onlp_sfpi_dev_read_write(port, devaddr, addr, rdata, size, 0);
    }

    return ret;
}

int onlp_sfpi_dev_write(int port, uint8_t devaddr, uint8_t addr, uint8_t* data, int size)
{
    int ret = 0;

    if ((port <= SFP_PORT_IDX_END) && (devaddr == 0x56)) {
        return onlp_sfpi_phy_dev_read_write(port, devaddr, addr, data, size, 1);
    }

    // Split into two byte operation if data cross the boundary
    if ((addr+size) > SFF_PAGE_DATA_SIZE) {
        ret = onlp_sfpi_dev_read_write(port, devaddr, addr, data, SFF_PAGE_DATA_SIZE-addr, 1);
        if (ret < 0) {
            return ret;
        }

        ret = onlp_sfpi_dev_read_write(port, devaddr, SFF_PAGE_DATA_SIZE, data + (SFF_PAGE_DATA_SIZE-addr), (addr+size-SFF_PAGE_DATA_SIZE), 1);
        if (ret < 0) {
            return ret;
        }
    }
    else {
        ret = onlp_sfpi_dev_read_write(port, devaddr, addr, data, size, 1);
    }

    return ret;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
