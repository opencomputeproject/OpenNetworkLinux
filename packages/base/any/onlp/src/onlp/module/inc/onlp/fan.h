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
 * @brief Fan Management.
 * @addtogroup oid-fan
 * @{
 *
 ***********************************************************/
#ifndef __ONLP_FAN_H__
#define __ONLP_FAN_H__

#include <onlp/oids.h>
#include <onlp/onlp.h>

#include <BigList/biglist.h>

/* <auto.start.enum(tag:fan).define> */
/** onlp_fan_caps */
typedef enum onlp_fan_caps_e {
    ONLP_FAN_CAPS_SET_DIR = (1 << 0),
    ONLP_FAN_CAPS_GET_DIR = (1 << 1),
    ONLP_FAN_CAPS_SET_RPM = (1 << 2),
    ONLP_FAN_CAPS_SET_PERCENTAGE = (1 << 3),
    ONLP_FAN_CAPS_GET_RPM = (1 << 4),
    ONLP_FAN_CAPS_GET_PERCENTAGE = (1 << 5),
} onlp_fan_caps_t;

/** onlp_fan_dir */
typedef enum onlp_fan_dir_e {
    ONLP_FAN_DIR_UNKNOWN,
    ONLP_FAN_DIR_B2F,
    ONLP_FAN_DIR_F2B,
    ONLP_FAN_DIR_LAST = ONLP_FAN_DIR_F2B,
    ONLP_FAN_DIR_COUNT,
    ONLP_FAN_DIR_INVALID = -1,
} onlp_fan_dir_t;
/* <auto.end.enum(tag:fan).define> */

/**
 * Fan information structure.
 */
typedef struct onlp_fan_info_s {
    /** OID Header */
    onlp_oid_hdr_t hdr;

    /** Current direction */
    onlp_fan_dir_t dir;

    /** Capabilities - a combination of @ref onlp_fan_caps_e. */
    uint32_t caps;

    /** Current fan speed, in RPM, if available */
    int rpm;

    /** Current fan percentage, if available */
    int percentage;

    /** Model (if applicable) */
    char model[ONLP_CONFIG_INFO_STR_MAX];

    /** Serial Number (if applicable) */
    char serial[ONLP_CONFIG_INFO_STR_MAX];

} onlp_fan_info_t;

/**
 * @brief Determine if a fan capability is set.
 */
#define ONLP_FAN_INFO_CAP_IS_SET(_pinfo, _name) \
    ((_pinfo)->caps & ONLP_FAN_CAPS_##_name)

/**
 * @brief Software Initialization of the Fan module.
 */
int onlp_fan_sw_init(void);

/**
 * @brief Hardware Initialization of the Fan module.
 * @param flags The hardware initialization flags.
 */
int onlp_fan_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the fan software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_fan_sw_denit(void);

/**
 * @brief Retrieve the fan's OID hdr.
 * @param oid The fan OID.
 * @param[out] hdr Receives the OID header.
 */
int onlp_fan_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr);

/**
 * @brief Retrieve fan information.
 * @param oid The fan OID.
 * @param[out] rv Receives the fan information.
 */
int onlp_fan_info_get(onlp_oid_t oid, onlp_fan_info_t* rv);

/**
 * @brief Retrieve the fan capabilities.
 * @param oid The fan OID.
 * @param[out] caps Receives the fan capabilities.
 */
int onlp_fan_caps_get(onlp_oid_t oid, uint32_t* caps);

/**
 * @brief Set the fan speed in RPMs.
 * @param oid The fan OID.
 * @param rpm The new RPM.
 * @note Only valid if the fan has the SET_RPM capability.
 */
int onlp_fan_rpm_set(onlp_oid_t oid, int rpm);

/**
 * @brief Set the fan speed in percentage.
 * @param oid The fan OID.
 * @param p The percentage.
 * @note Only valid if the fan has the SET_PERCENTAGE capability.
 */
int onlp_fan_percentage_set(onlp_oid_t oid, int p);

/**
 * @brief Set the fan direction.
 * @param oid The fan OID.
 * @param dir The fan direction (B2F or F2B)
 * @note Only called if both capabilities are set.
 */
int onlp_fan_dir_set(onlp_oid_t oid, onlp_fan_dir_t dir);

/**
 * @brief Convert a fan info structure to user JSON.
 * @param info The fan info structure.
 * @param [out] cj Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_fan_info_to_user_json(onlp_fan_info_t* info, cJSON** cj,
                               uint32_t flags);

/**
 * @brief Convert a fan info structure to JSON.
 * @param info The fan info structure.
 * @param [out] cj Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_fan_info_to_json(onlp_fan_info_t* info, cJSON** cj, uint32_t flags);

/**
 * @brief Convert a JSON object to a fan info structure.
 * @param cj The JSON object.
 * @param [out] info Recieves the fan info structure.
 */
int onlp_fan_info_from_json(cJSON* cj, onlp_fan_info_t* info);


/** Fan is present. */
#define ONLP_FAN_STATUS_PRESENT(_fi) ((_fi).hdr.status & ONLP_OID_STATUS.PRESENT)

/** Fan is missing. */
#define ONLP_FAN_STATUS_MISSING(_fi) (!ONLP_FAN_INFO_PRESENT(_fi))

/** Fan has failed. */
#define ONLP_FAN_STATUS_FAILED(_fi) ( (_fi).hdr.status & ONLP_OID_STATUS.FAILED)

/** Fan is operating normally */
#define ONLP_FAN_STATUS_NORMAL(_fi) ( ONLP_FAN_STATUS_PRESENT(_fi) && !ONLP_FAN_STATUS_FAILED(_fi) )


/******************************************************************************
 *
 * Enumeration Support Definitions.
 *
 * Please do not add additional code beyond this point.
 *
 *****************************************************************************/
/* <auto.start.enum(tag:fan).supportheader> */
/** Enum names. */
const char* onlp_fan_caps_name(onlp_fan_caps_t e);

/** Enum values. */
int onlp_fan_caps_value(const char* str, onlp_fan_caps_t* e, int substr);

/** Enum descriptions. */
const char* onlp_fan_caps_desc(onlp_fan_caps_t e);

/** Enum validator. */
int onlp_fan_caps_valid(onlp_fan_caps_t e);

/** validator */
#define ONLP_FAN_CAPS_VALID(_e) \
    (onlp_fan_caps_valid((_e)))

/** onlp_fan_caps_map table. */
extern aim_map_si_t onlp_fan_caps_map[];
/** onlp_fan_caps_desc_map table. */
extern aim_map_si_t onlp_fan_caps_desc_map[];

/** Strings macro. */
#define ONLP_FAN_DIR_STRINGS \
{\
    "UNKNOWN", \
    "B2F", \
    "F2B", \
}
/** Enum names. */
const char* onlp_fan_dir_name(onlp_fan_dir_t e);

/** Enum values. */
int onlp_fan_dir_value(const char* str, onlp_fan_dir_t* e, int substr);

/** Enum descriptions. */
const char* onlp_fan_dir_desc(onlp_fan_dir_t e);

/** validator */
#define ONLP_FAN_DIR_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= ONLP_FAN_DIR_F2B))

/** onlp_fan_dir_map table. */
extern aim_map_si_t onlp_fan_dir_map[];
/** onlp_fan_dir_desc_map table. */
extern aim_map_si_t onlp_fan_dir_desc_map[];
/* <auto.end.enum(tag:fan).supportheader> */


#endif /* __ONLP_FAN_H__ */
/* @} */
