#include <onlp/platformi/thermali.h>
#include <AIM/aim.h>

#include <onlp_platform_sim/oids.h>
#include "onlp_platform_sim_log.h"

int
onlp_thermali_sw_init(void)
{
    return 0;
}


int
onlp_thermali_hw_init(uint32_t flags)
{
    return 0;
}

int
onlp_thermali_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    return onlp_platform_sim_thermal_get(oid, hdr, NULL, NULL);
}

int
onlp_thermali_info_get(onlp_oid_t oid, onlp_thermal_info_t* info)
{
    return onlp_platform_sim_thermal_get(oid, NULL, info, NULL);
}
