/**************************************************************************//**
 *
 * x86_64_quanta_ix7_rglbmc Internal Header
 *
 *****************************************************************************/
#ifndef __X86_64_QUANTA_IX7_RGLBMC_INT_H__
#define __X86_64_QUANTA_IX7_RGLBMC_INT_H__

#include <x86_64_quanta_ix7_rglbmc/x86_64_quanta_ix7_rglbmc_config.h>
#include <limits.h>

/* <auto.start.enum(ALL).header> */
/** thermal_oid */
typedef enum thermal_oid_e {
	THERMAL_OID_THERMAL68 = ONLP_THERMAL_ID_CREATE(68),
	THERMAL_OID_THERMAL69 = ONLP_THERMAL_ID_CREATE(69),
	THERMAL_OID_THERMAL70 = ONLP_THERMAL_ID_CREATE(70),
	THERMAL_OID_THERMAL71 = ONLP_THERMAL_ID_CREATE(71),
	THERMAL_OID_THERMAL72 = ONLP_THERMAL_ID_CREATE(72),
	THERMAL_OID_THERMAL73 = ONLP_THERMAL_ID_CREATE(73),
	THERMAL_OID_THERMAL74 = ONLP_THERMAL_ID_CREATE(74),
	THERMAL_OID_THERMAL75 = ONLP_THERMAL_ID_CREATE(75),
	THERMAL_OID_THERMAL76 = ONLP_THERMAL_ID_CREATE(76),

	THERMAL_OID_THERMAL43 = ONLP_THERMAL_ID_CREATE(43),
	THERMAL_OID_THERMAL44 = ONLP_THERMAL_ID_CREATE(44),
	THERMAL_OID_THERMAL45 = ONLP_THERMAL_ID_CREATE(45),
	THERMAL_OID_THERMAL53 = ONLP_THERMAL_ID_CREATE(53),
	THERMAL_OID_THERMAL54 = ONLP_THERMAL_ID_CREATE(54),
	THERMAL_OID_THERMAL55 = ONLP_THERMAL_ID_CREATE(55),
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

#define QUANTA_IX7_PSU1_PIN_ID		41
#define QUANTA_IX7_PSU2_PIN_ID		51
#define QUANTA_IX7_PSU1_POUT_ID		42
#define QUANTA_IX7_PSU2_POUT_ID		52
#define QUANTA_IX7_PSU1_TEMP1_ID	43
#define QUANTA_IX7_PSU1_TEMP2_ID	44
#define QUANTA_IX7_PSU1_TEMP3_ID	45
#define QUANTA_IX7_PSU2_TEMP1_ID	53
#define QUANTA_IX7_PSU2_TEMP2_ID	54
#define QUANTA_IX7_PSU2_TEMP3_ID	55
#define QUANTA_IX7_PSU1_FAN_ID		40
#define QUANTA_IX7_PSU2_FAN_ID		50
#define QUANTA_IX7_PSU1_VIN_ID		46
#define QUANTA_IX7_PSU2_VIN_ID		56
#define QUANTA_IX7_PSU1_VOUT_ID		47
#define QUANTA_IX7_PSU2_VOUT_ID		57
#define QUANTA_IX7_PSU1_CIN_ID 		38
#define QUANTA_IX7_PSU2_CIN_ID 		48
#define QUANTA_IX7_PSU1_COUT_ID		39
#define QUANTA_IX7_PSU2_COUT_ID		49

/** psu_oid */
typedef enum psu_oid_e {
    PSU_OID_PSU101 = ONLP_PSU_ID_CREATE(101),
    PSU_OID_PSU102 = ONLP_PSU_ID_CREATE(102),
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
    THERMAL_ID_THERMAL1 = 68,
    THERMAL_ID_THERMAL2 = 69,
    THERMAL_ID_THERMAL3 = 70,
    THERMAL_ID_THERMAL4 = 71,
    THERMAL_ID_THERMAL5 = 72,
    THERMAL_ID_THERMAL6 = 73,
    THERMAL_ID_THERMAL7 = 74,
    THERMAL_ID_THERMAL8 = 75,
    THERMAL_ID_THERMAL9 = 76,

    THERMAL_ID_THERMAL10 = 43,
    THERMAL_ID_THERMAL11 = 44,
    THERMAL_ID_THERMAL12 = 45,
    THERMAL_ID_THERMAL13 = 53,
    THERMAL_ID_THERMAL14 = 54,
    THERMAL_ID_THERMAL15 = 55,
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
    FAN_ID_FAN1 = 21,
    FAN_ID_FAN2 = 22,
    FAN_ID_FAN3 = 23,
    FAN_ID_FAN4 = 24,
    FAN_ID_FAN5 = 25,
    FAN_ID_FAN6 = 26,
    FAN_ID_FAN7 = 27,
    FAN_ID_FAN8 = 28,
    FAN_ID_FAN9 = 29,
    FAN_ID_FAN10 = 30,
    FAN_ID_FAN11 = 31,
    FAN_ID_FAN12 = 32,

    FAN_ID_FAN13 = 40,
    FAN_ID_FAN14 = 50,
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
    PSU_ID_PSU1 = 101,
    PSU_ID_PSU2 = 102,
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
	FAN_OID_FAN21 = ONLP_FAN_ID_CREATE(21),
	FAN_OID_FAN22 = ONLP_FAN_ID_CREATE(22),
	FAN_OID_FAN23 = ONLP_FAN_ID_CREATE(23),
	FAN_OID_FAN24 = ONLP_FAN_ID_CREATE(24),
	FAN_OID_FAN25 = ONLP_FAN_ID_CREATE(25),
	FAN_OID_FAN26 = ONLP_FAN_ID_CREATE(26),
	FAN_OID_FAN27 = ONLP_FAN_ID_CREATE(27),
	FAN_OID_FAN28 = ONLP_FAN_ID_CREATE(28),
	FAN_OID_FAN29 = ONLP_FAN_ID_CREATE(29),
	FAN_OID_FAN30 = ONLP_FAN_ID_CREATE(30),
	FAN_OID_FAN31 = ONLP_FAN_ID_CREATE(31),
	FAN_OID_FAN32 = ONLP_FAN_ID_CREATE(32),

	FAN_OID_FAN40 = ONLP_FAN_ID_CREATE(40),
	FAN_OID_FAN50 = ONLP_FAN_ID_CREATE(50),
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

#define SYS_HWMON_PREFIX "/sys/class/hwmon/hwmon1"
#endif /* __X86_64_QUANTA_IX7_RGLBMC_INT_H__ */
