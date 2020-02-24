/************************************************************
 * <bsn.cl fy=2017 v=onl>
 *
 *        Copyright 2017 Delta Networks, Inc.
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <onlp/onlp.h>
#include <onlplib/i2c.h>

static onlp_shlock_t* dni_lock = NULL;

#define DNI_LOCK() \
    do{ \
        onlp_shlock_take(dni_lock); \
    }while(0)

#define DNI_UNLOCK() \
    do{ \
        onlp_shlock_give(dni_lock); \
    }while(0)

#define DNILOCK_MAGIC 0xB9D0E6A4

void lockinit()
{
    static int sem_inited = 0;
    if (!sem_inited)
    {
        onlp_shlock_create(DNILOCK_MAGIC, &dni_lock, "bus-lock");
        sem_inited = 1;
    }
}

vendor_dev_t thermal_dev_list[] =
{
    {"",           THERMAL_SENSOR, THERMAL_RESERVED,               0},
    {"TMP75-4F",   THERMAL_SENSOR, THERMAL_1_ON_MAIN_BOARD_TEMP,   0},
    {"TMP75-4A",   THERMAL_SENSOR, THERMAL_2_ON_MAIN_BOARD_TEMP,   0},
    {"TMP75-49",   THERMAL_SENSOR, THERMAL_3_ON_MAIN_BOARD_TEMP,   0},
    {"TMP75-4B",   THERMAL_SENSOR, THERMAL_4_ON_MAIN_BOARD_TEMP,   0},
    {"TMP75-48",   THERMAL_SENSOR, THERMAL_5_ON_MAIN_BOARD_TEMP,   0},
    {"Temp411-4C", THERMAL_SENSOR, THERMAL_6_ON_MAIN_BOARD_TEMP,   0},
    {"PSU1_Temp_1",THERMAL_SENSOR, THERMAL_7_ON_PSU1,              0},
    {"PSU1_Temp_2",THERMAL_SENSOR, THERMAL_8_ON_PSU1,              0},
    {"PSU2_Temp_1",THERMAL_SENSOR, THERMAL_9_ON_PSU2,              0},
    {"PSU2_Temp_2",THERMAL_SENSOR, THERMAL_10_ON_PSU2,             0}
};

vendor_dev_t fan_dev_list[] =
{
    {"",         FAN_SENSOR, FAN_RESERVED,       0},
    {"FanPWM_0", FAN_SENSOR, FAN_1_ON_FAN_BOARD, 0},
    {"FanPWM_1", FAN_SENSOR, FAN_2_ON_FAN_BOARD, 0},
    {"FanPWM_2", FAN_SENSOR, FAN_3_ON_FAN_BOARD, 0},
    {"FanPWM_3", FAN_SENSOR, FAN_4_ON_FAN_BOARD, 0},
    {"FanPSU1",  FAN_SENSOR, FAN_1_ON_PSU1,      0},
    {"FanPSU_2", FAN_SENSOR, FAN_1_ON_PSU2,      0}
};

vendor_dev_t psu_dev_list[] =
{
    {"",                PSU_SENSOR, PSU_RESERVED, 0},
    {"PSU1_Voltage_1",  PSU_SENSOR, PSU1_ID,      0},
    {"PSU1_Voltage_2",  PSU_SENSOR, PSU1_ID,      0},
    {"PSU1_Current_1",  PSU_SENSOR, PSU1_ID,      0},
    {"PSU_Current_2",   PSU_SENSOR, PSU1_ID,      0},
    {"PSU1_PWR_Con_1",  PSU_SENSOR, PSU1_ID,      0},
    {"PSU1_PWR_Con_2",  PSU_SENSOR, PSU1_ID,      0},
    {"PSU2_Voltage_1",  PSU_SENSOR, PSU2_ID,      0},
    {"PSU2_Voltage_2",  PSU_SENSOR, PSU2_ID,      0},
    {"PSU2_Current_1",  PSU_SENSOR, PSU2_ID,      0},
    {"PSU2_Current_2",  PSU_SENSOR, PSU2_ID,      0},
    {"PSU2_PWR_Con_1",  PSU_SENSOR, PSU2_ID,      0},
    {"PSU2_PWR_Con_2",  PSU_SENSOR, PSU2_ID,      0}
};

vendor_psu_dev_t psu_eeprom_info_table[] =
{
    {
        {
            {"Product Name",   {0}},
            {"Product Serial", {0}}
        }
    },
    {
        {
            {"Product Name",   {0}},
            {"Product Serial", {0}}
        }
    }
};

swpld_info_t swpld_table[]=
{
    {"SYSTEM_CPLD", 0x31, 0},
    {"PORT_CPLD0", 0x32, 0},
};

int dni_ipmi_data_time_check(long last_time, long new_time, int time_threshold)
{
    int ipmi_data_update = 0;

    if (last_time == 0)
        ipmi_data_update = 1;
    else
    {
         if (new_time > last_time)
         {
             if ((new_time - last_time) > time_threshold)
                ipmi_data_update = 1;
             else
                ipmi_data_update = 0;
         }
         else if (new_time == last_time)
             ipmi_data_update = 0;
         else
             ipmi_data_update = 1;
    }

    return ipmi_data_update;
}

int dni_check_file_exist(char *file_path, long *file_time)
{
    struct stat file_info;

    if (stat(file_path, &file_info) == 0)
    {
        if (file_info.st_size == 0)
            return 0;
        else
        {
            *file_time = file_info.st_mtime;
            return 1;
        }
    }
    else
       return 1;
}

check_time_t bmc_check = {0};

int dni_bmc_sensor_read(char *device_name, UINT4 *num, UINT4 multiplier, int sensor_type)
{
    struct timeval new_tv;
    FILE *fpRead = NULL;
    char ipmi_cmd[VENDOR_MAX_CMD_SIZE] = {0};
    char get_data_cmd[VENDOR_MAX_CMD_SIZE] = {0};
    char buf[VENDOR_MAX_BUF_SIZE];
    int rv = ONLP_STATUS_OK;
    int dev_list_index = 0;
    int time_threshold = 0;
    int ipmi_data_update = 0;
    float num_f = 0;
    long file_last_time = 0;

    switch (sensor_type)
    {
        case THERMAL_SENSOR:
            time_threshold = THERMAL_TIME_THRESHOLD;
            break;
        case FAN_SENSOR:
            time_threshold = FAN_TIME_THRESHOLD;
            break;
        case PSU_SENSOR:
            time_threshold = PSU_TIME_THRESHOLD;
            break;
    }

    /* Check if the /tmp/bmc_info is exist or not and use
       the current time to compare the time of temp file */
    if (dni_check_file_exist(BMC_INFO_TABLE, &file_last_time))
    {
        gettimeofday(&new_tv, NULL);
        if (dni_ipmi_data_time_check(file_last_time, new_tv.tv_sec, time_threshold))
            ipmi_data_update = 1;
        else
            ipmi_data_update = 0;
    }
    else
        ipmi_data_update = 1;

    /* A temp file is created but a new thread need to get the current time to compare */
    if (bmc_check.time == 0 && dni_check_file_exist(BMC_INFO_TABLE, &file_last_time))
    {
        ipmi_data_update = 1;
        gettimeofday(&new_tv, NULL);
        bmc_check.time = new_tv.tv_sec;
    }

    if (ipmi_data_update == 1)
    {
        DNI_LOCK();
        if (dni_ipmi_data_time_check(file_last_time, bmc_check.time, time_threshold))
        {
            sprintf(ipmi_cmd, "ipmitool sdr > /tmp/bmc_info");
            system(ipmi_cmd);
        }

        /* Parse thermal_dev_list data */
        for (dev_list_index = FIRST_DEV_INDEX; dev_list_index < THERMAL_LIST_SIZE; dev_list_index++)
        {
            memset(buf, 0, sizeof(buf));
            /* Ignore the empty dev_name */
            if (strcmp(thermal_dev_list[dev_list_index].dev_name, "") == 0)
                continue;
            sprintf(get_data_cmd, "cat /tmp/bmc_info | grep %s | awk -F'|' '{print $2}' | awk -F' ' '{ print $1}'",
                thermal_dev_list[dev_list_index].dev_name);
            fpRead = popen(get_data_cmd, "r");
            if (fpRead != NULL)
            {
                if (fgets(buf, sizeof(buf), fpRead) != NULL)
                {
                    num_f = atof(buf);
                    thermal_dev_list[dev_list_index].data = num_f;
                }
            }
            pclose(fpRead);
        }

        /* Parse fan_dev_list data */
        for (dev_list_index = FIRST_DEV_INDEX; dev_list_index < FAN_LIST_SIZE; dev_list_index++)
        {
            memset(buf, 0, sizeof(buf));
            sprintf(get_data_cmd, "cat /tmp/bmc_info | grep %s | awk -F'|' '{print $2}' | awk -F' ' '{ print $1}'",
                fan_dev_list[dev_list_index].dev_name);
            fpRead = popen(get_data_cmd, "r");
            if (fpRead != NULL)
            {
                if (fgets(buf, sizeof(buf), fpRead) != NULL)
                {
                    num_f = atof(buf);
                    fan_dev_list[dev_list_index].data = num_f;
                }
            }
            pclose(fpRead);
        }

        /* Parse psu_dev_list data */
        for (dev_list_index = FIRST_DEV_INDEX; dev_list_index < PSU_LIST_SIZE; dev_list_index++)
        {
            memset(buf, 0, sizeof(buf));
            sprintf(get_data_cmd, "cat /tmp/bmc_info | grep %s | awk -F'|' '{print $2}' | awk -F' ' '{ print $1}'",
                psu_dev_list[dev_list_index].dev_name);
            fpRead = popen(get_data_cmd, "r");
            if (fpRead != NULL)
            {
                if (fgets(buf, sizeof(buf), fpRead) != NULL)
                {
                    num_f = atof(buf);
                    psu_dev_list[dev_list_index].data = num_f;
                }
            }
            pclose(fpRead);
        }
        gettimeofday(&new_tv, NULL);
        bmc_check.time = new_tv.tv_sec;
        DNI_UNLOCK();
    }

    if (sensor_type == THERMAL_SENSOR)
    {
        for (dev_list_index = FIRST_DEV_INDEX; dev_list_index < THERMAL_LIST_SIZE; dev_list_index++)
        {
            if (strstr(thermal_dev_list[dev_list_index].dev_name, device_name) != NULL)
            {
                *num = thermal_dev_list[dev_list_index].data * multiplier;
                rv = ONLP_STATUS_OK;
                goto END;
            }
        }
    }
    else if (sensor_type == FAN_SENSOR)
    {
        for (dev_list_index = FIRST_DEV_INDEX; dev_list_index < FAN_LIST_SIZE; dev_list_index++)
        {
            if (strstr(fan_dev_list[dev_list_index].dev_name, device_name) != NULL)
            {
                *num = fan_dev_list[dev_list_index].data * multiplier;
                rv = ONLP_STATUS_OK;
                goto END;
            }
        }
    }
    else if (sensor_type == PSU_SENSOR)
    {
        for (dev_list_index = FIRST_DEV_INDEX; dev_list_index < PSU_LIST_SIZE; dev_list_index++)
        {
            if (strstr(psu_dev_list[dev_list_index].dev_name, device_name) != NULL)
            {
                *num = psu_dev_list[dev_list_index].data * multiplier;
                rv = ONLP_STATUS_OK;
                goto END;
            }
        }
    }

END:
    return rv;
}

int dni_bmc_data_get(int bus, int addr, int reg, int *r_data)
{
    char cmd[VENDOR_MAX_CMD_SIZE] = {0};
    char data_path[VENDOR_MAX_PATH_SIZE] = {0};
    char file_path[VENDOR_MAX_PATH_SIZE] = {0};
    char buf[VENDOR_MAX_BUF_SIZE] = {0};
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

    gettimeofday(&new_tv, NULL);

    for (swpld_num = 0; swpld_num < SWPLD_NUM; swpld_num++)
    {
        if (swpld_table[swpld_num].addr == addr)
            break;
    }
    sprintf(file_path, "/tmp/%s_data", swpld_table[swpld_num].name);
    if (dni_check_file_exist(file_path, &file_last_time))
    {
        gettimeofday(&new_tv, NULL);
        if (dni_ipmi_data_time_check(file_last_time, new_tv.tv_sec, SWPLD_DATA_TIME_THRESHOLD))
            ipmi_data_update = 1;
        else
            ipmi_data_update = 0;
    }
    else
        ipmi_data_update = 1;

    if (ipmi_data_update == 1)
    {
        DNI_LOCK();
        swpld_table[swpld_num].time = new_tv.tv_sec;
        sprintf(cmd, "ipmitool raw 0x38 0x2 %d 0x%x 0x00 255 > /tmp/%s_data", bus, addr, swpld_table[swpld_num].name);
        system(cmd);
        DNI_UNLOCK();
    }

    sprintf(data_path, "/tmp/%s_data", swpld_table[swpld_num].name);
    fp = fopen(data_path,"r");
    if (fp != NULL)
    {
        fseek(fp, dis_val, SEEK_SET);
        if (fgets(buf, 4, fp) != NULL)
            *r_data = strtol(buf, NULL, 16);
    }
    pclose(fp);
    return rv;
}

int dni_bmc_data_set(int bus, int addr, int reg, uint8_t w_data)
{
    int rv = ONLP_STATUS_OK;
    char cmd[VENDOR_MAX_CMD_SIZE] = {0};
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

platform_info_t fan_platform = {0, 0};

int dni_bmc_fanpresent_info_get(uint8_t *fan_present_bit)
{
    int ipmi_data_update = 0;
    int rv = ONLP_STATUS_OK;
    char fanpresent_cmd[VENDOR_MAX_CMD_SIZE] = {0};
    char str_data[VENDOR_MAX_DATA_SIZE] = {0};
    long present_bit = 0;
    FILE *fptr = NULL;
    struct timeval new_tv;

    gettimeofday(&new_tv, NULL);

    ipmi_data_update = dni_ipmi_data_time_check(fan_platform.time, new_tv.tv_sec, FAN_TIME_THRESHOLD);

    if (ipmi_data_update == 1)
    {
        DNI_LOCK();
        fan_platform.time = new_tv.tv_sec;
        sprintf(fanpresent_cmd, "ipmitool raw 0x38 0x0b 0x2");
        fptr = popen(fanpresent_cmd, "r");
        if (fptr != NULL)
        {
            if (fgets(str_data, sizeof(str_data), fptr) != NULL)
            {
                present_bit = strtol(str_data, NULL, 16);
                fan_platform.data = present_bit;
            }
            else
                rv = ONLP_STATUS_E_INVALID;
        }
        else
            rv = ONLP_STATUS_E_INVALID;
        pclose(fptr);
        DNI_UNLOCK();
    }
    *fan_present_bit = fan_platform.data;

    return rv;
}

check_time_t psu_eeprom_check = {0};

int dni_bmc_psueeprom_info_get(char *r_data, char *device_name, int number)
{
    struct timeval new_tv;
    int psu_num = 0;
    int table_num = 0;
    int chr_num = 0;
    int rv = ONLP_STATUS_OK;
    int ipmi_data_update = 0;
    long file_last_time = 0;
    FILE *fptr = NULL;
    char cmd[VENDOR_MAX_CMD_SIZE] = {0};
    char get_cmd[VENDOR_MAX_CMD_SIZE] = {0};
    char file_path[VENDOR_MAX_PATH_SIZE] = {0};
    char buf;
    char *renewCh;

    gettimeofday(&new_tv, NULL);
    sprintf(file_path, "/tmp/psu%d_eeprom", number);
    if (dni_check_file_exist(file_path, &file_last_time))
    {
        gettimeofday(&new_tv,NULL);
        if (dni_ipmi_data_time_check(file_last_time, new_tv.tv_sec, PSU_EEPROM_TIME_THRESHOLD))
            ipmi_data_update = 1;
        else
            ipmi_data_update = 0;
    }
    else
        ipmi_data_update = 1;

    if (psu_eeprom_check.time == 0 && dni_check_file_exist(file_path, &file_last_time))
    {
        ipmi_data_update = 1;
        gettimeofday(&new_tv, NULL);
        psu_eeprom_check.time = new_tv.tv_sec;
    }

    if (ipmi_data_update == 1)
    {
        DNI_LOCK();
        if (dni_ipmi_data_time_check(file_last_time, psu_eeprom_check.time, PSU_EEPROM_TIME_THRESHOLD))
        {
            for (psu_num = 1; psu_num <= PSU_EEPROM_NUM; psu_num++)
            {
                sprintf(cmd, "ipmitool fru print %d > /tmp/psu%d_eeprom", psu_num, psu_num);
                system(cmd);
            }
        }

        for (psu_num = 1; psu_num <= PSU_EEPROM_NUM; psu_num++)
        {
            for (table_num = 0; table_num < PSU_EEPROM_TABLE_NUM ; table_num++)
            {
                sprintf(get_cmd, "cat /tmp/psu%d_eeprom | grep '%s' | awk -F':' '{print $2}'", psu_num,
                    psu_eeprom_info_table[psu_num-1].psu_eeprom_table[table_num].name);
                fptr = popen(get_cmd, "r");
                while ((buf = fgetc(fptr)) != EOF)
                {
                    if (buf != ' ')
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

    for (table_num = 0; table_num < PSU_EEPROM_TABLE_NUM; table_num++)
    {
        if (strstr(psu_eeprom_info_table[number-1].psu_eeprom_table[table_num].name, device_name) != NULL)
        {
            for (chr_num = 0; chr_num < PSU_NUM_LENGTH; chr_num++)
                r_data[chr_num] = psu_eeprom_info_table[number-1].psu_eeprom_table[table_num].data[chr_num];
            renewCh = strstr(r_data, "\n");
            if (renewCh)
                *renewCh= '\0';
            goto END;
        }
    }

END:
    return rv;
}

int dni_i2c_lock_read_attribute(char *fullpath, int base)
{
    int fd, nbytes = VENDOR_MAX_BUF_SIZE;
    long rv = -1;
    char r_data[VENDOR_MAX_BUF_SIZE] = {0};

    DNI_LOCK();
    if ((fd = open(fullpath, O_RDONLY)) >= 0)
    {
        if ((read(fd, r_data, nbytes)) > 0)
        {
            if (base == ATTRIBUTE_BASE_DEC)
                rv = strtol(r_data, NULL, ATTRIBUTE_BASE_DEC);
            else if (base == ATTRIBUTE_BASE_HEX)
                rv = strtol(r_data, NULL, ATTRIBUTE_BASE_HEX);
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
    uint8_t present_bit = 0x00;

    rv = dni_bmc_fanpresent_info_get(&bit_data);
    if (rv == ONLP_STATUS_OK)
    {
        present_bit = bit_data;
        if (present_bit == 0x00)
            rv = ONLP_STATUS_OK;
        else
            rv = ONLP_STATUS_E_INVALID;
    }
    else
       rv = ONLP_STATUS_E_INVALID;
    return rv;
}

int dni_i2c_lock_read(mux_info_t * mux_info, dev_info_t * dev_info)
{
    int r_data = 0;

    DNI_LOCK();
    if(dev_info->size == 1)
        r_data = onlp_i2c_readb(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->flags);
    else
        r_data = onlp_i2c_readw(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->flags);

    DNI_UNLOCK();
    return r_data;
}

int dni_i2c_lock_write(mux_info_t * mux_info, dev_info_t * dev_info)
{
    DNI_LOCK();
    /* Write size */
    if(dev_info->size == 1)
        onlp_i2c_write(dev_info->bus, dev_info->addr, dev_info->offset, 1, &dev_info->data_8, dev_info->flags);
    else
        onlp_i2c_writew(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->data_16, dev_info->flags);

    DNI_UNLOCK();
    return 0;
}
