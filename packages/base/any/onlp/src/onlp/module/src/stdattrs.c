/**************************************************************************//**
 *
 *
 *
 *
 *****************************************************************************/
#include <onlp/stdattrs.h>
#include <onlp/onlp.h>
#include <cjson_util/cjson_util_format.h>

int
onlp_asset_info_free(onlp_asset_info_t* aip)
{
    if(!aip) {
        return ONLP_STATUS_E_PARAM;
    }

#define ONLP_ASSET_INFO_ENTRY_str(_field, _name)        \
    do {                                                \
        aim_free(aip->_field);                          \
    }  while(0);
#define ONLP_ASSET_INFO_ENTRY(_field, _name, _type) \
    ONLP_ASSET_INFO_ENTRY_##_type(_field, _name)

    #include <onlp/onlp.x>
#undef ONLP_ASSET_INFO_ENTRY_str

    return 0;
}

int
onlp_asset_info_to_json(onlp_asset_info_t* aip, cJSON** rv)
{
    cJSON* cj = cJSON_CreateObject();

#define ONLP_ASSET_INFO_ENTRY_str(_field, _name) \
    do {                                                                \
        if(aip->_field) {                                               \
            cJSON_AddStringToObject(cj, #_name, aip->_field);           \
        } else {                                                        \
            cJSON_AddNullToObject(cj, #_name);                          \
        }                                                               \
    } while(0);

#define ONLP_ASSET_INFO_ENTRY(_field, _name, _type) \
    ONLP_ASSET_INFO_ENTRY_##_type(_field, _name)
#include <onlp/onlp.x>
#undef ONLP_ASSET_INFO_str

    *rv = cj;
    return 0;
}

int
onlp_asset_info_show(onlp_asset_info_t* aip, aim_pvs_t* pvs)
{
    int rv;
    cJSON* cj;
    if(ONLP_SUCCESS(rv = onlp_asset_info_to_json(aip, &cj))) {
        cjson_util_yaml_pvs(pvs, cj);
        cJSON_Delete(cj);
    }
    return rv;
}
