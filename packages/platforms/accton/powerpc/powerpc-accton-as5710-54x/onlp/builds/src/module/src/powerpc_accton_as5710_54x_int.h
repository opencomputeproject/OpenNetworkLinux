/**************************************************************************//**
 *
 * powerpc_accton_as5710_54x Internal Header
 *
 *****************************************************************************/
#ifndef __POWERPC_ACCTON_AS5710_54X_INT_H__
#define __POWERPC_ACCTON_AS5710_54X_INT_H__

#include <powerpc_accton_as5710_54x/powerpc_accton_as5710_54x_config.h>

/* <auto.start.enum(ALL).header> */
/** platform_id */
typedef enum platform_id_e {
    PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_R0B,
    PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_R0,
    PLATFORM_ID_LAST = PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_R0,
    PLATFORM_ID_COUNT,
    PLATFORM_ID_INVALID = -1,
} platform_id_t;

/** Strings macro. */
#define PLATFORM_ID_STRINGS \
{\
    "powerpc-accton-as5710-54x-r0b", \
    "powerpc-accton-as5710-54x-r0", \
}
/** Enum names. */
const char* platform_id_name(platform_id_t e);

/** Enum values. */
int platform_id_value(const char* str, platform_id_t* e, int substr);

/** Enum descriptions. */
const char* platform_id_desc(platform_id_t e);

/** validator */
#define PLATFORM_ID_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_R0))

/** platform_id_map table. */
extern aim_map_si_t platform_id_map[];
/** platform_id_desc_map table. */
extern aim_map_si_t platform_id_desc_map[];
/* <auto.end.enum(ALL).header> */

#endif /* __POWERPC_ACCTON_AS5710_54X_INT_H__ */
