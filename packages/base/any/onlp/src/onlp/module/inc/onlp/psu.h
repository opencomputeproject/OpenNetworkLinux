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
 * @brief Power Supply Management.
 * @addtogroup oid-psu
 * @{
 *
 ***********************************************************/
#ifndef __ONLP_PSU_H__
#define __ONLP_PSU_H__

#include <onlp/onlp.h>
#include <onlp/oids.h>

/* <auto.start.enum(tag:psu).define> */
/** onlp_psu_caps */
typedef enum onlp_psu_caps_e {
    ONLP_PSU_CAPS_GET_TYPE = (1 << 0),
    ONLP_PSU_CAPS_GET_VIN = (1 << 1),
    ONLP_PSU_CAPS_GET_VOUT = (1 << 2),
    ONLP_PSU_CAPS_GET_IIN = (1 << 3),
    ONLP_PSU_CAPS_GET_IOUT = (1 << 4),
    ONLP_PSU_CAPS_GET_PIN = (1 << 5),
    ONLP_PSU_CAPS_GET_POUT = (1 << 6),
} onlp_psu_caps_t;

/** onlp_psu_type */
typedef enum onlp_psu_type_e {
    ONLP_PSU_TYPE_AC,
    ONLP_PSU_TYPE_DC12,
    ONLP_PSU_TYPE_DC48,
    ONLP_PSU_TYPE_LAST = ONLP_PSU_TYPE_DC48,
    ONLP_PSU_TYPE_COUNT,
    ONLP_PSU_TYPE_INVALID = -1,
} onlp_psu_type_t;
/* <auto.end.enum(tag:psu).define> */


/**
 * PSU Information Structure
 */
typedef struct onlp_psu_info_t {
    /** OID Header */
    onlp_oid_hdr_t hdr;

    /** Model */
    char model[ONLP_CONFIG_INFO_STR_MAX];

    /** Serial Number */
    char serial[ONLP_CONFIG_INFO_STR_MAX];

    /** Capabilities - a combination of @ref onlp_psu_caps_t */
    uint32_t caps;

    /** Type */
    onlp_psu_type_t type;

    /** millivolts in  */
    int mvin;

    /** millivolts out */
    int mvout;

    /** milliamps in */
    int miin;

    /** milliamps out */
    int miout;

    /** milliwatts in */
    int mpin;

    /** milliwatts out */
    int mpout;

} onlp_psu_info_t;


/**
 * Determine if a PSU capability is set.
 */
#define ONLP_PSU_INFO_CAP_IS_SET(_pinfo, _name) \
    ((_pinfo)->caps & ONLP_PSU_CAPS_##_name)

/**
 * @brief Software initialization of the PSU module.
 */
int onlp_psu_sw_init(void);

/**
 * @brief Hardware initialization of the PSU module.
 * @param flags The hardware initialization flags.
 */
int onlp_psu_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the psu software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_psu_sw_denit(void);

/**
 * @brief Get the PSU's oid header.
 * @param oid The PSU OID.
 * @param[out] rv Receives the header.
 */
int onlp_psu_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* rv);

/**
 * @brief Get the PSU information.
 * @param oid The PSU OID.
 * @param[out] rv Receives the information structure.
 */
int onlp_psu_info_get(onlp_oid_t oid, onlp_psu_info_t* rv);

/**
 * @brief Convert a PSU info structure to user JSON.
 * @param info The PSU info structure
 * @param [out] cj Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_psu_info_to_user_json(onlp_psu_info_t* info, cJSON** cj, uint32_t flags);

/**
 * @brief Convert a PSU info structure to JSON.
 * @param info The PSU info structure
 * @param [out] cj Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_psu_info_to_json(onlp_psu_info_t* info, cJSON** cj, uint32_t flags);

/**
 * @brief Convert a JSON object to a PSU info structure.
 * @param cj The JSON object.
 * @param [out] info Receives the PSU info structure.
 */
int onlp_psu_info_from_json(cJSON* cj, onlp_psu_info_t* info);


/** PSU is present. */
#define ONLP_PSU_STATUS_PRESENT(_pi) ( (_pi).status & ONLP_PSU_STATUS_PRESENT )

/** PSU is missing. */
#define ONLP_PSU_STATUS_MISSING(_pi) (!ONLP_PSU_STATUS_PRESENT(_pi))

/** PSU has failed. */
#define ONLP_PSU_STATUS_FAILED(_pi) ( (_pi).status & ONLP_PSU_STATUS_FAILED)



/******************************************************************************
 *
 * Enumeration Support Definitions.
 *
 * Please do not add additional code beyond this point.
 *
 *****************************************************************************/
/* <auto.start.enum(tag:psu).supportheader> */
/** Enum names. */
const char* onlp_psu_caps_name(onlp_psu_caps_t e);

/** Enum values. */
int onlp_psu_caps_value(const char* str, onlp_psu_caps_t* e, int substr);

/** Enum descriptions. */
const char* onlp_psu_caps_desc(onlp_psu_caps_t e);

/** Enum validator. */
int onlp_psu_caps_valid(onlp_psu_caps_t e);

/** validator */
#define ONLP_PSU_CAPS_VALID(_e) \
    (onlp_psu_caps_valid((_e)))

/** onlp_psu_caps_map table. */
extern aim_map_si_t onlp_psu_caps_map[];
/** onlp_psu_caps_desc_map table. */
extern aim_map_si_t onlp_psu_caps_desc_map[];

/** Strings macro. */
#define ONLP_PSU_TYPE_STRINGS \
{\
    "AC", \
    "DC12", \
    "DC48", \
}
/** Enum names. */
const char* onlp_psu_type_name(onlp_psu_type_t e);

/** Enum values. */
int onlp_psu_type_value(const char* str, onlp_psu_type_t* e, int substr);

/** Enum descriptions. */
const char* onlp_psu_type_desc(onlp_psu_type_t e);

/** validator */
#define ONLP_PSU_TYPE_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= ONLP_PSU_TYPE_DC48))

/** onlp_psu_type_map table. */
extern aim_map_si_t onlp_psu_type_map[];
/** onlp_psu_type_desc_map table. */
extern aim_map_si_t onlp_psu_type_desc_map[];
/* <auto.end.enum(tag:psu).supportheader> */

#endif /* __ONLP_PSU_H__ */
/* @} */
