/************************************************************
 * platform_lib.c
 *
 *           Copyright 2018 Inventec Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include <dirent.h>
#include <sys/types.h>
#include "platform_lib.h"

static char platform_name[32] = "x86-64-inventec-d7032q28b-r0";

char* get_core_thermal_path()
{
    DIR * dir;
    struct dirent * ptr;
    static char path[ONLP_CONFIG_INFO_STR_MAX];
    dir = opendir("/sys/devices/platform/coretemp.0/hwmon/");
    char* buf=NULL;
    int fail=1;

    while( (ptr = readdir(dir))!=NULL ) {
        buf=ptr->d_name;
        if( strncmp(buf,"hwmon",5)==0 ) {
            fail=0;
            break;
        }
    }
    closedir(dir);
    if(fail==1) {
        printf("[ERROR] Can't find valid path\n");
        return NULL;
    }

    snprintf(path, ONLP_CONFIG_INFO_STR_MAX, "/sys/devices/platform/coretemp.0/hwmon/%s/" , buf);
    return path;
}

int
onlp_platformi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t *rdata = aim_zmalloc(256);
    if(onlp_file_read(rdata, 256, size, EEPROM_NODE(eeprom)) == ONLP_STATUS_OK) {
        if(*size == 256) {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }

    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_INTERNAL;
}

const char*
onlp_platformi_get(void)
{
#if 0
    char eeprom_data[256], *edp = &eeprom_data[0];
    int size = 0;

    if (onlp_platformi_onie_data_get(&eeprom_data, &size) == ONLP_STATUS_OK) {
        for (edp = &eeprom_data[0]; edp < (&eeprom_data[0] + 256); edp++) {
            if (*edp == 0x28) {
                snprintf(platform_name, size, "%s", edp);
                break;
            }
        }
    }
#endif
    return &platform_name[0];
}

int
onlp_platformi_sw_init(void)
{
    return 0;
}

#if 0
typedef struct fan_ctrl_policy {
    int duty_cycle;
    int temp_down_adjust; /* The boundary temperature to down adjust fan speed */
    int temp_up_adjust;   /* The boundary temperature to up adjust fan speed */
} fan_ctrl_policy_t;

fan_ctrl_policy_t  fan_ctrl_policy_f2b[] = {
    {32,      0, 174000},
    {38, 170000, 182000},
    {50, 178000, 190000},
    {63, 186000,      0}
};

fan_ctrl_policy_t  fan_ctrl_policy_b2f[] = {
    {32,     0,  140000},
    {38, 135000, 150000},
    {50, 145000, 160000},
    {69, 155000,      0}
};

#define FAN_DUTY_CYCLE_MAX  100
#define FAN_SPEED_CTRL_PATH "/sys/bus/i2c/devices/2-0066/fan_duty_cycle_percentage"

/*
 * For AC power Front to Back :
 *	* If any fan fail, please fan speed register to 15
 *	* The max value of Fan speed register is 9
 *		[LM75(48) + LM75(49) + LM75(4A)] > 174  => set Fan speed value from 4 to 5
 *		[LM75(48) + LM75(49) + LM75(4A)] > 182  => set Fan speed value from 5 to 7
 *		[LM75(48) + LM75(49) + LM75(4A)] > 190  => set Fan speed value from 7 to 9
 *
 *		[LM75(48) + LM75(49) + LM75(4A)] < 170  => set Fan speed value from 5 to 4
 *		[LM75(48) + LM75(49) + LM75(4A)] < 178  => set Fan speed value from 7 to 5
 *		[LM75(48) + LM75(49) + LM75(4A)] < 186  => set Fan speed value from 9 to 7
 *
 *
 * For  AC power Back to Front :
 *	* If any fan fail, please fan speed register to 15
 *	* The max value of Fan speed register is 10
 *		[LM75(48) + LM75(49) + LM75(4A)] > 140  => set Fan speed value from 4 to 5
 *		[LM75(48) + LM75(49) + LM75(4A)] > 150  => set Fan speed value from 5 to 7
 *		[LM75(48) + LM75(49) + LM75(4A)] > 160  => set Fan speed value from 7 to 10
 *
 *		[LM75(48) + LM75(49) + LM75(4A)] < 135  => set Fan speed value from 5 to 4
 *		[LM75(48) + LM75(49) + LM75(4A)] < 145  => set Fan speed value from 7 to 5
 *		[LM75(48) + LM75(49) + LM75(4A)] < 155  => set Fan speed value from 10 to 7
 */
int
onlp_platformi_manage_fans(void)
{
    int i = 0, arr_size, temp;
    fan_ctrl_policy_t *policy;
    int cur_duty_cycle, new_duty_cycle;
    onlp_thermal_info_t thermal_1, thermal_2, thermal_3;

    int  fd, len;
    char  buf[10] = {0};

    /* Get each fan status
     */
    for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BOARD; i++) {
        onlp_fan_info_t fan_info;

        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get fan(%d) status\r\n", i);
            return ONLP_STATUS_E_INTERNAL;
        }

        /* Decision 1: Set fan as full speed if any fan is failed.
         */
        if(ONLP_OID_STATUS_FLAG_IS_SET(&fan_info, FAILED)) {
            AIM_LOG_ERROR("Fan(%d) is not working, set the other fans as full speed\r\n", i);
            return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
        }

        /* Decision 1.1: Set fan as full speed if any fan is not present.
         */
        if(ONLP_OID_STATUS_FLAG_NOT_SET(&fan_info, PRESENT)) {
            AIM_LOG_ERROR("Fan(%d) is not present, set the other fans as full speed\r\n", i);
            return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
        }

        /* Get fan direction (Only get the first one since all fan direction are the same)
         */
        if (i == 1) {
            if(fan_info.dir == ONLP_FAN_DIR_F2B) {
                policy   = fan_ctrl_policy_f2b;
                arr_size = AIM_ARRAYSIZE(fan_ctrl_policy_f2b);
            } else {
                policy   = fan_ctrl_policy_b2f;
                arr_size = AIM_ARRAYSIZE(fan_ctrl_policy_b2f);
            }
        }
    }

    /* Get current fan speed
     */
    fd = open(FAN_SPEED_CTRL_PATH, O_RDONLY);
    if (fd == -1) {
        AIM_LOG_ERROR("Unable to open fan speed control node (%s)", FAN_SPEED_CTRL_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }

    len = read(fd, buf, sizeof(buf));
    close(fd);
    if (len <= 0) {
        AIM_LOG_ERROR("Unable to read fan speed from (%s)", FAN_SPEED_CTRL_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }
    cur_duty_cycle = atoi(buf);


    /* Decision 2: If no matched fan speed is found from the policy,
     *             use FAN_DUTY_CYCLE_MIN as default speed
     */
    for (i = 0; i < arr_size; i++) {
        if (policy[i].duty_cycle != cur_duty_cycle)
            continue;

        break;
    }

    if (i == arr_size) {
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), policy[0].duty_cycle);
    }

    /* Get current temperature
     */
    if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(2), &thermal_1) != ONLP_STATUS_OK ||
            onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(3), &thermal_2) != ONLP_STATUS_OK ||
            onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(4), &thermal_3) != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read thermal status");
        return ONLP_STATUS_E_INTERNAL;
    }
    temp = thermal_1.mcelsius + thermal_2.mcelsius + thermal_3.mcelsius;


    /* Decision 3: Decide new fan speed depend on fan direction/current fan speed/temperature
     */
    new_duty_cycle = cur_duty_cycle;

    if ((temp >= policy[i].temp_up_adjust) && (i != (arr_size-1))) {
        new_duty_cycle = policy[i+1].duty_cycle;
    } else if ((temp <= policy[i].temp_down_adjust) && (i != 0)) {
        new_duty_cycle = policy[i-1].duty_cycle;
    }

    if (new_duty_cycle == cur_duty_cycle) {
        /* Duty cycle does not change, just return */
        return ONLP_STATUS_OK;
    }

    return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), new_duty_cycle);
}

int
onlp_platformi_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/* d5254 *********************************************/
#endif

int deviceNodeWrite(char *filename, char *buffer, int buf_size, int data_len)
{
    int fd;
    int len;

    if ((buffer == NULL) || (buf_size < 0)) {
        return -1;
    }

    if ((fd = open(filename, O_WRONLY, S_IWUSR)) == -1) {
        return -1;
    }

    if ((len = write(fd, buffer, buf_size)) < 0) {
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

int deviceNodeWriteInt(char *filename, int value, int data_len)
{
    char buf[8] = {0};
    sprintf(buf, "%d", value);

    return deviceNodeWrite(filename, buf, (int)strlen(buf), data_len);
}

int deviceNodeReadBinary(char *filename, char *buffer, int buf_size, int data_len)
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

int deviceNodeReadString(char *filename, char *buffer, int buf_size, int data_len)
{
    int ret;

    if (data_len >= buf_size) {
        return -1;
    }

    ret = deviceNodeReadBinary(filename, buffer, buf_size-1, data_len);

    if (ret == 0) {
        buffer[buf_size-1] = '\0';
    }

    return ret;
}

#if 0
#define I2C_PSU_MODEL_NAME_LEN 11
#define I2C_PSU_FAN_DIR_LEN    3
#include <ctype.h>
psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
    char *node = NULL;
    char  model_name[I2C_PSU_MODEL_NAME_LEN + 1] = {0};
    char  fan_dir[I2C_PSU_FAN_DIR_LEN + 1] = {0};

    /* Check AC model name */
    node = (id == PSU1_ID) ? PSU1_AC_HWMON_NODE(psu_model_name) : PSU2_AC_HWMON_NODE(psu_model_name);

    if (deviceNodeReadString(node, model_name, sizeof(model_name), 0) != 0) {
        return PSU_TYPE_UNKNOWN;
    }

    if(isspace(model_name[strlen(model_name)-1])) {
        model_name[strlen(model_name)-1] = 0;
    }

    if (strncmp(model_name, "YM-2651Y", 8) == 0) {
        if (modelname) {
            strncpy(modelname, model_name, 8);
        }

        node = (id == PSU1_ID) ? PSU1_AC_PMBUS_NODE(psu_fan_dir) : PSU2_AC_PMBUS_NODE(psu_fan_dir);
        if (deviceNodeReadString(node, fan_dir, sizeof(fan_dir), 0) != 0) {
            return PSU_TYPE_UNKNOWN;
        }

        if (strncmp(fan_dir, "F2B", strlen("F2B")) == 0) {
            return PSU_TYPE_AC_F2B;
        }

        if (strncmp(fan_dir, "B2F", strlen("B2F")) == 0) {
            return PSU_TYPE_AC_B2F;
        }
    }

    if (strncmp(model_name, "YM-2651V", 8) == 0) {
        if (modelname) {
            strncpy(modelname, model_name, 8);
        }

        node = (id == PSU1_ID) ? PSU1_AC_PMBUS_NODE(psu_fan_dir) : PSU2_AC_PMBUS_NODE(psu_fan_dir);
        if (deviceNodeReadString(node, fan_dir, sizeof(fan_dir), 0) != 0) {
            return PSU_TYPE_UNKNOWN;
        }

        if (strncmp(fan_dir, "F2B", strlen("F2B")) == 0) {
            return PSU_TYPE_DC_48V_F2B;
        }

        if (strncmp(fan_dir, "B2F", strlen("B2F")) == 0) {
            return PSU_TYPE_DC_48V_B2F;
        }
    }

    if (strncmp(model_name, "PSU-12V-750", 11) == 0) {
        if (modelname) {
            strncpy(modelname, model_name, 11);
        }

        node = (id == PSU1_ID) ? PSU1_AC_HWMON_NODE(psu_fan_dir) : PSU2_AC_HWMON_NODE(psu_fan_dir);
        if (deviceNodeReadString(node, fan_dir, sizeof(fan_dir), 0) != 0) {
            return PSU_TYPE_UNKNOWN;
        }

        if (strncmp(fan_dir, "F2B", 3) == 0) {
            return PSU_TYPE_DC_12V_F2B;
        }

        if (strncmp(fan_dir, "B2F", 3) == 0) {
            return PSU_TYPE_DC_12V_B2F;
        }

        if (strncmp(fan_dir, "NON", 3) == 0) {
            return PSU_TYPE_DC_12V_FANLESS;
        }
    }

    return PSU_TYPE_UNKNOWN;
}

#define PSU_SERIAL_NUMBER_LEN	18

int psu_serial_number_get(int id, char *serial, int serial_len)
{
    int   size = 0;
    int   ret  = ONLP_STATUS_OK;
    char *prefix = NULL;

    if (serial == NULL || serial_len < PSU_SERIAL_NUMBER_LEN) {
        return ONLP_STATUS_E_PARAM;
    }

    prefix = (id == PSU1_ID) ? PSU1_AC_PMBUS_PREFIX : PSU2_AC_PMBUS_PREFIX;

    ret = onlp_file_read((uint8_t*)serial, PSU_SERIAL_NUMBER_LEN, &size, "%s%s", prefix, "psu_mfr_serial");
    if (ret != ONLP_STATUS_OK || size != PSU_SERIAL_NUMBER_LEN) {
        return ONLP_STATUS_E_INTERNAL;

    }

    serial[PSU_SERIAL_NUMBER_LEN] = '\0';
    return ONLP_STATUS_OK;
}

#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include <onlplib/file.h>
#include <onlp/onlp.h>
#include "platform_lib.h"

#define PSU_NODE_MAX_PATH_LEN 64

int onlp_file_read_binary(char *filename, char *buffer, int buf_size, int data_len)
{
    if ((buffer == NULL) || (buf_size < 0)) {
        return -1;
    }

    return onlp_file_read((uint8_t*)buffer, buf_size, &data_len, "%s", filename);
}

int onlp_file_read_string(char *filename, char *buffer, int buf_size, int data_len)
{
    int ret;

    if (data_len >= buf_size) {
        return -1;
    }

    ret = onlp_file_read_binary(filename, buffer, buf_size-1, data_len);

    if (ret == 0) {
        buffer[buf_size-1] = '\0';
    }

    return ret;
}

#define I2C_PSU_MODEL_NAME_LEN 32
#define I2C_PSU_FAN_DIR_LEN    8
#include <ctype.h>
psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
    char *node = NULL;
    char  model_name[I2C_PSU_MODEL_NAME_LEN + 1] = {0};
    char  fan_dir[I2C_PSU_FAN_DIR_LEN + 1] = {0};

    /* Check AC model name */
    node = (id == PSU1_ID) ? PSU1_AC_HWMON_NODE(psu_model_name) : PSU2_AC_HWMON_NODE(psu_model_name);

    if (onlp_file_read_string(node, model_name, sizeof(model_name), 0) != 0) {
        return PSU_TYPE_UNKNOWN;
    }

    if(isspace(model_name[strlen(model_name)-1])) {
        model_name[strlen(model_name)] = 0;
    }

    if (strncmp(model_name, "YM-2651Y", 8) == 0) {
        if (modelname) {
            strncpy(modelname, model_name, 8);
        }

        node = (id == PSU1_ID) ? PSU1_AC_PMBUS_NODE(psu_fan_dir) : PSU2_AC_PMBUS_NODE(psu_fan_dir);
        if (onlp_file_read_string(node, fan_dir, sizeof(fan_dir), 0) != 0) {
            return PSU_TYPE_UNKNOWN;
        }

        if (strncmp(fan_dir, "F2B", strlen("F2B")) == 0) {
            return PSU_TYPE_AC_F2B;
        }

        if (strncmp(fan_dir, "B2F", strlen("B2F")) == 0) {
            return PSU_TYPE_AC_B2F;
        }
    }

    if (strncmp(model_name, "YM-2651V", 8) == 0) {
        if (modelname) {
            strncpy(modelname, model_name, 8);
        }

        node = (id == PSU1_ID) ? PSU1_AC_PMBUS_NODE(psu_fan_dir) : PSU2_AC_PMBUS_NODE(psu_fan_dir);
        if (onlp_file_read_string(node, fan_dir, sizeof(fan_dir), 0) != 0) {
            return PSU_TYPE_UNKNOWN;
        }

        if (strncmp(fan_dir, "F2B", strlen("F2B")) == 0) {
            return PSU_TYPE_DC_48V_F2B;
        }

        if (strncmp(fan_dir, "B2F", strlen("B2F")) == 0) {
            return PSU_TYPE_DC_48V_B2F;
        }
    }

    if (strncmp(model_name, "PSU-12V-750", 11) == 0) {
        if (modelname) {
            strncpy(modelname, model_name, 11);
        }

        node = (id == PSU1_ID) ? PSU1_AC_HWMON_NODE(psu_fan_dir) : PSU2_AC_HWMON_NODE(psu_fan_dir);
        if (onlp_file_read_string(node, fan_dir, sizeof(fan_dir), 0) != 0) {
            return PSU_TYPE_UNKNOWN;
        }

        if (strncmp(fan_dir, "F2B", 3) == 0) {
            return PSU_TYPE_DC_12V_F2B;
        }

        if (strncmp(fan_dir, "B2F", 3) == 0) {
            return PSU_TYPE_DC_12V_B2F;
        }

        if (strncmp(fan_dir, "NON", 3) == 0) {
            return PSU_TYPE_DC_12V_FANLESS;
        }
    }

    if (strncmp(model_name, "DPS-150AB-10", 12) == 0) {
        if (modelname) {
            strncpy(modelname, model_name, 12);
        }

        return PSU_TYPE_DC_12V_F2B;
    }

    return PSU_TYPE_UNKNOWN;
}

int psu_pmbus_info_get(int id, char *node, int *value)
{
    int  ret = 0;
    *value = 0;

    if (PSU1_ID == id) {
        ret = onlp_file_read_int(value, "%s%s%d", PSU1_AC_PMBUS_PREFIX, node, PSU1_ID);
    } else if (PSU2_ID == id) {
        ret = onlp_file_read_int(value, "%s%s%d", PSU2_AC_PMBUS_PREFIX, node, PSU2_ID);
    } else {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ret;
}

int psu_pmbus_info_set(int id, char *node, int value)
{
    char path[PSU_NODE_MAX_PATH_LEN] = {0};

    switch (id) {
    case PSU1_ID:
        sprintf(path, "%s%s%d", PSU1_AC_PMBUS_PREFIX, node, PSU1_ID);
        break;
    case PSU2_ID:
        sprintf(path, "%s%s%d", PSU2_AC_PMBUS_PREFIX, node, PSU2_ID);
        break;
    default:
        return ONLP_STATUS_E_UNSUPPORTED;
    };

    if (onlp_file_write_int(value, path, NULL) != 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
#endif
