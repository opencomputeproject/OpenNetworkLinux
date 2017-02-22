/**************************************************************************//**
 *
 * powerpc_quanta_ly2 Internal Header
 *
 *****************************************************************************/
#ifndef __POWERPC_QUANTA_LY2_R0_INT_H__
#define __POWERPC_QUANTA_LY2_R0_INT_H__

#include <powerpc_quanta_ly2/powerpc_quanta_ly2_config.h>

/* <auto.start.enum(ALL).header> */
/** thermal_oid */
typedef enum thermal_oid_e {
    THERMAL_OID_THERMAL1 = ONLP_THERMAL_ID_CREATE(1),
    THERMAL_OID_THERMAL2 = ONLP_THERMAL_ID_CREATE(2),
    THERMAL_OID_THERMAL3 = ONLP_THERMAL_ID_CREATE(3),
    THERMAL_OID_THERMAL4 = ONLP_THERMAL_ID_CREATE(4),
    THERMAL_OID_THERMAL5 = ONLP_THERMAL_ID_CREATE(5),
    THERMAL_OID_THERMAL6 = ONLP_THERMAL_ID_CREATE(6),
    THERMAL_OID_THERMAL7 = ONLP_THERMAL_ID_CREATE(7),
    THERMAL_OID_THERMAL8 = ONLP_THERMAL_ID_CREATE(8),
    THERMAL_OID_THERMAL9 = ONLP_THERMAL_ID_CREATE(9),
    THERMAL_OID_THERMAL10 = ONLP_THERMAL_ID_CREATE(10),
    THERMAL_OID_THERMAL11 = ONLP_THERMAL_ID_CREATE(11),
} thermal_oid_t;

/** Enum names. */
const char* thermal_oid_name(thermal_oid_t e);

/** Enum values. */
int thermal_oid_value(const char* str, thermal_oid_t* e, int substr);

/** Enum descriptions. */
const char* thermal_oid_desc(thermal_oid_t e);

/** Enum validator. */
int thermal_oid_valid(thermal_oid_t e);

/** validator */
#define THERMAL_OID_VALID(_e) \
    (thermal_oid_valid((_e)))

/** thermal_oid_map table. */
extern aim_map_si_t thermal_oid_map[];
/** thermal_oid_desc_map table. */
extern aim_map_si_t thermal_oid_desc_map[];

/** psu_oid */
typedef enum psu_oid_e {
    PSU_OID_PSU1 = ONLP_PSU_ID_CREATE(1),
    PSU_OID_PSU2 = ONLP_PSU_ID_CREATE(2),
} psu_oid_t;

/** Enum names. */
const char* psu_oid_name(psu_oid_t e);

/** Enum values. */
int psu_oid_value(const char* str, psu_oid_t* e, int substr);

/** Enum descriptions. */
const char* psu_oid_desc(psu_oid_t e);

/** Enum validator. */
int psu_oid_valid(psu_oid_t e);

/** validator */
#define PSU_OID_VALID(_e) \
    (psu_oid_valid((_e)))

/** psu_oid_map table. */
extern aim_map_si_t psu_oid_map[];
/** psu_oid_desc_map table. */
extern aim_map_si_t psu_oid_desc_map[];

/** thermal_id */
typedef enum thermal_id_e {
    THERMAL_ID_THERMAL1 = 1,
    THERMAL_ID_THERMAL2 = 2,
    THERMAL_ID_THERMAL3 = 3,
    THERMAL_ID_THERMAL4 = 4,
    THERMAL_ID_THERMAL5 = 5,
    THERMAL_ID_THERMAL6 = 6,
    THERMAL_ID_THERMAL7 = 7,
    THERMAL_ID_THERMAL8 = 8,
    THERMAL_ID_THERMAL9 = 9,
    THERMAL_ID_THERMAL10 = 10,
    THERMAL_ID_THERMAL11 = 11,
} thermal_id_t;

/** Enum names. */
const char* thermal_id_name(thermal_id_t e);

/** Enum values. */
int thermal_id_value(const char* str, thermal_id_t* e, int substr);

/** Enum descriptions. */
const char* thermal_id_desc(thermal_id_t e);

/** Enum validator. */
int thermal_id_valid(thermal_id_t e);

/** validator */
#define THERMAL_ID_VALID(_e) \
    (thermal_id_valid((_e)))

/** thermal_id_map table. */
extern aim_map_si_t thermal_id_map[];
/** thermal_id_desc_map table. */
extern aim_map_si_t thermal_id_desc_map[];

/** fan_id */
typedef enum fan_id_e {
    FAN_ID_FAN1 = 1,
    FAN_ID_FAN2 = 2,
    FAN_ID_FAN3 = 3,
    FAN_ID_FAN4 = 4,
    FAN_ID_FAN5 = 5,
    FAN_ID_FAN6 = 6,
} fan_id_t;

/** Enum names. */
const char* fan_id_name(fan_id_t e);

/** Enum values. */
int fan_id_value(const char* str, fan_id_t* e, int substr);

/** Enum descriptions. */
const char* fan_id_desc(fan_id_t e);

/** Enum validator. */
int fan_id_valid(fan_id_t e);

/** validator */
#define FAN_ID_VALID(_e) \
    (fan_id_valid((_e)))

/** fan_id_map table. */
extern aim_map_si_t fan_id_map[];
/** fan_id_desc_map table. */
extern aim_map_si_t fan_id_desc_map[];

/** psu_id */
typedef enum psu_id_e {
    PSU_ID_PSU1 = 1,
    PSU_ID_PSU2 = 2,
} psu_id_t;

/** Enum names. */
const char* psu_id_name(psu_id_t e);

/** Enum values. */
int psu_id_value(const char* str, psu_id_t* e, int substr);

/** Enum descriptions. */
const char* psu_id_desc(psu_id_t e);

/** Enum validator. */
int psu_id_valid(psu_id_t e);

/** validator */
#define PSU_ID_VALID(_e) \
    (psu_id_valid((_e)))

/** psu_id_map table. */
extern aim_map_si_t psu_id_map[];
/** psu_id_desc_map table. */
extern aim_map_si_t psu_id_desc_map[];

/** fan_oid */
typedef enum fan_oid_e {
    FAN_OID_FAN1 = ONLP_FAN_ID_CREATE(1),
    FAN_OID_FAN2 = ONLP_FAN_ID_CREATE(2),
    FAN_OID_FAN3 = ONLP_FAN_ID_CREATE(3),
    FAN_OID_FAN4 = ONLP_FAN_ID_CREATE(4),
    FAN_OID_FAN5 = ONLP_FAN_ID_CREATE(5),
    FAN_OID_FAN6 = ONLP_FAN_ID_CREATE(6),
} fan_oid_t;

/** Enum names. */
const char* fan_oid_name(fan_oid_t e);

/** Enum values. */
int fan_oid_value(const char* str, fan_oid_t* e, int substr);

/** Enum descriptions. */
const char* fan_oid_desc(fan_oid_t e);

/** Enum validator. */
int fan_oid_valid(fan_oid_t e);

/** validator */
#define FAN_OID_VALID(_e) \
    (fan_oid_valid((_e)))

/** fan_oid_map table. */
extern aim_map_si_t fan_oid_map[];
/** fan_oid_desc_map table. */
extern aim_map_si_t fan_oid_desc_map[];
/* <auto.end.enum(ALL).header> */


#define SYS_HWMON_PREFIX "/sys/devices/soc@ffe00000/ffe03000.i2c/i2c-0/i2c-4/4-002e"

#endif /* __POWERPC_QUANTA_LY2_R0_INT_H__ */
