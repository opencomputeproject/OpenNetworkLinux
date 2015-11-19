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
#include <onlplib/i2c.h>

#if ONLPLIB_CONFIG_INCLUDE_I2C == 1

#include <onlplib/file.h>
#include <fcntl.h>
#include <unistd.h>

#if ONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER == 1
#include <linux/i2c-devices.h>
#else
#include <linux/i2c-dev.h>
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <onlp/onlp.h>
#include "onlplib_log.h"

int
onlp_i2c_open(int bus, uint8_t addr, uint32_t flags)
{
    int fd;
    int rv;

    fd = onlp_file_open(O_RDWR, 1, "/dev/i2c-%d", bus);
    if(fd < 0) {
        return fd;
    }

    /* Set 10 or 7 bit mode */
    rv = ioctl(fd, I2C_TENBIT, (flags & ONLP_I2C_F_TENBIT) ? 1 : 0);
    if(rv != 0) {
        AIM_LOG_ERROR("i2c-%d: failed to set %d bit mode",
                      (flags & ONLP_I2C_F_TENBIT) ? 10 : 7);
        goto error;
    }

    /* Enable/Disable PEC */
    rv = ioctl(fd, I2C_PEC, (flags & ONLP_I2C_F_PEC) ? 1 : 0);
    if(rv != 0) {
        AIM_LOG_ERROR("i2c-%d: failed to set PEC mode %d",
                      (flags & ONLP_I2C_F_PEC) ? 1 : 0);
        goto error;
    }

    /* Set SLAVE or SLAVE_FORCE address */
    rv = ioctl(fd,
               (flags & ONLP_I2C_F_FORCE) ? I2C_SLAVE_FORCE : I2C_SLAVE,
               addr);

    if(rv != 0) {
        AIM_LOG_ERROR("i2c-%d: %s slave address 0x%x failed: %{errno}",
                      bus,
                      (flags & ONLP_I2C_F_FORCE) ? "forcing" : "setting",
                      errno);
        goto error;
    }

    return fd;

 error:
    close(fd);
    return -1;
}

int
onlp_i2c_block_read(int bus, uint8_t addr, uint8_t offset, int size,
                    uint8_t* rdata, uint32_t flags)
{
    int fd;

    fd = onlp_i2c_open(bus, addr, flags);

    if(fd < 0) {
        return fd;
    }

    int count = size;
    uint8_t* p = rdata;
    while(count > 0) {
        int rsize = (count >= ONLPLIB_CONFIG_I2C_BLOCK_SIZE) ? ONLPLIB_CONFIG_I2C_BLOCK_SIZE : count;
        int rv = i2c_smbus_read_i2c_block_data(fd,
                                               p - rdata,
                                               rsize,
                                               p);

        if(rv != rsize) {
            AIM_LOG_ERROR("i2c-%d: reading address 0x%x, offset %d, size=%d failed: %{errno}",
                          bus, addr, p - rdata, rsize, errno);
            goto error;
        }

        p += rsize;
        count -= rsize;
    }

    close(fd);
    return 0;

 error:
    close(fd);
    return -1;
}

int
onlp_i2c_read(int bus, uint8_t addr, uint8_t offset, int size,
              uint8_t* rdata, uint32_t flags)
{
    int i;
    int fd;

    fd = onlp_i2c_open(bus, addr, flags);

    if(fd < 0) {
        return fd;
    }

    for(i = 0; i < size; i++) {
        uint32_t rv = i2c_smbus_read_byte_data(fd, offset+i);
        if(rv < 0) {
            AIM_LOG_ERROR("i2c-%d: reading address 0x%x, offset %d failed: %{errno}",
                          bus, addr, offset+i, errno);
            goto error;
        }
        else {
            rdata[i] = rv;
        }
    }
    close(fd);
    return 0;

 error:
    close(fd);
    return -1;
}


int
onlp_i2c_write(int bus, uint8_t addr, uint8_t offset, int size,
               uint8_t* data, uint32_t flags)
{
    int i;
    int fd;

    fd = onlp_i2c_open(bus, addr, flags);

    if(fd < 0) {
        return fd;
    }

    for(i = 0; i < size; i++) {
        uint32_t rv = i2c_smbus_write_byte_data(fd, offset+i, data[i]);
        if(rv < 0) {
            AIM_LOG_ERROR("i2c-%d: writing address 0x%x, offset %d failed: %{errno}",
                          bus, addr, offset+i, errno);
            goto error;
        }
    }
    close(fd);
    return 0;

 error:
    close(fd);
    return -1;
}

int
onlp_i2c_readb(int bus, uint8_t addr, uint8_t offset, uint32_t flags)
{
    uint8_t byte;
    int rv = onlp_i2c_read(bus, addr, offset, 1, &byte, flags);
    return (rv < 0) ? rv : byte;
}

int
onlp_i2c_writeb(int bus, uint8_t addr, uint8_t offset, uint8_t byte,
                uint32_t flags)
{
    return onlp_i2c_write(bus, addr, offset, 1, &byte, flags);
}

int
onlp_i2c_modifyb(int bus, uint8_t addr, uint8_t offset,
                 uint8_t andmask, uint8_t ormask, uint32_t flags)
{
    int v;
    ONLP_IF_ERROR_RETURN(v=onlp_i2c_readb(bus, addr, offset, flags));
    v &= andmask;
    v |= ormask;
    return onlp_i2c_writeb(bus, addr, offset, v, flags);
}


int
onlp_i2c_readw(int bus, uint8_t addr, uint8_t offset, uint32_t flags)
{
    int fd;
    int rv;

    fd = onlp_i2c_open(bus, addr, flags);

    if(fd < 0) {
        return fd;
    }

    rv = i2c_smbus_read_word_data(fd, offset);

    close(fd);
    return rv;
}

int
onlp_i2c_writew(int bus, uint8_t addr, uint8_t offset, uint16_t word,
                    uint32_t flags)
{
    int fd;
    int rv;

    fd = onlp_i2c_open(bus, addr, flags);

    if(fd < 0) {
        return fd;
    }

    rv = i2c_smbus_write_word_data(fd, offset, word);

    close(fd);
    return rv;

}

#endif /* ONLPLIB_CONFIG_INCLUDE_I2C */
