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
#include "sfp.h"
#include "inv_def.h"

static sff_sfp_field_map_t sfp_field_map[] = {
    /* Base page values, including alarms and sensors */
    {IDENTIFIER, {0x50, 0, 1}},
    {STATUS_CONTROL, {0x51, 110, 1}},
    {EXTENDED_STATUS_CONTROL, {0x51, 118, 1}},
    {ETHERNET_COMPLIANCE, {0x50, 3, 1}},
    {ETHERNET_EXTENDED_COMPLIANCE, {0x50, 36, 1}},
    {CONNECTOR_TYPE, {0x50, 2, 1}},
    {TRANSCEIVER_CODE, {0x50, 3, 8}},
    {RATE_IDENTIFIER, {0x50, 13, 1}},
    {ALARM_WARN_FLAGS, {0x51, 112, 6}},

};
const u8 Sfp_Eth_Comp_Table[] = {
    SR_10GBASE,
    LR_10GBASE,
    LRM_10GBASE,
    ER_10GBASE
};
/*0x50*/
#define SFP_TX_EQ_OFFSET (114)
#define SFP_RX_EM_OFFSET (115)
#define SFP_VENDOR_NAME_OFFSET (20)
#define SFP_VENDOR_PN_OFFSET (40)
#define SFP_VENDOR_SN_OFFSET (68)
#define SFP_VENDOR_REV_OFFSET (56)

#define SFP_VENDOR_NAME_LEN (16)
#define SFP_VENDOR_PN_LEN (16)
#define SFP_VENDOR_SN_LEN (16)
#define SFP_VENDOR_REV_LEN (4)

/*0x51*/
#define SFP_DDM_TEMP_OFFSET (96)
#define SFP_DDM_VCC_OFFSET (98)
#define SFP_DDM_TX_BIAS_OFFSET (100)
#define SFP_DDM_TX_POWER_OFFSET (102)
#define SFP_DDM_RX_POWER_OFFSET (104)

/* return sff_field_info_t for a given field in qsfp_field_map[] */
static sff_sfp_field_info_t *get_sff_sfp_field_addr(const Sff_field field)
{
    int i = 0;
    int cnt = sizeof(sfp_field_map) / sizeof(sff_sfp_field_map_t);

    for (i = 0; i < cnt; i++) {
        if (sfp_field_map[i].field == field) {
            return (&(sfp_field_map[i].data));
        }
    }
    return NULL;
}
/* return the contents of the  sff_field_info_t for a given field */
static int get_sfp_field_addr(Sff_field field,
                              u8 *slave_addr,
                              u8 *offset,
                              int *length)
{
    sff_sfp_field_info_t *info_data = NULL;
    info_data = get_sff_sfp_field_addr(field);

    if (field >= SFF_FIELD_MAX) {
        return -1;
    }
    if (!info_data) {
        return -1;
    }
    *offset = info_data->offset;
    *slave_addr = info_data->slave_addr;
    *length = info_data->length;
    return 0;
}
static int _sfp_transvr_codes_read(struct sff_obj_t *sff_obj, u8 *transvr_codes, int size)
{
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int data_len = 0;

    if(IS_ERR_OR_NULL(transvr_codes)) {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    get_sfp_field_addr(TRANSCEIVER_CODE, &slave_addr, &offset, &data_len);
    if(data_len != size) {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(sff_obj->func_tbl->eeprom_read(sff_obj, slave_addr, offset, transvr_codes, data_len) < 0) {
        SFF_MGR_ERR("sff_obj->func_tbl->eeprom_read fail");
        return -1;
    }

    return 0;
}

static int _sfp_eth_extended_compliance_get(struct sff_obj_t *sff_obj, u8 *eth_ext_comp)
{
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int data_len = 0;
    u8 data = 0;
    if(IS_ERR_OR_NULL(eth_ext_comp)) {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    get_sfp_field_addr(ETHERNET_EXTENDED_COMPLIANCE, &slave_addr, &offset, &data_len);
    if(data_len != sizeof(data)) {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(sff_obj->func_tbl->eeprom_read(sff_obj, slave_addr, offset, &data, data_len) < 0) {
        SFF_MGR_ERR("sff_obj->func_tbl->eeprom_read fail");
        return -1;
    }
    *eth_ext_comp = data;

    return 0;
}
static int _sfp_connector_type_read(struct sff_obj_t *sff_obj, u8 *conn_type)
{
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int data_len = 0;
    u8 data = 0;
    if(NULL == conn_type) {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    get_sfp_field_addr(CONNECTOR_TYPE, &slave_addr, &offset, &data_len);
    if(data_len != sizeof(data)) {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(sff_obj->func_tbl->eeprom_read(sff_obj, slave_addr, offset, &data, data_len) < 0) {
        SFF_MGR_ERR("sff_obj->func_tbl->eeprom_read fail");
        return -1;
    }
    *conn_type = data;

    return 0;
}
static bool sfp_eth_comp_is_supported(struct sff_obj_t *sff_obj, u8 eth_comp)
{
    bool is_supported = false;
    int size = ARRAY_SIZE(Sfp_Eth_Comp_Table);
    int idx = 0;
    u8 comp_codes = 0;
    for (idx = 0; idx < size; idx++) {
        if(eth_comp & Sfp_Eth_Comp_Table[idx]) {
            SFF_MGR_DEBUG("port:%d known eth_cmp:0x%x\n", sff_obj->port, eth_comp & Sfp_Eth_Comp_Table[idx]);
            comp_codes = eth_comp & Sfp_Eth_Comp_Table[idx];
            break;
        }
    }
    if(idx >= size) {

        SFF_MGR_DEBUG("port:%d unknown eth_cmp:0x%x\n", sff_obj->port, (eth_comp & 0xf0) >> 4);
    }

    switch (comp_codes) {
    case SR_10GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_10G_S_SR);
        break;
    case LR_10GBASE:
    case LRM_10GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_10G_S_LR);
        break;
    case ER_10GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_10G_S_ER);
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

static bool sfp_eth_ext_is_supported(struct sff_obj_t *sff_obj, u8 eth_ext_comp)
{
    bool is_supported = false;
    Ethernet_extended_compliance eth_ext_code = eth_ext_comp;
    switch(eth_ext_code) {
    case AOC_100G_BER_5:
    case AOC_100G_BER_12: /* 100G AOC or 25G AUI C2M AOC 10^^-12 BER */
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_25G_AOC);
        break;
    case SR4_100GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_25G_SR);
        break;
    case LR4_100GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_25G_LR);
        break;
    case ER4_100GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_25G_ER);
        break;
    case CR4_100GBASE:
        transvr_type_set(sff_obj, TRANSVR_CLASS_COPPER_L1_25G);
        break;
    case ACC_100G_BER_5:
    case ACC_100G_BER_12: /* 100G ACC or 25G AUI C2M ACC 10^^-12 BER */
    case CR_25GBASE_CA_S:
    case CR_25GBASE_CA_N:
    case T_10BASE_SFI: { /* 10BASE-T with SFI electrical interface */
        /*<to do> how about these types!?*/
        SFF_MGR_DEBUG("port:%d known eth_ext_cmp:0x%x\n", sff_obj->port, eth_ext_comp);
    }
    break;
    default:
        SFF_MGR_DEBUG("port:%d unknown eth_ext_cmp:%d\n", sff_obj->port, eth_ext_comp);

        break;
    }
    if (TRANSVR_CLASS_UNKNOWN != transvr_type_get(sff_obj)) {
        is_supported = true;
        SFF_MGR_DEBUG("port:%d known eth_ext_cmp:0x%x transvr_type:%d\n",
                      sff_obj->port, eth_ext_comp, transvr_type_get(sff_obj));
    }
    return is_supported;
}
int sfp_id_get(struct sff_obj_t *sff_obj, u8 *id)
{
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int len = 0;
    get_sfp_field_addr(IDENTIFIER, &slave_addr, &offset, &len);
    return sff_obj->func_tbl->eeprom_read(sff_obj, slave_addr , offset, id, len);
}
static int sfp_type_identify(struct sff_obj_t *sff_obj, bool *is_found)
{
    u8 id = 0;
    u8 transvr_codes[8];
    u8 conn_type = 0;
    u8 eth_ext_comp = 0;
    int idx = 0;
    int ret = 0;
    if((ret = sfp_id_get(sff_obj, &id)) < 0) {
        goto exit_err;
    }
    if(0x03 != id) { //sfp/sfp+/sfp28
        SFF_MGR_ERR("unknown id:0x%x\n", id);
        goto exit_non_support_err; /*<TBD> error handling in the future*/
    }

    if((ret = _sfp_eth_extended_compliance_get(sff_obj, &eth_ext_comp)) < 0) {
        goto exit_err;
    }

    if(!sfp_eth_ext_is_supported(sff_obj, eth_ext_comp)) {

        if((ret = _sfp_transvr_codes_read(sff_obj, transvr_codes, ARRAY_SIZE(transvr_codes))) < 0) {
            goto exit_err;
        }
        for (idx = 0; idx < 8; idx++) {
            SFF_MGR_DEBUG("port:%d transvr[%d]:0x%x\n", sff_obj->port, idx, transvr_codes[idx]);
        }

        if((ret = _sfp_connector_type_read(sff_obj, &conn_type)) < 0) {
            goto exit_err;
        }

        SFF_MGR_DEBUG("port:%d conn_type:0x%x\n", sff_obj->port,conn_type);

        /*check connector type first*/
        /*check transvr codes start from offset:3 */
        if (0x07 == conn_type) {
            if(!sfp_eth_comp_is_supported(sff_obj, transvr_codes[0])) {
                goto exit_non_support_err;
            }
        } else if (0x0b == conn_type) {


        } else if (0x21 == conn_type) {
            /*check SFP+ Cable Technology*/
            /*offset 8, bit2: Passive Cable *8
             *          bit3: Active Cable *8 */
            if (transvr_codes[5] & 0x02) {
                /*passive*/
            } else if (transvr_codes[5] & 0x03) {
                /*active*/
            } else {

                goto exit_non_support_err; /*<TBD> error handling in the future*/
            }
        }

    }
    *is_found = true;
    return 0;
exit_non_support_err:
    return 0;
exit_err: /*could be i2c fail , need to define err code in the future <TBD>*/
    return ret;
}
/*reference sff 8472: TABLE 9-12  ALARM AND WARNING FLAG BITS
 * start from offset 112 -> 117 : total size 6 bytes
 * offset 114 : 7-4 : tx input equalization control RATE=HIGH, 3-0: tx input equalization control RATE=LOW
 * offset 115 : 7-4 : RX output emphasis control RATE=HIGH, 3-0: RX output emphasis control RATE=LOW

 * */
int sfp_lane_control_set(struct sff_obj_t *sff_obj, int type, u32 value)
{
    u8 addr = SFF_DDM_I2C_ADDR;
    u8 offset = 0;
    bool fail = false;
    int ret = 0;
    u8 data = value;

    switch(type) {
    case TX_EQ_TYPE:
        offset = SFP_TX_EQ_OFFSET;
        break;
    case RX_EM_TYPE:
        offset = SFP_RX_EM_OFFSET;
        break;

    default:
        fail = true;
        break;
    }
    if (fail) {
        return -EINVAL;
    }
    if((ret = sff_obj->func_tbl->eeprom_write(sff_obj, addr, offset, &data, sizeof(data))) < 0) {
        return ret;
    }

    return 0;
}
/*reference sff 8472: TABLE 9-12  ALARM AND WARNING FLAG BITS
 * start from offset 112 -> 117 : total size 6 bytes
 * offset 114 : 7-4 : tx input equalization control RATE=HIGH, 3-0: tx input equalization control RATE=LOW
 * offset 115 : 7-4 : RX output emphasis control RATE=HIGH, 3-0: RX output emphasis control RATE=LOW

 * */
int sfp_lane_control_get(struct sff_obj_t *sff_obj, int type, u32 *value)
{
    u8 addr = SFF_DDM_I2C_ADDR;
    u8 offset = 0;
    bool fail = false;
    int ret = 0;
    u8 data = 0;

    switch(type) {
    case TX_EQ_TYPE:
        offset = SFP_TX_EQ_OFFSET;
        break;
    case RX_EM_TYPE:
        offset = SFP_RX_EM_OFFSET;
        break;
    default:
        fail = true;
        break;
    }
    if (fail) {
        return -EINVAL;
    }
    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, addr, offset, &data, sizeof(data))) < 0) {
        return ret;
    }
    *value = data;
    return 0;
}
int sfp_lane_status_get(struct sff_obj_t *sff_obj, int type, u8 *value)
{
    u8 reg = 0;
    int ret = 0;
    int (*func)(struct sff_obj_t *, u8 *value) = NULL;

    switch(type) {
    case LN_STATUS_RX_LOS_TYPE:
        func = sff_obj->func_tbl->rx_los_get;
        break;
    case LN_STATUS_TX_FAULT_TYPE:
        func = sff_obj->func_tbl->tx_fault_get;
        break;
    case LN_STATUS_TX_LOS_TYPE:
    default:
        break;
    }

    if (NULL == func) {
        return -ENOSYS;
    }
    if ((ret = func(sff_obj, &reg)) < 0) {
        return ret;
    }
    *value = reg;
    return 0;

}
int sfp_temperature_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    int ret = 0;
    u8 addr = SFF_DDM_I2C_ADDR;
    u8 offset = SFP_DDM_TEMP_OFFSET;
    u8 reg[WORD_SIZE];
    s16 temp = 0;

    union monitor_data_t monitor_data;

    s16 divider = 256;
    s16 decimal = 0;
    char *unit = "c";

    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, addr, offset, reg, WORD_SIZE)) < 0) {
        return ret;
    }

    monitor_data.byte[1] = reg[0];
    monitor_data.byte[0] = reg[1];
    temp = monitor_data.signed_word;

    decimal = ((temp/divider)*divider)-temp;
    decimal = abs(decimal);
    decimal = decimal*1000/divider;

    scnprintf(buf, buf_size,
              "%d.%d %s\n",
              temp/divider,
              decimal, unit);

    return 0;
}
int sfp_voltage_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    int ret = 0;
    u8 addr = SFF_DDM_I2C_ADDR;
    u8 offset = SFP_DDM_VCC_OFFSET;
    u8 reg[WORD_SIZE];
    u16 vol = 0;
    union monitor_data_t monitor_data;
    u16 divider = 10000;
    char *unit = "v";

    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, addr, offset, reg, WORD_SIZE)) < 0) {
        return ret;
    }

    monitor_data.byte[1] = reg[0];
    monitor_data.byte[0] = reg[1];
    vol = monitor_data.word;

    scnprintf(buf, buf_size,
              "%d.%d %s\n",
              vol/divider,
              vol%divider, unit);

    return 0;
}
int sfp_lane_monitor_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size)
{
    u8 addr = SFF_DDM_I2C_ADDR;
    u8 offset = 0;
    u8 reg[WORD_SIZE];
    union monitor_data_t monitor_data;
    u16 divider = 0;
    char *unit = NULL;
    struct monitor_para_t *para = NULL;
    bool fail = false;
    int ret = 0;

    switch(type) {
    case LN_MONITOR_TX_PWR_TYPE:
        offset = SFP_DDM_TX_POWER_OFFSET;
        break;
    case LN_MONITOR_RX_PWR_TYPE:
        offset = SFP_DDM_RX_POWER_OFFSET;
        break;
    case LN_MONITOR_TX_BIAS_TYPE:
        offset = SFP_DDM_TX_BIAS_OFFSET;

        break;
    default:
        fail = true;
        break;
    }
    if(fail) {
        return -EINVAL;
    }
    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, addr, offset, reg, sizeof(monitor_data))) < 0) {
        return ret;
    }
    para = monitor_para_find(type);
    if (IS_ERR_OR_NULL(para)) {
        return -EINVAL;
    }
    divider = para->divider;
    unit = para->unit;
    /*big edian*/
    monitor_data.byte[1] = reg[0];
    monitor_data.byte[0] = reg[1];
    scnprintf(buf, buf_size,
              "ch1: %d.%d %s\n",
              monitor_data.word/divider,
              monitor_data.word%divider, unit);
    return ret;
}
int sfp_vendor_info_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size)
{

    u8 addr = SFF_EEPROM_I2C_ADDR;
    u8 offset = 0;
    int len = 0;
    u8 reg[VENDOR_INFO_BUF_SIZE];
    bool fail = false;
    int ret = 0;

    switch(type) {
    case VENDOR_NAME_TYPE:
        offset = SFP_VENDOR_NAME_OFFSET;
        len = SFP_VENDOR_NAME_LEN;
        break;
    case VENDOR_PN_TYPE:
        offset = SFP_VENDOR_PN_OFFSET;
        len = SFP_VENDOR_PN_LEN;

        break;
    case VENDOR_SN_TYPE:
        offset = SFP_VENDOR_SN_OFFSET;
        len = SFP_VENDOR_SN_LEN;

        break;
    case VENDOR_REV_TYPE:
        offset = SFP_VENDOR_REV_OFFSET;
        len = SFP_VENDOR_REV_LEN;
        break;
    default:
        fail = true;
        break;
    }

    if(fail) {
        return -EINVAL;
    }
    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, addr, offset, reg, len)) < 0) {
        return ret;
    }
    /*add terminal of string*/
    reg[len] = '\0';

    scnprintf(buf, buf_size, "%s\n", reg);

    return ret;
}
static int sfp_rate_select(struct sff_obj_t *sff_obj, u8 rate_bitmap)
{
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 status = 0;
    u8 ext_status = 0;
    int port = sff_obj->port;
    if(rate_bitmap & SOFT_RX_RATE_RS0) {
        get_sfp_field_addr(STATUS_CONTROL, &slave_addr, &offset, &len);
        if( sff_obj->func_tbl->eeprom_read(sff_obj, slave_addr , offset, &status, len) < 0) {
            goto exit_err;

        }
        SFF_MGR_DEBUG("port:%d status:0x%x\n", port, status);
        /*set bit 3:  Soft Rate_Select
        *          * Select * [aka. "RS(0)"] */
        set_bit(3, (unsigned long *)&status);
        if( sff_obj->func_tbl->eeprom_write(sff_obj, slave_addr , offset, &status, len) < 0) {
            goto exit_err;
        }
#ifdef READBACK_CHECK
        if( sff_obj->func_tbl->eeprom_read(sff_obj, slave_addr , offset, &status, len) < 0) {
            goto exit_err;

        }
        SFF_MGR_DEBUG("port:%d status:0x%x\n", port, status);
#endif
    }
    if(rate_bitmap & SOFT_TX_RATE_RS1) {
        get_sfp_field_addr(EXTENDED_STATUS_CONTROL, &slave_addr, &offset, &len);
        if( sff_obj->func_tbl->eeprom_read(sff_obj, slave_addr , offset, &ext_status, len) < 0) {
            goto exit_err;

        }
        SFF_MGR_DEBUG("port:%d status:0x%x\n", port, ext_status);
        /*set bit 3:  Soft Rate_Select
        *          * Select * [aka. "RS(0)"] */
        set_bit(3, (unsigned long *)&ext_status);
        if( sff_obj->func_tbl->eeprom_write(sff_obj, slave_addr , offset, &ext_status, len) < 0) {
            goto exit_err;
        }
#ifdef READBACK_CHECK
        if( sff_obj->func_tbl->eeprom_read(sff_obj, slave_addr , offset, &ext_status, len) < 0) {
            goto exit_err;

        }
        SFF_MGR_DEBUG("port:%d status:0x%x\n", port, status);
#endif
    }
    return 0;
exit_err:
    return -1;
}
static int sfp_rate_select_control(struct sff_obj_t *sff_obj)
{
    u8 rate_id = 0;
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int len = 0;
    bool found = false;
    int port = sff_obj->port;
    /*read rate id*/
    get_sfp_field_addr(RATE_IDENTIFIER, &slave_addr, &offset, &len);
    if( sff_obj->func_tbl->eeprom_read(sff_obj, slave_addr , offset, &rate_id, len) < 0) {
        goto exit_err;
    }
    SFF_MGR_DEBUG("port:%d rate_id:0x%x\n", port, rate_id);
    switch (rate_id) {
    case 0x00: /* Unspecified */
    case 0x03: /* Unspecified */
    case 0x05: /* Unspecified */
    case 0x07: /* Unspecified */
    case 0x09: /* Unspecified */
    case 0x0B: /* Unspecified */
    case 0x0D: /* Unspecified */
    case 0x0F: /* Unspecified */
        break;
    case 0x02: /* SFF-8431 (8/4/2G Rx Rate_Select only) */
        if(sfp_rate_select(sff_obj, SOFT_RX_RATE_RS0) < 0) {
            goto exit_err;
        }
        found = true;
        break;
    case 0x04: /* SFF-8431 (8/4/2G Tx Rate_Select only) */
        if(sfp_rate_select(sff_obj, SOFT_TX_RATE_RS1) < 0) {
            goto exit_err;
        }
        found = true;
        break;
    case 0x06: /* SFF-8431 (8/4/2G Independent Rx & Tx Rate_select) */
        if(sfp_rate_select(sff_obj, SOFT_RX_RATE_RS0|SOFT_TX_RATE_RS1) < 0) {
            goto exit_err;
        }
        found = true;
        break;
    case 0x01: /* SFF-8079 (4/2/1G Rate_Select & AS0/AS1) */
    case 0x08: /* FC-PI-5 (16/8/4G Rx Rate_select only)
                  * High=16G only, Low=8G/4G
                  *         *                     */
    case 0x0A: /* FC-PI-5 (16/8/4G Independent Rx, Tx Rate_select)
                  * High=16G only, Low=8G/4G
                  *         *                     */
    case 0x0C: /* FC-PI-6 (32/16/8G Independent Rx, Tx Rate_Select)
                  * High=32G only, Low = 16G/8G
                  *         *                     */
    case 0x0E: /* 10/8G Rx and Tx Rate_Select controlling the operation or
                  * locking modes of the internal signal conditioner, retimer
                  *         *                     * or CDR, according to the logic table defined in Table 10-2,
                  *                 *                                         * High Bit Rate (10G) =9.95-11.3 Gb/s; Low Bit Rate (8G) =
                  *                         *                                                             * 8.5 Gb/s. In this mode, the default value of bit 110.3 (Soft
                  *                                 *                                                                                 * Rate Select RS(0), Table 9-11) and of bit 118.3 (Soft Rate
                  *                                         *                                                                                                     * Select RS(1), Table 10-1) is 1.
                  *                                                 *                                                                                                                         */
    default:
        break;
    }
    if(!found) {
        SFF_MGR_DEBUG("port:%d no support\n", port);
    }
    return 0;
exit_err:
    return -1;
}

static int _sfp_data_ready_check(struct sff_obj_t *sff_obj, u8 *is_ready)
{
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int data_len = 0;
    int ret = 0;
    u8 status = 0;
    if(IS_ERR_OR_NULL(is_ready)) {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    get_sfp_field_addr(STATUS_CONTROL, &slave_addr, &offset, &data_len);
    if((ret = sff_obj->func_tbl->eeprom_read(sff_obj, slave_addr, offset, &status, data_len)) < 0) {
        return ret;
    }
    status = ~(status & 0x01) & 0x01;
    *is_ready = status;

    return 0;
#if 0
    SFF_MGR_DEBUG("special_case of checking data ready\n");
    /*some cables don't support status control, in this case, check id item instead*/
    get_sfp_field_addr(IDENTIFIER, &slave_addr, &offset, &data_len);
    if (data_len != sizeof(status)) {
        goto exit_err;
    }
    if(sff_obj->func_tbl->eeprom_read(sff_obj, slave_addr, offset, &status, data_len) < 0) {
        goto exit_err;
    }
    *is_ready = 1;

    return 0;
exit_err:
    return -1;
#endif
}
int sff_fsm_sfp_task(struct sff_obj_t *sff_obj)
{
    sff_fsm_state_t st = sff_fsm_st_get(sff_obj);
    int ret = 0;
    u8 is_ready = 0;
    bool is_found = false;
    switch (st) {

    case SFF_FSM_ST_IDLE:
        break;
    case SFF_FSM_ST_REMOVED:
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_IDLE);
        break;
    case SFF_FSM_ST_INSERTED:
        ret = _sfp_data_ready_check(sff_obj, &is_ready);
        if (ret < 0) {
            break;
        }

        if(is_ready) {
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_INIT);
        }

        break;

    case SFF_FSM_ST_IDENTIFY:
        if((ret = sfp_type_identify(sff_obj, &is_found)) < 0) {
            break;
        }

        if(is_found) {
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTED);
        } else {
            /*unknown type: special handling*/
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_UNKNOWN_TYPE);
        }

        break;

    case SFF_FSM_ST_DETECTED:

        if((ret = sff_obj->func_tbl->tx_disable_set(sff_obj, 0)) < 0) {
            break;
        }
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_MONITOR);
        break;
    case SFF_FSM_ST_ISOLATED:

        break;
    case SFF_FSM_ST_INIT:

        if((ret = sff_obj->func_tbl->tx_disable_set(sff_obj, 1)) < 0) {
            break;
        }
        if ((ret = sfp_rate_select_control(sff_obj)) < 0) {
            break;
        }
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_IDENTIFY);
        break;
    case SFF_FSM_ST_FAULT:
        break;
    case SFF_FSM_ST_MONITOR:
        break;
    case SFF_FSM_ST_UNKNOWN_TYPE:
        break;
    default:
        SFF_MGR_ERR("unknown fsm st:%d\n", st);
        break;

    }
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

