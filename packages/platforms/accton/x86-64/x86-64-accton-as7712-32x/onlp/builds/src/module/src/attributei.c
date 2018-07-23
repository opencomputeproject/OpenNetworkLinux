#include <onlp/platformi/attributei.h>
#include <onlp/stdattrs.h>
#include <onlplib/file.h>

#include "platform_lib.h"

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
    int rv;
    if(!strcmp(attribute, ONLP_ATTRIBUTE_ONIE_INFO)) {
        onlp_onie_info_t* oip = aim_zmalloc(sizeof(*oip));
        if(ONLP_SUCCESS(rv = onlp_onie_decode_file(oip, IDPROM_PATH))) {
            *value = oip;
        }
        else {
            aim_free(oip);
        }
        return rv;
    }

    if(!strcmp(attribute, ONLP_ATTRIBUTE_ASSET_INFO)) {
        onlp_asset_info_t* aip = aim_zmalloc(sizeof(*aip));
        aip->oid = id;

        int v[3] = {0};
        if(ONLP_SUCCESS(rv = onlp_file_read_int(v, "/sys/bus/i2c/devices/4-0060/version")) &&
           ONLP_SUCCESS(rv = onlp_file_read_int(v+1, "/sys/bus/i2c/devices/5-0062/version")) &&
           ONLP_SUCCESS(rv = onlp_file_read_int(v+2, "/sys/bus/i2c/devices/6-0064/version"))) {

            aip->firmware_revision =
                aim_fstrdup("%d.%d.%d", v[0], v[1], v[2]);
            *value = aip;
        }
        else {
            aim_free(aip);
        }
        return rv;
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
