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
        .status = ONLP_MODULE_STATUS_PIU_ACO_PRESENT | ONLP_MODULE_STATUS_PIU_CFP2_PRESENT,
    },
    {
        .hdr = {
            .description = "{\"location\": \"1\"}"
        },
        .status = ONLP_MODULE_STATUS_PIU_DCO_PRESENT | ONLP_MODULE_STATUS_PIU_CFP2_PRESENT,
    },
    {
        .hdr = {
            .description = "{\"location\": \"2\"}"
        },
        .status = ONLP_MODULE_STATUS_PIU_QSFP28_PRESENT | ONLP_MODULE_STATUS_PIU_QSFP28_1_PRESENT | ONLP_MODULE_STATUS_PIU_QSFP28_2_PRESENT,
    },
    {
        .hdr = {
            .description = "{\"location\": \"3\"}"
        },
        .status = ONLP_MODULE_STATUS_UNPLUGGED,
    },
    {
        .hdr = {
            .description = "{\"location\": \"4\"}"
        },
        .status = ONLP_MODULE_STATUS_PIU_ACO_PRESENT,
    },
    {
        .hdr = {
            .description = "{\"location\": \"5\"}"
        },
        .status = ONLP_MODULE_STATUS_PIU_DCO_PRESENT,
    },
};

int onlp_modulei_info_get(onlp_oid_t id, onlp_module_info_t *info) {
    *info = minfo[ONLP_OID_ID_GET(id)];
    return ONLP_STATUS_OK;
}

int onlp_modulei_status_get(onlp_oid_t id, uint32_t* status) {
    *status = minfo[ONLP_OID_ID_GET(id)].status;
    return ONLP_STATUS_OK;
}
