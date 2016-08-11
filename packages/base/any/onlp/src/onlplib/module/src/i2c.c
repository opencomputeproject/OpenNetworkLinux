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
    if(rv == -1) {
        AIM_LOG_ERROR("i2c-%d: failed to set %d bit mode", bus,
                      (flags & ONLP_I2C_F_TENBIT) ? 10 : 7);
        goto error;
    }

    /* Enable/Disable PEC */
    rv = ioctl(fd, I2C_PEC, (flags & ONLP_I2C_F_PEC) ? 1 : 0);
    if(rv == -1) {
        AIM_LOG_ERROR("i2c-%d: failed to set PEC mode %d", bus,
                      (flags & ONLP_I2C_F_PEC) ? 1 : 0);
        goto error;
    }

    /* Set SLAVE or SLAVE_FORCE address */
    rv = ioctl(fd,
               (flags & ONLP_I2C_F_FORCE) ? I2C_SLAVE_FORCE : I2C_SLAVE,
               addr);

    if(rv == -1) {
        AIM_LOG_ERROR("i2c-%d: %s slave address 0x%x failed: %{errno}",
                      bus,
                      (flags & ONLP_I2C_F_FORCE) ? "forcing" : "setting",
                      addr,
                      errno);
        goto error;
    }

    return fd;

 error:
    close(fd);
    return ONLP_STATUS_E_I2C;
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
    return ONLP_STATUS_E_I2C;
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
        int rv = i2c_smbus_read_byte_data(fd, offset+i);
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
    return ONLP_STATUS_E_I2C;
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
        int rv = i2c_smbus_write_byte_data(fd, offset+i, data[i]);
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
    return ONLP_STATUS_E_I2C;
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

int
onlp_i2c_mux_select(onlp_i2c_mux_device_t* dev, int channel)
{
    int i;
    for(i = 0; i < AIM_ARRAYSIZE(dev->driver->channels); i++) {
        if(dev->driver->channels[i].channel == channel) {
            AIM_LOG_VERBOSE("i2c_mux_select: Selecting channel %2d on device '%s'  [ bus=%d addr=0x%x offset=0x%x value=0x%x ]...",
                        channel, dev->name, dev->bus, dev->devaddr,
                        dev->driver->control, dev->driver->channels[i].value);

            int rv = onlp_i2c_writeb(dev->bus,
                                     dev->devaddr,
                                     dev->driver->control,
                                     dev->driver->channels[i].value,
                                     0);

            if(rv < 0) {
                AIM_LOG_ERROR("i2c_mux_select: Selecting channel %2d on device '%s'  [ bus=%d addr=0x%x offset=0x%x value=0x%x ] failed: %d",
                              channel, dev->name, dev->bus, dev->devaddr,
                              dev->driver->control, dev->driver->channels[i].value, rv);
            }
            return rv;
        }
    }
    return ONLP_STATUS_E_PARAM;
}


int
onlp_i2c_mux_deselect(onlp_i2c_mux_device_t* dev)
{
    return onlp_i2c_mux_select(dev, -1);
}


int
onlp_i2c_mux_channel_select(onlp_i2c_mux_channel_t* mc)
{
    return onlp_i2c_mux_select(mc->mux, mc->channel);
}


int
onlp_i2c_mux_channel_deselect(onlp_i2c_mux_channel_t* mc)
{
    return onlp_i2c_mux_deselect(mc->mux);
}


int
onlp_i2c_mux_channels_select(onlp_i2c_mux_channels_t* mcs)
{
    int i;
    for(i = 0; i < AIM_ARRAYSIZE(mcs->channels); i++) {
        if(mcs->channels[i].mux) {
            int rv = onlp_i2c_mux_channel_select(mcs->channels + i);
            if(rv < 0) {
                /** Error already reported */
                return rv;
            }
        }
    }
    return 0;
}


int
onlp_i2c_mux_channels_deselect(onlp_i2c_mux_channels_t* mcs)
{
    int i;
    for(i = AIM_ARRAYSIZE(mcs->channels) - 1; i >= 0; i--) {
        if(mcs->channels[i].mux) {
            int rv = onlp_i2c_mux_channel_deselect(mcs->channels + i);
            if(rv < 0) {
                /** Error already reported. */
                return rv;
            }
        }
    }
    return 0;
}


int
onlp_i2c_dev_mux_channels_select(onlp_i2c_dev_t* dev)
{
    if(dev->pchannels) {
        return onlp_i2c_mux_channels_select(dev->pchannels);
    }
    else {
        return onlp_i2c_mux_channels_select(&dev->ichannels);
    }
}


int
onlp_i2c_dev_mux_channels_deselect(onlp_i2c_dev_t* dev)
{
    if(dev->pchannels) {
        return onlp_i2c_mux_channels_deselect(dev->pchannels);
    }
    else {
        return onlp_i2c_mux_channels_deselect(&dev->ichannels);
    }
}


static int
dev_mux_channels_select__(onlp_i2c_dev_t* dev, uint32_t flags)
{
    if(flags & ONLP_I2C_F_NO_MUX_SELECT) {
        return 0;
    }
    return onlp_i2c_dev_mux_channels_select(dev);
}

static int
dev_mux_channels_deselect__(onlp_i2c_dev_t* dev, uint32_t flags)
{
    if(flags & ONLP_I2C_F_NO_MUX_DESELECT) {
        return 0;
    }
    return onlp_i2c_dev_mux_channels_deselect(dev);
}


int
onlp_i2c_dev_read(onlp_i2c_dev_t* dev, uint8_t offset, int size,
                  uint8_t* rdata, uint32_t flags)
{
    int error, rv;

    if( (error = dev_mux_channels_select__(dev, flags)) < 0) {
        return error;
    }

    if( (rv = onlp_i2c_read(dev->bus, dev->addr, offset, size, rdata, flags)) < 0) {
        AIM_LOG_ERROR("Device %s: read() failed: %d",
                      dev->name, rv);
        return rv;
    }

    if( (error = dev_mux_channels_deselect__(dev, flags)) < 0) {
        return error;
    }

    return rv;
}


int
onlp_i2c_dev_write(onlp_i2c_dev_t* dev,
                   uint8_t offset, int size, uint8_t* data, uint32_t flags)
{
    int error, rv;

    if( (error = dev_mux_channels_select__(dev, flags)) < 0) {
        return error;
    }

    if( (rv = onlp_i2c_write(dev->bus, dev->addr, offset, size, data, flags)) < 0) {
        AIM_LOG_ERROR("Device %s: write() failed: %d",
                      dev->name, rv);
        return rv;
    }

    if( (error = dev_mux_channels_deselect__(dev, flags)) < 0) {
        return error;
    }

    return rv;
}


int
onlp_i2c_dev_readb(onlp_i2c_dev_t* dev, uint8_t offset, uint32_t flags)
{
    int error, rv;

    if( (error = dev_mux_channels_select__(dev, flags)) < 0) {
        return error;
    }

    if( (rv = onlp_i2c_readb(dev->bus, dev->addr, offset, flags)) < 0) {
        AIM_LOG_ERROR("Device %s: readb() failed: %d",
                      dev->name, rv);
        return rv;
    }

    if( (error = dev_mux_channels_deselect__(dev, flags)) < 0) {
        return error;
    }

    return rv;
}


int
onlp_i2c_dev_writeb(onlp_i2c_dev_t* dev,
                    uint8_t offset, uint8_t byte, uint32_t flags)
{
    int error, rv;

    if( (error = dev_mux_channels_select__(dev, flags)) < 0) {
        return error;
    }

    if( (rv = onlp_i2c_writeb(dev->bus, dev->addr, offset, byte, flags)) < 0) {
        AIM_LOG_ERROR("Device %s: writeb() failed: %d",
                      dev->name, rv);
        return rv;
    }

    if( (error = dev_mux_channels_deselect__(dev, flags)) < 0) {
        return error;
    }

    return rv;
}


int
onlp_i2c_dev_readw(onlp_i2c_dev_t* dev,
                   uint8_t offset, uint32_t flags)
{
    int error, rv;

    if( (error = dev_mux_channels_select__(dev, flags)) < 0) {
        return error;
    }

    if( (rv = onlp_i2c_readw(dev->bus, dev->addr, offset, flags)) < 0) {
        AIM_LOG_ERROR("Device %s: readw() failed: %d",
                      dev->name, rv);
        return rv;
    }

    if( (error = dev_mux_channels_deselect__(dev, flags)) < 0) {
        return error;
    }

    return rv;
}


int
onlp_i2c_dev_writew(onlp_i2c_dev_t* dev,
                        uint8_t offset, uint16_t word, uint32_t flags)
{
    int error, rv;

    if( (error = dev_mux_channels_select__(dev, flags)) < 0) {
        return error;
    }

    if( (rv = onlp_i2c_writew(dev->bus, dev->addr, offset, word, flags)) < 0) {
        AIM_LOG_ERROR("Device %s: writew() failed: %d",
                      dev->name, rv);
        return rv;
    }

    if( (error = dev_mux_channels_deselect__(dev, flags)) < 0) {
        return error;
    }

    return rv;
}

/**
 * PCA9547A
 */
onlp_i2c_mux_driver_t onlp_i2c_mux_driver_pca9547a =
    {
        .name = "PCA9547A",
        .control = 0,
        .channels =
        {
            { -1,   0 },
            {  0, 0x8 },
            {  1, 0x9 },
            {  2, 0xA },
            {  3, 0xB },
            {  4, 0xC },
            {  5, 0xD },
            {  6, 0xE },
            {  7, 0xF },
        },
    };

/**
 * PCA9548
 */
onlp_i2c_mux_driver_t onlp_i2c_mux_driver_pca9548 =
    {
        .name = "PCA9548A",
        .control = 0,
        .channels =
        {
            { -1, 0x00 },
            {  0, 0x01 },
            {  1, 0x02 },
            {  2, 0x04 },
            {  3, 0x08 },
            {  4, 0x10 },
            {  5, 0x20 },
            {  6, 0x40 },
            {  7, 0x80 },
        },
    };


#endif /* ONLPLIB_CONFIG_INCLUDE_I2C */
