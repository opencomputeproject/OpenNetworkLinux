/************************************************************
 * psui.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#include <onlp/platformi/psui.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "platform_lib.h"

typedef enum hwmon_psu_state_e {
    INV_PSU_NORMAL = 0,
    INV_PSU_UNPOWERED = 2,       //010
    INV_PSU_FAULT = 4,           //100
    INV_PSU_NOT_INSTALLED = 7    //111
} inv_psu_state_t;

#define MAKE_PSU_NODE_INFO_1					\
    {								\
        .hdr = {						\
            .id = ONLP_PSU_ID_CREATE(PSU1_ID),			\
            .description = "PSU-1",				\
            .poid = ONLP_OID_CHASSIS,				\
	        .coids = {					\
                ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1),	\
                ONLP_FAN_ID_CREATE(FAN_1_ON_PSU1)		\
            },							\
            .status = 0,					\
        },							\
        .model = "",						\
        .serial= "",						\
        .caps = ONLP_PSU_CAPS_GET_VIN |				\
		ONLP_PSU_CAPS_GET_VOUT |			\
		ONLP_PSU_CAPS_GET_IIN |				\
		ONLP_PSU_CAPS_GET_IOUT |			\
		ONLP_PSU_CAPS_GET_PIN |				\
		ONLP_PSU_CAPS_GET_POUT,				\
    }
#define MAKE_PSU_NODE_INFO_2					\
    {								\
        .hdr = {						\
            .id = ONLP_PSU_ID_CREATE(PSU2_ID),			\
            .description = "PSU-2",				\
            .poid = ONLP_OID_CHASSIS,				\
	        .coids = {					\
                ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2),	\
                ONLP_FAN_ID_CREATE(FAN_1_ON_PSU2)		\
            },							\
            .status = 0,					\
        },							\
        .model = "",						\
        .serial= "",						\
        .caps = ONLP_PSU_CAPS_GET_VIN |				\
		ONLP_PSU_CAPS_GET_VOUT |			\
		ONLP_PSU_CAPS_GET_IIN |				\
		ONLP_PSU_CAPS_GET_IOUT |			\
		ONLP_PSU_CAPS_GET_PIN |				\
		ONLP_PSU_CAPS_GET_POUT,				\
    }

static onlp_psu_info_t __onlp_psu_info[] = {
    {},
    MAKE_PSU_NODE_INFO_1,
    MAKE_PSU_NODE_INFO_2
};

int
onlp_psui_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, PSU1_ID, PSU2_ID);
}

int
inv_psui_status_get(onlp_oid_id_t id, uint32_t* rv)
{
    int result = ONLP_STATUS_OK;
    inv_psu_state_t psu_state;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX];
    int idx = LOCAL_ID_TO_INFO_IDX(id);

    ONLP_TRY(onlp_file_read((uint8_t*)&buf, ONLP_CONFIG_INFO_STR_MAX, &len, "%spsu%d", INV_SYSLED_PREFIX, idx));
    psu_state = (uint8_t)strtoul(buf, NULL, 0);
    if( psu_state == INV_PSU_UNPOWERED) {
        *rv = ONLP_OID_STATUS_FLAG_PRESENT | ONLP_OID_STATUS_FLAG_UNPLUGGED;
    } else if ( psu_state == INV_PSU_NORMAL) {
        *rv = ONLP_OID_STATUS_FLAG_PRESENT;
    } else if( psu_state == INV_PSU_FAULT) {
        *rv = ONLP_OID_STATUS_FLAG_PRESENT | ONLP_OID_STATUS_FLAG_FAILED;
    } else if( psu_state == INV_PSU_NOT_INSTALLED) {
        *rv = 0;
    } else {
        result = ONLP_STATUS_E_INVALID;
    }

    return result;
}

int
onlp_psui_info_get(onlp_oid_id_t id, onlp_psu_info_t* info)
{
    int ret   = ONLP_STATUS_OK;
    onlp_oid_hdr_t *hdr;

    ONLP_TRY( ONLP_OID_ID_VALIDATE_RANGE( id, PSU1_ID, PSU2_ID ) );
    *info = __onlp_psu_info[id]; /* Set the onlp_oid_hdr_t */
#if 1
    ONLP_TRY(onlp_file_read_str_dst(info->model,  ONLP_CONFIG_INFO_STR_MAX,INV_HWMON_PREFIX"psu%d_vendor", id));
    ONLP_TRY(onlp_file_read_str_dst(info->serial, ONLP_CONFIG_INFO_STR_MAX,INV_HWMON_PREFIX"psu%d_sn", id));
#else
    snprintf(info->model, 3, "N/A");
    snprintf(info->serial, 3, "N/A");
#endif

    hdr = &info->hdr;
    ret = onlp_psui_hdr_get( id, hdr);
    if(ret != ONLP_STATUS_OK) {
        return ret;
    }

    /*millivolts*/
    ONLP_TRY(onlp_file_read_int(&info->mvin, INV_HWMON_PREFIX"psoc_psu%d_vin",id));
    ONLP_TRY(onlp_file_read_int(&info->mvout, INV_HWMON_PREFIX"psoc_psu%d_vout", id));

    /* milliamps */
    ONLP_TRY(onlp_file_read_int(&info->miin, INV_HWMON_PREFIX"psoc_psu%d_iin", id));
    ONLP_TRY(onlp_file_read_int(&info->miout, INV_HWMON_PREFIX"psoc_psu%d_iout", id));

    /* milliwatts */
    ONLP_TRY(onlp_file_read_int(&info->mpin, INV_HWMON_PREFIX"psoc_psu%d_pin", id));
    ONLP_TRY(onlp_file_read_int(&info->mpout, INV_HWMON_PREFIX"psoc_psu%d_pout", id));

    return ret;
}

int
onlp_psui_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    int result = ONLP_STATUS_OK;
    onlp_psu_info_t* info;

    info = &__onlp_psu_info[id];
    *hdr  = info->hdr;

    result = inv_psui_status_get( id , &hdr->status);

    return result;
}


int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
