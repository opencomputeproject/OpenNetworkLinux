/************************************************************
 * <bsn.cl v=2014 v=onl>
 *
 *           Copyright 2015 Big Switch Networks, Inc.
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
 * Common I2C processing for all platform implementations.
 *
 ***********************************************************/
#ifndef __ONLP_I2C_H__
#define __ONLP_I2C_H__

#include <onlplib/onlplib_config.h>

#if ONLPLIB_CONFIG_INCLUDE_I2C == 1

/**
 * Use TENBIT mode. Default is to disable TENBIT mode.
 */
#define ONLP_I2C_F_TENBIT 0x1

/**
 * Use SLAVE_FORCE instead of SLAVE when setting the
 * i2c slave address.
 */
#define ONLP_I2C_F_FORCE 0x2

/**
 * Enable PEC.
 */
#define ONLP_I2C_F_PEC 0x4

/**
 * @brief Open and prepare for reading or writing.
 * @param bus The i2c bus number.
 * @param addr The slave address.
 * @param flags See ONLP_I2C_F_*
 * @note Normal applications will not use this function directly.
 */
int onlp_i2c_open(int bus, uint8_t addr, uint32_t flags);


/**
 * @brief Read i2c data.
 * @param bus The i2c bus number.
 * @param addr The slave address.
 * @param offset The byte offset.
 * @param size The byte count.
 * @param rdata [out] Receives the data.
 * @param flags See ONLP_I2C_F_*
 * @note This function reads a byte at a time.
 * See onlp_i2c_read_block() for block reads.
 */

int onlp_i2c_read(int bus, uint8_t addr, uint8_t offset, int size,
                  uint8_t* rdata, uint32_t flags);

/**
 * @brief Read i2c data blocks.
 * @param bus The i2c bus number.
 * @param add The slave address.
 * @param offset The starting offset.
 * @param size The byte count.
 * @param flags Seel ONLP_I2C_F_*
 * @note This function reads in increments of ONLPLIB_CONFIG_I2C_BLOCK_SIZE
 */
int onlp_i2c_block_read(int bus, uint8_t addr, uint8_t offset, int size,
                        uint8_t* rdata, uint32_t flags);

/**
 * @brief Write i2c data.
 * @param bus The i2c bus number.
 * @param addr The slave address.
 * @param offset The byte offset.
 * @param size The byte count.
 * @param data The data to write.
 * @param flags See ONLP_I2C_F_*
 */
int onlp_i2c_write(int bus, uint8_t addr, uint8_t offset, int size,
                   uint8_t* data, uint32_t flags);

/**
 * @brief Read a single byte over i2c
 * @param bus The i2c bus number.
 * @param addr The slave address.
 * @param offset The byte offset.
 * @param flags See ONLP_I2C_F_*
 * @returns The byte if successfull, errno on error.
 */
int onlp_i2c_readb(int bus, uint8_t addr, uint8_t offset, uint32_t flags);

/**
 * @brief Write a single byte over i2c
 * @param bus The i2c bus number.
 * @param addr The slave address.
 * @param offset The byte offset.
 * @param byte The byte
 * @param flags See ONLP_I2C_F_*
 */
int onlp_i2c_writeb(int bus, uint8_t addr, uint8_t offset, uint8_t byte,
                    uint32_t flags);


/**
 * @brief Modify a single byte over i2c
 * @param bus The i2c bus number.
 * @param addr The slave address.
 * @param offset The byte offset.
 * @param andmask The and mask.
 * @param ormask The or mask.
 * @param flags See ONLP_I2C_F_*
 */
int onlp_i2c_modifyb(int bus, uint8_t addr, uint8_t offset,
                     uint8_t andmask, uint8_t ormask, uint32_t flags);

/**
 * @brief Read a word over i2c
 * @param bus The i2c bus number.
 * @param addr The address.
 * @param offset The byte offset.
 * @param flags See ONLP_I2C_F_*
 * @returns The word if successfull, errno on error.
 */
int onlp_i2c_readw(int bus, uint8_t addr, uint8_t offset, uint32_t flags);

/**
 * @brief Write a word over i2c
 * @param bus The i2c bus number.
 * @param addr The slave address.
 * @param offset The byte offset.
 * @param byte The byte
 * @param flags See ONLP_I2C_F_*
 */
int onlp_i2c_writew(int bus, uint8_t addr, uint8_t offset, uint16_t word,
                    uint32_t flags);


#endif /* ONLPLIB_CONFIG_INCLUDE_I2C */

#endif /* __ONLP_I2C_H__ */
