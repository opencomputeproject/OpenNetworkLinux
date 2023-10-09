#ifndef _PLATFORM_SILVERSTONE_V2_H_
#define _PLATFORM_SILVERSTONE_V2_H_
#include <stdint.h>

#define PREFIX_PATH_LEN 100
#define BMC_STATE   0x08
#define BMC_PRESENT 0
#define BMC_ABSENT 1

#define ERR            -1
#define OK               0
#define FAULT            2


//FAN
#define FAN_COUNT   9
#define CHASSIS_FAN_COUNT 7
#define PSU_FAN_COUNT 2

#define CHASSIS_FAN_MAX_RPM   40000

//ledcontrol
#define CHASSIS_REAR_FAN_MAX_RPM_F2B   30200
#define CHASSIS_FRONT_FAN_MAX_RPM_F2B   32000
#define CHASSIS_REAR_FAN_MAX_RPM_B2F   28000
#define CHASSIS_FRONT_FAN_MAX_RPM_B2F   32000


/* fan speed tolerant scope(duty), exclusive 
 * duty should regard as fan fault
 */
#define CHASIS_FAN_TOLERANT_PER 25

#define PSU_FAN_MAX_RPM   33400 

//THERMAL
#define THERMAL_COUNT 18
#define THERMAL_REGISTER 0xA176
#define CHASSIS_THERMAL_COUNT 12
#define LPC_SWITCH_PWCTRL_REG  0xA126
#define SWITCH_OFF             0x01
#define SWITCH_ON           0x00


//PSU
#define PSU_COUNT 2
#define PSU_STA_REGISTER       0x60
#define PSU_LED_REGISTER       0x61
#define LPC_LED_PSU_REG        0xA161
#define LED_PSU_GRN            0x02
#define LED_PSU_AMB            0x01

#define LED_ALARM_REG          0x63

#define LED_FAN_REG            0x65
#define LPC_LED_FAN_REG        0xA165
#define LED_FAN_GRN            0x02
#define LED_FAN_AMB            0x01
#define LED_FAN_OFF            0x03


#define QSFP_COUNT 32
#define SFP_COUNT 2

//LED
#define LED_COUNT 11
enum led_id {
    LED_SYSTEM_ID = 1,
    LED_ALARM_ID,
    LED_PSU_ID,
    LED_FAN_ID,
    LED_FAN1_ID,
    LED_FAN2_ID,
    LED_FAN3_ID,
    LED_FAN4_ID,
    LED_FAN5_ID,
    LED_FAN6_ID, 
    LED_FAN7_ID,
};
#define LED_ALARM_REGISTER 0x63
#define LED_PSU_REGISTER 0x61
#define LED_FAN1_REGISTER 0x24
#define LED_FAN2_REGISTER 0x34
#define LED_FAN3_REGISTER 0x44
#define LED_FAN4_REGISTER 0x54
#define LED_FAN5_REGISTER 0x64
#define LED_FAN6_REGISTER 0x74
#define LED_FAN7_REGISTER 0x84

//Thermal
enum thermal_id{
	TEMP_SW_U52_ID = 1,
	TEMP_SW_U16_ID,
	TEMP_FB_U52_ID,
	TEMP_FB_U17_ID,
	VDD_CORE_Temp_ID,
	XP0R8V_Temp_ID,
	XP3R3V_R_Temp_ID,
	XP3R3V_L_Temp_ID,
	TEMP_SW_Internal_ID,
	CPU_THERMAL_ID,
	TEMP_DIMMA0_ID,
	TEMP_DIMMB0_ID,
	PSU1_THERMAL1_ID,
	PSU1_THERMAL2_ID,
	PSU1_THERMAL3_ID,
	PSU2_THERMAL1_ID,
	PSU2_THERMAL2_ID,
	PSU2_THERMAL3_ID,
};
	


#define LED_SYSTEM_H  1
#define LED_SYSTEM_REGISTER 0x62
#define LED_SYSTEM_BOTH 0
#define LED_SYSTEM_GREEN 1
#define LED_SYSTEM_YELLOW 2
#define LED_SYSTEM_OFF 3
#define LED_SYSTEM_4_HZ 2
#define LED_SYSTEM_1_HZ 1

#define LED_FAN_H   4
#define LED_FAN_REGISTER 0x65
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

enum psu_id {
    PSU1_ID = 1, 
    PSU2_ID,
};
	
enum fan_id{
    CHASSIS_FAN1_ID = 1,   
    CHASSIS_FAN2_ID,
    CHASSIS_FAN3_ID,
    CHASSIS_FAN4_ID,   
    CHASSIS_FAN5_ID,
    CHASSIS_FAN6_ID,
    CHASSIS_FAN7_ID,
    PSU1_FAN_ID,
    PSU2_FAN_ID,
};


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
#define PLATFORM_SFP_PATH "/sys/devices/platform/fpga-xcvr"
#define PLATFORM_QSFP_PATH "/sys/class/SFF"
#define I2C_DEVICE_PATH "/sys/bus/i2c/devices/"
#define SYS_FPGA_PATH "/sys/devices/platform/fpga-sys/"
#define PREFIX_PATH_ON_SYS_EEPROM "/sys/bus/i2c/devices/i2c-0/0-0056/eeprom"

uint8_t get_led_status(int id);
int get_psu_model_sn(int id,char* model,char* serial_number);
uint8_t read_register(uint16_t dev_reg,char *path);
int get_psu_info(int id,int *mvin,int *mvout,int *mpin,int *mpout,int *miin,int *miout);
char* trim (char *s);
int get_fan_info(int id,char* model,char* serial,int *isfanb2f);
int get_sensor_info(int id, int *temp, int *warn, int *error, int *shutdown);
int read_device_node_binary(char *filename, char *buffer, int buf_size, int data_len);
int read_device_node_string(char *filename, char *buffer, int buf_size, int data_len);
int get_fan_speed(int id,int* per,int* rpm);
uint8_t get_psu_status(int id);
int create_cache();
void update_shm_mem(void);
int is_cache_exist();
void array_trim(char *strIn, char *strOut);
uint16_t check_bmc_status(void);
int syscpld_setting(uint32_t reg, uint8_t val);



//BMC
uint8_t read_fan_led_wb(uint16_t dev_reg);
int get_psu_model_sn_wb(int id,char* model,char* serial_number);
int get_psu_info_wb(int id,int *mvin,int *mvout,int *mpin,int *mpout,int *miin,int *miout);
int get_fan_info_wb(int id,char* model,char* serial,int *isfanb2f);
int get_sensor_info_wb(int id, int *temp, int *warn, int *error, int *shutdown);
int get_fan_speed_wb(int id,int* per,int* rpm);
int dump_shared_memory(const char *shm_path, const char *sem_path, struct shm_map_data *shared_mem);
int fill_shared_memory(const char *shm_path, const char *sem_path, const char *cache_path);
int open_file(const char *shm_path, const char *sem_path, char **cache_data, int *cache_size);


//nonbmc

int get_sysnode_value(const char *path, void *dat);
int set_sysnode_value(const char *path, int data);
uint8_t read_fan_led_nb(uint16_t dev_reg);
int get_psu_model_sn_nb(int id,char* model,char* serial_number);
int get_psu_info_nb(int id,int *mvin,int *mvout,int *mpin,int *mpout,int *miin,int *miout);
int get_fan_info_nb(int id,char* model,char* serial,int *isfanb2f);
int get_sensor_info_nb(int id, int *temp, int *warn, int *error, int *shutdown);
int get_fan_speed_nb(int id,int* per,int* rpm);





#define DEBUG_MODE 0

#if DEBUG_MODE
	#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
	#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif


#endif /* _PLATFORM_SEASTONE_H_ */
