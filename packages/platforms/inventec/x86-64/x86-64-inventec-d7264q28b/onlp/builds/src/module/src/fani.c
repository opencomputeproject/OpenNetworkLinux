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
#include <onlplib/mmap.h>
#include <fcntl.h>
#include "platform_lib.h"

#define FAN_GPI_ON_MAIN_BOARD	INV_PSOC_PREFIX"/fan_gpi"

#define MAX_FAN_SPEED     18000
#define MAX_PSU_FAN_SPEED 25500

#define PROJECT_NAME
#define LEN_FILE_NAME 80

static char* devfiles__[FAN_MAX] =  /* must map with onlp_thermal_id */
{
    "reserved",
    INV_PSOC_PREFIX"/fan1_input",
    INV_PSOC_PREFIX"/fan2_input",
    INV_PSOC_PREFIX"/fan3_input",
    INV_PSOC_PREFIX"/fan4_input",
    INV_PSOC_PREFIX"/fan5_input",
    INV_PSOC_PREFIX"/fan6_input",
    INV_PSOC_PREFIX"/fan7_input",
    INV_PSOC_PREFIX"/fan8_input",
    INV_PSOC_PREFIX"/rpm_psu1",
    INV_PSOC_PREFIX"/rpm_psu2",
};

#define MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##id##_ON_MAIN_BOARD), "Chassis Fan "#id, 0 }, \
        0x0, \
        (ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id, fan_id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fan_id##_ON_PSU##psu_id), "Chassis PSU-"#psu_id " Fan "#fan_id, 0 }, \
        0x0, \
        (ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

/* Static fan information */
onlp_fan_info_t linfo[FAN_MAX] = {
    { }, /* Not used */
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(2),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(3),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(4),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(5),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(6),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(7),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(8),
    MAKE_FAN_INFO_NODE_ON_PSU(1,1),
    MAKE_FAN_INFO_NODE_ON_PSU(2,1),
};

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int
_onlp_fani_info_get_fan(int fid, onlp_fan_info_t* info)
{
    int   value, ret;
    char  vstr[32], *vstrp = vstr, **vp = &vstrp;

    memset(vstr, 0, 32);
    /* get fan present status */
    ret = onlp_file_read_str(vp, FAN_GPI_ON_MAIN_BOARD);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    sscanf(*vp, "%x", &value);
    if (value & (1 << (fid-1))) {
	info->status |= ONLP_FAN_STATUS_FAILED;
    }
    else {
	info->status |= ONLP_FAN_STATUS_PRESENT;
	info->status |= ONLP_FAN_STATUS_F2B;
    }

    /* get front fan speed */
    memset(vstr, 0, 32);
    ret = onlp_file_read_str(vp, devfiles__[fid]);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    sscanf(*vp, "%d", &value);
    info->rpm = value;
    info->percentage = (info->rpm * 100) / MAX_PSU_FAN_SPEED;

    snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "NA");
    snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "NA");

    return ONLP_STATUS_OK;
}


static uint32_t
_onlp_get_fan_direction_on_psu(void)
{
    /* Try to read direction from PSU1.
     * If PSU1 is not valid, read from PSU2
     */
    int i = 0;

    for (i = PSU1_ID; i <= PSU2_ID; i++) {
        psu_type_t psu_type;
        psu_type = get_psu_type(i, NULL, 0);

        if (psu_type == PSU_TYPE_UNKNOWN) {
            continue;
        }

        if (PSU_TYPE_AC_F2B == psu_type) {
            return ONLP_FAN_STATUS_F2B;
        }
        else {
            return ONLP_FAN_STATUS_B2F;
        }
    }

    return 0;
}


static int
_onlp_fani_info_get_fan_on_psu(int fid, onlp_fan_info_t* info)
{
    int   value, ret, index;
    char  vstr[32], *vstrp = vstr, **vp = &vstrp;

    info->status |= ONLP_FAN_STATUS_PRESENT;
    info->status |= ONLP_FAN_STATUS_F2B;

    /* get fan direction */
    info->status |= _onlp_get_fan_direction_on_psu();

    if (info->status & ONLP_FAN_STATUS_FAILED) {
	return ONLP_STATUS_OK;
    }

    index = ONLP_OID_ID_GET(info->hdr.id);
    info->hdr.coids[0] = ONLP_FAN_ID_CREATE(index + CHASSIS_FAN_COUNT);
    /* get front fan speed */
    memset(vstr, 0, 32);
    ret = onlp_file_read_str(vp, devfiles__[fid]);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    sscanf(*vp, "%d", &value);
    info->rpm = value;
    info->percentage = (info->rpm * 100) / MAX_PSU_FAN_SPEED;
    info->status |= (value == 0) ? ONLP_FAN_STATUS_FAILED : 0;

    snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "NA");
    snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "NA");

    return ONLP_STATUS_OK;
}

/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
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
    switch (local_id)
    {
	case FAN_1_ON_PSU1:
	case FAN_1_ON_PSU2:
	    rc = _onlp_fani_info_get_fan_on_psu(local_id, info);
	    break;
        case FAN_1_ON_MAIN_BOARD:
        case FAN_2_ON_MAIN_BOARD:
        case FAN_3_ON_MAIN_BOARD:
        case FAN_4_ON_MAIN_BOARD:
        case FAN_5_ON_MAIN_BOARD:
        case FAN_6_ON_MAIN_BOARD:
        case FAN_7_ON_MAIN_BOARD:
        case FAN_8_ON_MAIN_BOARD:
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
    if (p == 0){
        return ONLP_STATUS_E_INVALID;
    }

    switch (fid)
        {
	case FAN_1_ON_PSU1:
			return psu_pmbus_info_set(PSU1_ID, "rpm_psu1", p);
	case FAN_1_ON_PSU2:
			return psu_pmbus_info_set(PSU2_ID, "rpm_psu2", p);
        case FAN_1_ON_MAIN_BOARD:
        case FAN_2_ON_MAIN_BOARD:
        case FAN_3_ON_MAIN_BOARD:
        case FAN_4_ON_MAIN_BOARD:
        case FAN_5_ON_MAIN_BOARD:
        case FAN_6_ON_MAIN_BOARD:
        case FAN_7_ON_MAIN_BOARD:
        case FAN_8_ON_MAIN_BOARD:
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
