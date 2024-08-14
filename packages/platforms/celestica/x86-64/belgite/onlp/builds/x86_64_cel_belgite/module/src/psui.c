#include <onlp/platformi/psui.h>
#include "platform.h"

static onlp_psu_info_t psu_info[] =
    {
        {},
        {
            {ONLP_PSU_ID_CREATE(PSU1_ID), "PSU1", 0},
            "",
            "",
            0,
            ONLP_PSU_CAPS_AC | ONLP_PSU_CAPS_VIN | ONLP_PSU_CAPS_VOUT | ONLP_PSU_CAPS_IIN | ONLP_PSU_CAPS_IOUT | ONLP_PSU_CAPS_PIN | ONLP_PSU_CAPS_POUT,
        },
        {
            {ONLP_PSU_ID_CREATE(PSU2_ID), "PSU2", 0},
            "",
            "",
            0,
            ONLP_PSU_CAPS_AC | ONLP_PSU_CAPS_VIN | ONLP_PSU_CAPS_VOUT | ONLP_PSU_CAPS_IIN | ONLP_PSU_CAPS_IOUT | ONLP_PSU_CAPS_PIN | ONLP_PSU_CAPS_POUT,
        }};

static const struct psu_reg_bit_mapper psu_mapper[PSU_COUNT + 1] = {
    {},
    {0xa141, 4, 0, 2},
    {0xa141, 5, 0, 3},
};

uint8_t psu_status = 0;

struct psuInfo_p temp_info[] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

int onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

int onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t *info_p)
{
    int psu_id,psu_offset=0;;
    psu_id = ONLP_OID_ID_GET(id);
    *info_p = psu_info[psu_id];

    int present_status=0,pow_status=0;

    if(psu_status == 0)
        psu_status = get_psu_status(psu_id);

    present_status = (psu_status >> psu_mapper[psu_id].bit_present) & 0x01;
    pow_status = (psu_status >> psu_mapper[psu_id].bit_pow_sta) & 0x01;

    if (present_status == 0)
    {
        info_p->status |= ONLP_PSU_STATUS_PRESENT;
        if (pow_status == 0)
            info_p->status |= ONLP_PSU_STATUS_UNPLUGGED;
    }
    else
    {
        info_p->status = ONLP_PSU_STATUS_FAILED;
    }

    get_psu_model_sn(psu_id,info_p->model,info_p->serial);

    get_psu_info(psu_id,&(info_p->mvin),&(info_p->mvout),&(info_p->mpin),&(info_p->mpout),&(info_p->miin),&(info_p->miout));
    if((info_p->mvin == 0) && (info_p->mpin == 0) && (info_p->miin == 0)){
        info_p->status |= ONLP_PSU_STATUS_UNPLUGGED;
    }

    if(psu_id == 1){
        psu_offset = 1;
    }else if(psu_id == 2){
        psu_offset = 3;
    }

    info_p->hdr.coids[0] = ONLP_THERMAL_ID_CREATE(psu_offset + CHASSIS_THERMAL_COUNT);
    info_p->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(psu_offset+1 + CHASSIS_THERMAL_COUNT);
    info_p->hdr.coids[2] = ONLP_FAN_ID_CREATE(psu_id + CHASSIS_FAN_COUNT);

    return ONLP_STATUS_OK;
}
