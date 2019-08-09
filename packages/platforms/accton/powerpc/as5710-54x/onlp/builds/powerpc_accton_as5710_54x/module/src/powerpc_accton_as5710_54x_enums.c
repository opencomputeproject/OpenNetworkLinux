/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <powerpc_accton_as5710_54x/powerpc_accton_as5710_54x_config.h>
#include "powerpc_accton_as5710_54x_int.h"

/* <auto.start.enum(ALL).source> */
aim_map_si_t platform_id_map[] =
{
    { "powerpc-accton-as5710-54x-r0b", PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_R0B },
    { "powerpc-accton-as5710-54x-r0", PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_R0 },
    { NULL, 0 }
};

aim_map_si_t platform_id_desc_map[] =
{
    { "None", PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_R0B },
    { "None", PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_R0 },
    { NULL, 0 }
};

const char*
platform_id_name(platform_id_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, platform_id_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'platform_id'";
    }
}

int
platform_id_value(const char* str, platform_id_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, platform_id_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
platform_id_desc(platform_id_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, platform_id_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'platform_id'";
    }
}

/* <auto.end.enum(ALL).source> */

