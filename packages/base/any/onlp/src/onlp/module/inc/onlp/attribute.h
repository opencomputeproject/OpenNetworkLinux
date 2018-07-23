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

/**
 * @brief Initialize the attribute subsystem.
 */
int onlp_attribute_sw_init(void);
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

#endif /* __ONLP_ATTRIBUTE_H__ */
/* @} */
