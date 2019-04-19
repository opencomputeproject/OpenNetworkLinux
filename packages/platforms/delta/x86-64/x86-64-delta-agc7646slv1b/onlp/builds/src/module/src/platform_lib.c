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
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

static onlp_shlock_t* dni_lock = NULL;

#define DNI_LOCK() \
    do{ \
        onlp_shlock_take(dni_lock); \
    }while(0)

#define DNI_UNLOCK() \
    do{ \
        onlp_shlock_give(dni_lock); \
    }while(0)

#define DNILOCK_MAGIC 0xA6D2B4E8

void lockinit()
{
    static int sem_inited = 0;
    if(!sem_inited)
    {
        onlp_shlock_create(DNILOCK_MAGIC, &dni_lock, "bus-lock");
        sem_inited = 1;
    }
}

int dni_ipmi_data_time_check(long last_time, long new_time, int time_threshold)
{
    int ipmi_data_update = 0;

    if(last_time == 0)
    {
        ipmi_data_update = 1;
    }
    else
    {
         if(new_time > last_time)
         {
             if((new_time - last_time) > time_threshold)
             {
                ipmi_data_update = 1;
             }
             else
                ipmi_data_update = 0;
         }
         else if(new_time == last_time)
         {
             ipmi_data_update = 0;
         }
         else
         {
             ipmi_data_update = 1;
         }
    }

    return ipmi_data_update;
}

int dni_check_file_exist(char *file_path, long *file_time)
{
    struct stat file_info;

    if(stat(file_path, &file_info) == 0)
    {
        if(file_info.st_size == 0)
        {
            return 0;
        }
        else
        {
            *file_time = file_info.st_mtime;
            return 1;
        }
    }
    else
    {
       return 1;
    }
}

bmc_info_t dev[] =
{
    {"Fantray_1_1", 0},
    {"Fantray_1_2", 0},
    {"Fantray_1_3", 0},
    {"Fantray_1_4", 0},
    {"Fantray_2_1", 0},
    {"Fantray_2_2", 0},
    {"Fantray_2_3", 0},
    {"Fantray_2_4", 0},
    {"PSU1_Fan", 0},
    {"PSU2_Fan", 0},
    {"PSU1_Vin", 0},
    {"PSU1_Vout", 0},
    {"PSU1_Iin", 0},
    {"PSU1_Iout", 0},
    {"PSU1_Pin",0},
    {"PSU1_Pout",0},
    {"PSU2_Vin", 0},
    {"PSU2_Vout", 0},
    {"PSU2_Iin", 0},
    {"PSU2_Iout", 0},
    {"PSU2_Pin",0},
    {"PSU2_Pout",0},
    {"Fan_Temp", 0},
    {"TMP75_CPU-4d", 0},
    {"TMP75_FAN-4f", 0},
    {"TMP75-4e", 0},
    {"TMP75-4b", 0},
    {"TMP431_REMOTE-4c", 0},
    {"TMP431_LOCAL-4c", 0},
    {"PSU1_Temp_1", 0},
    {"PSU2_Temp_1", 0}
};

check_time_t bmc_check = {0};

int dni_bmc_sensor_read(char *device_name, UINT4 *num, UINT4 multiplier, int sensor_type)
{
    struct timeval new_tv;
    FILE *fpRead = NULL;
    char ipmi_cmd[120] = {0};
    char get_data_cmd[120] = {0};
    char Buf[10];
    int rv = ONLP_STATUS_OK;
    int dev_num = 0;
    int time_threshold = 0;
    int ipmi_data_update = 0;
    float num_f = 0;
    long file_last_time = 0;

    switch(sensor_type)
    {
        case FAN_SENSOR:
            time_threshold = FAN_TIME_THRESHOLD;
            break;
        case PSU_SENSOR:
            time_threshold = PSU_TIME_THRESHOLD;
            break;
        case THERMAL_SENSOR:
            time_threshold = THERMAL_TIME_THRESHOLD;
            break;
    }

    if(dni_check_file_exist(BMC_INFO_TABLE, &file_last_time))
    {
        gettimeofday(&new_tv,NULL);
        if(dni_ipmi_data_time_check(file_last_time, new_tv.tv_sec, time_threshold))
        {
            ipmi_data_update = 1;
        }
        else
        {
            ipmi_data_update = 0;
        }
    }
    else
    {
        ipmi_data_update = 1;
    }

    if(bmc_check.time == 0 && dni_check_file_exist(BMC_INFO_TABLE, &file_last_time))
    {
        ipmi_data_update = 1;
        gettimeofday(&new_tv,NULL);
        bmc_check.time = new_tv.tv_sec;
    }

    if(ipmi_data_update == 1)
    {
        DNI_LOCK();
        if(dni_ipmi_data_time_check(file_last_time, bmc_check.time, time_threshold))
        {
            sprintf(ipmi_cmd, "ipmitool sdr > /tmp/bmc_info");
            system(ipmi_cmd);
        }

        for(dev_num = 0; dev_num < DEV_NUM; dev_num++)
        {
            memset(Buf, 0, sizeof(Buf));
            sprintf(get_data_cmd, "cat /tmp/bmc_info | grep %s | awk -F'|' '{print $2}' | awk -F' ' '{ print $1}'", dev[dev_num].tag);
            fpRead = popen(get_data_cmd, "r");
            if(fpRead != NULL)
            {
                if(fgets(Buf, sizeof(Buf), fpRead) != NULL)
                {
                    num_f = atof(Buf);
                    dev[dev_num].data = num_f;
                }
            }
            pclose(fpRead);
        }
        gettimeofday(&new_tv,NULL);
        bmc_check.time = new_tv.tv_sec;
        DNI_UNLOCK();
    }

    for(dev_num = 0; dev_num < DEV_NUM; dev_num++)
    {
        if(strstr(dev[dev_num].tag, device_name) != NULL)
        {
            *num = dev[dev_num].data * multiplier;
            rv =  ONLP_STATUS_OK;
            goto END;
        }
    }
END:
    return rv;
}

swpld_info_t swpld_table[]=
{
   {"SWPLD_1", 0x6a, 0},
   {"SWPLD_2", 0x75, 0},
};

int dni_bmc_data_get(int bus, int addr, int reg, int *r_data)
{
    char cmd[120] = {0};
    char data_path[120] = {0};
    char file_path[120] = {0};
    char buf[10] = {0};
    struct timeval new_tv;
    int rv = ONLP_STATUS_OK;
    int ipmi_data_update = 0;
    int swpld_num = 0;
    int div_val = 0;
    int rem_val = 0;
    int dis_val = 0;
    long file_last_time = 0;
    FILE *fp = NULL;

    div_val = (reg / 16);
    rem_val = (reg % 16);
    dis_val = (div_val * 45) + (div_val * 3) + div_val + 1  + (rem_val * 3);

    gettimeofday(&new_tv,NULL);

    for(swpld_num = 0; swpld_num < 2; swpld_num++)
    {
        if(swpld_table[swpld_num].addr == addr)
        {
            break;
        }
    }
    sprintf(file_path, "/tmp/%s_data", swpld_table[swpld_num].name);
    if(dni_check_file_exist(file_path, &file_last_time))
    {
        gettimeofday(&new_tv,NULL);
        if(dni_ipmi_data_time_check(file_last_time, new_tv.tv_sec, SWPLD_DATA_TIME_THRESHOLD))
        {
            ipmi_data_update = 1;
        }
        else
        {
            ipmi_data_update = 0;
        }
    }
    else
    {
        ipmi_data_update = 1;
    }

    if(ipmi_data_update == 1)
    {
        DNI_LOCK();
        swpld_table[swpld_num].time = new_tv.tv_sec;
        sprintf(cmd, "ipmitool raw 0x38 0x2 %d 0x%x 0x00 255 > /tmp/%s_data", bus, addr, swpld_table[swpld_num].name);
        system(cmd);
        DNI_UNLOCK();
    }

    sprintf(data_path, "/tmp/%s_data",swpld_table[swpld_num].name);
    fp = fopen(data_path,"r");
    if(fp != NULL)
    {
        fseek(fp, dis_val, SEEK_SET);
        if(fgets(buf, 4, fp) != NULL)
        {
            *r_data = strtol(buf, NULL, 16);
        }
    }
    pclose(fp);
    return rv;
}

int dni_bmc_data_set(int bus, int addr, int reg, uint8_t w_data)
{
    int rv = ONLP_STATUS_OK;
    char cmd[50] = {0};
    FILE *fptr = NULL;

    DNI_LOCK();
    sprintf(cmd, "ipmitool raw 0x38 0x3 %d 0x%x 0x%x %d > /dev/null", bus, addr, reg, w_data);
    fptr = popen(cmd, "w");
    if(fptr != NULL)
        rv = ONLP_STATUS_OK;
    else
        rv = ONLP_STATUS_E_INVALID;
    pclose(fptr);
    DNI_UNLOCK();
    return rv;
}

platform_info_t fan = {0, 0};

int dni_bmc_fanpresent_info_get(uint8_t *fan_present_bit)
{
    int ipmi_data_update = 0;
    int rv = ONLP_STATUS_OK;
    char fanpresent_cmd[120] = {0};
    char str_data[100] = {0};
    long present_bit = 0;
    FILE *fptr = NULL;
    struct timeval new_tv;

    gettimeofday(&new_tv,NULL);

    ipmi_data_update = dni_ipmi_data_time_check(fan.time, new_tv.tv_sec, FAN_TIME_THRESHOLD);

    if(ipmi_data_update == 1)
    {
        DNI_LOCK();
        fan.time = new_tv.tv_sec;
        sprintf(fanpresent_cmd, "ipmitool raw 0x38 0x0e");
        fptr = popen(fanpresent_cmd, "r");
        if(fptr != NULL)
        {
            if(fgets(str_data, sizeof(str_data), fptr) != NULL)
            {
                present_bit = strtol(str_data, NULL, 16);
                fan.data = present_bit;
            }
            else
                rv = ONLP_STATUS_E_INVALID;
        }
        else
            rv = ONLP_STATUS_E_INVALID;
        pclose(fptr);
        DNI_UNLOCK();
    }

    *fan_present_bit = fan.data;
    return rv;
}

onlp_psu_dev_t psu_eeprom_info_table[] =
{
    {
        {
            {"Product Name", {0}},
            {"Product Serial", {0}}
        }
    },
    {
        {
            {"Product Name", {0}},
            {"Product Serial", {0}}
        }
    }
};

check_time_t psu_eeprom_check = {0};

int dni_bmc_psueeprom_info_get(char * r_data, char *device_name, int number)
{
    struct timeval new_tv;
    int psu_num = 0;
    int table_num = 0;
    int chr_num = 0;
    int rv = ONLP_STATUS_OK;
    int ipmi_data_update = 0;
    long file_last_time = 0;
    FILE *fptr = NULL;
    char cmd[120] = {0};
    char get_cmd[120] = {0};
    char file_path[120] = {0};
    char buf;
    char* renewCh;

    gettimeofday(&new_tv,NULL);
    sprintf(file_path, "/tmp/psu%d_eeprom", number);
    if(dni_check_file_exist(file_path, &file_last_time))
    {
        gettimeofday(&new_tv,NULL);
        if(dni_ipmi_data_time_check(file_last_time, new_tv.tv_sec, PSU_EEPROM_TIME_THRESHOLD))
        {
            ipmi_data_update = 1;
        }
        else
        {
            ipmi_data_update = 0;
        }
    }
    else
    {
        ipmi_data_update = 1;
    }

    if(psu_eeprom_check.time == 0 && dni_check_file_exist(file_path, &file_last_time))/*onlpdump*/
    {
        ipmi_data_update = 1;
        gettimeofday(&new_tv,NULL);
        psu_eeprom_check.time = new_tv.tv_sec;
    }

    if(ipmi_data_update == 1)
    {
        DNI_LOCK();
        if(dni_ipmi_data_time_check(file_last_time, psu_eeprom_check.time, PSU_EEPROM_TIME_THRESHOLD))
        {
            for(psu_num = 1; psu_num < 3; psu_num++)
            {
                sprintf(cmd, "ipmitool fru print %d > /tmp/psu%d_eeprom", psu_num, psu_num);
                system(cmd);
            }
        }

        for(psu_num = 1; psu_num < 3; psu_num++)
        {
            for(table_num = 0; table_num < 2 ; table_num++)
            {
                sprintf(get_cmd, "cat /tmp/psu%d_eeprom | grep '%s' | awk -F':' '{print $2}'", psu_num, psu_eeprom_info_table[psu_num-1].psu_eeprom_table[table_num].tag);
                fptr = popen(get_cmd, "r");
                while((buf = fgetc(fptr)) != EOF)
                {
                    if(buf != ' ')
                    {
                        psu_eeprom_info_table[psu_num-1].psu_eeprom_table[table_num].data[chr_num] = buf;
                        chr_num++;
                    }
                }
                chr_num = 0;
                pclose(fptr);

            }
        }
        psu_eeprom_check.time = new_tv.tv_sec;
        DNI_UNLOCK();
    }
    for(table_num = 0; table_num < 2 ; table_num++)
    {
        if(strstr(psu_eeprom_info_table[number - 1].psu_eeprom_table[table_num].tag, device_name) != NULL)
        {
            for(chr_num = 0; chr_num < PSU_NUM_LENGTH; chr_num++)
            {
                r_data[chr_num] = psu_eeprom_info_table[number - 1].psu_eeprom_table[table_num].data[chr_num];
            }
            renewCh = strstr(r_data,"\n");
            if(renewCh)
                *renewCh= '\0';
            goto END;
        }
    }
END:
    return rv;
}

int dni_i2c_lock_read_attribute(mux_info_t * mux_info, char * fullpath)
{
    int fd, nbytes = 10, rv = -1;
    char r_data[10] = {0};

    DNI_LOCK();
    if ((fd = open(fullpath, O_RDONLY)) >= 0)
    {
        if ((read(fd, r_data, nbytes)) > 0)
        {
            rv = atoi(r_data);
        }
    }
    close(fd);
    DNI_UNLOCK();
    return rv;
}

int dni_fan_present(int id)
{
    int rv;
    uint8_t bit_data = 0;
    int data = 0;
    uint8_t present_bit = 0x00;

    rv = dni_bmc_fanpresent_info_get(&bit_data);
    if(rv == ONLP_STATUS_OK)
    {
        present_bit = bit_data;
        data = (present_bit & (1 << -(id - NUM_OF_LED_ON_MAIN_BROAD)));
        if(data == 0)
            rv = ONLP_STATUS_OK;
        else
            rv = ONLP_STATUS_E_INVALID;
    }
    else
       rv = ONLP_STATUS_E_INVALID;
    return rv;
}
