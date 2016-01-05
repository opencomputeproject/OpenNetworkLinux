#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <quanta_lib/i2c.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>

/* first byte of rdata is length of result */
int
i2c_block_read(int bus, uint8_t addr, uint8_t offset, int size,
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
                                               offset,
                                               rsize,
                                               p);

        if(rv != rsize) {
            printf("i2c-%d: reading address 0x%x, offset %d, size=%d failed",
                          bus, addr, offset, rsize);
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

