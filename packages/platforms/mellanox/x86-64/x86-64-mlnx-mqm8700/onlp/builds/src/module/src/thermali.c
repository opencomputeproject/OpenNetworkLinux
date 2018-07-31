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
#include "mlnx_common/mlnx_common.h"

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE_0,
    THERMAL_CPU_CORE_1,
    THERMAL_CPU_PACK,
    THERMAL_ASIC,
    THERMAL_BOARD_AMB,
    THERMAL_PORT,
    THERMAL_ON_PSU1,
    THERMAL_ON_PSU2,
};

static char* thermal_fnames[] =  /* must map with onlp_thermal_id */
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
static onlp_thermal_info_t tinfo[] = {
    { }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_0), "CPU Core 0", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, CPU_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_1), "CPU Core 1", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, CPU_THERMAL_THRESHOLD_INIT_DEFAULTS
        },

    { { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_PACK), "CPU pack", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, CPU_THERMAL_THRESHOLD_INIT_DEFAULTS
        },

	{ { ONLP_THERMAL_ID_CREATE(THERMAL_ASIC), "Asic Thermal Sensor", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ASIC_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_BOARD_AMB), "Board AMB Thermal Sensor", 0},
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
    mlnx_platform_info_t* mlnx_platform_info = get_platform_info();
    mlnx_platform_info->tinfo=tinfo;
    mlnx_platform_info->thermal_fnames=thermal_fnames;
    return ONLP_STATUS_OK;
}
