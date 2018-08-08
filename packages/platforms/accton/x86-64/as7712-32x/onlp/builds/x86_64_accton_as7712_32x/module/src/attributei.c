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
        return 1;
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
        return 1;
    }

    int rv;
    int v[3] = {0};

    rp->oid = oid;
    if(ONLP_SUCCESS(rv = onlp_file_read_int(v, "/sys/bus/i2c/devices/4-0060/version")) &&
       ONLP_SUCCESS(rv = onlp_file_read_int(v+1, "/sys/bus/i2c/devices/5-0062/version")) &&
       ONLP_SUCCESS(rv = onlp_file_read_int(v+2, "/sys/bus/i2c/devices/6-0064/version"))) {

        rp->firmware_revision =
            aim_fstrdup("%d.%d.%d", v[0], v[1], v[2]);
    }
    return rv;
}
