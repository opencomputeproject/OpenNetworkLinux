/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <unistd.h>
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#define PSU1_ID 1
#define PSU2_ID 2

#define SYS_CPLD_PATH_FMT	"/sys/bus/i2c/drivers/syscpld/12-0031/%s"
#define PSU_PRESENT_FMT		"psu%d_present"
#define PSU_PWROK_FMT		"psu%d_output_pwr_sts"

#define PSU_PFE1100_PATH_FMT	"/sys/bus/i2c/devices/7-%s/%s\r\n"
#define PSU_PFE1100_MODEL	"mfr_model_label"
#define PSU_PFE1100_SERIAL	"mfr_serial_label"

static const char *psu_pfedrv_i2c_devaddr[] = {"0059", "005a"};

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


static int
twos_complement_to_int(uint16_t data, uint8_t valid_bit, int mask)
{
    uint16_t valid_data = data & mask;
    bool is_negative = valid_data >> (valid_bit - 1);

    return is_negative ? (-(((~valid_data) & mask) + 1)) : valid_data;
}

static int
pmbus_parse_literal_format(uint16_t value)
{
    int exponent, mantissa, multiplier = 1000;

    exponent = twos_complement_to_int(value >> 11, 5, 0x1f);
    mantissa = twos_complement_to_int(value & 0x7ff, 11, 0x7ff);

    return (exponent >= 0) ? (mantissa << exponent) * multiplier :
                             (mantissa * multiplier) / (1 << -exponent);
}

/**
 * @brief Software initialization of the PSU module.
 */
int onlp_psui_sw_init(void) {
    return ONLP_STATUS_OK;
}

/**
 * @brief Hardware initialization of the PSU module.
 * @param flags The hardware initialization flags.
 */
int onlp_psui_hw_init(uint32_t flags) {
    return ONLP_STATUS_OK;
}

/**
 * @brief Deinitialize the psu software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_psui_sw_denit(void) {
    return ONLP_STATUS_OK;
}

/**
 * @brief Get the PSU's oid header.
 * @param id The PSU OID.
 * @param[out] rv Receives the header.
 */
int onlp_psui_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* rv) {
    onlp_psu_info_t info;
    onlp_psui_info_get(id, &info);
    *rv = info.hdr;
    return ONLP_STATUS_OK;
}

/**
 * @brief Get the information structure for the given PSU
 * @param id The PSU OID
 * @param[out] rv Receives the PSU information.
 */
int onlp_psui_info_get(onlp_oid_id_t id, onlp_psu_info_t* info) {
    int value, addr, ret = 0;
    char file[32] = {0};
    char path[80] = {0};
    
    *info = pinfo[id]; /* Set the onlp_oid_hdr_t */

    /* Get the present status
     */
    ret = snprintf(file, sizeof(file), PSU_PRESENT_FMT, id);
    if( ret >= sizeof(file) ){
        AIM_LOG_ERROR("file size overwrite (%d,%d)\r\n", ret, sizeof(file));
        return ONLP_STATUS_E_INTERNAL;
    }
    ret = snprintf(path, sizeof(path), SYS_CPLD_PATH_FMT, file);
    if( ret >= sizeof(path) ){
        AIM_LOG_ERROR("path size overwrite (%d,%d)\r\n", ret, sizeof(path));
        return ONLP_STATUS_E_INTERNAL;
    }
    if (bmc_file_read_int(&value, path, 16) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (value) {
        info->hdr.status &= ~ONLP_OID_STATUS_FLAG_PRESENT;
        return ONLP_STATUS_OK;
    }
    info->hdr.status |= ONLP_OID_STATUS_FLAG_PRESENT;
    info->type = ONLP_PSU_TYPE_AC;

    /* Get power good status 
     */
    ret = snprintf(file, sizeof(file), PSU_PWROK_FMT, id);
    if( ret >= sizeof(file) ){
        AIM_LOG_ERROR("file size overwrite (%d,%d)\r\n", ret, sizeof(file));
        return ONLP_STATUS_E_INTERNAL;
    }
    ret = snprintf(path, sizeof(path), SYS_CPLD_PATH_FMT, file);
    if( ret >= sizeof(path) ){
        AIM_LOG_ERROR("path size overwrite (%d,%d)\r\n", ret, sizeof(path));
        return ONLP_STATUS_E_INTERNAL;
    }
    if (bmc_file_read_int(&value, path, 16) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (!value) {
        info->hdr.status |= ONLP_OID_STATUS_FLAG_FAILED;
        return ONLP_STATUS_OK;
    }


    /* Get input output power status
     */
    value = (id == PSU1_ID) ? 0x2 : 0x1; /* mux channel for psu */
    if (bmc_i2c_write_quick_mode(7, 0x70, value) < 0) {
        AIM_LOG_ERROR("Unable to set i2c device (7/0x70)\r\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    usleep(1200);

    /* Read vin */
    addr  = (id == PSU1_ID) ? 0x59 : 0x5a;
    value = bmc_i2c_readw(7, addr, 0x88);
    if (value >= 0) {
        info->mvin = pmbus_parse_literal_format(value);
        info->caps |= ONLP_PSU_CAPS_GET_VIN;
    }

    /* Read iin */
    value = bmc_i2c_readw(7, addr, 0x89);
    if (value >= 0) {
        info->miin = pmbus_parse_literal_format(value);
        info->caps |= ONLP_PSU_CAPS_GET_IIN;
    }

    /* Get pin */
    if ((info->caps & ONLP_PSU_CAPS_GET_VIN) && (info->caps & ONLP_PSU_CAPS_GET_IIN)) {
        info->mpin = info->mvin * info->miin / 1000;
        info->caps |= ONLP_PSU_CAPS_GET_PIN;
    }

    /* Read iout */
    value = bmc_i2c_readw(7, addr, 0x8c);
    if (value >= 0) {
        info->miout = pmbus_parse_literal_format(value);
        info->caps |= ONLP_PSU_CAPS_GET_IOUT;
    }

    /* Read pout */
    value = bmc_i2c_readw(7, addr, 0x96);
    if (value >= 0) {
        info->mpout = pmbus_parse_literal_format(value);
        info->caps |= ONLP_PSU_CAPS_GET_POUT;
    }

    /* Get vout */
    if ((info->caps & ONLP_PSU_CAPS_GET_IOUT) && (info->caps & ONLP_PSU_CAPS_GET_POUT) && info->miout != 0) {
        info->mvout = info->mpout / info->miout * 1000;
        info->caps |= ONLP_PSU_CAPS_GET_VOUT;
    }

    /* Get model name */
    ret = snprintf(path, sizeof(path), PSU_PFE1100_PATH_FMT, psu_pfedrv_i2c_devaddr[id-1], PSU_PFE1100_MODEL);
    if( ret >= sizeof(path) ){
        AIM_LOG_ERROR("path size overwrite (%d,%d)\r\n", ret, sizeof(path));
        return ONLP_STATUS_E_INTERNAL;
    }
    bmc_file_read_str(path, info->model, sizeof(info->model));

    /* Get serial number */
    ret = snprintf(path, sizeof(path), PSU_PFE1100_PATH_FMT, psu_pfedrv_i2c_devaddr[id-1], PSU_PFE1100_SERIAL);
    if( ret >= sizeof(path) ){
        AIM_LOG_ERROR("path size overwrite (%d,%d)\r\n", ret, sizeof(path));
        return ONLP_STATUS_E_INTERNAL;
    }
    bmc_file_read_str(path, info->serial, sizeof(info->serial));
    return ONLP_STATUS_OK;
}

int
onlp_psui_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}


