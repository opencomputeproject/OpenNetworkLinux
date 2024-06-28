/*************************************************************
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
 *************************************************************
 *
 * SFP Management Interface.
 *
 ************************************************************/
#ifndef __ONLP_SFP_H__
#define __ONLP_SFP_H__

#include <onlp/onlp_config.h>
#include <onlp/onlp.h>
#include <AIM/aim_bitmap.h>
#include <AIM/aim_pvs.h>
#include <sff/sff.h>

/* <auto.start.enum(tag:sfp1).define> */
/** onlp_sfp_control */
typedef enum onlp_sfp_control_e {
    ONLP_SFP_CONTROL_RESET,
    ONLP_SFP_CONTROL_RESET_STATE,
    ONLP_SFP_CONTROL_RX_LOS,
    ONLP_SFP_CONTROL_TX_FAULT,
    ONLP_SFP_CONTROL_TX_DISABLE,
    ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL,
    ONLP_SFP_CONTROL_LP_MODE,
    ONLP_SFP_CONTROL_SOFT_RATE_SELECT,
    ONLP_SFP_CONTROL_POWER_OVERRIDE,
    ONLP_SFP_CONTROL_LAST = ONLP_SFP_CONTROL_POWER_OVERRIDE,
    ONLP_SFP_CONTROL_COUNT,
    ONLP_SFP_CONTROL_INVALID = -1,
} onlp_sfp_control_t;
/* <auto.end.enum(tag:sfp1).define> */

/* <auto.start.enum(tag:sfp2).define> */
/** onlp_sfp_control_flag */
typedef enum onlp_sfp_control_flag_e {
    ONLP_SFP_CONTROL_FLAG_RESET = (1 << 0),
    ONLP_SFP_CONTROL_FLAG_RESET_STATE = (1 << 1),
    ONLP_SFP_CONTROL_FLAG_RX_LOS = (1 << 2),
    ONLP_SFP_CONTROL_FLAG_TX_FAULT = (1 << 3),
    ONLP_SFP_CONTROL_FLAG_TX_DISABLE = (1 << 4),
    ONLP_SFP_CONTROL_FLAG_TX_DISABLE_CHANNEL = (1 << 5),
    ONLP_SFP_CONTROL_FLAG_LP_MODE = (1 << 6),
    ONLP_SFP_CONTROL_FLAG_SOFT_RATE_SELECT = (1 << 7),
    ONLP_SFP_CONTROL_FLAG_POWER_OVERRIDE = (1 << 8),
} onlp_sfp_control_flag_t;
/* <auto.end.enum(tag:sfp2).define> */

/**
 * Initialize the SFP subsystem.
 */
int onlp_sfp_init(void);

/**
 * Valid SFP Port bitmaps are communicated using this type.
 */
typedef aim_bitmap256_t onlp_sfp_bitmap_t;

/**
 * Convenience function for initializing SFP bitmaps.
 * @param bmap The address of the bitmap to initialize.
 */
void onlp_sfp_bitmap_t_init(onlp_sfp_bitmap_t* bmap);

/**
 * @brief Get the set of valid {Q}SFP ports.
 * @param bmap Returns the valid set of SFP-capable port numbers.
 */
int onlp_sfp_bitmap_get(onlp_sfp_bitmap_t* bmap);

/**
 * @brief Determine if a given port number is a valid SFP port.
 * @param port The port number.
 */
int onlp_sfp_port_valid(int port);

/**
 * @brief Determine if an SFP is currently plugged in.
 * @param port The SFP port number.
 * @returns 1 if an SFP is present.
 * @returns 0 if an SFP is not present.
 * @returns <0 on error.
 */
int onlp_sfp_is_present(int port);

/**
 * @brief Return the presence bitmap for all ports.
 * @param dst The receives the presence bitmap for all ports.
 * @note This function can return Unsupported.
 * It will not be emulated if the SFPI driver does not support
 * batch collection of the SFP presence.
 */
int onlp_sfp_presence_bitmap_get(onlp_sfp_bitmap_t* dst);

/**
 * @brief Read IEEE standard EEPROM data from the given port.
 * @param port The SFP Port
 * @param rv Receives a buffer containing the EEPROM data.
 * @notes The buffer must be freed after use.
 * @returns The size of the eeprom data, if successful
 * @returns -1 on error.
 */
int onlp_sfp_eeprom_read(int port, uint8_t** rv);


/**
 * @brief Read the DOM data from the given port.
 * @param port The SFP Port
 * @param rv Receives a buffer containing the DOM data.
 * @notes The buffer must be freed after use.
 * @returns The size of the eeprom data, if successful
 * @returns -1 on error.
 * @note This should only be called if the SFP
 * has advertised DOM support.
 */
int onlp_sfp_dom_read(int port, uint8_t** rv);

/**
 * @brief Deinitialize the SFP subsystem.
 */
int onlp_sfp_denit(void);

/**
 * @brief Get the RX_LOS bitmap for all ports.
 * @param dst Receives the RX_LOS bitmap for all ports.
 * @note This function can return Unsupported.
 * It will not be emulated if the SFPI driver does not support
 * batch collection of the rx_los status.
 */
int onlp_sfp_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst);


/**
 * @brief Read a byte from an address on the given SFP port's bus.
 * @param port The port number.
 * @param devaddr The device address.
 * @param addr The address.
 */
int onlp_sfp_dev_readb(int port, uint8_t devaddr, uint8_t addr);

/**
 * @brief Write a byte to an address on the given SFP port's bus.
 */
int onlp_sfp_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value);

/**
 * @brief Read a byte from an address on the given SFP port's bus.
 * @param port The port number.
 * @param devaddr The device address.
 * @param addr The address.
 */
int onlp_sfp_dev_readw(int port, uint8_t devaddr, uint8_t addr);

/**
 * @brief Write a byte to an address on the given SFP port's bus.
 */
int onlp_sfp_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value);




/**
 * @brief Dump the status of all SFPs
 * @param pvs The output pvs.
 */
void onlp_sfp_dump(aim_pvs_t* pvs);

/**
 * @brief Issue an ioctl to the SFP interface.
 * @param port The port.
 * @param ... Ioctl arguments.
 */
int onlp_sfp_ioctl(int port, ...);

/**
 * @brief Issue an ioctl to the SFP interface.
 * @param port The port.
 * @param vargs Ioctl arguments.
 */
int onlp_sfp_vioctl(int port, va_list vargs);

/**
 * @brief Call the SFP post-insertion handler.
 *
 */
int onlp_sfp_post_insert(int port, sff_info_t* info);

/**
 * @brief Set an SFP control.
 * @param port The port.
 * @param control The control.
 * @param value The value.
 */
int onlp_sfp_control_set(int port, onlp_sfp_control_t control, int value);

/**
 * @brief Get an SFP control.
 * @param port The port.
 * @param control The control
 * @param [out] value Receives the current value.
 */
int onlp_sfp_control_get(int port, onlp_sfp_control_t control, int* value);

/**
 * @brief Get the value of all SFP controls.
 * @param port The port.
 * @param flags Receives the control flag values. See onlp_sfp_control_flags_t
 */
int onlp_sfp_control_flags_get(int port, uint32_t* flags);

/******************************************************************************
 *
 * Enumeration Support Definitions.
 *
 * Please do not add additional code beyond this point.
 *
 *****************************************************************************/
/* <auto.start.enum(tag:sfp1).supportheader> */
/** Strings macro. */
#define ONLP_SFP_CONTROL_STRINGS \
{\
    "RESET", \
    "RESET_STATE", \
    "RX_LOS", \
    "TX_FAULT", \
    "TX_DISABLE", \
    "TX_DISABLE_CHANNEL", \
    "LP_MODE", \
    "SOFT_RATE_SELECT", \
    "POWER_OVERRIDE", \
}
/** Enum names. */
const char* onlp_sfp_control_name(onlp_sfp_control_t e);

/** Enum values. */
int onlp_sfp_control_value(const char* str, onlp_sfp_control_t* e, int substr);

/** Enum descriptions. */
const char* onlp_sfp_control_desc(onlp_sfp_control_t e);

/** validator */
#define ONLP_SFP_CONTROL_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= ONLP_SFP_CONTROL_POWER_OVERRIDE))

/** onlp_sfp_control_map table. */
extern aim_map_si_t onlp_sfp_control_map[];
/** onlp_sfp_control_desc_map table. */
extern aim_map_si_t onlp_sfp_control_desc_map[];
/* <auto.end.enum(tag:sfp1).supportheader> */

/* <auto.start.enum(tag:sfp2).supportheader> */
/** Enum names. */
const char* onlp_sfp_control_flag_name(onlp_sfp_control_flag_t e);

/** Enum values. */
int onlp_sfp_control_flag_value(const char* str, onlp_sfp_control_flag_t* e, int substr);

/** Enum descriptions. */
const char* onlp_sfp_control_flag_desc(onlp_sfp_control_flag_t e);

/** Enum validator. */
int onlp_sfp_control_flag_valid(onlp_sfp_control_flag_t e);

/** validator */
#define ONLP_SFP_CONTROL_FLAG_VALID(_e) \
    (onlp_sfp_control_flag_valid((_e)))

/** onlp_sfp_control_flag_map table. */
extern aim_map_si_t onlp_sfp_control_flag_map[];
/** onlp_sfp_control_flag_desc_map table. */
extern aim_map_si_t onlp_sfp_control_flag_desc_map[];
/* <auto.end.enum(tag:sfp2).supportheader> */

#endif /* __ONLP_SFP_H__ */
