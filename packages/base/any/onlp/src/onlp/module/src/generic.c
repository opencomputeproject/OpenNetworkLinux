#include <onlp/generic.h>
#include <onlp/platformi/generici.h>
#include "onlp_int.h"
#include "onlp_locks.h"
#include "onlp_log.h"

static int
onlp_generic_sw_init_locked__(void)
{
    return onlp_generici_sw_init();
}
ONLP_LOCKED_API0(onlp_generic_sw_init);

static int
onlp_generic_hw_init_locked__(uint32_t flags)
{
    return onlp_generici_hw_init(flags);
}
ONLP_LOCKED_API1(onlp_generic_hw_init, uint32_t, flags);


static int
onlp_generic_sw_denit_locked__(void)
{
    return onlp_generici_sw_denit();
}
ONLP_LOCKED_API0(onlp_generic_sw_denit);

static int
onlp_generic_hdr_get_locked__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    int rv;
    onlp_oid_id_t id;

    ONLP_OID_GENERIC_VALIDATE_GET_ID(oid, id);
    ONLP_PTR_VALIDATE_ZERO(hdr);

    rv = onlp_log_error(0,
                        onlp_generici_hdr_get(id, hdr),
                        "generici hdr get %{onlp_oid}", oid);
    hdr->id = oid;
    return rv;
}
ONLP_LOCKED_API2(onlp_generic_hdr_get, onlp_oid_t, oid, onlp_oid_hdr_t*, hdr)

static int
onlp_generic_info_get_locked__(onlp_oid_t oid, onlp_generic_info_t* info)
{
    int rv;
    onlp_oid_id_t id;

    ONLP_OID_GENERIC_VALIDATE_GET_ID(oid, id);
    ONLP_PTR_VALIDATE_ZERO(info);

    rv = onlp_log_error(0,
                        onlp_generici_info_get(id, info),
                        "generici info get %{onlp_oid}", oid);

    info->hdr.id = oid;
    return rv;
}
ONLP_LOCKED_API2(onlp_generic_info_get, onlp_oid_t, oid, onlp_generic_info_t*, info)

int
onlp_generic_info_to_user_json(onlp_generic_info_t* info, cJSON** cjp, uint32_t flags)
{
    cJSON* object;

    onlp_info_to_user_json_create(&info->hdr, &object, flags);
    return onlp_info_to_user_json_finish(&info->hdr, object, cjp, flags);
}

int
onlp_generic_info_to_json(onlp_generic_info_t* info, cJSON** cjp, uint32_t flags)
{
    cJSON* cj;

    ONLP_IF_ERROR_RETURN(onlp_info_to_json_create(&info->hdr, &cj, flags));
    return onlp_info_to_json_finish(&info->hdr, cj, cjp, flags);
}

int
onlp_generic_info_from_json(cJSON* cj, onlp_generic_info_t* info)
{
    memset(info, 0, sizeof(*info));

    ONLP_IF_ERROR_RETURN(onlp_oid_hdr_from_json(cj, &info->hdr));
    return 0;
}
