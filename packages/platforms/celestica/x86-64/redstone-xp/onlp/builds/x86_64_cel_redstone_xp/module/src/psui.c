#include <onlp/platformi/psui.h>

#include "i2c_chips.h"
#include "platform.h"

static
onlp_psu_info_t psu_info[] =
{
    {
            { ONLP_PSU_ID_CREATE(1), "PSU-1", 0 },
    },
    {
            { ONLP_PSU_ID_CREATE(2), "PSU-2", 0 },
    }
};

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int psu_id;
    struct psuInfo psu;

    psu_id = ONLP_OID_ID_GET(id) - 1;
    *info = psu_info[psu_id];

    if (!getPsuPresent(psu_id))
        info->status |= ONLP_PSU_STATUS_PRESENT;
    else
        return ONLP_STATUS_E_MISSING;

    getPsuInfo(psu_id, &psu);

    info->mvin = psu.vin;
    info->mvout = psu.vout;
    info->miin  = psu.iin;
    info->miout = psu.iout;
    info->mpin = psu.pin;
    info->mpout = psu.pout;

    if (!(info->mpin))
        info->status |= ONLP_PSU_STATUS_UNPLUGGED;

    return ONLP_STATUS_OK;
}
