#ifndef _PLATFORM_WBMC_H_
#define _PLATFORM_WBMC_H_

#define ONLP_SENSOR_CACHE_SHARED      "/onlp-sensor-cache-shared"
#define ONLP_FRU_CACHE_SHARED         "/onlp-fru-cache-shared"
#define ONLP_SENSOR_LIST_CACHE_SHARED "/onlp-sensor-list-cache-shared"

#define ONLP_SENSOR_CACHE_SEM         "/onlp-sensor-cache-sem"
#define ONLP_FRU_CACHE_SEM            "/onlp-fru-cache-sem"
#define ONLP_SENSOR_LIST_SEM          "/onlp-sensor-list-cache-sem"

#define ONLP_SENSOR_CACHE_FILE        "/tmp/onlp-sensor-cache.txt"
#define ONLP_FRU_CACHE_FILE           "/tmp/onlp-fru-cache.txt"
#define ONLP_SENSOR_LIST_FILE         "/tmp/onlp-sensor-list-cache.txt"
#define USE_SHM_METHOD 0

struct device_info{
    char serial_number[256];
    char model[256];
    int airflow;
};

struct shm_map_data{
    char data[16384];
    int size;
};

int exec_ipmitool_cmd(char *cmd, char *retd);
int get_psu_model_sn_wbmc(int id,char* model,char* serial_number);
int get_psu_info_wbmc(int id,int *mvin,int *mvout,int *mpin,int *mpout,int *miin,int *miout);
int get_fan_info_wbmc(int id,char* model,char* serial,int *get_fan_info);
int get_sensor_info_wbmc(int id, int *temp, int *warn, int *error, int *shutdown);
int get_fan_speed_wbmc(int id,int* per,int* rpm);
int dump_shared_memory(const char *shm_path, const char *sem_path, struct shm_map_data *shared_mem);
int fill_shared_memory(const char *shm_path, const char *sem_path, const char *cache_path);
int open_file(const char *shm_path, const char *sem_path, char **cache_data, int *cache_size);
int create_cache();
void update_shm_mem(void);
int is_cache_exist();

#endif /* _PLATFORM_WBMC_H_ */
