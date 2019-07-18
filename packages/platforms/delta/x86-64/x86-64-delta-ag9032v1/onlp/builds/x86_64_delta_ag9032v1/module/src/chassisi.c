#include <onlp/platformi/chassisi.h>
#include "platform_lib.h"

int
onlp_chassisi_sw_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_chassisi_sw_denit(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_chassisi_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    int i;
    onlp_oid_t* e = hdr->coids;

    ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
    ONLP_OID_STATUS_FLAG_SET(hdr, OPERATIONAL);

    /* 6 Thermal sensors on the chassis */
    for (i = 1; i <= NUM_OF_THERMAL_ON_BOARDS; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 4 LEDs on the chassis */
    for (i = 1; i <= NUM_OF_LED_ON_BOARDS; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= NUM_OF_PSU_ON_PSU_BOARD; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 5 Fans on the chassis */
    for (i = 1; i <= NUM_OF_FAN_ON_FAN_BOARD; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 32 SFPs */
    for(i = 1; i <= 32; i++) {
        *e++ = ONLP_SFP_ID_CREATE(i);
    }
    return ONLP_STATUS_OK;
}

int
onlp_chassisi_info_get(onlp_oid_id_t id, onlp_chassis_info_t* info)
{
    onlp_chassisi_hdr_get(id, &info->hdr);
    return ONLP_STATUS_OK;
}
