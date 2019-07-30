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
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-devices.h>
#include "platform_lib.h"

#define MAX_I2C_BUSSES               2
#define I2C_PSU_MAX_EEPROM_ADDR      1
#define I2C_BUFFER_MAXSIZE           16
#define I2C_PSU_MODEL_NAME_LEN       13
#define I2C_PSU_FAN_TYPE_STRING_LEN  4   /* only for CHICONY AC power supply*/

/*-- i2c slave address --*/
#define I2C_SLAVE_ADDR_PSU_1_EEPROM    0x52
#define I2C_SLAVE_ADDR_PSU_2_EEPROM    0x51

typedef struct platform_psu_eeprom_i2c
{
    unsigned char  i2c_eeprom_addr[I2C_PSU_MAX_EEPROM_ADDR];
} platfrom_psu_eeprom_i2c_t;

static platfrom_psu_eeprom_i2c_t psu_data[] = {
{},
{{I2C_SLAVE_ADDR_PSU_1_EEPROM}},
{{I2C_SLAVE_ADDR_PSU_2_EEPROM}}
};

/* FUNCTION NAME : I2C_nRead
 * PURPOSE : This function read the i2c slave device data
 * INPUT   : file,data_adr and count
 * OUTPUT  : buf
 * RETUEN  : I2C_SUCCESS/I2C_ERROR
 * NOTES   :
 */
int I2C_nRead
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
//		printf("%s(): failed to i2c_smbus_read_i2c_block_data(I2C_SLAVE=0x%x)\n", __FUNCTION__, i2c_addr);
        close(i2c_fd);
        return 1;
    }

    memcpy(buf, sendbuffer, size);

    close(i2c_fd);
    return 0;

}

/* FUNCTION NAME : I2C_nWrite
 * PURPOSE : This function write the data to I2C devices !
 * INPUT   : i2c_fd,data_adr ,buf and count
 * OUTPUT  :
 * RETUEN  : ret/I2C_ERROR
 * NOTES   :
 */
int I2C_nWrite
(unsigned int bus_id, unsigned char i2c_addr, unsigned char offset, unsigned int size, unsigned char * buf)
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

    ret = ioctl(i2c_fd, I2C_SLAVE, i2c_addr);

    if (0 != ret)
    {
        printf("%s(): failed to ioctl(I2C_SLAVE=0x%x)\n", __FUNCTION__, i2c_addr);
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

int two_complement_to_int(unsigned short data, unsigned char valid_bit, unsigned short mask)
{
    unsigned short  valid_data  = data & mask;
    int  is_negative = valid_data >> (valid_bit - 1);

    return is_negative ? (-(((~valid_data) & mask) + 1)) : valid_data;
}

/* Transfer the data format from chip value to decimal
 * The chip value is linear format data bytes. Please refer to datasheet to get more detail info.
 * The format:
 * Bit 0~10: mantissa
 * Bit 11~15: exponent
 * Input value: multiplier_value. We use this input value to extend the decimal to milli-x.
 */
int parse_literal_format(unsigned short val, int multiplier_value)
{
    int exponent, mantissa;

    exponent = two_complement_to_int(val >> 11, 5, 0x1f);
    mantissa = two_complement_to_int(val & 0x7ff, 11, 0x7ff);

    if (exponent >= 0)
    {
        return (mantissa << exponent) * multiplier_value;
    }
    else
    {
        return (mantissa * multiplier_value)/(1 << -exponent);
    }
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

#ifdef CRC_LOOK_UP_TABLE
char CRC_8_table[256] =
{
0x0,  0x7,  0xE,  0x9,  0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31,
0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9,
0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1,
0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, 0xB7, 0xB0, 0xB9, 0xBE,
0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16,
0x3,  0x4,  0xD,  0xA,  0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E, 0x87, 0x80,
0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8,
0xDD, 0xDA, 0xD3, 0xD4, 0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, 0x19, 0x1E, 0x17, 0x10,
0x5,  0x2,  0xB,  0xC,  0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F,
0x6A, 0x6D, 0x64, 0x63, 0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
0x6,  0x1,  0x8,  0xF,  0x1A, 0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7,
0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF,
0xFA, 0xFD, 0xF4
};//for x^8 + x^2 + x + 1
#endif

void crc_calc(char data_in, int *remainder)
{
    *remainder = ((*remainder) ^ (data_in))<<8;

#ifdef CRC_LOOK_UP_TABLE
    *remainder = CRC_8_table[*remainder];
#else
    int genpoly = (0x107<<7);//x^8 + x^2 + x + 1
    int msb = 1<<15;
    int i=0;

    for(i=8;i>0;i--)
    {
        if((*remainder & msb) != 0)
            *remainder = *remainder ^ genpoly;

        genpoly = genpoly>>1;
        msb = msb>>1;
    }
#endif

}

int as4600_54t_get_psu_type(int pid, char* modelname, int modelname_len)
{
    int i=0, ret=PSU_MODULE_TYPE_UNKNOWN; /* User insert a nuknown PSU or Not plugged in.*/
    unsigned int bus_id = 1;
    char model_name[I2C_PSU_MODEL_NAME_LEN+1] = {0}, fan_type[I2C_PSU_FAN_TYPE_STRING_LEN+1] = {0};

    for (i=0; i<I2C_PSU_MAX_EEPROM_ADDR; i++)
    {
        I2C_nRead(bus_id, psu_data[pid].i2c_eeprom_addr[i], 0x50, I2C_PSU_MODEL_NAME_LEN, (unsigned char*)model_name);
        model_name[I2C_PSU_MODEL_NAME_LEN]='\0';

        if (strncmp(model_name, "R12-300P1A", strlen("R12-300P1A")) == 0) /* CHICONY AC power supply */
        {
            I2C_nRead(bus_id, psu_data[pid].i2c_eeprom_addr[i], 0x40, I2C_PSU_FAN_TYPE_STRING_LEN, (unsigned char*)fan_type);
            fan_type[I2C_PSU_FAN_TYPE_STRING_LEN]='\0';

            if (strncmp(fan_type, "C302", strlen("C302")) == 0)
            {
                ret = PSU_MODULE_TYPE_AC_CHICONY_F2B;
            }
            /* Some old PSUs dont contain the string "C301", only have "1xxxx".
             * PM/PL said we can use the char "1" to verify, too.
             */
            else if((strncmp(fan_type, "C301", strlen("C301")) == 0) || (strncmp(fan_type, "1", 1) == 0))
            {
                ret = PSU_MODULE_TYPE_AC_CHICONY_B2F;
            }
            break;
        }

        if (strncmp(model_name, "um300d03-01G", strlen("um300d03-01G")) == 0) /* UMEC 48V DC power supply (F2B) */
        {
            ret = PSU_MODULE_TYPE_DC_UMEC_48V_F2B;
            break;
        }

        if (strncmp(model_name, "um300d03G", strlen("um300d03G")) == 0) /* UMEC 48V DC power supply (B2F)*/
        {
            ret = PSU_MODULE_TYPE_DC_UMEC_48V_B2F;
            break;
        }
    }

    if(modelname) {
        /* Return the model name in the given buffer */
        aim_strlcpy(modelname, model_name, modelname_len-1);
    }

    return ret;
}

