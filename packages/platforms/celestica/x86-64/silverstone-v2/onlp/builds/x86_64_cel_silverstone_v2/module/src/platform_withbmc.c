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
#include "platform_common.h"
#include "platform_withbmc.h"

char command[256];

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
        while (c)
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
uint8_t read_fan_led_wb(uint16_t dev_reg)
{
    unsigned int status;
    sprintf(command, "ipmitool raw  0x%x 0x%x %d %d 0x%x", NETFN, CPLD_OME, FAN_CPLD_NUMBER,R_FLAG,dev_reg);
	FILE *fp;
	
    fp = popen(command, "r");
    if (!fp)
    {
        printf("Failed : Can't specify Fan CPLD register\n");
        return -1;
    }
    fscanf(fp, "%d", &status);
    pclose(fp);

    return status;
}

int get_psu_info_wb(int id, int *mvin, int *mvout, int *mpin, int *mpout, int *miin, int *miout)
{
    char *tmp = (char *)NULL;
    int len = 0;
    int index  = 0;

	int i = 0;
	int ret = 0;
    char strTmp[12][128] = {{0}, {0}};
    char *token = NULL;
    char *psu_sensor_name[12] = {
        "PSU1_VIn", "PSU1_VOut", "PSU1_CIn", "PSU1_COut",
		"PSU1_PIn", "PSU1_POut", "PSU2_VIn", "PSU2_VOut",
        "PSU2_CIn", "PSU2_COut", "PSU2_PIn", "PSU2_POut"};
		

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

int get_psu_model_sn_wb(int id, char *model, char *serial_number)
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
       
        content = strtok_r(tmp, "\n", &temp_pointer);

        while(content != NULL){
            if (strstr(content, "FRU Device Description : FRU_PSU")) {
                flag = 1;
                index++;
            }
            if(flag == 1){
                if (strstr(content, "Device not present")) {
                    flag=0;
                }
                else if (strstr(content, "Board Product")) {
                    strtok(content, ":");
                    token = strtok(NULL, ":");
                    char* trim_token = trim(token);
                    sprintf(psu_information[index].model,"%s",trim_token);
                }
                else if (strstr(content, "Board Serial")) {
                    strtok(content, ":");
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

int get_fan_info_wb(int id, char *model, char *serial, int *isfanb2f)
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
                    flag=0;
                }
                else if (strstr(content, "Board Serial")) {
                    strtok(content, ":");
                    token = strtok(NULL, ":");
                    char* trim_token = trim(token);
                    sprintf(fan_information[index].serial_number,"%s",trim_token);
                }
                else if (strstr(content, "Board Part Number")) {
                    strtok(content, ":");
                    token = strtok(NULL, ":");
                    char* trim_token = trim(token);
                    sprintf(fan_information[index].model,"%s",trim_token);
                    
                }else if (strstr(content, "Board Extra")) 
                {
                    strtok(content, ":");
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

int get_sensor_info_wb(int id, int *temp, int *warn, int *error, int *shutdown)
{
    char *tmp = (char *)NULL;
    int len = 0;
    int index  = 0;

	int i = 0;
	int ret = 0;
    char strTmp[10][128] = {{0}, {0}};
    char *token = NULL;
    char *Thermal_sensor_name[18] = {
        "TEMP_SW_U52","TEMP_SW_U16", "TEMP_FB_U52", "TEMP_FB_U17", "VDD_CORE_Temp",
        "XP0R8V_Temp", "XP3R3V_R_Temp","XP3R3V_L_Temp",
		"TEMP_SW_Internal","TEMP_CPU","TEMP_DIMMA0","TEMP_DIMMB0","PSU1_Temp1",
		"PSU1_Temp2", "PSU1_Temp3","PSU2_Temp1", "PSU2_Temp2", "PSU2_Temp3" };

	if((NULL == temp) || (NULL == warn) || (NULL == error) || (NULL == shutdown))
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

int get_fan_speed_wb(int id,int *per, int *rpm)
{
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
    else if(strstr(&strTmp[0][0],"Rear"))    
        *per = (atof(&strTmp[1][0]) * 100 )/ CHASSIS_FAN_MAX_RPM;
    else if(strstr(&strTmp[0][0],"PSU"))
        *per = (atof(&strTmp[1][0]) * 100 )/ PSU_FAN_MAX_RPM;

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
    fclose(cache_fp);

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
