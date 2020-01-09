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
#include <unistd.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <fcntl.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"
#include <sys/types.h>
#include <dirent.h>

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)


typedef struct thermali_path_s {
    char file[ONLP_CONFIG_INFO_STR_MAX];
} thermali_path_t;

#define PSU1_ADDR "58"
#define PSU2_ADDR "59"

static char* thermal_i2c_addr[]= {
    "",
    "",
    "",
    "",
    "",
    "",
    "3-0018",
    "3-0018",
    "3-0048",
    "3-0049",
    "3-004a",
    "3-004d",
    "3-004e"
};

static thermali_path_t __path_list[ONLP_THERMAL_MAX] ;

#define MAKE_THERMAL_INFO_NODE_ON_CPU_PHY \
    {   { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_PHY), "CPU Physical", 0}, \
        ONLP_THERMAL_STATUS_PRESENT, \
        ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS \
    }
#define MAKE_THERMAL_INFO_NODE_ON_CPU_CORE(id) \
    {   { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_CORE##id), "CPU Core "#id, 0},\
        ONLP_THERMAL_STATUS_PRESENT, \
        ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS \
    }
#define MAKE_THERMAL_INFO_NODE_ON_BROADS(id) \
    {   { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_##id##_ON_MAIN_BROAD), "Thermal Sensor "#id, 0}, \
        ONLP_THERMAL_STATUS_PRESENT, \
        ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS \
    }
#define MAKE_THERMAL_INFO_NODE_ON_PSU(thermal_id, psu_id) \
    {   { \
            ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_##thermal_id##_ON_PSU##psu_id), \
           "PSU-"#psu_id" Thermal Sensor "#thermal_id, \
            ONLP_PSU_ID_CREATE(ONLP_PSU_##psu_id) \
        }, \
        ONLP_THERMAL_STATUS_PRESENT, \
        ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS \
    }

/* Static values */
static onlp_thermal_info_t __onlp_thermal_info[ ] = {
    {},
    MAKE_THERMAL_INFO_NODE_ON_CPU_PHY,
    MAKE_THERMAL_INFO_NODE_ON_CPU_CORE(0),
    MAKE_THERMAL_INFO_NODE_ON_CPU_CORE(1),
    MAKE_THERMAL_INFO_NODE_ON_CPU_CORE(2),
    MAKE_THERMAL_INFO_NODE_ON_CPU_CORE(3),
    MAKE_THERMAL_INFO_NODE_ON_BROADS(1),
    MAKE_THERMAL_INFO_NODE_ON_BROADS(2),
    MAKE_THERMAL_INFO_NODE_ON_BROADS(3),
    MAKE_THERMAL_INFO_NODE_ON_BROADS(4),
    MAKE_THERMAL_INFO_NODE_ON_BROADS(5),
    MAKE_THERMAL_INFO_NODE_ON_BROADS(6),
    MAKE_THERMAL_INFO_NODE_ON_BROADS(7),
    MAKE_THERMAL_INFO_NODE_ON_PSU(1,1),
    MAKE_THERMAL_INFO_NODE_ON_PSU(2,1),
    MAKE_THERMAL_INFO_NODE_ON_PSU(3,1),
    MAKE_THERMAL_INFO_NODE_ON_PSU(1,2),
    MAKE_THERMAL_INFO_NODE_ON_PSU(2,2),
    MAKE_THERMAL_INFO_NODE_ON_PSU(3,2),
};


static int _get_hwmon_path( char* parent_dir, char* target_path)
{

    DIR * dir;
    struct dirent * ptr;
    dir = opendir(parent_dir);
    char* buf=NULL;
    int ret=ONLP_STATUS_E_INVALID;

    while( (ptr = readdir(dir))!=NULL ) {
        buf=ptr->d_name;
        if( strncmp(buf,"hwmon",5)==0 ) {
            ret=ONLP_STATUS_OK;
            break;
        }
    }
    closedir(dir);
    if(ret==ONLP_STATUS_OK) {
        snprintf(target_path, ONLP_CONFIG_INFO_STR_MAX, "%s%s/", parent_dir , buf);
    } else {
        printf("[ERROR] Can't find valid path\n");
    }

    return ret;

}

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    char base[ONLP_CONFIG_INFO_STR_MAX];
    int thermal_id=ONLP_THERMAL_CPU_PHY;
    int ret=ONLP_STATUS_OK;

    for(thermal_id=ONLP_THERMAL_CPU_PHY; thermal_id<ONLP_THERMAL_MAX; thermal_id++) {
        switch(thermal_id) {
        case ONLP_THERMAL_CPU_PHY:
        case ONLP_THERMAL_CPU_CORE0:
        case ONLP_THERMAL_CPU_CORE1:
        case ONLP_THERMAL_CPU_CORE2:
        case ONLP_THERMAL_CPU_CORE3:
            ret=_get_hwmon_path(INV_CTMP_BASE,base);
            snprintf(__path_list[thermal_id].file,ONLP_CONFIG_INFO_STR_MAX,"%stemp%d_input",base,thermal_id-ONLP_THERMAL_CPU_PHY+1);
            break;
        case ONLP_THERMAL_1_ON_MAIN_BROAD:
        case ONLP_THERMAL_2_ON_MAIN_BROAD:
            snprintf(__path_list[thermal_id].file,ONLP_CONFIG_INFO_STR_MAX,INV_PSU_BASE"%s/hwmon/",thermal_i2c_addr[thermal_id]);
            ret=_get_hwmon_path(__path_list[thermal_id].file,base);
            snprintf(__path_list[thermal_id].file,ONLP_CONFIG_INFO_STR_MAX,"%stemp%d_input",base,thermal_id-ONLP_THERMAL_1_ON_MAIN_BROAD+1);
            break;
        case ONLP_THERMAL_3_ON_MAIN_BROAD:
        case ONLP_THERMAL_4_ON_MAIN_BROAD:
        case ONLP_THERMAL_5_ON_MAIN_BROAD:
        case ONLP_THERMAL_6_ON_MAIN_BROAD:
        case ONLP_THERMAL_7_ON_MAIN_BROAD:
            snprintf(__path_list[thermal_id].file,ONLP_CONFIG_INFO_STR_MAX,INV_PSU_BASE"%s/hwmon/",thermal_i2c_addr[thermal_id]);
            ret=_get_hwmon_path(__path_list[thermal_id].file,base);
            snprintf(__path_list[thermal_id].file,ONLP_CONFIG_INFO_STR_MAX,"%stemp1_input",base);
            break;
        case ONLP_THERMAL_1_ON_PSU1:
        case ONLP_THERMAL_2_ON_PSU1:
        case ONLP_THERMAL_3_ON_PSU1:
            snprintf(__path_list[thermal_id].file,ONLP_CONFIG_INFO_STR_MAX,INV_PSU_BASE"%d-00"PSU1_ADDR"/temp%d_input",PSU_I2C_CHAN,thermal_id-ONLP_THERMAL_1_ON_PSU1+1);
            break;
        case ONLP_THERMAL_1_ON_PSU2:
        case ONLP_THERMAL_2_ON_PSU2:
        case ONLP_THERMAL_3_ON_PSU2:
            snprintf(__path_list[thermal_id].file,ONLP_CONFIG_INFO_STR_MAX,INV_PSU_BASE"%d-00"PSU2_ADDR"/temp%d_input",PSU_I2C_CHAN,thermal_id-ONLP_THERMAL_1_ON_PSU2+1);
            break;
        }
        if(ret!=ONLP_STATUS_OK) {
            break;
        }
    }

    return ONLP_STATUS_OK;
}


/*
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */


int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    VALIDATE(id);
    int ret;
    int thermal_id = ONLP_OID_ID_GET(id);
    if(thermal_id >= ONLP_THERMAL_MAX) {
        return ONLP_STATUS_E_INVALID;
    }

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = __onlp_thermal_info[ thermal_id];
    ret = onlp_thermali_status_get(id, &info->status);
    if( ret != ONLP_STATUS_OK ) {
        return ret;
    }
    if(info->status & ONLP_THERMAL_STATUS_PRESENT) {
        ret = onlp_file_read_int(&info->mcelsius, __path_list[thermal_id].file );
    }

    return ret;
}

/**
 * @brief Retrieve the thermal's operational status.
 * @param id The thermal oid.
 * @param rv [out] Receives the operational status.
 */
int onlp_thermali_status_get(onlp_oid_t id, uint32_t* rv)
{
    int ret = ONLP_STATUS_OK;
    onlp_thermal_info_t* info;
    VALIDATE(id);
    uint32_t psu_status;

    int thermal_id = ONLP_OID_ID_GET(id);
    if(thermal_id >= ONLP_THERMAL_MAX) {
        return ONLP_STATUS_E_INVALID;
    }
    info = &__onlp_thermal_info[ thermal_id];

    switch(thermal_id) {
    case ONLP_THERMAL_1_ON_PSU1:
    case ONLP_THERMAL_2_ON_PSU1:
    case ONLP_THERMAL_3_ON_PSU1:
    case ONLP_THERMAL_1_ON_PSU2:
    case ONLP_THERMAL_2_ON_PSU2:
    case ONLP_THERMAL_3_ON_PSU2:
        ret = onlp_psui_status_get((&info->hdr)->poid, &psu_status);
        if(ret != ONLP_STATUS_OK) {
            return ret;
        }
        if(psu_status & ONLP_PSU_STATUS_PRESENT) {
            info->status = ADD_STATE(info->status,ONLP_PSU_STATUS_PRESENT);
        } else {
            info->status = 0;
        }
        break;
    default:
        break;
    }

    *rv = info->status;

    return ret;
}

/**
 * @brief Retrieve the thermal's oid header.
 * @param id The thermal oid.
 * @param rv [out] Receives the header.
 */
int onlp_thermali_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv)
{
    onlp_thermal_info_t* info;
    VALIDATE(id);

    int thermal_id = ONLP_OID_ID_GET(id);
    if(thermal_id >= ONLP_THERMAL_MAX) {
        return ONLP_STATUS_E_INVALID;
    }
    info = &__onlp_thermal_info[ thermal_id];

    *rv = info->hdr;

    return ONLP_STATUS_OK;
}
