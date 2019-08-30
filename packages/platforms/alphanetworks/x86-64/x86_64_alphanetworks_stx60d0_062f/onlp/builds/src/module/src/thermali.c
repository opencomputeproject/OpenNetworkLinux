/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2018 Alpha Networks Incorporation
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
#include <unistd.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <fcntl.h>
#include "platform_lib.h"

#define LOCAL_DEBUG 0

#define THERMALI_LM75_AMBIENT_ID 1
#define THERMALI_LM75_HOT_SPOT_ID 2

/* IPMI Table 22-14 define Request Data byte 2 as I2C address. 
  * [7:1] -Slave address and [0] - reserved.
  * So Thermal Physical Address in the I2C is 0x49( 0100 1001) should transfer to 0x92 (1001 0010).
  */
#define BMC_THERMALI_LM75_HOT_SPOT_ADDR 0x92    /* Thermal addr */
#define BMC_THERMALI_LM75_AMBIENT_ADDR  0x90    /* Thermal addr */

#define THERMALI_LM75_HOT_SPOT_ADDR 0x49    /* Thermal addr */
#define THERMALI_LM75_AMBIENT_ADDR  0x48    /* Thermal addr */

 #define BMC_THERMALI_LM75_BUS_ID   BMC_I2C6


#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)




enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_1_ON_MAIN_BROAD_HOT_SPOT,    /* The LM75 on Main Board for hot spot */
    THERMAL_2_ON_MAIN_BROAD_AMBIENT      /* The LM75 on Main Board for ambient */
};

static int thermali_current_temp_get(int id, char *data)
{
    int ret = 0;

    if (id == THERMALI_LM75_AMBIENT_ID)
    {
        ret = bmc_i2c_read_byte(BMC_THERMALI_LM75_BUS_ID, BMC_THERMALI_LM75_AMBIENT_ADDR, 0x0, data);
        if (ret < 0)
            printf("I2C command 0x%X Read Fail, id=%d\n", BMC_THERMALI_LM75_AMBIENT_ADDR, BMC_THERMALI_LM75_BUS_ID);
    }
    else if (id == THERMALI_LM75_HOT_SPOT_ID)
    {
        ret = bmc_i2c_read_byte(BMC_THERMALI_LM75_BUS_ID, BMC_THERMALI_LM75_HOT_SPOT_ADDR, 0x0, data);
        if (ret < 0)
            printf("I2C command 0x%X Read Fail, id=%d\n", BMC_THERMALI_LM75_HOT_SPOT_ADDR, BMC_THERMALI_LM75_BUS_ID);
    }


    return ret;
}


/* Static values */
static onlp_thermal_info_t linfo[] = {
    { }, /* Not used */
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD_HOT_SPOT), "Chassis Thermal Sensor 1 (HOT SPOT)", 0 },
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD_AMBIENT), "Chassis Thermal Sensor 2 (AMBIENT)", 0 },
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
    DIAG_PRINT("%s", __FUNCTION__);
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
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t *info)
{
    DIAG_PRINT("%s, id=%d", __FUNCTION__, id);
    int local_id, milli_unit = 1000;
    char r_data[10] = { 0 };
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    if (LOCAL_DEBUG)
        printf("\n[Debug][%s][%d][local_id: %d]", __FUNCTION__, __LINE__, local_id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];
#if 0
    if(local_id == THERMAL_CPU_CORE)
    {
        int rv = onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
        return rv;
    }
#endif

    if (thermali_current_temp_get(local_id, r_data) < 0)
        printf("[thermali]get current temperature fail!\n");

    info->mcelsius = (*r_data) * milli_unit;

    if (LOCAL_DEBUG)
        printf("\n[Debug][%s][%d][save data: %d]\n", __FUNCTION__, __LINE__, info->mcelsius);

    return ONLP_STATUS_OK;
}


