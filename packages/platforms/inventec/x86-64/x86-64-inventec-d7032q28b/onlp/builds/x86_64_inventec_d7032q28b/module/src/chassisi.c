#include <onlp/platformi/chassisi.h>
#include "platform_lib.h"

int
onlp_chassisi_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    int i;
    onlp_oid_t* e = hdr->coids;

    ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
    ONLP_OID_STATUS_FLAG_SET(hdr, OPERATIONAL);

    /* Thermal sensors on the chassis */
    for (i = THERMAL_CPU_CORE_FIRST; i <= THERMAL_5_ON_MAIN_BROAD; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    for (i = FAN_1_ON_MAIN_BOARD; i <= FAN_4_ON_MAIN_BOARD; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    for (i = LED_SYS; i <= LED_SYS; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    for (i = PSU1_ID; i <= PSU2_ID; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* SFPs */
    for(i = 1; i <= CHASSIS_PORT_COUNT; i++) {
        *e++ = ONLP_SFP_ID_CREATE(i);
    }
    return 0;
}
