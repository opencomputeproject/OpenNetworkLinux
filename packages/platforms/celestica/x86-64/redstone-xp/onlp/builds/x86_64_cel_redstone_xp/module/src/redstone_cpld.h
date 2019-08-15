#ifndef _REDSTONE_CPLD_H_
#define _REDSTONE_CPLD_H_

#include <stdint.h>
#include <AIM/aim_log.h>
#include <asm/ioctl.h>

#define BIT0	0x1
#define BIT1	0x1 << 1
#define BIT2	0x1 << 2
#define BIT3	0x1 << 3
#define BIT4	0x1 << 4
#define BIT5	0x1 << 5
#define BIT6	0x1 << 6
#define BIT7	0x1 << 7

typedef struct RegData
{
    unsigned short regId;		/*register number*/
    char val;		    		/*register value*/
    unsigned char rw;			/*0:read 1:write*/
}RegData_t;

typedef struct SfpData
{
	char regId;
	int portId;
	char devAddr;
	char val[256];
	unsigned char rw;
	unsigned char len;
}SfpData_t;

#define CPLD_TYPE ( 0x89 )
#define IOCTL_READ_REG  _IOR(CPLD_TYPE,0x05,RegData_t)
#define IOCTL_WRITE_REG _IOW(CPLD_TYPE,0x08,RegData_t)
#define IOCTL_SFP_READ _IOR(CPLD_TYPE,0x0F,RegData_t)
#define IOCTL_SFP_WRITE _IOW(CPLD_TYPE,0x10,RegData_t)

#define CPLD1_REVISION	0x100
#define CPLD_RESET_CONTROL	0x102

#define CPLD2_REVISION	0x200
#define CPLD3_REVISION	0x280
#define CPLD4_REVISION	0x300
#define CPLD5_REVISION	0x380
#define CPLD_FAN_PRESENT	0x194
#define CPLD_FAN_STATUS_1	0x195
#define CPLD_FAN_STATUS_2	0x195
#define CPLD_PSU_STATUS	0x197
#define CPLD_FP_LED	0x303


int cpldRegRead(int regId, unsigned char *data, int size);
int cpldRegWrite(int regId, unsigned char *data, int size);
int cpldVersionGet(int cpld);

int read_cpld(int reg, unsigned char *value);
int write_cpld(int reg, unsigned char value);
int read_sfp(int portID, char devAddr, char reg, char *data, int len);
int write_sfp(int portID, char devAddr, char reg, char *data, int len);

int cpld_io_init(void);
int cpld_read(int addr);
void cpld_write(int addr, uint8_t value);
void cpld_modify(int addr, uint8_t andmask, uint8_t ormask);
int cpld_dump(aim_pvs_t* pvs, int cpldid);

#endif /* _REDSTONE_CPLD_H_ */
