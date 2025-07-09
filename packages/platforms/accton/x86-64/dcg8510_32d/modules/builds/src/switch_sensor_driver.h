#ifndef SWITCH_SENSOR_DRIVER_H
#define SWITCH_SENSOR_DRIVER_H

#include "switch.h"

#define SENSOR_ERR(fmt, args...)      LOG_ERR("sensor: ", fmt, ##args)
#define SENSOR_WARNING(fmt, args...)  LOG_WARNING("sensor: ", fmt, ##args)
#define SENSOR_INFO(fmt, args...)     LOG_INFO("sensor: ", fmt, ##args)
#define SENSOR_DEBUG(fmt, args...)    LOG_DBG("sensor: ", fmt, ##args)


/* temperature sensor */
struct temp_sensor_node_s {
    uint   bus_num;
    ushort dev_addr;
    long   temp_max;
    long   temp_max_hyst;
    long   temp_min;
    char*  alias_string;
    char*  type_string;
};
typedef struct temp_sensor_node_s temp_sensor_node_t;


enum temp_index {
    LSW_CORE = 0,
    X86_CORE1 = 1,
    X86_CORE2 = 2,
    X86_CORE3 = 3,
    X86_CORE4 = 4,
    X86_PACKAGE = 5,
    TMP75_48_TEMP = 6,
    TMP75_49_TEMP = 7,
    TMP75_4A_TEMP = 8,
    TMP75_4B_TEMP = 9,
    TMP431_4C_TEMP1 = 10,
    TMP431_4C_TEMP2 = 11,
    TMP75_4E_TEMP = 12,
    TMP275_4D_TEMP = 13,
    TMP275_4E_TEMP = 14,
    VCORE_MAC_TEMP = 15,
    VDDT0V9_R_MAC_TEMP = 16,
    VDD1V5_1V8_MAC_TEMP = 17,
    VCC3V3_QSFP_AB_TEMP = 18,
    VCC3V3_QSFP_CD_TEMP = 19,
    TEMP_TOTAL_NUM = 20,
};

/* voltage regulator */
struct vr_sensor_node_s {
    u8  chip_id;
    u8  channel;
    u8  attr_type;
    long  data_max;
    long  data_min;
    long  data_nominal;
    long  data_range;
    char* alias_string;
    char* type_string;
    char* node_name;
};
typedef struct vr_sensor_node_s vr_sensor_node_t;

struct vr_chip_info_s {
    unsigned char  chip_id;
    unsigned int   bus_num;
    unsigned short dev_addr;
};
typedef struct vr_chip_info_s vr_chip_info_t;


enum vr_chip_id {
    UCD90320 = 0,
    LM25066 = 1,
    MP2973 = 2,
    MP2940 = 3,
    MP2976_U1B1 = 4,
    MP2976_U1J1 = 5,
    MP2976_U1J2 = 6,
    TOTAL_VR_CHIP_NUM = 7,
};

enum vr_channel_num {
    CHANNEL_DEFAULT = 0,
    CHANNEL_FIRST   = 1,
    CHANNEL_SECORND = 2,
    CHANNEL_THIRD   = 3,
    CHANNEL_FORTH   = 4,
    CHANNEL_FIFTH   = 5,
    CHANNEL_SIXTH   = 6,
    CHANNEL_SEVENTH = 7,
    CHANNEL_EIGHTH  = 8,
    CHANNEL_NINTH   = 9,
    CHANNEL_TEHTH   = 10,
    CHANNEL_ELEVEN    = 11,
    CHANNEL_TWELVE    = 12,
    CHANNEL_THIRTEEN  = 13,
    CHANNEL_FOURTEEN  = 14,
    CHANNEL_FIFTEEN   = 15,
    CHANNEL_SIXTEEN   = 16,
    CHANNEL_SEVENTEEN = 17,
    CHANNEL_EIGHTEEN  = 18,
    CHANNEL_NINETEEN  = 19,
    CHANNEL_TWENTY    = 20,
    CHANNEL_TWENTY_ONE   = 21,
    CHANNEL_TWENTY_TWO   = 22,
    CHANNEL_TWENTY_THREE = 23,
};

enum vr_attr_type {
    VR_TEMP = 0,
    VR_VIN = 1,
    VR_VOUT = 2,
    VR_IIN = 3,
    VR_IOUT = 4,
    VR_PIN = 5,
    VR_POUT = 6,
};

enum vol_index {
    M_V12V_PSU_VOUT = 0,
    M_V12V_COME_VOUT = 1,
    M_V12V_MB_VOUT = 2,
    M_V5V_SB_VOUT = 3,
    M_3V3SB_PSU_VOUT = 4,
    M_V3V3_SB_VOUT = 5,
    M_VDD3P3_VOUT = 6,
    M_3V3_QSFPA_VOUT = 7,
    M_3V3_QSFPB_VOUT = 8,
    M_3V3_QSFPC_VOUT = 9,
    M_3V3_QSFPD_VOUT = 10,
    M_VDDA_1V8_VOUT = 11,
    M_VDD_1V8_VOUT = 12,
    M_V1V5_VOUT = 13,
    M_VDDA_1V5_VOUT = 14,
    M_V1V2_VOUT = 15,
    M_V1V05_VOUT = 16,
    M_VDD_1V05_VOUT = 17,
    M_VDDA_1V0_VOUT = 18,
    M_VDDT_1V0_VOUT = 19,
    M_VDDT_0V9_VOUT = 20,
    M_VDD_0V75_VOUT = 21,
    M_VDD_CORE_VOUT = 22,
    VCC12VIN_HOTSWAP_VIN = 23,
    VCC12VIN_HOTSWAP_VOUT = 24,
    VCORE_MAC_VIN = 25,
    VCORE_MAC_VOUT = 26,
    VDDT0V9_R_MAC_VIN = 27,
    VDDT0V9_R_MAC_VOUT = 28,
    VDD1V5_1V8_MAC_VIN = 29,
    VDDA1V5_MAC_VOUT = 30,
    VDDA1V8_MAC_VOUT = 31,
    VCC3V3_QSFP_AB_VIN = 32,
    VCC3V3_QSFP_A_VOUT1 = 33,
    VCC3V3_QSFP_B_VOUT1 = 34,
    VCC3V3_QSFP_CD_VIN = 35,
    VCC3V3_QSFP_C_VOUT1 = 36,
    VCC3V3_QSFP_D_VOUT1 = 37,
    VOL_TOTAL_NUM = 38,
};

enum curr_index {
    VCC12VIN_HOTSWAP_IIN = 0,
    VCC12VIN_HOTSWAP_IOUT = 1,
    VCORE_MAC_IIN = 2,
    VCORE_MAC_IOUT = 3,
    VDDT0V9_R_MAC_IOUT = 4,
    VDD1V5_1V8_MAC_IIN = 5,
    VDDA1V5_MAC_IOUT = 6,
    VDDA1V8_MAC_IOUT = 7,
    VCC3V3_QSFP_AB_IIN = 8,
    VCC3V3_QSFP_A_IOUT = 9,
    VCC3V3_QSFP_B_IOUT = 10,
    VCC3V3_QSFP_CD_IIN = 11,
    VCC3V3_QSFP_C_IOUT = 12,
    VCC3V3_QSFP_D_IOUT = 13,
    CURR_TOTAL_NUM = 14,
};

struct sensor_drivers_t{
    ssize_t (*get_temp_alias) (unsigned int index, char *buf);
    ssize_t (*get_temp_type) (unsigned int index, char *buf);
    bool (*get_temp_max) (unsigned int index, long *val);
    bool (*set_temp_max) (unsigned int index, long data);
    bool (*get_temp_max_hyst) (unsigned int index, long *val);
    bool (*get_temp_min) (unsigned int index, long *val);
    bool (*set_temp_min) (unsigned int index, long data);
    bool (*get_temp_input) (unsigned int index, long *temp_input);
    ssize_t (*get_vol_alias) (unsigned int index, char *buf);
    ssize_t (*get_vol_type) (unsigned int index, char *buf);
    bool (*get_vol_max) (unsigned int index, long *val);
    bool (*set_vol_max) (unsigned int index, long data);
    bool (*get_vol_min) (unsigned int index, long *val);
    bool (*set_vol_min) (unsigned int index, long data);
    bool (*get_vol_input) (unsigned int index, long *in_input);
    bool (*get_vol_range) (unsigned int index, long *val);
    bool (*get_vol_nominal) (unsigned int index, long *val);
    ssize_t (*get_curr_alias) (unsigned int index, char *buf);
    ssize_t (*get_curr_type) (unsigned int index, char *buf);
    bool (*get_curr_max) (unsigned int index, long *val);
    bool (*set_curr_max) (unsigned int index, long data);
    bool (*get_curr_min) (unsigned int index, long *val);
    bool (*set_curr_min) (unsigned int index, long data);
    bool (*get_curr_input) (unsigned int index, long *curr_input);
    void (*get_loglevel) (long *lev);
    void (*set_loglevel) (long lev);
    ssize_t (*debug_help) (char *buf);
    ssize_t (*debug_help_hisonic) (char *buf);
    ssize_t (*debug) (const char *buf, int count);
};

ssize_t drv_get_temp_alias(unsigned int index, char *buf);
ssize_t drv_get_temp_type(unsigned int index, char *buf);
bool drv_get_temp_max(unsigned int index, long *val);
bool drv_set_temp_max(unsigned int index, long data);
bool drv_get_temp_max_hyst(unsigned int index, long *val);
bool drv_get_temp_min(unsigned int index, long *val);
bool drv_set_temp_min(unsigned int index, long data);
bool drv_get_temp_input(unsigned int index, long *temp_input);
ssize_t drv_get_vol_alias(unsigned int index, char *buf);
ssize_t drv_get_vol_type(unsigned int index, char *buf);
bool drv_get_vol_max(unsigned int index, long *val);
bool drv_set_vol_max(unsigned int index, long data);
bool drv_get_vol_min(unsigned int index, long *val);
bool drv_set_vol_min(unsigned int index, long data);
bool drv_get_vol_input(unsigned int index, long *vol_input);
bool drv_get_vol_range(unsigned int index, long *val);
bool drv_get_vol_nominal(unsigned int index, long *val);
bool drv_get_vol_input_from_bmc(unsigned int index, long *vol_input);
ssize_t drv_get_curr_alias(unsigned int index, char *buf);
ssize_t drv_get_curr_type(unsigned int index, char *buf);
bool drv_get_curr_max(unsigned int index, long *val);
bool drv_set_curr_max(unsigned int index, long data);
bool drv_get_curr_min(unsigned int index, long *val);
bool drv_set_curr_min(unsigned int index, long data);
bool drv_get_curr_input(unsigned int index, long *curr_input);
bool drv_get_curr_input_from_bmc(unsigned int index, long *curr_input);
void drv_get_loglevel(long *lev);
void drv_set_loglevel(long lev);
ssize_t drv_debug_help(char *buf);
ssize_t drv_debug_help_hisonic(char *buf);
ssize_t drv_debug(const char *buf, int count);

void s3ip_sensor_drivers_register(struct sensor_drivers_t *p_func);
void s3ip_sensor_drivers_unregister(void);

#endif /* SWITCH_SENSOR_DRIVER_H */
