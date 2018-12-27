/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
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
#include <time.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#include "x86_64_accton_minipack_log.h"

#define BIT(i)          (1 << (i))
#define NUM_OF_PIM              (8)
#define TYPES_OF_PIM            (2)
#define NUM_OF_SFP_PORT         (128)
#define SFP_PORT_PER_PIM        (NUM_OF_SFP_PORT/NUM_OF_PIM)
#define PORT_EEPROM_FORMAT      "/sys/bus/i2c/devices/%d-0050/eeprom"

/* PIM stands for "Port Interface Module".
 * For minipack, there are hot-pluggable 8 PIMs.
 * Each PIM can have 16*16Q or 4*4DD ports.
 */
#define I2C_BUS                 (1)
#define PIM_POLL_INTERVAL       (8) /*in seconds*/
#define PORT_POLL_INTERVAL      (5) /*per PIM, in seconds*/


typedef struct {
    bool      valid;
    time_t    last_poll;
    uint32_t  present;
} present_status_t;

typedef struct {
    present_status_t pim;
    present_status_t port_at_pim[NUM_OF_SFP_PORT/NUM_OF_PIM];
} port_status_t;

static port_status_t ps;
/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
static int
sfpi_eeprom_close_all_channels(void);

int
onlp_sfpi_init(void)
{
    /* Called at initialization time */
    memset(&ps, 0, sizeof(ps));
    sfpi_eeprom_close_all_channels();
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    /*
     * Ports {0, 32}
     */
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 0; p < NUM_OF_SFP_PORT; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

/*
0 @bit 14
1 @bit 15
2 @bit 12
3 @bit 13
4 @bit 10
5 @bit 11
6 @bit 8
7 @bit 9
... same order for port 8-15.
*/
static uint32_t
onlp_sfpi_reg_val_to_port_sequence(uint32_t value, int revert)
{
    int i, j;
    uint32_t ret = 0;

    for (i = 0; i < SFP_PORT_PER_PIM; i++) {
        if (i % 2) {
            j = 16 - i;
        }
        else {
            j = 14 - i;
        }
        ret |= (!!(value & BIT(j)) << i);
    }

    return revert ? ~ret : ret;
}


/* "PIM" stands for "Port Interface Module".
 * A pim can have 16 QSFP ports of 100Gbps, or 4 DD ports of 400 Gbps.
 *
 * Return 1 if present.
 * Return 0 if not present.
 * Return < 0 if error.
 */
static int
onlp_pim_is_present(int pim)
{
    time_t cur, elapse;
    uint32_t present;
    int bus = 12;
    int addr = 0x3e;
    int offset = 0x32;

    cur = time (NULL);
    elapse = cur - ps.pim.last_poll;

    if (!ps.pim.valid || (elapse > PIM_POLL_INTERVAL)) {
        present = bmc_i2c_readb(bus, addr, offset);
        if (present < 0) {
            ps.pim.valid = 0;
            return ONLP_STATUS_E_INTERNAL;
        }
        ps.pim.valid = 1;
        ps.pim.present = present;
        ps.pim.last_poll = time (NULL);
    } else {
        present = ps.pim.present;
    }

    return !(present & BIT(pim % NUM_OF_PIM));
}

static int
get_port_present_bmap(int port, uint32_t *bit_array)
{
    time_t cur, elapse;
    uint32_t present, pim;
    present_status_t *ports;
    int bus[NUM_OF_PIM] = {80, 88, 96, 104, 112, 120, 128, 136};
    int addr[TYPES_OF_PIM] = {0x60, 0x61};  /*Different for 16Q and 4DD.*/
    int offset = 0x12;

    pim = port/SFP_PORT_PER_PIM;

    ports = &ps.port_at_pim[pim];
    cur = time (NULL);
    elapse = cur - ports->last_poll;

    if (!ports->valid || (elapse > PORT_POLL_INTERVAL)) {
        present = bmc_i2c_readw(bus[pim], addr[0], offset);
        if (present < 0) {
            ports->valid = 0;
            return ONLP_STATUS_E_INTERNAL;
        }
        ports->valid = 1;
        ports->present = present;
        ports->last_poll = time (NULL);
    } else {
        present = ports->present;
    }

    *bit_array = onlp_sfpi_reg_val_to_port_sequence(present, 0);
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
    int present, pim, ret;
    uint32_t bit_array;

    pim = port/SFP_PORT_PER_PIM;
    present = onlp_pim_is_present(pim);
    if (present < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    if (!present) {
        return 0;
    }

    ret = get_port_present_bmap(port, &bit_array);
    if (ret < 0) {
        return ret;
    }


    return !(bit_array & BIT(port % SFP_PORT_PER_PIM));
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int i, port, ret;
    uint32_t bmp;
    uint32_t bmap_pim[NUM_OF_PIM] = {0};

    /*Get present bitmap per PIM.*/
    for (i = 0; i < NUM_OF_PIM; i++) {
        port = i*SFP_PORT_PER_PIM;
        ret = get_port_present_bmap(port, &bmap_pim[i]);
        if (ret < 0)
            return ret;
    }

    for (i = 0; i < NUM_OF_SFP_PORT; i++) {
        AIM_BITMAP_CLR(dst, i);
    }
    for (i = 0; i < NUM_OF_SFP_PORT; i++) {
        bmp  = bmap_pim[i/SFP_PORT_PER_PIM];
        if (!(bmp & BIT(i%SFP_PORT_PER_PIM))) {
            AIM_BITMAP_SET(dst, i);
        }
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    return ONLP_STATUS_OK;
}

static int
sfpi_eeprom_close_all_channels(void)
{
    int i, k;
    int value = 0 ;
    int mux_1st = 0x70;
    int mux_2st[] = {0x72, 0x71};
    int offset = 0;
    int channels = 8;

    for (i = 0; i < channels; i++) {
        /* Skip checking if PIM is plugged. Cuz BMC traffic may not be ready at init.
        if (onlp_pim_is_present(i) != 1)
             continue;
        */
        value = 1<<i;
        /*Open only 1 channel of level-1 mux*/
        if (onlp_i2c_writeb(I2C_BUS, mux_1st, offset, value, ONLP_I2C_F_FORCE) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
        /*Close mux on each PIM.*/
        for (k = 0; k < AIM_ARRAYSIZE(mux_2st); k++) {
            if (onlp_i2c_writeb(I2C_BUS, mux_2st[k], offset, 0, ONLP_I2C_F_FORCE) < 0) {
                ; /*return ONLP_STATUS_E_INTERNAL;*/
            }
        }
    }

    /*close level-1 mux*/
    if (onlp_i2c_writeb(I2C_BUS, mux_1st, offset, 0, ONLP_I2C_F_FORCE) < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}


/*Retrieve the channel order on a PIM.*/
static int
sfpi_get_channel_mapping(int port)
{
    int pp = port % SFP_PORT_PER_PIM;
    int index, base;

    base  = (pp)/8*8;
    index = pp % 8;
    index = 7 - index;
    if (index % 2) {
        index --;
    } else {
        index ++;
    }
    index += base;

    return index;
}

/*Set the 2-level i2c mux to open channel to that port.*/
static int
sfpi_eeprom_channel_open(int port)
{
    int pim, reg, i, index;
    int mux_1st = 0x70;
    int mux_2st[] = {0x72, 0x71};
    int offset = 0;

    pim = port/SFP_PORT_PER_PIM;
    reg = 1 << pim;

    /*Open only 1 channel of level-1 mux*/
    if (onlp_i2c_writeb(I2C_BUS, mux_1st, offset, reg, ONLP_I2C_F_FORCE) < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /*Open only 1 channel on that PIM.*/
    index = sfpi_get_channel_mapping(port);
    for (i = 0; i < AIM_ARRAYSIZE(mux_2st); i++) {
        if ((index/8) != i) {
            reg = 0;
        } else {
            reg = BIT(index%8);
        }
        if (onlp_i2c_writeb(I2C_BUS, mux_2st[i], offset, reg, ONLP_I2C_F_FORCE) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    return ONLP_STATUS_OK;
}

/* Due to PIM can be hot swapped, here the eeprom driver is always at root bus.
 * To avoid multi-slave condition, only 1 channel is opened on reading.
 */
int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    FILE* fp;
    int ret;
    char file[64] = {0};

    ret = sfpi_eeprom_channel_open(port);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to set i2c channel for the module_eeprom of port(%d, %d)", port, ret);
        return ONLP_STATUS_E_INTERNAL;
    }

    sprintf(file, PORT_EEPROM_FORMAT, I2C_BUS);
    fp = fopen(file, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the eeprom device file of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    ret = fread(data, 1, 256, fp);
    fclose(fp);
    if (ret != 256) {
        AIM_LOG_ERROR("Unable to read the module_eeprom device file of port(%d, %d)", port, ret);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

/* Due to PIM can be hot swapped, here the eeprom driver is always at root bus.
 * To avoid multi-slave condition, only 1 channel is opened on reading.
 */
int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    FILE* fp;
    int ret;
    char file[64] = {0};

    ret = sfpi_eeprom_channel_open(port);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to set i2c channel for the module_eeprom of port(%d, %d)", port, ret);
        return ONLP_STATUS_E_INTERNAL;
    }

    sprintf(file, PORT_EEPROM_FORMAT, I2C_BUS);
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

    ret = fread(data, 1, 256, fp);
    fclose(fp);
    if (ret != 256) {
        AIM_LOG_ERROR("Unable to read the module_eeprom device file of port(%d, %d)", port, ret);
        return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

