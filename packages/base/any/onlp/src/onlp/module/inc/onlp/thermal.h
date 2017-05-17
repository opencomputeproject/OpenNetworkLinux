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
 * Thermal Sensor Management.
 *
 ************************************************************/
#ifndef __ONLP_THERMAL_H__
#define __ONLP_THERMAL_H__

#include <onlp/onlp_config.h>
#include <onlp/onlp.h>
#include <onlp/oids.h>

/* <auto.start.enum(tag:thermal).define> */
/** onlp_thermal_caps */
typedef enum onlp_thermal_caps_e {
    ONLP_THERMAL_CAPS_GET_TEMPERATURE = (1 << 0),
    ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD = (1 << 1),
    ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD = (1 << 2),
    ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD = (1 << 3),
} onlp_thermal_caps_t;

/** onlp_thermal_status */
typedef enum onlp_thermal_status_e {
    ONLP_THERMAL_STATUS_PRESENT = (1 << 0),
    ONLP_THERMAL_STATUS_FAILED = (1 << 1),
} onlp_thermal_status_t;

/** onlp_thermal_threshold */
typedef enum onlp_thermal_threshold_e {
    ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT = 45000,
    ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT = 55000,
    ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT = 60000,
} onlp_thermal_threshold_t;
/* <auto.end.enum(tag:thermal).define> */

/**
 * Shortcut for specifying all capabilties.
 */
#define ONLP_THERMAL_CAPS_ALL 0xF

/**
 * Shortcut for determining the availability of any threshold value.
 */
#define ONLP_THERMAL_CAPS_GET_ANY_THRESHOLD                             \
    ( ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD |                         \
      ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD   |                         \
      ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD )

/**
 * Shortcut for all default thermal threshold value.
 */
#define ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS            \
    { ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT,           \
      ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT,             \
      ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT }

/**
 * Thermal sensor information structure.
 */
typedef struct onlp_thermal_info_s {

    /** OID Header */
    onlp_oid_hdr_t hdr;

    /** Status */
    uint32_t status;

    /** Capabilities */
    uint32_t caps;

    /* Current temperature in milli-celsius */
    int mcelsius;

    struct {
        /* Warning temperature threshold in milli-celsius */
        int warning;

        /* Error temperature threshold in milli-celsius */
        int error;

        /* System shutdown temperature threshold in milli-celsius */
        int shutdown;
    } thresholds;

} onlp_thermal_info_t;

/**
 * @brief Initialize the thermal subsystem.
 */
int onlp_thermal_init(void);

/**
 * @brief Retrieve information about the given thermal id.
 * @param id The thermal oid.
 * @param rv [out] Receives the thermal information.
 */
int onlp_thermal_info_get(onlp_oid_t id, onlp_thermal_info_t* rv);

/**
 * @brief Retrieve the thermal's operational status.
 * @param id The thermal oid.
 * @param rv [out] Receives the operational status.
 */
int onlp_thermal_status_get(onlp_oid_t id, uint32_t* rv);

/**
 * @brief Retrieve the thermal's oid header.
 * @param id The thermal oid.
 * @param rv [out] Receives the header.
 */
int onlp_thermal_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv);

/**
 * @brief Thermal driver ioctl.
 * @param code Thermal ioctl code.
 * @param ... Arguments
 */
int onlp_thermal_ioctl(int code, ...);

/**
 * @brief Thermal driver ioctl.
 * @param code The thermal ioctl code.
 * @param vargs The arguments.
 */
int onlp_thermal_vioctl(int code, va_list vargs);

/**
 * @brief Thermal OID debug dump.
 * @param id The thermal id.
 * @param pvs The output pvs.
 * @param flags The dump flags.
 */
void onlp_thermal_dump(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags);

/**
 * @brief Show the given thermal OID.
 * @param id The Thermal OID
 * @param pvs The output pvs
 * @param flags The output flags
 */
void onlp_thermal_show(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags);




/******************************************************************************
 *
 * Enumeration Support Definitions.
 *
 * Please do not add additional code beyond this point.
 *
 *****************************************************************************/
/* <auto.start.enum(tag:thermal).supportheader> */
/** Enum names. */
const char* onlp_thermal_caps_name(onlp_thermal_caps_t e);

/** Enum values. */
int onlp_thermal_caps_value(const char* str, onlp_thermal_caps_t* e, int substr);

/** Enum descriptions. */
const char* onlp_thermal_caps_desc(onlp_thermal_caps_t e);

/** Enum validator. */
int onlp_thermal_caps_valid(onlp_thermal_caps_t e);

/** validator */
#define ONLP_THERMAL_CAPS_VALID(_e) \
    (onlp_thermal_caps_valid((_e)))

/** onlp_thermal_caps_map table. */
extern aim_map_si_t onlp_thermal_caps_map[];
/** onlp_thermal_caps_desc_map table. */
extern aim_map_si_t onlp_thermal_caps_desc_map[];

/** Enum names. */
const char* onlp_thermal_status_name(onlp_thermal_status_t e);

/** Enum values. */
int onlp_thermal_status_value(const char* str, onlp_thermal_status_t* e, int substr);

/** Enum descriptions. */
const char* onlp_thermal_status_desc(onlp_thermal_status_t e);

/** Enum validator. */
int onlp_thermal_status_valid(onlp_thermal_status_t e);

/** validator */
#define ONLP_THERMAL_STATUS_VALID(_e) \
    (onlp_thermal_status_valid((_e)))

/** onlp_thermal_status_map table. */
extern aim_map_si_t onlp_thermal_status_map[];
/** onlp_thermal_status_desc_map table. */
extern aim_map_si_t onlp_thermal_status_desc_map[];

/** Enum names. */
const char* onlp_thermal_threshold_name(onlp_thermal_threshold_t e);

/** Enum values. */
int onlp_thermal_threshold_value(const char* str, onlp_thermal_threshold_t* e, int substr);

/** Enum descriptions. */
const char* onlp_thermal_threshold_desc(onlp_thermal_threshold_t e);

/** Enum validator. */
int onlp_thermal_threshold_valid(onlp_thermal_threshold_t e);

/** validator */
#define ONLP_THERMAL_THRESHOLD_VALID(_e) \
    (onlp_thermal_threshold_valid((_e)))

/** onlp_thermal_threshold_map table. */
extern aim_map_si_t onlp_thermal_threshold_map[];
/** onlp_thermal_threshold_desc_map table. */
extern aim_map_si_t onlp_thermal_threshold_desc_map[];
/* <auto.end.enum(tag:thermal).supportheader> */

#endif /* __ONLP_THERMAL_H__ */
