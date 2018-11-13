/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017 Delta Networks, Inc.
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
#include "platform_lib.h"
#include <onlp/onlp.h>
#include <time.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include "platform_lib.h"
#include <onlplib/i2c.h>
#include <onlplib/mmap.h>
#include <pthread.h>

int dni_get_bmc_data(char *device_name, UINT4 *num, UINT4 multiplier)
{
    FILE *fpRead;
    char Buf[ 10 ]={0};
    char ipmi_command[120] = {0};
    int lenth=10;
    float num_f;

    sprintf(ipmi_command, "ipmitool sdr get %s |grep 'Sensor Reading'| awk -F':' '{print $2}'| awk -F' ' '{ print $1}'",      device_name);
    fpRead = popen(ipmi_command, "r");

    if(fpRead == NULL){
        pclose(fpRead);
                return ONLP_STATUS_E_GENERIC;
    }
    fgets(Buf, lenth , fpRead);
    num_f = atof( Buf );
    *num = num_f * multiplier;
    pclose(fpRead);
    return ONLP_STATUS_OK;
}
int
dni_bmc_check()
{
    char cmd[30] = {0};
    char str_data[100] = {0};
    FILE *fptr = NULL;
    int rv = 0;
    sprintf(cmd, "ipmitool raw 0x38 0x1a 0x00");
    fptr = popen(cmd, "r");
    if(fptr != NULL)
    {
        if(fgets(str_data, sizeof(str_data), fptr) !=NULL)
        {
            rv = strtol(str_data, NULL, 16);
        }
        if( rv == 1)
            rv = BMC_OFF;
        else
            rv = BMC_ON;
        pclose(fptr);
    }
    else
    {
        pclose(fptr);
        rv = ONLP_STATUS_E_INVALID;
    }

    return rv;
}

int
dni_bmc_data_get(int bus, int addr, int reg, int len, int *r_data)
{
    int rv = ONLP_STATUS_OK;
    char cmd[50] = {0};
    char rdata[10] = {0};
    FILE *fptr = NULL;

    sprintf(cmd, "ipmitool raw 0x38 0x2 %d 0x%x 0x%x %d", bus ,addr, reg, len);
    fptr = popen(cmd, "r");
    if(fptr != NULL)
    {
        if(fgets(rdata, sizeof(rdata), fptr) != NULL){
            *r_data = strtol(rdata, NULL, 16);
        }
        else{
           rv = ONLP_STATUS_E_INVALID;
        }
        pclose(fptr);
    }
    else
    {
        pclose(fptr);
        rv = ONLP_STATUS_E_INVALID;
    }

    return rv;
}

int
dni_bmc_fanpresent_info_get(int *r_data)
{
    int rv = ONLP_STATUS_OK;
    char cmd[30] = {0};
    char str_data[100] = {0};
    FILE *fptr = NULL;

    sprintf(cmd, "ipmitool raw 0x38 0x0e");
    fptr = popen(cmd, "r");
    if(fptr != NULL)
    {
        if(fgets(str_data, sizeof(str_data), fptr) != NULL)
            *r_data = strtol(str_data, NULL, 16);
        else
           rv = ONLP_STATUS_E_INVALID;
        pclose(fptr);
    }
    else
    {
        pclose(fptr);
        rv = ONLP_STATUS_E_INVALID;
    }

    return rv;
}

int hex_to_int(char hex_input){
        int first = hex_input / 16 - 3;
        int second = hex_input % 16;
        int result = first*10 + second;
        if(result > 9) result--;
        return result;
}

int hex_to_ascii(char hex_high, char hex_low){
        int high = hex_to_int(hex_high) * 16;
        int low = hex_to_int(hex_low);
        return high+low;
}
int dni_psu_present(int *r_data)
{
    FILE *fptr        = NULL;
    int rv            = ONLP_STATUS_OK;
    char cmd[35]      = {0};
    char str_data[50] = {0};
    sprintf(cmd, "ipmitool raw 0x38 0x2 3 0x6a 0x03 1");
    fptr = popen(cmd, "r");
    if(fptr != NULL)
    {
        if(fgets(str_data, sizeof(str_data), fptr) != NULL)
            *r_data = strtol(str_data, NULL, 16);
        else
           rv = ONLP_STATUS_E_INVALID;
        pclose(fptr);
    }
    else
    {
        pclose(fptr);
        rv = ONLP_STATUS_E_INVALID;
    }
    return rv;
}

int
dni_psui_eeprom_info_get(char * r_data,char *device_name,int number)
{
    int i = 0;
    int rv            = ONLP_STATUS_OK;
    FILE *fptr        = NULL;
    char cmd[35]      = {0};
    char str_data[50] = {0} ;
    char buf;
    char* renewCh; 
    sprintf(cmd, "ipmitool fru print %d | grep '%s'  | awk -F':' '{print $2}'",number,device_name);
    fptr = popen(cmd, "r");
    while( (buf = fgetc(fptr)) != EOF) {
            if( buf != ' '){
                str_data[i] = buf;
                i++;
            }
    }
    for(i = 0; i < PSU_NUM_LENGTH; i++)
    {
        r_data[i] = str_data[i];
    }
    pclose(fptr);
    renewCh=strstr(r_data,"\n");
    if(renewCh)
        *renewCh= '\0';

    return rv;
}


int dni_i2c_lock_read( mux_info_t * mux_info, dev_info_t * dev_info)
{
    int r_data=0;
    pthread_mutex_lock(&mutex);

    if(dev_info->size == 1)
        r_data = onlp_i2c_readb(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->flags);
    else
        r_data = onlp_i2c_readw(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->flags);

    pthread_mutex_unlock(&mutex);
    return r_data;
}

int dni_i2c_lock_write( mux_info_t * mux_info, dev_info_t * dev_info)
{
    pthread_mutex_lock(&mutex);
    /* Write size */
    if(dev_info->size == 1)
        onlp_i2c_write(dev_info->bus, dev_info->addr, dev_info->offset, 1, &dev_info->data_8, dev_info->flags);
    else
        onlp_i2c_writew(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->data_16, dev_info->flags);

    pthread_mutex_unlock(&mutex);
    return 0;
}
int dni_i2c_lock_read_attribute(mux_info_t * mux_info, char * fullpath)
{
    int fd, len, nbytes = 10;
    char  r_data[10]   = {0};

    pthread_mutex_lock(&mutex);
    if ((fd = open(fullpath, O_RDONLY)) == -1)
    {
        goto ERROR;
    }
    if ((len = read(fd, r_data, nbytes)) <= 0)
    {
        goto ERROR;
    }
    close(fd);
    pthread_mutex_unlock(&mutex);
    return atoi(r_data);
ERROR:
    close(fd);
    pthread_mutex_unlock(&mutex);
    return -1;
}

int dni_i2c_lock_write_attribute(mux_info_t * mux_info, char * data,char * fullpath)
{
    int fd, len, nbytes = 10;
    pthread_mutex_lock(&mutex);
    /* Create output file descriptor */
    fd = open(fullpath, O_WRONLY,  0644);
    if (fd == -1)
    {
        goto ERROR;
    }
    len = write (fd, data, (ssize_t) nbytes);
    if (len != nbytes)
    {
        goto ERROR;
    }
    close(fd);
    pthread_mutex_unlock(&mutex);
    return 0;
ERROR:
    close(fd);
    pthread_mutex_unlock(&mutex);
    return -1;
}
/* Use this function to select MUX and read data on CPLD */
int dni_lock_cpld_read_attribute(char *cpld_path, int addr)
{
    int fd, len, nbytes = 10,data = 0;
    char r_data[10]   = {0};
    char address[10] = {0};
    char cpld_data_path[100] = {0};
    char cpld_addr_path[100] = {0};
    sprintf(cpld_data_path, "%s/swpld1_reg_value", cpld_path);
    sprintf(cpld_addr_path, "%s/swpld1_reg_addr", cpld_path);
    sprintf(address, "0x%02x", addr);
    pthread_mutex_lock(&mutex1);
    /* Create output file descriptor */
    fd = open(cpld_addr_path, O_WRONLY,  0644);
    if (fd == -1)
    {
        goto ERR_HANDLE;
    }

    len = write (fd, address, 4);
    if (len <= 0)
    {
      goto ERR_HANDLE;
    }
    close(fd);

    if ((fd = open(cpld_data_path, O_RDONLY)) == -1)
    {
        goto ERR_HANDLE;
    }

    if ((len = read(fd, r_data, nbytes)) <= 0)
    {
        goto ERR_HANDLE;
    }
    close(fd);
    pthread_mutex_unlock(&mutex1);
    sscanf(r_data, "%x", &data);
    return data;

    ERR_HANDLE:
    close(fd);
    pthread_mutex_unlock(&mutex1);
    return -1;
}
/* Use this function to select MUX and write data on CPLD */
int dni_lock_cpld_write_attribute(char *cpld_path, int addr, int data)
{
    int fd, len;
    char address[10] = {0};
    char datas[10] = {0};
    char cpld_data_path[100] = {0};
    char cpld_addr_path[100] = {0};

    sprintf(cpld_data_path, "%s/swpld1_reg_value", cpld_path);
    sprintf(cpld_addr_path, "%s/swpld1_reg_addr", cpld_path);
    sprintf(address, "0x%02x", addr);
    pthread_mutex_lock(&mutex1);
    /* Create output file descriptor */
    fd = open(cpld_addr_path, O_WRONLY,  0644);
    if (fd == -1)
    {
        goto ERR_HANDLE;
    }
    len = write(fd, address, 4);
    if(len <= 0)
    {
        goto ERR_HANDLE;
    }
    close(fd);

    fd = open(cpld_data_path, O_WRONLY,  0644);
    if (fd == -1)
    {
        goto ERR_HANDLE;
    }
    sprintf(datas, "0x%02x", data);
    len = write (fd, datas, 4);
    if(len <= 0)
    {
        goto ERR_HANDLE;
    }
    close(fd);
    pthread_mutex_unlock(&mutex1);

    return 0;

    ERR_HANDLE:
    close(fd);
    pthread_mutex_unlock(&mutex1);
    return -1;
}

int
dni_fanpresent_info_get(int *r_data)
{
    int rv = ONLP_STATUS_OK;
    char cmd[30] = {0};
    char str_data[100] = {0};
    FILE *fptr = NULL;

    sprintf(cmd, "ipmitool raw 0x38 0x0e");
    fptr = popen(cmd, "r");
    if(fptr != NULL)
    {
        if(fgets(str_data, sizeof(str_data), fptr) != NULL)
        {
            *r_data = strtol(str_data, NULL, 16);
        }
        else
        {
           rv = ONLP_STATUS_E_INVALID;
        }
        pclose(fptr);
    }
    else
    {
        pclose(fptr);
        rv = ONLP_STATUS_E_INVALID;
    }

    return rv;
}
int
dni_fan_present(int id){
    int rv;
    dev_info_t dev_info;
    int bit_data = 0;
    int data = 0;
    uint8_t bit = 0x00;
    uint8_t present_bit = 0x00;
    int fantray_present = 0;

    if(dni_bmc_check() == BMC_ON)
    {
        rv = dni_fanpresent_info_get(&bit_data);

        if(rv == ONLP_STATUS_OK)
        {
            present_bit = bit_data;
            data = (present_bit & ((bit+1) << -(id - 9)));
            if(data == 0)
                rv = ONLP_STATUS_OK;
            else
                rv = ONLP_STATUS_E_INVALID;
        }
        else{
           rv = ONLP_STATUS_E_INVALID;
        }
    }
    else if(dni_bmc_check() == BMC_OFF){
        switch(id){
            case LED_REAR_FAN_TRAY_1:
                dev_info.addr = FAN_TRAY_1;
                dev_info.bus = I2C_BUS_25;
                break;
            case LED_REAR_FAN_TRAY_2:
                dev_info.addr = FAN_TRAY_2;
                dev_info.bus = I2C_BUS_24;
                break;
            case LED_REAR_FAN_TRAY_3:
                dev_info.addr = FAN_TRAY_3;
                dev_info.bus = I2C_BUS_23;
                break;
            case LED_REAR_FAN_TRAY_4:
                dev_info.addr = FAN_TRAY_4;
                dev_info.bus = I2C_BUS_22;
                break;
            case LED_REAR_FAN_TRAY_5:
                dev_info.addr = FAN_TRAY_5;
                dev_info.bus = I2C_BUS_21;
                break;
        }
        fantray_present = dni_i2c_lock_read(NULL, &dev_info);
        if(fantray_present >= 0)
            rv = ONLP_STATUS_OK;
        else
            rv = ONLP_STATUS_E_INVALID;
    }
    else{
        rv =  ONLP_STATUS_E_INVALID;
    }
    return rv;
}
int dni_fan_speed_good()
{
    int rpm = 0, rpm1 = 0, speed_good = 0;

    rpm = dni_i2c_lock_read_attribute(NULL, FAN1_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN1_REAR);
    if(rpm != 0 && rpm != FAN_ZERO_RPM && rpm1 != 0 && rpm1 != FAN_ZERO_RPM)
        speed_good++;
    rpm = dni_i2c_lock_read_attribute(NULL, FAN2_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN2_REAR);
    if(rpm != 0 && rpm != FAN_ZERO_RPM && rpm1 != 0 && rpm1 != FAN_ZERO_RPM)
        speed_good++;
    rpm = dni_i2c_lock_read_attribute(NULL, FAN3_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN3_REAR);
    if(rpm != 0 && rpm != FAN_ZERO_RPM && rpm1 != 0 && rpm1 != FAN_ZERO_RPM)
        speed_good++;
    rpm = dni_i2c_lock_read_attribute(NULL, FAN4_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN4_REAR);
    if(rpm != 0 && rpm != FAN_ZERO_RPM && rpm1 != 0 && rpm1 != FAN_ZERO_RPM)
        speed_good++;
    rpm = dni_i2c_lock_read_attribute(NULL, FAN5_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN5_REAR);
    if(rpm != 0 && rpm != FAN_ZERO_RPM && rpm1 != 0 && rpm1 != FAN_ZERO_RPM)
        speed_good++;
    return speed_good;
}


int dni_i2c_read_attribute_binary(char *filename, char *buffer, int buf_size, int data_len)
{
    int fd;
    int len;

    if ((buffer == NULL) || (buf_size < 0)) {
        return -1;
    }

    if ((fd = open(filename, O_RDONLY)) == -1) {
        return -1;
    }

    if ((len = read(fd, buffer, buf_size)) < 0) {
        close(fd);
        return -1;
    }

    if ((close(fd) == -1)) {
        return -1;
    }

    if ((len > buf_size) || (data_len != 0 && len != data_len)) {
        return -1;
    }

    return 0;
}

int dni_i2c_read_attribute_string(char *filename, char *buffer, int buf_size, int data_len)
{
    int ret;

    if (data_len >= buf_size) {
            return -1;
    }

    ret = dni_i2c_read_attribute_binary(filename, buffer, buf_size-1, data_len);

    if (ret == 0) {
        buffer[buf_size-1] = '\0';
    }

    return ret;
}

