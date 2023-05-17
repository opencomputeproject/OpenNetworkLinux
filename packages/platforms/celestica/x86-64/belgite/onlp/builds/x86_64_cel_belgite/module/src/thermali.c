#include <unistd.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <fcntl.h>
#include "platform.h"

static onlp_thermal_info_t thermal_info[] = {
    { },
    { { ONLP_THERMAL_ID_CREATE(CHASSIS_THERMAL1_ID), "Switchboard_Inlet_Temp_U10", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD, 0 
        },
    { { ONLP_THERMAL_ID_CREATE(CHASSIS_THERMAL2_ID), "Switchboard_Inlet_Temp_U4",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD, 0 
        },
    { { ONLP_THERMAL_ID_CREATE(CHASSIS_THERMAL3_ID), "Switchboard_Inlet_Temp_U60",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
    { { ONLP_THERMAL_ID_CREATE(CHASSIS_THERMAL4_ID), "Switchboard_Inlet_Temp_U7",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD, 0 
        },
    { { ONLP_THERMAL_ID_CREATE(CPU_THERMAL_ID), "CPU_Internal_Temp",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
    { { ONLP_THERMAL_ID_CREATE(PSU1_THERMAL1_ID), "PSU1_Temp1",   ONLP_PSU_ID_CREATE(1)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0
        },
    { { ONLP_THERMAL_ID_CREATE(PSU1_THERMAL2_ID), "PSU1_Temp2",   ONLP_PSU_ID_CREATE(1)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0
        },
    { { ONLP_THERMAL_ID_CREATE(PSU2_THERMAL1_ID), "PSU2_Temp1",   ONLP_PSU_ID_CREATE(2)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0
        },
    { { ONLP_THERMAL_ID_CREATE(PSU2_THERMAL2_ID), "PSU2_Temp2",   ONLP_PSU_ID_CREATE(2)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0
        },
};

int onlp_thermali_init(void)
{
    return ONLP_STATUS_OK;
}

int onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t *info_p)
{
    int thermal_id;
    int thermal_status = 0;
    int temp, warn, err, shutdown;

    thermal_id = ONLP_OID_ID_GET(id);
    memcpy(info_p, &thermal_info[thermal_id], sizeof(onlp_thermal_info_t));

    /* Get thermal temperature. */
    thermal_status = get_sensor_info(thermal_id, &temp, &warn, &err, &shutdown);
    if (-1 == thermal_status)
    {
        info_p->status = ONLP_THERMAL_STATUS_FAILED;
    }
    else
    {
        info_p->status = ONLP_THERMAL_STATUS_PRESENT;
        info_p->mcelsius = temp;
        info_p->thresholds.warning = warn;
        info_p->thresholds.error = err;
        info_p->thresholds.shutdown = shutdown;
    }

    return ONLP_STATUS_OK;
}
