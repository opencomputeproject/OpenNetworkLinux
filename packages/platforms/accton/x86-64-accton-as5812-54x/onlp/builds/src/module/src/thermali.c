/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
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

#define prefix_path "/sys/bus/i2c/devices/"
#define filename    "temp1_input"
#define LOCAL_DEBUG 0

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
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
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

static char* last_path[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    NULL,                  /* CPU_CORE files */
    "61-0048/",
    "62-0049/",
    "63-004a/",
    "57-003c/psu_",
    "58-003f/psu_",
};

#if 0 /* Temprarily comment below function to avoid compiler error
		 "implicit declaration of function 'onlp_file_read_int_max'" */
static char* cpu_coretemp_files[] =
    {
        "/sys/devices/platform/coretemp.0/temp2_input",
        "/sys/devices/platform/coretemp.0/temp3_input",
        "/sys/devices/platform/coretemp.0/temp4_input",
        "/sys/devices/platform/coretemp.0/temp5_input",
        NULL,
    };
#endif
	
/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core",   0},
            ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "Chassis Thermal Sensor 1 (Front middle)",   0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "Chassis Thermal Sensor 2 (Rear right)",    0},
	    ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "Chassis Thermal Sensor 3 (Front right)",    0},
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

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];

    if(local_id == THERMAL_CPU_CORE) {
	#if 0 /* Temprarily comment below function to avoid compiler error
	         "implicit declaration of function 'onlp_file_read_int_max'" */
        int rv = onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
        return rv;
	#else
		return ONLP_STATUS_OK;
	#endif
    }

    /* get fullpath */
    sprintf(fullpath, "%s%s%s", prefix_path, last_path[local_id], filename);

    OPEN_READ_FILE(fd,fullpath,r_data,nbytes,len);

    info->mcelsius = atoi(r_data)/temp_base;

    if (LOCAL_DEBUG)
        printf("\n[Debug][%s][%d][save data: %d]\n", __FUNCTION__, __LINE__, info->mcelsius);

    return ONLP_STATUS_OK;
}


