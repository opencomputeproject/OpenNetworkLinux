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
 ********************************************************//**
 *
 * @file
 * @brief SFP Platform Implementation Interface.
 * This interface must be implemented and available for all
 * platforms that support SFP interfaces.
 *
 * @addtogroup sfpi
 * @{
 *
 ***********************************************************/
#ifndef __ONLP_SFPI_H__
#define __ONLP_SFPI_H__

#include <onlp/onlp_config.h>
#include <onlp/sfp.h>
#include <sff/sff.h>

/**
 * @brief Software initialization of the SFP module.
 */
int onlp_sfpi_sw_init(void);

/**
 * @brief Hardware initialization of the SFP module.
 * @param flags The hardware initialization flags.
 */
int onlp_sfpi_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the chassis software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_sfpi_sw_denit(void);


/**
 * @brief Get the bitmap of SFP-capable port numbers.
 * @param[out] bmap Receives the bitmap.
 */
int onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap);

/**
 * @brief Determine the SFP connector type.
 * @param port The SFP Port ID.
 * @param[out] Receives the connector type.
 */
int onlp_sfpi_type_get(int port, onlp_sfp_type_t* rtype);

/**
 * @brief Determine if an SFP is present.
 * @param port The SFP Port ID.
 * @returns 1 if present
 * @returns 0 if absent
 * @returns An error condition.
 */
int onlp_sfpi_is_present(int port);

/**
 * @brief Return the presence bitmap for all SFP ports.
 * @param[out] dst Receives the presence bitmap.
 */
int onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst);

/**
 * @brief Return the RX_LOS bitmap for all SFP ports.
 * @param[out] dst Receives the RX_LOS bitmap.
 */
int onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst);


/**
 * @brief Read bytes from the target device on the given SFP port.
 * @param port The SFP Port ID.
 * @param devaddr The device address.
 * @param addr Read offset.
 * @param[out] dst Receives the data.
 * @param len Read length.
 * @returns The number of bytes read or ONLP_STATUS_E_* no error.
 */
int onlp_sfpi_dev_read(int port, int devaddr, int addr,
                       uint8_t* dst, int len);


/**
 * @brief Write bytes to the target device on the given SFP port.
 * @param port The SFP Port ID.
 * @param devaddr The device address.
 * @param src The bytes to write.
 * @param offset Write offset.
 * @param len Write length.
 */
int onlp_sfpi_dev_write(int port, int devaddr, int addr,
                        uint8_t* src, int len);

/**
 * @brief Read a byte from the target device on the given SFP port.
 * @param port The SFP Port ID.
 * @param devaddr The device address.
 * @param addr The read address.
 * @returns The byte on success or ONLP_STATUS_E* on error.
 */
int onlp_sfpi_dev_readb(int port, int devaddr, int addr);

/**
 * @brief Write a byte to the target device on the given SFP port.
 * @param port The SFP Port ID.
 * @param devaddr The device address.
 * @param addr The write address.
 * @param value The write value.
 */
int onlp_sfpi_dev_writeb(int port, int devaddr, int addr,
                         uint8_t value);

/**
 * @brief Read a word from the target device on the given SFP port.
 * @param port The SFP Port ID.
 * @param devaddr The device address.
 * @param addr The read address.
 * @returns The word if successful, ONLP_STATUS_E* on error.
 */
int onlp_sfpi_dev_readw(int port, int devaddr, int addr);

/**
 * @brief Write a word to the target device on the given SFP port.
 * @param port The SFP Port ID.
 * @param devaddr The device address.
 * @param addr The write address.
 * @param value The write value.
 */
int onlp_sfpi_dev_writew(int port, int devaddr, int addr,
                         uint16_t value);

/**
 * @brief Perform any actions required after an SFP is inserted.
 * @param port The SFP Port ID.
 * @param info The SFF Module information structure.
 * @note This function is optional. If your platform must
 * adjust equalizer or preemphasis settings internally then
 * this function should be implemented as the trigger.
 */
int onlp_sfpi_post_insert(int port, sff_info_t* info);

/**
 * @brief Returns whether or not the given control is supported on the given port.
 * @param port The SFP Port ID.
 * @param control The control.
 * @param[out] rv Receives 1 if supported, 0 if not supported.
 * @note This provided for convenience and is optional.
 * If you implement this function your control_set and control_get APIs
 * will not be called on unsupported ports.
 */
int onlp_sfpi_control_supported(int port,
                                onlp_sfp_control_t control, int* rv);

/**
 * @brief Set an SFP control.
 * @param port The SFP Port ID.
 * @param control The control.
 * @param value The value.
 */
int onlp_sfpi_control_set(int port, onlp_sfp_control_t control,
                          int value);

/**
 * @brief Get an SFP control.
 * @param port The SFP Port ID.
 * @param control The control
 * @param[out] value Receives the current value.
 */
int onlp_sfpi_control_get(int port, onlp_sfp_control_t control,
                          int* value);

/**
 * @brief Remap SFP user SFP port numbers before calling the SFPI interface.
 * @param port The SFP Port ID.
 * @param[out] rport Receives the new port.
 * @note This function will be called to remap the user SFP port number
 * to the number returned in rport before the SFPI functions are called.
 * This is an optional convenience for platforms with dynamic or
 * variant physical SFP numbering.
 */
int onlp_sfpi_port_map(int port, int* rport);


/**
 * @brief Get the SFP's OID header.
 * @param oid The SFP oid.
 * @param hdr Receives the header.
 */
int onlp_sfpi_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* rhdr);

/**
 * @brief GEt the SFP's info structure.
 * @param oid The SFP oid.
 * @param info Receives the SFP information.
 * @note It is not normally necessary to implement this function.
 * The upper layer implements this on behalf of the platform
 * using the existing SFP primitives.
 */
int onlp_sfpi_info_get(onlp_oid_t oid, onlp_sfp_info_t* info);

#endif /* __ONLP_SFPI_H__ */
/* @} */
