#include <onlp/generic.h>

int
onlp_generic_sw_init(void)
{
    return 0;
}

int
onlp_generic_hw_init(uint32_t flags)
{
    return 0;
}

int
onlp_generic_sw_denit(void)
{
    return 0;
}

int
onlp_generic_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_generic_info_get(onlp_oid_t id, onlp_generic_info_t* info)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_generic_format(onlp_oid_t oid, onlp_oid_format_t format,
                    aim_pvs_t* pvs, uint32_t flags)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_generic_info_format(onlp_generic_info_t* info,
                         onlp_oid_format_t format,
                         aim_pvs_t* pvs, uint32_t flags)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_generic_info_to_user_json(onlp_generic_info_t* info, cJSON** rv, uint32_t flags)
{
    return 0;
}

int
onlp_generic_info_to_json(onlp_generic_info_t* info, cJSON** rv, uint32_t flags)
{
    return 0;
}

int
onlp_generic_info_from_json(cJSON* cj, onlp_generic_info_t* info)
{
    return 0;
}
