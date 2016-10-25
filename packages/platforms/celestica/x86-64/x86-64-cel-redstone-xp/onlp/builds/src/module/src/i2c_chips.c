#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/io.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>


#include "cpld.h"
#include "i2c_dev.h"
#include "i2c_chips.h"

//#define PSOC_TAKE_I2C
#define PSOC_CTRL_SMBUS 0x01

static struct ts_info ts[] = {
	{0x48, 0x0, 0x3},
	{0x4E, 0x0, 0x3},
	{0x49, 0x0, 0x3},
	{0x4A, 0x0, 0x3},
};

static const struct dev_info i2c_dev[NUM_CHIPS] = {
	{1,0x73, 4, "lm75_cpu"},
	{2,0x73, 5, "lm75_out"},
	{3,0x71, 4, "lm75_in"},
	{4,0x71, 5, "lm75_sw"},
	{5, 0x73, 2, "syseeprom"},
	{6, 0x71, 2, "gbeeeprom"},
	{9, 0x73, 7, "pca9506"},
};

#define FANGROUP_NUM	4
static const struct fan_config fan[FAN_NUM] = {
	{0x4D, 0x4E, 0x40},//Fan 1  Front
	{0x2E, 0x4E, 0x40},//Fan 1  Rear
	{0x4D, 0x6E, 0x60},//Fan 2 F
	{0x2e, 0x3E, 0x30},//Fan 2 R
	{0x4D, 0x5E, 0x50},//Fan 3 F
	{0x2E, 0x7E, 0x70},//Fan 3 R
	{0x4D, 0x3E, 0x30},//Fan 4 F
	{0x2E, 0x5E, 0x50},//Fan 4 R
};

static struct led_gpio ledGpio[] = {
	{32, 31},//4_0,3_7, gpio(x_y) = x*8+y
	{30, 29},//3_6,3_5
	/*{36, 35},//4_4,4_3 For smallstone*/
	{38, 37},//4_6,4_5
	{34, 33},//4_2,4_1
};
static unsigned char airFlowGpio[] = {
	16,//IO2_0  gpio = 2*8+0, gpio(x_y) = x*8+y
	15,//IO1_7
	/*18,//IO2_2 //For smallstone */
	19,//IO2_3
	17,//IO2_1
};
static unsigned char presentGpio[] = {
	11,//IO1_3  gpio = 2*8+0, gpio(x_y) = x*8+y
	10,//IO1_2
	/* 13,//IO1_5 //For smallstone */
	14,//IO1_6
	12,//IO1_4
};

static int has_been_read = 0;
static unsigned char cpu_id = 0x00;

unsigned long in_sus_val = 0x00000000;
unsigned long in_core_val = 0x00000000;

int openChannel(unsigned char dev_id)
{
	unsigned char data;
	int ret = -1;
	int i;
	for(i=0; i< NUM_CHIPS; i++)
	{
		if(dev_id ==  i2c_dev[i].dev_id)
		{
			data = 0x0 | (0x1 << i2c_dev[i].channel) ;
			ret = chips_write_byte(i2c_dev[i].sw_addr, 0x0, data);
			if(ret < 0)
				return ret;
		}
	}
	return ret;
}

int closeChannel(unsigned char dev_id)
{
	int ret = -1;
	int i;
	for(i=0; i< NUM_CHIPS; i++)
	{
		if(dev_id ==  i2c_dev[i].dev_id)
		{
			ret = chips_write_byte(i2c_dev[i].sw_addr, 0x0, 0x0);
			if(ret < 0)
				return ret;
		}
	}
	return ret;
}


/* GPIO INPUT:	return zero or nonzero */
void gpio_sus_init(unsigned int gpio)
{
	outl_p((inl_p(0x580) | (1 << gpio)), 0x580);//100 0000 0001 0011 0000
}

unsigned char gpio_sus_get_value(unsigned int gpio)
{
	unsigned char ucRet = 0;

	if ( inl_p( 0x588 ) & (1 << gpio) ) {
		ucRet = 0x01;
	}
	else {
		ucRet = 0x00;
	}

	return ( ucRet );

}
void gpio_sus_set_value(unsigned int gpio, unsigned char val)
{
	if(val)
		in_sus_val |= (1 << gpio);
	else
		in_sus_val &= ~(1 << gpio);

	outl_p( in_sus_val, 0x588 );
}


/****************************************************************
 * gpio_set_dir
 ****************************************************************/
void gpio_sus_set_dir(unsigned int gpio, unsigned int out_flag)
{

	if (out_flag)
		outl_p((inl_p(0x584) & (~(1 << gpio))), 0x584);//output set 0
	else
		outl_p((inl_p(0x584) | (1 << gpio)), 0x584);//input set 1
}

void gpio_core_init(unsigned int gpio)
{
	outl_p((inl_p(0x500) | (1 << gpio)), 0x500);//100 0000 0001 0011 0000
}

unsigned char gpio_core_get_value(unsigned int gpio)
{
	unsigned char ucRet = 0;

	if ( inl_p( 0x508 ) & (1 << gpio) ) {
		ucRet = 0x01;
	}
	else {
		ucRet = 0x00;
	}

	return ( ucRet );

}
void gpio_core_set_value(unsigned int gpio, unsigned char val)
{

    inl_p(0x508);
	if(val)
		in_core_val |= (1 << gpio);
	else
		in_core_val &= ~(1 << gpio);

	outl_p( in_core_val, 0x508 );
}


/****************************************************************
 * gpio_set_dir
 ****************************************************************/
void gpio_core_set_dir(unsigned int gpio, unsigned int out_flag)
{

	if (out_flag)
		outl_p((inl_p(0x504) & (~(1 << gpio))), 0x504);//output set 0
	else
		outl_p((inl_p(0x504) | (1 << gpio)), 0x504);//input set 1
}


#define setbit(x,y) x|=(1<<y)
#define clrbit(x,y) x&=~(1<<y)

#define WDO_MASK	0x04

/*delay in ms */
void Sleep(int ms)
{
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = ms * 1000; // 20 ms
    select(0, NULL, NULL, NULL, &delay);
}

void wdo_kick()
{
	/*kick watchdog*/
	if (iopl(3))
	{
		perror("iopl");
		exit(1);/* reminder here: do not use "return", I warned */
	}
	else
	{
		gpio_core_set_value(15,0);
		Sleep(10);
		gpio_core_set_value(15,1);
	}
	if (iopl(0))
	{
		perror("iopl");
		exit(1);/* reminder here: do not use "return", I warned */
	}

}

int wdo_enable(int value)
{
	unsigned char ret;
	unsigned char buf = 0x00;

	if (value == 0 ) {
		ret = read_cpld(CPLD_RESET_CONTROL, &buf);
		if(ret < 0)
			return ret;

        buf &= ~WDO_MASK;

        ret = write_cpld(CPLD_RESET_CONTROL, buf);
		if(ret < 0)
			return ret;
	} else {
		if (iopl(3))
		{
			perror("iopl");
			exit(1);/* reminder here: do not use "return", I warned */
		}
		else
		{
			gpio_core_init(15);
			gpio_core_set_dir(15,gpio_out);
		}
		if (iopl(0))
		{
			perror("iopl");
			exit(1);/* reminder here: do not use "return", I warned */
		}

		ret = read_cpld(CPLD_RESET_CONTROL, &buf);
		if(ret < 0)
			return ret;
		wdo_kick();

		buf |= WDO_MASK;
		ret = write_cpld(CPLD_RESET_CONTROL, buf);
		if(ret < 0)
			return ret;
	}
	return ret;
}

void watchdog_disable()
{
    wdo_enable(0);
}

// watchdog timer must less than 1.2s
void watchdog_enable()
{
    wdo_enable(1);
}

void watchdog_service()
{
	wdo_kick();
}

unsigned char getCpuId()
{
#ifdef PSOC_TAKE_I2C
	return PSOC_CTRL_SMBUS;
#endif

	if (iopl(3))
	{
		perror("iopl");
		exit(1);/* reminder here: do not use "return", I warned */
	}
	else
	{
		gpio_sus_init(19);
		gpio_sus_set_dir(19,gpio_in);
	}
	/*read cpu id*/
	cpu_id |= gpio_sus_get_value(19) << 0 ;
	if (iopl(0))
	{
		perror("iopl");
		exit(1);/* reminder here: do not use "return", I warned */
	}
	has_been_read = 1;

	return cpu_id;
}
int enableChip(char addr)
{
	char data;
	int ret = -1;
	int i;

	for(i=0; i< NUM_CHIPS; i++)
	{
		if(addr ==  i2c_chips[i].addr)
		{
			data = 0x0 | (0x1 << i2c_chips[i].channel) ;
			ret = chips_write_byte(i2c_chips[i].sw_addr, 0x0, data);
			if(ret < 0)
				return ret;
		}
	}
	return ret;
}

int disableChip(char addr)
{
	int ret = -1;
	int i;
	for(i=0; i< NUM_CHIPS; i++)
	{
		if(addr ==  i2c_chips[i].addr)
		{
			ret = chips_write_byte(i2c_chips[i].sw_addr, 0x0, 0x0);
			if(ret < 0)
				return ret;
		}
	}
	return ret;
}

#define OTHER_MAST_LOCK_MASK 0x01
#define OTHER_MAST_LOCK_MASK 0x01
#define I2C_MASTER_SELECTOR_DEV_ADDR 0x70
#define I2C_MASTER_SELECTOR_DEV_SR   0x02
#define I2C_MASTER_SELECTOR_DEV_CR   0x01
#define I2C_MASTER_SELECTOR_DEV_ISR  0x04
#define I2C_MASTER_SELECTOR_DEV_IMR  0x05
#define ENABLE_LOCK_GRANT   0xFB
#define CLEARE_LOCK_GRANT   0xFF
#define REQUEST_DOWNSTREAM  0x01
#define CONNECT_DOWNSTREAM  0x05
#define GIVEUP_DOWNSTREAM   0x00
int getCtrlOfBus_9641(void)
{
    int errStatus = -1;
    unsigned char data;
    errStatus = chips_write_byte(I2C_MASTER_SELECTOR_DEV_ADDR,
                                    I2C_MASTER_SELECTOR_DEV_IMR,
                                    ENABLE_LOCK_GRANT);

    if (errStatus < 0)
        return errStatus;

    errStatus = chips_write_byte(I2C_MASTER_SELECTOR_DEV_ADDR,
                                I2C_MASTER_SELECTOR_DEV_CR,
                                REQUEST_DOWNSTREAM);
    if (errStatus < 0)
        return errStatus;


    errStatus = chips_read_byte(I2C_MASTER_SELECTOR_DEV_ADDR,
                              I2C_MASTER_SELECTOR_DEV_CR,
                              &data);
    if (data == 0x03)
    {
        errStatus = chips_write_byte(I2C_MASTER_SELECTOR_DEV_ADDR,
                                I2C_MASTER_SELECTOR_DEV_CR,
                                CONNECT_DOWNSTREAM);
    }

    return errStatus;
}

static unsigned char Get_Control_Bus_Mask(unsigned char data)
{
    unsigned char Mask = 0xff;
    switch(data & 0x0f)
    {
        case 0x09:
        case 0x0c:
        case 0x0d:
            Mask = 0x00;
            break;

        case 0x0A:
        case 0x0E:
        case 0x0F:
            Mask = 0x01;
            break;

        case 0x00:
        case 0x01:
        case 0x05:
            Mask = 0x04;
            break;

        case 0x02:
        case 0x03:
        case 0x06:
            Mask = 0x05;
            break;
        default:
            Mask = 0xff;
            break;
    }

    return Mask;
}

static int getCtrlOfBus_9541(void)
{
    int errStatus = 0;
    unsigned char data;
	unsigned char mask;

    errStatus = chips_read_byte(I2C_MASTER_SELECTOR_DEV_ADDR,
                              I2C_MASTER_SELECTOR_DEV_CR,
                              &data);

    mask = Get_Control_Bus_Mask(data);

    if (mask == 0xff)
        return 0;

    errStatus = chips_write_byte(I2C_MASTER_SELECTOR_DEV_ADDR,
                                    I2C_MASTER_SELECTOR_DEV_CR,
                                    mask);

    return errStatus;
}

int getCtrlOfBus(void)
{
	return getCtrlOfBus_9541();
}

static const struct fan_cpld_reg fan_cpld_reg[FAN_NUM] = {
	{0x180, 0x181},
	{0x182, 0x183},
	{0x184, 0x185},
	{0x186, 0x187},
	{0x188, 0x189},
	{0x18A, 0x18B},
	{0x18C, 0x18D},
	{0x18E, 0x18F},
};

static short swap(short data)
{
	data = ((data & 0x00ff) << 8) |
		((data & 0xff00) >> 8);
	return data;
}

/*added by Gerald*/
#define CPLD_RAM_ADDR_HIGH_BYTE_REG (0x130)
#define CPLD_RAM_ADDR_LOW_BYTE_REG 	(0x131)
#define CPLD_RAM_READ_REG 			(0x132)

#define FAN1_RAM_SPEED_LOW          (20)
#define FAN_PRES_STATUS             (15)
#define FRONT_FAN_STATUS            (FAN_PRES_STATUS + 1)
#define REAR_FAN_STATUS             (FAN_PRES_STATUS + 2)

#define PSU_PRES_STATUS				(50)

int read_ram_from_cpld(const unsigned short ram_offset, unsigned char *value)
{
	int ret;
	unsigned char ram_hbyte, ram_lbyte;
	unsigned char data;

	/* set RAM offset high byte first as described in spec */
	ram_hbyte = (ram_offset & 0xff00) >> 8 ;
	ret = write_cpld(CPLD_RAM_ADDR_HIGH_BYTE_REG, ram_hbyte);
	if (ret < 0)
	{
			printf("write ram offset high byte error\n");
			return ret;
	}

	/* set RAM offset low byte */
	ram_lbyte = ram_offset & 0xff;
	ret = write_cpld(CPLD_RAM_ADDR_LOW_BYTE_REG, ram_lbyte);
	if (ret < 0)
	{
			printf("write ram offset low byte error\n");
			return ret;
	}

	ret = read_cpld(CPLD_RAM_READ_REG, &data);
	if(ret >= 0)
		*value = data;
	return ret;
}

int read_ram_short_from_cpld(const unsigned short ram_offset, unsigned short *value)
{
	int ret = 0;
	unsigned char data;

	ret = read_ram_from_cpld(ram_offset + 1, &data);
	if(ret < 0)
		return ret;

	*value = data << 8;

	ret = read_ram_from_cpld(ram_offset, &data);
    *value |= data ;
	return ret;
}

#define PSOC_WATCHDOG_STATE 	(2)
#define PSOC_RXP_SXP_FLAG 		(4)
#define PSOC_PWM_RAM_REG  		(18)
#define PSOC_9506PORT_RAM_BASE 	(40)
#define PSU_STATE_RAM (50)

int getWdFromCpldRam(unsigned char *val)
{
	int ret = 0;
	unsigned char data;

	if(getCpuId() == PSOC_CTRL_SMBUS) {
		ret = read_ram_from_cpld(PSOC_WATCHDOG_STATE, &data);
		//printf("getWdFromCpldRam: ret=%d, data=%u\n", ret, data);
		if(ret >= 0)
			*val = data;
	}
	return ret;
}

int getPwmFromCpldRam(unsigned char *val)
{
	int ret = 0;
	unsigned char data;

	if(getCpuId() == PSOC_CTRL_SMBUS) {
		ret = read_ram_from_cpld(PSOC_PWM_RAM_REG, &data);
		//printf("getWdFromCpldRam: ret=%d, data=%u\n", ret, data);
		if(ret >= 0)
			*val = data;
	}
	return ret;
}

#if 0
int getAirFlowFromCpldRam(unsigned char *val, int index)
{
	int ret = 0;
	unsigned char data;

	if(index > 5 || index < -1)
		return -1;

	if(getCpuId() == PSOC_CTRL_SMBUS) {
		ret = read_ram_from_cpld(PSOC_9506PORT_RAM_BASE + (index - 1), &data);
		//printf("getWdFromCpldRam: ret=%d, data=%u\n", ret, data);
		if(ret >= 0) {
			*val = data;
		}
	}
	return ret;
}
#endif

int getRxpSxpFlag(unsigned char *val)
{
	int ret = 0;
	unsigned char data;

	if(getCpuId() == PSOC_CTRL_SMBUS) {
		ret = read_ram_from_cpld(PSOC_RXP_SXP_FLAG, &data);
		//printf("getRxpSxpFlag: ret=%d, data=%u\n", ret, data);
		if(ret >= 0)
			*val = data;
	}

	return ret;
}

/*end of Gerald*/
static int LM75_TEMP_FROM_REG(short data)
{
	return (swap(data&0xFFFF) /128)/2;
}

#define ROA_TEMP_REM_REG  (6)
#define BCM_TEMP_REM_REG  (ROA_TEMP_REM_REG + 1)
#define CPU_TEMP_REM_REG  (ROA_TEMP_REM_REG + 2)
#define FOA_TEMP_REM_REG  (ROA_TEMP_REM_REG + 3)
#define TEMP_STATUS_RAM_REG (ROA_TEMP_REM_REG + 4)
#define PSOC_VER_BIT1       (ROA_TEMP_REM_REG + 5)
#define PSOC_VER_BIT2       (ROA_TEMP_REM_REG + 6)

int getPSoCVersion(unsigned char *val)
{
	int ret = 0;
	unsigned char data;

	if(getCpuId() == PSOC_CTRL_SMBUS) {
		ret = read_ram_from_cpld(PSOC_VER_BIT1, &data);
		if(ret >= 0)
			*val = data;
		if(ret < 0)
			return ret;

		ret = read_ram_from_cpld(PSOC_VER_BIT2, &data);
		if(ret >= 0)
			*(val+1) = data;
		if(ret < 0)
			return ret;
	}
	return ret;
}

int getTempStatus(unsigned char *val)
{
	int ret = 0;
	unsigned char data;

	if(getCpuId() == PSOC_CTRL_SMBUS) {
		ret = read_ram_from_cpld(TEMP_STATUS_RAM_REG, &data);
		//printf("getRxpSxpFlag: ret=%d, data=%u\n", ret, data);
		if(ret >= 0)
			*val = data;

		return ret;
	}
	return ret;
}
int tsTempGet(int id, short *temp)
{
	int ret;
	unsigned char data;

    if(getCpuId() == PSOC_CTRL_SMBUS)
	{
		ret = read_ram_from_cpld(ROA_TEMP_REM_REG + id, &data);
		//printf("getPsuStatus: data=%u\n", data);
		if(ret >= 0)
			*temp = (short)data;
		return ret;
	}
	else
	{
		unsigned short bufTmp = 0;
		if((ret = getCtrlOfBus()) < 0)
			return ret;
		if((ret = enableChip(ts[id].ts_addr)) < 0)
			return ret;

		ret = chips_read_word(ts[id].ts_addr, ts[id].temp_reg, &bufTmp);
		if(ret >= 0)
			*temp = LM75_TEMP_FROM_REG((short)bufTmp);

		disableChip(ts[id].ts_addr);
	}
	return ret;
}

int tsOsGet(int id, unsigned short *temp)
{
	int ret;
	unsigned char data;

	if(getCpuId() == PSOC_CTRL_SMBUS)
	{
		ret = read_ram_from_cpld(ROA_TEMP_REM_REG + id, &data);
		//printf("getPsuStatus: data=%u\n", data);
		if(ret >= 0)
			*temp = (short)data;
		return ret;
	}
	else
	{
		unsigned short bufTmp = 0;
		if((ret = getCtrlOfBus()) < 0)
			return ret;

        if((ret = enableChip(ts[id].ts_addr)) < 0)
			return ret;

		ret = chips_read_word(ts[id].ts_addr, ts[id].os_reg, &bufTmp);
		if(ret >= 0)
			*temp = LM75_TEMP_FROM_REG((short)bufTmp);

        disableChip(ts[id].ts_addr);
	}
	return ret;
}

int getTsShutdown(int id)
{
	int ret;
	unsigned char data;

    if(getCpuId() == PSOC_CTRL_SMBUS)
	{
		ret = read_ram_from_cpld(ROA_TEMP_REM_REG + id, &data);
		//printf("getPsuStatus: data=%u\n", data);
		if(ret >= 0)
			ret = (short)data;
		return ret;
	}
	else
	{
		if((ret = getCtrlOfBus()) < 0)
			return ret;
		if((ret = enableChip(ts[id].ts_addr)) < 0)
			return ret;

		ret = chips_read_byte(ts[id].ts_addr, 0x1, &data);
		if(ret >= 0)
			ret = data & 0x1;

        disableChip(ts[id].ts_addr);

	}
	return ret;
}

int setTsShutdown(int id, int shutdown)
{
	unsigned char data;
	int ret;

    if((ret = getCtrlOfBus()) < 0)
		return ret;

    if((ret = enableChip(ts[id].ts_addr)) < 0)
		return ret;

    ret = chips_read_byte(ts[id].ts_addr, 0x1, &data);

	if(ret >= 0) {
		if(shutdown == 1) {
			data |= 0x1;
			ret = chips_write_byte(ts[id].ts_addr, 0x1, data);
		} else {
			data &= 0xFE;
			ret = chips_write_byte(ts[id].ts_addr, 0x1, data);
		}
	}
    disableChip(ts[id].ts_addr);
	return ret;
}

static short convert_short(char *s, int len)
{
    short i;
    short res = 0;

    for(i = 1; i < len; i++) {
        res = res * 2 + (s[i] - '0');
    }
    if(s[0] == '1')
        return -res;

    return res;
}

static long convert_linear(char *data)
{
    unsigned char low, high;
    short N, Y;
    long ret;
    int temp = 1;
    char s[11];
    int i;

    low = data[0];
    high = data[1];

    if((high & 0x80) > 0)
        high = ~high + 0x88;
    for(i = 0; i < 5; i++) {
        s[i] = (high & (0x80 >> i)) > 0 ? '1' : '0';
    }

    N = convert_short(s, 5);
    high = data[1];
    if((high & 0x04) > 0) {
        high = ~high + 0x40;
        if((low != 0xff) && (low != 0x0))
            low = ~low + 0x1;
        else {
            low = 0x00;
            high += 0x1;
        }

    }
    for(i = 5; i < 8; i++)
        s[i-5] = (high & (0x80 >> i)) > 0 ? '1' : '0';
    for(i = 0; i < 8; i++)
        s[i + 3] = (low & (0x80 >> i)) > 0 ? '1' : '0';

    Y = convert_short(s, 11);

    if(N > 0) {
        while(N > 0) {
            temp = temp * 2;
            N--;
        }

        ret = Y * temp * 100;
    }
    else {
        N = 0 - N;
        while(N > 0) {
            temp = temp * 2;
            N--;
        }
        ret = (Y * 100) / temp;
    }

    return ret;
}
int convert_vol(char *data)
{
        char low, high;
        short temp;
        short ret;
        int i;
        char s[16];

        low = data[0];
        high = data[1];
        //printf("low = %x, high = %x\n", low, high);
        if((high & 0x80) > 0)
        {
                high = ~high + 0x80;
                if(low != 0xff)
                        low += ~low + 0x1;
                else
                {
                        low = 0x0;
                        high += 0x1;
                }
        }
        for(i = 0; i < 8; i++)
                s[i] = (high & (0x80 >> i)) > 0 ? '1' : '0';
        for(i = 0; i < 8; i++)
                s[i + 8] = (low & (0x80 >> i)) > 0 ? '1' : '0';
        //printf("s = %s\n", s);
        temp = convert_short(s, 16);
        //printf("temp = %x\n", temp);
        ret = (temp * 100) / 0x200;

        return ret;
}
#define PSU_RAM_VIN_LOW(i) (90 - i * 20)
#define PSU_RAM_IIN_LOW(i) (PSU_RAM_VIN_LOW(i) + 4)
#define PSU_RAM_VOUT_LOW(i) (PSU_RAM_VIN_LOW(i) + 2)
#define PSU_RAM_IOUT_LOW(i) (PSU_RAM_VIN_LOW(i) + 6)
#define PSU_RAM_POUT_LOW(i) (PSU_RAM_VIN_LOW(i) + 10)
#define PSU_RAM_PIN_LOW(i) (PSU_RAM_VIN_LOW(i) + 8)
#define PSU_RAM_TEMP1_LOW(i) (56 - (i) * 2)
#define PSU_RAM_FAN_SPEED_LOW(i) (62 - (i)*2)
#define PSU_RAM_STATUS       (50)

int getPsuInfo(int id, struct psuInfo * info)
{
	int status;
	int ret;
//	char cpld_psu_reg;
//	char cpld_psu_reg_tmp;
	unsigned short vin;
	unsigned short iin;
	unsigned short vout;
	unsigned short iout;
	unsigned short pout;
	unsigned short pin;
	unsigned short temp;
	char psu_addr[] = {
		0x58,
		0x59,
	};

	if(getCpuId() == PSOC_CTRL_SMBUS)	{
		status = getPsuPresent(id);
		if(status == 1)
			return -1;

		if(status == 0) {//PSU-L present
			if ((ret = read_ram_short_from_cpld(PSU_RAM_VIN_LOW(id), &vin)) < 0)
				return -1;
			if ((ret = read_ram_short_from_cpld(PSU_RAM_IIN_LOW(id), &iin)) < 0)
				return -1;
			if ((ret = read_ram_short_from_cpld(PSU_RAM_VOUT_LOW(id), &vout)) < 0)
				return -1;
			if ((ret = read_ram_short_from_cpld(PSU_RAM_IOUT_LOW(id), &iout)) < 0)
				return -1;
			if ((ret = read_ram_short_from_cpld(PSU_RAM_POUT_LOW(id), &pout)) < 0)
				return -1;
			if ((ret = read_ram_short_from_cpld(PSU_RAM_PIN_LOW(id), &pin)) < 0)
				return -1;
			if ((ret = read_ram_short_from_cpld(PSU_RAM_TEMP1_LOW(id), &temp)) < 0)
				return -1;

			info->vin = convert_linear((char *)&vin) * 10;
			info->iin = convert_linear((char *)&iin) * 10;
			info->vout = convert_vol((char *)&vout) * 10;
			info->iout = convert_linear((char *)&iout) * 10;
			info->pout = convert_linear((char *)&pout) * 10;
			info->pin = convert_linear((char *)&pin) * 10;
			info->temp = convert_linear((char *)&temp)/100;
			return 0;
		}

	} else {
    status = getPsuPresent(id);
    if (status == 0)
	{
		if((ret = getCtrlOfBus()) < 0)
			return ret;
		if((ret = enableChip(psu_addr[id])) < 0)
			return ret;

		if((ret = chips_read_word(psu_addr[id], PSU_READ_VIN, &vin)) < 0)
			goto error;
		if((ret = chips_read_word(psu_addr[id], PSU_READ_IIN, &iin)) < 0)
			goto error;
		if((ret = chips_read_word(psu_addr[id], PSU_READ_VOUT, &vout)) < 0)
			goto error;
		if((ret = chips_read_word(psu_addr[id], PSU_READ_IOUT, &iout)) < 0)
			goto error;
		if((ret = chips_read_word(psu_addr[id], PSU_READ_POUT, &pout)) < 0)
			goto error;
		if((ret = chips_read_word(psu_addr[id], PSU_READ_PIN, &pin)) < 0)
			goto error;
		if((ret = chips_read_word(psu_addr[id], PSU_READ_TEMP_1, &temp)) < 0)
			goto error;
		info->vin = convert_linear((char *)&vin) * 10;
		info->iin = convert_linear((char *)&iin) * 10;
		info->vout = convert_vol((char *)&vout) * 10;
		info->iout = convert_linear((char *)&iout) * 10;
		info->pout = convert_linear((char *)&pout) * 10;
		info->pin = convert_linear((char *)&pin) * 10;
		info->temp = convert_linear((char *)&temp)/100;
		if((ret = disableChip(psu_addr[id])) < 0)
			return ret;
		return 0;
	}
   }

error:
	ret = disableChip(psu_addr[id]);
	return -1;

}
//return value 0:present, 1,absent
int getPsuPresent(int id)
{

	unsigned char data;
	int ret = -1;

	if(getCpuId() == PSOC_CTRL_SMBUS)
	{
		ret = read_ram_from_cpld(PSU_STATE_RAM, &data);
		if(ret >= 0) {
			ret = (data >> (1-id)) & 0x1;
			ret = (ret == 0) ? 1 : 0;
		}
	}
	else
	{
		int gpio;
		switch(id)
		{
			case 0:
				gpio = 27;//IO3_3
				break;
			case 1:
				gpio = 28;//IO3_4
				break;
			default:
				ret = -1;
				return ret;
		}
		ret = pca9506_init_pin(gpio,gpio_in);
		if(ret < 0)
			return ret;
		ret = pca9506_read_pin(gpio);
	}


	return ret;
}

int read_psu(unsigned char addr, unsigned char reg, unsigned short *value)
{
	int status;
	int ret = -1;
	unsigned short data;
	int id = 0;
	if(addr == 0x58)
		id = 8;
	else if(addr == 0x59)
		id = 9;
	status = getFanPresent(id);

	if (status == 0)
	{
		if((ret = getCtrlOfBus()) < 0)
			return ret;
		if((ret = enableChip(addr)) < 0)
			return ret;

		if((ret = chips_read_word(addr, reg, &data)) < 0)
			goto error;
		*value = data;
		if((ret = disableChip(addr)) < 0)
			return ret;
		return 0;
	}
	else
	{
		goto error;
	}

error:
	ret = disableChip(addr);
	return -1;

}

int write_psu(unsigned char addr, unsigned char reg, unsigned short value)
{
#if 0
	int status;
	int ret = -1;
	int id;
    unsigned char data[5];
    unsigned char crc = 0;
    unsigned char errStatus = 0;
	unsigned char blockData[3];

	unsigned char *wrData = (unsigned char *)&value;

    data[0] = (addr << 1) | 0;//0 i2c write
    data[1] = reg;
    data[2] = wrData[0];
    data[3] = wrData[1];
    crc = i2c_smbus_pec(crc, data, 4);

	blockData[0] = data[2];
	blockData[1] = data[3];
	blockData[2] = crc;

	if(addr == 0x58)
		id = 8;
	else if(addr == 0x59)
		id = 9;
	status = getFanPresent(id);

	if (status == 0)
	{
		if((ret = getCtrlOfBus()) < 0)
			return ret;
		if((ret = enableChip(addr)) < 0)
			return ret;

		if((ret = chips_write_block(addr, reg, 3, blockData)) < 0)
			goto error;

		if((ret = disableChip(addr)) < 0)
			return ret;
		return 0;
	}
	else
	{
		goto error;
	}

error:
	ret = disableChip(addr);
	return -1;
#endif
	int status;
	int ret = -1;
	//unsigned short data;
	int id = 0;

	if(addr == 0x58)
		id = 8;
	else if(addr == 0x59)
		id = 9;
	status = getFanPresent(id);

	if (status == 0)
	{
		if((ret = getCtrlOfBus()) < 0)
			return ret;
		if((ret = enableChip(addr)) < 0)
			return ret;

		if((ret = chips_write_word_pec(addr, reg, value, 1)) < 0)
			goto error;
		if((ret = disableChip(addr)) < 0)
			return ret;
		return 0;
	}
	else
	{
		goto error;
	}

error:
	ret = disableChip(addr);
	return -1;

}

/*dev_id 5: system eeprom, dev_id 6: switch eeprom*/
int
eeprom_enable(unsigned char dev_id)
{
    int ret =0;
    unsigned char buf = 0x00;
    int regNum = 0x123;

    ret = read_cpld(regNum, &buf);
    buf &= ~(BIT0);
    ret = write_cpld(regNum, buf);

    if(getCpuId() != PSOC_CTRL_SMBUS) {
        getCtrlOfBus();
        ret = openChannel(dev_id);
    }
    return ret;

}



int
eeprom_disable(unsigned char dev_id)
{
    int ret =0;
    unsigned char buf = 0x00;
    int regNum = 0x123;;


    if(getCpuId() != PSOC_CTRL_SMBUS) {
        closeChannel(dev_id);
        read_cpld(regNum, &buf);
        buf |= BIT0;
        ret = write_cpld(regNum, buf);
    }
    return ret;
}

/*
close bios flash protect
[3]	RW	0b0	Control The Status of SPD2_WP.
[2]	RW	0b0	Control The Status of SPD1_WP.
*/
int biosflash_enable()
{
	int ret =0;
	unsigned char buf = 0x00;
	int regNum = 0x123;;

	ret = read_cpld(regNum, &buf);
	buf &= ~(BIT2 | BIT3);
	ret = write_cpld(regNum, buf);

	return ret;

}
/*
open bios flash protect
*/
int biosflash_disable()
{
	int ret =0;
	unsigned char buf = 0x00;
	int regNum = 0x123;;

	ret = read_cpld(regNum, &buf);
	buf |= (BIT2 | BIT3);
	ret = write_cpld(regNum, buf);

	return ret;
}

static inline int pca9506_read_byte_data(unsigned char reg,unsigned char *value)
{
		int ret;

		if(getCpuId() == PSOC_CTRL_SMBUS) {
			unsigned char buf;
			ret = read_cpld((int)0x198, &buf);
			if(ret >= 0)
				*value = (short)buf;
		} else {
			unsigned char data;
			if((ret = getCtrlOfBus()) < 0)
				return ret;

			if((ret = openChannel(9)) < 0)
			{
				return ret;
			}


			ret = chips_read_byte(PCA9506_ADDR, reg, &data);
			if(ret >= 0)
				*value = data;

			closeChannel(9);
		}
		return ret;
}

static inline int pca9506_write_byte_data(unsigned char reg, unsigned char val)
{
	int ret = -1;

	if(getCpuId() == PSOC_CTRL_SMBUS)
	{
		unsigned char buf;
		return read_cpld((int)0x198, &buf);
	} else {
		//unsigned char data;
		if((ret = getCtrlOfBus()) < 0)
			return ret;

		if((ret = openChannel(9)) < 0)
			return ret;


		ret = chips_write_byte(PCA9506_ADDR, reg, val);
		closeChannel(9);
	}
	return ret;
}

int pca9506_init_pin(unsigned char port, gpio_dir val)
{
	int ret = -1;
	unsigned char bank = 0;
	unsigned char bit = 0;

	unsigned char cfg_reg = 0;
	//unsigned char input_reg = 0;
	unsigned char output_reg = 0;

	unsigned char cfg_val = 0;
	//unsigned char input_val = 0;
	unsigned char output_val = 0;

	bank = port/8;
	bit = port%8;

	cfg_reg = PCA9506_IOC0_REG + bank;
	output_reg = PCA9506_OP0_REG + bank;

	// set output direction
	ret = pca9506_read_byte_data(cfg_reg,&cfg_val);

	bit &= 0xf;
	if(val == gpio_out)
		cfg_val &= ~(0x01 << bit);
	else
		cfg_val |= (PCA9506_IO_INPUT << bit);

	ret = pca9506_write_byte_data(cfg_reg, cfg_val);

	if(val == gpio_out)
	{
		// set value
		ret = pca9506_read_byte_data(output_reg, &output_val);

		val &= 0x01;
		output_val &= ~(0x01 << bit);
		output_val |= (val << bit);
		ret = pca9506_write_byte_data(output_reg, output_val);
	}
	return ret;
}

int pca9506_read_pin(unsigned char port)
{
	unsigned char bank = 0;
	unsigned char bit = 0;

	//unsigned char cfg_reg = 0;
	unsigned char input_reg = 0;

	//unsigned char cfg_val = 0;
	unsigned char input_val = 0;

	bank = port/8;
	bit = port%8;

	input_reg = PCA9506_IP0_REG + bank;

	// get value
	pca9506_read_byte_data(input_reg, &input_val);

	return (input_val >> bit) & 0x01;
}


int pca9506_write_pin(unsigned char port, unsigned char val)
{
	int ret = -1;
	unsigned char bank = 0;
	unsigned char bit = 0;

	//unsigned char cfg_reg = 0;
	unsigned char input_reg = 0;
	unsigned char output_reg = 0;

	//unsigned char cfg_val = 0;
	unsigned char input_val = 0;
	//unsigned char output_val = 0;

	bank = port/8;
	bit = port%8;

	input_reg = PCA9506_IP0_REG + bank;
	output_reg = PCA9506_OP0_REG + bank;

	// get value
	ret = pca9506_read_byte_data(input_reg,&input_val);

	val &= 0x01;
	input_val &= ~(0x01 << bit);
	input_val |= (val << bit);
	ret = pca9506_write_byte_data(output_reg, input_val);

	return ret;
}

/*
*   param: id is fan id, from 0 to FAN_NUM + 1
*   return: 0 means present, 1 means absent
*/
int getFanPresent(int id)
{

	unsigned char data;
	int ret = -1;

	if(getCpuId() == PSOC_CTRL_SMBUS)
	{
		int off;
		if(id >= (FAN_NUM + 2) || id < 0)
			return -1;

		if(id < FAN_NUM)
			off = id / 2;
		else
			off = id - FAN_NUM;

		if(id <= (FAN_NUM - 1))
			ret = read_ram_from_cpld(FAN_PRES_STATUS, &data);
		else
			ret = read_ram_from_cpld(PSU_PRES_STATUS, &data);

		if(id <= (FAN_NUM - 1))
			ret = (data >> off) & 0x1;
		else {
			//ret = (data >> (1 - off)) & 0x1;
			ret = (data & (1 << (1 - off))) ? 1 : 0;
		}

		ret ^= 1;
	} else {
		int gpio = 0;

		if(id < FAN_NUM)
			gpio = presentGpio[id>>1];
		else if(id == FAN_NUM)
			gpio = 27;//IO3_3;
		else if(id == (FAN_NUM + 1))
			gpio = 28;//IO3_4
		ret = pca9506_init_pin(gpio,gpio_in);
		if(ret < 0)
			return ret;
		ret = pca9506_read_pin(gpio);

	}


	return ret;
}

//ret 1 failed, 0 OK
int getFanStatus(int id)
{
		unsigned char data;
		//unsigned short value;
		int ret = -1;

		if(id <0 || id >= FAN_NUM)
			return ret;

		if(getCpuId() == PSOC_CTRL_SMBUS)
		{
			if(id % 2) {//front
				ret = read_ram_from_cpld(FRONT_FAN_STATUS, &data);
				//printf("getFanStatus, front: id=%d, data=%u, ret=%d\n", id, data, ret);
				if(ret < 0)
					return ret;
				else
					ret = (data >> (id / 2)) & 0x1;
			}else { //rear
				ret = read_ram_from_cpld(REAR_FAN_STATUS, &data);
				if(ret < 0)
					return ret;
				else
					ret = (data >> (id / 2)) & 0x1;
			}

		}
		else
		{
			if(id < FAN_NUM) //emc2305 fan controller
			{
				if((ret = getCtrlOfBus()) < 0)
					return ret;
				if((ret = enableChip(fan[id].emc_addr)) < 0)
					return ret;
				//ret = getFanPresent(id);
				//if(ret < 0)
				//	return ret;
				ret = chips_read_byte(fan[id].emc_addr, 0x25, &data);
				if(ret < 0)
				{
					disableChip(fan[id].emc_addr);
					return ret;
				}

				if (((data >> (0x1 << id)) & 0x01) == 0x01) {
					ret = 1;
				}
				disableChip(fan[id].emc_addr);
			}
			else
			{
				if((id == FAN_NUM)||(ret == (FAN_NUM + 1)))
				{
					ret = 0;
				}
			}
		}
		return ret;

}


// 1 is F2B, 0 is B2F
int getFanAirflow(int id)
{

	unsigned char data_io1, data_io2;
	int ret = -1;

	if(getCpuId() == 0x01)
	{
		int off;
		switch(id)
		{
			case 0:
			case 1:
				off = 0;
				break;
			case 2:
			case 3:
				off = 1;
				break;
			case 4:
			case 5:
				off = 2;
				break;
			case 6:
			case 7:
				off = 3;
				break;
			default:
				ret = -1;
				return ret;
		}

		ret = read_ram_from_cpld(PSOC_9506PORT_RAM_BASE + 1, &data_io1);
		if(ret < 0)
			return -1;

		ret = read_ram_from_cpld(PSOC_9506PORT_RAM_BASE + 2, &data_io2);
		if(ret < 0)
			return -1;

		//printf("getFanAirflow: ret=%d, data_io1=%u, data_io2=%u\n", ret, data_io1, data_io2);

		if(off == 0)
			ret = (data_io2 & 1) ? 1 : 0;
		else if(off == 1)
			ret = (data_io1 & (1 << 7)) ? 1 : 0;
		else if(off == 2)
			ret = (data_io2 & (1 << 3)) ? 1 : 0;
		else if(off == 3)
			ret = (data_io2 & (1 << 1)) ? 1 : 0;
		else
			ret = -1;

#if 0
		if(id <= 7)
			ret = read_cpld(CPLD_FAN_PRESENT, &data);
		else
			ret = read_cpld(CPLD_PSU_STATUS, &data);

		if(ret >= 0)
			ret = (data >> off) & 0x1;
#endif
	}
	else
	{
		int gpio;

		if(id < FAN_NUM)
			gpio = airFlowGpio[id>>1];
		else
			return ret;
		ret = pca9506_init_pin(gpio,gpio_in);
		if(ret < 0)
			return ret;
		ret = pca9506_read_pin(gpio);

	}


	return ret;
}

//on_off 0 set let red/green led on, 1 off. color 0 green 1 red, 2 yellow
static int setFanLed(int id, unsigned char color, unsigned char on_off)
{

	unsigned char data;
	int ret = -1;

	if(getCpuId() == 0x01)
	{
		int off;
		switch(id)
		{
			case 0:
			case 1:
				off = 0;
				break;
			case 2:
			case 3:
				off = 1;
				break;
			case 4:
			case 5:
				off = 2;
				break;
			case 6:
			case 7:
				off = 3;
				break;
			case 8:
				off = 4;
				break;
			case 9:
				off = 5;
				break;
			default:
				ret = -1;
				return ret;
		}
		if(id <= 7)
			ret = read_cpld(CPLD_FAN_PRESENT, &data);
		else
			ret = read_cpld(CPLD_PSU_STATUS, &data);
		if(ret >= 0)
			ret = (data >> off) & 0x1;
	}
	else
	{
		int gpio;
		gpio = (color == 0)?ledGpio[id>>1].green_gpio:ledGpio[id>>1].red_gpio;
		ret = pca9506_init_pin(gpio,gpio_out);
		if(ret < 0)
			return ret;
		ret = pca9506_write_pin(gpio,on_off);

	}


	return ret;
}
int setFanLedGreen(int id)
{
	int ret = -1;
	ret = setFanLed(id,red,1);
	ret = setFanLed(id,green,0);
	return ret;
}
int setFanLedRed(int id)
{
	int ret = -1;
	ret = setFanLed(id,green,1);
	ret = setFanLed(id,red,0);
	return ret;
}
int setFanLedYellow(int id)
{
	int ret = -1;
	ret = setFanLed(id,red,0);
	ret = setFanLed(id,green,0);
	return ret;
}
int setFanLedOff(int id)
{
	int ret = -1;
	ret = setFanLed(id,red,1);
	ret = setFanLed(id,green,1);
	return ret;
}

int setPsuLedOn(int id)
{
	int ret = -1;
	unsigned char buf = 0x00;
	ret = read_cpld(CPLD_FP_LED, &buf);
	buf &= ~(1 << (2 + id));
	ret = write_cpld(CPLD_FP_LED, buf);
	return ret;
}
int setPsuLedOff(int id)
{
	int ret = -1;
	unsigned char buf = 0x00;

	ret = read_cpld(CPLD_FP_LED, &buf);

	buf |= 1 << (2 + id);
	ret = write_cpld(CPLD_FP_LED, buf);
	if (ret < 0)
	{
			printf("write error\n");
			return ret;
	}

	return ret;

}
int setSysLedOn()
{
	int ret = -1;
	unsigned char buf = 0x00;
	ret = read_cpld(CPLD_FP_LED, &buf);
	buf &=  ~(BIT0);
	buf &=  ~(BIT1);;
	ret = write_cpld(CPLD_FP_LED, buf);
	if (ret < 0)
	{
			printf("setSysLedOn error\n");
			return ret;
	}

	return ret;

}

int
setSysLedOff()
{
	int ret = -1;
	unsigned char buf = 0x00;

	ret = read_cpld(CPLD_FP_LED, &buf);
	buf |= 0x11;
	ret = write_cpld(CPLD_FP_LED, buf);
	if (ret < 0)
	{
			printf("setSysLedOff error\n");
			return ret;
	}

	return ret;

}


static int FAN_PERIOD_TO_RPM(int x)
{
	if (x > (0xf5c2 >> 3)) /*<1000rpm, fan fail*/
		return 0;

	return 3932160 * 2 / x;
}

int fanPwmGet(int id, int *pwm)
{
	unsigned char data;
	unsigned short value;
	int ret = -1;


	if(getCpuId() == 0x01)
	{
		ret = read_ram_from_cpld(PSOC_PWM_RAM_REG, &data);
		//printf("getWdFromCpldRam: ret=%d, data=%u\n", ret, data);
		if(ret >= 0)
			*pwm = (data * 100)/255;
		//*speed = FAN_PERIOD_TO_RPM((*speed & 0x0FFFF)>> 3);
		//*speed = FAN_PERIOD_TO_RPM(*speed >> 3);
	}
	else
	{
		if(id < FAN_NUM) //emc2305 fan controller
		{
			if((ret = getCtrlOfBus()) < 0)
				return ret;
			if((ret = enableChip(fan[id].emc_addr)) < 0)
				return ret;
			//ret = getFanPresent(id);
			//if(ret < 0)
			//	return ret;
			ret = chips_read_byte(fan[id].emc_addr, fan[id].driver_reg, &data);//pwm = speed*255/100
			if(ret < 0)
			{
				disableChip(fan[id].emc_addr);
				return ret;
			}
			*pwm = (data * 100)/255;
			disableChip(fan[id].emc_addr);
		}
		else
		{
			if(id == FAN_NUM)//psu fan
			{
				//unsigned short temp;

				ret = read_psu(0x58, PSU_PWM_REG, &value);


				//ret = read_psu(0x58, PSU_PWM_REG, &temp);
				if(ret < 0)
					return ret;
				*pwm = value;
			}
			else if(id == (FAN_NUM + 1))
			{
				ret = read_psu(0x59, PSU_PWM_REG, &value);
				if(ret < 0)
					return ret;
				*pwm = value;
			}

		}
	}
	return ret;
}

int fanSpeedGet(int id, int *speed)
{
	unsigned char data;
	unsigned short value;
	int ret = -1;

	if(getCpuId() == PSOC_CTRL_SMBUS)
	{
		if(id <= FAN_NUM - 1) {
			ret = read_ram_from_cpld(FAN1_RAM_SPEED_LOW + id * 2 + 1, &data);
			if(ret < 0) {
				return ret;
			}

		*speed = data << 8;

			ret = read_ram_from_cpld(FAN1_RAM_SPEED_LOW + id * 2, &data);
			if(ret >= 0) {
				*speed |= data ;
			} else {
			}
		} else {
			if ((ret = read_ram_short_from_cpld(PSU_RAM_FAN_SPEED_LOW(id-8), &value)) < 0)
				return -1;
			//*speed = value;
			*speed = convert_linear((char *)&value)/100;
		}
		//*speed = FAN_PERIOD_TO_RPM((*speed & 0x0FFFF)>> 3);
		//*speed = FAN_PERIOD_TO_RPM(*speed >> 3);
	}
	else
	{
		if(id < FAN_NUM) //emc2305 fan controller
		{
			if((ret = getCtrlOfBus()) < 0)
				return ret;
			if((ret = enableChip(fan[id].emc_addr)) < 0)
				return ret;
			//ret = getFanPresent(id);
			//if(ret < 0)
			//	return ret;
			ret = chips_read_byte(fan[id].emc_addr, fan[id].speed_reg, &data);
			if(ret < 0) {
				disableChip(fan[id].emc_addr);
				return ret;
			}
			*speed = data << 8;
			usleep(50000);
			ret = chips_read_byte(fan[id].emc_addr, (fan[id].speed_reg+1), &data);
			if(ret >= 0)
			{
				*speed |= data ;
			}
			//*speed = FAN_PERIOD_TO_RPM((*speed & 0x0FFFF)>> 3);
			*speed = FAN_PERIOD_TO_RPM(*speed >> 3);
			disableChip(fan[id].emc_addr);
		}
		else
		{
			if(id == FAN_NUM)
			{
				ret = read_psu(0x58, PSU_SPEED_REG, &value);
				if(ret < 0)
					return ret;
				*speed = convert_linear((char *)&value)/100;
			}
			else if(id == (FAN_NUM + 1))
			{
				ret = read_psu(0x59, PSU_SPEED_REG, &value);
				if(ret < 0)
					return ret;
				*speed = convert_linear((char *)&value)/100;
			}
		}
	}
	return ret;
}
#define EMC2305_REG_FAN_DRIVE(n) (0x30 + 0x10 * n)
#define EMC2305_REG_FAN_MIN_DRIVE(n) (0x38 + 0x10 * n)
#define EMC2305_REG_FAN_TACH(n) (0x3E + 0x10 * n)


int fanInit()
{
	int ret = -1;
	int id = 0;
	unsigned char sys_fan_speed = 0x66;

	if(getCpuId() == PSOC_CTRL_SMBUS)
	{

		//*speed = FAN_PERIOD_TO_RPM((*speed & 0x0FFFF)>> 3);
		//*speed = FAN_PERIOD_TO_RPM(*speed >> 3);
	}
	else
	{
		for (id = 0; id < FAN_NUM; id++) {
			if((ret = getCtrlOfBus()) < 0)
				return ret;
			if((ret = enableChip(fan[id].emc_addr)) < 0)
				return ret;

			/* set minimum drive to 20% */
			ret = chips_write_byte(fan[id].emc_addr, (fan[id].driver_reg-0x30)/0x10+0x38, 0x33);
			ret = chips_write_byte(fan[id].emc_addr, fan[id].driver_reg, sys_fan_speed);
			if(ret < 0)
			{
				disableChip(fan[id].emc_addr);
				return ret;
			}

			disableChip(fan[id].emc_addr);
		}
	}
	return ret;
}
//id 0-based
int fanSpeedSet(int id, unsigned short speed)
{
	int ret = -1;

	if(getCpuId() == PSOC_CTRL_SMBUS)
	{

		//*speed = FAN_PERIOD_TO_RPM((*speed & 0x0FFFF)>> 3);
		//*speed = FAN_PERIOD_TO_RPM(*speed >> 3);
	}
	else
	{
		if(id < FAN_NUM) //emc2305 fan controller
		{
			if((ret = getCtrlOfBus()) < 0)
				return ret;
			if((ret = enableChip(fan[id].emc_addr)) < 0)
				return ret;

			ret = chips_write_byte(fan[id].emc_addr, fan[id].driver_reg, speed*255/100);//current_pwm_value*255/100
			if(ret < 0)
			{
				disableChip(fan[id].emc_addr);
				return ret;
			}

			disableChip(fan[id].emc_addr);
		}
		else
		{
			if(id == FAN_NUM)//psu fan
			{
				//unsigned short temp;

				ret = write_psu(0x58, PSU_PWM_REG, speed);

				//ret = read_psu(0x58, PSU_PWM_REG, &temp);
				if(ret < 0)
					return ret;
			}
			else if(id == (FAN_NUM + 1))
			{
				ret = write_psu(0x59, PSU_PWM_REG, speed);
				if(ret < 0)
					return ret;
			}
		}
	}
	return ret;
}


