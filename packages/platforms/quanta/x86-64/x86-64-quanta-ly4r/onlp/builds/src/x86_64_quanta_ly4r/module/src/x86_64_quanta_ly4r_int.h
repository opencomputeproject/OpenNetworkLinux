/**************************************************************************//**
 *
 * x86_64_quanta_ly4r Internal Header
 *
 *****************************************************************************/
#ifndef __X86_64_QUANTA_LY4R_INT_H__
#define __X86_64_QUANTA_LY4R_INT_H__

#include <x86_64_quanta_ly4r/x86_64_quanta_ly4r_config.h>
#include <limits.h>

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
    THERMAL_OID_THERMAL12 = ONLP_THERMAL_ID_CREATE(12),
    THERMAL_OID_THERMAL13 = ONLP_THERMAL_ID_CREATE(13),
    THERMAL_OID_THERMAL14 = ONLP_THERMAL_ID_CREATE(14),
    THERMAL_OID_THERMAL15 = ONLP_THERMAL_ID_CREATE(15),
    THERMAL_OID_THERMAL16 = ONLP_THERMAL_ID_CREATE(16),
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
    THERMAL_ID_THERMAL12 = 12,
    THERMAL_ID_THERMAL13 = 13,
    THERMAL_ID_THERMAL14 = 14,
    THERMAL_ID_THERMAL15 = 15,
    THERMAL_ID_THERMAL16 = 16,
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
    FAN_ID_FAN7 = 7,
    FAN_ID_FAN8 = 8,
    FAN_ID_FAN9 = 9,
    FAN_ID_FAN10 = 10,
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
    FAN_OID_FAN7 = ONLP_FAN_ID_CREATE(7),
    FAN_OID_FAN8 = ONLP_FAN_ID_CREATE(8),
    FAN_OID_FAN9 = ONLP_FAN_ID_CREATE(9),
    FAN_OID_FAN10 = ONLP_FAN_ID_CREATE(10),
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

/* psu info table */
struct psu_info_s {
	char path[PATH_MAX];
	int present;
	int busno;
	int addr;
};

/** led_id */
typedef enum led_id_e {
    LED_ID_SYSTEM = 1,
} led_id_t;

/** Enum names. */
const char* led_id_name(led_id_t e);

/** Enum values. */
int led_id_value(const char* str, led_id_t* e, int substr);

/** Enum descriptions. */
const char* led_id_desc(led_id_t e);

/** Enum validator. */
int led_id_valid(led_id_t e);

/** validator */
#define LED_ID_VALID(_e) \
    (led_id_valid((_e)))

/** led_id_map table. */
extern aim_map_si_t led_id_map[];
/** led_id_desc_map table. */
extern aim_map_si_t led_id_desc_map[];

/** led_oid */
typedef enum led_oid_e {
    LED_OID_SYSTEM     = ONLP_LED_ID_CREATE(LED_ID_SYSTEM),
} led_oid_t;

/** Enum names. */
const char* led_oid_name(led_oid_t e);

/** Enum values. */
int led_oid_value(const char* str, led_oid_t* e, int substr);

/** Enum descriptions. */
const char* led_oid_desc(led_oid_t e);

/** Enum validator. */
int led_oid_valid(led_oid_t e);

/** validator */
#define LED_OID_VALID(_e) \
    (led_oid_valid((_e)))

/** led_oid_map table. */
extern aim_map_si_t led_oid_map[];
/** led_oid_desc_map table. */
extern aim_map_si_t led_oid_desc_map[];
/* <auto.end.enum(ALL).header> */

#define SYS_HWMON_PREFIX "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/0-004e"

#endif /* __X86_64_QUANTA_LY4R_INT_H__ */
