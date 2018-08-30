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

#define PSU_STATUS_PRESENT	0
#define PSU_STATUS_UNPOWERED	2
#define PSU_STATUS_FAULT	4
#define PSU_STATUS_UNINSTALLED	7

#define PSU_NODE_MAX_INT_LEN  8
#define PSU_NODE_MAX_PATH_LEN 64

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int 
psu_status_info_get(int id, char *node, int *value)
{
    int ret = 0;
    char node_path[PSU_NODE_MAX_PATH_LEN] = {0};
    char vstr[32], *vstrp = vstr, **vp = &vstrp;
    
    *value = 0;

    if (PSU1_ID == id) {
        sprintf(node_path, "%s%s0", PSU_HWMON_CPLD_PREFIX, node);
    }
    else if (PSU2_ID == id) {
        sprintf(node_path, "%s%s1", PSU_HWMON_CPLD_PREFIX, node);
    }
    
    ret = onlp_file_read_str(vp, node_path);

    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", node_path);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (!isdigit(*vstrp)) {
	return ONLP_STATUS_E_INTERNAL;
    }
    *value = *vstrp - '0';
    return ONLP_STATUS_OK;
}


int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

static int
psu_module_info_get(int id, onlp_psu_info_t* info)
{
    int ret = 0;
    char node_path[PSU_NODE_MAX_PATH_LEN] = {0};
    int value = 0;

    memset(node_path, 0, PSU_NODE_MAX_PATH_LEN);
    if (PSU1_ID == id) {
        sprintf(node_path, "%spsu2_vout", PSU_HWMON_PSOC_PREFIX);
    }
    else if (PSU2_ID == id) {
        sprintf(node_path, "%spsu1_vout", PSU_HWMON_PSOC_PREFIX);
    }
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", node_path);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->mvout = value;
    info->caps |= ONLP_PSU_CAPS_VOUT;

    memset(node_path, 0, PSU_NODE_MAX_PATH_LEN);
    if (PSU1_ID == id) {
        sprintf(node_path, "%spsu2_iout", PSU_HWMON_PSOC_PREFIX);
    }
    else if (PSU2_ID == id) {
        sprintf(node_path, "%spsu1_iout", PSU_HWMON_PSOC_PREFIX);
    }
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", node_path);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->miout = value;
    info->caps |= ONLP_PSU_CAPS_IOUT;

    memset(node_path, 0, PSU_NODE_MAX_PATH_LEN);
    if (PSU1_ID == id) {
        sprintf(node_path, "%spsu2_pout", PSU_HWMON_PSOC_PREFIX);
    }
    else if (PSU2_ID == id) {
        sprintf(node_path, "%spsu1_pout", PSU_HWMON_PSOC_PREFIX);
    }
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", node_path);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->mpout = value;
    info->caps |= ONLP_PSU_CAPS_POUT;

    memset(node_path, 0, PSU_NODE_MAX_PATH_LEN);
    if (PSU1_ID == id) {
        sprintf(node_path, "%spsu2_vin", PSU_HWMON_PSOC_PREFIX);
    }
    else if (PSU2_ID == id) {
        sprintf(node_path, "%spsu1_vin", PSU_HWMON_PSOC_PREFIX);
    }
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", node_path);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->mvin = value;
    info->caps |= ONLP_PSU_CAPS_VIN;

    memset(node_path, 0, PSU_NODE_MAX_PATH_LEN);
    if (PSU1_ID == id) {
        sprintf(node_path, "%spsu2_iin", PSU_HWMON_PSOC_PREFIX);
    }
    else if (PSU2_ID == id) {
        sprintf(node_path, "%spsu1_iin", PSU_HWMON_PSOC_PREFIX);
    }
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", node_path);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->miin = value;
    info->caps |= ONLP_PSU_CAPS_IIN;

    memset(node_path, 0, PSU_NODE_MAX_PATH_LEN);
    if (PSU1_ID == id) {
        sprintf(node_path, "%spsu2_pin", PSU_HWMON_PSOC_PREFIX);
    }
    else if (PSU2_ID == id) {
        sprintf(node_path, "%spsu1_pin", PSU_HWMON_PSOC_PREFIX);
    }
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", node_path);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->mpin = value;
    info->caps |= ONLP_PSU_CAPS_PIN;

    return ONLP_STATUS_OK;
}

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0 },
    },
    {
        { ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0 },
    }
};

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int val   = 0;
    int index = ONLP_OID_ID_GET(id);

    VALIDATE(id);

    memset(info, 0, sizeof(onlp_psu_info_t));
    *info = pinfo[index]; /* Set the onlp_oid_hdr_t */

    /* Get the present state */
    if (psu_status_info_get(index, "psu", &val) == ONLP_STATUS_E_INTERNAL) {
        printf("Unable to read PSU(%d) node(psu_present)\r\n", index);
    }

    if (val == PSU_STATUS_PRESENT) {
	info->status |= ONLP_PSU_STATUS_PRESENT;
	if (psu_module_info_get(index, info) != ONLP_STATUS_OK) {
	    printf("Unable to read PSU(%d) module information\r\n", index);
	}
	return ONLP_STATUS_OK;
    }

    if (val == PSU_STATUS_UNPOWERED) {
	info->status |= ONLP_PSU_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }

    if (val == PSU_STATUS_FAULT) {
	info->status |= ONLP_PSU_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }

    if (val == PSU_STATUS_UNINSTALLED) {
	info->status |= ONLP_PSU_STATUS_UNPLUGGED;
        return ONLP_STATUS_OK;
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}
