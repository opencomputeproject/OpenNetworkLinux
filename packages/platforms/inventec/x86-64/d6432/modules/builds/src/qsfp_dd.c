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
#include "qsfp_dd.h"

/*page 0x00 register*/
#define DD_MODULE_GLOBAL_CONTROL_OFFSET  (26)
#define DD_MODULE_GLOBAL_CONTROL_LOW_PWR_BIT (6)
#define DD_CMIS_REV_OFFSET (1)

/*page 0x10 register*/
#define DD_DATA_PATH_DEINIT_OFFSET (128)

#define DD_CMIS_REV4_VAL (0x40)
#define DD_CMIS_REV3_VAL (0x30)
#define DD_VCC_OFFSET (16)
#define DD_TEMP_OFFSET (14)

#define DD_LN_MONITOR_TX_PWR_OFFSET (154)
#define DD_LN_MONITOR_RX_PWR_OFFSET (186)
#define DD_LN_MONITOR_TX_BIAS_OFFSET (170)

extern bool int_flag_monitor_en;
/*qsfp-dd*/
const u8 app_advert_fields_offset[APSEL_NUM][APP_ADVERT_FIELD_NUM] = {
    { 86, 87, 88, 89, 176},
    { 90, 91, 92, 93, 177},
};
u8 app_advert_fields_page[APP_ADVERT_FIELD_NUM] = {
    QSFP_PAGE0,
    QSFP_PAGE0,
    QSFP_PAGE0,
    QSFP_PAGE0,
    QSFP_PAGE_01h,
};

static char *module_flag_str[] = {

    "FW_FAULT",
    "L_VCC_TEMP_WARN",
    "L_AUX_ALARM_WARN",
    "VENDOR_DEFINED_ERR",
};

static char *lane_tx_flag_str[] = {

    "L_TX_FAULT",
    "L_TX_LOS",
    "L_TX_CDR_LOL",
    "L_TX_APAP_EQ_INPUT_FAULT",
    "L_TX_POWER_HIGH_ALARM",
    "L_TX_POWER_LOW_ALARM",
    "L_TX_POWER_HIGH_WARN",
    "L_TX_POWER_LOW_WARN",
    "L_TX_BIAS_HIGH_ALARM",
    "L_TX_BIAS_LOW_ALARM",
    "L_TX_BIAS_HIGH_WARN",
    "L_TX_BIAS_LOW_WARN",

};
static char *lane_rx_flag_str[] = {

    "L_RX_LOS",
    "L_RX_CDR_LOL",
    "L_RX_POWER_HIGH_ALARM",
    "L_RX_POWER_LOW_ALARM",
    "L_RX_POWER_HIGH_WARN",
    "L_RX_POWER_LOW_WARN",
};
#if 0
static int app_advertising_field_get(struct sff_obj_t *sff_obj);
static int qsfp_dd_active_control_set_indicator(struct sff_obj_t *sff_obj);
#endif
static int data_path_power_up(struct sff_obj_t *sff_obj, bool up);
static int qsfp_dd_is_data_path_activated(struct sff_obj_t *sff_obj, bool *activated);
static int qsfp_dd_lowPwr_set(struct sff_obj_t *sff_obj);
static int stage_control_set0(struct sff_obj_t *sff_obj);
static int config_error_code_check(struct sff_obj_t *sff_obj, bool *is_pass);
static int advert_check(struct sff_obj_t *sff_obj, bool *pass);
static int module_ready_check_rev3(struct sff_obj_t *sff_obj, bool *ready);
static int module_ready_check_rev4(struct sff_obj_t *sff_obj, bool *ready);

static int sw_config_rev3(struct sff_obj_t *sff_obj);
static int sw_config_check_rev3(struct sff_obj_t *sff_obj, bool *pass);
static int sw_control_rev3(struct sff_obj_t *sff_obj);
static int sw_config_rev4(struct sff_obj_t *sff_obj);
static int sw_config_check_rev4(struct sff_obj_t *sff_obj, bool *pass);
static int sw_control_rev4(struct sff_obj_t *sff_obj);


struct qsfp_dd_fsm_func_t qsfp_dd_FsmFuncRev3 = {

    .advert_check = advert_check,
    .sw_config = sw_config_rev3,
    .sw_config_check = sw_config_check_rev3,
    .sw_control = sw_control_rev3,
    .module_ready_check = module_ready_check_rev3
};
struct qsfp_dd_fsm_func_t qsfp_dd_FsmFuncRev4 = {

    .advert_check = advert_check,
    .sw_config = sw_config_rev4,
    .sw_config_check = sw_config_check_rev4,
    .sw_control = sw_control_rev4,
    .module_ready_check = module_ready_check_rev4
};
static int advert_check(struct sff_obj_t *sff_obj, bool *pass)
{
#if 0
    if ((ret = qsfp_dd_active_control_set_indicator(sff_obj)) < 0) {
        return ret;
    }
    if ((ret = app_advertising_field_get(sff_obj)) < 0) {
        /*print error code*/
        return ret;
    }
#endif
    /*<TBD> do we need to check advertise?*/
    *pass = true;
    return 0;
}
static int sw_config_rev3(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    /* host select application*/
    /*stage control set 0*/
    if ((ret = stage_control_set0(sff_obj)) < 0) {
        /*print error code*/
        return ret;
    }
    return 0;
}
static int sw_config_check_rev3(struct sff_obj_t *sff_obj, bool *pass)
{
    int ret = 0;
    bool is_pass = false;
    if ((ret = config_error_code_check(sff_obj, &is_pass)) < 0) {

        return ret;
    }
    *pass = is_pass;
    return 0;
}
static int sw_control_rev3(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    /*set tx_disable true to avoid link flapping*/
    if ((ret = qsfp_dd_tx_disable_set(sff_obj, 0xff)) < 0) {
        return ret;
    }
    /*Host sets the
     *         * DataPathPwrUp bits for
     *                 * all host lanes in the
     *                         * data path(s) that it
     *                                 * wants to enable (See
     *                                         * Table 53, Page 10h
     *                                                 * Register 128)*/

    if ((ret = data_path_power_up(sff_obj, true)) < 0) {
        return ret;
    }

    return 0;
}
static int module_ready_check_rev3(struct sff_obj_t *sff_obj, bool *ready)
{
    u8 module_st = 0;
    int ret = 0;
    bool tmp_ready = false;
    bool activated = false;
    if ((ret = qsfp_dd_module_st_get(sff_obj, &module_st)) < 0) {
        return ret;
    }
    if ((ret = qsfp_dd_is_data_path_activated(sff_obj, &activated)) < 0) {
        return ret;
    }
    /*check module state register if it transit to module ready*/
    if (MODULE_READY_ST_ENCODE == module_st &&
            true == activated ) {
        /*set tx_disable false, */
        if ((ret = qsfp_dd_tx_disable_set(sff_obj, 0x0)) < 0) {
            return ret;
        }
        tmp_ready = true;
    }

    *ready = tmp_ready;
    return 0;
}
static int module_ready_check_rev4(struct sff_obj_t *sff_obj, bool *ready)
{
    u8 module_st = 0;
    int ret = 0;
    bool tmp_ready = false;
    bool activated = false;

    if ((ret = qsfp_dd_module_st_get(sff_obj, &module_st)) < 0) {
        return ret;
    }
    if ((ret = qsfp_dd_is_data_path_activated(sff_obj, &activated)) < 0) {
        return ret;
    }
    /*check module state register if it transit to module ready*/
    if (MODULE_READY_ST_ENCODE == module_st &&
            true == activated ) {
        tmp_ready = true;
    }

    *ready = tmp_ready;
    return 0;
}
static int sw_config_rev4(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    if ((ret = qsfp_dd_lowPwr_set(sff_obj)) < 0) {
        return ret;
    }
    return 0;
}
static int sw_config_check_rev4(struct sff_obj_t *sff_obj, bool *pass)
{
    *pass = true;
    return 0;
}
static int sw_control_rev4(struct sff_obj_t *sff_obj)
{

    return 0;
}
static int cmis_ver_get(struct sff_obj_t *sff_obj, u8 *ver)
{
    int ret = 0;
    u8 buf = 0;

    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, DD_CMIS_REV_OFFSET, &buf, 1)) < 0) {
        return ret;
    }
    SFF_MGR_DEBUG("port:%d cmis ver:0x%x\n", sff_obj->port, buf);
    *ver = buf;
    return 0;
}
static int fsm_func_get(struct sff_obj_t *sff_obj, struct qsfp_dd_fsm_func_t **ppfunc)
{
    int ret = 0;
    u8 ver = 0;
    struct qsfp_dd_fsm_func_t *func = &qsfp_dd_FsmFuncRev3;
    if ((ret = cmis_ver_get(sff_obj, &ver)) < 0) {
        return ret;
    }

    if (ver == DD_CMIS_REV3_VAL) {

        func = &qsfp_dd_FsmFuncRev3;

    } else if (ver == DD_CMIS_REV4_VAL) {

        func = &qsfp_dd_FsmFuncRev4;
    } else {

        SFF_MGR_ERR("port:%d rev:%x not supported, using rev 3.0 as default", sff_obj->port, ver);
    }
    *ppfunc = func;
    return 0;
}
/*TBD lan number can be changed*/
static int valid_lane_num_get(struct sff_obj_t *sff_obj)
{

    int num = 8;
    return num;

}
static bool is_bank_num_valid(struct sff_obj_t *sff_obj)
{
    u8 bank_no = 0xff;
    bool valid = false;
    /*check if it's bank0? */
    if (sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 126, &bank_no, 1) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", sff_obj->port);
        return valid;
    }
    if (0x00 == bank_no) {

        valid = true;
    } else {
        SFF_MGR_ERR("port:%d bank no is not 0", sff_obj->port);
    }

    return valid;
}
/*CMIS REV 4.0*/
static int qsfp_dd_lowPwr_set(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    u8 buf = 0;
    u8 page = QSFP_PAGE0;
    u8 offset = DD_MODULE_GLOBAL_CONTROL_OFFSET;

    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, page, offset, &buf, 1)) < 0) {

        return ret;
    }

    clear_bit(DD_MODULE_GLOBAL_CONTROL_LOW_PWR_BIT, (unsigned long *)&buf);

    if( (ret = sff_obj->func_tbl->eeprom_write(sff_obj, page, offset, &buf, 1)) < 0) {
        return ret;
    }

    return 0;
}
#if 0
/*Table 8-44  Page 10h Overview offset 128*/
static int qsfp_dd_data_path_deinit(struct sff_obj_t *sff_obj, u8 val)
{
    int ret = 0;
    u8 buf = val;
    u8 page = QSFP_PAGE_10h;
    u8 offset = DD_DATA_PATH_DEINIT_OFFSET;

    if( (ret = sff_obj->func_tbl->eeprom_write(sff_obj, page, offset, &buf, 1)) < 0) {
        return ret;
    }

    return 0;
}
/*Table 23- Module Global and Squelch Mode Controls (Lower Page, active modules only)
 *  * byte 26 bit 4 */
static int qsfp_dd_force_low_pwr(struct sff_obj_t *sff_obj, int en)
{
    u8 buf = 0;
    int ret = 0;
    u8 page = QSFP_PAGE0;
    u8 offset = 26;
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, page, offset, &buf, 1)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", port);
        return ret;
    }
    buf = ret;
    if (en) {
        set_bit(4, (unsigned long *)&buf);
    } else {

        clear_bit(4, (unsigned long *)&buf);
    }

    if( (ret = sff_obj->func_tbl->eeprom_write(sff_obj, page, offset, &buf, 1)) < 0) {
        SFF_MGR_ERR("write fail\n");
        return ret;
    }
    return 0;
}
#endif
#if 0
/*(Page 11h, active modules only)
 *  * app selected: offset 206-213 (Table 73)
 *   * tx control offset 214-221 (Table 74)
 *    * rx control offset 222-239 (Table 75)*/
static int qsfp_dd_active_control_set_indicator(struct sff_obj_t *sff_obj)
{
    u8 data[34];
    int ret = 0;
    int count = 0;
    int size = sizeof(data);
    if (!is_bank_num_valid(sff_obj)) {
        return -1;
    }
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE_11h, 206, data, size)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", sff_obj->port);
        return ret;
    }

    for (count = 0; count < size; count++) {

    }

    return 0;
}
/*Table 46- Implemented Signal Integrity Controls (Page 01h)
 *  *
 *   * */
int qsfp_dd_channel_control_get(struct sff_obj_t *sff_obj, int type, u32 *value)
{
    int ret = 0;
    u8 data[2];
    int port = sff_obj->port;
    if (!is_bank_num_valid(sff_obj)) {
        return -1;
    }
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE_01h, 161, data, 2)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", port);
        return -1;
    }

    /*offset:161*/
    if (data[0] | TX_CDR_BIT) {

        SFF_MGR_DEBUG("port:%d Tx CDR implemented", port);
    }
    if (data[0] | TX_CDR_BYPASS_BIT) {

        SFF_MGR_DEBUG("port:%d Tx CDR Bypass control implemented ", port);
    }
    if (data[0] | TX_INPUT_FIX_MANUAL_BIT) {

        SFF_MGR_DEBUG("port:%d Tx Input Eq fixed manual", port);
    }
    if (data[0] | ADAPTIVE_TX_INPUT_FIX_MANUAL_BIT) {

        SFF_MGR_DEBUG("port:%d Adaptive Tx Input Eq", port);
    }
    if (data[0] | TX_INPUT_FREEZE_BIT) {

        SFF_MGR_DEBUG("port:%d Tx Input Eq Freeze", port);
    }
    /*offset:162*/
    if (data[1] | RX_CDR_BIT) {

        SFF_MGR_DEBUG("port:%d Rx CDR implemented", port);
    }
    if (data[1] | RX_CDR_BYPASS_BIT) {

        SFF_MGR_DEBUG("port:%d Rx CDR Bypass control implemented ", port);
    }
    if (data[1] | RX_OUTPUT_AMP_CONTROL_BIT) {

        SFF_MGR_DEBUG("port:%d Rx output amp", port);
    }
    if (data[1] | RX_OUTPUT_EQ_CONTROL_BIT) {

        SFF_MGR_DEBUG("port:%d Rx output eq", port);
    }
    if (data[1] | STAGE_SET1_BIT) {

        SFF_MGR_DEBUG("port:%d stage_set1", port);
    }

    return 0;
}
#endif
int qsfp_dd_temperature_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    int ret = 0;
    u8 page = QSFP_PAGE0;
    u8 offset = 14;
    u8 reg[WORD_SIZE];
    s16 temp = 0;
    s16 data_h = 0;
    s16 data_l = 0;
    s16 divider = 256;
    s16 decimal = 0;
    char *unit = "c";

    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, page, offset, reg, sizeof(reg))) < 0) {
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

/*Table 22- Module Monitors (Lower Page, active modules only)
 *  * offset 16,17*/
int qsfp_dd_voltage_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    int ret = 0;
    u8 page = QSFP_PAGE0;
    u8 offset = 16;
    u8 reg[WORD_SIZE];
    u16 vol = 0;
    u16 data_h = 0;
    u16 data_l = 0;
    u16 divider = 10000;
    char *unit = "v";

    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, page, offset, reg, sizeof(reg))) < 0) {
        return ret;
    }
    data_h = reg[0];
    data_l = reg[1];
    vol = (data_h << 8) | data_l;
    scnprintf(buf, buf_size,
              "%d.%d %s\n",
              vol/divider,
              vol%divider, unit);

    return 0;
}

/*1.7.7.3  Lane-Specific Monitors */
/*Table 70- Lane-Specific Monitors (Page 11h, active modules only) */
int qsfp_dd_lane_monitor_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size)
{
    int ret = 0;
    u8 page = QSFP_PAGE_11h;
    u8 offset = 0;
    int len = 0;
    int count = 0;
    int idx = 0;
    u8 ch_monitor[QSFP_DD_LANE_NUM*WORD_SIZE]; /* 2(u16 data) *4 (channel)*/
    u16 data_h = 0;
    u16 data_l = 0;
    u16 ch_data = 0;
    u16 divider = 10000;
    char *unit = NULL;
    struct monitor_para_t *para = NULL;

    len = sizeof(ch_monitor);

    switch(type) {
    case LN_MONITOR_TX_PWR_TYPE:
        offset = 154;
        break;
    case LN_MONITOR_RX_PWR_TYPE:
        offset = 186;
        break;
    case LN_MONITOR_TX_BIAS_TYPE:
        offset = 170;
        break;
    default:
        break;
    }
    if (0 == offset) {
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
        /*big edian*/
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
/*rev3 1.7.3.2*/
int qsfp_dd_vendor_info_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size)
{
    int ret = 0;
    u8 page = QSFP_PAGE0;
    u8 offset = 0;
    int len = 0;
    u8 byte_data[VENDOR_INFO_BUF_SIZE];

    switch(type) {
    case VENDOR_NAME_TYPE:
        offset = 129;
        len = 16;

        break;
    case VENDOR_PN_TYPE:
        offset = 148;
        len = 16;

        break;
    case VENDOR_SN_TYPE:
        offset = 166;
        len = 16;

        break;
    case VENDOR_REV_TYPE:
        offset = 164;
        len = 2;

        break;
    default:
        break;
    }
    if (0 == offset) {
        return -EINVAL;
    }
    /*use a big enough buf to handle all the vendor info*/
    if (len >= VENDOR_INFO_BUF_SIZE) {
        return -EBADRQC;
    }

    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, page, offset, byte_data, len)) < 0) {
        return ret;
    }
    /*add terminal of string*/
    byte_data[len] = '\0';
    scnprintf(buf, buf_size, "%s\n", byte_data);

    return 0;
}
/*Table 18- Identifier and Status Summary (Lower Page) byte3 , 3-1 Module state*/
int qsfp_dd_module_st_get(struct sff_obj_t *sff_obj, u8 *st)
{
    u8 data = 0;
    int ret = 0;
    if (!st) {
        return -EINVAL;
    }
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 0x03, &data, 1)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", sff_obj->port);
        return ret;
    }

    *st = masked_bits_get(data, 1, 3);
    return 0;
}
/*value : the status of 8 lane, one lane: 4 bits*/
/*table 65 page 11h offset 128~131*/
static int qsfp_dd_data_path_st_get(struct sff_obj_t *sff_obj, u32 *st)
{
    int ret = 0;
    u32 data = 0;

    if (!st) {
        return -EINVAL;
    }

    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE_11h, 128, (u8*)&data, sizeof(data))) < 0) {
        *st = 0;
        return ret;
    }
    *st = data;

    return 0;
}
static int qsfp_dd_is_data_path_activated(struct sff_obj_t *sff_obj, bool *activated)
{
    u32 data_path_st = 0;
    u32 st_lane = 0;
    int ln = 0;
    int ret = 0;
    bool tmp_active = false;
    int lane_num = valid_lane_num_get(sff_obj);

    if ((ret = qsfp_dd_data_path_st_get(sff_obj, &data_path_st)) < 0) {
        return ret;
    }
    /*TBD check 8 lane as default*/
    for (ln = 0; ln < lane_num; ln++) {
        st_lane = data_path_st & 0x0f;
        if (DATA_PATH_ACTIVATED_ST_ENCODE != st_lane) {
            break;
        }
        st_lane = st_lane >> 4;
    }
    if (ln >= lane_num) {
        SFF_MGR_DEBUG("port:%d ok", sff_obj->port);
        tmp_active = true;
    }
    *activated = tmp_active;
    return 0;
}
/*Table 18- Identifier and Status Summary (Lower Page)
 *  * lower page :offset 0　*/
int qsfp_dd_id_get(struct sff_obj_t *sff_obj, u8 *val)
{
    u8 buf = 0;
    int ret = 0;

    if (!val) {
        return -EINVAL;
    }
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 0, &buf, 1)) < 0) {
        return ret;
    }
    *val = buf;
    return 0;
}
#if 0
static int qsfp_dd_module_state_is_changed(struct sff_obj_t *sff_obj, bool *is_changed)
{
    u8 page = QSFP_PAGE0;
    u8 offset = 8;
    u8 val = 0;
    int ret = 0;
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, page, offset, &val, 1)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", sff_obj->port);
        return ret;
    }
    *is_changed = ((val&0x1) ? true : false);
    return 0;
}

/*table 18 : offset 3 bit 0*/
static int qsfp_dd_digital_int_get(struct sff_obj_t *sff_obj, u8 *value)
{
    u8 st = 0;
    int ret = 0;
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 0x03, &st, 1)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", sff_obj->port);
        return ret;
    }
    *value = st & 0x1;
    return ret;

}
#endif
#if 0
/*table 36: byte 212 :that defines aspects of the device or cable technology*/
static int qsfp_dd_media_interface_tech_get(struct sff_obj_t *sff_obj, u8 *interface)
{

    u8 data = 0;

    if(IS_ERR_OR_NULL(interface)) {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 212, &data, 1) < 0) {
        SFF_MGR_ERR("sff_obj->func_tbl->eeprom_read fail");
        *interface = 0xff; /*non define*/
        return -1;
    }
    *interface = data;

    return 0;

}
#endif

/*Table 25- Byte 85 Module Type Encodings*/
static int qsfp_dd_module_type_get(struct sff_obj_t *sff_obj, u8 *type)
{
    u8 data = 0;
    int ret = 0;

    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 85, &data, 1)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", sff_obj->port);
        return ret;
    }
    *type = data;
    return 0;
}
static bool is_loopback_module(struct sff_obj_t *sff_obj)
{
    /*<TBD> need check more loopback modules*/
    bool is_loopback = false;
    u8 type = 0;
    if (qsfp_dd_module_type_get(sff_obj, &type) < 0) {
        return false;
    }

    if (0 == type) {
        is_loopback = true;
    }
    return is_loopback;
}
#if 0
/*table 26*: lower page:00*/
static int app_advertising_field_get(struct sff_obj_t *sff_obj)
{
    u8 module_type = 0;
    union qsfp_dd_app_advert_fields *fields = sff_obj->priv_data.qsfp_dd.fields;
    int ret = 0;
    int apsel = 0;
    int i = 0;
    bool end_of_table = false;
    int port = sff_obj->port;

    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 85, &module_type, 1)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", port);
        return ret;
    }

    SFF_MGR_DEBUG("port:%d module_type:0x%x\n", port, module_type);
    sff_obj->priv_data.qsfp_dd.module_type = module_type;
    for (apsel = APSEL_1; apsel < APSEL_NUM; apsel++) {
        for (i = 0; i < APP_ADVERT_FIELD_NUM; i++) {

            if ((ret =sff_obj->func_tbl->eeprom_read(sff_obj, app_advert_fields_page[i],
                      app_advert_fields_offset[apsel][i],
                      &fields[apsel].reg[i], 1)) < 0) {

                SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", port);
                return ret;
            }
            if (ret == 0xff) {
                end_of_table = true;
                break;
            }
        }
        if (end_of_table) {
            break;
        }

    }

    for (apsel = 0; apsel < APSEL_NUM; apsel++) {
        SFF_MGR_DEBUG("port:%d host_electrical_interface_code[%d]:0x%x\n",
                      port, apsel, fields[apsel].data.host_electrical_interface_code);
        SFF_MGR_DEBUG("port:%d module_media_interface_code[%d]:0x%x\n",
                      port, apsel, fields[apsel].data.module_media_interface_code);
        SFF_MGR_DEBUG("port:%d lane_count[%d]:0x%x\n",
                      port, apsel, fields[apsel].data.lane_count);
        SFF_MGR_DEBUG("port:%d  host_lane_assignment_options[%d]:0x%x\n",
                      port, apsel, fields[apsel].data.host_lane_assignment_options);
    }
    return 0;
}
#endif
#if 0
static int app_advert_field_dump(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    int apsel = APSEL_1;
    int ret = 0;
    union qsfp_dd_app_advert_fields *fields = sff_obj->priv_data.qsfp_dd.fields;
    /*<TBD> the field data will be cached when module is plugged in , so may not need to capture again in the future*/
    ret = app_advertising_field_get(sff_obj);
    if (ret < 0) {
        return ret;
    }
    scnprintf(buf, buf_size,
              "module_type 0x%x \
                                                                 host electrical intf 0x%x \
                                                                                module media intf 0x%x  \
                                                                                               host_media lane count %d \
                                                                                                              host lane assignment 0x%x \
                                                                                                                             media lane assignment 0x%x\n",
              sff_obj->priv_data.qsfp_dd.module_type,
              fields[apsel].data.host_electrical_interface_code,
              fields[apsel].data.module_media_interface_code,
              fields[apsel].data.lane_count,
              fields[apsel].data.host_lane_assignment_options,
              fields[apsel].data.media_lane_assignment_options

             );
    return 0;

}
#endif
/*note: so far only page >= 0x10 , need to check bank number*/
static int stage_control_set0(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    u8 apply_stage_control_set = 0;
    u8 stage_set_offset_begin = 145;
    u8 stage_set_offset_end = stage_set_offset_begin + valid_lane_num_get(sff_obj) - 1;
    u8 offset = 0;
    struct stage_set_t stage_set;
    if (!is_bank_num_valid(sff_obj)) {
        return -EBADRQC;
    }
    /*table 56 stage control set 0, application select controls (Page 10h, active modules only) */
    /*case1: normal case 400G lane = 8*/
    stage_set.explicit_control = 0;
    stage_set.datapath_code = 0x00;
    stage_set.app_code = 0x01;

    for (offset = stage_set_offset_begin; offset <= stage_set_offset_end; offset++) {
        if((ret = sff_obj->func_tbl->eeprom_write(sff_obj, QSFP_PAGE_10h, offset, (u8*)(&stage_set),1)) < 0) {
            break;
        }
    }
    if (ret < 0) {
        return ret;
    }
    /*table 55 Apply_DataPathInit*/
    apply_stage_control_set = 0xff;
    if((ret = sff_obj->func_tbl->eeprom_write(sff_obj, QSFP_PAGE_10h, 143, &apply_stage_control_set, 1)) < 0) {
        return ret;
    }
    return 0;
}
/*qsfp-dd used only*/

/*page 11
 *  * table 71/72 : active modules only
 *   * Table 72- Configuration Error Codes
 *    * para:
 *     * port(in)
 *      * is_pass(out)*/
static int config_error_code_check(struct sff_obj_t *sff_obj, bool *is_pass)
{
    int ret = 0;
    int port = sff_obj->port;
    u8 data[4];
    int lane = 0;
    int i = 0;
    int size = sizeof(data);
    /*since the one reg reprsL 2 lanes*/
    int lane_num = valid_lane_num_get(sff_obj);
    u8 config_err_code[lane_num]; /*lane number 8*/

    if (!is_pass) {
        return -EINVAL;
    }
    if (!is_bank_num_valid(sff_obj)) {
        return -EBADRQC;
    }
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE_11h, 202, data, size)) < 0) {
        return ret;
    }

    for (lane = 0, i = 0; i < size; i++) {
        config_err_code[lane++] = data[i] & 0xf;
        config_err_code[lane++] = (data[i] & 0xf0) >> 4;
    }

    for (lane = 0; lane < lane_num; lane++) {

        SFF_MGR_DEBUG("port:%d lane:%d err_code:0x%x", port, lane, config_err_code[lane]);
        if (CONFIG_ACCEPTED == config_err_code[lane]) {
        } else {
            break;
        }
    }
    if (lane >= lane_num) {
        *is_pass = true;
    }
    return 0;
}
/*table 54 page 10h offset 130 */
int qsfp_dd_tx_disable_set(struct sff_obj_t *sff_obj, u8 val)
{
    int ret = 0;
    if (!is_bank_num_valid(sff_obj)) {
        return -EBADRQC;
    }
    if((ret = sff_obj->func_tbl->eeprom_write(sff_obj, QSFP_PAGE_10h, 130, &val, 1)) < 0) {
        return ret;
    }

    return 0;
}
/*table 54 page 10h offset 130 */
int qsfp_dd_tx_disable_get(struct sff_obj_t *sff_obj, u8 *val)
{
    int ret = 0;
    u8 buf = 0;
    if (!val) {
        return -EINVAL;
    }
    if (!is_bank_num_valid(sff_obj)) {
        return -EBADRQC;
    }
    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE_10h, 130, &buf, 1)) < 0) {
        return -ret;
    }
    *val = buf;
    return 0;
}
/*table 53 */
static int data_path_power_up(struct sff_obj_t *sff_obj, bool up)
{
    int ret = 0;
    u8 value = 0;

    if (up) {
        value = 0xff;
    }
    if (!is_bank_num_valid(sff_obj)) {
        return -EBADRQC;
    }
    if((ret = sff_obj->func_tbl->eeprom_write(sff_obj, QSFP_PAGE_10h, 128, &value, 1)) < 0) {
        return ret;
    }

    return 0;
}
#if 0
/*Table 18- Identifier and Status Summary (Lower Page) offset 2 bit 7:Flat_mem*/
static int qsfp_dd_is_flat_mem(struct sff_obj_t *sff_obj, bool *is_flat)
{
    u8 buf = 0;
    u8 offset = 2;

    if(IS_ERR_OR_NULL(is_flat)) {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    if (sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, offset, &buf, 1) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", sff_obj->port);
        *is_flat = false;
        return -1;
    }

    if (buf & 0x80) {
        *is_flat = true;
    } else {
        *is_flat = false;
    }
    return 0;
}
#endif
/*optical: true copper: false*/
static bool is_passive_module(struct sff_obj_t *sff_obj, bool *passive_module)
{
    int ret = 0;
    bool tmp_passive_module = false;
    u8 module_type = 0;

    if ((ret = qsfp_dd_module_type_get(sff_obj, &module_type)) < 0) {
        return ret;
    }

    switch (module_type) {

    case MODULE_PASSIVE_CU:
        tmp_passive_module = true;
        break;
    case MODULE_TYPE_MMF:
    case MODULE_TYPE_SMF:
    case MODULE_ACTIVE_CABLES:
    case MODULE_BASE_T:
    default:
        break;

    }
    *passive_module = tmp_passive_module;
    return 0;
}
#if 0
/*<TBD> so far it's used to de-assert- intL*/
static int qsfp_dd_interrupt_flags_read(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    u8 module_flags[4];
    u8 lane_flags[19];
    /*table 15 module flags*/
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 8, module_flags, 4)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", port);
        return ret;
    }
    /* table 16 Lane-Specific Flag*/
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE_11h, 134, lane_flags, 19)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", port);
        return ret;
    }

    return 0;
}
#endif

/*table 15*/
static int qsfp_dd_module_flag_monitor(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    int i = 0;
    u8 module_flag[4];
    int port = sff_obj->port;
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE0, 8, module_flag, 4)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", port);
        return ret;
    }
    for (i = 0; i < 4; i++) {

        if (module_flag[i] != 0x00) {

            SFF_MGR_ERR("port:%d error %s[%d]:0x%x", port, module_flag_str[i], i+8, module_flag[i]);

        }
    }

    return 0;
}
/*Table 68 TX Flags , 69- RX Flags (Page 11h, active modules only)*/
static int qsfp_dd_lane_flag_monitor(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    int i = 0;
    u8 rx_flag[6];
    u8 tx_flag[12];
    u8 data_path_st_change = 0;
    int port = sff_obj->port;
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE_11h, 134, &data_path_st_change, 1)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", port);
        return ret;
    }
    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE_11h, 135, tx_flag, 12)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", port);
        return ret;
    }

    if ((ret = sff_obj->func_tbl->eeprom_read(sff_obj, QSFP_PAGE_11h, 147, rx_flag, 6)) < 0) {

        SFF_MGR_ERR("port:%d sff_obj->func_tbl->eeprom_read fail", port);
        return ret;
    }
    SFF_MGR_DEBUG("port:%d  data path st change:0x%x\n", port, data_path_st_change);
    for (i = 0; i < 12; i++) {

        if (tx_flag[i] != 0x00) {

            SFF_MGR_ERR("port:%d error %s[%d]:0x%x", port, lane_tx_flag_str[i], i+135, tx_flag[i]);

        }
    }
    for (i = 0; i < 6; i++) {

        if (rx_flag[i] != 0x00) {

            SFF_MGR_ERR("port:%d error %s[%d]:0x%x", port, lane_rx_flag_str[i], i+147, rx_flag[i]);

        }
    }

    return 0;
}
int sff_fsm_qsfp_dd_task(struct sff_obj_t *sff_obj)
{
    sff_fsm_state_t st = sff_fsm_st_get(sff_obj);
    int ret = 0;
    u8 value = 0;
    u8 lpmode = 0;
    u8 resetL = 0;
    u8 module_st = 0;
    u8 intL = 0;
    bool pass = false;
    bool ready = false;
    bool passive_module = false;

    switch (st) {
    case SFF_FSM_ST_IDLE:
        break;
    case SFF_FSM_ST_REMOVED:
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_IDLE);
        break;
    case SFF_FSM_ST_INSERTED:
        if ((ret = sff_obj->func_tbl->reset_get(sff_obj, &resetL)) < 0) {
            break;
        }
        if (resetL) {
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_MGMT_INIT);
        } else {

            sff_fsm_st_set(sff_obj, SFF_FSM_ST_RESET_ASSERTED);
        }

        break;
    case SFF_FSM_ST_MGMT_INIT:
        /*check module state register if it transit to module low power state*/
        if ((ret = qsfp_dd_module_st_get(sff_obj, &value)) < 0) {

            break;
        }

        if ((ret = is_passive_module(sff_obj, &passive_module)) < 0) {
            break;
        }

        if (passive_module) {
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTED);
        } else {

            if ((ret = sff_obj->func_tbl->lpmode_get(sff_obj, &lpmode)) < 0) {
                break;
            }
            if (lpmode) {

                if (MODULE_LOW_PWR_ST_ENCODE == value) {
                    if (is_loopback_module(sff_obj)) {

                        sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_LOOPBACK_INIT);
                    } else {

                        sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_CMIS_VER_CHECK);
                    }
                }
            } else {
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_HW_INIT);
            }
        }

        break;
    case SFF_FSM_ST_MODULE_CMIS_VER_CHECK:

        if ((ret = fsm_func_get(sff_obj, &(sff_obj->dd_fsm_func))) < 0) {
            break;
        }

        sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_ADVERT_CHECK);
        break;
    case SFF_FSM_ST_MODULE_ADVERT_CHECK:

        if ((ret = sff_obj->dd_fsm_func->advert_check(sff_obj, &pass)) < 0) {
            break;
        }
        if (pass) {
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_SW_CONFIG);
        }
        break;
    case SFF_FSM_ST_MODULE_SW_CONFIG:

        if ((ret = sff_obj->dd_fsm_func->sw_config(sff_obj)) < 0) {
            break;
        }
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_SW_CONFIG_CHECK);
        break;
    case SFF_FSM_ST_MODULE_SW_CONFIG_CHECK:

        if ((ret = sff_obj->dd_fsm_func->sw_config_check(sff_obj, &pass)) < 0) {
            break;
        }

        if (pass) {

            sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_SW_CONTROL);
        }
        break;
    case SFF_FSM_ST_MODULE_SW_CONTROL:

        if ((ret = sff_obj->dd_fsm_func->sw_control(sff_obj)) < 0) {
            break;
        }

        sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_READY_CHECK);
        break;
    case SFF_FSM_ST_MODULE_READY_CHECK:

        if ((ret = sff_obj->dd_fsm_func->module_ready_check(sff_obj, &ready)) < 0) {
            break;
        }
        if (ready) {
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_READY);
        }
        break;
    case SFF_FSM_ST_MODULE_READY:

        sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTED);
        break;

    case SFF_FSM_ST_MODULE_HW_INIT:
    case SFF_FSM_ST_MODULE_LOOPBACK_INIT:

        if ((ret = qsfp_dd_module_st_get(sff_obj, &module_st)) < 0) {
            break;
        }
        /*check module state register if it transit to module ready*/
        if (MODULE_READY_ST_ENCODE == module_st) {
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTED);
        }
        break;
    case SFF_FSM_ST_DETECTED:
        if (int_flag_monitor_en) {

            if ((ret = is_passive_module(sff_obj, &passive_module)) < 0) {
                break;
            }
            if (!passive_module) {
                if ((ret = sff_obj->func_tbl->intL_get(sff_obj, &intL)) < 0) {
                    break;
                }
                if (!intL) {
                    if ((ret = qsfp_dd_module_flag_monitor(sff_obj)) < 0) {
                        break;
                    }
                    if ((ret = qsfp_dd_lane_flag_monitor(sff_obj)) < 0 ) {
                        break;
                    }
                }
            }
        }
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
}

