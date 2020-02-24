#include <onlp/platformi/fani.h>

#include "i2c_chips.h"
#include "platform.h"

onlp_fan_info_t f_info[FAN_COUNT + 1] = {
    {
        { ONLP_FAN_ID_CREATE(1), "Chassis Fan 1", 0 },
        0x0,
    },
    {
        { ONLP_FAN_ID_CREATE(2), "Chassis Fan 2", 0 },
        0x0,
    },
    {
        { ONLP_FAN_ID_CREATE(3), "Chassis Fan 3", 0 },
        0x0,
    },
    {
        { ONLP_FAN_ID_CREATE(4), "Chassis Fan 4", 0 },
        0x0,
    },
    {
        { ONLP_FAN_ID_CREATE(5), "Chassis Fan 5", 0 },
        0x0,
    },
    {
        { ONLP_FAN_ID_CREATE(6), "Chassis Fan 6", 0 },
        0x0,
    },
    {
        { ONLP_FAN_ID_CREATE(7), "Chassis Fan 7", 0 },
        0x0,
    },
    {
        { ONLP_FAN_ID_CREATE(8), "Chassis Fan 8", 0 },
        0x0,
    },
    {
        { ONLP_FAN_ID_CREATE(8), "PSU Fan 1", 0 },
        0x0,
    },
    {
        { ONLP_FAN_ID_CREATE(8), "PSU Fan 2", 0 },
        0x0,
    },
};


int
onlp_fani_init(void)
{
    fanInit();
    return ONLP_STATUS_OK;
}

int
onlp_fani_rpm_set(onlp_oid_t id, int rpm)
{
    int fan_id;
    unsigned short p;

    /* Max speed 12000 RPM. 1 % is 120 RPM */
    p = (unsigned short) (rpm/120);
    if ( p > 100)
        p = 100;

    fan_id = ONLP_OID_ID_GET(id) - 1;

    fanSpeedSet(fan_id, p);
    return ONLP_STATUS_OK;
}

int
onlp_fani_percentage_set(onlp_oid_t id, int p)
{
    int fan_id;

    fan_id = ONLP_OID_ID_GET(id) - 1;

    fanSpeedSet(fan_id, (unsigned short)p);
    return ONLP_STATUS_OK;
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    int fan_id;

    fan_id = ONLP_OID_ID_GET(id) - 1;

    if (fan_id > FAN_COUNT)
        return ONLP_STATUS_E_INTERNAL;

    *info = f_info[fan_id];

    if (!getFanPresent(fan_id))
        info->status |= ONLP_FAN_STATUS_PRESENT;
    else
        return ONLP_STATUS_E_MISSING;

    if (getFanAirflow(fan_id))
        info->status |= ONLP_FAN_STATUS_F2B;
    else
        info->status |= ONLP_FAN_STATUS_B2F;

    fanSpeedGet(fan_id, &(info->rpm));
    fanPwmGet(fan_id, &(info->percentage));

    return ONLP_STATUS_OK;
}
