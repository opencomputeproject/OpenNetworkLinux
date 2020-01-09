#include <onlp/platformi/attributei.h>
#include <onlp/stdattrs.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <unistd.h>
#include <fcntl.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_delta_ag9032v1_int.h"
#include "x86_64_delta_ag9032v1_log.h"
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

    int cpld_version = 0;
    int swpld_version = 0;
    char manufacturer[3] = "DNI";
    rp->oid = oid;
    rp->manufacturer = aim_fstrdup("%s",manufacturer);
    cpld_version = onlp_i2c_readb(I2C_BUS_2, CPUCPLD, CPUPLD_VERSION_ADDR, DEFAULT_FLAG);
    swpld_version = dni_lock_swpld_read_attribute(SWPLD_VERSION_ADDR);
    rp->firmware_revision = aim_fstrdup("%d.%d", cpld_version, swpld_version);
    
    return ONLP_STATUS_OK;
}
