/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 MiTAC Computing Technology Corporation.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <onlp/platformi/psui.h>
#include <onlplib/mmap.h>
#include <stdio.h>
#include <string.h>
#include "platform_lib.h"

#define PSU_STATUS_PRESENT    1
#define PSU_STATUS_POWER_GOOD 1

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
    char buf[PSU_NODE_MAX_INT_LEN + 1] = {0};
    char node_path[PSU_NODE_MAX_PATH_LEN] = {0};

    *value = 0;

    if (PSU1_ID == id) {
        sprintf(node_path, "%s%s", PSU1_AC_HWMON_PREFIX, node);
    }
    else if (PSU2_ID == id) {
        sprintf(node_path, "%s%s", PSU2_AC_HWMON_PREFIX, node);
    }

    ret = deviceNodeReadString(node_path, buf, sizeof(buf), 0);

    if (ret == 0) {
        *value = atoi(buf);
    }

    return ret;
}

static int
psu_ym2651_pmbus_info_get(int id, char *node, int *value)
{
    int  ret = 0;
    char buf[PSU_NODE_MAX_INT_LEN + 1]    = {0};
    char node_path[PSU_NODE_MAX_PATH_LEN] = {0};

    *value = 0;

    if (PSU1_ID == id) {
        sprintf(node_path, "%s%s", PSU1_AC_PMBUS_PREFIX, node);
    }
    else {
        sprintf(node_path, "%s%s", PSU2_AC_PMBUS_PREFIX, node);
    }

    ret = deviceNodeReadString(node_path, buf, sizeof(buf), 0);

    if (ret == 0) {
        *value = atoi(buf);
    }

    return ret;
}

int
onlp_psui_init(void)
{
    //printf("Hello, this is %s() in %s.\n", __func__, __FILE__);
    return ONLP_STATUS_OK;
}

static int
psu_ym2651_info_get(onlp_psu_info_t* info)
{
    int val   = 0;
    int index = ONLP_OID_ID_GET(info->hdr.id);

    if (info->status & ONLP_PSU_STATUS_FAILED) {
        return ONLP_STATUS_OK;
    }

    /* Set the associated oid_table */
    info->hdr.coids[0] = ONLP_FAN_ID_CREATE(index + CHASSIS_FAN_COUNT);
    info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(index + CHASSIS_THERMAL_COUNT);

    /* Read voltage, current and power */
    if (psu_ym2651_pmbus_info_get(index, "psu_v_out", &val) == 0) {
        info->mvout = val;
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }

    if (psu_ym2651_pmbus_info_get(index, "psu_i_out", &val) == 0) {
        info->miout = val;
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }

    if (psu_ym2651_pmbus_info_get(index, "psu_p_out", &val) == 0) {
        info->mpout = val;
        info->caps |= ONLP_PSU_CAPS_POUT;
    }

    return ONLP_STATUS_OK;
}

#include <onlplib/i2c.h>
#define DC12V_750_REG_TO_CURRENT(low, high) (((low << 4 | high >> 4) * 20 * 1000) / 754)
#define DC12V_750_REG_TO_VOLTAGE(low, high) ((low << 4 | high >> 4) * 25)

static int
psu_dc12v_750_info_get(onlp_psu_info_t* info)
{
    int pid = ONLP_OID_ID_GET(info->hdr.id);
    int bus = (PSU1_ID == pid) ? 18 : 17;
    int iout_low, iout_high;
    int vout_low, vout_high;

    /* Set capability
     */
    info->caps = ONLP_PSU_CAPS_DC12;

    if (info->status & ONLP_PSU_STATUS_FAILED) {
        return ONLP_STATUS_OK;
    }

    /* Get current
     */
    iout_low  = onlp_i2c_readb(bus, 0x6f, 0x0, ONLP_I2C_F_FORCE);
    iout_high = onlp_i2c_readb(bus, 0x6f, 0x1, ONLP_I2C_F_FORCE);

    if ((iout_low >= 0) && (iout_high >= 0)) {
        info->miout = DC12V_750_REG_TO_CURRENT(iout_low, iout_high);
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }

    /* Get voltage
     */
    vout_low  = onlp_i2c_readb(bus, 0x6f, 0x2, ONLP_I2C_F_FORCE);
    vout_high = onlp_i2c_readb(bus, 0x6f, 0x3, ONLP_I2C_F_FORCE);

    if ((vout_low >= 0) && (vout_high >= 0)) {
        info->mvout = DC12V_750_REG_TO_VOLTAGE(vout_low, vout_high);
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }

    /* Get power based on current and voltage
     */
    if ((info->caps & ONLP_PSU_CAPS_IOUT) && (info->caps & ONLP_PSU_CAPS_VOUT)) {
        info->mpout = (info->miout * info->mvout) / 1000;
        info->caps |= ONLP_PSU_CAPS_POUT;
    }

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
    // FIXME: WARNING FREEZE
    if ( 0 ) {
        int ret   = 0;
        int index = 0;

        /* Get the present state */
        if (psu_status_info_get(index, "psu_present", &ret) != 0) {
            printf("Unable to read PSU(%d) node(psu_present)\r\n", index);
        }

        ret = psu_ym2651_info_get(&pinfo[0]);
        ret = psu_dc12v_750_info_get(&pinfo[0]);
        printf("%dx", ret);
    }
    // END WARNING FREEZE
    return ONLP_STATUS_OK;
}

/*
 * Additional Functions
 */
int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_psui_status_get(onlp_oid_t id, uint32_t* rv)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_psui_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

