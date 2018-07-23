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
 * Attribute Implementation.
 *
 ***********************************************************/
#include <onlp/attribute.h>
#include <onlp/platformi/attributei.h>
#include <onlp/stdattrs.h>
#include <onlp/oids.h>
#include "onlp_int.h"
#include "onlp_locks.h"
#include "onlp_log.h"

/**
 * @brief Initialize the attribute subsystem.
 */
static int
onlp_attribute_sw_init_locked__(void)
{
    return onlp_attributei_sw_init();
}
ONLP_LOCKED_API0(onlp_attribute_sw_init)

static int
onlp_attribute_hw_init_locked__(uint32_t flags)
{
    return onlp_attributei_hw_init(flags);
}
ONLP_LOCKED_API1(onlp_attribute_hw_init, uint32_t, flags)

/**
 * @brief Determine whether the OID supports the given attribute.
 * @param oid The OID.
 * @param attribute The attribute name.
 */
static int
onlp_attribute_supported_locked__(onlp_oid_t oid, const char* attribute)
{
    if(ONLP_ATTRIBUTE_EQUALS(attribute, ONLP_ATTRIBUTE_ONIE_INFO_JSON)) {
        attribute = ONLP_ATTRIBUTE_ONIE_INFO;
    }
    if(ONLP_ATTRIBUTE_EQUALS(attribute, ONLP_ATTRIBUTE_ASSET_INFO_JSON)) {
        attribute = ONLP_ATTRIBUTE_ASSET_INFO;
    }
    return onlp_attributei_supported(oid, attribute);
}
ONLP_LOCKED_API2(onlp_attribute_supported, onlp_oid_t, oid, const char*, attribute)

/**
 * @brief Set an attribute on the given OID.
 * @param oid The OID.
 * @param attribute The attribute name.
 * @param value A pointer to the value.
 */
static int
onlp_attribute_set_locked__(onlp_oid_t oid, const char* attribute, void* value)
{
    return onlp_attributei_set(oid, attribute, value);
}
ONLP_LOCKED_API3(onlp_attribute_set, onlp_oid_t, oid, const char*, attribute, void*, value)


/**
 * @brief Free an attribute value returned from onlp_attribute_get().
 * @param oid The OID.
 * @param attribute The attribute.
 * @param value The value.
 */
static int
onlp_attribute_free_locked__(onlp_oid_t oid, const char* attribute, void* value)
{
    if(ONLP_ATTRIBUTE_EQUALS(attribute, ONLP_ATTRIBUTE_ONIE_INFO_JSON) ||
       ONLP_ATTRIBUTE_EQUALS(attribute, ONLP_ATTRIBUTE_ASSET_INFO_JSON)) {
        cJSON_Delete(value);
        return 0;
    }
    return onlp_attributei_free(oid, attribute, value);
}
ONLP_LOCKED_API3(onlp_attribute_free,onlp_oid_t, oid, const char*, attribute, void*, value)


/**
 * @brief Get an attribute from the given OID.
 * @param oid The OID.
 * @param attribute The attribute to retrieve.
 * @param[out] value Receives the attribute's value.
 */
static int
onlp_attribute_get_locked__(onlp_oid_t oid, const char* attribute,
                            void** value)
{
    int rv;
    const char* rattr = attribute;

    if(ONLP_ATTRIBUTE_EQUALS(attribute, ONLP_ATTRIBUTE_ONIE_INFO_JSON)) {
        rattr = ONLP_ATTRIBUTE_ONIE_INFO;
    }
    if(ONLP_ATTRIBUTE_EQUALS(attribute, ONLP_ATTRIBUTE_ASSET_INFO_JSON)) {
        rattr = ONLP_ATTRIBUTE_ASSET_INFO;
    }

    rv = onlp_attributei_get(oid, rattr, value);

    if(ONLP_FAILURE(rv)) {
        return rv;
    }

    if(ONLP_ATTRIBUTE_EQUALS(attribute, ONLP_ATTRIBUTE_ONIE_INFO_JSON)) {
        cJSON* cj;
        onlp_onie_info_to_json(*(onlp_onie_info_t**)value, &cj);
        onlp_attribute_free_locked__(oid, rattr, *value);
        *value = cj;
    }

    if(ONLP_ATTRIBUTE_EQUALS(attribute, ONLP_ATTRIBUTE_ASSET_INFO_JSON)) {
        cJSON* cj;
        onlp_asset_info_to_json(*(onlp_asset_info_t**)value, &cj);
        onlp_attribute_free_locked__(oid, rattr, *value);
        *value = cj;
    }

    return rv;
}
ONLP_LOCKED_API3(onlp_attribute_get, onlp_oid_t, oid, const char*, attribute, void**, value)
