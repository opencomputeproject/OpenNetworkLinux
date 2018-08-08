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
 * @brief Chassis OID Interface.
 *
 * The Chassis OID represents the root of the system OID tree.
 * There must be one and only one Chassis OID.
 *
 *
 * @addtogroup oid-chassis
 * @{
 *
 ***********************************************************/

#ifndef __ONLP_CHASSIS_H__
#define __ONLP_CHASSIS_H__

#include <onlp/onlp_config.h>
#include <onlp/onlp.h>
#include <onlplib/onie.h>
#include <onlplib/pi.h>
#include <onlp/oids.h>

/**
 * @brief Chassis Information Structure.
 */

typedef struct onlp_chassis_info_s {
    /** OID Header */
    onlp_oid_hdr_t hdr;

    /* Chassis objects have no dedicated fields. */

} onlp_chassis_info_t;

/**
 * @brief Initialize the chassis software module.
 * @note This will be called at software initialization
 * time. This should not initialize any hardware.
 */
int onlp_chassis_sw_init(void);

/**
 * @brief Initialize the chassis.
 * @param flags The initialization flags.
 */
int onlp_chassis_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the chassis software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_chassis_sw_denit(void);

/**
 * @brief Get the Chassis Header structure.
 * @param oid The Chassis oid.
 * @param[out] hdr Receives the hdr.
 */
int onlp_chassis_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr);

/**
 * @brief Get the chassis info structure.
 * @param oid The Chassis oid.
 * @param[out] info
 */
int onlp_chassis_info_get(onlp_oid_t oid, onlp_chassis_info_t* info);

/**
 * @brief Convert a chassis info structure to user JSON.
 * @param info The chassis info structure.
 * @param [out] cj Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_chassis_info_to_user_json(onlp_chassis_info_t* info, cJSON** cj, uint32_t flags);

/**
 * @brief Convert a chassis info structure to JSON.
 * @param info The chassis info structure.
 * @param [out] cj Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_chassis_info_to_json(onlp_chassis_info_t* info, cJSON** cj, uint32_t flags);

/**
 * @brief Convert a JSON object to a chassis info structure.
 * @param cj The JSON object.
 * @param [out] info Receives the chassis info.
 */
int onlp_chassis_info_from_json(cJSON* cj, onlp_chassis_info_t* info);

/******************************************************************************
 *
 * Standard Chassis Features
 *
 *****************************************************************************/

/**
 * @brief Get the user or full environmental json data.
 * @param [out] cjp Receives the JSON data.
 * @param flags Zero or ONLP_OID_JSON_FLAG_TO_USER_JSON only.
 * @note This will provide the user view of the environmental data

 * with keys in the following order:
 *   Chassis Fans
 *   Chassis Thermals
 *   Chassis PSUs
 */
int onlp_chassis_environment_to_json(cJSON** cjp, uint32_t flags);


/**
 * @brief Show the environmental data.
 * @param pvs The output pvs.
 * @param flags Zero or ONLP_OID_JSON_FLAG_TO_USER_JSON only.
 * @note See onlp_chassis_environment_to_json()
 */
int onlp_chassis_environment_show(aim_pvs_t* pvs, uint32_t flags);

/**
 * @brief Construct the Chassis debug JSON object.
 * @param [out] rv Receives the JSON object.
 */
int onlp_chassis_debug_get_json(cJSON** rv);

/**
 * @brief Show the Chassis debug information.
`* @param pvs The output pvs.
 */
int onlp_chassis_debug_show(aim_pvs_t* pvs);

#endif /* __ONLP_CHASSIS_H_ */
/* @} */
