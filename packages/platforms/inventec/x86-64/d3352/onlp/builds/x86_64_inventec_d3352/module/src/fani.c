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
#include <stdlib.h>
#include <onlplib/file.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include <onlplib/mmap.h>
#include <fcntl.h>
#include "platform_lib.h"

#define FAN_GPI_ON_MAIN_BOARD	INV_PSOC_PREFIX"/fan_gpi"

#define MAX_FAN_SPEED     18000
#define MAX_PSU_FAN_SPEED 25500

#define PROJECT_NAME
#define LEN_FILE_NAME 80

#define LOCAL_ID_TO_PSU_ID(id) (id-CHASSIS_FAN_COUNT)

static char* devfiles__[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    INV_PSOC_PREFIX"/fan1_input",
    INV_PSOC_PREFIX"/fan2_input",
    INV_PSOC_PREFIX"/fan3_input",
    INV_PSOC_PREFIX"/fan4_input",
    INV_PSOC_PREFIX"/rpm_psu1",
    INV_PSOC_PREFIX"/rpm_psu2",
};

#define MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(id) \
    { \
        { \
            ONLP_FAN_ID_CREATE(FAN_##id##_ON_MAIN_BOARD), "Chassis Fan "#id, 0,  \
            {  \
                ONLP_LED_ID_CREATE(LED_FAN##id),     \
            }   \
        }, \
        0x0, \
        (ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id, fan_id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fan_id##_ON_PSU##psu_id), "Chassis PSU-"#psu_id " Fan "#fan_id, ONLP_PSU_ID_CREATE(PSU##psu_id##_ID) }, \
        0x0, \
        ( ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

/* Static fan information */
onlp_fan_info_t linfo[FAN_MAX] = {
    { }, /* Not used */
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(2),
    MAKE_FAN_INFO_NODE_ON_PSU(1,1),
    MAKE_FAN_INFO_NODE_ON_PSU(2,1),
};

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

int onlp_fani_status_get(onlp_oid_t id, uint32_t* rv)
{
    int  lrpm=0, rrpm=0, ret, len, gpi;
    char buf[ONLP_CONFIG_INFO_STR_MAX];
    int fid= ONLP_OID_ID_GET(id);
    int local_id=LOCAL_ID_TO_INFO_IDX(fid);

    ret = onlp_file_read( (uint8_t*)buf , ONLP_CONFIG_INFO_STR_MAX, &len, FAN_GPI_ON_MAIN_BOARD);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    sscanf( buf, "0x%x\n", &gpi);

    /* B[0-3] installed(0)/uninstalled(1)
       B[4-7] FRtype(0)/RFtype(1) */
    if ( (gpi>>(local_id)&1) ) { //fan failed
        *rv |= ONLP_FAN_STATUS_FAILED;
    } else {
        *rv |= ONLP_FAN_STATUS_PRESENT;
        if( (gpi>>(local_id+4))&1 ) *rv |= ONLP_FAN_STATUS_B2F;
        else *rv |= ONLP_FAN_STATUS_F2B;
    }

    /* get front fan speed */
    ret = onlp_file_read_int(&lrpm, devfiles__[fid*2-1]);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    ret = onlp_file_read_int(&rrpm, devfiles__[fid*2]);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    if(lrpm <=0 && rrpm <=0) {
        *rv |= ONLP_FAN_STATUS_FAILED;
    }
    return ONLP_STATUS_OK;
}

static int
_onlp_fani_info_get_fan(int fid, onlp_fan_info_t* info)
{
    int  lrpm=0, rrpm=0, ret;

    /* get fan present status */
    onlp_fani_status_get( info->hdr.id, &info->status );

    /* get front fan speed */
    ret = onlp_file_read_int(&lrpm, devfiles__[fid*2-1]);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    ret = onlp_file_read_int(&rrpm, devfiles__[fid*2]);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    if(lrpm <=0 && rrpm <=0) {
        info->rpm = 0;
    } else if(lrpm <= 0) {
        info->rpm = rrpm;
    } else if(rrpm <= 0) {
        info->rpm = lrpm;
    } else {
        info->rpm = (lrpm+rrpm)/2;
    }
    info->percentage = (info->rpm * 100) / MAX_PSU_FAN_SPEED;

    snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "N/A");
    snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "N/A");

    return ONLP_STATUS_OK;
}



static int
_onlp_fani_info_get_fan_on_psu(int fid, onlp_fan_info_t* info)
{
    int  ret;
    uint32_t psu_status;
    ret=onlp_psui_status_get( (&info->hdr)->poid, &psu_status );
    if(ret!=ONLP_STATUS_OK) return ret;

    if(psu_status & ONLP_PSU_STATUS_PRESENT) {
        info->status |= ONLP_FAN_STATUS_PRESENT;
    } else {
        info->status = 0;
        return ONLP_STATUS_OK;
    }

    /* get front fan speed */
    ret = onlp_file_read_int(&(info->rpm), INV_PSOC_PREFIX"/rpm_psu%d",LOCAL_ID_TO_PSU_ID(fid));
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    info->percentage = (info->rpm * 100) / MAX_PSU_FAN_SPEED;

    if (info->rpm==0) info->status |=ONLP_FAN_STATUS_FAILED;

    snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "N/A");
    snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "N/A");

    return ONLP_STATUS_OK;
}

/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    DEBUG_PRINT("%s(%d): %s\r\n", __FUNCTION__, __LINE__, INV_PLATFORM_NAME);

    return ONLP_STATUS_OK;
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    int rc = 0;
    int local_id;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    *info = linfo[local_id];

    switch (local_id) {
    case FAN_1_ON_PSU1:
    case FAN_1_ON_PSU2:
        rc = _onlp_fani_info_get_fan_on_psu(local_id, info);
        break;
    case FAN_1_ON_MAIN_BOARD:
    case FAN_2_ON_MAIN_BOARD:
        rc = _onlp_fani_info_get_fan(local_id, info);
        break;
    default:
        rc = ONLP_STATUS_E_INVALID;
        break;
    }

    return rc;
}

/*
 * This function sets the fan speed of the given OID as a percentage.
 *
 * This will only be called if the OID has the PERCENTAGE_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_percentage_set(onlp_oid_t id, int p)
{
    int  fid;
    char *path = NULL;

    VALIDATE(id);

    fid = ONLP_OID_ID_GET(id);

    /* reject p=0 (p=0, stop fan) */
    if (p == 0) {
        return ONLP_STATUS_E_INVALID;
    }

    switch (fid) {
    case FAN_1_ON_PSU1:
        return psu_pmbus_info_set(PSU1_ID, "rpm_psu1", p);
    case FAN_1_ON_PSU2:
        return psu_pmbus_info_set(PSU2_ID, "rpm_psu2", p);
    case FAN_1_ON_MAIN_BOARD:
    case FAN_2_ON_MAIN_BOARD:
        path = FAN_NODE(fan_duty_cycle_percentage);
        break;
    default:
        return ONLP_STATUS_E_INVALID;
    }

    if (onlp_file_write_int(p, path, NULL) != 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
