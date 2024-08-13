#ifndef _PLATFORM_COMMON_H_
#define _PLATFORM_COMMON_H_
#include <stdint.h>
#include <pthread.h>
#define PREFIX_PATH_LEN       100

//DEVICE NUMBERS
#define FAN_COUNT             9
#define CHASSIS_FAN_COUNT     7
#define PSU_COUNT             2
#define THERMAL_COUNT         24
#define CHASSIS_THERMAL_COUNT 18
#define POWER_COUNT           40
#define LED_COUNT             4
#define OSFP_COUNT            32
#define SFP_COUNT             2

//DEVICE REGISTERS
#define LED_PSU_REG            0x61
#define LPC_LED_PSU_REG        0xA161
#define LED_PSU_AMB            0x01
#define LED_PSU_GRN            0x02

#define LED_ALARM_REG          0x63
#define LPC_LED_ALARM_REG      0xA163
#define LED_ALARM_AMB          0x20
#define LED_ALARM_GRN          0x10
#define LED_ALARM_AMB_1HZ      0x21
#define LED_ALARM_AMB_4HZ      0x22

#define LED_FAN_REG            0x65
#define LPC_LED_FAN_REG        0xA165
#define LED_FAN_AMB            0x01
#define LED_FAN_GRN            0x02
#define LED_FAN_OFF            0x03

#define LED_SYSTEM_REG         0x62

#define PSU_STA_REG            0x60
#define BMC_STATUS_REG         0x08

#define LPC_COME_BTNCTRL_REG   0xA124
#define LPC_SWITCH_PWCTRL_REG  0xA126
#define SWITCH_PWCTRL_REG      0x26

#define COME_BTNOFF_HOLD       0x00
#define COME_BTNOFF_RELEASE    0x01
#define COME_BTNOFF_HOLD_SEC   8

#define SWITCH_OFF             0x01
#define SWITCH_ON              0x10


//OFFSET
#define OSFP_I2C_START        15
#define SFP_I2C_START         2

//PSU and chassis FANs maximum rpm
#define PSU_FAN_MAX           31000
#define CHASSIS_FAN_FRONT_MAX 32000
#define CHASSIS_FAN_REAR_MAX  28000

/* fan speed tolerant scope(duty), exclusive
 * duty should regard as fan fault
 */
#define CHASIS_FAN_TOLERANT_PER 25

#define FAN_F2B               1
#define FAN_B2F               0
#define ABSENT                1
#define PRESENT               0
#define ERR                   -1
#define OK                    0
#define FAULT                 2

#define SYS_CPLD_PATH               "/sys/devices/platform/sys_cpld/"
#define SYS_FPGA_PATH               "/sys/devices/platform/fpga-sys/"
#define PLATFORM_SFP_PATH           "/sys/devices/platform/fpga-xcvr"
#define PLATFORM_OSFP_PATH          "/sys/class/SFF"
#define I2C_DEVICE_PATH             "/sys/bus/i2c/devices/"
#define PREFIX_PATH_ON_SYS_EEPROM   "/sys/bus/i2c/devices/i2c-0/0-0056/eeprom"

#define PSU_F2B_AIRFLOW_FIELD   "TDPS2000LB A "
#define PSU_B2F_AIRFLOW_FIELD   "DPS-1300AB-36 "

//COMMON USE
#define NELEMS(x)             (sizeof(x) / sizeof((x)[0]))
#define DEBUG_MODE 0

#if (DEBUG_MODE==1)
#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

enum led_id {
    LED_RESERVED = 0,
    LED_SYSTEM,
    LED_ALARM,
    LED_PSU,
    LED_FAN
};

enum psu_id {
    PSUL_ID = 1,
    PSUR_ID,
};

enum thermal_id {
    TEMP_CPU_ID = 1,
    TEMP_DIMMA0_ID,
    TEMP_DIMMB0_ID,
    TEMP_SW_Internal_ID,
    BMC_Temp_ID,
    TEMP_BB_U3_ID,
    TEMP_SW_U15_ID,
    TEMP_SW_U17_ID,
    TEMP_SW_U16_ID,
    TEMP_SW_U13_ID,
    TEMP_SW_U11_ID,
    TEMP_SW_U12_ID,
    TEMP_FCB_U52_ID,
    TEMP_FCB_U17_ID,
    MP5023_T_ID,
    VDD_CORE_T_ID,
    XP3R3V_LEFT_T_ID,
    XP3R3V_RIGHT_T_ID,
    PSU1_TEMP1_ID,
    PSU1_TEMP2_ID,
    PSU1_TEMP3_ID,
    PSU2_TEMP1_ID,
    PSU2_TEMP2_ID,
    PSU2_TEMP3_ID,
};

enum fan_id {
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


struct led_reg_mapper{
    char *name;
    uint16_t device;
    uint16_t dev_reg;
};

struct psu_reg_bit_mapper{
    uint16_t sta_reg;
    uint8_t bit_present;
    uint8_t bit_alert;
    uint8_t bit_ac_sta;
    uint8_t bit_pow_sta;
};

int read_device_node_binary(char *filename, char *buffer, int buf_size, int data_len);
int read_device_node_string(char *filename, char *buffer, int buf_size, int data_len);
int set_sysnode_value(const char *path, int data);
int get_sysnode_value(const char *path, void *data);
int syscpld_setting(uint32_t reg, uint8_t val);
int read_eeprom(char* path, int offset, char* data, unsigned int *count);
int read_smbus_block(int bus, int address, int bank, unsigned char *cblock);
int exec_cmd(char *cmd, char *retd);

uint8_t read_register(uint16_t dev_reg);
char* trim (char *s);
void array_trim(char *strIn, char *strOut);

uint8_t get_led_status(int id);
int get_psu_model_sn(int id, char* model,char* serial_number);
int get_psu_info(int id,int *mvin,int *mvout,int *mpin,int *mpout,int *miin,int *miout);

int get_fan_info(int id,char* model,char* serial,int *get_fan_info);
int get_sensor_info(int id, int *temp, int *warn, int *error, int *shutdown);
int get_fan_speed(int id,int* per,int* rpm);
uint8_t get_psu_status(int id);


#endif /* _PLATFORM_COMMON_H_ */
