#ifndef SWITCH_PSU_DRIVER_H
#define SWITCH_PSU_DRIVER_H

#include "switch.h"

#define PSU_ERR(fmt, args...)      LOG_ERR("psu: ", fmt, ##args)
#define PSU_WARNING(fmt, args...)  LOG_WARNING("psu: ", fmt, ##args)
#define PSU_INFO(fmt, args...)     LOG_INFO("psu: ", fmt, ##args)
#define PSU_DEBUG(fmt, args...)    LOG_DBG("psu: ", fmt, ##args)

#define CHECK_BIT(a, b)  (((a) >> (b)) & 1U)

#define PSU_TOTAL_NUM               2
#define PSU1_INDEX                  1
#define PSU2_INDEX                  2
#define PSU_FAN_DIRECT_F2B  0
#define PSU_FAN_DIRECT_B2F  1
#define PSU_TOTAL_TEMP_NUM          2
#define PSU_NAME_STRING             "psu"
#define TEMP_NAME_STRING            "temp"
#define PSU_MAX_STR_BUFF_LENGTH     40

#define PSU_ONLINE_OFFSET           0x2D
#define PSU_ONLINE_PSU1_MASK        0x04
#define PSU_ONLINE_PSU2_MASK        0x40

#define PSU_STATUS_OFFSET           0x0D
#define PSU_STATUS_PSU1_ALARM       0x01
#define PSU_STATUS_PSU2_ALARM       0x10
#define PSU_STATUS_PSU1_IPOK        0x02
#define PSU_STATUS_PSU2_IPOK        0x20
#define PSU_STATUS_PSU1_OPOK        0x08
#define PSU_STATUS_PSU2_OPOK        0x80

// the following definition should be the same as pmbus.h
#define PMBUS_POUT_MAX              0x31
#define PMBUS_VIN_OV_WARN_LIMIT     0x57
#define PMBUS_IIN_OC_WARN_LIMIT     0x5D
#define PMBUS_STATUS_WORD           0x79
#define PMBUS_STATUS_INPUT          0x7C
#define PMBUS_READ_VIN              0x88
#define PMBUS_READ_IIN              0x89
#define PMBUS_READ_VOUT             0x8B
#define PMBUS_READ_IOUT             0x8C
#define PMBUS_READ_TEMPERATURE_1    0x8D
#define PMBUS_READ_TEMPERATURE_2    0x8E
#define PMBUS_READ_TEMPERATURE_3    0x8F
#define PMBUS_READ_FAN_SPEED_1      0x90
#define PMBUS_MFR_ID                0x99
#define PMBUS_MFR_MODEL             0x9A
#define PMBUS_MFR_REVISION          0x9B
#define PMBUS_MFR_DATE              0x9D
#define PMBUS_MFR_SERIAL            0x9E
#define PMBUS_READ_POUT             0x96
#define PMBUS_READ_PIN              0x97
// new define
#define PMBUS_PART_NUM              0xDE

#define PSU_ABSENT_RET              0
#define PSU_OK_RET                  1
#define PSU_NOT_OK_RET              2

#define PSU_FAN_SPEED_MAX_RPM       24500

struct psu_drivers_t{
    int (*get_mfr_info) (unsigned int psu_index, u8 pmbus_command, char *buf);
    ssize_t (*get_temp_input) (unsigned int psu_index, unsigned int psu_temp_index, long *temp_input);
    bool (*get_alarm_threshold_curr) (unsigned int psu_index, long *buf);
    bool (*get_alarm_threshold_vol) (unsigned int psu_index, long *buf);
    bool (*get_max_output_power) (unsigned int psu_index, long *buf);
    ssize_t (*get_num_temp_sensors) (unsigned int psu_index, char *buf);
    ssize_t (*get_psu_temp_alias) (unsigned int psu_index, unsigned int psu_temp_index, char *buf);
    ssize_t (*get_psu_temp_type) (unsigned int psu_index, unsigned int psu_temp_index, char *buf);
    ssize_t (*get_psu_temp_max) (unsigned int psu_index, unsigned int psu_temp_index, char *buf);
    ssize_t (*set_psu_temp_max) (unsigned int psu_index, unsigned int psu_temp_index, int val);
    ssize_t (*get_psu_temp_max_hyst) (unsigned int psu_index, unsigned int psu_temp_index, char *buf);
    ssize_t (*get_psu_temp_min) (unsigned int psu_index, unsigned int psu_temp_index, char *buf);
    ssize_t (*set_psu_temp_min) (unsigned int psu_index, unsigned int psu_temp_index, int val);
    ssize_t (*get_psu_pok_alarm) (unsigned int psu_index, unsigned int *alarm);
    ssize_t (*get_psu_in_pok) (unsigned int psu_index, unsigned int *status);
    ssize_t (*get_psu_out_pok) (unsigned int psu_index, unsigned int *status);
    void (*get_loglevel) (long *lev);
    void (*set_loglevel) (long lev);
    ssize_t (*debug_help) (char *buf);
    ssize_t (*debug_help_hisonic) (char *buf);
    ssize_t (*debug) (const char *buf, int count);
    ssize_t (*get_scan_period) (char *buf);
    void (*set_scan_period) (unsigned int period);
    ssize_t (*get_present) (unsigned int psu_index, unsigned int *present);
};

int drv_get_mfr_info(unsigned int psu_index, u8 pmbus_command, char *buf);
ssize_t drv_get_num_temp_sensors(unsigned int psu_index, char *buf);
ssize_t drv_get_psu_temp_alias(unsigned int psu_index, unsigned int psu_temp_index, char *buf);
ssize_t drv_get_psu_temp_type(unsigned int psu_index, unsigned int psu_temp_index, char *buf);
ssize_t drv_get_psu_temp_max(unsigned int psu_index, unsigned int psu_temp_index, char *buf);
ssize_t drv_set_psu_temp_max(unsigned int psu_index, unsigned int psu_temp_index, int val);
ssize_t drv_get_psu_temp_max_hyst(unsigned int psu_index, unsigned int psu_temp_index, char *buf);
ssize_t drv_get_psu_temp_min(unsigned int psu_index, unsigned int psu_temp_index, char *buf);
ssize_t drv_set_psu_temp_min(unsigned int psu_index, unsigned int psu_temp_index, int val);
ssize_t get_psu_pok_alarm(unsigned int psu_index, unsigned int *alarm);
ssize_t drv_get_psu_in_pok(unsigned int psu_index, unsigned int *status);
ssize_t drv_get_psu_out_pok(unsigned int psu_index, unsigned int *status);
void drv_get_loglevel(long *lev);
void drv_set_loglevel(long lev);
ssize_t drv_debug_help(char *buf);
ssize_t drv_debug_help_hisonic(char *buf);
ssize_t drv_debug(const char *buf, int count);
ssize_t drv_get_scan_period(char *buf);
void drv_set_scan_period(unsigned int period);
ssize_t drv_get_psu_present(unsigned int psu_index, unsigned int *present);

void s3ip_psu_drivers_register(struct psu_drivers_t *p_func);
void s3ip_psu_drivers_unregister(void);

#endif /* SWITCH_PSU_DRIVER_H */
