#include <onlp/platformi/chassisi.h>
#include <AIM/aim.h>

#include <onlp_platform_sim/oids.h>
#include "onlp_platform_sim_log.h"

int
onlp_chassisi_sw_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_chassisi_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    return onlp_platform_sim_chassis_get(oid, hdr, NULL, NULL);
}

int
onlp_chassisi_info_get(onlp_oid_t oid, onlp_chassis_info_t* info)
{
    return onlp_platform_sim_chassis_get(oid, NULL, info, NULL);
}
