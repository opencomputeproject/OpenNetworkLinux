#include <onlp/platformi/platformi.h>
#include <onlp_platform_sim/oids.h>

const char*
onlp_platformi_get(void)
{
    return "onlp-platform-sim";
}

char* onlp_platform_sim_platform_name = NULL;

int
onlp_platformi_set(const char* platform)
{
    onlp_platform_sim_platform_name = aim_strdup(platform);
    return 0;
}

int
onlp_platformi_sw_init(void)
{
    char* json = getenv("ONLP_PLATFORM_SIM_JSON");
    if(!json) {
        json = "/lib/platform-config/current/onl/onlp-platform-sim.json";
    }
    return onlp_platform_sim_oids_init(json);
}

int
onlp_platformi_manage_init(void)
{
    return 0;
}

int
onlp_platformi_manage_fans(void)
{
    return 0;
}

int
onlp_platformi_manage_leds(void)
{
    return 0;
}
