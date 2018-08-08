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
 *********************************************************//**
 *
 * @file
 * @brief SFP Management Interface.
 * @addtogroup oid-sfp
 * @{
 *
 ************************************************************/
#ifndef __ONLP_SFP_H__
#define __ONLP_SFP_H__

#include <onlp/onlp_config.h>
#include <onlp/onlp.h>
#include <onlp/oids.h>
#include <AIM/aim_bitmap.h>
#include <AIM/aim_pvs.h>
#include <sff/sff.h>
#include <sff/dom.h>

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
    ONLP_SFP_CONTROL_FLAG_POWER_OVERRIDE = (1 << 7),
} onlp_sfp_control_flag_t;

/** onlp_sfp_type */
typedef enum onlp_sfp_type_e {
    ONLP_SFP_TYPE_SFP,
    ONLP_SFP_TYPE_QSFP,
    ONLP_SFP_TYPE_SFP28,
    ONLP_SFP_TYPE_QSFP28,
    ONLP_SFP_TYPE_LAST = ONLP_SFP_TYPE_QSFP28,
    ONLP_SFP_TYPE_COUNT,
    ONLP_SFP_TYPE_INVALID = -1,
} onlp_sfp_type_t;
/* <auto.end.enum(tag:sfp2).define> */



/**
 * SFP Block Data Size
 */
#define ONLP_SFP_BLOCK_DATA_SIZE 256


/**
 * SFP Information Structure.
 */
typedef struct onlp_sfp_info_t {
    /** OID Header */
    onlp_oid_hdr_t hdr;

    /** SFP Connector Type */
    onlp_sfp_type_t type;

    /** The SFP Control Status */
    uint32_t controls;


    /*
     * The following fields are only relevant
     * if the SFP is present.
     */


    /**
     * The SFF Parse of the idprom.
     * The parse is valid if sff.sfp_type != INVALID.
     *
     * Note that the vendor, model, and serial will likely
     * be populated correctly event if the rest of the
     * data could not be parsed correctly.
     */
    sff_info_t sff;

    /**
     * The SFF Diagnostics information.
     */
    sff_dom_info_t dom;

    /** The raw data upon which the meta info is based. */
    struct {
        /** The last A0 data */
        uint8_t a0[ONLP_SFP_BLOCK_DATA_SIZE];

        /** The last A2 data (for SFP+ only) */
        uint8_t a2[ONLP_SFP_BLOCK_DATA_SIZE];
    } bytes;

} onlp_sfp_info_t;

/**
 * Valid SFP Port bitmaps are communicated using this type.
 */
typedef aim_bitmap256_t onlp_sfp_bitmap_t;


/**
 * @brief Software initialization of the SFP module.
 */
int onlp_sfp_sw_init(void);

/**
 * @brief Hardware initialization of the SFP module.
 * @param flags The hardware initialization flags.
 */
int onlp_sfp_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the sfp software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_sfp_sw_denit(void);

/**
 * Convenience function for initializing SFP bitmaps.
 * @param bmap The address of the bitmap to initialize.
 */
void onlp_sfp_bitmap_t_init(onlp_sfp_bitmap_t* bmap);

/**
 * @brief Get the set of valid SFP ports.
 * @param bmap Returns the valid set of SFP-capable port numbers.
 */
int onlp_sfp_bitmap_get(onlp_sfp_bitmap_t* bmap);

/**
 * @brief Get the SFP information structure.
 * @param port The SFP OID or Port ID.
 * @param[out] info Receives the information structure.
 */
int onlp_sfp_info_get(onlp_oid_t port, onlp_sfp_info_t* info);

/**
 * @brief Get the SFP information structure (including DOM)
 * @param port The SFP OID or Port ID.
 * @param[out] info Receives the information structure.
 */
int onlp_sfp_info_dom_get(onlp_oid_t port, onlp_sfp_info_t* info);

/**
 * @brief Get the SFP's oid header.
 * @param port The SFP OID.
 * @param[out] rv Receives the header.
 */
int onlp_sfp_hdr_get(onlp_oid_t port, onlp_oid_hdr_t* rv);

/**
 * @brief Determine if a given port number is a valid SFP port.
 * @param port The port number.
 */
int onlp_sfp_port_valid(onlp_oid_t port);

/**
 * @brief Determine the SFP Connector type.
 * @param port The port number.
 * @param[out] rtype Receives the type.
 */
int onlp_sfp_type_get(onlp_oid_t port, onlp_sfp_type_t* rtype);

/**
 * @brief Determine if an SFP is currently plugged in.
 * @param port The SFP port number.
 * @returns 1 if an SFP is present.
 * @returns 0 if an SFP is not present.
 * @returns <0 on error.
 */
int onlp_sfp_is_present(onlp_oid_t port);

/**
 * @brief Return the presence bitmap for all ports.
 * @param dst The receives the presence bitmap for all ports.
 * @note This function can return Unsupported.
 * It will not be emulated if the SFPI driver does not support
 * batch collection of the SFP presence.
 */
int onlp_sfp_presence_bitmap_get(onlp_sfp_bitmap_t* dst);


/**
 * @brief Get the RX_LOS bitmap for all ports.
 * @param dst Receives the RX_LOS bitmap for all ports.
 * @note This function can return Unsupported.
 * It will not be emulated if the SFPI driver does not support
 * batch collection of the rx_los status.
 */
int onlp_sfp_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst);

/**
 * @brief Read bytes from the target device on the given SFP port.
 * @param port The SFP OID or Port ID.
 * @param devaddr The device address.
 * @param addr The address to read.
 * @param dst Receives the data.
 * @param count The read length.
 * @returns The number of bytes read or ONLP_STATUS_E_* no error.
 */
int onlp_sfp_dev_read(onlp_oid_t port, int devaddr, int addr,
                      uint8_t* dst, int count);

/**
 * @brief Read bytes from the target device on the given SFP port.
 * @param port The SFP OID or Port ID.
 * @param devaddr The device address.
 * @param addr The start target address.
 * @param count The number of bytes to read.
 * @param [out] rv Receives the allocated buffer.
 * @note The returned buffer must be freed after use.
 */
int onlp_sfp_dev_alloc_read(onlp_oid_t port,
                            int devaddr, int addr, int count,
                            uint8_t** rv);

/**
 * @brief Write bytes to the target device on the given SFP port.
 * @param port The SFP OID or Port ID.
 * @param devaddr The device address.
 * @param addr The address to write.
 * @param src The source data.
 * @param count The write length.
 */
int onlp_sfp_dev_write(onlp_oid_t port, int devaddr, int addr,
                       uint8_t* src, int count);

/**
 * @brief Read a byte from the target device on the given SFP port.
 * @param port The SFP Port ID.
 * @param devaddr The device address.
 * @param addr The read address.
 * @returns The byte on success or ONLP_STATUS_E* on error.
 */
int onlp_sfp_dev_readb(onlp_oid_t port, int devaddr, int addr);

/**
 * @brief Write a byte to the target device on the given SFP port.
 * @param port The SFP Port ID.
 * @param devaddr The device address.
 * @param addr The write address.
 * @param value The write value.
 */
int onlp_sfp_dev_writeb(onlp_oid_t port, int devaddr, int addr,
                        uint8_t value);

/**
 * @brief Read a word from the target device on the given SFP port.
 * @param port The SFP Port ID.
 * @param devaddr The device address.
 * @param addr The read address.
 * @returns The word if successful, ONLP_STATUS_E* on error.
 */
int onlp_sfp_dev_readw(onlp_oid_t port, int devaddr, int addr);

/**
 * @brief Write a word to the target device on the given SFP port.
 * @param port The SFP Port ID.
 * @param devaddr The device address.
 * @param addr The write address.
 * @param value The write value.
 */
int onlp_sfp_dev_writew(onlp_oid_t port, int devaddr, int addr,
                        uint16_t value);



/**
 * @brief Perform any actions required after an SFP is inserted.
 * @param port The SFP Port ID.
 * @param info The SFF Module information structure.
 * @note This function is optional. If your platform must
 * adjust equalizer or preemphasis settings internally then
 * this function should be implemented as the trigger.
 */
int onlp_sfp_post_insert(onlp_oid_t port, sff_info_t* info);

/**
 * @brief Set an SFP control.
 * @param port The SFP Port ID.
 * @param control The control.
 * @param value The value.
 */
int onlp_sfp_control_set(onlp_oid_t port, onlp_sfp_control_t control, int value);

/**
 * @brief Get an SFP control.
 * @param port The SFP Port ID.
 * @param control The control
 * @param[out] value Receives the current value.
 */
int onlp_sfp_control_get(onlp_oid_t port, onlp_sfp_control_t control,
                         int* value);

/**
 * @brief Get the value of all SFP controls.
 * @param port The port.
 * @param flags Receives the control flag values. See onlp_sfp_control_flags_t
 */
int onlp_sfp_control_flags_get(onlp_oid_t port, uint32_t* flags);


/**
 * @brief Convert an SFP info structure to user JSON.
 * @param info The SFP info structure.
 * @param [out] rv Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_sfp_info_to_user_json(onlp_sfp_info_t* info, cJSON** rv, uint32_t flags);

/**
 * @brief Convert an SFP info structure to JSON.
 * @param info The SFP info structure.
 * @param [out] rv Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_sfp_info_to_json(onlp_sfp_info_t* info, cJSON** rv, uint32_t flags);


/**
 * @brief Convert a JSON object to an SFP info structure.
 * @param cj The JSON object.
 * @param [out] info Receives the SFP info structure.
 */
int onlp_sfp_info_from_json(cJSON* cj, onlp_sfp_info_t* info);

/**
 * @brief Software deinitialize the SFP subsystem.
 */
int onlp_sfp_sw_denit(void);

/**
 * @brief Hardware deinitialize the SFP subsystem.
 */
int onlp_sfp_hw_denit(void);

/**
 * @brief Show the current SFP inventory.
 */
int onlp_sfp_inventory_show(aim_pvs_t* pvs);

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

/** Strings macro. */
#define ONLP_SFP_TYPE_STRINGS \
{\
    "SFP", \
    "QSFP", \
    "SFP28", \
    "QSFP28", \
}
/** Enum names. */
const char* onlp_sfp_type_name(onlp_sfp_type_t e);

/** Enum values. */
int onlp_sfp_type_value(const char* str, onlp_sfp_type_t* e, int substr);

/** Enum descriptions. */
const char* onlp_sfp_type_desc(onlp_sfp_type_t e);

/** validator */
#define ONLP_SFP_TYPE_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= ONLP_SFP_TYPE_QSFP28))

/** onlp_sfp_type_map table. */
extern aim_map_si_t onlp_sfp_type_map[];
/** onlp_sfp_type_desc_map table. */
extern aim_map_si_t onlp_sfp_type_desc_map[];
/* <auto.end.enum(tag:sfp2).supportheader> */

#endif /* __ONLP_SFP_H__ */
/* @} */
