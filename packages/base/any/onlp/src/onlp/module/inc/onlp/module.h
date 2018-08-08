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
 * @brief Module OID Interface.
 * @addtogroup oid-module
 * @{
 *
 ***********************************************************/

#ifndef __ONLP_MODULE_H__
#define __ONLP_MODULE_H__

#include <onlp/onlp_config.h>
#include <onlp/onlp.h>
#include <onlplib/onie.h>
#include <onlplib/pi.h>
#include <onlp/oids.h>

/**
 * @brief Module OID Information Structure.
 */
typedef struct onlp_module_info_s {
    /** OID Header */
    onlp_oid_hdr_t hdr;

    /* Module objects have no dedicated fields. */

} onlp_module_info_t;

/**
 * @brief Initialize the module software module.
 * @note This will be called at software initialization
 * time. This should not initialize any hardware.
 */
int onlp_module_sw_init(void);

/**
 * @brief Initialize the module.
 * @param flags The initialization flags.
 */
int onlp_module_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the module software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_module_sw_denit(void);

/**
 * @brief Get the module header structure.
 * @param oid The Module oid.
 * @param[out] hdr Receives the header.
 */
int onlp_module_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr);

/**
 * @brief Get the module info structure.
 * @param oid The Module oid.
 * @param[out] info
 */
int onlp_module_info_get(onlp_oid_t oid, onlp_module_info_t* info);

/**
 * @brief Convert a module info structure to user JSON.
 * @param info The module info structure.
 * @param [out] rv Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_module_info_to_user_json(onlp_module_info_t* info, cJSON** rv, uint32_t flags);

/**
 * @brief Convert a module info structure to JSON.
 * @param info The module info structure.
 * @param [out] rv Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_module_info_to_json(onlp_module_info_t* info, cJSON** rv, uint32_t flags);

/**
 * @brief Convert a JSON object to a module info structure.
 * @param cj The JSON object.
 * @param [out] info Receives the module info structure.
 */
int onlp_module_info_from_json(cJSON* cj, onlp_module_info_t* info);

#endif /* __ONLP_MODULE_H_ */
/* @} */
