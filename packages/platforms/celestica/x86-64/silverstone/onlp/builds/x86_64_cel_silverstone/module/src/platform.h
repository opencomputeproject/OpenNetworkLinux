#ifndef _PLATFORM_SEASTONE_H_
#define _PLATFORM_SEASTONE_H_
#include <stdint.h>

#define PREFIX_PATH_LEN 100

//FAN
#define FAN_COUNT   9
#define CHASSIS_FAN_COUNT 7

//PSU
#define PSU_COUNT 2
#define PSU_STA_REGISTER 0xA160
#define PSU_LED_REGISTER 0xA161

//THERMAL
#define THERMAL_COUNT 13
#define THERMAL_REGISTER 0xA176
#define CHASSIS_THERMAL_COUNT 9

//ALARM
#define ALARM_REGISTER 0xA163

//LED
#define LED_COUNT   4

#define LED_SYSTEM_H  1
#define LED_SYSTEM_REGISTER 0xA162
#define LED_SYSTEM_BOTH 3
#define LED_SYSTEM_GREEN 1
#define LED_SYSTEM_YELLOW 2
#define LED_SYSTEM_OFF 3
#define LED_SYSTEM_4_HZ 2
#define LED_SYSTEM_1_HZ 1

#define LED_FAN_H   4
#define LED_FAN_REGISTER 0xA165
#define LED_ALARM_H   2
#define LED_PSU_H   3
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

#define ONLP_SENSOR_CACHE_SHARED "/onlp-sensor-cache-shared"
#define ONLP_FRU_CACHE_SHARED "/onlp-fru-cache-shared"
#define ONLP_SENSOR_LIST_CACHE_SHARED "/onlp-sensor-list-cache-shared"

#define ONLP_SENSOR_CACHE_SEM "/onlp-sensor-cache-sem"
#define ONLP_FRU_CACHE_SEM "/onlp-fru-cache-sem"
#define ONLP_SENSOR_LIST_SEM "/onlp-sensor-list-cache-sem"

#define ONLP_SENSOR_CACHE_FILE "/tmp/onlp-sensor-cache.txt"
#define ONLP_FRU_CACHE_FILE "/tmp/onlp-fru-cache.txt"
#define ONLP_SENSOR_LIST_FILE "/tmp/onlp-sensor-list-cache.txt"

#define PSUL_ID 1
#define PSUR_ID 2

#define NUM_OF_CPLD 1

#define USE_SHM_METHOD 0

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

#define SYS_CPLD_PATH "/sys/devices/platform/sys_cpld/"
#define PLATFORM_PATH "/sys/devices/platform/cls-xcvr/"
#define I2C_DEVICE_PATH "/sys/bus/i2c/devices/"
#define PREFIX_PATH_ON_SYS_EEPROM "/sys/bus/i2c/devices/i2c-0/0-0056/eeprom"

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
int dump_shared_memory(const char *shm_path, const char *sem_path, struct shm_map_data *shared_mem);
int fill_shared_memory(const char *shm_path, const char *sem_path, const char *cache_path);
int open_file(const char *shm_path, const char *sem_path, char **cache_data, int *cache_size);
int create_cache();
void update_shm_mem(void);
int is_cache_exist();

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)   printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...)
#endif

#endif /* _PLATFORM_SEASTONE_H_ */
