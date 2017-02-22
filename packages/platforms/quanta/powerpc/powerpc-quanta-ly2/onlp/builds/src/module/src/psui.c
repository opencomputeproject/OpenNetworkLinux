/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <powerpc_quanta_ly2/powerpc_quanta_ly2_config.h>
#include <onlp/platformi/psui.h>
#include <onlplib/file.h>
#include "powerpc_quanta_ly2_int.h"
#include "powerpc_quanta_ly2_log.h"

int
onlp_psui_init(void)
{
    return 0;
}

static onlp_psu_info_t psus__[] = {
    { }, /* Not used */
    {
        {
            PSU_OID_PSU1,
            "PSU-1",
            0,
            {
                FAN_OID_FAN5,
                THERMAL_OID_THERMAL6,
                THERMAL_OID_THERMAL7,
                THERMAL_OID_THERMAL8,
            },
        }
    },
    {
        {
            PSU_OID_PSU2,
            "PSU-2",
            0,
            {
                FAN_OID_FAN6,
                THERMAL_OID_THERMAL9,
                THERMAL_OID_THERMAL10,
                THERMAL_OID_THERMAL11,
            },
        }
    },
};

char* psu_paths[] = {
    NULL, /* Not used */
    "/sys/bus/i2c/devices/6-0058/hwmon/hwmon0",
    "/sys/bus/i2c/devices/7-0059/hwmon/hwmon1",
};

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int rv;
    int pid = ONLP_OID_ID_GET(id);
    *info = psus__[pid];
    const char* dir = psu_paths[pid];

    /*
     * Todo -- use the GPIO to determine PSU presence.
     * For this first version we'll approximate the status using
     * the input voltage sensor.
     */
    rv = onlp_file_read_int(&info->mvin, "%s/in1_input", dir);
    if(rv == ONLP_STATUS_E_MISSING || info->mvin == 0) {
        info->status &= ~1;
        return 0;
    }
    else if(rv < 0) {
        return rv;
    }
    else {
        info->caps |= ONLP_PSU_CAPS_VIN;
    }

    /* PSU is present and powered. */
    info->status |= 1;
    strcpy(info->model, "PSU-LY2");
    info->caps |= ONLP_PSU_CAPS_AC;

    if(onlp_file_read_int(&info->miin, "%s/curr1_input", dir) == 0) {
        info->caps |= ONLP_PSU_CAPS_IIN;
    }
    if(onlp_file_read_int(&info->miout, "%s/curr2_input", dir) == 0) {
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }
    if(onlp_file_read_int(&info->mvout, "%s/in2_input", dir) == 0) {
        info->caps |= ONLP_PSU_CAPS_VOUT;
        /* Empirical */
        info->mvout /= 500;
    }
    if(onlp_file_read_int(&info->mpin, "%s/power1_input", dir) == 0) {
        info->caps |= ONLP_PSU_CAPS_PIN;
        /* The pmbus driver reports power in micro-units */
        info->mpin /= 1000;
    }
    if(onlp_file_read_int(&info->mpout, "%s/power2_input", dir) == 0) {
        info->caps |= ONLP_PSU_CAPS_POUT;
        /* the pmbus driver reports power in micro-units */
        info->mpout /= 1000;
    }
    return ONLP_STATUS_OK;
}
