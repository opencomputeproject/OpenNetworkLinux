/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <x86_64_netberg_aurora_420_rangeley/x86_64_netberg_aurora_420_rangeley_config.h>
#include <onlp/platformi/psui.h>
#include <onlplib/file.h>
#include "x86_64_netberg_aurora_420_rangeley_int.h"
#include "x86_64_netberg_aurora_420_rangeley_log.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static onlp_psu_info_t psus__[] = {
    { }, /* Not used */
    {
        {
            PSU_OID_PSU1,
            "PSU-1",
            0,
            {
                FAN_OID_FAN9,
            },
        }
    },
    {
        {
            PSU_OID_PSU2,
            "PSU-2",
            0,
            {
                FAN_OID_FAN10,
            },
        }
    },
};

/*
 * This function will be called prior to any other onlp_psui functions.
 */
int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int rv;
    int pid;
    uint8_t data[256];
    int value = -1;
    int len;
    double dvalue;
    int i;

    VALIDATE(id);

    memset(info, 0, sizeof(onlp_psu_info_t));
    pid = ONLP_OID_ID_GET(id);
    *info = psus__[pid];

    rv = onlp_file_read_int(&value, SYS_HWMON1_PREFIX "/psu%d_abs", pid);
    if (rv != ONLP_STATUS_OK)
        return rv;
    if (value == 0)
    {
        info->status = ONLP_PSU_STATUS_UNPLUGGED;
        return ONLP_STATUS_OK;
    }

    /* PSU is present. */
    info->status = ONLP_PSU_STATUS_PRESENT;

    memset(data, 0, sizeof(data));
    rv = onlp_file_read(data, sizeof(data), &len, SYS_HWMON2_PREFIX "/psu%d_eeprom", pid);
    if (rv == ONLP_STATUS_OK)
    {
        i = 11;

        /* Manufacturer Name */
        len = (data[i]&0x0f);
        i++;
        i += len;

        /* Product Name */
        len = (data[i]&0x0f);
        i++;
        memcpy(info->model, (char *) &(data[i]), len);
        i += len;

        /* Product part,model number */
        len = (data[i]&0x0f);
        i++;
        i += len;

        /* Product Version */
        len = (data[i]&0x0f);
        i++;
        i += len;

        /* Product Serial Number */
        len = (data[i]&0x0f);
        i++;
        memcpy(info->serial, (char *) &(data[i]), len);
    }
    else
    {
        strcpy(info->model, "Missing");
        strcpy(info->serial, "Missing");
    }

    info->caps |= ONLP_PSU_CAPS_AC;

#if 0
    /* PSU is powered. */
    rv = onlp_file_read_int(&value, SYS_HWMON1_PREFIX "/psu%d_pg", pid);
    if (rv != ONLP_STATUS_OK)
        return rv;
    if (value == 0)
    {
        info->status |=  ONLP_PSU_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }
#endif

    memset(data, 0, sizeof(data));
    rv = onlp_file_read(data, sizeof(data), &len, SYS_HWMON2_PREFIX "/psu%d_iout", pid);
    if (rv == ONLP_STATUS_OK)
    {
        dvalue = atof((const char *)data);
        if (dvalue > 0.0)
        {
            info->caps |= ONLP_PSU_CAPS_IOUT;
            info->miout = (int)(dvalue * 1000);
        }
    }

    memset(data, 0, sizeof(data));
    rv = onlp_file_read(data, sizeof(data), &len, SYS_HWMON2_PREFIX "/psu%d_vout", pid);
    if (rv == ONLP_STATUS_OK)
    {
        dvalue = atof((const char *)data);
        if (dvalue > 0.0)
        {
            info->caps |= ONLP_PSU_CAPS_VOUT;
            info->mvout = (int)(dvalue * 1000);
        }
    }

    memset(data, 0, sizeof(data));
    rv = onlp_file_read(data, sizeof(data), &len, SYS_HWMON2_PREFIX "/psu%d_pin", pid);
    if (rv == ONLP_STATUS_OK)
    {
        dvalue = atof((const char *)data);
        if (dvalue > 0.0)
        {
            info->caps |= ONLP_PSU_CAPS_PIN;
            info->mpin = (int)(dvalue * 1000);
        }
    }

    memset(data, 0, sizeof(data));
    rv = onlp_file_read(data, sizeof(data), &len, SYS_HWMON2_PREFIX "/psu%d_pout", pid);
    if (rv == ONLP_STATUS_OK)
    {
        dvalue = atof((const char *)data);
        if (dvalue > 0.0)
        {
            info->caps |= ONLP_PSU_CAPS_POUT;
            info->mpout = (int)(dvalue * 1000);
        }
    }

    return ONLP_STATUS_OK;
}

/*
 * This is an optional generic ioctl() interface.
 * Its purpose is to allow future expansion and
 * custom functionality that is not otherwise exposed
 * in the standard interface.
 *
 * The semantics of this function are platform specific.
 * This function is completely optional.
 */
int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

