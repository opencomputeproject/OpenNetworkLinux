#ifndef _PLATFORM_BELGITE_H_
#define _PLATFORM_BELGITE_H_
#include <stdint.h>
#include <onlp/platformi/fani.h>

#define PREFIX_PATH_LEN 100

#define SYS_CPLD_PATH "/sys/devices/platform/sys_cpld/"
#define PLATFORM_PATH "/sys/devices/platform/sys_cpld/SFP"
#define I2C_DEVICE_PATH "/sys/bus/i2c/devices/"
#define PREFIX_PATH_ON_SYS_EEPROM "/sys/bus/i2c/devices/i2c-1/1-0052/eeprom"

//FAN
#define FAN_COUNT  5
#define CHASSIS_FAN_COUNT 3 
#define PSU_FAN_COUNT 2

#define CHASSIS_FAN_MAX_RPM   28600
#define PSU_FAN_MAX_RPM   18000 /* torelent is 15% up to 20700*/

#define CHASSIS_FAN1_DIRECTION        "/sys/bus/i2c/devices/i2c-2/2-0032/fan1_direction"
#define CHASSIS_FAN1_FAULT            "/sys/bus/i2c/devices/i2c-2/2-0032/fan1_fault"
#define CHASSIS_FAN1_SPEED            "/sys/bus/i2c/devices/i2c-2/2-0032/fan1_input"
#define CHASSIS_FAN1_LED              "/sys/bus/i2c/devices/i2c-2/2-0032/fan1_led"
#define CHASSIS_FAN1_PRESENT          "/sys/bus/i2c/devices/i2c-2/2-0032/fan1_present"
#define CHASSIS_FAN1_PWM              "/sys/bus/i2c/devices/i2c-2/2-0032/pwm1"

#define CHASSIS_FAN2_DIRECTION        "/sys/bus/i2c/devices/i2c-2/2-0032/fan2_direction"
#define CHASSIS_FAN2_FAULT            "/sys/bus/i2c/devices/i2c-2/2-0032/fan2_fault"
#define CHASSIS_FAN2_SPEED            "/sys/bus/i2c/devices/i2c-2/2-0032/fan2_input"
#define CHASSIS_FAN2_LED              "/sys/bus/i2c/devices/i2c-2/2-0032/fan2_led"
#define CHASSIS_FAN2_PRESENT          "/sys/bus/i2c/devices/i2c-2/2-0032/fan2_present"
#define CHASSIS_FAN2_PWM              "/sys/bus/i2c/devices/i2c-2/2-0032/pwm2"

#define CHASSIS_FAN3_DIRECTION        "/sys/bus/i2c/devices/i2c-2/2-0032/fan3_direction"
#define CHASSIS_FAN3_FAULT            "/sys/bus/i2c/devices/i2c-2/2-0032/fan3_fault"
#define CHASSIS_FAN3_SPEED            "/sys/bus/i2c/devices/i2c-2/2-0032/fan3_input"
#define CHASSIS_FAN3_LED              "/sys/bus/i2c/devices/i2c-2/2-0032/fan3_led"
#define CHASSIS_FAN3_PRESENT          "/sys/bus/i2c/devices/i2c-2/2-0032/fan3_present"
#define CHASSIS_FAN3_PWM              "/sys/bus/i2c/devices/i2c-2/2-0032/pwm3"

#define PSU1_FAN                      "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/hwmon*/fan1_input"
#define PSU2_FAN                      "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/hwmon*/fan1_input"

#define CHASSIS_FAN1_ID  1   /* This related to above paths should match structure in fani.c*/
#define CHASSIS_FAN2_ID  2
#define CHASSIS_FAN3_ID  3
#define PSU1_FAN_ID      4
#define PSU2_FAN_ID      5

//PSU
#define PSU_COUNT  2 
#define PSU_STA_REGISTER 0xA141
#define LED_PSU_REGISTER 0xA142

#define PSU1_CIN "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/hwmon*/curr1_input"
#define PSU1_VIN "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/hwmon*/in1_input"
#define PSU1_PIN "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/hwmon*/power1_input"
#define PSU1_COUT "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/hwmon*/curr2_input"
#define PSU1_VOUT "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/hwmon*/in2_input"
#define PSU1_POUT "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/hwmon*/power2_input"
#define PSU2_CIN "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/hwmon*/curr1_input"
#define PSU2_VIN "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/hwmon*/in1_input"
#define PSU2_PIN "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/hwmon*/power1_input"
#define PSU2_COUT "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/hwmon*/curr2_input"
#define PSU2_VOUT "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/hwmon*/in2_input"
#define PSU2_POUT "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/hwmon*/power2_input"
#define PSU1_ID 1  /* This related to above paths should match structure in psui.c*/
#define PSU2_ID 2 

#define PSU_I2C_BUS 4
#define PSU1_I2C_ADDR 0x59
#define PSU2_I2C_ADDR 0x58
#define PMBUS_MODEL_REG 0x9a
#define PMBUS_SERIAL_REG 0x9e

//THERMAL
#define THERMAL_COUNT 9
#define THERMAL_REGISTER 0xA145
#define CHASSIS_THERMAL_COUNT 5 

#define CHASSIS_THERMAL1      "/sys/bus/i2c/devices/i2c-5/5-0048/hwmon/hwmon*/temp1_input"
#define CHASSIS_THERMAL2      "/sys/bus/i2c/devices/i2c-5/5-0049/hwmon/hwmon*/temp1_input"
#define CHASSIS_THERMAL3      "/sys/bus/i2c/devices/i2c-6/6-0049/hwmon/hwmon*/temp1_input"
#define CHASSIS_THERMAL4      "/sys/bus/i2c/devices/i2c-6/6-004a/hwmon/hwmon*/temp1_input"
#define CPU_THERMAL           "/sys/bus/platform/drivers/coretemp/coretemp.0/hwmon/hwmon*/temp1_input"
#define PSU1_THERMAL1         "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/hwmon*/temp1_input"
#define PSU1_THERMAL2         "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/hwmon*/temp2_input"
#define PSU2_THERMAL1         "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/hwmon*/temp1_input"
#define PSU2_THERMAL2         "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/hwmon*/temp2_input"

#define CHASSIS_THERMAL1_ID  1  /* This related to above paths should match structure in thermali.c*/
#define CHASSIS_THERMAL2_ID  2
#define CHASSIS_THERMAL3_ID  3
#define CHASSIS_THERMAL4_ID  4
#define CPU_THERMAL_ID       5
#define PSU1_THERMAL1_ID     6
#define PSU1_THERMAL2_ID     7
#define PSU2_THERMAL1_ID     8
#define PSU2_THERMAL2_ID     9


#define NOT_DEFINE 345000
/* Warning */
#define CHASSIS_THERMAL1_HI_WARN  55000
#define CHASSIS_THERMAL2_HI_WARN  55000
#define CHASSIS_THERMAL3_HI_WARN  72000
#define CHASSIS_THERMAL4_HI_WARN  55000
#define CPU_THERMAL_HI_WARN       89000
#define PSU1_THERMAL1_HI_WARN     NOT_DEFINE
#define PSU1_THERMAL2_HI_WARN     NOT_DEFINE
#define PSU2_THERMAL1_HI_WARN     NOT_DEFINE
#define PSU2_THERMAL2_HI_WARN     NOT_DEFINE

#define CHASSIS_THERMAL1_LO_WARN  -5
#define CHASSIS_THERMAL2_LO_WARN  -5
#define CHASSIS_THERMAL3_LO_WARN  NOT_DEFINE  /* U60 has not lower warning */
#define CHASSIS_THERMAL4_LO_WARN  -5
#define CPU_THERMAL_LO_WARN       NOT_DEFINE
#define PSU1_THERMAL1_LO_WARN     NOT_DEFINE
#define PSU1_THERMAL2_LO_WARN     NOT_DEFINE
#define PSU2_THERMAL1_LO_WARN     NOT_DEFINE
#define PSU2_THERMAL2_LO_WARN     NOT_DEFINE

/* Error */
#define CHASSIS_THERMAL1_HI_ERR  NOT_DEFINE
#define CHASSIS_THERMAL2_HI_ERR  NOT_DEFINE
#define CHASSIS_THERMAL3_HI_ERR  75000
#define CHASSIS_THERMAL4_HI_ERR  NOT_DEFINE
#define CPU_THERMAL_HI_ERR       91000
#define PSU1_THERMAL1_HI_ERR     NOT_DEFINE
#define PSU1_THERMAL2_HI_ERR     NOT_DEFINE
#define PSU2_THERMAL1_HI_ERR     NOT_DEFINE
#define PSU2_THERMAL2_HI_ERR     NOT_DEFINE

/* Shutdown */
#define CHASSIS_THERMAL1_HI_SHUTDOWN  NOT_DEFINE
#define CHASSIS_THERMAL2_HI_SHUTDOWN  NOT_DEFINE
#define CHASSIS_THERMAL3_HI_SHUTDOWN  78000
#define CHASSIS_THERMAL4_HI_SHUTDOWN  NOT_DEFINE
#define CPU_THERMAL_HI_SHUTDOWN       93000
#define PSU1_THERMAL1_HI_SHUTDOWN     NOT_DEFINE
#define PSU1_THERMAL2_HI_SHUTDOWN     NOT_DEFINE
#define PSU2_THERMAL1_HI_SHUTDOWN     NOT_DEFINE
#define PSU2_THERMAL2_HI_SHUTDOWN     NOT_DEFINE


#define CHASSIS_THERMAL2_ID  2
#define CHASSIS_THERMAL3_ID  3
#define CHASSIS_THERMAL4_ID  4
#define CPU_THERMAL_ID       5
#define PSU1_THERMAL1_ID     6
#define PSU1_THERMAL2_ID     7
#define PSU2_THERMAL1_ID     8
#define PSU2_THERMAL2_ID     9

//ALARM
#define LED_ALARM_REGISTER 0xA144

//LED
#define LED_COUNT  7 
#define LED_SYSTEM_ID  1
#define LED_ALARM_ID   2
#define LED_PSU_ID     3
#define LED_FAN_ID    4
#define LED_FAN1_ID   5
#define LED_FAN2_ID   6 
#define LED_FAN3_ID   7 

#define LED_SYSTEM_REGISTER 0xA143
#define LED_SYSTEM_BOTH 0 
#define LED_SYSTEM_GREEN 1
#define LED_SYSTEM_YELLOW 2

#define LED_SYSTEM_OFF 3
#define LED_SYSTEM_4_HZ 2
#define LED_SYSTFAN_DIR_REGEM_1_HZ 1

#define LED_FAN_REGISTER 0xA133
#define LED_FAN_INTERVAL 4 /* 0xA133-0xA137-0xA13B */
#define LED_FAN1_REGISTER 0xA133
#define LED_FAN2_REGISTER 0xA137
#define LED_FAN3_REGISTER 0xA13B
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

//SFP
#define QSFP_COUNT 0
#define SFP_COUNT  8
#define SFP_BUS_START 9

//CPLD
#define  CPLD_FAN_DIR_REG 0xA188
#define  NUM_OF_CPLD 1

#if 0
/* will discard in the future*/
#define USE_SHM_METHOD 0
#define ONLP_SENSOR_CACHE_SHARED "/onlp-sensor-cache-shared"
#define ONLP_FRU_CACHE_SHARED "/onlp-fru-cache-shared"
#define ONLP_SENSOR_LIST_CACHE_SHARED "/onlp-sensor-list-cache-shared"

#define ONLP_SENSOR_CACHE_SEM "/onlp-sensor-cache-sem"
#define ONLP_FRU_CACHE_SEM "/onlp-fru-cache-sem"
#define ONLP_SENSOR_LIST_SEM "/onlp-sensor-list-cache-sem"

#define ONLP_SENSOR_CACHE_FILE "/tmp/onlp-sensor-cache.txt"
#define ONLP_FRU_CACHE_FILE "/tmp/onlp-fru-cache.txt"
#define ONLP_SENSOR_LIST_FILE "/tmp/onlp-sensor-list-cache.txt"
/* will discard in the future*/
#endif

struct shm_map_data{
    char data[16384]; 
    int size;
}; 

struct device_info{
    char serial_number[256];
    char model[256];
    int airflow;
};

struct fan_config_p{
    uint16_t pwm_reg;
    uint16_t ctrl_sta_reg;
    uint16_t rear_spd_reg;
    uint16_t front_spd_reg;
};

struct led_reg_mapper{
    char *name;
    uint16_t device;
    uint16_t dev_reg;
};

struct psu_reg_bit_mapper{
    uint16_t sta_reg;
    uint8_t bit_present;
    uint8_t bit_ac_sta;
    uint8_t bit_pow_sta;
};

struct search_psu_sdr_info_mapper{
    char* keyword;
    char unit;
};

typedef struct psuInfo_p
{
    unsigned int lvin;
    unsigned int liin;
    unsigned int lvout;
    unsigned int liout;
    unsigned int lpout;
    unsigned int lpin;
    unsigned int ltemp;
    
    unsigned int rvin;
    unsigned int riin;
    unsigned int rvout;
    unsigned int riout;
    unsigned int rpout;
    unsigned int rpin;
    unsigned int rtemp;
}psuInfo_p;

int get_sysnode_value(const char *path, void *dat);
int set_sysnode_value(const char *path, int data);
uint8_t get_led_status(int id);
int get_psu_model_sn(int id,char* model,char* serial_number);

int get_psu_info(int id,int *mvin,int *mvout,int *mpin,int *mpout,int *miin,int *miout);
char* trim (char *s);
int get_fan_info(int id,char* model,char* serial,int *get_fan_info);
int get_sensor_info(int id, int *temp, int *warn, int *error, int *shutdown);
int read_device_node_binary(char *filename, char *buffer, int buf_size, int data_len);
int read_device_node_string(char *filename, char *buffer, int buf_size, int data_len);
int get_fan_speed(int id,int* per,int* rpm);
uint8_t get_psu_status(int id);
// int dump_shared_memory(const char *shm_path, const char *sem_path, struct shm_map_data *shared_mem);
// int fill_shared_memory(const char *shm_path, const char *sem_path, const char *cache_path);
// int open_file(const char *shm_path, const char *sem_path, char **cache_data, int *cache_size);
// int create_cache();
// void update_shm_mem(void);
// int is_cache_exist();

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)   printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...)
#endif

#endif /* _PLATFORM_BELGITE_H_ */
