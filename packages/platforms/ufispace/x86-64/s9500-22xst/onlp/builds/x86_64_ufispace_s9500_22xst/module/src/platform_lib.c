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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/io.h>
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlplib/shlocks.h>
#include <AIM/aim.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "platform_lib.h"

const char * thermal_id_str[] = {
    "",
    "Temp_CPU",
    "Temp_MAC",
    "Temp_BMC",
    "Temp_100GCage",
    "Temp_DDR4",
    "Temp_FANCARD1",
    "Temp_FANCARD2",
    "PSU0_Temp",
    "PSU1_Temp",
    "CPU_PACKAGE",
    "CPU1",
    "CPU2",
    "CPU3",
    "CPU4",
    "HWM_Temp_AMB",
    "HWM_Temp_PHY1",
    "HWM_Temp_Heater"
};

const char * fan_id_str[] = {
    "",
    "FAN1_RPM",
    "FAN2_RPM",
    "FAN3_RPM",
    "PSU0_FAN",
    "PSU1_FAN",
};

const char * fan_id_presence_str[] = {
    "",
    "FAN1_PRSNT_H",
    "FAN2_PRSNT_H",
    "FAN3_PRSNT_H",
};

const char * psu_id_str[] = {
    "",
    "PSU0",
    "PSU1",
    "PSU0_Presence",
    "PSU1_Presence",
    "PSU0_POWEROK",
    "PSU1_POWEROK",
    "PSU0_VIN",
    "PSU0_VOUT",
    "PSU0_IIN",
    "PSU0_IOUT",
    "PSU1_VIN",
    "PSU1_VOUT",
    "PSU1_IIN",
    "PSU1_IOUT",
};

bmc_info_t bmc_cache[] =
{
    {"Temp_CPU", 0},
    {"Temp_MAC", 0},
    {"Temp_BMC", 0},
    {"Temp_100GCage", 0},
    {"Temp_DDR4", 0},
    {"Temp_FANCARD1", 0},
    {"Temp_FANCARD2", 0},
    {"PSU0_Temp", 0},
    {"PSU1_Temp", 0},
    {"FAN_1", 0},
    {"FAN_2", 0},
    {"FAN_3", 0},
    {"PSU0_FAN", 0},
    {"PSU1_FAN", 0},
    {"Fan1_Presence", 0},
    {"Fan2_Presence", 0},
    {"Fan3_Presence", 0},
    {"PSU0_Presence", 0},
    {"PSU1_Presence", 0},
    {"PSU0_POWEROK", 0},
    {"PSU1_POWEROK", 0},
    {"PSU0_VIN", 0},
    {"PSU0_VOUT", 0},
    {"PSU0_IIN",0},
    {"PSU0_IOUT",0},
    {"PSU1_VIN", 0},
    {"PSU1_VOUT", 0},
    {"PSU1_IIN", 0},
    {"PSU1_IOUT", 0},
    {"HWM_Temp_AMB", 0},
    {"HWM_Temp_PHY1", 0},
    {"HWM_Temp_Heater", 0}
};

static onlp_shlock_t* onlp_lock = NULL;

#define ONLP_LOCK() \
    do{ \
        onlp_shlock_take(onlp_lock); \
    }while(0)

#define ONLP_UNLOCK() \
    do{ \
        onlp_shlock_give(onlp_lock); \
    }while(0)

#define LOCK_MAGIC 0xA2B4C6D8

void lock_init()
{
    static int sem_inited = 0;
    if(!sem_inited)
    {
        onlp_shlock_create(LOCK_MAGIC, &onlp_lock, "bmc-file-lock");
        sem_inited = 1;
    }
}

int check_file_exist(char *file_path, long *file_time)
{
    struct stat file_info;

    if(stat(file_path, &file_info) == 0) {
        if(file_info.st_size == 0) {
            return 0;
        }
        else {
            *file_time = file_info.st_mtime;
            return 1;
        }
    }
    else {
       return 0;
    }
}

int bmc_cache_expired_check(long last_time, long new_time, int cache_time)
{
    int bmc_cache_expired = 0;

    if(last_time == 0) {
        bmc_cache_expired = 1;
    }
    else {
         if(new_time > last_time) {
             if((new_time - last_time) > cache_time) {
                bmc_cache_expired = 1;
             }
             else {
                bmc_cache_expired = 0;
             }
         }
         else if(new_time == last_time) {
             bmc_cache_expired = 0;
         }
         else {
             bmc_cache_expired = 1;
         }
    }

    return bmc_cache_expired;
}

int bmc_sensor_read(int bmc_cache_index, int sensor_type, float *data)
{
    struct timeval new_tv;
    FILE *fp = NULL;
    char ipmi_cmd[400] = {0};
    char get_data_cmd[120] = {0};
    char buf[20];
    int rv = ONLP_STATUS_OK;
    int dev_num = 0;
    int dev_size = sizeof(bmc_cache)/sizeof(bmc_cache[0]);
    int cache_time = 0;
    int bmc_cache_expired = 0;
    float f_rv = 0;
    long file_last_time = 0;
    static long bmc_cache_time = 0;
    char* presence_str = "Present";

    switch(sensor_type) {
        case FAN_SENSOR:
            cache_time = FAN_CACHE_TIME;
            break;
        case PSU_SENSOR:
            cache_time = PSU_CACHE_TIME;
            break;
        case THERMAL_SENSOR:
            cache_time = THERMAL_CACHE_TIME;
            break;
    }

    if(check_file_exist(BMC_SENSOR_CACHE, &file_last_time))
    {
        gettimeofday(&new_tv, NULL);
        if(bmc_cache_expired_check(file_last_time, new_tv.tv_sec, cache_time)) {
            bmc_cache_expired = 1;
        }
        else {
            bmc_cache_expired = 0;
        }
    }
    else {
        bmc_cache_expired = 1;
    }

    if(bmc_cache_time == 0 && check_file_exist(BMC_SENSOR_CACHE, &file_last_time)) {
        bmc_cache_expired = 1;
        gettimeofday(&new_tv,NULL);
        bmc_cache_time = new_tv.tv_sec;
    }

    //update cache
    if(bmc_cache_expired == 1)
    {
        ONLP_LOCK();
        if(bmc_cache_expired_check(file_last_time, bmc_cache_time, cache_time)) {
            snprintf(ipmi_cmd, sizeof(ipmi_cmd), CMD_BMC_SENSOR_CACHE);
            system(ipmi_cmd);
        }

        for(dev_num = 0; dev_num < dev_size; dev_num++)
        {
            memset(buf, 0, sizeof(buf));

            if( dev_num >= 14 && dev_num <=18 ) {
                /* FanX_Presence */
                snprintf(get_data_cmd, sizeof(get_data_cmd), CMD_BMC_CACHE_GET, bmc_cache[dev_num].name, 5);
                fp = popen(get_data_cmd, "r");
                if(fp != NULL)
                {
                    if(fgets(buf, sizeof(buf), fp) != NULL)
                    {
                        if( strstr(buf, presence_str) != NULL ) {
                            f_rv = 1;
                        } else {
                            f_rv = 0;
                        }
                        bmc_cache[dev_num].data = f_rv;
                    }
                }
                pclose(fp);
            } else {
                snprintf(get_data_cmd, sizeof(get_data_cmd), CMD_BMC_CACHE_GET, bmc_cache[dev_num].name, 2);

                fp = popen(get_data_cmd, "r");
                if(fp != NULL)
                {
                    if(fgets(buf, sizeof(buf), fp) != NULL) {
                        f_rv = atof(buf);
                        bmc_cache[dev_num].data = f_rv;
                    }
                }
                pclose(fp);
            }

        }
        gettimeofday(&new_tv,NULL);
        bmc_cache_time = new_tv.tv_sec;
        ONLP_UNLOCK();
    }

    //read from cache
    *data = bmc_cache[bmc_cache_index].data;

    return rv;
}

int
psu_thermal_get(onlp_thermal_info_t* info, int thermal_id)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
psu_fan_info_get(onlp_fan_info_t* info, int id)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
psu_vin_get(onlp_psu_info_t* info, int id)
{
    int i=0, token_idx=1, token_val=0;

    char cmd[48];
    char cmd_out[150];
    char* tokens[20];
    char delimiter[]=",";
    const char* sensor_str;

    memset(cmd, 0, sizeof(cmd));
    memset(cmd_out, 0, sizeof(cmd_out));

    sensor_str = (id == PSU_ID_PSU0 ? psu_id_str[PSU_ID_PSU0_VIN] : psu_id_str[PSU_ID_PSU1_VIN]);
    snprintf(cmd, sizeof(cmd), CMD_BMC_SDR_GET, sensor_str);

    //Get sensor info from BMC
    if (exec_cmd(cmd, cmd_out, sizeof(cmd_out)) < 0) {
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s\n", sensor_str);
        return ONLP_STATUS_E_INTERNAL;
    }

    //Check output is correct
    if (strnlen(cmd_out, sizeof(cmd_out))==0 ||
        strchr(cmd_out, ',')==NULL ||
        strstr(cmd_out, sensor_str)==NULL ){
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s, cmd=%s, out=%s\n", sensor_str, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    //parse cmd_out to tokens
    tokens[i++] = strtok(cmd_out, delimiter);
    while (tokens[i-1] != NULL) {
        tokens[i++] = strtok(NULL, delimiter);
    }

    //read token_idx field
    if (i>=token_idx) {
        token_val = (int) (atof(tokens[token_idx])*1000);
        info->mvin = token_val;
        info->caps |= ONLP_PSU_CAPS_VIN;
    } else {
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s, cmd=%s, out=%s\n", sensor_str, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
psu_vout_get(onlp_psu_info_t* info, int id)
{
    int i=0, token_idx=1, token_val=0;

    char cmd[48];
    char cmd_out[150];
    char* tokens[20];
    char delimiter[]=",";
    const char* sensor_str;

    memset(cmd, 0, sizeof(cmd));
    memset(cmd_out, 0, sizeof(cmd_out));

    sensor_str = (id == PSU_ID_PSU0 ? psu_id_str[PSU_ID_PSU0_VOUT] : psu_id_str[PSU_ID_PSU1_VOUT]);
    snprintf(cmd, sizeof(cmd), CMD_BMC_SDR_GET, sensor_str);

    //Get sensor info from BMC
    if (exec_cmd(cmd, cmd_out, sizeof(cmd_out)) < 0) {
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s\n", sensor_str);
        return ONLP_STATUS_E_INTERNAL;
    }

    //Check output is correct
    if (strnlen(cmd_out, sizeof(cmd_out))==0 ||
        strchr(cmd_out, ',')==NULL ||
        strstr(cmd_out, sensor_str)==NULL ){
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s, cmd=%s, out=%s\n", sensor_str, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    //parse cmd_out to tokens
    tokens[i++] = strtok(cmd_out, delimiter);
    while (tokens[i-1] != NULL) {
        tokens[i++] = strtok(NULL, delimiter);
    }

    //read token_idx field
    if (i>=token_idx) {
        token_val = (int) (atof(tokens[token_idx])*1000);
        info->mvout = token_val;
        info->caps |= ONLP_PSU_CAPS_VOUT;
    } else {
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s, cmd=%s, out=%s\n", sensor_str, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
psu_iin_get(onlp_psu_info_t* info, int id)
{
    int i=0, token_idx=1, token_val=0;

    char cmd[48];
    char cmd_out[150];
    char* tokens[20];
    char delimiter[]=",";
    const char* sensor_str;

    memset(cmd, 0, sizeof(cmd));
    memset(cmd_out, 0, sizeof(cmd_out));

    sensor_str = (id == PSU_ID_PSU0 ? psu_id_str[PSU_ID_PSU0_IIN] : psu_id_str[PSU_ID_PSU1_IIN]);
    snprintf(cmd, sizeof(cmd), CMD_BMC_SDR_GET, sensor_str);

    //Get sensor info from BMC
    if (exec_cmd(cmd, cmd_out, sizeof(cmd_out)) < 0) {
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s\n", sensor_str);
        return ONLP_STATUS_E_INTERNAL;
    }

    //Check output is correct
    if (strnlen(cmd_out, sizeof(cmd_out))==0 ||
        strchr(cmd_out, ',')==NULL ||
        strstr(cmd_out, sensor_str)==NULL ){
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s, cmd=%s, out=%s\n", sensor_str, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    //parse cmd_out to tokens
    tokens[i++] = strtok(cmd_out, delimiter);
    while (tokens[i-1] != NULL) {
        tokens[i++] = strtok(NULL, delimiter);
    }

    //read token_idx field
    if (i>=token_idx) {
        token_val = (int) (atof(tokens[token_idx])*1000);
        info->miin = token_val;
        info->caps |= ONLP_PSU_CAPS_IIN;
    } else {
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s, cmd=%s, out=%s\n", sensor_str, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
psu_iout_get(onlp_psu_info_t* info, int id)
{
    int i=0, token_idx=1, token_val=0;

    char cmd[48];
    char cmd_out[150];
    char* tokens[20];
    char delimiter[]=",";
    const char* sensor_str;

    memset(cmd, 0, sizeof(cmd));
    memset(cmd_out, 0, sizeof(cmd_out));

    sensor_str = (id == PSU_ID_PSU0 ? psu_id_str[PSU_ID_PSU0_IOUT] : psu_id_str[PSU_ID_PSU1_IOUT]);
    snprintf(cmd, sizeof(cmd), CMD_BMC_SDR_GET, sensor_str);

    //Get sensor info from BMC
    if (exec_cmd(cmd, cmd_out, sizeof(cmd_out)) < 0) {
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s\n", sensor_str);
        return ONLP_STATUS_E_INTERNAL;
    }

    //Check output is correct
    if (strnlen(cmd_out, sizeof(cmd_out))==0 ||
        strchr(cmd_out, ',')==NULL ||
        strstr(cmd_out, sensor_str)==NULL ){
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s, cmd=%s, out=%s\n", sensor_str, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    //parse cmd_out to tokens
    tokens[i++] = strtok(cmd_out, delimiter);
    while (tokens[i-1] != NULL) {
        tokens[i++] = strtok(NULL, delimiter);
    }

    //read token_idx field
    if (i>=token_idx) {
        token_val = (int) (atof(tokens[token_idx])*1000);
        info->miout = token_val;
        info->caps |= ONLP_PSU_CAPS_IOUT;
    } else {
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s, cmd=%s, out=%s\n", sensor_str, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
psu_pout_get(onlp_psu_info_t* info, int i2c_bus)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
psu_pin_get(onlp_psu_info_t* info, int i2c_bus)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
psu_eeprom_get(onlp_psu_info_t* info, int id)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
psu_fru_get(onlp_psu_info_t* info, int id)
{
    char cmd[100];
    char cmd_out[150];
    char fru_model[] = "Product Name";  //only Product Name can identify AC/DC
    char fru_serial[] = "Product Serial";

    //FRU (model)

    memset(cmd, 0, sizeof(cmd));
    memset(cmd_out, 0, sizeof(cmd_out));
    memset(info->model, 0, sizeof(info->model));

    snprintf(cmd, sizeof(cmd), CMD_FRU_INFO_GET, id, fru_model);

    //Get psu fru info (model) from BMC
    if (exec_cmd(cmd, cmd_out, sizeof(cmd_out)) < 0) {
        AIM_LOG_ERROR("unable to read fru info from BMC, fru id=%d, cmd=%s, out=%s\n", id, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    //Check output is correct
    if (strnlen(cmd_out, sizeof(cmd_out))==0){
        AIM_LOG_ERROR("unable to read fru info from BMC, fru id=%d, cmd=%s, out=%s\n", id, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    snprintf(info->model, sizeof(info->model), "%s", cmd_out);

    //FRU (serial)

    memset(cmd, 0, sizeof(cmd));
    memset(cmd_out, 0, sizeof(cmd_out));
    memset(info->serial, 0, sizeof(info->serial));

    snprintf(cmd, sizeof(cmd), CMD_FRU_INFO_GET, id, fru_serial);

    //Get psu fru info (model) from BMC
    if (exec_cmd(cmd, cmd_out, sizeof(cmd_out)) < 0) {
        AIM_LOG_ERROR("unable to read fru info from BMC, fru id=%d, cmd=%s, out=%s\n", id, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    //Check output is correct
    if (strnlen(cmd_out, sizeof(cmd_out))==0){
        AIM_LOG_ERROR("unable to read fru info from BMC, fru id=%d, cmd=%s, out=%s\n", id, cmd, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    snprintf(info->serial, sizeof(info->serial), "%s", cmd_out);

    return ONLP_STATUS_OK;
}

int
psu_present_get(int *pw_present, int id)
{
    FILE *fp = NULL;
    char buf[20];
    char get_data_cmd[120] = {0};
    char psu_present[20] = {0};
    int dev_num;
    int dev_size = sizeof(bmc_cache)/sizeof(bmc_cache[0]);
    char* presence_str = "Present";

    if (id == PSU_ID_PSU0) {
        strcpy(psu_present, "PSU0_Presence");
    } else if (id == PSU_ID_PSU1) {
        strcpy(psu_present, "PSU1_Presence");
    } else {
        return ONLP_STATUS_E_INTERNAL;
    }

    for (dev_num = 0; dev_num < dev_size; dev_num++) {
        if (strcmp(bmc_cache[dev_num].name, psu_present) != 0)
            continue;

        snprintf(get_data_cmd, sizeof(get_data_cmd), CMD_BMC_CACHE_GET, bmc_cache[dev_num].name, 5);
        fp = popen(get_data_cmd, "r");
        if (fp != NULL) {
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                if (strstr(buf, presence_str) != NULL) {
                    *pw_present = 1;
                } else {
                    *pw_present = 0;
                }
            }
        }
        pclose(fp);
    }

    return ONLP_STATUS_OK;
}

int
psu_pwgood_get(int *pw_good, int id)
{
    FILE *fp = NULL;
    char buf[20];
    char get_data_cmd[120] = {0};
    char psu_pwgood[20] = {0};
    int dev_num;
    int dev_size = sizeof(bmc_cache)/sizeof(bmc_cache[0]);
    char* pwgood_str = "Enabled";

    if (id == PSU_ID_PSU0) {
        strcpy(psu_pwgood, "PSU0_POWEROK");
    } else if (id == PSU_ID_PSU1) {
        strcpy(psu_pwgood, "PSU1_POWEROK");
    } else {
        return ONLP_STATUS_E_INTERNAL;
    }

    for (dev_num = 0; dev_num < dev_size; dev_num++) {
        if (strcmp(bmc_cache[dev_num].name, psu_pwgood) != 0)
            continue;

        snprintf(get_data_cmd, sizeof(get_data_cmd), CMD_BMC_CACHE_GET, bmc_cache[dev_num].name, 5);
        fp = popen(get_data_cmd, "r");
        if (fp != NULL) {
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                if (strstr(buf, pwgood_str) != NULL) {
                    *pw_good = 1;
                } else {
                    *pw_good = 0;
                }
            } else {
                pclose(fp);
                return ONLP_STATUS_E_INTERNAL;
            }
        } else {
            return ONLP_STATUS_E_INTERNAL;
        }
        pclose(fp);
        break;
    }

    return ONLP_STATUS_OK;
}

int
qsfp_present_get(int port, int *pres_val)
{
    int status, rc;
    int gpio_num = 0, gpio_base1 = 453, gpio_base2 = 452;
    uint8_t data[8];
    int data_len;

    memset(data, 0, sizeof(data));

    //for qsfp port 0&1
    if (port < (RJ45_NUM + SFP_NUM + SFP28_NUM) || port >= PORT_NUM) {
        AIM_LOG_ERROR("Invalid QSFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    //gpio_num = gpio_base - (port - SFP_NUM - SFP28_NUM);
    //only have 2 qsfp: 20,21
    gpio_num = (port == 20)?gpio_base1:gpio_base2;

    if ((rc = onlp_file_read(data, sizeof(data), &data_len,
            "/sys/class/gpio/gpio%d/value", gpio_num)) != ONLP_STATUS_OK) {
	    AIM_LOG_ERROR("onlp_file_read failed, error=%d, /sys/class/gpio/gpio%d/value",
            rc, gpio_num);
        return ONLP_STATUS_E_INTERNAL;
    }
    status = (int) strtol((char *)data, NULL, 0);

    *pres_val = !status;

    return ONLP_STATUS_OK;
}

int
sfp_present_get(int port, int *pres_val)
{
    int status, rc;
    int gpio_num = 0, gpio_base[3] = {383, 367, 355};
    uint8_t data[8];
    int data_len;

    memset(data, 0, sizeof(data));

    //for sfp port 4~19
    if (port >= 0 && port < 4) {
        AIM_LOG_ERROR("Invalid SFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    } else if (port < 12) {
        gpio_num = gpio_base[0] - (port - 4);
    } else if (port < 16) {
        gpio_num = gpio_base[2] - (port - 12);
    } else if (port < 20) {
        gpio_num = gpio_base[1] - (port - 16);
    } else {
        AIM_LOG_ERROR("Invalid SFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if ((rc = onlp_file_read(data, sizeof(data), &data_len,
            "/sys/class/gpio/gpio%d/value", gpio_num)) != ONLP_STATUS_OK) {
	    AIM_LOG_ERROR("onlp_file_read failed, error=%d, /sys/class/gpio/gpio%d/value",
            rc, gpio_num);
        return ONLP_STATUS_E_INTERNAL;
    }
    status = (int) strtol((char *)data, NULL, 0);

    *pres_val = !status;

    return ONLP_STATUS_OK;
}

int
sfp_rx_los_get(int port, int *ctrl_val)
{
    int status, rc;
    int gpio_num = 0, gpio_base[3] = {351, 335, 323};
    uint8_t data[8];
    int data_len;

    memset(data, 0, sizeof(data));

    //for sfp port 4~19
    if (port >= 0 && port < 4) {
        AIM_LOG_ERROR("Invalid SFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    } else if (port < 12) {
        gpio_num = gpio_base[0] - (port - 4);
    } else if (port < 16) {
        gpio_num = gpio_base[2] - (port - 12);
    } else if (port < 20) {
        gpio_num = gpio_base[1] - (port - 16);
    } else {
        AIM_LOG_ERROR("Invalid SFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if ((rc = onlp_file_read(data, sizeof(data), &data_len,
            "/sys/class/gpio/gpio%d/value", gpio_num)) != ONLP_STATUS_OK) {
	    AIM_LOG_ERROR("onlp_file_read failed, error=%d, /sys/class/gpio/gpio%d/value",
            rc, gpio_num);
        return ONLP_STATUS_E_INTERNAL;
    }
    status = (int) strtol((char *)data, NULL, 0);

    *ctrl_val = status;

    return ONLP_STATUS_OK;
}

int
sfp_tx_fault_get(int port, int *ctrl_val)
{
    int status, rc;
    int gpio_num = 0, gpio_base[3] = {447, 431, 419};
    uint8_t data[8];
    int data_len;

    memset(data, 0, sizeof(data));

    //for sfp port 4~19
    if (port >= 0 && port < 4) {
        AIM_LOG_ERROR("Invalid SFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    } else if (port < 12) {
        gpio_num = gpio_base[0] - (port - 4);
    } else if (port < 16) {
        gpio_num = gpio_base[2] - (port - 12);
    } else if (port < 20) {
        gpio_num = gpio_base[1] - (port - 16);
    } else {
        AIM_LOG_ERROR("Invalid SFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if ((rc = onlp_file_read(data, sizeof(data), &data_len,
            "/sys/class/gpio/gpio%d/value", gpio_num)) != ONLP_STATUS_OK) {
	    AIM_LOG_ERROR("onlp_file_read failed, error=%d, /sys/class/gpio/gpio%d/value",
            rc, gpio_num);
        return ONLP_STATUS_E_INTERNAL;
    }
    status = (int) strtol((char *)data, NULL, 0);

    *ctrl_val = status;

    return ONLP_STATUS_OK;
}

int
sfp_tx_disable_get(int port, int *ctrl_val)
{
    int status, rc;
    int gpio_num = 0, gpio_base[3] = {495, 479, 467};
    uint8_t data[8];
    int data_len;

    memset(data, 0, sizeof(data));

    //for sfp port 4~19
    if (port >= 0 && port < 4) {
        AIM_LOG_ERROR("Invalid SFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    } else if (port < 12) {
        gpio_num = gpio_base[0] - (port - 4);
    } else if (port < 16) {
        gpio_num = gpio_base[2] - (port - 12);
    } else if (port < 20) {
        gpio_num = gpio_base[1] - (port - 16);
    } else {
        AIM_LOG_ERROR("Invalid SFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if ((rc = onlp_file_read(data, sizeof(data), &data_len,
            "/sys/class/gpio/gpio%d/value", gpio_num)) != ONLP_STATUS_OK) {
	    AIM_LOG_ERROR("onlp_file_read failed, error=%d, /sys/class/gpio/gpio%d/value",
            rc, gpio_num);
        return ONLP_STATUS_E_INTERNAL;
    }
    status = (int) strtol((char *)data, NULL, 0);

    *ctrl_val = status;

    return ONLP_STATUS_OK;
}

int
qsfp_lp_mode_get(int port, int *ctrl_val)
{
    int status, rc;
    int gpio_num = 0, gpio_base1 = 457, gpio_base2 = 456;
    uint8_t data[8];
    int data_len;

    memset(data, 0, sizeof(data));

    //for qsfp port 0&1
    if (port < (RJ45_NUM + SFP_NUM + SFP28_NUM) || port >= PORT_NUM) {
        AIM_LOG_ERROR("Invalid QSFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    //gpio_num = gpio_base - (port - SFP_NUM - SFP28_NUM);
    //only have 2 qsfp: 20,21
    gpio_num = (port == 20)?gpio_base1:gpio_base2;

    if ((rc = onlp_file_read(data, sizeof(data), &data_len,
            "/sys/class/gpio/gpio%d/value", gpio_num)) != ONLP_STATUS_OK) {
	    AIM_LOG_ERROR("onlp_file_read failed, error=%d, /sys/class/gpio/gpio%d/value",
            rc, gpio_num);
        return ONLP_STATUS_E_INTERNAL;
    }
    status = (int) strtol((char *)data, NULL, 0);

    *ctrl_val = status;

    return ONLP_STATUS_OK;
}

int
qsfp_reset_get(int port, int *ctrl_val)
{
    int status, rc;
    int gpio_num = 0, gpio_base1 = 461, gpio_base2 = 460;
    uint8_t data[8];
    int data_len;

    memset(data, 0, sizeof(data));

    //for qsfp port 0&1
    if (port < (RJ45_NUM + SFP_NUM + SFP28_NUM) || port >= PORT_NUM) {
        AIM_LOG_ERROR("Invalid QSFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    //gpio_num = gpio_base - (port - SFP_NUM - SFP28_NUM);
    //only have 2 qsfp: 20,21
    gpio_num = (port == 20)?gpio_base1:gpio_base2;

    if ((rc = onlp_file_read(data, sizeof(data), &data_len,
            "/sys/class/gpio/gpio%d/value", gpio_num)) != ONLP_STATUS_OK) {
	    AIM_LOG_ERROR("onlp_file_read failed, error=%d, /sys/class/gpio/gpio%d/value",
            rc, gpio_num);
        return ONLP_STATUS_E_INTERNAL;
    }
    status = (int) strtol((char *)data, NULL, 0);

    *ctrl_val = status;

    return ONLP_STATUS_OK;
}

int
sfp_tx_disable_set(int port, int ctrl_val)
{
    int rc;
    int gpio_num = 0, gpio_base[3] = {495, 479, 467};

    
    //for sfp port 4~19
    if (port >= 0 && port < 4) {
        AIM_LOG_ERROR("Invalid SFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    } else if (port < 12) {
        gpio_num = gpio_base[0] - (port - 4);
    } else if (port < 16) {
        gpio_num = gpio_base[2] - (port - 12);
    } else if (port < 20) {
        gpio_num = gpio_base[1] - (port - 16);
    } else {
        AIM_LOG_ERROR("Invalid SFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if ((rc = onlp_file_write_int(ctrl_val, "/sys/class/gpio/gpio%d/value",
            gpio_num)) != ONLP_STATUS_OK) {
	    AIM_LOG_ERROR("onlp_file_write_int failed, error=%d, /sys/class/gpio/gpio%d/value",
            rc, gpio_num);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
qsfp_lp_mode_set(int port, int ctrl_val)
{
    int rc;
    int gpio_num = 0, gpio_base1 = 457, gpio_base2 = 456;

    //for qsfp port 0&1
    if (port < (RJ45_NUM + SFP_NUM + SFP28_NUM) || port >= PORT_NUM) {
        AIM_LOG_ERROR("Invalid QSFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    //gpio_num = gpio_base - (port - SFP_NUM - SFP28_NUM);
    //only have 2 qsfp: 20,21
    gpio_num = (port == 20)?gpio_base1:gpio_base2;

    if ((rc = onlp_file_write_int(ctrl_val, "/sys/class/gpio/gpio%d/value",
            gpio_num)) != ONLP_STATUS_OK) {
	    AIM_LOG_ERROR("onlp_file_write_int failed, error=%d, /sys/class/gpio/gpio%d/value",
            rc, gpio_num);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
qsfp_reset_set(int port, int ctrl_val)
{
    int rc;
    int gpio_num = 0, gpio_base1 = 461, gpio_base2 = 460;

    //for qsfp port 0&1
    if (port < (RJ45_NUM + SFP_NUM + SFP28_NUM) || port >= PORT_NUM) {
        AIM_LOG_ERROR("Invalid QSFP ports, port=%d\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    //gpio_num = gpio_base - (port - SFP_NUM - SFP28_NUM);
    //only have 2 qsfp: 20,21
    gpio_num = (port == 20)?gpio_base1:gpio_base2;

    if ((rc = onlp_file_write_int(ctrl_val, "/sys/class/gpio/gpio%d/value",
            gpio_num)) != ONLP_STATUS_OK) {
	    AIM_LOG_ERROR("onlp_file_write_int failed, error=%d, /sys/class/gpio/gpio%d/value",
            rc, gpio_num);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
system_led_set(onlp_led_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
fan_led_set(onlp_led_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
psu1_led_set(onlp_led_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
psu2_led_set(onlp_led_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
fan_tray_led_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
read_ioport(int addr, int *reg_val) {
    int ret;

    /*set r/w permission of all 65536 ports*/
    ret = iopl(3);
    if(ret < 0){
        AIM_LOG_ERROR("unable to read cpu cpld version, iopl enable error %d\n", ret);
        return ONLP_STATUS_E_INTERNAL;
    }
    *reg_val = inb(addr);

    /*set r/w permission of  all 65536 ports*/
    ret = iopl(0);
    if(ret < 0){
        AIM_LOG_ERROR("unable to read cpu cpld version, iopl disable error %d\n", ret);
        return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}

int
write_ioport(int addr, int val) {
    int ret;

    /*set r/w permission of all 65536 ports*/
    ret = iopl(3);
    if(ret < 0){
        AIM_LOG_ERROR("unable to read cpu cpld version, iopl enable error %d\n", ret);
        return ONLP_STATUS_E_INTERNAL;
    }
    outb(addr, val);

    /*set r/w permission of  all 65536 ports*/
    ret = iopl(0);
    if(ret < 0){
        AIM_LOG_ERROR("unable to read cpu cpld version, iopl disable error %d\n", ret);
        return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}

int
exec_cmd(char *cmd, char* out, int size) {
    FILE *fp;

    /* Open the command for reading. */
    fp = popen(cmd, "r");
    if (fp == NULL) {
        AIM_LOG_ERROR("Failed to run command %s\n", cmd );
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Read the output a line at a time - output it. */
    while (fgets(out, size-1, fp) != NULL) {
    }

    /* close */
    pclose(fp);

    return ONLP_STATUS_OK;
}

int
get_ipmitool_len(char *ipmitool_out){
    size_t str_len = 0, ipmitool_len = 0;

    str_len = strlen(ipmitool_out);
    if (str_len > 0) {
        ipmitool_len = str_len / 3;
    }
    return ipmitool_len;
}

int
parse_ucd_out(char *ucd_out, char *ucd_data, int start, int len){
    int i;
    char data[3];

    memset(data, 0, sizeof(data));

    for (i = 2; i < len; ++i) {
        data[0] = ucd_out[(start+i)*3 + 1];
        data[1] = ucd_out[(start+i)*3 + 2];
        //hex string to int
        ucd_data[i-2] = (int) strtol(data, NULL, 16);
    }
    return ONLP_STATUS_OK;
}

int
sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int cpu_cpld_ver_addr = 0x600, cpu_cpld_ver;
    int mb_cpld_ver_addr = 0x702, mb_cpld_ver;
    int mb_board_type = 0, mb_hw_rev = 0, mb_build_rev = 0, gpio_num, gpio_base = 319;
    int i, data_tmp, data_len;
    uint8_t data[8];
    char bios_out[32];
    char bmc_out1[8], bmc_out2[8], bmc_out3[8];
    char ucd_out[48];
    char ucd_ver[8];
    char ucd_date[8];
    int ucd_len=0;
    int rc=0;

    memset(bios_out, 0, sizeof(bios_out));
    memset(bmc_out1, 0, sizeof(bmc_out1));
    memset(bmc_out2, 0, sizeof(bmc_out2));
    memset(bmc_out3, 0, sizeof(bmc_out3));
    memset(ucd_out, 0, sizeof(ucd_out));
    memset(ucd_ver, 0, sizeof(ucd_ver));
    memset(ucd_date, 0, sizeof(ucd_date));

    //get CPU CPLD version
    if (read_ioport(cpu_cpld_ver_addr, &cpu_cpld_ver) < 0) {
        AIM_LOG_ERROR("unable to read CPU CPLD version\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    //get MB CPLD version
    if (read_ioport(mb_cpld_ver_addr, &mb_cpld_ver) < 0) {
        AIM_LOG_ERROR("unable to read MB CPLD version\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    pi->cpld_versions = aim_fstrdup(
        "\n"
        "[CPU CPLD] X.%02x\n"
        "[MB  CPLD] X.%02x\n",
        cpu_cpld_ver);

    //Get HW Build Version
    for (i = 0; i < 8; i++) {
        gpio_num = gpio_base - i;
        if ((rc = onlp_file_read(data, sizeof(data), &data_len,
            "/sys/class/gpio/gpio%d/value", gpio_num)) != ONLP_STATUS_OK) {
	        AIM_LOG_ERROR("onlp_file_read failed, error=%d, /sys/class/gpio/gpio%d/value",
                rc, gpio_num);
            return ONLP_STATUS_E_INTERNAL;
        }
        data_tmp = (int) strtol((char *)data, NULL, 0);

        if (i < 4) {
            mb_board_type = mb_board_type * 2 + data_tmp;
        } else if (i < 6) {
            mb_hw_rev = mb_hw_rev * 2 + data_tmp;
        } else {
            mb_build_rev = mb_build_rev * 2 + data_tmp;
        }
    }

    //Get BIOS version
    if (exec_cmd(CMD_BIOS_VER, bios_out, sizeof(bios_out)) < 0) {
        AIM_LOG_ERROR("unable to read BIOS version\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    //Get BMC version
    if (exec_cmd(CMD_BMC_VER_1, bmc_out1, sizeof(bmc_out1)) < 0 ||
        exec_cmd(CMD_BMC_VER_2, bmc_out2, sizeof(bmc_out2)) < 0 ||
        exec_cmd(CMD_BMC_VER_3, bmc_out3, sizeof(bmc_out3))) {
        AIM_LOG_ERROR("unable to read BMC version\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    //get UCD version
    if (exec_cmd(CMD_UCD_VER, ucd_out, sizeof(ucd_out)) < 0 ) {
        AIM_LOG_ERROR("unable to read UCD version\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    //parse UCD version and date
    ucd_len = get_ipmitool_len(ucd_out);
    parse_ucd_out(ucd_out, ucd_ver, 0, ucd_len);

    pi->other_versions = aim_fstrdup(
        "\n"
        "[HW   ] %d\n"
        "[BUILD] %d\n"
        "[BIOS ] %s\n"
        "[BMC  ] %d.%d.%d\n"
        "[UCD  ] %s\n",
        mb_hw_rev,
        mb_build_rev,
        bios_out,
        atoi(bmc_out1), atoi(bmc_out2), atoi(bmc_out3),
        ucd_ver);

    return ONLP_STATUS_OK;
}

bool
onlp_sysi_bmc_en_get(void)
{
//enable bmc by default
#if 0
    int value;

    if (onlp_file_read_int(&value, BMC_EN_FILE_PATH) < 0) {
        // flag file not exist, default to not enable
        return false;
    }

    /* 1 - enable, 0 - no enable */
    if ( value )
        return true;

    return false;
#endif
   return true;
}

int
parse_bmc_sdr_cmd(char *cmd_out, int cmd_out_size,
                  char *tokens[], int token_size,
                  const char *sensor_id_str, int *idx)
{
    char cmd[BMC_CMD_SDR_SIZE];
    char delimiter[]=",";
    char delimiter_c = ',';

    *idx=0;
    memset(cmd, 0, sizeof(cmd));
    memset(cmd_out, 0, cmd_out_size);

    snprintf(cmd, sizeof(cmd), CMD_BMC_SDR_GET, sensor_id_str);

    if (exec_cmd(cmd, cmd_out, cmd_out_size) < 0) {
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s\n", sensor_id_str);
        return ONLP_STATUS_E_INTERNAL;
    }

    //Check output is correct
    if (strnlen(cmd_out, cmd_out_size)==0 ||
        strchr(cmd_out, delimiter_c)==NULL ||
        strstr(cmd_out, sensor_id_str)==NULL ){
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%s, out=%s\n", sensor_id_str, cmd_out);
        return ONLP_STATUS_E_INTERNAL;
    }

    //parse cmd_out to tokens
    tokens[(*idx)++] = strtok(cmd_out, delimiter);
    while (tokens[(*idx)-1] != NULL) {
        tokens[(*idx)++] = strtok(NULL, delimiter);
    }

    return ONLP_STATUS_OK;
}

int
bmc_thermal_info_get(onlp_thermal_info_t* info, int id)
{
    int rc=0;
    float data=0;
    int presence = 0;

    //check presence for psu thermal
    if (id >= THERMAL_ID_PSU0 && id <= THERMAL_ID_PSU1) {
        rc = bmc_sensor_read(id + 9, THERMAL_SENSOR, &data);
        if (rc != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%d\n", id);
            return rc;
        }
        presence = (int) data;

        if (presence == 1) {
            info->status |= ONLP_THERMAL_STATUS_PRESENT;
        } else {
            info->status &= ~ONLP_THERMAL_STATUS_PRESENT;
            return ONLP_STATUS_OK;
        }
    }

    if (id >= THERMAL_ID_AMB && id <= THERMAL_ID_HEATER) {
        rc = bmc_sensor_read(id + 13, THERMAL_SENSOR, &data);
    } else {
        rc = bmc_sensor_read(id - 1, THERMAL_SENSOR, &data);
    }
    if ( rc != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%d\n", id);
        return rc;
    }
    info->mcelsius = (int) (data*1000);

    return rc;
}

int
bmc_fan_info_get(onlp_fan_info_t* info, int id)
{
    int rv = 0, rpm = 0, percentage = 0;
    int presence = 0;
    float data = 0;
    int sys_max_fan_speed = 25000;
    int psu_max_fan_speed = 19800;

    //check presence for fantray 1-3 and psu fan
    if (id >= FAN_ID_FAN1 && id <= FAN_ID_PSU1_FAN) {
        rv = bmc_sensor_read(id + 13, FAN_SENSOR, &data);
        if (rv != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%d\n", id);
            return rv;
        }
        presence = (int) data;

        if (presence == 1) {
            info->status |= ONLP_FAN_STATUS_PRESENT;
        } else {
            info->status &= ~ONLP_FAN_STATUS_PRESENT;
            return ONLP_STATUS_OK;
        }
    }

    //get fan rpm
    rv = bmc_sensor_read(id + 8, FAN_SENSOR, &data);
    if (rv != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("unable to read sensor info from BMC, sensor=%d\n", id);
        return rv;
    }
    rpm = (int) data;

    //set rpm field
    info->rpm = rpm;

    if (id >= FAN_ID_FAN1 && id <= FAN_ID_FAN3) {
        percentage = (info->rpm*100)/sys_max_fan_speed;
        info->percentage = percentage;
    } else if (id >= FAN_ID_PSU0_FAN && id <= FAN_ID_PSU1_FAN) {
        percentage = (info->rpm*100)/psu_max_fan_speed;
        info->percentage = percentage;
    }

    return ONLP_STATUS_OK;
}

int
file_read_hex(int* value, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = file_vread_hex(value, fmt, vargs);
    va_end(vargs);
    return rv;
}

int
file_vread_hex(int* value, const char* fmt, va_list vargs)
{
    int rv;
    uint8_t data[32];
    int len;
    rv = onlp_file_vread(data, sizeof(data), &len, fmt, vargs);
    if(rv < 0) {
        return rv;
    }
    //hex to int
    *value = (int) strtol((char *)data, NULL, 0);
    return 0;
}

int
sys_led_info_get(onlp_led_info_t* info, int id)
{
    int ctrl_addr, ctrl_onoff_mask, ctrl_color_mask;
    int blink_addr, blink_mask;
    int data;
    int led_val_color, led_val_blink, led_val_onoff;

    if (id == LED_ID_SYS_SYS) {
        ctrl_addr = LED_CTRL_REG;
        ctrl_onoff_mask = LED_SYS_ONOFF_MASK;
        ctrl_color_mask = LED_SYS_COLOR_MASK;
        blink_addr = LED_BLINKING_REG;
        blink_mask = LED_SYS_BLINK_MASK;
    } else if (id == LED_ID_SYS_SYNC) {
        ctrl_addr = LED_CTRL_REG;
        ctrl_onoff_mask = LED_SYNC_ONOFF_MASK;
        ctrl_color_mask = LED_SYNC_COLOR_MASK;
        blink_addr = LED_BLINKING_REG;
        blink_mask = LED_SYNC_BLINK_MASK;
    } else if (id == LED_ID_SYS_GPS) {
        ctrl_addr = LED_CTRL_REG;
        ctrl_onoff_mask = LED_GPS_ONOFF_MASK;
        ctrl_color_mask = LED_GPS_COLOR_MASK;
        blink_addr = LED_BLINKING_REG;
        blink_mask = LED_GPS_BLINK_MASK;
    } else {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get control status */
    if (read_ioport(ctrl_addr, &data) < 0) {
        AIM_LOG_ERROR("unable to read LED control register\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    led_val_onoff = data & ctrl_onoff_mask;
    led_val_color = data & ctrl_color_mask;
    /* Get blinking status */
    if (read_ioport(blink_addr, &data) < 0) {
        AIM_LOG_ERROR("unable to read LED control register\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    led_val_blink = data & blink_mask;

    //onoff
    if (led_val_onoff == 0) {
        info->mode = ONLP_LED_MODE_OFF;
    } else {
        //color
        if (led_val_color == 0) {
            info->mode = ONLP_LED_MODE_YELLOW;
        } else {
            info->mode = ONLP_LED_MODE_GREEN;
        }
        //blinking
        if (led_val_blink > 0) {
            info->mode = info->mode + 1;
        }
    }

    return ONLP_STATUS_OK;
}

int
sys_led_set(int id, int on_or_off)
{
    int ctrl_addr, ctrl_onoff_mask;
    int data;

    if (id == LED_ID_SYS_SYS) {
        ctrl_addr = LED_CTRL_REG;
        ctrl_onoff_mask = LED_SYS_ONOFF_MASK;
    } else if (id == LED_ID_SYS_SYNC) {
        ctrl_addr = LED_CTRL_REG;
        ctrl_onoff_mask = LED_SYNC_ONOFF_MASK;
    } else if (id == LED_ID_SYS_GPS) {
        ctrl_addr = LED_CTRL_REG;
        ctrl_onoff_mask = LED_GPS_ONOFF_MASK;
    } else {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get control status */
    if (read_ioport(ctrl_addr, &data) < 0) {
        AIM_LOG_ERROR("unable to read LED control register\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    if (on_or_off) {
        data |= ctrl_onoff_mask;
    } else {
        data &= ~ctrl_onoff_mask;
    }

    /* Set control status */
    if (write_ioport(ctrl_addr, data) < 0) {
        AIM_LOG_ERROR("unable to write LED control register\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
sys_led_mode_set(int id, int color, int blink)
{
    int ctrl_addr, ctrl_color_mask;
    int blink_addr, blink_mask;
    int data;

    if (id == LED_ID_SYS_SYS) {
        ctrl_addr = LED_CTRL_REG;
        ctrl_color_mask = LED_SYS_COLOR_MASK;
        blink_addr = LED_BLINKING_REG;
        blink_mask = LED_SYS_BLINK_MASK;
    } else if (id == LED_ID_SYS_SYNC) {
        ctrl_addr = LED_CTRL_REG;
        ctrl_color_mask = LED_SYNC_COLOR_MASK;
        blink_addr = LED_BLINKING_REG;
        blink_mask = LED_SYNC_BLINK_MASK;
    } else if (id == LED_ID_SYS_GPS) {
        ctrl_addr = LED_CTRL_REG;
        ctrl_color_mask = LED_GPS_COLOR_MASK;
        blink_addr = LED_BLINKING_REG;
        blink_mask = LED_GPS_BLINK_MASK;
    } else {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get control status */
    if (read_ioport(ctrl_addr, &data) < 0) {
        AIM_LOG_ERROR("unable to read LED control register\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    if (color == LED_COLOR_GREEN) {
        data |= ctrl_color_mask;
    } else {
        data &= ~ctrl_color_mask;
    }

    /* Set control status */
    if (write_ioport(ctrl_addr, data) < 0) {
        AIM_LOG_ERROR("unable to write LED control register\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get blinking status */
    if (read_ioport(blink_addr, &data) < 0) {
        AIM_LOG_ERROR("unable to read LED blinking register\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    if (blink == LED_BLINKING) {
        data |= blink_mask;
    } else {
        data &= ~blink_mask;
    }

    /* Set blinking status */
    if (write_ioport(blink_addr, data) < 0) {
        AIM_LOG_ERROR("unable to write LED blinking register\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
