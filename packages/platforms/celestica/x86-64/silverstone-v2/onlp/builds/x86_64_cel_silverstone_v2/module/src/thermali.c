#include <unistd.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <fcntl.h>
#include "platform_common.h"


static onlp_thermal_info_t thermal_info[] = {
    { },
    { { ONLP_THERMAL_ID_CREATE(TEMP_SW_U52_ID), "TEMP_SW_U52", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 
        },
    { { ONLP_THERMAL_ID_CREATE(TEMP_SW_U16_ID), "TEMP_SW_U16",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0 
        },
    { { ONLP_THERMAL_ID_CREATE(TEMP_FB_U52_ID), "TEMP_FB_U52",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 
        },
    { { ONLP_THERMAL_ID_CREATE(TEMP_FB_U17_ID), "TEMP_FB_U17",   0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0 
        },
    { { ONLP_THERMAL_ID_CREATE(VDD_CORE_Temp_ID), "VDD_CORE_Temp",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0 
        },
    { { ONLP_THERMAL_ID_CREATE(XP0R8V_Temp_ID), "XP0R8V_Temp",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0 
        }, 
    { { ONLP_THERMAL_ID_CREATE(XP3R3V_R_Temp_ID), "XP3R3V_R_Temp",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0 
        },
    { { ONLP_THERMAL_ID_CREATE(XP3R3V_L_Temp_ID), "XP3R3V_L_Temp",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0 
        },
    { { ONLP_THERMAL_ID_CREATE(TEMP_SW_Internal_ID), "TEMP_SW_Internal",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD | ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD, 0
        },
    { { ONLP_THERMAL_ID_CREATE(CPU_THERMAL_ID), "TEMP_CPU",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0, 
        },  
    { { ONLP_THERMAL_ID_CREATE(TEMP_DIMMA0_ID), "TEMP_DIMMA0",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0, 
        },
    { { ONLP_THERMAL_ID_CREATE(TEMP_DIMMB0_ID), "TEMP_DIMMB0",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0, 
        },
    { { ONLP_THERMAL_ID_CREATE(PSU1_THERMAL1_ID), "PSU1_Temp1",   ONLP_PSU_ID_CREATE(1)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0
        },
    { { ONLP_THERMAL_ID_CREATE(PSU1_THERMAL2_ID), "PSU1_Temp2",   ONLP_PSU_ID_CREATE(1)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0
        },
    { { ONLP_THERMAL_ID_CREATE(PSU1_THERMAL3_ID), "PSU1_Temp3",   ONLP_PSU_ID_CREATE(1)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0
        },
    { { ONLP_THERMAL_ID_CREATE(PSU2_THERMAL1_ID), "PSU2_Temp1",   ONLP_PSU_ID_CREATE(2)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0
        },
    { { ONLP_THERMAL_ID_CREATE(PSU2_THERMAL2_ID), "PSU2_Temp2",   ONLP_PSU_ID_CREATE(2)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0
        },
    { { ONLP_THERMAL_ID_CREATE(PSU2_THERMAL3_ID), "PSU2_Temp3",   ONLP_PSU_ID_CREATE(2)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_GET_TEMPERATURE | ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD, 0
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
