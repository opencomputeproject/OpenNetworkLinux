#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include "i2c_dev.h"

static int
i2c_write_2b(char addr, __u8 buf[2])
{

	int busNum = 1;
	dataType type = LEN_BYTE;
	int ret = -1;
	unsigned char reg;
	unsigned char data;

	reg = buf[0];
	data =  buf[1];

    ret = i2c_smbus_write(busNum, addr, reg, type, 1, &data);
	return ret;
}

static int
i2c_write_3b(char addr, __u8 buf[3])
{
	int busNum = 1;
	dataType type = LEN_WORD;
	int ret = -1;
	char data[2];

    data[0] = buf[1];
	data[1] = buf[2];

    ret = i2c_smbus_write(busNum, addr, buf[0], type, 1, (unsigned char *)&data);

    return ret;
}

static inline int
i2c_smbus_access(int file, char read_write, __u8 command,
                                     int size, union i2c_smbus_data *data)
{
    struct i2c_smbus_ioctl_data args;
    int ret;

    args.read_write = read_write;
    args.command = command;
    args.size = size;
    args.data = data;

	ret = ioctl(file, I2C_SMBUS, &args);

    return ret;
}

static int
i2c_smbus_write_word_data(int file, __u8 command,
        __u16 value)
{
    union i2c_smbus_data data;
    data.word = value;
    return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
                            I2C_SMBUS_WORD_DATA, &data);
}

static int
i2c_smbus_write_byte_data(int file, __u8 command,
        __u8 value)
{
    union i2c_smbus_data data;
    data.byte = value;
    return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
                            I2C_SMBUS_BYTE_DATA, &data);
}

static int
i2c_smbus_read_byte(int file,__u8 *buf)
{
	union i2c_smbus_data data;
	if (i2c_smbus_access(file,I2C_SMBUS_READ,0,I2C_SMBUS_BYTE,&data)) {
		return -1;
	} else {
		*buf = 0x0FF & data.byte;
        return 0;
	}
}
static int
i2_smbus_read_only(int busNum, short addr, dataType type,
									int size,__u8 *buf)
{
	int i;
	int fd;
	int ret = -1;
	char fname[20];

	sprintf(fname, "/dev/i2c-%d", busNum);
	fd = open(fname, O_RDWR);

    if(fd < 0)
		return ret;

	ret = ioctl(fd, I2C_SLAVE, addr);

    if(LEN_BYTE == type) {
		for(i=0; i<size; i++) {
			ret = i2c_smbus_read_byte(fd, &buf[i]);
			if(ret < 0)
				break;
		}
	}

	close(fd);
	return ret;
}


static int
i2c_smbus_read_byte_data(int file, __u8 command, __u8 *buf)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file, I2C_SMBUS_READ, command,
                         I2C_SMBUS_BYTE_DATA, &data))
        return -1;
    else
	{
		*buf = 0x0FF & data.byte;
        return 0;
	}
}
static int i2c_smbus_read_word_data(int file, __u8 command, __u16 *buf)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file, I2C_SMBUS_READ, command,
                         I2C_SMBUS_WORD_DATA, &data)) {
		return -1;
    } else {
		*buf = 0x0FFFF & data.word;
        return 0;
	}
}

int i2_smbus_read(int busNum, short addr, __u8 reg, dataType type,
									int size,__u8 *buf)
{
	int i = 0;
	int fd;
	int ret = -1;
	__u16 val;
	char fname[20];

    sprintf(fname, "/dev/i2c-%d", busNum);
	fd = open(fname, O_RDWR);
	if(fd < 0)
		return ret;

	ret = ioctl(fd, I2C_SLAVE, addr);

	if(LEN_BYTE == type)
	{
		for(i=0; i<size; i++)
		{
			ret = i2c_smbus_read_byte_data(fd, (reg+i), &buf[i]);
			if(ret < 0)
				break;
		}
	}
	else
	{
		for(i=0; i<size; i++)
		{
			ret = i2c_smbus_read_word_data(fd, (reg+i), &val);
			if(ret < 0)
				break;
			buf[2*i+1] = (val >> 8) & 0x0FF;
			buf[2*i] = val & 0x0FF;
		}
	}
	close(fd);
	return ret;
}



int i2c_smbus_write_pec(int busNum, short addr, __u8 reg, dataType type,
									int pec_flag, __u8 *buf)
{
	int fd;
	int ret = -1;
	short val;
	char fname[20];

	sprintf(fname, "/dev/i2c-%d", busNum);
	fd = open(fname, O_RDWR);

    if(fd < 0)
		return ret;

	ret = ioctl(fd, I2C_SLAVE, addr);
	ret = ioctl(fd, I2C_PEC, pec_flag);
	if(LEN_BYTE == type) {
			ret = i2c_smbus_write_byte_data(fd, reg, buf[0]);
	} else {
			val = (((unsigned short)(buf[1])) << 8) | buf[0];
			//val = (((unsigned short)(buf[2*i])) << 8) | buf[2*i+1];
			ret = i2c_smbus_write_word_data(fd, reg, val);
	}
	close(fd);
	return ret;
}

int i2c_smbus_write_block_data(int busNum, short addr, __u8 command,
                                               __u8 length, const __u8 *values)
{
	union i2c_smbus_data data;
	int i;
	int fd;
	int ret = -1;
	char fname[20];

	if (length > 32)
		length = 32;
	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];
	data.block[0] = length;


	sprintf(fname, "/dev/i2c-%d", busNum);
	fd = open(fname, O_RDWR);

    if(fd < 0)
		return ret;

    ret = ioctl(fd, I2C_SLAVE, addr);
	ret = ioctl(fd, I2C_PEC, 1);

	ret = i2c_smbus_access(fd,I2C_SMBUS_WRITE,command,
	                        I2C_SMBUS_BLOCK_DATA, &data);
	close(fd);
	return ret;
}

int i2c_smbus_read_16reg(int busNum, short addr, short reg,
									int size, __u8 *buf)
{
	int i;
	int fd;
	int ret = -1;
	char regNum[2];
	char fname[20];
	struct i2c_rdwr_ioctl_data ioctl_data;
	struct i2c_msg msgs[2];

	sprintf(fname, "/dev/i2c-%d", busNum);
	fd = open(fname, O_RDWR);
	if(fd < 0)
		return ret;

	ret = ioctl(fd, I2C_SLAVE, addr);

	for(i=0; i<size; i++)
	{
		regNum[1] = (reg + i) & 0x0FF;
		regNum[0] = ((reg + i) >> 8) & 0x0FF;
		msgs[0].addr= addr;
		msgs[0].len= 2;
		msgs[0].buf= regNum;
		msgs[1].addr= addr;
		msgs[1].flags |= I2C_M_RD;
		msgs[1].len= 1;
		msgs[1].buf= (char *)&buf[i];
		ioctl_data.nmsgs= 2;
		ioctl_data.msgs= msgs;
		ret = ioctl(fd, I2C_RDWR, &ioctl_data);
		if(ret)
			break;
	}
	close(fd);
	return ret;
}
//obsoleted
int i2c_smbus_write_16reg(int busNum, short addr, short reg,
									int size, const __u8 *buf)
{
	int i;
	int fd;
	int ret = -1;
	char wBuf[3];
	char fname[20];
	struct i2c_rdwr_ioctl_data ioctl_data;
	struct i2c_msg msg;

	sprintf(fname, "/dev/i2c-%d", busNum);
	fd = open(fname, O_RDWR);

    if(fd < 0)
		return ret;
	ret = ioctl(fd, I2C_SLAVE, addr);

    for(i=0; i<size; i++) {
		wBuf[1] = (reg + i) & 0x0FF;
		wBuf[0] = ((reg + i) >> 8) & 0x0FF;
		wBuf[2] = buf[i];
		msg.addr= addr;
		msg.flags = 0;
		msg.len= 3;
		msg.buf= wBuf;
		ioctl_data.nmsgs= 1;
		ioctl_data.msgs= &msg;
		ret = ioctl(fd, I2C_RDWR, &ioctl_data);
		if(ret)
			break;
	}
	close(fd);
	return ret;
}

int i2c_smbus_write_16reg_8byte(int busNum, short addr, short reg,
									int size, const __u8 *buf)
{
	int i;
	int fd;
	int ret = -1;
	char wBuf[10];
	char fname[20];
	struct i2c_rdwr_ioctl_data ioctl_data;
	struct i2c_msg msg;

    sprintf(fname, "/dev/i2c-%d", busNum);
	fd = open(fname, O_RDWR);

    if(fd < 0)
		return ret;

    for(i=0; i<size; i+=8) {
		wBuf[1] = (reg + i) & 0x0FF;
		wBuf[0] = ((reg + i) >> 8) & 0x0FF;
		memcpy(&wBuf[2],&buf[i], 8);
		msg.addr= addr;
		msg.flags = 0;
		msg.len= 10+1;
		msg.buf= wBuf;
		ioctl_data.nmsgs= 1;
		ioctl_data.msgs= &msg;
		ret = ioctl(fd, I2C_RDWR, &ioctl_data);
		if(ret)
			break;
	}
	close(fd);
	return ret;
}

int chips_read_byte(char addr, __u8 reg, __u8 *data)
{
	char busNum = 1;
	dataType type = LEN_BYTE;
	int ret = -1;
	ret = i2_smbus_read(busNum, addr, reg, type, 1, data);
	return ret;
}

int chips_read_word(char addr, __u8 reg, unsigned short *data)
{
	int busNum = 1;
	unsigned char buf[2];
	dataType type = LEN_WORD;
	int ret = -1;
	ret = i2_smbus_read(busNum, addr, reg, type, 1, buf);
	if(ret >= 0)
	{
		*data = ((unsigned short)buf[1] << 8) | buf[0];
	}
	return ret;
}

int
chips_write_word(char addr, __u8 reg, unsigned short data)
{
	int busNum = 1;
	dataType type = LEN_WORD;
	int ret = -1;
	unsigned char buf[2] =
		{ (data >> 8) & 0x00ff, data & 0x00ff};
	ret = i2c_smbus_write(busNum, addr, reg, type, 1, (unsigned char *)&buf);
	//ret = i2c_smbus_write(busNum, addr, reg, type, 1, (char *)&data);
	return ret;
}

int
chips_write_word_pec(char addr, __u8 reg, unsigned short data, int pec_flag)
{
	int busNum = 1;
	dataType type = LEN_WORD;
	int ret = -1;
	ret = i2c_smbus_write_pec(busNum, addr, reg, type, pec_flag, (unsigned char *)&data);
	return ret;
}

int
chips_write_block(char addr, __u8 reg, int byteCnt, unsigned char* wrData)
{
	int busNum = 1;
	int ret = -1;

	ret = i2c_smbus_write_block_data(busNum, addr, reg, byteCnt, wrData);
	return ret;
}

int
chips_write_byte(char addr, __u8 reg, const __u8 data)
{
	int busNum = 1;
	dataType type = LEN_BYTE;
	int ret = -1;
	ret = i2c_smbus_write(busNum, addr, reg, type, 1, (unsigned char *)&data);
	return ret;
}

int
i2c_smbus_write(int busNum, short addr, __u8 reg, dataType type,
									int size, __u8 *buf)
{
    int i;
	int fd;
	int ret = -1;
	short val;
	char fname[20];

	sprintf(fname, "/dev/i2c-%d", busNum);
	fd = open(fname, O_RDWR);
	if(fd < 0)
		return ret;

    ret = ioctl(fd, I2C_SLAVE, addr);
	if(LEN_BYTE == type)
	{
		for(i=0; i<size; i++)
		{
			ret = i2c_smbus_write_byte_data(fd, (reg+i), buf[i]);
			if(ret)
				break;
		}
	} else {
		for(i=0; i<size; i++)
		{
			val = (((unsigned short)(buf[2*i+1])) << 8) | buf[2*i];
			ret = i2c_smbus_write_word_data(fd, (reg+i), val);
			if(ret)
				break;
		}
	}
	close(fd);
	return ret;
}

int
eeprom_read_byte(char addr, __u16 mem_addr, __u8 *data)
{
	int ret;
	char busNum = 1;
	dataType type = LEN_BYTE;

	__u8 buf[2] = { (mem_addr >> 8) & 0x0ff, mem_addr & 0x0ff };
	ret = i2c_write_2b(addr, buf);

	if (ret < 0)
		return ret;
	ret = i2_smbus_read_only(busNum, addr, type, 1, data);
	return ret;
}

int
eeprom_write_byte(char addr, __u16 mem_addr, __u8 data)
{
	__u8 buf[3] =
		{ (mem_addr >> 8) & 0x00ff, mem_addr & 0x00ff, data };
	return i2c_write_3b(addr, buf);
}

