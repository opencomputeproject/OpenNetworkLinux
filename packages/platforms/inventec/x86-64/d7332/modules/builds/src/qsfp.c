#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include "sff.h"
#include "qsfp.h"

#define QSFP_ID_OFFSET (0)
#define QSFP_ST_INDICATOR2_OFFSET (2)
#define DATA_NOT_READY_BIT (0)
#define QSFP_ETH_COMP_OFFSET (131)
#define QSFP_ETH_EXT_COMP_OFFSET (192)
#define QSFP_VCC_OFFSET (26)
#define QSFP_TEMP_OFFSET (22)
#define QSFP_EXT_ID_OFFSET (129)
#define CDR_RX_PRS_BIT (2)
#define CDR_TX_PRS_BIT (3)
#define QSFP_LN_RX_PWR_OFFSET (34)
#define QSFP_LN_TX_PWR_OFFSET (50)
#define QSFP_LN_TX_BIAS_OFFSET (42)
#define QSFP_OPT_LN_CONTROL_TX_EQ_OFFSET (234)
#define QSFP_OPT_LN_CONTROL_RX_EM_OFFSET (236)
#define QSFP_OPT_LN_CONTROL_RX_AM_OFFSET (238)
#define QSFP_CDR_CONTROL_OFFSET (98)
#define QSFP_LN_TX_DISABLE_OFFSET (86)

#define QSFP_VENDOR_NAME_OFFSET (148)
#define QSFP_VENDOR_PN_OFFSET (168)
#define QSFP_VENDOR_SN_OFFSET (196)
#define QSFP_VENDOR_REV_OFFSET (184)

#define QSFP_VENDOR_NAME_LEN (16)
#define QSFP_VENDOR_PN_LEN (16)
#define QSFP_VENDOR_SN_LEN (16)
#define QSFP_VENDOR_REV_LEN (2)

#define bit_mask(bit) (1 << (bit))
/*bit:start bit from rightmost , num: the num of bits what to be extracted*/
#define bits_get(reg, bit, num) ((reg >> bit) & ((1 << num)-1))
static int qsfp_int_flag_read(struct sff_obj_t *sff_obj);
const u8 Qsfp_Eth_Comp_Table[] = {
    ACTIVE_CABLE,
    LR4_40GBASE,
    SR4_40GBASE,
    CR4_40GBASE,
    /*10G Ethernet Compliance Codes*/
    SR_10GBASE,
    LR_10GBASE,
    LRM_10GBASE,
    ER_10GBASE,/*COMPLIANCE_RSVD*/
};
int qsfp_id_get(struct sff_obj_t *sff_obj, u8 *id)
{
    if(!id) {
        return -EINVAL;
    }
    return sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, QSFP_ID_OFFSET, id, 1);
}
int qsfp_ext_id_get(struct sff_obj_t *sff_obj, u8 *ext_id)
{
    if(!ext_id) {
        return -EINVAL;
    }

    return sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, QSFP_EXT_ID_OFFSET, ext_id, 1);
}
static int _qsfp_data_ready_check(struct sff_obj_t *sff_obj, bool *ready)
{
    int ret = 0;
    u8 st = 0;
    if(!ready) {
        return -EINVAL;
    }

    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, QSFP_ST_INDICATOR2_OFFSET, &st, 1)) < 0) {
        return ret;
    }
    if (st & bit_mask(DATA_NOT_READY_BIT)) {
        *ready = false;
    } else {
        *ready = true;
    }
    return 0;

#if 0
    /*<TBD> special case:ex: AVAGO sn:AF1741GG04J pn: AFBR-89CDDZ*/

    SFF_MGR_DEBUG("special_case of checking data ready\n");
    if((ret = sfp_id_get(sff_obj, &val)) < 0) {
        return ret;
    }
    *is_ready = 1;
    return 0;
#endif
}
static int _qsfp_ethernet_compliance_read(struct sff_obj_t *sff_obj, u8 *eth_comp)
{
    int ret = 0;
    u8 data = 0;

    if(!eth_comp) {
        SFF_MGR_ERR("invalid para");
        return -EINVAL;
    }
    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, QSFP_ETH_COMP_OFFSET, &data, 1)) < 0) {
        SFF_MGR_ERR("sff_obj->func_tbl->eeprom_read fail");
        return ret;
    }
    *eth_comp = data;

    return 0;
}
static int _qsfp_eth_extended_compliance_get(struct sff_obj_t *sff_obj, u8 *eth_ext_comp)
{
    u8 data = 0;
    int ret = 0;

    if(!eth_ext_comp) {
        SFF_MGR_ERR("invalid para");
        return -EINVAL;
    }

    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, QSFP_ETH_EXT_COMP_OFFSET, &data, 1)) < 0) {
        return ret;
    }
    *eth_ext_comp = data;

    return 0;
}
static bool qsfp_eth_comp_is_supported(struct sff_obj_t *sff_obj, u8 eth_comp)
{
    bool is_supported = false;
    int size = ARRAY_SIZE(Qsfp_Eth_Comp_Table);
    int idx = 0;
    u8 comp_codes = 0;
    for (idx = 0; idx < size; idx++) {
        if(eth_comp & Qsfp_Eth_Comp_Table[idx]) {
            comp_codes = eth_comp & Qsfp_Eth_Comp_Table[idx];
            SFF_MGR_DEBUG("port:%d known eth_cmp:0x%x\n", sff_obj->port, eth_comp & Qsfp_Eth_Comp_Table[idx]);
            break;
        }
    }
    if(idx >= size) {

        SFF_MGR_DEBUG("port:%d unknown eth_cmp:0x%x\n", sff_obj->port, eth_comp);
    }

    switch (comp_codes) {
    case ACTIVE_CABLE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_40G_AOC);
        break;
    case LR4_40GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_40G_LR4);
        break;
    case SR4_40GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_40G_SR4);
        break;
    case CR4_40GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_COPPER_L4_40G);
        break;
    default:
        break;
    }

    if (TRANSVR_CLASS_UNKNOWN != transvr_type_get(sff_obj)) {
        is_supported = true;
        SFF_MGR_DEBUG("port:%d known eth_ext_cmp:0x%x transvr:%d\n",
                      sff_obj->port, eth_comp, transvr_type_get(sff_obj));
    }
    return is_supported;
}
static bool qsfp_eth_ext_is_supported(struct sff_obj_t *sff_obj, u8 eth_ext_comp)
{
    bool is_supported = false;
    Ethernet_extended_compliance eth_ext_code = eth_ext_comp;

    switch(eth_ext_code) {
    case AOC_100G_BER_5:
    case AOC_100G_BER_12:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_100G_AOC);
        break;
    case SR4_100GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_100G_SR4);
        break;
    case LR4_100GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_100G_LR4);
        break;
    case ER4_100GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_100G_ER4);
        break;
    case PSM4_100G_SMF:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_100G_PSM4);
        break;
    case CR4_100GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_COPPER_L4_100G);
        break;
    case SR10_100GBASE:
    case CWDM4_100G:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_100G_CWDM4);
    case ACC_100G_BER_5:
    case ACC_100G_BER_12:
    case ER4_40GBASE:
    case SR_10GBASE_4:
    case PSM4_40G_SMF:
    case CLR4_100G:
    case DWDM2_100GE:
        SFF_MGR_DEBUG("port:%d known eth_ext_cmp:0x%x\n", sff_obj->port, eth_ext_comp);

        break;
    default:
        SFF_MGR_DEBUG("port:%d unknown eth_ext_cmp:%d\n", sff_obj->port, eth_ext_comp);

        break;
    }

    if (TRANSVR_CLASS_UNKNOWN != transvr_type_get(sff_obj)) {
        is_supported = true;
        SFF_MGR_DEBUG("port:%d known eth_ext_cmp:0x%x transvr:%d\n",
                      sff_obj->port, eth_ext_comp, transvr_type_get(sff_obj));
    }
    return is_supported;
}

static int qsfp_type_identify(struct sff_obj_t *sff_obj, bool *found)
{
    u8 id = 0;
    u8 eth_comp = 0;
    u8 eth_ext_comp = 0;
    int ret = 0;

    if (!found) {
        return -EINVAL;
    }
    if((ret = qsfp_id_get(sff_obj, &id)) < 0) {
        goto exit_err;
    }
    if(0x0d != id &&
            0x11 != id &&
            0x0c != id) { // not qsfp+ (0x0d), qsfp28 (0x11), qsfp(INF 8438)(0xc)
        SFF_MGR_ERR("unknown id:0x%x\n", id);
        goto exit_non_support_err; /*<TBD> error handling in the future*/
    }

    if((ret = _qsfp_ethernet_compliance_read(sff_obj, &eth_comp)) < 0) {
        goto exit_err;
    }

    if (eth_comp & 0x80) { //bit7 == 1
        if((ret = _qsfp_eth_extended_compliance_get(sff_obj, &eth_ext_comp)) < 0) {
            goto exit_err;
        }

        if(!qsfp_eth_ext_is_supported(sff_obj, eth_ext_comp)) {
            goto exit_non_support_err;
        }
    } else { /*40G*/
        /* ACTIVE_CABLE = 1 << 0,
        *         *     LR4_40GBASE = 1 << 1,
        *                 *         SR4_40GBASE = 1 << 2,
        *                         *             CR4_40GBASE = 1 << 3,
        *                                 *
        *                                         */

        if(!qsfp_eth_comp_is_supported(sff_obj, eth_comp)) {
            goto exit_non_support_err;
        }
    }
    *found = true;
    return 0;

exit_non_support_err:
    *found = false;
    return 0;
exit_err: /*could be i2c fail , need to define err code in the future <TBD>*/
    return ret;
}

int qsfp_lane_control_set(struct sff_obj_t *sff_obj, int type, u32 value)
{
    int ret = 0;
    u8 page = QSFP_PAGE3;
    u8 offset = 0;
    bool fail = false;
    u8 ch_ctrl[WORD_SIZE];

    ch_ctrl[0] = (value >> 8) & 0xff;
    ch_ctrl[1] = value & 0xff;

    switch(type) {

    case TX_EQ_TYPE:
        offset = QSFP_OPT_LN_CONTROL_TX_EQ_OFFSET;
        break;

    case RX_EM_TYPE:
        offset = QSFP_OPT_LN_CONTROL_RX_EM_OFFSET;
        break;

    case RX_AM_TYPE:
        offset = QSFP_OPT_LN_CONTROL_RX_AM_OFFSET;
        break;
    default:
        fail = true;
        break;
    }

    if(fail) {
        return -EBADRQC;
    }
    if((ret = sff_obj->func_tbl->eeprom_write(sff_obj, page, offset, ch_ctrl, WORD_SIZE)) < 0) {
        return ret;
    }

    return 0;
}

int qsfp_lane_control_get(struct sff_obj_t *sff_obj, int type, u32 *value)
{
    int ret = 0;
    u8 page = QSFP_PAGE3;
    u8 offset = 0;
    bool fail = false;
    u8 ch_ctrl[WORD_SIZE];
    if (!value) {
        return -EINVAL;
    }
    switch(type) {

    case TX_EQ_TYPE:
        offset = QSFP_OPT_LN_CONTROL_TX_EQ_OFFSET;
        break;

    case RX_EM_TYPE:
        offset = QSFP_OPT_LN_CONTROL_RX_EM_OFFSET;
        break;

    case RX_AM_TYPE:
        offset = QSFP_OPT_LN_CONTROL_RX_AM_OFFSET;
        break;
    default:
        fail = true;
        break;
    }

    if(fail) {
        return -EBADRQC;
    }

    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, page, offset, ch_ctrl, sizeof(ch_ctrl))) < 0) {
        return ret;
    }
    *value = ((u32)ch_ctrl[0] << 8) | ((u32)ch_ctrl[1]);
    return 0;
}
int qsfp_lane_status_get(struct sff_obj_t *sff_obj, int type, u8 *st)
{
    u8 intL = 0;
    int ret = 0;
    if ((ret = sff_obj->func_tbl->intL_get(sff_obj, &intL)) < 0) {
        return ret;
    }
    if (intL) {
        if ((ret = qsfp_int_flag_read(sff_obj)) < 0) {
            return ret;
        }
    }

    *st = sff_obj->priv_data.qsfp.lane_st[type];
    return 0;
}
static int qsfp_lane_status_update(struct sff_obj_t *sff_obj, int type, u8 value)
{
    if (type > LN_STATUS_NUM ||
            type < LN_STATUS_RX_LOS_TYPE) {
        return -EINVAL;
    }

    sff_obj->priv_data.qsfp.lane_st[type] = value;

    return 0;
}

int qsfp_temperature_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    int ret = 0;
    u8 reg[WORD_SIZE];
    s16 temp = 0;
    s16 data_h = 0;
    s16 data_l = 0;
    s16 divider = 256;
    s16 decimal = 0;
    char *unit = "c";

    if (!buf) {
        return -EINVAL;
    }

    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, QSFP_TEMP_OFFSET, reg, sizeof(reg))) < 0) {
        return ret;
    }

    data_h = reg[0];
    data_l = reg[1];
    temp = (data_h << 8) | data_l;

    decimal = ((temp/divider)*divider)-temp;
    decimal = abs(decimal);
    decimal = decimal*1000/divider;

    scnprintf(buf, buf_size,
              "%d.%d %s\n",
              temp/divider,
              decimal, unit);

    return 0;
}

int qsfp_voltage_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    u8 reg[WORD_SIZE];
    u16 vol_data = 0;
    u16 data_h;
    u16 data_l;
    int ret = 0;
    u16 divider = 10000;
    char *unit = "v";

    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, QSFP_VCC_OFFSET, reg, sizeof(reg))) < 0) {
        SFF_MGR_ERR("NG1\n");
        return ret;
    }
    data_h = reg[0];
    data_l = reg[1];
    vol_data = (data_h << 8) | data_l;

    scnprintf(buf, buf_size,
              "%d.%d %s\n",
              vol_data/divider,
              vol_data%divider, unit);

    return 0;
}
int qsfp_lane_monitor_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size)
{
    u8 offset = 0;
    u8 page = QSFP_PAGE0;
    int idx = 0;
    int ret = 0;
    bool fail = false;
    u8 ch_monitor[QSFP_LANE_NUM*WORD_SIZE]; /* 2(u16 data) *4 (lane)*/
    u16 divider = 0;
    u16 data_h = 0;
    u16 data_l = 0;
    u16 ch_data = 0;
    char *unit = NULL;
    struct monitor_para_t *para = NULL;
    int len = 0;
    int count = 0;

    if (!buf) {
        return -EINVAL;
    }
    len = sizeof(ch_monitor);
    switch(type) {
    case LN_MONITOR_TX_PWR_TYPE:
        offset = QSFP_LN_TX_PWR_OFFSET;
        break;
    case LN_MONITOR_RX_PWR_TYPE:
        offset = QSFP_LN_RX_PWR_OFFSET;
        break;
    case LN_MONITOR_TX_BIAS_TYPE:
        offset = QSFP_LN_TX_BIAS_OFFSET;
        break;
    default:
        fail = true;
        break;
    }
    if (fail) {
        return -EINVAL;
    }
    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, page, offset, ch_monitor, len)) < 0) {
        return ret;
    }
    para = monitor_para_find(type);
    if (!para) {
        return -EBADRQC;
    }
    divider = para->divider;
    unit = para->unit;

    for (idx = 0; idx < len; idx += WORD_SIZE) {

        data_h = ch_monitor[idx];
        data_l = ch_monitor[idx+1];
        ch_data = (data_h << 8) | data_l;

        count += scnprintf(buf+count, buf_size-count,
                           "ch%d: %d.%d %s\n",
                           (idx >> 1) + 1, ch_data/divider,
                           ch_data%divider, unit);
    }

    return 0;
}
int qsfp_vendor_info_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size)
{
    int ret = 0;
    u8 page = QSFP_PAGE0;
    u8 offset = 0;
    int len = 0;
    bool fail = false;
    u8 byte_data[VENDOR_INFO_BUF_SIZE];

    switch(type) {
    case VENDOR_NAME_TYPE:
        offset = QSFP_VENDOR_NAME_OFFSET;
        len = QSFP_VENDOR_NAME_LEN;
        break;
    case VENDOR_PN_TYPE:
        offset = QSFP_VENDOR_PN_OFFSET;
        len = QSFP_VENDOR_PN_LEN;
        break;
    case VENDOR_SN_TYPE:
        offset = QSFP_VENDOR_SN_OFFSET;
        len = QSFP_VENDOR_SN_LEN;
        break;
    case VENDOR_REV_TYPE:
        offset = QSFP_VENDOR_REV_OFFSET;
        len = QSFP_VENDOR_REV_LEN;
        break;
    default:
        fail = true;
        break;
    }

    if (fail) {
        return -EINVAL;
    }
    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, page, offset, byte_data, len)) < 0) {
        return ret;
    }
    /*add terminal of string*/
    byte_data[len] = '\0';

    scnprintf(buf, buf_size, "%s\n", byte_data);

    return 0;
}

static int qsfp_lpmode_control(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    u8 ext_id = 0;
    u8 power_class = 0;
    u8 lpmode = 0;

    if ((ret = qsfp_ext_id_get(sff_obj, &ext_id)) < 0) {
        return ret;
    }

    /*sff-8436 power class takes up bit 6,7 in  Extended Identifier Values
    *      *
    *           *
    *                *
    *                     * */

    power_class = ext_id >> 6;
    SFF_MGR_DEBUG("port:%d power_class:0x%x\n", sff_obj->port, power_class);

    switch(power_class) {
    case POWER_CLASS_1_MODULE:/*lower power mode*/
        lpmode = 1;
        break;
    case POWER_CLASS_2_MODULE:
    case POWER_CLASS_3_MODULE:
    case POWER_CLASS_4_MODULE:
    default:
        lpmode = 0;
        break;
    }
    if((ret = sff_obj->func_tbl->lpmode_set(sff_obj, lpmode)) < 0) {
        return ret;
    }

    return 0;
}

/*6.2.3  Interrupt Flags Bytes 3-21 consist of interrupt flags for LOS, TX Fault, warnings and alarms.*/
/*TABLE 6-4  LNANNEL STATUS INTERRUPT FLAGS (PAGE 00H BYTES 3-5) */
/*offset 3 , 0-4: rxlos; 4-7 txlos*/
/*offset 4 , 0-4: txfault*/
static int qsfp_int_flag_read(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    int i = 0;
    u8 buf = 0;
    u8 data = 0;
    u8 reg[17];

    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 0x03, &buf, 1)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", sff_obj->port);
        return ret;
    }
    data = bits_get(buf, 0, 4);
    qsfp_lane_status_update(sff_obj, LN_STATUS_RX_LOS_TYPE, data);
    //if (data) {
    SFF_MGR_DEBUG("port:%d rxlos 0x%x",  sff_obj->port, data);
    //}
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 0x04, &buf, 1)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail",  sff_obj->port);
        return ret;
    }
    data = bits_get(buf, 0, 4);

    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 0x05, reg, 17)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail",  sff_obj->port);
        return ret;
    }

    for (i = 0; i < 17; i++) {
        //SFF_MGR_DEBUG("port:%d reg[%d] 0x%x", sff_obj->port, i+5, reg[i]);
    }

    return 0;
}
static int qsfp_cdr_control(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    u8 ext_id = 0;
    u8 cdr_ctrl = 0x00;

    if ((ret = qsfp_ext_id_get(sff_obj, &ext_id)) < 0) {
        return ret;
    }

    if ((ext_id & bit_mask(CDR_RX_PRS_BIT)) &&
            (ext_id & bit_mask(CDR_TX_PRS_BIT))) {

        if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, QSFP_CDR_CONTROL_OFFSET, &cdr_ctrl, 1)) < 0) {
            return ret;
        }
        SFF_MGR_DEBUG("default cdr:0x%x\n", cdr_ctrl);

        if(cdr_ctrl != 0xff) {
            cdr_ctrl = 0xff;
            if((ret = sff_obj->func_tbl->eeprom_write(sff_obj, QSFP_PAGE0, QSFP_CDR_CONTROL_OFFSET, &cdr_ctrl, 1)) < 0) {
                return ret;
            }
        }
        SFF_MGR_DEBUG("OK\n");
    } else {

        SFF_MGR_DEBUG("not supported:ext_id:0x%x\n", ext_id);
    }
    return 0;
}
int qsfp_tx_disable_set(struct sff_obj_t *sff_obj, u8 value)
{
    int ret = 0;
    u8 reg = value;

    if((ret = sff_obj->func_tbl->eeprom_write(sff_obj, QSFP_PAGE0, QSFP_LN_TX_DISABLE_OFFSET, &reg, 1)) < 0) {
        return ret;
    }
    return 0;
}

int qsfp_tx_disable_get(struct sff_obj_t *sff_obj, u8 *value)
{
    int ret = 0;
    u8 reg = 0;
    if (!value) {
        return -EINVAL;
    }
    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, QSFP_LN_TX_DISABLE_OFFSET, &reg, 1)) < 0) {
        return ret;
    }
    *value = reg;
    return 0;

}
int sff_fsm_qsfp_task(struct sff_obj_t *sff_obj)
{
    sff_fsm_state_t st = sff_fsm_st_get(sff_obj);
    int ret = 0;
    bool ready = false;
    u8 resetL = 0;
    bool found = false;
    u8 intL = 0;

    switch (st) {

    case SFF_FSM_ST_IDLE:
        break;
    case SFF_FSM_ST_REMOVED:

        /*always make sure, module plugged in with lower power mode*/
        if((ret = sff_obj->func_tbl->lpmode_set(sff_obj, 1)) < 0) {
            break;
        }
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_IDLE);
        break;
    case SFF_FSM_ST_INSERTED:

        if ((ret = sff_obj->func_tbl->reset_get(sff_obj, &resetL)) < 0) {
            break;
        }
        if (!resetL) {
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_RESET_ASSERTED);
            break;
        }

        ret = _qsfp_data_ready_check(sff_obj, &ready);

        if (ret < 0) {
            break;
        }

        if(ready) {
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTING);
        }

        break;

    case SFF_FSM_ST_DETECTING:
        if ((ret = sff_obj->func_tbl->tx_disable_set(sff_obj, 0xf)) < 0) {

            break;
        }
        if((ret = qsfp_type_identify(sff_obj, &found)) < 0) {
            break;
        }

        if(found) {
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_INIT);
        } else {
            /*unknown type: special handling*/
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_UNKNOWN_TYPE);
        }

        break;

    case SFF_FSM_ST_ISOLATED:
        break;
    case SFF_FSM_ST_DETECTED:
#if defined (QSFP_INT_FLAG_SUPPORT)
        if ((ret = sff_obj->func_tbl->intL_get(sff_obj, &intL)) < 0) {
            break;
        }
        if (!intL) {
            if ((ret = qsfp_int_flag_read(sff_obj)) < 0) {
                break;
            }
        }
#endif
        break;

    case SFF_FSM_ST_INIT:
        /*cdr control*/
        if((ret = qsfp_cdr_control(sff_obj)) < 0) {
            break;
        }

        /*low power mode control*/
        if((ret = qsfp_lpmode_control(sff_obj)) < 0) {
            break;

        }
        /*tx_disable control*/

        if((ret = sff_obj->func_tbl->tx_disable_set(sff_obj, 0x0)) < 0) {
            break;
        }
        sff_fsm_kobj_change_event(sff_obj);
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTED);
        break;

    case SFF_FSM_ST_RESET_ASSERTED:
        break;

    case SFF_FSM_ST_RESET_DEASSERTED:
        break;

    case SFF_FSM_ST_FAULT:/*i2c bus fail ... etc*/
        break;

    case SFF_FSM_ST_UNKNOWN_TYPE:
        break;
    default:
        SFF_MGR_ERR("unknown fsm st:%d\n", st);
        break;

    }
    if (ret < 0) {
        return ret;
    }
    return 0;
#if 0
    if(ret < 0) {
        if (!ioexp_is_lane_ready()) {
            SFF_MGR_ERR("i2c_crush port:%d\n", sff_obj->port);
            return (-1);
        }
    }
#endif
    return 0;
}

