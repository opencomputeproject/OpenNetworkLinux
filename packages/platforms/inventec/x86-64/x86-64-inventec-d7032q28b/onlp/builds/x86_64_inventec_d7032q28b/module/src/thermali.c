/************************************************************
 * thermali.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <onlp/platformi/base.h>
#include "platform_lib.h"

typedef struct thermali_path_s {
    char file[ONLP_CONFIG_INFO_STR_MAX];
} thermali_path_t;

#define MAKE_THERMAL_PATH_ON_MAIN_BROAD(id)   	{ INV_HWMON_PREFIX"temp"#id"_input"}
#define MAKE_THERMAL1_PATH_ON_PSU(psu_id)	{ INV_HWMON_PREFIX"thermal_psu"#psu_id}
#define MAKE_THERMAL2_PATH_ON_PSU(psu_id)       { INV_HWMON_PREFIX"thermal2_psu"#psu_id}

static thermali_path_t __path_list[ ] = {
    {},
    {},
    {},
    {},
    {},
    MAKE_THERMAL_PATH_ON_MAIN_BROAD(1),
    MAKE_THERMAL_PATH_ON_MAIN_BROAD(2),
    MAKE_THERMAL_PATH_ON_MAIN_BROAD(3),
    MAKE_THERMAL_PATH_ON_MAIN_BROAD(4),
    MAKE_THERMAL_PATH_ON_MAIN_BROAD(5),
    MAKE_THERMAL1_PATH_ON_PSU(1),
    MAKE_THERMAL1_PATH_ON_PSU(2),
};

/* Static values */
static onlp_thermal_info_t __onlp_thermal_info[ ] = {
    {},
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_CPU_CORE_FIRST, "CPU Physical"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_CPU_CORE_3, "CPU Core1"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_CPU_CORE_4, "CPU Core2"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_CPU_CORE_LAST, "CPU Core3"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_1_ON_MAIN_BROAD, "Thermal Sensor 1"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_2_ON_MAIN_BROAD, "Thermal Sensor 2"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_3_ON_MAIN_BROAD, "Thermal Sensor 3"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_4_ON_MAIN_BROAD, "Thermal Sensor 4"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(THERMAL_5_ON_MAIN_BROAD, "Thermal Sensor 5"),
    ONLP_PSU_THERMAL_INFO_ENTRY_INIT(THERMAL_1_ON_PSU1, "PSU-1 Thermal Sensor 1", PSU1_ID),
    ONLP_PSU_THERMAL_INFO_ENTRY_INIT(THERMAL_1_ON_PSU2, "PSU-2 Thermal Sensor 1", PSU2_ID),
};

int
onlp_thermali_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, THERMAL_CPU_CORE_FIRST, THERMAL_1_ON_PSU2);
}

int
onlp_thermali_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    ONLP_TRY( ONLP_OID_ID_VALIDATE_RANGE(id, THERMAL_CPU_CORE_FIRST, THERMAL_1_ON_PSU2) );
    *hdr = __onlp_thermal_info[id].hdr;

    return ONLP_STATUS_OK;
}

int
onlp_thermali_info_get(onlp_oid_id_t id, onlp_thermal_info_t* info)
{
    int ret;

    ONLP_TRY( ONLP_OID_ID_VALIDATE_RANGE(id, THERMAL_CPU_CORE_FIRST, THERMAL_1_ON_PSU2) );
    /* Set the onlp_oid_hdr_t and capabilities */
    *info = __onlp_thermal_info[id];
    if(id>=THERMAL_CPU_CORE_FIRST&&id<=THERMAL_CPU_CORE_LAST) {
        snprintf(__path_list[id].file,ONLP_CONFIG_INFO_STR_MAX,"%stemp%d_input",GET_CTMP_PREFIX,id+1);
    }
    ret = onlp_file_read_int(&info->mcelsius, __path_list[id].file);

    return ret;
}
