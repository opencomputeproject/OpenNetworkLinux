#ifndef __QUANTA_LIB_I2C_H__
#define __QUANTA_LIB_I2C_H__

int i2c_block_read(int bus, uint8_t addr, uint8_t offset, int size,
                    uint8_t* rdata, uint32_t flags);

#endif /* __QUANTA_LIB_I2C_H__ */
