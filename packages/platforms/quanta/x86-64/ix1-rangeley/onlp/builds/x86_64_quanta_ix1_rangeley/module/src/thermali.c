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
 *
 *
 ***********************************************************/
#include <onlp/platformi/thermali.h>
#include <onlplib/file.h>
#include "x86_64_quanta_ix1_rangeley_int.h"
#include "x86_64_quanta_ix1_rangeley_log.h"

int
onlp_thermali_init(void)
{
    return ONLP_STATUS_OK;
}

static int
sys_thermal_info_get__(onlp_thermal_info_t* info, int id)
{
    int rv;

    rv = onlp_file_read_int(&info->mcelsius,
                            SYS_HWMON_PREFIX "/temp%d_input", id);

    if(rv == ONLP_STATUS_E_INTERNAL) {
        return rv;
    }

    if(rv == ONLP_STATUS_E_MISSING) {
        info->status &= ~1;
        return 0;
    }

    return ONLP_STATUS_OK;
}

static int
psu_thermal_info_get__(onlp_thermal_info_t* info, int pid, int id)
{
    /* THERMAL6 -> PSU1 */
    /* THERMAL7 -> PSU2 */
    extern struct psu_info_s psu_info[];
    char* dir = psu_info[pid].path;

    info->status |= 1;
    return onlp_file_read_int(&info->mcelsius, "%s/temp%d_input", dir, id);
}

int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* rv)
{
    int tid = ONLP_OID_ID_GET(id);

    static onlp_thermal_info_t info[] = {
        { }, /* Not used */
        { { ONLP_THERMAL_ID_CREATE(1),  "Chassis Thermal 1",  0}, ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
        { { ONLP_THERMAL_ID_CREATE(2),  "Chassis Thermal 2",  0}, ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
        { { ONLP_THERMAL_ID_CREATE(3),  "Chassis Thermal 3",  0}, ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
        { { ONLP_THERMAL_ID_CREATE(4),  "Chassis Thermal 4",  0}, ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
        { { ONLP_THERMAL_ID_CREATE(5),  "Chassis Thermal 5",  0}, ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
        { { ONLP_THERMAL_ID_CREATE(6),  "Chassis Thermal 6",  0}, ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
        { { ONLP_THERMAL_ID_CREATE(7),  "Chassis Thermal 7",  0}, ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
        { { ONLP_THERMAL_ID_CREATE(8),  "Chassis Thermal 8",  0}, ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
        { { ONLP_THERMAL_ID_CREATE(9),  "Chassis Thermal 9",  0}, ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
        { { ONLP_THERMAL_ID_CREATE(10), "Chassis Thermal 10", 0}, ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },

        { { ONLP_THERMAL_ID_CREATE(11), "PSU-1 Thermal 1", 0 } },
        { { ONLP_THERMAL_ID_CREATE(12), "PSU-1 Thermal 2", 0 } },
        { { ONLP_THERMAL_ID_CREATE(13), "PSU-1 Thermal 3", 0 } },

        { { ONLP_THERMAL_ID_CREATE(14), "PSU-2 Thermal 1", 0 } },
        { { ONLP_THERMAL_ID_CREATE(15), "PSU-2 Thermal 2", 0 } },
        { { ONLP_THERMAL_ID_CREATE(16), "PSU-2 Thermal 3", 0 } },
    };

    *rv = info[tid];
    rv->caps |= ONLP_THERMAL_CAPS_GET_TEMPERATURE;

    switch(tid)
        {
        case THERMAL_ID_THERMAL1:
        case THERMAL_ID_THERMAL2:
        case THERMAL_ID_THERMAL3:
        case THERMAL_ID_THERMAL4:
        case THERMAL_ID_THERMAL5:
        case THERMAL_ID_THERMAL6:
        case THERMAL_ID_THERMAL7:
        case THERMAL_ID_THERMAL8:
        case THERMAL_ID_THERMAL9:
        case THERMAL_ID_THERMAL10:
            return sys_thermal_info_get__(rv, tid);

        case THERMAL_ID_THERMAL11:
        case THERMAL_ID_THERMAL12:
        case THERMAL_ID_THERMAL13:
            return psu_thermal_info_get__(rv, 1, tid - THERMAL_ID_THERMAL11 + 1);


        case THERMAL_ID_THERMAL14:
        case THERMAL_ID_THERMAL15:
        case THERMAL_ID_THERMAL16:
            return psu_thermal_info_get__(rv, 2, tid - THERMAL_ID_THERMAL14 + 1);

        }

    return ONLP_STATUS_E_INVALID;
}
