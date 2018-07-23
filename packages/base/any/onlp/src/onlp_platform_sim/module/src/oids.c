/**************************************************************************//**
 *
 *
 *
 *
 *****************************************************************************/
#include <onlp_platform_sim/oids.h>

#include <AIM/aim.h>
#include <onlp/oids.h>

#include <onlp/chassis.h>
#include <onlp/psu.h>
#include <onlp/fan.h>
#include <onlp/thermal.h>
#include <onlp/led.h>
#include <onlp/sfp.h>
#include <onlp/oids.h>
#include <onlp/module.h>
#include <onlp/generic.h>

#include <onlplib/file.h>

#include "onlp_platform_sim_log.h"

#include <cjson_util/cjson_util_file.h>

struct {
    biglist_t* oid_list;
    cjson_util_file_t cjf;
} ctrl__;


static int
rebuild__(void)
{
    /* Delete old entries */
    if(ctrl__.oid_list) {
        biglist_free_all(ctrl__.oid_list, aim_free);
        ctrl__.oid_list = NULL;
    }
    /* Rebuild current entries */
    int rv = onlp_oid_from_json(ctrl__.cjf.root, NULL, &ctrl__.oid_list,
                                ONLP_OID_JSON_FLAG_RECURSIVE);
    if(ONLP_FAILURE(rv)) {
        AIM_LOG_ERROR("error rebuilding from json file %s: %{onlp_status}",
                      ctrl__.cjf.filename);
    }
    return rv;
}

int
onlp_platform_sim_oids_init(const char* fname)
{
    /*
     * If the requested file doesn't exist then
     * we pre-populate it with the builtin default.
     */
    if(onlp_file_exists(fname) != 1) {
        extern char onlp_platform_sim_default_json[];
        cJSON* cj = cJSON_Parse(onlp_platform_sim_default_json);
        if(cj == NULL) {
            AIM_LOG_ERROR("The default JSON contents could not be parsed.");
            return ONLP_STATUS_E_INTERNAL;
        }
        char* s = cJSON_Print(cj);
        onlp_file_write_str(s, fname);
        free(s);
    }

    if(cjson_util_file_open(fname, &ctrl__.cjf, NULL) < 0) {
        AIM_DIE("could not open json file.");
    }

    if(ONLP_FAILURE(rebuild__())) {
        AIM_DIE("rebuild failed.");
    }
    return 0;
}

biglist_t*
reload__(void)
{
    if(cjson_util_file_reload(&ctrl__.cjf, 0) == 1) {
        rebuild__();
    }
    return ctrl__.oid_list;
}

onlp_oid_hdr_t*
onlp_platform_sim_oid_lookup(onlp_oid_t oid)
{
    biglist_t* ble;
    onlp_oid_hdr_t* hdr;
    biglist_t* list = reload__();
    BIGLIST_FOREACH_DATA(ble, list, onlp_oid_hdr_t*, hdr) {
        if(hdr->id == oid) {
            return hdr;
        }
    }
    return NULL;
}

#define ONLP_OID_TYPE_ENTRY(_name, _id, _upper, _lower)  \
    int                                                  \
    onlp_platform_sim_##_lower##_get(onlp_oid_t oid,                    \
                                     onlp_oid_hdr_t* hdr,               \
                                     onlp_##_lower##_info_t* info,      \
                                     onlp_##_lower##_info_t** pinfo)    \
    {                                                                   \
        if(!ONLP_OID_IS_TYPE(ONLP_OID_TYPE_##_name, oid)) {             \
            AIM_LOG_ERROR("%s: %{onlp_oid} is not a %s oid.",           \
                          __FUNCTION__, oid, #_lower);                  \
            return ONLP_STATUS_E_PARAM;                                 \
        }                                                               \
        onlp_##_lower##_info_t* p = (onlp_##_lower##_info_t*)           \
            onlp_platform_sim_oid_lookup(oid);                          \
                                                                        \
        if(!p) {                                                        \
            AIM_LOG_ERROR("%s: %{onlp_oid} does not exist.",            \
                          __FUNCTION__, oid);                           \
            return ONLP_STATUS_E_MISSING;                               \
        }                                                               \
        if(info) {                                                      \
            memcpy(info, p, sizeof(*info));                             \
        }                                                               \
        if(hdr) {                                                       \
            memcpy(hdr, &p->hdr, sizeof(*hdr));                         \
        }                                                               \
        if(pinfo) {                                                     \
            *pinfo = p;                                                 \
        }                                                               \
        return 0;                                                       \
    }
#include <onlp/onlp.x>
