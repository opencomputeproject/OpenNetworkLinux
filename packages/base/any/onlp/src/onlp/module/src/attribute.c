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

static int
onlp_attribute_sw_denit_locked__(void)
{
    return onlp_attributei_sw_denit();
}
ONLP_LOCKED_API0(onlp_attribute_sw_denit)

/**
 * @brief Determine whether the OID supports the given attribute.
 * @param oid The OID.
 * @param attribute The attribute name.
 */
static int
onlp_attribute_supported_locked__(onlp_oid_t oid, const char* attribute)
{
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
    return onlp_attributei_get(oid, attribute, value);
}
ONLP_LOCKED_API3(onlp_attribute_get, onlp_oid_t, oid, const char*, attribute, void**, value)


static int
onlp_attribute_onie_info_get_locked__(onlp_oid_t oid, onlp_onie_info_t** rvp)
{
    int rv;
    onlp_onie_info_t* rp;

    ONLP_PTR_VALIDATE_ZERO(rvp);
    ONLP_TRY(onlp_attributei_onie_info_get(oid, NULL));

    rp = aim_zmalloc(sizeof(*rp));
    rv = onlp_attributei_onie_info_get(oid, rp);

    if(ONLP_FAILURE(rv)) {
        aim_free(rp);
        rp = NULL;
        return rv;
    }

    *rvp = rp;
    return rv;
}
ONLP_LOCKED_API2(onlp_attribute_onie_info_get, onlp_oid_t, oid, onlp_onie_info_t**, rvp);

int
onlp_attribute_onie_info_free(onlp_oid_t oid, onlp_onie_info_t* p)
{
    if(p) {
        onlp_onie_info_free(p);
        aim_free(p);
    }
    return 0;
}

int
onlp_attribute_onie_info_get_json(onlp_oid_t oid, cJSON** rvp)
{
    int rv;
    onlp_onie_info_t* onie_info;
    if(ONLP_SUCCESS(rv = onlp_attribute_onie_info_get(oid, &onie_info))) {
        onlp_onie_info_to_json(onie_info, rvp);
        onlp_attribute_onie_info_free(oid, onie_info);
    }
    return rv;
}

int
onlp_attribute_onie_info_show(onlp_oid_t oid, aim_pvs_t* pvs)
{
    int rv;
    cJSON* cjp = NULL;

    if(ONLP_SUCCESS(rv = onlp_attribute_onie_info_get_json(oid, &cjp))) {
        cjson_util_yaml_pvs(pvs, cjp);
        cJSON_Delete(cjp);
    }
    else {
        aim_printf(pvs, "There was an error requesting the ONIE information from %{onlp_oid}: %{onlp_status}", oid, rv);
    }
    return rv;
}

int
onlp_attribute_onie_info_show_json(onlp_oid_t oid, aim_pvs_t* pvs)
{
    int rv;
    cJSON* cjp = NULL;

    if(ONLP_SUCCESS(rv = onlp_attribute_onie_info_get_json(oid, &cjp))) {
        cjson_util_json_pvs(pvs, cjp);
        cJSON_Delete(cjp);
    }
    else {
        aim_printf(pvs, "There was an error requesting the ONIE information from %{onlp_oid}: %{onlp_status}", oid, rv);
    }
    return rv;
}


static int
onlp_attribute_asset_info_get_locked__(onlp_oid_t oid, onlp_asset_info_t** rvp)
{
    int rv;
    onlp_asset_info_t* rp;

    ONLP_PTR_VALIDATE_ZERO(rvp);
    ONLP_TRY(onlp_attributei_asset_info_get(oid, NULL));

    rp = aim_zmalloc(sizeof(*rp));
    rv = onlp_attributei_asset_info_get(oid, rp);

    if(ONLP_FAILURE(rv)) {
        aim_free(rp);
        rp = NULL;
        return rv;
    }

    *rvp = rp;
    return rv;
}
ONLP_LOCKED_API2(onlp_attribute_asset_info_get, onlp_oid_t, oid, onlp_asset_info_t**, rvp);

int
onlp_attribute_asset_info_free(onlp_oid_t oid, onlp_asset_info_t* p)
{
    if(p) {
        onlp_asset_info_free(p);
        aim_free(p);
    }
    return 0;
}

int
onlp_attribute_asset_info_get_json(onlp_oid_t oid, cJSON** rvp)
{
    int rv;
    onlp_asset_info_t* asset_info;
    if(ONLP_SUCCESS(rv = onlp_attribute_asset_info_get(oid, &asset_info))) {
        onlp_asset_info_to_json(asset_info, rvp);
        onlp_attribute_asset_info_free(oid, asset_info);
    }
    return rv;
}

int
onlp_attribute_asset_info_show(onlp_oid_t oid, aim_pvs_t* pvs)
{
    int rv;
    cJSON* cjp = NULL;

    if(ONLP_SUCCESS(rv = onlp_attribute_asset_info_get_json(oid, &cjp))) {
        cjson_util_yaml_pvs(pvs, cjp);
        cJSON_Delete(cjp);
    }
    else {
        aim_printf(pvs, "There was an error requesting the asset information from %{onlp_oid}: %{onlp_status}", oid, rv);
    }
    return rv;
}


int
onlp_attribute_asset_info_show_json(onlp_oid_t oid, aim_pvs_t* pvs)
{
    int rv;
    cJSON* cjp = NULL;

    if(ONLP_SUCCESS(rv = onlp_attribute_asset_info_get_json(oid, &cjp))) {
        cjson_util_json_pvs(pvs, cjp);
        cJSON_Delete(cjp);
    }
    else {
        aim_printf(pvs, "There was an error requesting the asset information from %{onlp_oid}: %{onlp_status}", oid, rv);
    }
    return rv;
}
