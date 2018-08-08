/************************************************************
 * <bsn.cl fy=2017 v=onl>
 *
 *        Copyright 2017 Big Switch Networks, Inc.
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
 * @brief Attributes.
 *
 * @addtogroup attributes
 * @{
 *
 ***********************************************************/
#ifndef __ONLP_ATTRIBUTE_H__
#define __ONLP_ATTRIBUTE_H__

#include <onlp/oids.h>
#include <onlp/onlp.h>
#include <onlp/stdattrs.h>
#include <AIM/aim_pvs.h>

/**
 * @brief Initialize the attribute subsystem.
 * @note This function can only perform software module initialization.
 * It cannot affect the state of the hardware.
 */
int onlp_attribute_sw_init(void);

/**
 * @brief Hardware initialization of the attribute subsystem.
 * @param flags The initialization flags.
 * @note This function will be called once at system startup
 * by the ONLP core. You should not call it.
 */
int onlp_attribute_hw_init(uint32_t flags);

/**
 * @brief Determine whether the OID supports the given attribute.
 * @param oid The OID.
 * @param attribute The attribute name.
 */
int onlp_attribute_supported(onlp_oid_t oid, const char* attribute);

/**
 * @brief Set an attribute on the given OID.
 * @param oid The OID.
 * @param attribute The attribute name.
 * @param value A pointer to the value.
 */
int onlp_attribute_set(onlp_oid_t oid, const char* attribute, void* value);

/**
 * @brief Get an attribute from the given OID.
 * @param oid The OID.
 * @param attribute The attribute to retrieve.
 * @param[out] value Receives the attribute's value.
 */
int onlp_attribute_get(onlp_oid_t oid, const char* attribute,
                       void** value);

/**
 * @brief Free an attribute value returned from onlp_attribute_get().
 * @param oid The OID.
 * @param attribute The attribute.
 * @param value The value.
 */
int onlp_attribute_free(onlp_oid_t oid, const char* attribute, void* value);

/**
 * @brief Attribute comparitor.
 */
#define ONLP_ATTRIBUTE_EQUALS(_a, _b) (!strcmp(_a, _b))

/******************************************************************************
 *
 * These functions provide access to standard ONIE and Asset attributes.
 * These are so common that they have their own interface for
 * for both the user and the platform implementations.
 *
 *****************************************************************************/
/**
 * @brief Request the ONIE attribute.
 * @param oid The target OID.
 * @param [out] rp Receives the ONIE information structure pointer.
 * @note Setting rp to NULL will determine if the ONIE attribute is supported.
 */
int onlp_attribute_onie_info_get(onlp_oid_t oid, onlp_onie_info_t** rp);

/**
 * @brief Free an ONIE attribute pointer.
 * @param oid The target OID.
 * @param p The ONIE attribute pointer.
 */
int onlp_attribute_onie_info_free(onlp_oid_t oid, onlp_onie_info_t* p);

/**
 * @brief Request the ONIE attribute in JSON
 * @param oid The target OID.
 * @param [out] rp Receives the cJSON object.
 * @note The cJSON object should be freed after use using cJSON_Delete().
 */
int onlp_attribute_onie_info_get_json(onlp_oid_t oid, cJSON** rp);

/**
 * @brief Show the ONIE attribute.
 * @param oid The target OID.
 * @param pvs The output pvs.
 * @note The output is YAML in human-readble format.
 */
int onlp_attribute_onie_info_show(onlp_oid_t oid, aim_pvs_t* pvs);

/**
 * @brief Show the ONIE attribute (JSON)
 * @param oid The target OID.
 * @param pvs The output pvs.
 */
int onlp_attribute_onie_info_show_json(onlp_oid_t oid, aim_pvs_t* pvs);

/**
 * @brief Determine if the ONIE attribute is supported.
 * @param oid The target OID.
 * @note Retu
 */
/**
 * @brief Request the asset attribute.
 * @param oid The target oid.
 * @param [out] rp Receives the Asset information structure pointer.
 * @note Setting rp to NULL will determine if the attribute is supported.
 */
int onlp_attribute_asset_info_get(onlp_oid_t oid, onlp_asset_info_t** rp);

/**
 * @brief Free an asset attribute pointer.
 * @param oid The target oid.
 * @param p The asset attribute pointer.
 */
int onlp_attribute_asset_info_free(onlp_oid_t oid, onlp_asset_info_t* p);

/**
 * @brief Request the asset attribute in JSON
 * @param oid The target oid.
 * @param [out] rp Receives the cJSON object.
 * @note The cJSON object should be freed after use using cJSON_Delete()
 */
int onlp_attribute_asset_info_get_json(onlp_oid_t oid, cJSON** rp);

/**
 * @brief Show the asset attribute.
 * @param oid The target oid.
 * @param pvs The output pvs.
 * @note The output format is YAML in human-readable format.
 */
int onlp_attribute_asset_info_show(onlp_oid_t oid, aim_pvs_t* pvs);

/**
 * @brief Show the asset attribute (JSON).
 * @param oid The target oid.
 * @param pvs The output pvs.
 */
int onlp_attribute_asset_info_show_json(onlp_oid_t oid, aim_pvs_t* pvs);

#endif /* __ONLP_ATTRIBUTE_H__ */
/* @} */
