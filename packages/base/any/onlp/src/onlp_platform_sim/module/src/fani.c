#include <onlp/platformi/fani.h>
#include <AIM/aim.h>

#include <onlp_platform_sim/oids.h>
#include "onlp_platform_sim_log.h"

int
onlp_fani_sw_init(void)
{
    return 0;
}

int
onlp_fani_hw_init(uint32_t flags)
{
    return 0;
}

int
onlp_fani_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    return onlp_platform_sim_fan_get(oid, hdr, NULL, NULL);
}

int
onlp_fani_info_get(onlp_oid_t oid, onlp_fan_info_t* info)
{
    return onlp_platform_sim_fan_get(oid, NULL, info, NULL);
}
