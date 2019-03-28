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
 ************************************************************
 *
 * Attribute Platform Implementation.
 *
 ***********************************************************/
#ifndef __ONLP_ATTRIBUTEI_H__
#define __ONLP_ATTRIBUTEI_H__

#include <onlp/attribute.h>
#include <onlp/onlp.h>

/**
 * @brief Initialize the attribute subsystem.
 */
int onlp_attributei_sw_init(void);

/**
 * @brief Initialize the attribute subsystem.
 */
int onlp_attributei_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the attribute subsystem.
 */
int onlp_attributei_sw_denit(void);

/**
 * @brief Determine whether the OID supports the given attributei.
 * @param oid The OID.
 * @param attribute The attribute name.
 */
int onlp_attributei_supported(onlp_oid_t oid, const char* attribute);

/**
 * @brief Set an attribute on the given OID.
 * @param oid The OID.
 * @param attribute The attribute name.
 * @param value A pointer to the value.
 */
int onlp_attributei_set(onlp_oid_t oid, const char* attribute, void* value);

/**
 * @brief Get an attribute from the given OID.
 * @param oid The OID.
 * @param attribute The attribute to retrieve.
 * @param[out] value Receives the attributei's value.
 */
int onlp_attributei_get(onlp_oid_t oid, const char* attribute,
                            void** value);

/**
 * @brief Free an attribute value returned from onlp_attributei_get().
 * @param oid The OID.
 * @param attribute The attribute.
 * @param value The value.
 */
int onlp_attributei_free(onlp_oid_t oid, const char* attribute, void* value);

/**
 * Access to standard attributes.
 */

/**
 * @brief Get an OID's ONIE attribute.
 * @param oid The target OID
 * @param rv [out] Receives the ONIE information if supported.
 * @note if rv is NULL you should only return whether the attribute is supported.
 */
int onlp_attributei_onie_info_get(onlp_oid_t oid, onlp_onie_info_t* rv);

/**
 * @brief Get an OID's Asset attribute.
 * @param oid The target OID.
 * @param rv [out] Receives the Asset information if supported.
 * @note if rv is NULL you should only return whether the attribute is supported.
 */
int onlp_attributei_asset_info_get(onlp_oid_t oid, onlp_asset_info_t* rv);

#endif /* __ONLP_ATTRIBUTEI_H__ */
