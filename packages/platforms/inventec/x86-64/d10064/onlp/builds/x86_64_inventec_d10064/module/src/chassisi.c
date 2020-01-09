#include <onlp/platformi/chassisi.h>
#include "platform_lib.h"

static onlp_oid_t __oid_info[] = {
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_PHY),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_CORE0),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_CORE1),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_CORE2),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_CORE3),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_1_ON_MAIN_BROAD),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_2_ON_MAIN_BROAD),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_3_ON_MAIN_BROAD),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_4_ON_MAIN_BROAD),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_5_ON_MAIN_BROAD),
    ONLP_FAN_ID_CREATE(ONLP_FAN_1),
    ONLP_FAN_ID_CREATE(ONLP_FAN_2),
    ONLP_FAN_ID_CREATE(ONLP_FAN_3),
    ONLP_FAN_ID_CREATE(ONLP_FAN_4),
    ONLP_PSU_ID_CREATE(ONLP_PSU_1),
    ONLP_PSU_ID_CREATE(ONLP_PSU_2),
    ONLP_LED_ID_CREATE(ONLP_LED_MGMT),
};

int
onlp_chassisi_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    int i;
    ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
    ONLP_OID_STATUS_FLAG_SET(hdr, OPERATIONAL);

    memcpy(hdr->coids, __oid_info, sizeof(__oid_info));

    /** Add 64 SFP OIDs after the static table */
    onlp_oid_t* e = hdr->coids + AIM_ARRAYSIZE(__oid_info);
    /* 64 SFPs */
    for(i = 1; i <= 64; i++) {
        *e++ = ONLP_SFP_ID_CREATE(i);
    }

    return ONLP_STATUS_OK;
}
