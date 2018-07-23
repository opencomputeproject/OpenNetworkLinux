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
#include <onlp/onlp.h>
#include "onlp_int.h"

/**
 * The OID dump() and show() routines
 * need a default IOF.
 */
void
onlp_oid_dump_iof_init_default(iof_t* iof, aim_pvs_t* pvs)
{
    if(iof_init(iof, pvs) == 0) {
        /* Default settings */
        iof->indent_factor=4;
        iof->level=1;
        iof->indent_terminator="";
    }
}
void
onlp_oid_show_iof_init_default(iof_t* iof, aim_pvs_t* pvs, uint32_t flags)
{
    if(iof_init(iof, pvs) == 0) {
        /* Default settings */
        iof->indent_factor=2;
        iof->level=1;
        iof->indent_terminator="";
        iof->pop_string = NULL;
        iof->push_string = "";
    }
}

void
onlp_oid_info_get_error(iof_t* iof, int error)
{
    iof_iprintf(iof, "Error retrieving status: %{onlp_status}", error);
}
void
onlp_oid_show_description(iof_t* iof, onlp_oid_hdr_t* hdr)
{
    iof_iprintf(iof, "Description: %s", hdr->description);
}

void
onlp_oid_show_state_missing(iof_t* iof)
{
    iof_iprintf(iof, "State: Missing");
}

/**
 * Create the initial JSON object when serializing an info structure.
 */
int
onlp_info_to_json_create(onlp_oid_hdr_t* hdr, cJSON** cjp, uint32_t flags)
{
    int rv;
    cJSON* cj_hdr;
    if(ONLP_FAILURE(rv = onlp_oid_hdr_to_json(hdr, &cj_hdr, flags))) {
        return rv;
    }
    cJSON* cj = cJSON_CreateObject();
    cJSON_AddItemToObject(cj, "hdr", cj_hdr);
    *cjp = cj;
    return 0;
}
int
onlp_info_to_user_json_create(onlp_oid_hdr_t* hdr, cJSON** cjp, uint32_t flags)
{
    int rv;
    cJSON* object = cJSON_CreateObject();
    cjson_util_add_string_to_object(object, "Description", hdr->description[0] ? hdr->description : "None");

    if(hdr->status & ONLP_OID_STATUS_FLAG_PRESENT) {
        cjson_util_add_string_to_object(object, "State", "Present");

        char* status = "Unknown";
        if(hdr->status & ONLP_OID_STATUS_FLAG_UNPLUGGED) {
            status = "Unplugged";
            rv = 0;
        }
        else if(hdr->status & ONLP_OID_STATUS_FLAG_FAILED) {
            switch(ONLP_OID_TYPE_GET(hdr->id))
                {
                case ONLP_OID_TYPE_PSU: status = "Failed or Unplugged."; break;
                default: status = "Failed"; break;
                }
            rv = 0;
        }
        else {
            switch(ONLP_OID_TYPE_GET(hdr->id))
                {
                case ONLP_OID_TYPE_CHASSIS:
                case ONLP_OID_TYPE_THERMAL:
                    status = "Functional"; break;
                default:
                    status = "Running"; break;
                }
            rv = 1;
        }
        cjson_util_add_string_to_object(object, "Status", status);
    }
    else {
        cjson_util_add_string_to_object(object, "State", "Missing");
        rv = 0;
    }

    *cjp = object;
    return rv;
}


int
onlp_info_to_user_json_finish(onlp_oid_hdr_t* hdr, cJSON* object, cJSON** cjp,
                              uint32_t flags)
{
    char name[64];
    onlp_oid_to_user_str(hdr->id, name);
    if(*cjp) {
        cJSON_AddItemToObject(*cjp, name, object);
    }
    else {
        *cjp = object;
    }

    if(flags & ONLP_OID_JSON_FLAG_RECURSIVE) {
        onlp_oid_t* oidp;
        ONLP_OID_TABLE_ITER(hdr->coids, oidp) {
            onlp_oid_to_user_json(*oidp, &object, flags);
        }
    }
    return 0;
}
int
onlp_info_to_json_finish(onlp_oid_hdr_t* hdr, cJSON* object, cJSON** cjp,
                         uint32_t flags)
{
    char name[64];
    onlp_oid_to_str(hdr->id, name);

    if(flags & ONLP_OID_JSON_FLAG_RECURSIVE) {
        onlp_oid_t* oidp;
        cJSON* children = cJSON_CreateObject();
        ONLP_OID_TABLE_ITER(hdr->coids, oidp) {
            onlp_oid_to_json(*oidp, &children, flags);
        }
        if(cJSON_GetArraySize(children) > 0) {
            cJSON_AddItemToObject(object, "coids", children);
        }
        else {
            cJSON_Delete(children);
        }
    }

    if(*cjp) {
        cJSON_AddItemToObject(*cjp, name, object);
    }
    else {
        *cjp = object;
    }

    return 0;
}
