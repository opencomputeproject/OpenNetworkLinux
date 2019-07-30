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
 * Do not deselect mux channels after device operations.
 * The default is to deselect all intermediate muxes if possible.
 */
#define ONLP_I2C_F_NO_MUX_DESELECT 0x8

/**
 * Do not select mux channels prior to device operations.
 * The default is to select all intermediate muxes.
 *
 * This option is useful if you want to manually select
 * the mux channels for multiple operations.
 */
#define ONLP_I2C_F_NO_MUX_SELECT 0x10

/**
 * Use block reads if possible.
 */
#define ONLP_I2C_F_USE_BLOCK_READ 0x20

/**
 * Use SMBUS block reads if possible.
 */
#define ONLP_I2C_F_USE_SMBUS_BLOCK_READ 0x40

/**
 * Do not retry reads on I2C transaction failures.
 */
#define ONLP_I2C_F_DISABLE_READ_RETRIES 0x80

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



/****************************************************************************
 *
 * I2C Mux/Device Management.
 *
 *
 ***************************************************************************/

/**
 * An i2c mux device driver.
 */
typedef struct onlp_i2c_mux_driver_s {
    /** Driver Name */
    const char* name;

    /** Control register address for this mux. */
    uint8_t control;

    /** Channel select values. */
    struct {
        int channel;
        uint8_t value;
    } channels[16];

} onlp_i2c_mux_driver_t;


/**
 * An i2c mux device.
 *
 */
typedef struct onlp_i2c_mux_device_s {
    /* Instance description. */
    const char* name;

    /** i2c bus number */
    int bus;

    /** i2c address for this instance */
    uint8_t devaddr;

    /** Mux device driver */
    onlp_i2c_mux_driver_t* driver;

} onlp_i2c_mux_device_t;


/**
 * Description of a channel.
 */
typedef struct onlp_i2c_mux_channel_s {
    onlp_i2c_mux_device_t* mux;
    int channel;
} onlp_i2c_mux_channel_t;


/**
 * A set of i2c mux channels to program.
 */
typedef struct onlp_i2c_mux_channels_s {
    onlp_i2c_mux_channel_t channels[8];
} onlp_i2c_mux_channels_t;


/**
 * An i2c device.
 *
 * The device can be accessed only after selecting it's channel tree.
 */
typedef struct onlp_i2c_dev_s {
    const char* name;

    /**
     * The sequence of mux channels can be specified
     * inline or via pointer.
     */
    onlp_i2c_mux_channels_t  ichannels;
    onlp_i2c_mux_channels_t* pchannels;

    /**
     * Bus and address for this device after channel tree selection.
     */
    int bus;
    uint8_t addr;

} onlp_i2c_dev_t;


/**
 * @brief Select a mux channel.
 * @param muxdev The mux device instance.
 * @param channel The channel number to select.
 */
int onlp_i2c_mux_select(onlp_i2c_mux_device_t* muxdev, int channel);

/**
 * @brief Deselect a mux channel.
 * @param muxdev The mux device instance.
 */
int onlp_i2c_mux_deselect(onlp_i2c_mux_device_t* muxdev);

/**
 * @brief Select a mux channel.
 */
int onlp_i2c_mux_channel_select(onlp_i2c_mux_channel_t* mc);


/**
 * @brief Select a mux channel.
 */
int onlp_i2c_mux_channel_deselect(onlp_i2c_mux_channel_t* mc);


/**
 * @brief Select a mux channel tree.
 */
int onlp_i2c_mux_channels_select(onlp_i2c_mux_channels_t* channels);

/**
 * @brief Deselect a mux channel tree.
 */
int onlp_i2c_mux_channels_deselect(onlp_i2c_mux_channels_t* channels);

/**
 * @brief Select a device's mux channel tree.
 */
int onlp_i2c_dev_mux_channels_select(onlp_i2c_dev_t* dev);

/**
 * @brief Deselect a device's mux channel tree.
 */
int onlp_i2c_dev_mux_channels_deselect(onlp_i2c_dev_t* dev);


/**
 * @brief Read from an device.
 */
int onlp_i2c_dev_read(onlp_i2c_dev_t* dev, uint8_t offset, int size,
                      uint8_t* rdata, uint32_t flags);

/**
 * @brief Write to a device.
 */
int onlp_i2c_dev_write(onlp_i2c_dev_t* dev,
                       uint8_t offset, int size,
                       uint8_t* data, uint32_t flags);

int onlp_i2c_dev_readb(onlp_i2c_dev_t* dev,
                       uint8_t offset, uint32_t flags);

int onlp_i2c_dev_writeb(onlp_i2c_dev_t* dev,
                        uint8_t offset, uint8_t byte, uint32_t flags);

int onlp_i2c_dev_readw(onlp_i2c_dev_t* dev,
                       uint8_t offset, uint32_t flags);

int onlp_i2c_dev_writew(onlp_i2c_dev_t* dev,
                        uint8_t offset, uint16_t word, uint32_t flags);

/**************************************************************************//**
 *
 * Reusable MUX device drivers.
 *
 *****************************************************************************/
extern onlp_i2c_mux_driver_t onlp_i2c_mux_driver_pca9547a;
extern onlp_i2c_mux_driver_t onlp_i2c_mux_driver_pca9548;



#if ONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER == 1
#include <linux/i2c-devices.h>
#else
#include <linux/i2c-dev.h>
#endif

#if ONLPLIB_CONFIG_I2C_INCLUDE_SMBUS == 1
#include <i2c/smbus.h>
#endif


#endif /* ONLPLIB_CONFIG_INCLUDE_I2C */

#endif /* __ONLP_I2C_H__ */
