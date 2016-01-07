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
 * Fan Management.
 *
 ***********************************************************/
#ifndef __ONLP_FAN_H__
#define __ONLP_FAN_H__

#include <onlp/oids.h>
#include <onlp/onlp.h>

/* <auto.start.enum(tag:fan).define> */
/** onlp_fan_caps */
typedef enum onlp_fan_caps_e {
    ONLP_FAN_CAPS_B2F = (1 << 0),
    ONLP_FAN_CAPS_F2B = (1 << 1),
    ONLP_FAN_CAPS_SET_RPM = (1 << 2),
    ONLP_FAN_CAPS_SET_PERCENTAGE = (1 << 3),
    ONLP_FAN_CAPS_GET_RPM = (1 << 4),
    ONLP_FAN_CAPS_GET_PERCENTAGE = (1 << 5),
} onlp_fan_caps_t;

/** onlp_fan_dir */
typedef enum onlp_fan_dir_e {
    ONLP_FAN_DIR_B2F,
    ONLP_FAN_DIR_F2B,
    ONLP_FAN_DIR_LAST = ONLP_FAN_DIR_F2B,
    ONLP_FAN_DIR_COUNT,
    ONLP_FAN_DIR_INVALID = -1,
} onlp_fan_dir_t;

/** onlp_fan_mode */
typedef enum onlp_fan_mode_e {
    ONLP_FAN_MODE_OFF,
    ONLP_FAN_MODE_SLOW,
    ONLP_FAN_MODE_NORMAL,
    ONLP_FAN_MODE_FAST,
    ONLP_FAN_MODE_MAX,
    ONLP_FAN_MODE_LAST = ONLP_FAN_MODE_MAX,
    ONLP_FAN_MODE_COUNT,
    ONLP_FAN_MODE_INVALID = -1,
} onlp_fan_mode_t;

/** onlp_fan_status */
typedef enum onlp_fan_status_e {
    ONLP_FAN_STATUS_PRESENT = (1 << 0),
    ONLP_FAN_STATUS_FAILED = (1 << 1),
    ONLP_FAN_STATUS_B2F = (1 << 2),
    ONLP_FAN_STATUS_F2B = (1 << 3),
} onlp_fan_status_t;
/* <auto.end.enum(tag:fan).define> */

/**
 * Fan information structure.
 */
typedef struct onlp_fan_info_s {
    /** OID Header */
    onlp_oid_hdr_t hdr;

    /* Status */
    uint32_t status;

    /* Capabilities */
    uint32_t caps;

    /* Current fan speed, in RPM, if available */
    int rpm;

    /* Current fan percentage, if available */
    int percentage;

    /* Current fan mode, if available  */
    onlp_fan_mode_t mode;

    /* Model (if applicable) */
    char model[ONLP_CONFIG_INFO_STR_MAX];

    /* Serial Number (if applicable) */
    char serial[ONLP_CONFIG_INFO_STR_MAX];

} onlp_fan_info_t;


/**
 * @brief Initialize the fan subsystem.
 */
int onlp_fan_init(void);


/**
 * @brief Retrieve fan information.
 * @param id The fan OID.
 * @param rv [out] Receives the fan information.
 */
int onlp_fan_info_get(onlp_oid_t id, onlp_fan_info_t* rv);

/**
 * @brief Set the fan speed in RPMs.
 * @param id The fan OID.
 * @param rpm The new RPM.
 * @note Only valid if the fan has the SET_RPM capability.
 */
int onlp_fan_rpm_set(onlp_oid_t id, int rpm);

/**
 * @brief Set the fan speed in percentage.
 * @param id The fan OID.
 * @param p The percentage.
 * @note Only valid if the fan has the SET_PERCENTAGE capability.
 */
int onlp_fan_percentage_set(onlp_oid_t id, int p);

/**
 * @brief Set the fan speed by mode.
 * @param id The fan OID.
 * @param mode The fan mode value.
 */
int onlp_fan_mode_set(onlp_oid_t id, onlp_fan_mode_t mode);

/**
 * @brief Set the fan direction.
 * @param id The fan OID.
 * @param dir The fan direction (B2F or F2B)
 * @notes Only called if both capabilities are set.
 */
int onlp_fan_dir_set(onlp_oid_t id, onlp_fan_dir_t dir);

/**
 * @brief Fan OID debug dump.
 * @param id The fan OID.
 * @param pvs The output pvs.
 * @param flags The output flags.
 */
void onlp_fan_dump(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags);

/**
 * @brief Show the given Fan OID.
 * @param id The Fan OID
 * @param pvs The output pvs
 * @param flags The output flags.
 */
void onlp_fan_show(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags);

/**
 * Convenience macros for processing Fan status information.
 */
#define ONLP_FAN_STATUS_PRESENT(_fi) ((_fi).status & ONLP_FAN_STATUS_PRESENT)
#define ONLP_FAN_STATUS_MISSING(_fi) (!ONLP_FAN_INFO_PRESENT(_fi))
#define ONLP_FAN_STATUS_FAILED(_fi) ( (_fi).status & ONLP_FAN_STATUS_FAILED)
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

/** Strings macro. */
#define ONLP_FAN_MODE_STRINGS \
{\
    "OFF", \
    "SLOW", \
    "NORMAL", \
    "FAST", \
    "MAX", \
}
/** Enum names. */
const char* onlp_fan_mode_name(onlp_fan_mode_t e);

/** Enum values. */
int onlp_fan_mode_value(const char* str, onlp_fan_mode_t* e, int substr);

/** Enum descriptions. */
const char* onlp_fan_mode_desc(onlp_fan_mode_t e);

/** validator */
#define ONLP_FAN_MODE_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= ONLP_FAN_MODE_MAX))

/** onlp_fan_mode_map table. */
extern aim_map_si_t onlp_fan_mode_map[];
/** onlp_fan_mode_desc_map table. */
extern aim_map_si_t onlp_fan_mode_desc_map[];

/** Enum names. */
const char* onlp_fan_status_name(onlp_fan_status_t e);

/** Enum values. */
int onlp_fan_status_value(const char* str, onlp_fan_status_t* e, int substr);

/** Enum descriptions. */
const char* onlp_fan_status_desc(onlp_fan_status_t e);

/** Enum validator. */
int onlp_fan_status_valid(onlp_fan_status_t e);

/** validator */
#define ONLP_FAN_STATUS_VALID(_e) \
    (onlp_fan_status_valid((_e)))

/** onlp_fan_status_map table. */
extern aim_map_si_t onlp_fan_status_map[];
/** onlp_fan_status_desc_map table. */
extern aim_map_si_t onlp_fan_status_desc_map[];
/* <auto.end.enum(tag:fan).supportheader> */


#endif /* __ONLP_FAN_H__ */

