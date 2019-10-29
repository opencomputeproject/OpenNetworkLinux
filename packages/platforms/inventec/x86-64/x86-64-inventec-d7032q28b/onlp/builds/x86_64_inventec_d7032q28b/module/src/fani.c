/************************************************************
 * fani.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 * Fan Platform Implementation Defaults.
 *
 ***********************************************************/
#include <onlp/platformi/base.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#define MAX_FAN_SPEED		(18000)
#define MAX_PSU_FAN_SPEED	(25500)
#define FAN_GPI_ON_MAIN_BOARD INV_HWMON_PREFIX"fan_gpi"
#define LOCAL_ID_TO_PSU_ID(id) (id-CHASSIS_FAN_COUNT)

extern int inv_psui_status_get(onlp_oid_id_t id, uint32_t* rv);

static char* devfiles__[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    INV_HWMON_PREFIX"/fan1_input",
    INV_HWMON_PREFIX"/fan2_input",
    INV_HWMON_PREFIX"/fan3_input",
    INV_HWMON_PREFIX"/fan4_input",
    INV_HWMON_PREFIX"/fan5_input",
    INV_HWMON_PREFIX"/fan6_input",
    INV_HWMON_PREFIX"/fan7_input",
    INV_HWMON_PREFIX"/fan8_input",
    INV_HWMON_PREFIX"/rpm_psu1",
    INV_HWMON_PREFIX"/rpm_psu2",
};

#define MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(_id)				\
    {									\
        .hdr = {							\
            .id = ONLP_FAN_ID_CREATE(FAN_##_id##_ON_MAIN_BOARD),	\
            .description = "Chassis Fan "#_id,				\
            .poid = ONLP_OID_CHASSIS,					\
            .coids = {                                  \
                ONLP_LED_ID_CREATE(LED_FAN##_id),       \
            },                                          \
        },								\
        .dir = ONLP_FAN_DIR_UNKNOWN,					\
        .caps = (ONLP_FAN_CAPS_GET_DIR |				\
		 ONLP_FAN_CAPS_GET_RPM |				\
		 ONLP_FAN_CAPS_GET_PERCENTAGE),				\
    }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id, fan_id)			\
    {									\
        .hdr = {							\
            .id = ONLP_FAN_ID_CREATE(FAN_##fan_id##_ON_PSU##psu_id),	\
            .description = "Chassis PSU-"#psu_id " Fan "#fan_id,	\
            .poid = ONLP_PSU_ID_CREATE(PSU##psu_id##_ID)		\
        },								\
        .dir = ONLP_FAN_DIR_UNKNOWN,					\
        .caps = (ONLP_FAN_CAPS_GET_DIR |				\
		 ONLP_FAN_CAPS_GET_RPM |				\
		 ONLP_FAN_CAPS_GET_PERCENTAGE),				\
    }

/* Static fan information */
onlp_fan_info_t fan_info[FAN_MAX] = {
    { }, /* Not used */
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(2),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(3),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(4),
    MAKE_FAN_INFO_NODE_ON_PSU(1,1),
    MAKE_FAN_INFO_NODE_ON_PSU(2,1),
};

static int
psu_fani_hdr_get__(int id, onlp_oid_hdr_t* hdr)
{
    int rpm, ret;
    uint32_t psu_status;
    *hdr = fan_info[id].hdr;

    onlp_oid_id_t psu_id=ONLP_OID_ID_GET(hdr->poid);
    ret=inv_psui_status_get(psu_id, &psu_status);
    if(ret!=ONLP_STATUS_OK) {
        return ret;
    }

    if(psu_status & ONLP_OID_STATUS_FLAG_PRESENT) {
        ONLP_TRY(onlp_file_read_int(&rpm, "%s", devfiles__[id]));
        ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
        if (rpm == 0) {
            ONLP_OID_STATUS_FLAG_SET(hdr, FAILED);
        } else if(psu_status & ONLP_OID_STATUS_FLAG_UNPLUGGED) {
            ONLP_OID_STATUS_FLAG_SET(hdr,UNPLUGGED);
        }
    } else {
        hdr->status = 0;
    }

    return ONLP_STATUS_OK;
}

int inv_fani_status_get(onlp_oid_t id, uint32_t* rv)
{
    int  lrpm=0, rrpm=0, len, gpi;
    char buf[ONLP_CONFIG_INFO_STR_MAX];
    int fid= ONLP_OID_ID_GET(id);
    int local_id=LOCAL_ID_TO_INFO_IDX(fid);

    ONLP_TRY( ONLP_OID_ID_VALIDATE_RANGE( fid, FAN_1_ON_MAIN_BOARD, FAN_4_ON_MAIN_BOARD) );
    ONLP_TRY( onlp_file_read( (uint8_t*)buf , ONLP_CONFIG_INFO_STR_MAX, &len, FAN_GPI_ON_MAIN_BOARD) );

    sscanf( buf, "0x%x\n", &gpi);
    /* B[0-3] installed(0)/uninstalled(1)
       B[4-7] FRtype(0)/RFtype(1) */
    if ( (gpi>>(local_id)&1) ) {
        *rv = 0;
        return ONLP_STATUS_OK;
    } else {
        *rv |= ONLP_OID_STATUS_FLAG_PRESENT;
    }

    /* get front fan speed */
    ONLP_TRY( onlp_file_read_int(&lrpm, devfiles__[fid*2-1]) );
    ONLP_TRY( onlp_file_read_int(&rrpm, devfiles__[fid*2]) );
    if(lrpm <=0 && rrpm <=0) {
        *rv |= ONLP_OID_STATUS_FLAG_FAILED ;
    }
    return ONLP_STATUS_OK;
}

static int
chassis_fani_hdr_get__(int id, onlp_oid_hdr_t* hdr)
{
    int result = ONLP_STATUS_OK;
    onlp_fan_info_t* info;

    info = &fan_info[id];
    *hdr = info->hdr;
    result=inv_fani_status_get(id, &(hdr->status));

    return result;

}

static int
chassis_fani_info_get__(int id, onlp_fan_info_t* info)
{
    int lrpm, rrpm,gpi,len,ret;
    char buf[ONLP_CONFIG_INFO_STR_MAX];
    int local_id=LOCAL_ID_TO_INFO_IDX(id);
    onlp_oid_hdr_t *hdr;

    if(id<=0 || id>FAN_4_ON_MAIN_BOARD ) return ONLP_STATUS_E_INVALID;

    *info =  fan_info[id];
    hdr = &info->hdr;
    ret=chassis_fani_hdr_get__(id, hdr);
    if(ret!=ONLP_STATUS_OK) {
        return ret;
    }

    ONLP_TRY( onlp_file_read( (uint8_t*)buf , ONLP_CONFIG_INFO_STR_MAX, &len, FAN_GPI_ON_MAIN_BOARD) );
    sscanf( buf, "0x%x\n", &gpi);
    /* B[0-3] installed(0)/uninstalled(1)
       B[4-7] FRtype(0)/RFtype(1) */
    if(hdr->status==0) {
        info->dir=ONLP_FAN_DIR_UNKNOWN;
    } else if( (gpi>>(local_id+CHASSIS_FAN_COUNT-1))&1 ) {
        info->dir=ONLP_FAN_DIR_B2F;
    } else {
        info->dir=ONLP_FAN_DIR_F2B;
    }
    /*
     * Get fan speed (take the min from two speeds)
     */
    ONLP_TRY(onlp_file_read_int(&lrpm, "%sfan%d_input", INV_HWMON_PREFIX, id*2-1));
    ONLP_TRY(onlp_file_read_int(&rrpm, "%sfan%d_input", INV_HWMON_PREFIX, id*2));
    if(lrpm <=0 && rrpm <=0) {
        info->rpm = 0;
    } else if(lrpm <= 0) {
        info->rpm = rrpm;
    } else if(rrpm <= 0) {
        info->rpm = lrpm;
    } else {
        info->rpm = (lrpm+rrpm)/2;
    }

    /* get speed percentage from rpm */
    info->percentage = (info->rpm * 100)/MAX_FAN_SPEED;
    snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "N/A");
    snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "N/A");

    return ONLP_STATUS_OK;
}

int
onlp_fani_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    int id = ONLP_OID_ID_GET(oid);
    switch(id) {
    case FAN_1_ON_PSU1:
    case FAN_1_ON_PSU2:
        return psu_fani_hdr_get__(id, hdr);
    case FAN_1_ON_MAIN_BOARD:
    case FAN_2_ON_MAIN_BOARD:
    case FAN_3_ON_MAIN_BOARD:
    case FAN_4_ON_MAIN_BOARD:
        return chassis_fani_hdr_get__(id, hdr);
    default:
        return ONLP_STATUS_E_PARAM;
    }

    return ONLP_STATUS_OK;
}

static int
psu_fani_info_get__(int id, const char* device, onlp_fan_info_t* info)
{
    int ret;
    *info = fan_info[id];
    ret=psu_fani_hdr_get__(id, &info->hdr);
    if(ret!=ONLP_STATUS_OK) {
        return ret;
    }

    if(ONLP_OID_STATUS_FLAG_NOT_SET(info, PRESENT) ||
            ONLP_OID_STATUS_FLAG_IS_SET(info, FAILED)) {
        return 0;
    }

    /*
     * Direction.
     */
    if( ONLP_OID_STATUS_FLAG_IS_SET(info,UNPLUGGED) )  info->dir = ONLP_FAN_DIR_UNKNOWN;
    else info->dir = ONLP_FAN_DIR_F2B;

    ONLP_TRY(onlp_file_read_int(&info->rpm, INV_HWMON_PREFIX"rpm_psu%d", LOCAL_ID_TO_PSU_ID(id)));
    info->percentage = (info->rpm * 100) / MAX_PSU_FAN_SPEED;

    return ONLP_STATUS_OK;
}

int
onlp_fani_info_get(onlp_oid_t oid, onlp_fan_info_t* info)
{
    int id = ONLP_OID_ID_GET(oid);
    switch(id) {
    case FAN_1_ON_PSU1:
        return psu_fani_info_get__(id, "0", info);
    case FAN_1_ON_PSU2:
        return psu_fani_info_get__(id, "1", info);
    case FAN_1_ON_MAIN_BOARD:
    case FAN_2_ON_MAIN_BOARD:
    case FAN_3_ON_MAIN_BOARD:
    case FAN_4_ON_MAIN_BOARD:
        return chassis_fani_info_get__(id, info);
    default:
        return ONLP_STATUS_E_PARAM;
    }

    return ONLP_STATUS_OK;
}

int
onlp_fani_rpm_set(onlp_oid_t id, int rpm)
{
    return ONLP_STATUS_OK;
}

int
onlp_fani_percentage_set(onlp_oid_t id, int p)
{
    return ONLP_STATUS_OK;
}
