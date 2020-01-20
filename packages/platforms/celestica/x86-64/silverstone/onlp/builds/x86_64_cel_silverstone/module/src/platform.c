//////////////////////////////////////////////////////////////
//   PLATFORM FUNCTION TO INTERACT WITH SYS_CPLD AND BMC    //
//////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/io.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#include "platform.h"

char command[256];
FILE *fp;

static struct device_info fan_information[FAN_COUNT + 1] = {
    {"unknown", "unknown",1}, //check
    {}, //Fan 1
    {}, //Fan 2
    {}, //Fan 3
    {}, //Fan 4
    {},
    {},
    {},
    {},
    {},
};

static struct device_info psu_information[PSU_COUNT + 1] = {
    {"unknown", "unknown"}, //check
    {}, //PSU 1
    {}, //PSU 2
};

static const struct led_reg_mapper led_mapper[LED_COUNT + 1] = {
    {},
    {"LED_SYSTEM", LED_SYSTEM_H, LED_SYSTEM_REGISTER},
    {"LED_ALARM", LED_ALARM_H, ALARM_REGISTER},
    {"LED_PSU", LED_PSU_H, PSU_LED_REGISTER},
    {"LED_FAN",LED_FAN_H,LED_FAN_REGISTER}
};

static const struct psu_reg_bit_mapper psu_mapper [PSU_COUNT + 1] = {
    {},
    {0xa160, 3, 7, 1},
    {0xa160, 2, 6, 0},
};

void update_shm_mem(void)
{
    (void)fill_shared_memory(ONLP_SENSOR_CACHE_SHARED, ONLP_SENSOR_CACHE_SEM, ONLP_SENSOR_CACHE_FILE);
    (void)fill_shared_memory(ONLP_FRU_CACHE_SHARED, ONLP_FRU_CACHE_SEM, ONLP_FRU_CACHE_FILE);
    (void)fill_shared_memory(ONLP_SENSOR_LIST_CACHE_SHARED, ONLP_SENSOR_LIST_SEM, ONLP_SENSOR_LIST_FILE);
}

int is_cache_exist(){
    const char *sdr_cache_path="/tmp/onlp-sensor-list-cache.txt";
    const char *fru_cache_path="/tmp/onlp-fru-cache.txt";
    const char *time_setting_path="/var/opt/interval_time.txt";
    time_t current_time;
    int interval_time = 30; //set default to 30 sec
    double sdr_diff_time,fru_diff_time;
    struct stat sdr_fst,fru_fst;
    bzero(&sdr_fst,sizeof(sdr_fst));
    bzero(&fru_fst,sizeof(fru_fst));

    //Read setting
    if(access(time_setting_path, F_OK) == -1){ //Setting not exist
        return -1;
    }else{
        FILE *fp;
        
        fp = fopen(time_setting_path, "r"); // read setting
        
        if (fp == NULL)
        {
            perror("Error while opening the file.\n");
            exit(EXIT_FAILURE);
        }

        fscanf(fp,"%d", &interval_time);

        fclose(fp);
    }

    if ((access(sdr_cache_path, F_OK) == -1) && (access(fru_cache_path, F_OK) == -1)){ //Cache not exist
        return -1;
    }else{ //Cache exist
        current_time = time(NULL);
        if (stat(sdr_cache_path,&sdr_fst) != 0) { printf("stat() sdr_cache failed\n"); return -1; }
        if (stat(fru_cache_path,&fru_fst) != 0) { printf("stat() fru_cache failed\n"); return -1; }

        sdr_diff_time = difftime(current_time,sdr_fst.st_mtime);
        fru_diff_time = difftime(current_time,fru_fst.st_mtime);

        if((sdr_diff_time > interval_time)&&(fru_diff_time > interval_time)){
            return -1;
        }
        return 1;
    }
}

int is_shm_mem_ready(){

    if(USE_SHM_METHOD){
        const char *sdr_cache_path="/run/shm/onlp-sensor-list-cache-shared";
        const char *fru_cache_path="/run/shm/onlp-fru-cache-shared";

        if(access(fru_cache_path, F_OK) == -1 || access(sdr_cache_path, F_OK) == -1 ){ //Shared cache files not exist
            return 0;
        }

    return 1;
    }
    
    return 0;
}

int create_cache(){
    (void)system("ipmitool fru > /tmp/onlp-fru-cache.tmp; sync; rm -f /tmp/onlp-fru-cache.txt; mv /tmp/onlp-fru-cache.tmp /tmp/onlp-fru-cache.txt");
    (void)system("ipmitool sensor list > /tmp/onlp-sensor-list-cache.tmp; sync; rm -f /tmp/onlp-sensor-list-cache.txt; mv /tmp/onlp-sensor-list-cache.tmp /tmp/onlp-sensor-list-cache.txt");
    if(USE_SHM_METHOD){
        update_shm_mem();
    }
    return 1;
}

void array_trim(char *strIn, char *strOut)
{
    int i, j;

    i = 0;
    j = strlen(strIn) - 1;

    while(strIn[i] == ' ') ++i;
    while(strIn[j] == ' ') --j;

    strncpy(strOut, strIn + i , j - i + 1);
    strOut[j - i + 1] = '\0';
}

uint8_t read_register(uint16_t dev_reg)
{
    int status;
    sprintf(command, "echo 0x%x >  %sgetreg", dev_reg, SYS_CPLD_PATH);
    fp = popen(command, "r");
    if (!fp)
    {
        printf("Failed : Can't specify CPLD register\n");
        return -1;
    }
    pclose(fp);
    fp = popen("cat " SYS_CPLD_PATH "getreg", "r");
    if (!fp)
    {
        printf("Failed : Can't open sysfs\n");
        return -1;
    }
    fscanf(fp, "%x", &status);
    pclose(fp);

    return status;
}

int exec_ipmitool_cmd(char *cmd, char *retd)
{
    int ret = 0;
    int i = 0;
    char c;
    FILE *pFd = NULL;

    pFd = popen(cmd, "r");
    if (pFd != NULL)
    {
        c = fgetc(pFd);
        while (c != EOF)
        {
            //printf ("%c", c);
            retd[i] = c;
            i++;
            c = fgetc(pFd);
        }
        pclose(pFd);
    }

    return ret;
}

uint8_t get_led_status(int id)
{
    uint8_t ret = 0xFF;

    if (id >= (LED_COUNT + 2) || id < 0)
        return 0xFF;

    if (id <= (LED_COUNT))
    {
        uint8_t result = 0;
        uint16_t led_stat_reg;
        led_stat_reg = led_mapper[id].dev_reg; 
        result = read_register(led_stat_reg);     
        ret = result;
    }

    return ret;
}

char *read_tmp_cache(char *cmd, char *cache_file_path)
{
    FILE* pFd = NULL;
    char *str = NULL;
    int round = 1;

    for(round = 1;round <= 10;round++){
        pFd = fopen(cache_file_path, "r");
        if(pFd != NULL ){

            struct stat st;

            stat(cache_file_path, &st);

            int size = st.st_size;
            str = (char *)malloc(size + 1);
            
            memset (str, 0, size+1);

            fread(str, size+1, 1, pFd);

            fclose(pFd);
            break;
        }else{
            usleep(5000); //Sleep for 5 Microsec for waiting the file operation complete
        }
    }

    if(round >= 10 && str == NULL){
        str = (char *)malloc(1);
        memset (str, 0, 1);
    }
    
    return str;
}

uint8_t get_psu_status(int id)
{
    uint8_t ret = 0xFF;
    uint16_t psu_stat_reg;

    if (id <= (PSU_COUNT))
    {
        uint8_t result = 0;
        psu_stat_reg = psu_mapper[id].sta_reg;
        result = read_register(psu_stat_reg);
        ret = result;
    }

    return ret;
}

int get_psu_info(int id, int *mvin, int *mvout, int *mpin, int *mpout, int *miin, int *miout)
{
    char *tmp = (char *)NULL;
    int len = 0;
    int index  = 0;

	int i = 0;
	int ret = 0;
    char strTmp[12][128] = {{0}, {0}};
    char *token = NULL;
    char *psu_sensor_name[12] = {
        "PSU1_VIn", "PSU1_CIn", "PSU1_PIn", "PSU1_VOut",
        "PSU1_COut", "PSU1_POut", "PSU2_VIn", "PSU2_CIn",
        "PSU2_PIn", "PSU2_VOut","PSU2_COut","PSU2_POut"};

    /*
        String example:			  
        root@localhost:~# ipmitool sensor list | grep PSU
        PSU1_Status      | 0x0        | discrete   | 0x0180| na        | na        | na        | na        | na        | na        
        PSU2_Status      | 0x0        | discrete   | 0x0180| na        | na        | na        | na        | na        | na        
        PSU1_Fan         | na         | RPM        | na    | na        | na        | na        | na        | na        | na        
        PSU2_Fan         | 15800.000  | RPM        | ok    | na        | na        | na        | na        | na        | na        
        PSU1_VIn         | na         | Volts      | na    | na   0     | na        | na        | 239.800   | 264.000   | na        
        PSU1_CIn         | na         | Amps       | na    | na   1    | na        | na        | na        | 14.080    | na        
        PSU1_PIn         | na         | Watts      | na    | na   2    | na        | na        | na        | 1500.000  | na        
        PSU1_Temp1       | na         | degrees C  | na    | na       | na        | na        | na        | na        | na        
        PSU1_Temp2       | na         | degrees C  | na    | na        | na        | na        | na        | na        | na        
        PSU1_VOut        | na         | Volts      | na    | na   3    | na        | na        | na        | 13.500    | 15.600    
        PSU1_COut        | na         | Amps       | na    | na   4    | na        | na        | na        | 125.000   | na        
        PSU1_POut        | na         | Watts      | na    | na   5    | na        | na        | na        | 1500.000  | na        
        PSU2_VIn         | 228.800    | Volts      | ok    | na   6    | na        | na        | 239.800   | 264.000   | na        
        PSU2_CIn         | 0.480      | Amps       | ok    | na   7    | na        | na        | na        | 14.080    | na        
        PSU2_PIn         | 114.000    | Watts      | ok    | na   8     | na        | na        | na        | 1500.000  | na        
        PSU2_Temp1       | 26.000     | degrees C  | ok    | na        | na        | na        | na        | na        | na        
        PSU2_Temp2       | 43.000     | degrees C  | ok    | na        | na        | na        | na        | na        | na        
        PSU2_VOut        | 12.000     | Volts      | ok    | na   9     | na        | na        | na        | 13.500    | 15.600    
        PSU2_COut        | 7.500      | Amps       | ok    | na   10     | na        | na        | na        | 125.000   | na        
        PSU2_POut        | 90.000     | Watts      | ok    | na   11     | na        | na        | na        | 1500.000  | na        
        root@localhost:~# 
    */
    if((NULL == mvin) || (NULL == mvout) ||(NULL == mpin) || (NULL == mpout) || (NULL == miin) || (NULL == miout))
	{
		printf("%s null pointer!\n", __FUNCTION__);
		return -1;
	}

    if(is_shm_mem_ready()){
        ret = open_file(ONLP_SENSOR_LIST_CACHE_SHARED,ONLP_SENSOR_LIST_SEM, &tmp, &len);
        if(ret < 0 || !tmp){
            printf("Failed - Failed to obtain system information\n");
            (void)free(tmp);
            tmp = (char *)NULL;
            return ret;
        }
    }else{
        // use unsafe method to read the cache file.
        sprintf(command, "cat %s",ONLP_SENSOR_LIST_FILE);
        tmp = read_tmp_cache(command,ONLP_SENSOR_LIST_FILE);
    }

    char *content, *temp_pointer;
    int flag = 0;
    content = strtok_r(tmp, "\n", &temp_pointer);

    int search_from = 0;
    int search_to = 0;

    if(id == 1){
        search_from = 0;
        search_to = 5;
    }else{
        search_from = 6;
        search_to = 11;
    }
    
    while(content != NULL && search_from <= search_to){
        if (strstr(content, psu_sensor_name[search_from]))
        {
            flag = 1;
            index++;
        }

        if(flag == 1){
            i = 0;
            token = strtok(content, "|");
            while( token != NULL ) 
            {
                if(i == 1){
                    array_trim(token, &strTmp[search_from][i]);
                    search_from++;
                }
                i++;
                if(i > 2) break;
                token = strtok(NULL, "|");
            }
        }


        flag = 0;
        content = strtok_r(NULL, "\n", &temp_pointer);
    }

    if(content){
        content = (char *)NULL;
    }
    if(temp_pointer){
        temp_pointer = (char *)NULL;
    }
    if(tmp){
    	(void)free(tmp);
	    tmp = (char *)NULL;
    }

    if (id == 1)
    {
        if (0 == strcmp(&strTmp[0][1], "na"))
            *mvin = 0;
        else
            *mvin = atof(&strTmp[0][1]) * 1000.0;

        if (0 == strcmp(&strTmp[3][1], "na"))
            *mvout = 0;
        else
            *mvout = atof(&strTmp[3][1]) * 1000.0;

        if (0 == strcmp(&strTmp[2][1], "na"))
            *mpin = 0;
        else
            *mpin = atof(&strTmp[2][1]) * 1000.0;

        if (0 == strcmp(&strTmp[5][1], "na"))
            *mpout = 0;
        else
            *mpout = atof(&strTmp[5][1]) * 1000.0;

        if (0 == strcmp(&strTmp[1][1], "na"))
            *miin = 0;
        else
            *miin = atof(&strTmp[1][1]) * 1000.0;

        if (0 == strcmp(&strTmp[4][1], "na"))
            *miout = 0;
        else
            *miout = atof(&strTmp[4][1]) * 1000.0;
    }
    else
    {
        if (0 == strcmp(&strTmp[6][1], "na"))
            *mvin = 0;
        else
            *mvin = atof(&strTmp[6][1]) * 1000.0;

        if (0 == strcmp(&strTmp[9][1], "na"))
            *mvout = 0;
        else
            *mvout = atof(&strTmp[9][1]) * 1000.0;

        if (0 == strcmp(&strTmp[8][1], "na"))
            *mpin = 0;
        else
            *mpin = atof(&strTmp[8][1]) * 1000.0;

        if (0 == strcmp(&strTmp[11][1], "na"))
            *mpout = 0;
        else
            *mpout = atof(&strTmp[11][1]) * 1000.0;

        if (0 == strcmp(&strTmp[7][1], "na"))
            *miin = 0;
        else
            *miin = atof(&strTmp[7][1]) * 1000.0;

        if (0 == strcmp(&strTmp[10][1], "na"))
            *miout = 0;
        else
            *miout = atof(&strTmp[10][1]) * 1000.0;
    }

    return ret;
}

int get_psu_model_sn(int id, char *model, char *serial_number)
{
    int index;
    char *token;
    char *tmp = (char *)NULL;
    int len = 0;
    int ret = -1;
    int search_psu_id = 1;

    if (0 == strcasecmp(psu_information[0].model, "unknown")) {
        
        index = 0;
        if(is_shm_mem_ready()){
            ret = open_file(ONLP_FRU_CACHE_SHARED,ONLP_FRU_CACHE_SEM, &tmp, &len);
            if(ret < 0 || !tmp){
                printf("Failed - Failed to obtain system information\n");
                (void)free(tmp);
                tmp = (char *)NULL;
                return ret;
            }
        }else{
            // use unsafe method to read the cache file.
            sprintf(command, "cat %s",ONLP_FRU_CACHE_FILE);
            tmp = read_tmp_cache(command,ONLP_FRU_CACHE_FILE);
        }
        
        char *content, *temp_pointer;
        int flag = 0;
        /*
        String example:			  
        root@localhost:~# ipmitool fru (Pull out PSU1)

        FRU Device Description : FRU_PSUL (ID 3)
         Device not present (Unknown (0x81))

        FRU Device Description : FRU_PSUR (ID 4)
         Board Mfg Date        : Mon Mar 11 08:55:00 2019
         Board Mfg             : DELTA-THAILAND  
         Board Product         : 1500W-CAPELLA-PSU 1500W 
         Board Serial          : FFGT1911000697
         Board Part Number     : TDPS1500AB6B

        */
        content = strtok_r(tmp, "\n", &temp_pointer);

        while(content != NULL){
            if (strstr(content, "FRU Device Description : FRU_PSU")) {
                flag = 1;
                index++;
            }
            if(flag == 1){
                if (strstr(content, "Device not present")) {
                    index++;
                    flag=0;
                }
                else if (strstr(content, "Board Product")) {
                    token = strtok(content, ":");
                    token = strtok(NULL, ":");
                    char* trim_token = trim(token);
                    sprintf(psu_information[index].model,"%s",trim_token);
                }
                else if (strstr(content, "Board Serial")) {
                    token = strtok(content, ":");
                    token = strtok(NULL, ":");
                    char* trim_token = trim(token);
                    sprintf(psu_information[index].serial_number,"%s",trim_token);
                    flag = 0;
                    search_psu_id++;
                }
            }
            if(search_psu_id > PSU_COUNT){
                content = NULL;
            }else{
                content = strtok_r(NULL, "\n", &temp_pointer);
            }
        }

        sprintf(psu_information[0].model,"pass"); //Mark as complete

        if(temp_pointer){
            temp_pointer = (char *)NULL;
        }
        if(tmp){
    	    (void)free(tmp);
	        tmp = (char *)NULL;
        }
    }

    strcpy(model, psu_information[id].model);
    strcpy(serial_number, psu_information[id].serial_number);

    return 1;
}

int get_fan_info(int id, char *model, char *serial, int *isfanb2f)
{
    int index;
    char *token;
    char *tmp = (char *)NULL;
    int len = 0;
    int ret = -1;

    if (0 == strcasecmp(fan_information[0].model, "unknown")) {
        index = 0;
        if(is_shm_mem_ready()){
            ret = open_file(ONLP_FRU_CACHE_SHARED,ONLP_FRU_CACHE_SEM, &tmp, &len);
            if(ret < 0 || !tmp){
                printf("Failed - Failed to obtain system information\n");
                (void)free(tmp);
                tmp = (char *)NULL;
                return ret;
            }
        }else{
            // use unsafe method to read the cache file.
            sprintf(command, "cat %s",ONLP_FRU_CACHE_FILE);
            tmp = read_tmp_cache(command,ONLP_FRU_CACHE_FILE);
        }
        /*
        String example:			  
        root@localhost:~# ipmitool fru (Pull out FAN1)

        FRU Device Description : FRU_FAN1 (ID 6)
         Device not present (Unknown (0x81))

        FRU Device Description : FRU_FAN2 (ID 7)
         Board Mfg Date        : Sat Apr  6 10:26:00 2019
         Board Mfg             : Celestica
         Board Product         : Fan Board 2
         Board Serial          : R1141-F0018-01GD0119170163
         Board Part Number     : R1141-F0018-01
         Board Extra           : Mt.Echo-Fan
         Board Extra           : 02
         Board Extra           : F2B

        .
        .

        FRU Device Description : FRU_FAN7 (ID 12)
         Board Mfg Date        : Sat Apr  6 10:26:00 2019
         Board Mfg             : Celestica
         Board Product         : Fan Board 7
         Board Serial          : R1141-F0018-01GD0119170165
         Board Part Number     : R1141-F0018-01
         Board Extra           : Mt.Echo-Fan
         Board Extra           : 02
         Board Extra           : F2B

        */
        char *content, *temp_pointer;
        int flag = 0;
        content = strtok_r(tmp, "\n", &temp_pointer);

        while(content != NULL){
            if (strstr(content, "FRU_FAN") && !strstr(content,"FRU_FANBRD")) {
                flag = 1;
                index++;
            }
            if(flag == 1){
                if (strstr(content, "Device not present")) {
                    index++;
                    flag=0;
                }
                else if (strstr(content, "Board Serial")) {
                    token = strtok(content, ":");
                    token = strtok(NULL, ":");
                    char* trim_token = trim(token);
                    sprintf(fan_information[index].serial_number,"%s",trim_token);
                }
                else if (strstr(content, "Board Part Number")) {
                    token = strtok(content, ":");
                    token = strtok(NULL, ":");
                    char* trim_token = trim(token);
                    sprintf(fan_information[index].model,"%s",trim_token);
                    
                }else if (strstr(content, "Board Extra")) 
                {
                    token = strtok(content, ":");
                    token = strtok(NULL, ":");
                    char* trim_token = trim(token);
                    //Check until find B2F or F2B
                    if(strcmp(trim_token, "B2F") == 0){
                        fan_information[index].airflow = 4;
                        flag = 0;
                    }else if(strcmp(trim_token ,"F2B") == 0){
                        fan_information[index].airflow = 8;
                        flag = 0;
                    }

                }
                
            }
            if(index > FAN_COUNT){
                content = NULL;
            }else{
                content = strtok_r(NULL, "\n", &temp_pointer);
            }
        }

        sprintf(fan_information[0].model,"pass"); //Mark as complete

        if(temp_pointer){
            temp_pointer = (char *)NULL;
        }
        if(tmp){
    	    (void)free(tmp);
	        tmp = (char *)NULL;
        }
    }
    strcpy(model, fan_information[id].model);
    strcpy(serial, fan_information[id].serial_number);
    *isfanb2f = fan_information[id].airflow;
    
    return 1;
}

int get_sensor_info(int id, int *temp, int *warn, int *error, int *shutdown)
{
    char *tmp = (char *)NULL;
    int len = 0;
    int index  = 0;

	int i = 0;
	int ret = 0;
    char strTmp[10][128] = {{0}, {0}};
    char *token = NULL;
    char *Thermal_sensor_name[13] = {
        "TEMP_CPU", "TEMP_BB", "TEMP_SW_U16", "TEMP_SW_U52",
        "TEMP_FAN_U17", "TEMP_FAN_U52","SW_U04_Temp","SW_U14_Temp","SW_U4403_Temp",
        "PSU1_Temp1", "PSU1_Temp2","PSU2_Temp1", "PSU2_Temp2"};

	if((NULL == temp) || (NULL == warn) || (NULL == error) || (NULL == shutdown))
	{
		printf("%s null pointer!\n", __FUNCTION__);
		return -1;
	}

    /*
        String example:			  
        ipmitool sensor list
        TEMP_CPU     | 1.000      | degrees C  | ok  | 5.000  | 9.000  | 16.000  | 65.000  | 73.000  | 75.606
        TEMP_FAN_U52 | 32.000	  | degrees C  | ok  | na  |  na  | na  | na  | 70.000  | 75.000
        PSUR_Temp1   | na         | degrees C  | na  | na  | na   | na   | na  | na  | na
    */
    if(is_shm_mem_ready()){
        ret = open_file(ONLP_SENSOR_LIST_CACHE_SHARED,ONLP_SENSOR_LIST_SEM, &tmp, &len);
        if(ret < 0 || !tmp){
            printf("Failed - Failed to obtain system information\n");
            (void)free(tmp);
            tmp = (char *)NULL;
            return ret;
        }
    }else{
        // use unsafe method to read the cache file.
        sprintf(command, "cat %s",ONLP_SENSOR_LIST_FILE);
        tmp = read_tmp_cache(command,ONLP_SENSOR_LIST_FILE);
    }
    
    char *content, *temp_pointer;
    int flag = 0;
    content = strtok_r(tmp, "\n", &temp_pointer);
    while(content != NULL){
        if (strstr(content, Thermal_sensor_name[id - 1])) {
            flag = 1;
            index++;
        }
        if(flag == 1){

            i = 0;
            token = strtok(content, "|");
            while( token != NULL ) 
            {
                array_trim(token, &strTmp[i][0]);
                i++;
                if(i > 10) break;
                token = strtok(NULL, "|");
            }
            
            flag = 3;
        }
        
        if(flag == 3){
            content = NULL;
        }else{
            content = strtok_r(NULL, "\n", &temp_pointer);
        }
    }

    if (0 == strcmp(&strTmp[1][0], "na"))
        *temp = 0;
    else
        *temp = atof(&strTmp[1][0]) * 1000.0;

    if (0 == strcmp(&strTmp[7][0], "na"))
        *warn = 0;
    else
        *warn = atof(&strTmp[7][0]) * 1000.0;

    if (0 == strcmp(&strTmp[8][0], "na"))
        *error = 0;
    else
        *error = atof(&strTmp[8][0]) * 1000.0;

    if (0 == strcmp(&strTmp[9][0], "na"))
        *shutdown = 0;
    else
        *shutdown = atof(&strTmp[9][0]) * 1000.0;

    if(content){
        content = (char *)NULL;
    }
    if(temp_pointer){
        temp_pointer = (char *)NULL;
    }
    if(tmp){
    	(void)free(tmp);
	    tmp = (char *)NULL;
    }

    return 0;
}

int get_fan_speed(int id,int *per, int *rpm)
{
    
    int max_rpm_speed = 29700;// = 100% speed
    char *tmp = (char *)NULL;
    int len = 0;
    int index  = 0;

	int i = 0;
	int ret = 0;
    char strTmp[2][128] = {{0}, {0}};
    char *token = NULL;
    char *Fan_sensor_name[9] = {
        "Fan1_Rear", "Fan2_Rear", "Fan3_Rear", "Fan4_Rear",
        "Fan5_Rear", "Fan6_Rear", "Fan7_Rear","PSU1_Fan","PSU2_Fan"};

	if((NULL == per) || (NULL == rpm))
	{
		printf("%s null pointer!\n", __FUNCTION__);
		return -1;
	}

    /*
        String example:			  
        ipmitool sensor list (Plug out FAN1 and PSU 1)
        Fan1_Rear        | na         | RPM        | na    | na        | 1050.000  | na        | na        | na        | na        
        Fan2_Rear        | 28650.000  | RPM        | ok    | na        | 1050.000  | na        | na        | na        | na        
        Fan3_Rear        | 29250.000  | RPM        | ok    | na        | 1050.000  | na        | na        | na        | na        
        Fan4_Rear        | 28650.000  | RPM        | ok    | na        | 1050.000  | na        | na        | na        | na        
        Fan5_Rear        | 29400.000  | RPM        | ok    | na        | 1050.000  | na        | na        | na        | na        
        Fan6_Rear        | 29100.000  | RPM        | ok    | na        | 1050.000  | na        | na        | na        | na        
        Fan7_Rear        | 29100.000  | RPM        | ok    | na        | 1050.000  | na        | na        | na        | na   
        PSU1_Fan         | na         | RPM        | na    | na        | na        | na        | na        | na        | na        
        PSU2_Fan         | 15800.000  | RPM        | ok    | na        | na        | na        | na        | na        | na      
    */
    if(is_shm_mem_ready()){
        ret = open_file(ONLP_SENSOR_LIST_CACHE_SHARED,ONLP_SENSOR_LIST_SEM, &tmp, &len);
        if(ret < 0 || !tmp){
            printf("Failed - Failed to obtain system information\n");
            (void)free(tmp);
            tmp = (char *)NULL;
            return ret;
        }
    }else{
        // use unsafe method to read the cache file.
        sprintf(command, "cat %s",ONLP_SENSOR_LIST_FILE);
        tmp = read_tmp_cache(command,ONLP_SENSOR_LIST_FILE);
    }
    
    char *content, *temp_pointer;
    int flag = 0;
    content = strtok_r(tmp, "\n", &temp_pointer);
    while(content != NULL){
        if (strstr(content, Fan_sensor_name[id - 1])) {
            flag = 1;
            index++;
        }
        if(flag == 1){

            i = 0;
            token = strtok(content, "|");
            while( token != NULL ) 
            {
                array_trim(token, &strTmp[i][0]);
                i++;
                if(i > 2) break;
                token = strtok(NULL, "|");
            }
            
            flag = 3;
        }
        
        if(flag == 3){
            content = NULL;
        }else{
            content = strtok_r(NULL, "\n", &temp_pointer);
        }
    }

    if (0 == strcmp(&strTmp[1][0], "na")){
        *rpm = 0;
        ret = -1;
    }else{
        *rpm = atof(&strTmp[1][0]);
    }

    if (0 == strcmp(&strTmp[1][0], "na"))
        *per = 0;
    else
        *per = (atof(&strTmp[1][0]) * 100 )/ max_rpm_speed;

    if(content){
        content = (char *)NULL;
    }
    if(temp_pointer){
        temp_pointer = (char *)NULL;
    }
    if(tmp){
    	(void)free(tmp);
	    tmp = (char *)NULL;
    }

    return ret;
}

int read_device_node_binary(char *filename, char *buffer, int buf_size, int data_len)
{
    int fd;
    int len;

    if ((buffer == NULL) || (buf_size < 0))
    {
        return -1;
    }

    if ((fd = open(filename, O_RDONLY)) == -1)
    {
        return -1;
    }

    if ((len = read(fd, buffer, buf_size)) < 0)
    {
        close(fd);
        return -1;
    }

    if ((close(fd) == -1))
    {
        return -1;
    }

    if ((len > buf_size) || (data_len != 0 && len != data_len))
    {
        return -1;
    }

    return 0;
}

int read_device_node_string(char *filename, char *buffer, int buf_size, int data_len)
{
    int ret;

    if (data_len >= buf_size)
    {
        return -1;
    }

    ret = read_device_node_binary(filename, buffer, buf_size - 1, data_len);
    if (ret == 0)
    {
        buffer[buf_size - 1] = '\0';
    }

    return ret;
}

char* trim (char *s)
{
    int i;

    while (isspace (*s)) s++;   // skip left side white spaces
    for (i = strlen (s) - 1; (isspace (s[i])); i--) ;   // skip right side white spaces
    s[i + 1] = '\0';
    
    return s;
}

int fill_shared_memory(const char *shm_path, const char *sem_path, const char *cache_path)
{
    int seg_size = 0;    
    int shm_fd = -1;   
    struct shm_map_data * shm_map_ptr = (struct shm_map_data *)NULL;

    if(!shm_path || !sem_path || !cache_path){
	return -1;
    }

    seg_size = sizeof(struct shm_map_data);

    shm_fd = shm_open(shm_path, O_CREAT|O_RDWR, S_IRWXU|S_IRWXG);
    if(shm_fd < 0){
        
	printf("\nshm_path:%s. errno:%d\n", shm_path, errno);
        return -1;
    }   
 
    ftruncate(shm_fd, seg_size);

    shm_map_ptr = (struct shm_map_data *)mmap(NULL, seg_size, PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0); 
    if(shm_map_ptr == MAP_FAILED){
	    printf("\nMAP_FAILED. errno:%d.\n", errno);
    	close(shm_fd);
        return -1;
    }

    if(access(cache_path, F_OK) == -1)
    {
        munmap(shm_map_ptr, seg_size);
        close(shm_fd);
        return -1;
    }
 
    struct stat sta;
    stat(cache_path, &sta);
    int st_size = sta.st_size;
    if(st_size == 0){
        munmap(shm_map_ptr, seg_size);
	    close(shm_fd);
	    return -1;
    }

    char *cache_buffer = (char *)malloc(st_size); 
    if(!cache_buffer){ 
        munmap(shm_map_ptr, seg_size);
	    close(shm_fd);
        return -1;
    }

    memset(cache_buffer, 0, st_size);
 
    FILE *cache_fp = fopen(cache_path, "r");
    if(!cache_fp)
    {
        free(cache_buffer);   
        munmap(shm_map_ptr, seg_size);
	    close(shm_fd);
        return -1;
    }

    int cache_len = fread(cache_buffer, 1, st_size, cache_fp);
    if(st_size != cache_len)
    {
        munmap(shm_map_ptr, seg_size);
        close(shm_fd);
        free(cache_buffer);
        fclose(cache_fp);
        return -1;
    }

    sem_t * sem_id = sem_open(sem_path, O_CREAT, S_IRUSR | S_IWUSR, 1);
    if(sem_id == SEM_FAILED){
        munmap(shm_map_ptr, seg_size);
	    close(shm_fd);
        free(cache_buffer);
        fclose(cache_fp);
        return -1;
    }    

    sem_wait(sem_id);

    memcpy(shm_map_ptr->data, cache_buffer, st_size); 
    
    shm_map_ptr->size = st_size;
 
    sem_post(sem_id);

    (void)free(cache_buffer);
    
    sem_close(sem_id);

    munmap(shm_map_ptr, seg_size);
   
    close(shm_fd);

    return 0; 
}

int dump_shared_memory(const char *shm_path, const char *sem_path, struct shm_map_data *shared_mem)
{
    sem_t *sem_id = (sem_t *)NULL;
    struct shm_map_data *map_ptr = (struct shm_map_data *)NULL;
    int seg_size = 0;
    int shm_fd = -1;

    if(!shm_path || !sem_path || !shared_mem){
	    return -1;
    }

    seg_size = sizeof(struct shm_map_data);


    shm_fd = shm_open(shm_path, O_RDONLY, 0666);
    if(shm_fd < 0){
        return -1; 
    }

    map_ptr = (struct shm_map_data *)mmap(NULL, seg_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if(map_ptr == MAP_FAILED){
        close(shm_fd);
        return -1;
    }   
 
    sem_id = sem_open(sem_path, 0);
    if(SEM_FAILED == sem_id){
        munmap(map_ptr, seg_size);
        close(shm_fd);
        return -1;
    }

    sem_wait(sem_id);
    
    memcpy(shared_mem, map_ptr, sizeof(struct shm_map_data));
   
    sem_post(sem_id);

    sem_close(sem_id);
    
    munmap(map_ptr, seg_size);
    close(shm_fd);

    return 0;
}

int open_file(const char *shm_path, const char *sem_path, char **cache_data, int *cache_size)
{
    int res = -1;
    char *tmp_ptr = (char *)NULL;
    struct shm_map_data shm_map_tmp;

    memset(&shm_map_tmp, 0, sizeof(struct shm_map_data));

    res = dump_shared_memory(shm_path, sem_path, &shm_map_tmp);
    if(!res){
	tmp_ptr = malloc(shm_map_tmp.size);
        if(!tmp_ptr){
	    res = -1;
	    return res;
	}	

	memset(tmp_ptr, 0, shm_map_tmp.size);

        memcpy(tmp_ptr, shm_map_tmp.data, shm_map_tmp.size);

        *cache_data = tmp_ptr;

        *cache_size = shm_map_tmp.size;
    }

    return res; 
}
