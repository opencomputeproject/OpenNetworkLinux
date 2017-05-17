#ifndef I2C_CHIPS_HH
#define I2C_CHIPS_HH

/* PSU registers */
#define PSU_STATUS_VOUT		0x7a
#define	PSU_STATUS_IOUT		0x7b
#define PSU_STATUS_TEMP		0x7d

#define PSU_READ_VIN		0x88
#define	PSU_READ_IIN		0x89
#define PSU_READ_VOUT		0x8b
#define	PSU_READ_IOUT		0x8c
#define PSU_READ_TEMP_1		0x8d
#define PSU_READ_POUT		0x96
#define PSU_READ_PIN		0x97
#define PSU_PWM_REG 	    0x3b
#define PSU_SPEED_REG	    0x90

#define PSU_MFR_ID			0x99	// 7 byte
#define PSU_MFR_MODEL		0x9a	// 14 byte
#define PSU_MFR_REVISION	0x9b	// 5 byte
#define PSU_MFR_LOCATION	0x9c
#define PSU_MFR_DATE		0x9d
#define PSU_MFR_SERIAL		0x9e
#define PCA9506_IP0_REG		0x00
#define PCA9506_IP1_REG		0x01
#define PCA9506_IP2_REG		0x02
#define PCA9506_IP3_REG		0x03
#define PCA9506_IP4_REG		0x04

#define PCA9506_OP0_REG		0x08
#define PCA9506_OP1_REG		0x09
#define PCA9506_OP2_REG		0x0A
#define PCA9506_OP3_REG		0x0B
#define PCA9506_OP4_REG		0x0C

#define PCA9506_PI0_REG		0x10
#define PCA9506_PI1_REG		0x11
#define PCA9506_PI2_REG		0x12
#define PCA9506_PI3_REG		0x13
#define PCA9506_PI4_REG		0x14

#define PCA9506_IOC0_REG	0x18
#define PCA9506_IOC1_REG	0x19
#define PCA9506_IOC2_REG	0x1A
#define PCA9506_IOC3_REG	0x1B
#define PCA9506_IOC4_REG	0x1C

#define PCA9506_MSK0_REG	0x20
#define PCA9506_MSK1_REG	0x21
#define PCA9506_MSK2_REG	0x22
#define PCA9506_MSK3_REG	0x23
#define PCA9506_MSK4_REG	0x24

#define PCA9506_IO_INPUT	1
#define PCA9506_IO_OUTPUT	0

#define PCA9506_IO_LOW		0
#define PCA9506_IO_HIGH		1
#define PCA9506_ADDR		0x20



struct psuInfo
{
    unsigned int vin;
	unsigned int iin;
	unsigned int vout;
	unsigned int iout;
	unsigned int pout;
	unsigned int pin;
	unsigned int temp;
};

struct chips_info
{
	char addr;
	char sw_addr;
	int channel;
	char *name;
};

struct dev_info
{
	unsigned char dev_id;
	char sw_addr;
	int channel;
	char *name;
};

struct fan_config
{
	char emc_addr;
	char speed_reg;
	char driver_reg;
};
struct fan_cpld_reg
{
	unsigned short hight_byte_reg;
	unsigned short low_byte_reg;
};


struct ts_info
{
	char ts_addr;
	char temp_reg;
	char os_reg;
};

typedef enum
{
  gpio_in = 0,
  gpio_out = 1
} gpio_dir;

typedef enum
{
  F2B= 0,
  B2F = 1
} fan_airflow;
typedef enum
{
  green = 0,
  red = 1,
  yellow = 1
} gpio_color;
struct led_gpio
{
	int green_gpio;
	int red_gpio;
};


int getCtrlOfBus(void);
int enableChip(char addr);
int disableChip(char addr);

int fanSpeedGet(int id, int *speed);
int fanSpeedSet(int id, unsigned short speed);
int fanPwmGet(int id, int *pwm);

int tsTempGet(int id, short *temp);
int tsOsGet(int id, unsigned short *temp);
int getTsShutdown(int id);
int setTsShutdown(int id, int shutdown);
int getPsuInfo(int id, struct psuInfo * info);
int getWdFromCpldRam(unsigned char *val);
int getRxpSxpFlag(unsigned char *val);
//int getPsuPresStatus(int id);
int getPSoCVersion(unsigned char *val);
int getTempStatus(unsigned char *val);
unsigned char getCpuId();
int openChannel(unsigned char dev_id);
int closeChannel(unsigned char dev_id);
int eeprom_enable(unsigned char dev_id);
int eeprom_disable(unsigned char dev_id);
int pca9506_init_pin(unsigned char port, gpio_dir val);
int pca9506_write_pin(unsigned char port, unsigned char val);
int pca9506_read_pin(unsigned char port);

int fanInit();
int getFanPresent(int id);
int getFanAirflow(int id);
int getFanStatus(int id);

int setFanLedGreen(int id);
int setFanLedRed(int id);
int setFanLedYellow(int id);
int setFanLedOff(int id);
int setPsuLedOn(int id);
int setPsuLedOff(int id);
int setSysLedOn();
int setSysLedOff();


void watchdog_disable();
/* watchdog timer must less than 1.2s */
void watchdog_enable();
void watchdog_service();



int read_psu(unsigned char addr, unsigned char reg, unsigned short *value);
int write_psu(unsigned char addr, unsigned char reg, unsigned short value);
int setPsuLedOff(int id);
int setPsuLedOn(int id);
int getPsuPresent(int id);

int biosflash_enable();
int biosflash_disable();



struct i2cMutex {
	int init_flag;
	pthread_mutex_t mutex;
	pthread_cond_t  lock_free;
	volatile int is_locking;
};

#define NUM_CHIPS 10
static const struct chips_info i2c_chips[NUM_CHIPS] = {
	{0x50, 0x73, 2, "eeprom"},
	{0x4D, 0x73, 3, "emc2305_1"},
	{0x2E, 0x73, 3, "emc2305_2"},
	{0x58, 0x73, 0, "PSU_L"},
	{0x59, 0x73, 1, "PSU_R"},
	{0x48, 0x73, 4, "lm75_cpu"},
	{0x4E, 0x73, 5, "lm75_out"},
	{0x49, 0x71, 4, "lm75_in"},
    {0x4A, 0x71, 5, "lm75_sw"},
	{0x20, 0x73, 7, "pca9506"},
};


#ifdef L7_MAX_FANS_PER_UNIT
#define FAN_NUM L7_MAX_FANS_PER_UNIT
#else
#define FAN_NUM	8
#endif

#define FANGROUP_NUM	4

#endif
