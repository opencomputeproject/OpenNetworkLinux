#include <onlp/platformi/attributei.h>
#include <onlp/stdattrs.h>
#include <onlplib/file.h>
#include "platform_lib.h"

int
onlp_attributei_onie_info_get(onlp_oid_t oid, onlp_onie_info_t* rp)
{
    if(oid != ONLP_OID_CHASSIS) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if(rp == NULL) {
        return ONLP_STATUS_OK;
    }

    return onlp_onie_decode_file(rp, IDPROM_PATH);
}

int
onlp_attributei_asset_info_get(onlp_oid_t oid, onlp_asset_info_t* rp)
{
    if(oid != ONLP_OID_CHASSIS) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if(rp == NULL) {
        return ONLP_STATUS_OK;
    }

    rp->oid = oid;

    rp->manufacturer = aim_strdup("Accton");

    char* versions[] = {
        "/sys/bus/i2c/devices/0-0060/version",
        "/sys/bus/i2c/devices/0-0061/version",
        "/sys/bus/i2c/devices/0-0062/version",
        NULL,
    };

    onlp_file_join_files(&rp->firmware_revision,
                         ".", versions);

    return ONLP_STATUS_OK;
}
