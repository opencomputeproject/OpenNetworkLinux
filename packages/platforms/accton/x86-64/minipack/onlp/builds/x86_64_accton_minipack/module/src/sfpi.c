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
#include <sys/types.h>
#include <unistd.h>
#include "platform_lib.h"
#include "x86_64_accton_minipack_log.h"

/* PIM stands for "Port Interface Module".
 * For minipack, there are hot-pluggable 8 PIMs.
 * Each PIM can have 16*16Q or 4*4DD ports.
 */
#define NUM_OF_PIM              (8)
#define TYPES_OF_PIM            (2)
#define NUM_OF_SFP_PORT         (128)
#define SFP_PORT_PER_PIM        (NUM_OF_SFP_PORT/NUM_OF_PIM)
#define I2C_BUS                 (1)
#define PIM_POLL_INTERVAL       (5) /*in seconds*/
#define PORT_POLL_INTERVAL      (8) /*per PIM, in seconds*/
#define PORT_EEPROM_FORMAT      "/sys/bus/i2c/devices/%d-0050/eeprom"

typedef struct {
    bool      valid;
    time_t    last_poll;
    uint32_t  present;
} present_status_t;

typedef struct {
    present_status_t pim;
    present_status_t port_at_pim[NUM_OF_SFP_PORT/NUM_OF_PIM];

    sem_t mutex;
} sfpi_port_status_t;

static int sfpi_eeprom_close_all_channels(void);
static int onlp_read_pim_present(uint32_t *bmap);

#define SEM_LOCK    do {sem_wait(&global_sfpi_st->mutex);} while(0)
#define SEM_UNLOCK  do {sem_post(&global_sfpi_st->mutex);} while(0)

sfpi_port_status_t *global_sfpi_st = NULL;

/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
static int sfpi_create_shm(key_t id) {
    int rv;

    if (global_sfpi_st == NULL) {
        rv = onlp_shmem_create(id, sizeof(sfpi_port_status_t),
                               (void**)&global_sfpi_st);
        if (rv >= 0) {
            if(pltfm_create_sem(&global_sfpi_st->mutex) != 0) {
                AIM_DIE("onlpi_create_sem(): mutex_init failed\n");
                return ONLP_STATUS_E_INTERNAL;
            }
        }
        else {
            AIM_DIE("Global %s created failed.", __func__);
        }
    }
    return ONLP_STATUS_OK;
}

static int update_ports(int pim, bool valid, uint32_t present) {
    present_status_t *ports;

    SEM_LOCK;
    ports = &global_sfpi_st->port_at_pim[pim];
    ports->valid = valid;
    ports->present = present;
    ports->last_poll = time (NULL);

    SEM_UNLOCK;
    return ONLP_STATUS_OK;
}

int onlp_sfpi_init(void)
{
    if (sfpi_create_shm(ONLP_SFPI_SHM_KEY) < 0) {
        AIM_DIE("onlp_sfpi_init::sfpi_create_shm created failed.");
        return ONLP_STATUS_E_INTERNAL;
    }
    sfpi_eeprom_close_all_channels();
    return ONLP_STATUS_OK;
}

int onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
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
_sfpi_port_present_remap_reg(uint32_t value)
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

    return ret;
}

static int
onlp_read_pim_present(uint32_t *bmap)
{
    uint32_t present;
    int bus = 12;
    int addr = 0x3e;
    int offset = 0x32;

    present = bmc_i2c_readb(bus, addr, offset);
    if (present < 0) {
        *bmap = 0;
        return ONLP_STATUS_E_INTERNAL;
    }
    *bmap = ~present;
    return ONLP_STATUS_OK;
}

/* "PIM" stands for "Port Interface Module". They are hot-pluggable.
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
    int ret;
    sfpi_port_status_t *ps;

    SEM_LOCK;
    ps = global_sfpi_st;
    cur = time (NULL);
    elapse = cur - ps->pim.last_poll;
    if (!ps->pim.valid || (elapse > PIM_POLL_INTERVAL)) {
        ret = onlp_read_pim_present(&present);
        if (ret < 0) {
            ps->pim.valid = false;
            ps->pim.present = 0;
            present = 0;
            SEM_UNLOCK;
            return ONLP_STATUS_E_INTERNAL;
        } else {
            ps->pim.valid = true;
            ps->pim.present = present;
            ps->pim.last_poll = time (NULL);
        }
    } else {
        present = ps->pim.present;
    }
    SEM_UNLOCK;
    return !!(present & BIT(pim % NUM_OF_PIM));
}



/*bit_array is the present bitmap of a PIM, not all ports on this machine.*/
static int
get_pim_port_present_bmap(int port, uint32_t *bit_array)
{
    time_t cur, elapse;
    int    ret;
    uint32_t present, pim;
    present_status_t *ports;
    int bus[NUM_OF_PIM] = {80, 88, 96, 104, 112, 120, 128, 136};
    int addr[TYPES_OF_PIM] = {0x60, 0x61};  /*Different for 16Q and 4DD.*/
    int offset = 0x12;

    pim = port/SFP_PORT_PER_PIM;

    /*If PIM not present, set all 0's to pbmap.*/
    if(onlp_pim_is_present(pim) == 0) {
        present  = 0;
        update_ports(pim, 0, present);
        *bit_array = _sfpi_port_present_remap_reg(present);
        return ONLP_STATUS_OK;
    }

    ports = &global_sfpi_st->port_at_pim[pim];
    cur = time (NULL);
    elapse = cur - ports->last_poll;

    if (!ports->valid || (elapse > PORT_POLL_INTERVAL)) {
        ret = bmc_i2c_readw(bus[pim], addr[0], offset, (uint16_t*)&present);
        if (ret < 0) {
            present  = 0;
            update_ports(pim, 0, present);
            *bit_array = present;        /*No needs for remmaped.*/
            return ONLP_STATUS_E_INTERNAL;
        } else {
            present = ~present;
            update_ports(pim, 1, present);
        }
    } else {
        present = ports->present;
    }
    *bit_array = _sfpi_port_present_remap_reg(present);
    return ONLP_STATUS_OK;
}

/*Retrieve the channel order on a PIM.*/
static int
sfpi_get_i2cmux_mapping(int port)
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
    reg = BIT(pim);
    /*Open only 1 channel of level-1 mux*/
    if (onlp_i2c_writeb(I2C_BUS, mux_1st, offset, reg, ONLP_I2C_F_FORCE) < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /*Open only 1 channel on that PIM.*/
    index = sfpi_get_i2cmux_mapping(port);
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


/*---------------Public APIs------------------------*/
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
        return present;
    }
    if (!present) {
        update_ports(pim, 0, 0);
        return 0;
    }
    ret = get_pim_port_present_bmap(port, &bit_array);
    if (ret < 0) {
        return ret;
    }

    return !!(bit_array & BIT(port % SFP_PORT_PER_PIM));
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
        ret = get_pim_port_present_bmap(port, &bmap_pim[i]);
        if (ret < 0)
            return ret;
    }

    for (i = 0; i < NUM_OF_SFP_PORT; i++) {
        AIM_BITMAP_CLR(dst, i);
    }
    for (i = 0; i < NUM_OF_SFP_PORT; i++) {
        bmp  = bmap_pim[i/SFP_PORT_PER_PIM];
        if ((bmp & BIT(i%SFP_PORT_PER_PIM))) {
            AIM_BITMAP_SET(dst, i);
        }
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    AIM_BITMAP_CLR_ALL(dst);
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
    uint32_t present;

    SEM_LOCK;
    onlp_read_pim_present(&present);

    for (i = 0; i < channels; i++) {
        if (!(present & BIT(i)))
            continue;

        value = BIT(i);
        /*Open only 1 channel of level-1 mux*/
        if (onlp_i2c_writeb(I2C_BUS, mux_1st, offset, value, ONLP_I2C_F_FORCE) < 0) {
            SEM_UNLOCK;
            return ONLP_STATUS_E_INTERNAL;
        }
        /*Close mux on each PIM.*/
        for (k = 0; k < AIM_ARRAYSIZE(mux_2st); k++) {
            if (onlp_i2c_writeb(I2C_BUS, mux_2st[k], offset, 0, ONLP_I2C_F_FORCE) < 0) {
                DEBUG_PRINT("Unable to write to I2C slave(0x%x)", mux_2st[k]);
            }
        }
    }

    /*close level-1 mux*/
    if (onlp_i2c_writeb(I2C_BUS, mux_1st, offset, 0, ONLP_I2C_F_FORCE) < 0) {
        SEM_UNLOCK;
        return ONLP_STATUS_E_INTERNAL;
    }
    SEM_UNLOCK;
    return ONLP_STATUS_OK;
}

/* Due to PIM can be hot swapped, here the eeprom driver is always at root bus.
 * To avoid multi-slave condition, only 1 channel is opened on reading.
 */
int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    FILE* fp;
    int ret, bytes;
    char file[64] = {0};

    bytes = 256;
    SEM_LOCK;
    ret = sfpi_eeprom_channel_open(port);
    if (ret != ONLP_STATUS_OK) {
        DEBUG_PRINT("Unable to set i2c channel for the module_eeprom of port(%d, %d)", port, ret);
        goto exit;
    }

    sprintf(file, PORT_EEPROM_FORMAT, I2C_BUS);
    fp = fopen(file, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the eeprom device file of port(%d)", port);
        ret = ONLP_STATUS_E_INTERNAL;
        goto exit;
    }
    ret = fread(data, 1, bytes, fp);
    fclose(fp);
    if (ret != bytes) {
        ret = ONLP_STATUS_E_INTERNAL;
        goto exit;
    }
    ret = ONLP_STATUS_OK;
exit:
    SEM_UNLOCK;
    return ret;
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

