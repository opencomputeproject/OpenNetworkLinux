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
void dni_ipmi_sensor_get()
{
    FILE *pp;
    int dev_num;
    char buf[2000]={0};
    char dev_name_str[10];
    float sensor_data;

    pp = popen("cat /root/load_time_and_data.txt","r");
    //token = strtok(buf, "\n");
    dev_num = 0;
    while(!feof(pp)){
        if(fgets(buf,sizeof(buf),pp)!=NULL){
            sscanf(buf,"%s %*s %f", dev_name_str, &sensor_data);
            strcpy(dev_name[dev_num], dev_name_str);
            dev_sensor[dev_num] = sensor_data;
            dev_num++;
        }
    }
    pclose(pp);
}

void dni_ipmi_sensor_set()
{
    FILE *fPtr = NULL;
    int dev_num;
    char *token = NULL;    
    char buf[2000]={0};
    char dev_name_str[10];
    float sensor_data;

    fPtr = popen("ipmitool sdr", "r");

    if(fread(buf, 1,sizeof(buf),fPtr) > 0)
    {
        FILE *fp = fopen("/root/load_time_and_data.txt", "w");
        if (fp != NULL)
        {
            fputs(buf, fp);
            fclose(fp);
        }

        token = strtok(buf, "\n");
        dev_num = 1;

        while(token != NULL)
        {
            sscanf(token,"%s %*s %f", dev_name_str, &sensor_data);
            strcpy(dev_name[dev_num], dev_name_str);
            dev_sensor[dev_num] = sensor_data;
            dev_num++;
            token = strtok(NULL, "\n");
        }
    }
    else
    {
        printf("Read buf error!\n");
    }
    pclose(fPtr);
}

int dni_diff_time_()
{
    int t_previous  = 0;
    int t_diff = 0;       
    int t_current ;
    int file_exist = 0;
    char cat_data_command[120] = {0};                    

    t_current = time(NULL);
    
    sprintf(cat_data_command, "test -e /root/load_time_and_data.txt && echo \"1\" || echo \"0\"");
    cmd_get(cat_data_command, &file_exist);
 
    if (file_exist == 1){
        sprintf(cat_data_command, "head -n 1 /root/load_time_and_data.txt");
        cmd_get(cat_data_command, &t_previous);       
    }
    else{
        t_previous = 0;
    }
  
    t_diff = t_current - t_previous; 

    if ((t_diff > UPDATE_THRESHOLD) || file_exist == 0){
        
        t_previous = t_current;
        dni_ipmi_sensor_set();

        //load_time_and_data.txt : current time and "ipmitool sdr" table.
        //shell : sed -i '1 i\1513044459' /root/load_time_and_data.txt
        sprintf(cat_data_command, "sed -i '1 i\\%d' /root/load_time_and_data.txt", t_current);
        cmd_set(cat_data_command); 

        return ONLP_STATUS_OK;
    }
    else
    {
        dni_ipmi_sensor_get();
    }
    return ONLP_STATUS_OK;
}


int dni_sensor_get(char *device_name, UINT4 *num, UINT4 multiplier)

{
    int i;
    for (i = 0;i < NUM_OF_SENSORS; i++){
        if(strcmp(dev_name[i],device_name) == 0 ){
            *num = (UINT4)(dev_sensor[i] * multiplier);        
            return ONLP_STATUS_OK;
        }

    }
    return ONLP_STATUS_E_INVALID;
}



void cmd_get(char *cmd, int *num)
{
    FILE *fpRead;
    char Buf[ 12 ]={0};
    char command[120] = {0};                    

    //shell command:ipmitool sdr get PSU2_Iout|grep  'Sensor Reading'|awk -F':' '{print $2}'|awk -F' ' '{print $1}'
    sprintf(command, "%s",cmd);
    fpRead = popen(command, "r");
                     
    if(fpRead == NULL){
        pclose(fpRead);
        return ;
    }
    fgets(Buf, sizeof(Buf) , fpRead);
    *num = atoi( Buf );
    pclose(fpRead);
    return ;
}

void cmd_set(char *cmd)
{
    FILE *fp;
    char command[120] = {0};                    

    sprintf(command, "%s",cmd);
    fp = popen(command, "w");
                     
    if(fp == NULL){
        pclose(fp);
        return ;
    }

    pclose(fp);
    return ;
}

int
dni_fanpresent_info_get(int *r_data)
{
    int rv = ONLP_STATUS_OK;
    char cmd[30] = {0};
    char str_data[100] = {0};
    FILE *pFd = NULL;

    sprintf(cmd, "ipmitool raw 0x38 0x0e");
    pFd = popen(cmd, "r");
    if(pFd != NULL)
    {
        if(fgets(str_data, sizeof(str_data), pFd) != NULL)
        {
            *r_data = strtol(str_data, NULL, 16);
        }
        else
        {
           rv = ONLP_STATUS_E_INVALID;
        }
        pclose(pFd);
    }
    else
    {
        pclose(pFd);
        rv = ONLP_STATUS_E_INVALID;
    }

    return rv;
}

int hex_to_int(char c){
        int first = c / 16 - 3;
        int second = c % 16;
        int result = first*10 + second;
        if(result > 9) result--;
        return result;
}

int hex_to_ascii(char c, char d){
        int high = hex_to_int(c) * 16;
        int low = hex_to_int(d);
        return high+low;
}

int
dni_psui_eeprom_info_get(char *r_data, int psu_id, int psu_reg)
{
    int i             = 0;
    int rv            = ONLP_STATUS_OK;
    FILE *pFd         = NULL;
    char cmd[35]      = {0};
    char str_data[50] = {0};
    
    sprintf(cmd, "ipmitool raw 0x38 0x12 %d %d", psu_id, psu_reg);
    pFd = popen(cmd, "r");
    if(pFd != NULL)
    {
        if(fgets(str_data, sizeof(str_data), pFd) != NULL)
        {
            /*  "str_data"  :psu model name or serial number in hex code
             *   str_data(hex) ex:0e 44 50 53 2d 38 30 30 41 42 2d 31 36 20 44
             */
            for(i = 1; i < PSU_NUM_LENGTH; i++)
            {
                r_data[i] = hex_to_ascii(str_data[3*i + 1], str_data[3*i + 2]);
            }
        }
        else
        {
           rv = ONLP_STATUS_E_INVALID;
        }
        pclose(pFd);
    }
    else
    {
        pclose(pFd);
        rv = ONLP_STATUS_E_INVALID;
    }

    return rv;
}

