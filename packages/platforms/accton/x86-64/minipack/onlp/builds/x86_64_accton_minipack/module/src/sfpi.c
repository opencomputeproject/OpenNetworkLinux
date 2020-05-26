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
#include <sys/mman.h>
#include <fcntl.h>
#include "platform_lib.h"
#include "x86_64_accton_minipack_log.h"

/* PIM stands for "Port Interface Module".
 * For minipack, there are hot-pluggable 8 PIMs.
 * Each PIM can have 16*16Q or 4*4DD ports.
 */
#define NUM_OF_PIM              (PLATFOTM_NUM_OF_PIM)
#define TYPES_OF_PIM            (2)
#define NUM_OF_SFP_PORT         (128)
#define SFP_PORT_PER_PIM        (NUM_OF_SFP_PORT/NUM_OF_PIM)
#define I2C_BUS                 (1)
#define PIM_POLL_INTERVAL       (5) /*in seconds*/
#define PORT_POLL_INTERVAL      (8) /*per PIM, in seconds*/
#define PORT_EEPROM_FORMAT      "/sys/bus/i2c/devices/%d-0050/eeprom"

#define PORT_TO_PIM(_port)        (_port / SFP_PORT_PER_PIM)
#define PORT_OF_PIM(_port)        (_port % SFP_PORT_PER_PIM)

#define NUM_I2C_MUX_ON_PIM      2
static const int muxAddrOnPIM[NUM_I2C_MUX_ON_PIM] = {0x72, 0x71};
static const int muxAddrRoot = 0x70;

typedef struct {
    bool      valid;
    time_t    last_poll;
    uint32_t  present;
} present_status_t;

typedef struct {
    present_status_t pim;
    present_status_t port_at_pim[NUM_OF_SFP_PORT/NUM_OF_PIM];
    int root_muxReg;
    int pim_muxReg[NUM_OF_PIM][NUM_I2C_MUX_ON_PIM];
} sfpi_port_status_t;

int onlp_read_pim_present(uint32_t *bmap);
static int get_ports_presence(uint32_t pimId, uint32_t *pbmp);
static int get_ports_lpmode(uint32_t pimId, uint32_t *pbmp);
static int get_ports_reset(uint32_t pimId, uint32_t *pbmp);
static int set_ports_lpmode(uint32_t pimId, uint32_t value);
static int set_ports_reset(uint32_t pimId, uint32_t value);


#define SEM_LOCK    do { \
            onlp_shlock_take(g_sfpiLock);} while(0)

#define SEM_UNLOCK    do { \
            onlp_shlock_give(g_sfpiLock);} while(0)

static sfpi_port_status_t *g_sfpiPortStat = NULL;
static onlp_shlock_t* g_sfpiLock = NULL;
/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
static int sfpi_create_shm(key_t id) {
    int rv = ONLP_STATUS_OK;

    if(g_sfpiLock == NULL) {
        if(onlp_shlock_create(ONLP_SFPI_SHLOCK_KEY, &g_sfpiLock,
                              "onlp-sfpi-lock") < 0) {
            AIM_DIE("onlp-sfpi lock created failed.");
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    if (g_sfpiPortStat == NULL) {
        rv = onlp_shmem_create(id, sizeof(sfpi_port_status_t),
                               (void**)&g_sfpiPortStat);
        if (rv < 0) {
            AIM_DIE("Global %s created failed.", __func__);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    return rv;
}

static int update_ports(int pim, bool valid, uint32_t present) {
    present_status_t *ports;

    SEM_LOCK;
    ports = &g_sfpiPortStat->port_at_pim[pim];
    ports->valid = valid;
    ports->present = present;
    ports->last_poll = time (NULL);

    SEM_UNLOCK;
    return ONLP_STATUS_OK;
}

void onlp_sfpi_sleep(uint8_t sec)
{
    onlp_sfpi_init();
    SEM_LOCK;
    sleep(sec);
    SEM_UNLOCK;
}

int onlp_sfpi_init(void)
{
    int i, j;

    int rv = sfpi_create_shm(ONLP_SFPI_SHM_KEY);
    if (rv < 0) {
        AIM_DIE("onlp_sfpi_init::sfpi_create_shm created failed.");
        return ONLP_STATUS_E_INTERNAL;
    }

    if (rv == 1) { /* shared memory was newly created*/
        /*Clear cache for muxes on PIM */
        SEM_LOCK;
        for (i = 0; i < NUM_OF_PIM; i++) {
            for (j = 0; j < NUM_I2C_MUX_ON_PIM; j++) {
                g_sfpiPortStat->pim_muxReg[i][j] = -1;
            }
        }
        g_sfpiPortStat->root_muxReg = -1;
        SEM_UNLOCK;
    }

    /* Unleash the Reset pin again.
     * It might be unleashed too early for some types of transcievers.
     */
    for (i = 0; i < NUM_OF_PIM; i++) {
        set_ports_reset(i, 0);
    }
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
    return value;
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
    ps = g_sfpiPortStat;
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

    pim = PORT_TO_PIM(port);
    /*If PIM not present, set all 0's to pbmap.*/
    if(onlp_pim_is_present(pim) == 0) {
        present  = 0;
        update_ports(pim, 0, present);
        *bit_array = _sfpi_port_present_remap_reg(present);
        return ONLP_STATUS_OK;
    }

    ports = &g_sfpiPortStat->port_at_pim[pim];
    cur = time (NULL);
    elapse = cur - ports->last_poll;

    if (!ports->valid || (elapse > PORT_POLL_INTERVAL)) {
        ret = get_ports_presence(pim, &present);
        if (ret < 0) {
            present  = 0;
            update_ports(pim, 0, present);
            *bit_array = present;        /*No needs for remmaped.*/
            return ONLP_STATUS_E_INTERNAL;
        } else {
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


static int i2c_writebF(uint8_t addr, uint8_t offset, uint8_t byte)
{
    return onlp_i2c_writeb(I2C_BUS,  addr,  offset,  byte, ONLP_I2C_F_FORCE);
}

/*Set the i2c mux of PIM to open channel to that port.*/
static int
sfpi_eeprom_channel_open(int port)
{
    uint32_t pim, reg, i, index;
    int offset = 0;

    pim = PORT_TO_PIM(port);
    reg = BIT(pim);
    if (g_sfpiPortStat->root_muxReg != reg) {
        if (i2c_writebF(muxAddrRoot, offset, reg) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
        g_sfpiPortStat->root_muxReg = reg;
    }
    /*Open only 1 channel on that PIM.*/
    index = sfpi_get_i2cmux_mapping(port);
    for (i = 0; i < NUM_I2C_MUX_ON_PIM; i++) {
        if ((index/8) != i) {
            reg = 0;
        } else {
            reg = BIT(index%8);
        }

        if (g_sfpiPortStat->pim_muxReg[pim][i] != reg) {
            if (i2c_writebF(muxAddrOnPIM[i], offset, reg) < 0) {
                return ONLP_STATUS_E_INTERNAL;
            }
            g_sfpiPortStat->pim_muxReg[pim][i] = reg;
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

    pim = PORT_TO_PIM(port);
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

    return !!(bit_array & BIT(PORT_OF_PIM(port)));
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
        bmp  = bmap_pim[PORT_TO_PIM(i)];
        if ((bmp & BIT(PORT_OF_PIM(i)))) {
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

/* Due to PIM can be hot swapped, here the eeprom driver is always at root bus.
 * To avoid multi-slave condition, only 1 channel is opened on reading.
 */

static int st_sfpi_eeprom_read(int port, uint8_t data[256], int foffset)
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

    if (fseek(fp, foffset, SEEK_CUR) != 0) {
        fclose(fp);
        AIM_LOG_ERROR("Unable to set the file position indicator of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
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
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    return st_sfpi_eeprom_read(port, data, 0);
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    return st_sfpi_eeprom_read(port, data, 256);
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int ret;

    SEM_LOCK;
    ret = sfpi_eeprom_channel_open(port);
    if (ret != ONLP_STATUS_OK) {
        DEBUG_PRINT("Unable to set i2c channel for the module_eeprom of port(%d, %d)", port, ret);
        goto exit;
    }
    ret = onlp_i2c_readb(I2C_BUS, devaddr, addr, ONLP_I2C_F_FORCE);
exit:
    SEM_UNLOCK;
    return ret;
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int ret;

    SEM_LOCK;
    ret = sfpi_eeprom_channel_open(port);
    if (ret != ONLP_STATUS_OK) {
        DEBUG_PRINT("Unable to set i2c channel for the module_eeprom of port(%d, %d)", port, ret);
        goto exit;
    }
    ret = i2c_writebF(devaddr, addr, value);
exit:
    SEM_UNLOCK;
    return ret;
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int ret;

    SEM_LOCK;
    ret = sfpi_eeprom_channel_open(port);
    if (ret != ONLP_STATUS_OK) {
        DEBUG_PRINT("Unable to set i2c channel for the module_eeprom of port(%d, %d)", port, ret);
        goto exit;
    }
    ret = onlp_i2c_readw(I2C_BUS, devaddr, addr, ONLP_I2C_F_FORCE);
exit:
    SEM_UNLOCK;
    return ret;
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int ret;

    SEM_LOCK;
    ret = sfpi_eeprom_channel_open(port);
    if (ret != ONLP_STATUS_OK) {
        DEBUG_PRINT("Unable to set i2c channel for the module_eeprom of port(%d, %d)", port, ret);
        goto exit;
    }
    ret = onlp_i2c_writew(I2C_BUS, devaddr, addr, value, ONLP_I2C_F_FORCE);
exit:
    SEM_UNLOCK;
    return ret;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    uint32_t pbmp;
    int rv = ONLP_STATUS_OK;
    if (port < 0) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
    {
    case ONLP_SFP_CONTROL_RESET:
        rv = get_ports_reset(PORT_TO_PIM(port), &pbmp);
        if (rv < 0) {
            AIM_LOG_ERROR("Unable to get_ports_lpmode for port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        if (value) {
            pbmp |= BIT(PORT_OF_PIM(port));
        } else {
            pbmp &= ~BIT(PORT_OF_PIM(port));
        }
        rv = set_ports_reset(PORT_TO_PIM(port), pbmp);
        if (rv < 0) {
            AIM_LOG_ERROR("Unable to set_ports_lpmode for port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        break;
    case ONLP_SFP_CONTROL_LP_MODE:
        rv = get_ports_lpmode(PORT_TO_PIM(port), &pbmp);
        if (rv < 0) {
            AIM_LOG_ERROR("Unable to get_ports_lpmode for port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        if(value) {
            pbmp |= BIT(PORT_OF_PIM(port));
        } else {
            pbmp &= ~BIT(PORT_OF_PIM(port));
        }
        rv = set_ports_lpmode(PORT_TO_PIM(port), pbmp);
        if (rv < 0) {
            AIM_LOG_ERROR("Unable to set_ports_lpmode for port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        break;
    default:
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }

    return rv;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    uint32_t pbmp;
    int rv = ONLP_STATUS_OK;
    if (port < 0) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
    {
    case ONLP_SFP_CONTROL_RESET:
        rv = get_ports_reset(PORT_TO_PIM(port), &pbmp);
        if (rv < 0) {
            AIM_LOG_ERROR("Unable to get_ports_reset for port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        else {
            *value = !(pbmp & BIT(PORT_OF_PIM(port)));
            rv = ONLP_STATUS_OK;
        }
        break;

    case ONLP_SFP_CONTROL_LP_MODE:
        rv = get_ports_lpmode(PORT_TO_PIM(port), &pbmp);
        if (rv < 0) {
            AIM_LOG_ERROR("Unable to get_ports_lpmode for port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        else {
            *value = !!(pbmp & BIT(PORT_OF_PIM(port)));
            rv = ONLP_STATUS_OK;
        }
        break;

    default:
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }

    return rv;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}


/*Access to FPGA register.*/
#define FPGA_RESOURCE_NODE "/sys/devices/pci0000:00/0000:00:03.0/0000:05:00.0/resource0"
#define FPGA_RESOURCE_LENGTH 0x80000
static int hw_handle = -1;
static void *io_base = NULL;

static int fbfpgaio_hw_init(void)
{
    const char fpga_resource_node[] = FPGA_RESOURCE_NODE;

    if (io_base != NULL && io_base != MAP_FAILED) {
        return ONLP_STATUS_OK;
    }

    /* Open hardware resource node */
    hw_handle = open(fpga_resource_node, O_RDWR|O_SYNC);
    if (hw_handle == -1) {
        AIM_LOG_ERROR("%d %s\\n",errno,strerror(errno));
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Mapping hardware resource */
    io_base = mmap(NULL, FPGA_RESOURCE_LENGTH, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_NORESERVE, hw_handle, 0);
    if (io_base == MAP_FAILED) {
        AIM_LOG_ERROR("%d %s\\n",errno,strerror(errno));
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

static uint32_t fbfpgaio_write(uint32_t addr, uint32_t input_data)
{
    int ret = fbfpgaio_hw_init();

    if (ONLP_STATUS_OK != ret) {
        return 0;
    }

    unsigned int *address =  (unsigned int *) ((unsigned long) io_base
                             + (unsigned long) addr);
    unsigned int data = (unsigned int) (input_data & 0xFFFFFFFF);
    *address = data;

    return ONLP_STATUS_OK;
}

static uint32_t fbfpgaio_read(uint32_t addr)
{
    int ret = fbfpgaio_hw_init();

    if (ONLP_STATUS_OK != ret) {
        return 0;
    }

    void *offset = io_base + addr;
    return *(uint32_t*)offset;
}

#define IOB_PIM_STATUS_REG 0x40

static uint32_t dom_offset[] = {
    0x40000,
    0x48000,
    0x50000,
    0x58000,
    0x60000,
    0x68000,
    0x70000,
    0x78000,
};
#define QSFP_PRESENT_REG    0x48
#define QSFP_RESET_REG      0x70
#define QSFP_LPMODE_REG     0x78

int onlp_read_pim_present(uint32_t *pbmp) {
    uint32_t pim_status = fbfpgaio_read(IOB_PIM_STATUS_REG );
    *pbmp = (pim_status >> 16); /*bit 23~16*/
    return ONLP_STATUS_OK;
}

static int read_pom_reg(uint32_t pimId, uint32_t reg, uint32_t *pbmp) {
    if (pimId >= AIM_ARRAYSIZE(dom_offset)) {
        return ONLP_STATUS_E_INTERNAL;
    }
    uint32_t addr = dom_offset[pimId] + reg;
    *pbmp = fbfpgaio_read(addr);
    return ONLP_STATUS_OK;
}

static int get_ports_presence(uint32_t pimId, uint32_t *pbmp) {
    return read_pom_reg(pimId, QSFP_PRESENT_REG, pbmp);
}

static int get_ports_lpmode(uint32_t pimId, uint32_t *pbmp) {
    read_pom_reg(pimId, QSFP_RESET_REG, pbmp);
    return read_pom_reg(pimId, QSFP_LPMODE_REG, pbmp);
}

static int get_ports_reset(uint32_t pimId, uint32_t *pbmp) {
    return read_pom_reg(pimId, QSFP_RESET_REG, pbmp);
}

static int write_pom_reg(uint32_t pimId, uint32_t reg, uint32_t value) {
    if (pimId >= AIM_ARRAYSIZE(dom_offset)) {
        return ONLP_STATUS_E_INTERNAL;
    }

    uint32_t addr = dom_offset[pimId] + reg;
    return fbfpgaio_write(addr, value);
}

static int set_ports_lpmode(uint32_t pimId, uint32_t value) {
    return write_pom_reg(pimId, QSFP_LPMODE_REG, value);
}

static int set_ports_reset(uint32_t pimId, uint32_t value) {
    return write_pom_reg(pimId, QSFP_RESET_REG, value);
}

