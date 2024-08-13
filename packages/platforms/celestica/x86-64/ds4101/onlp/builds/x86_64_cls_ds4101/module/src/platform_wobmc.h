#ifndef _PLATFORM_WOBMC_H_
#define _PLATFORM_WOBMC_H_
#include "platform_comm.h"
#include "fancontrol.h"

#define PSU1_I2C_BUS            47
#define PSU2_I2C_BUS            48
#define PSU1_I2C_ADDR           0x58
#define PSU2_I2C_ADDR           0x59

#define PMBUS_MODEL_REG         0x9a
#define PMBUS_SERIAL_REG        0x9e

#define NOT_DEFINE              125000
#define PRESSURE_RAW_PATH       "/sys/bus/i2c/devices/i2c-61/61-0060/iio:device0/in_pressure_raw"
#define PRESSURE_SCALE_PATH     "/sys/bus/i2c/devices/i2c-61/61-0060/iio:device0/in_pressure_scale"

struct temp {
    int lo_warn;
    int lo_err;
    int lo_shutdown;
    int hi_warn;
    int hi_err;
    int hi_shutdown;
    int temp; /* /1000 */
    const char *descr;
    char *path;
    uint8_t is_ovwarn_fault:4;
    uint8_t is_overr_fault:2;
    uint8_t is_ovshutdown_fault:2;
};

struct vol_curr_pwr {
    long lo_warn;
    long lo_err;
    long lo_shutdown;
    long hi_warn;
    long hi_err;
    long hi_shutdown;
    long vol_curr_pwr;
    const char *descr;
    char *path;
    uint8_t is_udwarn_fault:1;
    uint8_t is_uderr_fault:1;
    uint8_t is_udshutdown_fault:1;
    uint8_t is_ovwarn_fault:1;
    uint8_t is_overr_fault:1;
    uint8_t is_ovshutdown_fault:3;
};

struct fan {
    const char *pres_path;
    const char *frpm_path;
    const char *rrpm_path;
    const char *pwm_path;
    const char *dir_path;
    const char *led_path;
    const char *eeprom_path;
    const char *descr;
    uint8_t is_absent:1;
    uint8_t is_airflow_fault:1;
    uint8_t is_udminspd_ffault:1;
    uint8_t is_udminspd_rfault:1;
    uint8_t is_udspd_ffault:1;
    uint8_t is_udspd_rfault:1;
    uint8_t is_ovspd_ffault:1;
    uint8_t is_ovspd_rfault:1;
};

struct psu {
    const char *vin_path;
    const char *vout_path;
    const char *cin_path;
    const char *cout_path;
    const char *pin_path;
    const char *pout_path;
    const char *speed_path;
    const char *descr;
    uint8_t is_absent:1;
    uint8_t is_alert:1;
    uint8_t is_airflow_fault:2;
    uint8_t is_ac_fault:2;
    uint8_t is_power_fault:2;
};

#define BITS_PER_LONG_LONG 64
#define GENMASK_ULL(h, l) \
    (((~0ULL) - (1ULL << (l)) + 1) & \
     (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))

#define FRU_COMM_HDR_LEN                8
#define FRU_BRDINFO_MFT_TLV_STRT        6   //Board Manufacturer byte offset in "Board Info Area Format" field.
#define TLV_TYPE_MASK                   GENMASK_ULL(7,6)
#define TLV_LEN_MASK                    GENMASK_ULL(5,0)


int get_psu_model_sn_wobmc(int id,char* model,char* serial_number);
int get_psu_info_wobmc(int id,int *mvin,int *mvout,int *mpin,int *mpout,int *miin,int *miout);
int get_fan_info_wobmc(int id,char* model,char* serial,int *get_fan_info);
int get_sensor_info_wobmc(int id, int *temp, int *warn, int *error, int *shutdown);
int get_fan_speed_wobmc(int id,int* per,int* rpm);

int check_sys_airflow(void);
int check_fan_airflow_fault(uint8_t sys_airflow);
int check_fan_absent(void);
int check_fan_fault(void);
int check_psu_airflow_fault(uint8_t sys_airflow);
int check_psu_absent(void);
int check_psu_fault(void);

#endif /* _PLATFORM_WOBMC_H_ */

