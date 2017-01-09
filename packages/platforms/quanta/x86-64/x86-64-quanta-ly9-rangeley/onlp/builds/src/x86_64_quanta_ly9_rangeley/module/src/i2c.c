#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <quanta_lib/i2c.h>
#include <fcntl.h>
#include <unistd.h>
#if ONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER == 1
#include <linux/i2c-devices.h>
#else
#include <linux/i2c-dev.h>
#endif
#include <sys/types.h>
#include <sys/ioctl.h>
#include "x86_64_quanta_ly9_rangeley_log.h"
#include <AIM/aim_printf.h>
#include <errno.h>

int i2c_dev_smbus_access(int file, char read_write, __u8 command,
                         int size, union i2c_smbus_data *data)
{
    struct i2c_smbus_ioctl_data args;

    args.read_write = read_write;
    args.command = command;
    args.size = size;
    args.data = data;
    return ioctl(file, I2C_SMBUS, &args);
}

int i2c_dev_read_block(int file, uint8_t command, unsigned char *buffer, int length)
{
    union i2c_smbus_data data;
    int i;

    if (length > 32)
        length = 32;
    data.block[0] = length;
    if (i2c_dev_smbus_access(file, I2C_SMBUS_READ, command,
                             length == 32 ? I2C_SMBUS_I2C_BLOCK_BROKEN :
                             I2C_SMBUS_I2C_BLOCK_DATA, &data))
        return -1;
    else{
        for (i = 1; i <= data.block[0]; i++)
            buffer[i - 1] = data.block[i];
    }
    return 0;
}

/* first byte of rdata is length of result */
int
i2c_block_read(int bus, uint8_t addr, uint8_t offset, int size,
                    unsigned char *rdata, uint32_t flags)
{
    int fd;
    int force = I2C_SLAVE, rv;

    fd = onlp_i2c_open(bus, addr, flags);

    if(fd < 0) {
        return fd;
    }

    fd = onlp_file_open(O_RDWR, 1, "/dev/i2c-%d", bus);
    if(fd < 0) {
        AIM_LOG_MSG("/dev/i2c-%d open Error!", bus);
        return fd;
    }

    /* Set SLAVE or SLAVE_FORCE address */
    rv = ioctl(fd,
               force ? I2C_SLAVE_FORCE : I2C_SLAVE,
               addr);

    if(rv == -1) {
        AIM_LOG_ERROR("i2c-%d: %s slave address 0x%x failed: %{errno}",
                      bus,
                      (flags & ONLP_I2C_F_FORCE) ? "forcing" : "setting",
                      addr,
                      errno);
        goto error;
    }

    if ((i2c_dev_read_block(fd, offset, rdata, size)) < 0)
    {
        printf("/dev/i2c-%d read Error!\n", bus);
        close (fd);
        return -1;
    }

    close(fd);
    return 0;

 error:
    close(fd);
    return -1;
}

