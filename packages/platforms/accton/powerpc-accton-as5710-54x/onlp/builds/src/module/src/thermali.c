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
#define LOCAL_DEBUG 0


#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)


#define OPEN_READ_FILE(fd,fullpath,data,nbytes,len) \
    if (LOCAL_DEBUG)                            \
        printf("[Debug][%s][%d][openfile: %s]\n", __FUNCTION__, __LINE__, fullpath); \
    if ((fd = open(fullpath, O_RDONLY)) == -1)  \
       return ONLP_STATUS_E_INTERNAL;           \
    if ((len = read(fd, r_data, nbytes)) <= 0){ \
        close(fd);                              \
        return ONLP_STATUS_E_INTERNAL;}         \
    if (LOCAL_DEBUG)                            \
        printf("[Debug][%s][%d][read data: %s]\n", __FUNCTION__, __LINE__, r_data); \
    if (close(fd) == -1)                        \
        return ONLP_STATUS_E_INTERNAL


enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_1_ON_CPU_BROAD,
    THERMAL_2_ON_CPU_BROAD,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

static char last_path[][30] =  /* must map with onlp_thermal_id */
{
    "reserved",
    "0-0018",
    "0-0018/temp2_input",
    "9-0048",
    "10-0049",
    "11-004a",
    "5-003c/psu_temp1_input",
    "6-003f/psu_temp1_input",
};


/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_CPU_BROAD),  "Chassis Thermal Sensor 1 (Sensor on CPU board)", 0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_CPU_BROAD),  "Chassis Thermal Sensor 2 (Measurement point on CPU)",     0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "Chassis Thermal Sensor 3 (Front middle)",   0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "Chassis Thermal Sensor 4 (Rear right)",    0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "Chassis Thermal Sensor 5 (Front right)",    0},
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
    int   fd, len, nbytes = 10, temp_base=1, local_id;
    char  r_data[10]   = {0};
    char  fullpath[50] = {0};
    VALIDATE(id);

	local_id = ONLP_OID_ID_GET(id);

    if (LOCAL_DEBUG)
        printf("\n[Debug][%s][%d][local_id: %d]", __FUNCTION__, __LINE__, local_id);

    /* get fullpath */
    if (strchr(last_path[local_id], '/') != NULL)
	{
        sprintf(fullpath, "%s%s", prefix_path, last_path[local_id]);
	}
	else
	{
        sprintf(fullpath, "%s%s/%s", prefix_path, last_path[local_id], filename);
    }

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];

    OPEN_READ_FILE(fd,fullpath,r_data,nbytes,len);

    info->mcelsius = atoi(r_data)/temp_base;

    if (LOCAL_DEBUG)
        printf("\n[Debug][%s][%d][save data: %d]\n", __FUNCTION__, __LINE__, info->mcelsius);


    if (THERMAL_2_ON_CPU_BROAD == local_id) /* need to check fault status */
    {
        /* get fullpath */
        sprintf(fullpath, "%s%s", prefix_path, "0-0018/temp2_fault");

        OPEN_READ_FILE(fd,fullpath,r_data,nbytes,len);

        if (atoi(r_data)>0)
        info->status |= ONLP_THERMAL_STATUS_FAILED;
    }

    return ONLP_STATUS_OK;
}


