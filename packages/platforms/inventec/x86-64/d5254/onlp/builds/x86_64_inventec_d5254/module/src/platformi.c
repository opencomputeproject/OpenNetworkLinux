
#include <onlp/platformi/base.h>
#include "platform_lib.h"

const char*
onlp_platformi_get(void)
{
    return "x86-64-inventec-d5254-r0";
}

int
onlp_platformi_sw_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_platformi_manage_fans(void)
{
    /*Ensure switch manager is working*/
    PLATFORM_PSOC_DIAG_LOCK;
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_platformi_manage_leds(void)
{
    /*Ensure switch manager is working*/
    PLATFORM_PSOC_DIAG_LOCK;
    return ONLP_STATUS_E_UNSUPPORTED;
}
