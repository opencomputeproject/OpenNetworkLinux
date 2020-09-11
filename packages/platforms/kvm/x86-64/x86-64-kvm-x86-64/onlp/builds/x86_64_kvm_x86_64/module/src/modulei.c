#include <onlp/platformi/modulei.h>

/*
 * this abuses the description field to put PIU information
 * since onlp_module_info_t is generic and doesn't have fields for them
 */
static onlp_module_info_t minfo[] = {
    {},
    {
        .hdr = {
            .description = "{\"location\": \"0\"}"
        },
        .status = 1,
    }
};

int onlp_modulei_info_get(onlp_oid_t id, onlp_module_info_t *info) {
    *info = minfo[ONLP_OID_ID_GET(id)];
    return ONLP_STATUS_OK;
}

int onlp_modulei_status_get(onlp_oid_t id, uint32_t* status) {
    *status = minfo[ONLP_OID_ID_GET(id)].status;
    return ONLP_STATUS_OK;
}
