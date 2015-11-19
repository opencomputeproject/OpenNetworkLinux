/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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
#include <onlp/platformi/thermali.h>
#include <fcntl.h>

#define prefix_path "/sys/bus/i2c/devices/"
#define filename    "temp1_input"

#define local_debug 0


#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_6_ON_MAIN_BROAD,
    THERMAL_7_ON_MAIN_BROAD,
    THERMAL_8_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

static char last_path[][30] =  /* must map with onlp_thermal_id */
{
    "reserved",
    "4-004d/temp1",
    "4-004d/temp2",
    "4-004d/temp3",
    "4-004d/temp4",
    "4-004d/temp5",
    "4-004d/temp6",
    "4-004d/temp7",
    "4-004d/temp8",
    "6-003d/psu_temp1_input",
    "6-003e/psu_temp1_input",
};


/* Static values */
static onlp_thermal_info_t linfo[] = {
  { }, /* Not used */
  { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD),  "Chassis Thermal Sensor 1 (CPU)", 0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
  { { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD),  "Chassis Thermal Sensor 2 (Front middle)", 0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
  { { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD),  "Chassis Thermal Sensor 3 (Front left)", 0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
  { { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD),  "Chassis Thermal Sensor 4 (Rear right)", 0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
  { { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD),  "Chassis Thermal Sensor 5 (Rear middle)", 0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
  { { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD),  "Chassis Thermal Sensor 6 (Rear left)", 0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
  { { ONLP_THERMAL_ID_CREATE(THERMAL_7_ON_MAIN_BROAD),  "Chassis Thermal Sensor 7 (Front left corner)", 0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
  { { ONLP_THERMAL_ID_CREATE(THERMAL_8_ON_MAIN_BROAD),  "Chassis Thermal Sensor 8 (Next to PSU connector)", 0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
  { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(1)},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
  { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(2)},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 }
};


/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
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
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    int   fd, len, nbytes = 10, temp_base = 1, local_id;
    char  r_data[10]   = {0}, r2_data[10]   = {0};
    char  fullpath[50] = {0}, fullpath2[50] = {0};  //fullpath for get temp_input, fullpath for get temp_fault

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    if (local_debug)
        printf("\n[Debug][%s][%d][local_id: %d]", __FUNCTION__, __LINE__, local_id);

    if (local_id <= THERMAL_8_ON_MAIN_BROAD)
    {
        sprintf(fullpath, "%s%s%s", prefix_path, last_path[local_id], "_input");
        sprintf(fullpath2, "%s%s%s", prefix_path, last_path[local_id], "_fault");
    }
    else
    {
        sprintf(fullpath, "%s%s", prefix_path, last_path[local_id]);
    }

    if (local_debug)
    {
        printf("\n[Debug][%s][%d][openfile: %s, %s]", __FUNCTION__, __LINE__, fullpath, fullpath2);
    }

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];

    /* Set current mode */
    if ((fd = open(fullpath, O_RDONLY)) == -1)
    {
        return ONLP_STATUS_E_INTERNAL;
    }

    if ((len = read(fd, r_data, nbytes)) <= 0)
    {
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* If the read byte count is less, the format is different and calc will be wrong*/
    if (close(fd) == -1)
    {
        return ONLP_STATUS_E_INTERNAL;
    }

    if ((THERMAL_1_ON_MAIN_BROAD < local_id) && (local_id < THERMAL_1_ON_PSU1))
    {
        /* Set current mode */
        if ((fd = open(fullpath2, O_RDONLY)) == -1)
        {
            return ONLP_STATUS_E_INTERNAL;
        }

        if ((len = read(fd, r2_data, nbytes)) <= 0)
        {
            close(fd);
            return ONLP_STATUS_E_INTERNAL;
        }

        /* If the read byte count is less, the format is different and calc will be wrong*/
        if (close(fd) == -1)
        {
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    if (local_debug)
    {
        printf("\n[Debug][%s][%d][read data: %s, %s]", __FUNCTION__, __LINE__, r_data, r2_data);
    }

    info->mcelsius = atoi(r_data)/temp_base;

    if (atoi(r2_data) > 0)
        info->status |= ONLP_THERMAL_STATUS_FAILED;

    if (local_debug)
    {
        printf("\n[Debug][%s][%d][save data: %d, %u]\n", __FUNCTION__, __LINE__, info->mcelsius, info->status);
    }

    return ONLP_STATUS_OK;
}

