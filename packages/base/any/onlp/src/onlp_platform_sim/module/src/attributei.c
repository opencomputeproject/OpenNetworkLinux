#include <onlp/platformi/attributei.h>
#include <onlp/stdattrs.h>
#include <onlplib/file.h>

int
onlp_attributei_supported(onlp_oid_t id, const char* attribute)
{
    if(!strcmp(attribute, ONLP_ATTRIBUTE_ONIE_INFO) ||
       !strcmp(attribute, ONLP_ATTRIBUTE_ASSET_INFO)) {
        return 1;
    }
    return 0;
}

int
onlp_attributei_get(onlp_oid_t id, const char* attribute,
                    void** value)
{
    if(!strcmp(attribute, ONLP_ATTRIBUTE_ONIE_INFO)) {
        /** TODO: fix this fake data using JSON and onie-sysinfo */
        onlp_onie_info_t* oip = aim_zmalloc(sizeof(*oip));
        list_init(&oip->vx_list);
        oip->product_name = aim_strdup("ONLP Platform Simulation.");
        extern char* onlp_platform_sim_platform_name;
        oip->platform_name = aim_strdup(onlp_platform_sim_platform_name);
        oip->manufacturer = aim_strdup("ONL");
        *value = oip;
        return ONLP_STATUS_OK;
    }

    if(!strcmp(attribute, ONLP_ATTRIBUTE_ASSET_INFO)) {
        onlp_asset_info_t* aip = aim_zmalloc(sizeof(*aip));
        aip->oid = id;
        aip->firmware_revision = aim_fstrdup("1.2.3");
        *value = aip;
        return ONLP_STATUS_OK;
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_attributei_free(onlp_oid_t id, const char* attribute, void* value)
{
    if(!strcmp(attribute, ONLP_ATTRIBUTE_ONIE_INFO)) {
        onlp_onie_info_free(value);
        aim_free(value);
        return 0;

    }

    if(!strcmp(attribute, ONLP_ATTRIBUTE_ASSET_INFO)) {
        onlp_asset_info_free(value);
        aim_free(value);
        return 0;
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}
