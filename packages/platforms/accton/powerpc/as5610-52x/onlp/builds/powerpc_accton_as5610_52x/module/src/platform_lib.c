/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-devices.h>
#include "platform_lib.h"

#define CPLD_BASE_ADDRESS       0xEA000000

#define MAX_I2C_BUSSES     2
#define I2C_BUFFER_MAXSIZE 16

#define PMBUS_LITERAL_DATA_MULTIPLIER 1000

static int mem_read(unsigned int base, unsigned int offset, unsigned char *val)
{
    int fd;
    void* pMem;

    fd = open("/dev/mem", O_RDWR);
    if (fd < 0)
    {
        printf("IO R/W: can't open /dev/mem.\n");
        return -1;
    }
    else
    {
        pMem = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, base);

        if (pMem == MAP_FAILED)
        {
            printf("IO R/W: can't get the address pointer (%s)\n", strerror(errno));
            close(fd);
            return -1;
        }

        *val = *((volatile unsigned char*)(pMem + offset));

        if ( -1 == munmap(pMem, getpagesize() ) )
            printf("IO R/W: can't release the address pointer (%s)\n", strerror(errno));

        close(fd);
    }

    return 0;
}

static int mem_write(unsigned int base, unsigned int offset, unsigned char val)
{
    int fd;
    void* pMem;

    fd = open("/dev/mem", O_RDWR);
    if (fd < 0)
    {
        printf("IO R/W: can't open /dev/mem.\n");
        return -1;
    }
    else
    {
        pMem = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED,fd, base);

        if (pMem == MAP_FAILED)
        {
            printf("IO R/W: can't get the address pointer (%s)\n", strerror(errno));
            close(fd);
            return -1;
        }

        *((volatile unsigned char *)(pMem + offset)) = val;

        if ( -1 == munmap(pMem, getpagesize() ) )
		{
            printf("IO R/W: can't release the address pointer (%s)\n", strerror(errno));
		}

        close(fd);

        return 0;
    }
}

int cpld_read(unsigned int regOffset, unsigned char *val)
{
    return mem_read((unsigned int)CPLD_BASE_ADDRESS, regOffset, val);
}

int cpld_write(unsigned int regOffset, unsigned char val)
{
    return mem_write((unsigned int)CPLD_BASE_ADDRESS, regOffset, val);
}

int i2c_write(unsigned int bus_id, unsigned char i2c_addr,
        unsigned char offset, unsigned char *buf)
{
    static char sendbuffer[I2C_BUFFER_MAXSIZE + 1];
    int i2c_fd;
    int ret = 0;
    unsigned int len = 1;
    char fname[15];

    if (bus_id >= MAX_I2C_BUSSES) {
        printf("%s(): bus(%d) \n", __FUNCTION__, bus_id);
        return 1;
    }

    sprintf(fname, "/dev/i2c-%d", bus_id);
    i2c_fd = open(fname, O_RDWR); /* open device */
    if (i2c_fd < 0) {
        printf("%s(): failed to open %s \n", __FUNCTION__, fname);
        return 1;
    }

    ret = ioctl(i2c_fd, I2C_TENBIT, 0);/* set i2c 7bit mode */
    if (0 != ret) {
        printf("%s(): failed to set 7 bit mode %s \n", __FUNCTION__, fname);
        close(i2c_fd);
        return 1;
    }

    ret = ioctl(i2c_fd, I2C_SLAVE_FORCE, i2c_addr);
	if (0 != ret) {
        printf("%s(): failed to ioctl(I2C_SLAVE=0x%x)\n", __FUNCTION__, i2c_addr);
        close(i2c_fd);
		return 1;
    }

    /* write address: offset address */
    sendbuffer[0] = offset;
    memcpy(sendbuffer+1, buf, len);
    if ((ret = write (i2c_fd,  sendbuffer, len + 1)) < 0) {
        printf("%s(): failed to write i2c_fd(%d) \n", __FUNCTION__, i2c_fd);
        close(i2c_fd);
        return 1;
    }

    close(i2c_fd);
    return 0;
}

int i2c_read(unsigned int bus_id, unsigned char i2c_addr,
        unsigned char offset, unsigned char *buf)
{
    static char sendbuffer[I2C_BUFFER_MAXSIZE + 1];
    char fname[15];
    int i2c_fd = -1;
    int ret = 0;

    if (bus_id >= MAX_I2C_BUSSES) {
        printf("%s(): bus(%d) \n", __FUNCTION__, bus_id);
        return 1;
    }

    sprintf(fname, "/dev/i2c-%d", bus_id);
    sendbuffer[0] = offset;
    i2c_fd = open(fname, O_RDWR); /* open device */
    if (i2c_fd < 0) {
        printf("%s(): failed to open %s \n", __FUNCTION__, fname);
        return 1;
    }

    ret = ioctl(i2c_fd, I2C_TENBIT, 0);/* set i2c 7bit mode */
    if (0 != ret) {
        printf("%s(): failed to set 7 bit mode %s \n", __FUNCTION__, fname);
        close(i2c_fd);
        return 1;
    }

    ret = ioctl(i2c_fd, I2C_SLAVE_FORCE, i2c_addr);
	if (0 != ret) {
        printf("%s(): failed to ioctl(I2C_SLAVE=0x%x)\n", __FUNCTION__, i2c_addr);
        close(i2c_fd);
		return 1;
    }

    /* write data address: offset address */
    if ((ret = write(i2c_fd, sendbuffer, 1)) !=1 ) {
        printf("%s(): failed to write data_adr=0x%x\n", __FUNCTION__, offset);
        close(i2c_fd);
        return 1;
    }

	/* read data */
    if ((ret = read(i2c_fd, buf, 1)) < 0) {
        printf("%s(): failed to read \n", __FUNCTION__);
        close(i2c_fd);
        return 1;
    }

    close(i2c_fd);
    return 0;
}

/* FUNCTION NAME : i2c_nRead
 * PURPOSE : This function read the i2c slave device data
 * INPUT   : file,data_adr and count
 * OUTPUT  : buf
 * RETUEN  : I2C_SUCCESS/I2C_ERROR
 * NOTES   :
 */
int i2c_nRead
(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned int size, unsigned char * buf)
{
    static unsigned char sendbuffer[I2C_BUFFER_MAXSIZE + 1];
    char fname[15];
    int i2c_fd = -1;
    int ret = 0;

    if (bus_id >= MAX_I2C_BUSSES)
    {
        printf("%s(): bus(%d) \n", __FUNCTION__, bus_id);
        return 1;
    }

    sprintf(fname, "/dev/i2c-%d", bus_id);
    i2c_fd = open(fname, O_RDWR); /* open device */

    if (i2c_fd < 0)
    {
        printf("%s(): failed to open %s \n", __FUNCTION__, fname);
        return 1;
    }

    ret = ioctl(i2c_fd, I2C_TENBIT, 0);/* set i2c 7bit mode */

    if (0 != ret)
    {
        printf("%s(): failed to set 7 bit mode %s \n", __FUNCTION__, fname);
        close(i2c_fd);
        return 1;
    }

	ret = ioctl(i2c_fd, I2C_SLAVE_FORCE, i2c_addr);

	if (0 != ret)
    {
        printf("%s(): failed to ioctl(I2C_SLAVE=0x%x)\n", __FUNCTION__, i2c_addr);
        close(i2c_fd);
		return 1;
    }

	/* Returns the number of read bytes */
	/*s32 i2c_smbus_read_i2c_block_data(const struct i2c_client *client, u8 command,
										u8 length, u8 *values)*/
	if(i2c_smbus_read_i2c_block_data(i2c_fd, offset, size, sendbuffer) == -1)
	{
		//printf("%s(): failed to i2c_smbus_read_i2c_block_data(I2C_SLAVE=0x%x)\n", __FUNCTION__, i2c_addr);
		close(i2c_fd);
		return 1;
	}

	memcpy(buf, sendbuffer, size);

    close(i2c_fd);
    return 0;

}

/* FUNCTION NAME : i2c_nWrite
 * PURPOSE : This function write the data to I2C devices !
 * INPUT   : i2c_fd,data_adr ,buf and count
 * OUTPUT  :
 * RETUEN  : ret/I2C_ERROR
 * NOTES   :
 */
int
i2c_nWriteForce(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned int size, unsigned char * buf, int force)
{
    static unsigned char sendbuffer[I2C_BUFFER_MAXSIZE + 1];
    int i2c_fd;
    int ret = 0;
    char fname[15];

    if (bus_id >= MAX_I2C_BUSSES)
        {
            printf("%s(): bus(%d) \n", __FUNCTION__, bus_id);
            return 1;
        }

    sprintf(fname, "/dev/i2c-%d", bus_id);

    i2c_fd = open(fname, O_RDWR); /* open device */

    if (i2c_fd < 0)
        {
            printf("%s(): failed to open %s \n", __FUNCTION__, fname);
            return 1;
        }

    ret = ioctl(i2c_fd, I2C_TENBIT, 0);/* set i2c 7bit mode */

    if (0 != ret)
        {
            printf("%s(): failed to set 7 bit mode %s \n", __FUNCTION__, fname);
            close(i2c_fd);
            return 1;
        }

    ret = ioctl(i2c_fd, I2C_PEC, 1);/* set i2c 7bit mode */

    if (0 != ret)
        {
            printf("%s(): failed to set PEC %s \n", __FUNCTION__, fname);
            close(i2c_fd);
            return 1;
        }

    ret = ioctl(i2c_fd, force ? I2C_SLAVE_FORCE : I2C_SLAVE, i2c_addr);

    if (0 != ret)
        {
            printf("%s(): failed to ioctl(%s=0x%x)\n", __FUNCTION__,
                   force ? "I2C_SLAVE_FORCE" : "I2C_SLAVE", i2c_addr);
            close(i2c_fd);
            return 1;
        }

    /* write address: offset address */
    memcpy(sendbuffer, buf, size);

    if(i2c_smbus_write_i2c_block_data(i2c_fd, offset, size, sendbuffer) == -1)
	{
            printf("%s(): failed to ioctl(I2C_SLAVE=0x%x)\n", __FUNCTION__, i2c_addr);
            close(i2c_fd);
            return 1;
	}

    close(i2c_fd);

    return 0;
}

int
i2c_nWrite(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned int size, unsigned char * buf)
{
	/* Default writes are unforced. */
	return i2c_nWriteForce(bus_id, i2c_addr, offset, size, buf, 0);
}


int two_complement_to_int(unsigned short data, unsigned char valid_bit, unsigned short mask)
{
    unsigned short  valid_data  = data & mask;
    int  is_negative = valid_data >> (valid_bit - 1);

    return is_negative ? (-(((~valid_data) & mask) + 1)) : valid_data;
}

int
pmbus_parse_literal_format(unsigned short val)
{
    int exponent, mantissa;

    exponent = two_complement_to_int(val >> 11, 5, 0x1f);
    mantissa = two_complement_to_int(val & 0x7ff, 11, 0x7ff);

    if (exponent >= 0) {
        return (mantissa << exponent) * PMBUS_LITERAL_DATA_MULTIPLIER;
    }
    else {
        return (mantissa * PMBUS_LITERAL_DATA_MULTIPLIER) / (1 << -exponent);
    }
}

int
pmbus_read_literal_data(unsigned char bus, unsigned char i2c_addr, unsigned char reg, int *val)
{
    unsigned char buf[2] = {0};
    *val = 0;

    if (i2c_nRead(bus, i2c_addr, reg, sizeof(buf), buf) != 0) {
        printf("%s(%d): Failed to read literal data reg(0x%x)\r\n", __FUNCTION__, __LINE__, reg);
        return -1;
    }

    *val = pmbus_parse_literal_format((unsigned char)buf[1] << 8 | buf[0]);

    return 0;
}

int
pmbus_parse_vout_format(unsigned char vout_mode, unsigned short val)
{
    int exponent, mantissa;

    exponent = two_complement_to_int(vout_mode, 5, 0x1f);
    mantissa = val;

    if (exponent >= 0) {
        return (mantissa << exponent) * PMBUS_LITERAL_DATA_MULTIPLIER;
    }
    else {
        return (mantissa * PMBUS_LITERAL_DATA_MULTIPLIER) / (1 << -exponent);
    }
}

int
pmbus_read_vout_data(unsigned char bus, unsigned char i2c_addr, int *val)
{
    unsigned char vout_mode = 0;
    unsigned char vout_val[2] = {0};
    *val = 0;

    /* Read VOUT_MODE first */
    if (i2c_nRead(bus, i2c_addr, 0x20, sizeof(vout_mode), &vout_mode) != 0) {
        printf("%s(%d): Failed to read vout data reg(0x20)\r\n", __FUNCTION__, __LINE__);
        return -1;
    }

    /* Read VOUT */
    if (i2c_nRead(bus, i2c_addr, 0x8B, sizeof(vout_val), vout_val) != 0) {
        printf("%s(%d): Failed to read vout data reg(0x8B)\r\n", __FUNCTION__, __LINE__);
        return -1;
    }

    *val = pmbus_parse_vout_format(vout_mode, vout_val[1] << 8 | vout_val[0]);

    return 0;
}

int int_to_pmbus_linear(int val)
{
    int mantissa = val, exponent = 0;

    while (1)
    {
        if (mantissa < 1024)
            break;

        exponent++;
        mantissa >>= 1;
    }
    return (exponent << 11) | mantissa;
}


int as5610_52x_i2c0_pca9548_channel_set(unsigned char channel)
{
	/*
     * The RTC lives on Channel0 of the PCA9548 and is also accessed by the kernel RTC driver.
     * It has the PCA9548 reserved, which requires us to force the slave address programming.
     * The PCA9548 kernel driver always rewrites the channel selector when accessing it.
     *
     * This is fine for the first production version but will need to be revisited.
     */
	return i2c_nWriteForce(0, 0x70, 0x0, sizeof(channel), &channel, 1);
}

#define I2C_PSU_BUS_ID                     0
#define I2C_PSU_MODEL_NAME_LEN            13
#define I2C_AC_PSU1_SLAVE_ADDR_EEPROM   0x3A
#define I2C_AC_PSU2_SLAVE_ADDR_EEPROM   0x39
#define I2C_DC_PSU1_SLAVE_ADDR_EEPROM   0x56
#define I2C_DC_PSU2_SLAVE_ADDR_EEPROM   0x55
#define I2C_CPR_4011_MODEL_NAME_REG	    0x26
#define I2C_UM400D_MODEL_NAME_REG       0x50

as5610_52x_psu_type_t as5610_52x_get_psu_type(int id, char* modelname, int modelname_len)
{
    as5610_52x_psu_type_t ret = PSU_TYPE_UNKNOWN;
    unsigned char mux_ch[] = {0x2, 0x4};
    char model_name[I2C_PSU_MODEL_NAME_LEN + 1] = {0};
    unsigned char ac_eeprom_addr[] = {I2C_AC_PSU1_SLAVE_ADDR_EEPROM, I2C_AC_PSU2_SLAVE_ADDR_EEPROM};
    unsigned char dc_eeprom_addr[] = {I2C_DC_PSU1_SLAVE_ADDR_EEPROM, I2C_DC_PSU2_SLAVE_ADDR_EEPROM};

    /* Open channel to read eeprom of PSU
     */
    if (as5610_52x_i2c0_pca9548_channel_set(mux_ch[id-1]) != 0) {
        return PSU_TYPE_UNKNOWN;
    }

    /* Get model name from eeprom to see if it is AC or DC power
     */
    if (i2c_nRead(I2C_PSU_BUS_ID, ac_eeprom_addr[id-1], I2C_CPR_4011_MODEL_NAME_REG,
                  I2C_PSU_MODEL_NAME_LEN, (unsigned char*)model_name) == 0)
    {
        model_name[I2C_PSU_MODEL_NAME_LEN] = '\0';

        if (strncmp(model_name, "CPR-4011-4M11", strlen("CPR-4011-4M11")) == 0) {
            ret = PSU_TYPE_AC_F2B;
        }
        else if (strncmp(model_name, "CPR-4011-4M21", strlen("CPR-4011-4M21")) == 0) {
            ret = PSU_TYPE_AC_B2F;
        }
    }
    else if (i2c_nRead(I2C_PSU_BUS_ID, dc_eeprom_addr[id-1], I2C_UM400D_MODEL_NAME_REG,
             I2C_PSU_MODEL_NAME_LEN, (unsigned char*)model_name) == 0)
    {
        model_name[I2C_PSU_MODEL_NAME_LEN] = '\0';

        if (strncmp(model_name, "um400d01G", strlen("um400d01G")) == 0) {
            ret = PSU_TYPE_DC_48V_B2F;
        }
        else if (strncmp(model_name, "um400d01-01G", strlen("um400d01-01G")) == 0) {
            ret = PSU_TYPE_DC_48V_F2B;
        }
    }

    /* Close psu channel
     */
    as5610_52x_i2c0_pca9548_channel_set(0);

    if(modelname) {
        /* Return the model name in the given buffer */
        aim_strlcpy(modelname, model_name, modelname_len-1);
    }

    return ret;
}

