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
 ************************************************************
 *
 * Power Supply Management.
 *
 ***********************************************************/
#ifndef __ONLP_PSU_H__
#define __ONLP_PSU_H__

#include <onlp/onlp.h>
#include <onlp/oids.h>

/* <auto.start.enum(tag:psu).define> */
/** onlp_psu_caps */
typedef enum onlp_psu_caps_e {
    ONLP_PSU_CAPS_AC = (1 << 0),
    ONLP_PSU_CAPS_DC12 = (1 << 1),
    ONLP_PSU_CAPS_DC48 = (1 << 2),
    ONLP_PSU_CAPS_VIN = (1 << 3),
    ONLP_PSU_CAPS_VOUT = (1 << 4),
    ONLP_PSU_CAPS_IIN = (1 << 5),
    ONLP_PSU_CAPS_IOUT = (1 << 6),
    ONLP_PSU_CAPS_PIN = (1 << 7),
    ONLP_PSU_CAPS_POUT = (1 << 8),
} onlp_psu_caps_t;

/** onlp_psu_status */
typedef enum onlp_psu_status_e {
    ONLP_PSU_STATUS_PRESENT = (1 << 0),
    ONLP_PSU_STATUS_FAILED = (1 << 1),
    ONLP_PSU_STATUS_UNPLUGGED = (1 << 2),
} onlp_psu_status_t;
/* <auto.end.enum(tag:psu).define> */


/**
 * PSU Information Structure
 */
typedef struct onlp_psu_info_t {
    /** OID Header */
    onlp_oid_hdr_t hdr;

    /* Model */
    char model[ONLP_CONFIG_INFO_STR_MAX];

    /* Serial Number */
    char serial[ONLP_CONFIG_INFO_STR_MAX];

    /* Status */
    uint32_t status;

    /* Capabilities */
    uint32_t caps;

    /* millivolts */
    int mvin;
    int mvout;

    /* milliamps */
    int miin;
    int miout;

    /* milliwatts */
    int mpin;
    int mpout;

} onlp_psu_info_t;

/**
 * @brief Initialize the PSU subsystem.
 */
int onlp_psu_init(void);

/**
 * @brief Get the PSU information.
 * @param id The PSU OID.
 * @param rv [out] Receives the information structure.
 */
int onlp_psu_info_get(onlp_oid_t id, onlp_psu_info_t* rv);

/**
 * @brief Get the PSU's operational status.
 * @param id The PSU OID.
 * @param rv [out] Receives the operational status.
 */
int onlp_psu_status_get(onlp_oid_t id, uint32_t* rv);

/**
 * @brief Get the PSU's oid header.
 * @param id The PSU OID.
 * @param rv [out] Receives the header.
 */
int onlp_psu_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv);

/**
 * @brief Issue a PSU ioctl.
 * @param id The PSU OID
 * @param ... Ioctl arguments.
 */
int onlp_psu_ioctl(onlp_oid_t id, ...);

/**
 * @brief Issue a PSU ioctl.
 * @param id The PSU OID
 * @param vargs Ioctl arguments.
 */
int onlp_psu_vioctl(onlp_oid_t id, va_list vargs);

/**
 * @brief PSU OID debug dump
 * @param id The PSU OID
 * @param pvs The output pvs
 * @param flags The output flags
 */
void onlp_psu_dump(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags);


/**
 * @brief Show the given PSU OID.
 * @param id The PSU OID
 * @param pvs The output pvs
 * @param flags The output flags
 */
void onlp_psu_show(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags);

/**
 * Convenience macros for processing PSU status.
 */
#define ONLP_PSU_STATUS_PRESENT(_pi) ( (_pi).status & ONLP_PSU_STATUS_PRESENT )
#define ONLP_PSU_STATUS_MISSING(_pi) (!ONLP_PSU_STATUS_PRESENT(_pi))
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

/** Enum names. */
const char* onlp_psu_status_name(onlp_psu_status_t e);

/** Enum values. */
int onlp_psu_status_value(const char* str, onlp_psu_status_t* e, int substr);

/** Enum descriptions. */
const char* onlp_psu_status_desc(onlp_psu_status_t e);

/** Enum validator. */
int onlp_psu_status_valid(onlp_psu_status_t e);

/** validator */
#define ONLP_PSU_STATUS_VALID(_e) \
    (onlp_psu_status_valid((_e)))

/** onlp_psu_status_map table. */
extern aim_map_si_t onlp_psu_status_map[];
/** onlp_psu_status_desc_map table. */
extern aim_map_si_t onlp_psu_status_desc_map[];
/* <auto.end.enum(tag:psu).supportheader> */

#endif /* __ONLP_PSU_H__ */
