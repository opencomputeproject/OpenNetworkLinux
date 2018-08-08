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
 *
 *
 ***********************************************************/
#include <onlp/onlp_config.h>
#include <onlp/oids.h>
#include "onlp_log.h"
#include "onlp_int.h"
#include <AIM/aim.h>
#include <AIM/aim_printf.h>

#include <onlp/chassis.h>
#include <onlp/module.h>
#include <onlp/thermal.h>
#include <onlp/fan.h>
#include <onlp/led.h>
#include <onlp/psu.h>
#include <onlp/sfp.h>
#include <onlp/generic.h>

#include <cjson_util/cjson_util_format.h>

#include <ctype.h>
#include <limits.h>

int
onlp_oid_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    switch(ONLP_OID_TYPE_GET(oid))
        {
#define ONLP_OID_TYPE_ENTRY(_name, _value, _upper, _lower)              \
            case ONLP_OID_TYPE_##_name:                                 \
                return onlp_##_lower##_hdr_get(oid, hdr);

#include <onlp/onlp.x>
        }

    return ONLP_STATUS_E_PARAM;
}

int
onlp_oid_info_get(onlp_oid_t oid, onlp_oid_hdr_t** info)
{
    int rv;
    switch(ONLP_OID_TYPE_GET(oid))
        {
#define ONLP_OID_TYPE_ENTRY(_name, _value, _upper, _lower)              \
            case ONLP_OID_TYPE_##_name:                                 \
                {                                                       \
                    onlp_##_lower##_info_t* ip = aim_zmalloc(sizeof(*ip)); \
                    if(ONLP_SUCCESS(rv = onlp_##_lower##_info_get(oid, (onlp_##_lower##_info_t*)ip))) { \
                        *info = (onlp_oid_hdr_t*)ip;                    \
                    }                                                   \
                    else {                                              \
                        aim_free(ip);                                   \
                    }                                                   \
                    return rv;                                          \
                }
#include <onlp/onlp.x>
        }
    return ONLP_STATUS_E_PARAM;
}

int
onlp_oid_info_to_json(onlp_oid_hdr_t* info, cJSON** cj, uint32_t flags)
{
    switch(ONLP_OID_TYPE_GET(info->id))
        {
#define ONLP_OID_TYPE_ENTRY(_name, _value, _upper, _lower) \
            case ONLP_OID_TYPE_##_name: \
                return onlp_##_lower##_info_to_json((onlp_##_lower##_info_t*)info, cj, flags);
#include <onlp/onlp.x>
        }
    return ONLP_STATUS_E_PARAM;
}

int
onlp_oid_to_user_json(onlp_oid_t oid, cJSON** cjp, uint32_t flags)
{
    switch(ONLP_OID_TYPE_GET(oid))
        {
#define ONLP_OID_TYPE_ENTRY(_name, _value, _upper, _lower)              \
            case ONLP_OID_TYPE_##_name:                                 \
                {                                                       \
                    int rv;                                             \
                    onlp_##_lower##_info_t info;                        \
                    if(ONLP_SUCCESS(rv = onlp_##_lower##_info_get(oid, &info))) { \
                        return onlp_##_lower##_info_to_user_json(&info, cjp, flags); \
                    }                                                   \
                    return rv;                                          \
                }
#include <onlp/onlp.x>
        }
    return ONLP_STATUS_E_PARAM;




}

int
onlp_oid_to_json(onlp_oid_t oid, cJSON** cjp, uint32_t flags)
{
    if(flags & ONLP_OID_JSON_FLAG_TO_USER_JSON) {
        return onlp_oid_to_user_json(oid, cjp, flags);
    }

    switch(ONLP_OID_TYPE_GET(oid))
        {
#define ONLP_OID_TYPE_ENTRY(_name, _value, _upper, _lower)              \
            case ONLP_OID_TYPE_##_name:                                 \
                {                                                       \
                    int rv;                                             \
                    onlp_##_lower##_info_t info;                        \
                    if(ONLP_SUCCESS(rv = onlp_##_lower##_info_get(oid, &info))) { \
                        rv = onlp_##_lower##_info_to_json(&info, cjp, flags); \
                    }                                                   \
                    return rv;                                          \
                }
#include <onlp/onlp.x>
        }
    return ONLP_STATUS_E_PARAM;
}


int
onlp_oid_iterate(onlp_oid_t oid, onlp_oid_type_flags_t types,
                 onlp_oid_iterate_f itf, void* cookie)
{
    int rv;
    onlp_oid_hdr_t hdr;
    onlp_oid_t* oidp;

    if(oid == 0) {
        oid = ONLP_OID_CHASSIS;
    }

    rv = onlp_oid_hdr_get(oid, &hdr);
    if(rv < 0) {
        return rv;
    }

    /** Iterate over all top level ids */
    ONLP_OID_TABLE_ITER(hdr.coids, oidp) {
        if(ONLP_OID_IS_TYPE_FLAGSZ(types, *oidp)) {
            int rv = itf(*oidp, cookie);
            if(rv < 0) {
                return rv;
            }
        }
    }
    ONLP_OID_TABLE_ITER(hdr.coids, oidp) {
        rv = onlp_oid_iterate(*oidp, types, itf, cookie);
        if(rv < 0) {
            return rv;
        }
    }
    return ONLP_STATUS_OK;
}

typedef struct onlp_oid_get_all_ctrl_s {
    biglist_t* list;
    uint32_t flags;
    int rv;
} onlp_oid_get_all_ctrl_t;

static int
onlp_oid_info_get_all_iterate__(onlp_oid_t oid, void* cookie)
{
    int rv;
    onlp_oid_get_all_ctrl_t* ctrl = (onlp_oid_get_all_ctrl_t*)cookie;
    onlp_oid_hdr_t* obj;

    if(ONLP_SUCCESS(rv = onlp_oid_info_get(oid, &obj))) {
        ctrl->list = biglist_append(ctrl->list, obj);
        return 0;
    }
    else {
        ctrl->rv = rv;
        return -1;
    }
}

int
onlp_oid_info_get_all(onlp_oid_t root, onlp_oid_type_flags_t types,
                      uint32_t flags, biglist_t** list)
{
    onlp_oid_get_all_ctrl_t ctrl;

    ctrl.list = NULL;
    ctrl.flags = flags;
    ctrl.rv = 0;

    onlp_oid_iterate(root, types, onlp_oid_info_get_all_iterate__,
                     &ctrl);
    if(ONLP_SUCCESS(ctrl.rv)) {
        *list = ctrl.list;
    }
    else {
        onlp_oid_get_all_free(ctrl.list);
    }

    return ctrl.rv;
}

static int
onlp_oid_hdr_get_all_iterate__(onlp_oid_t oid, void* cookie)
{
    int rv;
    onlp_oid_get_all_ctrl_t* ctrl = (onlp_oid_get_all_ctrl_t*)cookie;
    onlp_oid_hdr_t* obj = aim_zmalloc(sizeof(*obj));
    if(ONLP_SUCCESS(rv = onlp_oid_hdr_get(oid, obj))) {
        ctrl->list = biglist_append(ctrl->list, obj);
        return 0;
    }
    else {
        ctrl->rv = rv;
        return -1;
    }
}


int
onlp_oid_hdr_get_all(onlp_oid_t root, onlp_oid_type_flags_t types,
                     uint32_t flags, biglist_t** list)
{
    onlp_oid_get_all_ctrl_t ctrl;

    ctrl.list = NULL;
    ctrl.flags = flags;
    ctrl.rv = 0;

    onlp_oid_iterate(root, types, onlp_oid_hdr_get_all_iterate__,
                     &ctrl);
    if(ONLP_SUCCESS(ctrl.rv)) {
        *list = ctrl.list;
    }
    else {
        onlp_oid_get_all_free(ctrl.list);
    }

    return ctrl.rv;
}

int
onlp_oid_get_all_free(biglist_t* list)
{
    biglist_free_all(list, aim_free);
    return 0;
}

int
onlp_oid_to_str(onlp_oid_t oid, char* rstr)
{
    switch(ONLP_OID_TYPE_GET(oid))
        {
#define ONLP_OID_TYPE_ENTRY(_upper, _id, _desc, _lower)                 \
            case _id:                                                   \
                {                                                       \
                    sprintf(rstr, "%s-%d", #_lower, ONLP_OID_ID_GET(oid)); \
                    return ONLP_STATUS_OK;                              \
                }
#include <onlp/onlp.x>

        default:
            return ONLP_STATUS_E_PARAM;
        }
}

int
onlp_oid_to_user_str(onlp_oid_t oid, char* rstr)
{
    switch(ONLP_OID_TYPE_GET(oid))
        {
#define ONLP_OID_TYPE_ENTRY(_name, _id, _upper, _lower)                 \
            case _id:                                                   \
                {                                                       \
                    if(_id == ONLP_OID_TYPE_PSU) {                      \
                        sprintf(rstr, "%s %d", #_upper, ONLP_OID_ID_GET(oid)); \
                    }                                                   \
                    else{                                               \
                        sprintf(rstr, "%s %d", #_lower, ONLP_OID_ID_GET(oid)); \
                        rstr[0] = toupper(rstr[0]);                     \
                    }                                                   \
                    return ONLP_STATUS_OK;                              \
                }
#include <onlp/onlp.x>
        default:
            return ONLP_STATUS_E_PARAM;
        }
}


int
onlp_oid_from_str(char* str, onlp_oid_t* oid)
{
#define ONLP_OID_TYPE_ENTRY(_upper, _tid, _desc, _lower)        \
    do {                                                        \
        int id;                                                 \
        if((sscanf(str, #_lower "-%d", &id) == 1) ||            \
           (sscanf(str, #_upper "-%d", &id) == 1)) {            \
            *oid = ONLP_OID_TYPE_CREATE(_tid, id);              \
            return ONLP_STATUS_OK;                              \
        }                                                       \
    } while(0);
#include <onlp/onlp.x>
    ONLP_LOG_JSON("%s : could not convert 'str' to an oid.",
                  __FUNCTION__, str);
    return ONLP_STATUS_E_PARAM;
}

int
onlp_oid_table_to_json(onlp_oid_table_t table, cJSON** cjp)
{
    int rv;
    cJSON* cj = cJSON_CreateArray();
    onlp_oid_t* oidp;
    ONLP_OID_TABLE_ITER(table, oidp) {
        char str[32];
        if(ONLP_FAILURE(rv = onlp_oid_to_str(*oidp, str))) {
            cJSON_Delete(cj);
            return rv;
        }
        cJSON_AddItemToArray(cj, cJSON_CreateString(str));
    }
    *cjp = cj;
    return ONLP_STATUS_OK;
}

int
onlp_oid_table_from_json(cJSON* cj, onlp_oid_table_t table)
{
    int i, rv;

    int s = cJSON_GetArraySize(cj);
    if(s < 0 || s > ONLP_OID_TABLE_SIZE) {
        return ONLP_STATUS_E_PARAM;
    }

    for(i = 0; i < ONLP_OID_TABLE_SIZE; i++) {
        cJSON* item = cJSON_GetArrayItem(cj, i);
        if(!item) {
            break;
        }

        if(item->type != cJSON_String) {
            return ONLP_STATUS_E_PARAM;
        }

        if(ONLP_FAILURE(rv = onlp_oid_from_str(item->valuestring, table+i))) {
            return rv;
        }
    }
    return ONLP_STATUS_OK;
}

int
onlp_oid_hdr_to_json(onlp_oid_hdr_t* hdr, cJSON** cjp, uint32_t flags)
{
    int rv;
    char str[32];

    if(ONLP_FAILURE(rv = onlp_oid_to_str(hdr->id, str))) {
        return ONLP_STATUS_E_PARAM;
    }

    cJSON* cj = cJSON_CreateObject();
    cJSON_AddStringToObject(cj, "id", str);

    if(hdr->description[0]) {
        cJSON_AddStringToObject(cj, "description", hdr->description);
    }
    else {
        cJSON_AddNullToObject(cj, "description");
    }
    if(hdr->poid) {
        if(ONLP_FAILURE(rv = onlp_oid_to_str(hdr->poid, str))) {
            goto error;
        }
        cJSON_AddStringToObject(cj, "poid", str);
    }
    else {
        cJSON_AddNullToObject(cj, "poid");
    }

    cJSON* coids = NULL;
    if(ONLP_FAILURE(rv = onlp_oid_table_to_json(hdr->coids, &coids))) {
        goto error;
    }

    cJSON_AddItemToObject(cj, "coids", coids);
    cJSON* status = cjson_util_flag_array(hdr->status, onlp_oid_status_flag_map);
    cJSON_AddItemToObject(cj, "status", status);

    *cjp = cj;
    return ONLP_STATUS_OK;

 error:
    cJSON_Delete(cj);
    return ONLP_STATUS_E_PARAM;
}

int
onlp_oid_hdr_from_json(cJSON* cj, onlp_oid_hdr_t* hdr)
{
    int rv;
    char* str;

    memset(hdr, 0, sizeof(*hdr));

    cJSON* jhdr = NULL;
    if(ONLP_SUCCESS(cjson_util_lookup(cj, &jhdr, "hdr")) && jhdr) {
        cj = jhdr;
    }

    if(cjson_util_lookup_string(cj, &str, "id") < 0) {
        ONLP_LOG_JSON("%s: 'id' entry not found.", __FUNCTION__);
        return ONLP_STATUS_E_PARAM;
    }
    if(ONLP_FAILURE(rv = onlp_oid_from_str(str, &hdr->id))) {
        ONLP_LOG_JSON("%s: onlp_oid_from_str failed: %{onlp_status}",
                      __FUNCTION__, rv);
        return rv;
    }

    if(ONLP_SUCCESS(cjson_util_lookup_string(cj, &str, "description"))) {
        aim_strlcpy(hdr->description, str, sizeof(hdr->description));
    }

    if(ONLP_SUCCESS(cjson_util_lookup_string(cj, &str, "poid"))) {
        if(ONLP_FAILURE(rv = onlp_oid_from_str(str, &hdr->poid))) {
            return rv;
        }
    }
    cJSON* coids = cJSON_GetObjectItem(cj, "coids");
    if(coids) {
        if(ONLP_FAILURE(rv = onlp_oid_table_from_json(coids, hdr->coids))) {
            return rv;
        }
    }
    cJSON* status = cJSON_GetObjectItem(cj, "status");
    if(status) {
        if(ONLP_FAILURE(rv = cjson_util_array_to_flags(status,
                                                       &hdr->status,
                                                       onlp_oid_status_flag_map))) {
            return rv;
        }
    }

    return ONLP_STATUS_OK;
}

int
onlp_oid_from_json(cJSON* cj, onlp_oid_hdr_t** rhdr,
                   biglist_t** all_oids_list, uint32_t flags)
{
    int rv;
    onlp_oid_hdr_t hdr;

    if(cj == NULL) {
        ONLP_LOG_JSON("%s : JSON pointer is null.", __FUNCTION__);
        return ONLP_STATUS_E_PARAM;
    }

    if(flags & ONLP_OID_JSON_FLAG_RECURSIVE) {
        if(all_oids_list == NULL) {
            ONLP_LOG_JSON("%s : recursive selected but the return list is null.",
                          __FUNCTION__);
            return ONLP_STATUS_E_PARAM;
        }
    }
    else {
        if(rhdr == NULL) {
            ONLP_LOG_JSON("%s : rhdr pointer is null.", __FUNCTION__);
            return ONLP_STATUS_E_PARAM;
        }
    }

    if(ONLP_FAILURE(rv = onlp_oid_hdr_from_json(cj, &hdr))) {
        ONLP_LOG_JSON("%s: onlp_oid_hdr_from_json failed: %{onlp_status}",
                      __FUNCTION__, rv);
        return rv;
    }

    switch(ONLP_OID_TYPE_GET(hdr.id))
        {
#define ONLP_OID_TYPE_ENTRY(_name, _value, _upper, _lower)              \
            case ONLP_OID_TYPE_##_name:                                 \
                {                                                       \
                    onlp_##_lower##_info_t* pinfo = aim_zmalloc(sizeof(*pinfo)); \
                    if(ONLP_FAILURE(rv = onlp_##_lower##_info_from_json(cj, pinfo))) { \
                        ONLP_LOG_JSON("%s: onlp_%s_info_from_json failed: %{onlp_status}", \
                                      __FUNCTION__, #_lower, rv);                     \
                        aim_free(pinfo);                                \
                        return rv;                                      \
                    }                                                   \
                    if(flags & ONLP_OID_JSON_FLAG_RECURSIVE) {          \
                        cJSON* children;                                \
                        if(ONLP_SUCCESS(cjson_util_lookup(cj, &children, "coids"))) { \
                            int i;                                      \
                            for(i = 0; i < cJSON_GetArraySize(children); i++) { \
                                cJSON* child = cJSON_GetArrayItem(children, i); \
                                rv = onlp_oid_from_json(child, NULL, all_oids_list, flags); \
                                if(ONLP_FAILURE(rv)) {                  \
                                    aim_free(pinfo);                    \
                                    return rv;                          \
                                }                                       \
                            }                                           \
                        }                                               \
                    }                                                   \
                    if(rhdr) {                                          \
                        *rhdr = (onlp_oid_hdr_t*)pinfo;                 \
                    }                                                   \
                    if(all_oids_list) {                                 \
                        *all_oids_list = biglist_prepend(*all_oids_list, pinfo); \
                    }                                                   \
                    return 0;                                           \
                }
#include <onlp/onlp.x>
        }
    return ONLP_STATUS_E_PARAM;
}


int
onlp_oid_json_verify(onlp_oid_t oid)
{
    int rv;
    cJSON* cj;

    switch(ONLP_OID_TYPE_GET(oid))
        {

#define ONLP_OID_TYPE_ENTRY(_name, _value, _upper, _lower)              \
            case ONLP_OID_TYPE_##_name:                                 \
                {                                                       \
                    onlp_##_lower##_info_t info1, info2;                \
                    memset(&info1, 0, sizeof(info1));                   \
                    memset(&info2, 0, sizeof(info2));                   \
                    if(ONLP_FAILURE(rv = onlp_##_lower##_info_get(oid, &info1))) { \
                        AIM_LOG_ERROR("onlp_%s_info_get)(%{onlp_oid}) failed: %{onlp_status}", \
                                      #_lower, oid, rv);                \
                        return rv;                                      \
                                      }                                 \
                                      if(ONLP_FAILURE(rv = onlp_##_lower##_info_to_json(&info1, &cj, 0))) { \
                                      AIM_LOG_ERROR("onlp_%s_info_to_json(%{onlp_oid}) failed: %{onlp_status}", \
                                      #_lower, oid);                    \
                                      return rv;                        \
                                      }                                 \
                                      if(ONLP_FAILURE(rv = onlp_##_lower##_info_from_json(cj, &info2))) { \
                                      AIM_LOG_ERROR("onlp_%s_info_from_json(%{onlp_oid}) failed: %{onlp_status}", \
                                                    #_lower, oid, rv);  \
                                      return rv;                        \
                                                    }                   \
                                                    if(memcmp(&info1, &info2, sizeof(info1))) { \
                                                        AIM_LOG_ERROR("info1/info2 mismatch for %{onlp_oid}", \
                                                        oid);           \
                                                        cJSON* cj2;     \
                                                        onlp_##_lower##_info_to_json(&info2, &cj2, 0); \
                                                        AIM_LOG_ERROR("cj1: %s", cJSON_Print(cj)); \
                                                        AIM_LOG_ERROR("cj2: %s", cJSON_Print(cj2)); \
                                                        return -1;      \
                                                        }               \
                                                        return 0;       \
                                                        }
#include <onlp/onlp.x>
        }


    return -1;
}

static int oid_compare__(const void* a, const void* b)
{
    onlp_oid_t A = *((onlp_oid_t*) a);
    onlp_oid_t B = *((onlp_oid_t*) b);

    if(A == 0) A = UINT_MAX;
    if(B == 0) B = UINT_MAX;

    if(A < B) {
        return -1;
    }

    if(A > B) {
        return 1;
    }

    return 0;
}

void
onlp_oid_hdr_sort(onlp_oid_hdr_t* hdr)
{
    qsort(hdr->coids, sizeof(onlp_oid_t), AIM_ARRAYSIZE(hdr->coids),
          oid_compare__);
}

/* Local variables: */
/* c-file-style: "cc-mode" */
/* End: */
