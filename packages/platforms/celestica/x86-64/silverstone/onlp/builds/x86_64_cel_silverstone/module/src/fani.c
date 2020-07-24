#include <onlp/platformi/fani.h>
#include "platform.h"

onlp_fan_info_t f_info[FAN_COUNT + 1] = {
    {},
    {
        {ONLP_FAN_ID_CREATE(1), "Chassis Fan 1", 0},
        0,
        ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        {ONLP_FAN_ID_CREATE(2), "Chassis Fan 2", 0},
        0,
        ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        {ONLP_FAN_ID_CREATE(3), "Chassis Fan 3", 0},
        0,
        ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        {ONLP_FAN_ID_CREATE(4), "Chassis Fan 4", 0},
        0,
        ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        {ONLP_FAN_ID_CREATE(5), "Chassis Fan 5", 0},
        0,
        ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        {ONLP_FAN_ID_CREATE(6), "Chassis Fan 6", 0},
        0,
        ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        {ONLP_FAN_ID_CREATE(7), "Chassis Fan 7", 0},
        0,
        ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        {ONLP_FAN_ID_CREATE(8), "PSU Fan 1", ONLP_PSU_ID_CREATE(1)},
        0,
        ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
    {
        {ONLP_FAN_ID_CREATE(9), "PSU Fan 2", ONLP_PSU_ID_CREATE(2)},
        0,
        ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_F2B | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,
    },
};

int onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}

int onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t *info_p)
{
    int fan_id;

    fan_id = ONLP_OID_ID_GET(id);

    *info_p = f_info[fan_id];

    uint8_t spd_result;
    int isfanb2f = 0;

    if(fan_id <= 7){
        get_fan_info(fan_id, info_p->model, info_p->serial,&isfanb2f);
    }else{
        int psu_id = 0;
        if(fan_id == 8){
            psu_id = 1;
        }else if(fan_id == 9){
            psu_id = 2;
        }
        get_psu_model_sn(psu_id, info_p->model, info_p->serial);
        isfanb2f = -1;
    }
    

    spd_result = get_fan_speed(fan_id,&(info_p->percentage), &(info_p->rpm));
    if(spd_result){
        return ONLP_FAN_STATUS_FAILED;
    }

    info_p->status |= ONLP_FAN_STATUS_PRESENT;

    switch (isfanb2f)
    {
    case ONLP_FAN_STATUS_F2B:
        info_p->status |= ONLP_FAN_STATUS_F2B;
        break;
    case ONLP_FAN_STATUS_B2F:
        info_p->status |= ONLP_FAN_STATUS_B2F;
        break;
    default:
        break;
    }

    return ONLP_STATUS_OK;
}
