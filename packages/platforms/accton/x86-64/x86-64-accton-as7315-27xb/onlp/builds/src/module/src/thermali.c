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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include "platform_lib.h"

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};


#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static char* cpu_coretemp_files[] =
{
    "/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp10_input",
    "/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp14_input",
    "/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp1_input",
    "/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp4_input",
    "/sys/devices/platform/coretemp.0/hwmon/hwmon1/temp8_input",
    NULL,
};

static char* board_devfiles__[] =  /* must map with onlp_thermal_id */
{
    NULL,
    NULL,                  /* CPU_CORE files */
    "/sys/bus/i2c/devices/51-0049*temp1_input",
    "/sys/bus/i2c/devices/52-004a*temp1_input",
    "/sys/bus/i2c/devices/53-004c*temp1_input",
    "/sys/bus/i2c/devices/13-005b/psu_temp1_input",
    "/sys/bus/i2c/devices/12-0058/psu_temp1_input",
};



/* Static values */
static onlp_thermal_info_t linfo[] = {
    { }, /* Not used */
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "LM75-1", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "LM75-2", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "LM75-3", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID)},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID)},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    return ONLP_STATUS_OK;
}

#if 0
int
onlp_thermali_mb_channel_set(int tid)
{
    int rv;
    int channel = tid - THERMAL_1_ON_MAIN_BROAD + 2;
    uint32_t flags = ONLP_I2C_F_FORCE;
    uint8_t addr = 0x64;
    uint8_t offset = 0x80;

    if( (rv = onlp_i2c_readb(I2C_BUS, addr, offset, flags)) < 0) {
        AIM_LOG_ERROR("Device %s: readb() failed: %d",
                      linfo[tid].hdr.description, rv);
        return rv;
    }
    rv &= 0x0f; /*set bit [7:4] = 0.*/
    rv |= (channel << 5); /*set bit [7:5] = channel.*/
    rv = onlp_i2c_writeb(I2C_BUS, addr, offset, rv, flags);
    if( rv < 0) {
        AIM_LOG_ERROR("Device %s: readb() failed: %d",
                      linfo[tid].hdr.description, rv);
        return rv;
    }
    return ONLP_STATUS_OK;
}
#endif

int
onlp_thermali_read_devfile(int tid, onlp_thermal_info_t* info)
{
    return onlp_file_read_int(&info->mcelsius, board_devfiles__[tid]);
}

int
onlp_thermali_read_mainboard(int tid, onlp_thermal_info_t* info)
{
    if (tid >= THERMAL_1_ON_MAIN_BROAD || tid <= THERMAL_1_ON_PSU2) {
        return  onlp_thermali_read_devfile(tid, info);
    }

    return ONLP_STATUS_E_INVALID;
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
    int   tid, rv ;

    VALIDATE(id);
    tid = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[tid];

    if(tid == THERMAL_CPU_CORE) {
        rv = onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
    } else {
        rv = onlp_thermali_read_mainboard(tid, info);
    }
    return rv;
}

