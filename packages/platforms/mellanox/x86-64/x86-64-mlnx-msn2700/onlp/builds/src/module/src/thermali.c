/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
#include <fcntl.h>
#include <unistd.h>
#include <AIM/aim_log.h>
#include <onlplib/file.h>
#include <onlplib/mmap.h>
#include <onlp/platformi/thermali.h>
#include "platform_lib.h"

#define prefix_path "/bsp/thermal"

/** CPU thermal_threshold */
typedef enum cpu_thermal_threshold_e {
    CPU_THERMAL_THRESHOLD_WARNING_DEFAULT  = 87000,
    CPU_THERMAL_THRESHOLD_ERROR_DEFAULT    = 100000,
    CPU_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT = 105000,
} cpu_thermal_threshold_t;

/**
 * Shortcut for CPU thermal threshold value.
 */
#define CPU_THERMAL_THRESHOLD_INIT_DEFAULTS  \
    { CPU_THERMAL_THRESHOLD_WARNING_DEFAULT, \
      CPU_THERMAL_THRESHOLD_ERROR_DEFAULT,   \
      CPU_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT }

/** Asic thermal_threshold */
typedef enum asic_thermal_threshold_e {
    ASIC_THERMAL_THRESHOLD_WARNING_DEFAULT  = 105000,
    ASIC_THERMAL_THRESHOLD_ERROR_DEFAULT    = 115000,
    ASIC_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT = 120000,
} asic_thermal_threshold_t;

/**
 * Shortcut for CPU thermal threshold value.
 */
#define ASIC_THERMAL_THRESHOLD_INIT_DEFAULTS  \
    { ASIC_THERMAL_THRESHOLD_WARNING_DEFAULT, \
      ASIC_THERMAL_THRESHOLD_ERROR_DEFAULT,   \
      ASIC_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT }

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE_0,
    THERMAL_CPU_CORE_1,
    THERMAL_CPU_PACK,
    THERMAL_ASIC,
    THERMAL_BOAR_AMB,
    THERMAL_PORT,
    THERMAL_ON_PSU1,
    THERMAL_ON_PSU2,
};

static char* last_path[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    "cpu_core0",
    "cpu_core1",
    "cpu_pack",
    "asic",
    "board_amb",
    "port_amb",
    "psu1",
    "psu2"
};

/* Static values */
static onlp_thermal_info_t linfo[] = {
    { }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_0), "CPU Core 0", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, CPU_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_1), "CPU Core 1", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, CPU_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_PACK), "CPU Pack", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, CPU_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_ASIC), "Asic Thermal Sensor", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ASIC_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_BOAR_AMB), "Board AMB Thermal Sensor", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0, {0,0,0}
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_PORT), "Port AMB Thermal Sensor", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0, {0,0,0}
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0, {0,0,0}
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0, {0,0,0}
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
    int   rv, len = 10, temp_base=1, local_id = 0;
    char  r_data[10]   = {0};
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];

    rv = onlp_file_read((uint8_t*)r_data, sizeof(r_data), &len, "%s/%s",
    		prefix_path, last_path[local_id]);
	if (rv < 0) {
		return ONLP_STATUS_E_INTERNAL;
	}

    info->mcelsius = atoi(r_data) / temp_base;

    return ONLP_STATUS_OK;
}

