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
 * @brief Thermal Sensor Management.
 * @addtogroup oid-thermal
 * @{
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

    /** Capabilities - a combination of @ref onlp_thermal_caps_t */
    uint32_t caps;

    /** Current temperature in milli-celsius */
    int mcelsius;

    /** Thermal Thresholds */
    struct {
        /** Warning temperature threshold in milli-celsius */
        int warning;

        /** Error temperature threshold in milli-celsius */
        int error;

        /** System shutdown temperature threshold in milli-celsius */
        int shutdown;
    } thresholds;

} onlp_thermal_info_t;


/**
 * Determine if a thermal capability is set.
 */
#define ONLP_THERMAL_INFO_CAP_IS_SET(_pinfo, _name)     \
    ((_pinfo)->caps & ONLP_THERMAL_CAPS_##_name)

/**
 * @brief Software initialization of the thermal module.
 */
int onlp_thermal_sw_init(void);

/**
 * @brief Hardware initialization of the thermal module.
 * @param flags The hardware initialization flags.
 */
int onlp_thermal_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the thermal software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_thermal_sw_denit(void);

/**
 * @brief Retrieve the thermal's oid header.
 * @param oid The thermal oid.
 * @param[out] rv Receives the header.
 */
int onlp_thermal_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* rv);

/**
 * @brief Retrieve information about the given thermal id.
 * @param oid The thermal oid.
 * @param[out] rv Receives the thermal information.
 */
int onlp_thermal_info_get(onlp_oid_t oid, onlp_thermal_info_t* rv);

/**
 * @brief Convert a thermal info structure to json.
 * @param info The thermal info structure.
 * @param [out] rv Receives the JSON object.
 * @param flags The JSON processing flags.
 */
int onlp_thermal_info_to_json(onlp_thermal_info_t* info, cJSON** rv, uint32_t flags);


/**
 * @brief Convert a JSON object to a thermal info structure.
 * @param cj The JSON object representing the structure.
 * @param [out] info Receives the thermal info.
 */
int onlp_thermal_info_from_json(cJSON* cj, onlp_thermal_info_t* info);

/**
 * @brief Convert a thermal info structure to user json.
 * @param info The thermal info structure.
 * @param [out] rv Receives the JSON object.
 * @param flags The JSON processing flags.
 * @note The user json format contains only the fields relevant to user output.
 */
int onlp_thermal_info_to_user_json(onlp_thermal_info_t* info, cJSON** rv, uint32_t flags);



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
/* @} */
