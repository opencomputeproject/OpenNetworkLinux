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
#include <onlplib/i2c.h>
#include <unistd.h>
#include <onlplib/file.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum {
    PSU1_ID = 1,    /*Left-upper*/
    PSU2_ID,        /*Left-lower*/
    PSU3_ID,        /*Right-upper*/
    PSU4_ID,        /*Right-lower*/
};

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    { {ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0}, },
    { {ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0}, },
    { {ONLP_PSU_ID_CREATE(PSU3_ID), "PSU-3", 0}, },
    { {ONLP_PSU_ID_CREATE(PSU4_ID), "PSU-4", 0}, },    
};

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}
/*
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
}*/

#define PMBUS_PATH_FORMAT "/sys/bus/platform/devices/minipack_psensor/%s%d_input"

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int pid, value, pid_in, pid_out;
    char  path[64] = {0};

    VALIDATE(id);

    pid  = ONLP_OID_ID_GET(id);
    *info = pinfo[pid]; /* Set the onlp_oid_hdr_t */

    /* Get the present status
     */
#if 0     
    uint8_t mask = 0;
    mask = 1 << ((pid-1) * 4);
    value = onlp_i2c_readb(1, 0x32, 0x10, ONLP_I2C_F_FORCE);
    if (value < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (value & mask) {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    }
    info->status |= ONLP_PSU_STATUS_PRESENT;
    info->caps = ONLP_PSU_CAPS_AC; 

    /* Get power good status 
     */
    mask = 1 << ((pid-1) * 4 + 1);
    if (!(value & mask)) {
        info->status |= ONLP_PSU_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }


    /* Get input output power status
     */
    value = (pid == PSU1_ID) ? 0x2 : 0x1; /* mux channel for psu */
    if (bmc_i2c_writeb(7, 0x70, 0, value) < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    usleep(1200);
#else

    info->status |= ONLP_PSU_STATUS_PRESENT;
    info->caps = ONLP_PSU_CAPS_AC; 
#endif
    pid_in  = (pid * 2) - 1;
    pid_out = pid * 2;



    /* Read vin */
    sprintf(path, PMBUS_PATH_FORMAT, "in", pid_in);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (value >= 1000) {
        info->mvin = value;
        info->caps |= ONLP_PSU_CAPS_VIN;
    }

    /* Read iin */
    sprintf(path, PMBUS_PATH_FORMAT, "curr", pid_in);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (value >= 0) {
        info->miin = value;
        info->caps |= ONLP_PSU_CAPS_IIN;
    }

    /* Get pin */
    sprintf(path, PMBUS_PATH_FORMAT, "power", pid_in);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (value >= 0) {
        info->mpin = value / 1000;  /*power is in unit of microWatts.*/
        info->caps |= ONLP_PSU_CAPS_PIN;
    }
    /* Get vout */
    sprintf(path, PMBUS_PATH_FORMAT, "in", pid_out);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (value >= 0) {
            info->mvout = value;
            info->caps |= ONLP_PSU_CAPS_VOUT;
    }

    /* Read iout */
    sprintf(path, PMBUS_PATH_FORMAT, "curr", pid_out);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (value >= 0) {
        info->miout = value;
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }

    /* Read pout */
    sprintf(path, PMBUS_PATH_FORMAT, "power", pid_out);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (value >= 0) {
        info->mpout = value/1000;  /*power is in unit of microWatts.*/;
        info->caps |= ONLP_PSU_CAPS_POUT;
    }

    /* Get model name */
    //bmc_i2c_readraw(7, addr, 0x9a, info->model, sizeof(info->model));
    info->model[0] = '0';
    /* Get serial number */
    //bmc_i2c_readraw(7, addr, 0x9e, info->serial, sizeof(info->serial));
    info->serial[0] = '0';
    
    return 0;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}


