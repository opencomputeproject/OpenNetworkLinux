//////////////////////////////////////////////////////////////
//   PLATFORM FUNCTION TO INTERACT WITH SYS_CPLD AND OTHER DRIVERS    //
//////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/io.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <onlplib/i2c.h>
#include "platform.h"

char command[256];
FILE *fp;

static struct device_info fan_information[FAN_COUNT + 1] = {
    {"unknown", "unknown",1}, //check
    {}, //Fan 1
    {}, //Fan 2
    {}, //Fan 3
    {}, //PSU Fan1
    {}, //PSU Fan2
};

static const struct led_reg_mapper led_mapper[LED_COUNT + 1] = {
    {},
    {"LED_SYSTEM", LED_SYSTEM_ID, LED_SYSTEM_REGISTER},
    {"LED_ALARM",  LED_ALARM_ID, LED_ALARM_REGISTER},
    {"LED_PSU",    LED_PSU_ID, LED_PSU_REGISTER},
    {"LED_FAN",    LED_FAN_ID, LED_FAN1_REGISTER},
    {"LED_FAN1",   LED_FAN1_ID,LED_FAN1_REGISTER},
    {"LED_FAN2",   LED_FAN2_ID,LED_FAN2_REGISTER},
    {"LED_FAN3",   LED_FAN3_ID,LED_FAN3_REGISTER},
};

static const struct psu_reg_bit_mapper psu_mapper [PSU_COUNT + 1] = {
    {},
    {PSU_STA_REGISTER, 3, 7, 1},
    {PSU_STA_REGISTER, 2, 6, 0},
};

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

int read_smbus_block(int bus, int address, int bank, unsigned char *cblock)
{
    int res = 0, file;
    char filename[20];

    snprintf(filename, 19, "/dev/i2c-%d", bus);
    file = open(filename, O_RDWR);
    if (file < 0) {
          return -1;
    }
    if (ioctl(file, I2C_SLAVE_FORCE, address) < 0) {
          return -1;
    }
    res = i2c_smbus_read_block_data(file, bank, cblock);
    close(file);

    return res;
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

int get_sysnode_value(const char *path, void *dat)
{
    int fd, ret = 0;
    FILE *fp = NULL;
    char new_path[200] = {0};
    char buffer[50] = {0};
    int *data = (int *)dat;

    /* replace hwmon* with hwmon[0-9] */
    sprintf(new_path, "ls %s", path);
    fp = popen(new_path, "r");
    if (fp == NULL)
    {
        printf("call popen fail\n");
        return -1;
    }
    fscanf(fp, "%s", new_path);
    pclose(fp);

    fd = open(new_path, O_RDONLY);
    if(fd < 0){
        ret = -1;
    }else{
        read(fd, buffer, sizeof(buffer));
        *data = atoi(buffer);
        close(fd);
    }

    return ret;
}

int set_sysnode_value(const char *path, int data)
{
    int fd, ret = 0;
    FILE *fp = NULL;
    char new_path[200] = {0};
    char buffer[50] = {0};

    /* replace hwmon* with hwmon[0-9] */
    sprintf(new_path, "ls %s", path);
    fp = popen(new_path, "r");
    if (fp == NULL)
    {
        printf("call popen fail\n");
        return -1;
    }
    fscanf(fp, "%s", new_path);
    pclose(fp);

    fd = open(new_path, O_WRONLY);
    if(fd < 0){
        ret = -1;
    }else{
    sprintf(buffer, "%d", data);
        write(fd, buffer, strlen(buffer));
        close(fd);
    }

    return ret;
}

int get_psu_info(int id, int *mvin, int *mvout, int *mpin, int *mpout, int *miin, int *miout)
{
    int ret = 0, fail = 0;

    *mvin = *mvout = *mpin = *mpout = *miin = *miout = 0;

    if((NULL == mvin) || (NULL == mvout) ||(NULL == mpin) || (NULL == mpout) || (NULL == miin) || (NULL == miout))
    {
        printf("%s null pointer!\n", __FUNCTION__);
        return -1;
    }
    if (id == 1)
    {
        /* voltage in and out */
        ret = get_sysnode_value(PSU1_VIN, mvin);
        if(ret < 0){
            fail++;
        }
        ret = get_sysnode_value(PSU1_VOUT, mvout);
        if(ret < 0){
            fail++;
        }
        /* power in and out */
        ret = get_sysnode_value(PSU1_PIN, mpin);
        if(ret < 0){
            fail++;
        }
        *mpin = *mpin / 1000;
        ret = get_sysnode_value(PSU1_POUT, mpout);
        if(ret < 0){
            fail++;
        }
        *mpout = *mpout / 1000;
        /* current in and out*/
        ret = get_sysnode_value(PSU1_CIN, miin);
        if(ret < 0){
            fail++;
        }
        ret = get_sysnode_value(PSU1_COUT, miout);
        if(ret < 0){
            fail++;
        }
    } else {
        /* voltage in and out */
        ret = get_sysnode_value(PSU2_VIN, mvin);
        if(ret < 0){
            fail++;
        }
        ret = get_sysnode_value(PSU2_VOUT, mvout);
        if(ret < 0){
            fail++;
        }
        /* power in and out */
        ret = get_sysnode_value(PSU2_PIN, mpin);
        if(ret < 0){
            fail++;
        }
        *mpin = *mpin / 1000;
        ret = get_sysnode_value(PSU2_POUT, mpout);
        if(ret < 0){
            fail++;
        }
        *mpout = *mpout / 1000;
        /* current in and out*/
        ret = get_sysnode_value(PSU2_CIN, miin);
        if(ret < 0){
            fail++;
        }
        ret = get_sysnode_value(PSU2_COUT, miout);
        if(ret < 0){
            fail++;
        }
    }

    return fail;
}

int get_psu_model_sn(int id, char *model, char *serial_number)
{
    unsigned char cblock[288] = {0};
    int ret = 0;
    int bus = PSU_I2C_BUS, address = 0;
    int bank = 0;

    if (id == 1){
        address = PSU1_I2C_ADDR;
    }else{
        address = PSU2_I2C_ADDR;
    }
    bank = PMBUS_MODEL_REG;
    ret = read_smbus_block(bus, address, bank, cblock);
    if (ret > 0)
        strcpy(model, (char *)cblock);
    else
        return -1;

    bank = PMBUS_SERIAL_REG;
    ret = read_smbus_block(bus, address, bank, cblock);
    if (ret > 0)
        strcpy(serial_number, (char *)cblock);
    else
        return -1;

    return 0;
}

int get_fan_info(int id, char *model, char *serial, int *isfanb2f)
{
    uint8_t value = 0, i = 0;

    if (0 == strcasecmp(fan_information[0].model, "unknown")) {
        value = read_register(CPLD_FAN_DIR_REG);
        for(i = 0; i < FAN_COUNT; i++)
        {
            /* bit0--Fan1:0-F2B,1-B2F*/
            fan_information[i + 1].airflow = (value >> i & 1) ? ONLP_FAN_STATUS_B2F : ONLP_FAN_STATUS_F2B;
        }
        strcpy(fan_information[0].model,"pass"); //Mark as complete
    }
    /* all chassis and PSU fans SN and model are null in Belgite, so not set here*/
    // strcpy(model, fan_information[id].model);
    // strcpy(serial, fan_information[id].serial_number);
    *isfanb2f = fan_information[id].airflow;

    return 0;
}

int get_sensor_info(int id, int *temp, int *warn, int *error, int *shutdown)
{
    int ret = 0, fail = 0;;

     *temp = *warn = *error = *shutdown = 0;

    if((NULL == temp) || (NULL == warn) || (NULL == error) || (NULL == shutdown))
    {
        printf("%s null pointer!\n", __FUNCTION__);
        return -1;
    }
    switch (id)
    {
        case CHASSIS_THERMAL1_ID:
            *warn = CHASSIS_THERMAL1_HI_WARN;
            *error = CHASSIS_THERMAL1_HI_ERR;
            *shutdown = CHASSIS_THERMAL1_HI_SHUTDOWN;
            ret = get_sysnode_value(CHASSIS_THERMAL1, temp);
            if(ret < 0 || *temp == 0)
            {
                fail++;
            }
            break;
        case CHASSIS_THERMAL2_ID:
            *warn = CHASSIS_THERMAL2_HI_WARN;
            *error = CHASSIS_THERMAL2_HI_ERR;
            *shutdown = CHASSIS_THERMAL2_HI_SHUTDOWN;
            ret = get_sysnode_value(CHASSIS_THERMAL2, temp);
            if(ret < 0 || *temp == 0)
            {
                fail++;
            }
            break;
        case CHASSIS_THERMAL3_ID:
            *warn = CHASSIS_THERMAL3_HI_WARN;
            *error = CHASSIS_THERMAL3_HI_ERR;
            *shutdown = CHASSIS_THERMAL3_HI_SHUTDOWN;
            ret = get_sysnode_value(CHASSIS_THERMAL3, temp);
            if(ret < 0 || *temp == 0)
            {
                fail++;
            }
            break;
        case CHASSIS_THERMAL4_ID:
            *warn = CHASSIS_THERMAL4_HI_WARN;
            *error = CHASSIS_THERMAL4_HI_ERR;
            *shutdown = CHASSIS_THERMAL4_HI_SHUTDOWN;
            ret = get_sysnode_value(CHASSIS_THERMAL4, temp);
            if(ret < 0 || *temp == 0)
            {
                fail++;
            }
            break;
        case CPU_THERMAL_ID:
            *warn = CPU_THERMAL_HI_WARN;
            *error = CPU_THERMAL_HI_ERR;
            *shutdown = CPU_THERMAL_HI_SHUTDOWN;
            ret = get_sysnode_value(CPU_THERMAL, temp);
            if(ret < 0 || *temp == 0)
            {
                fail++;
            }
            break;
        case PSU1_THERMAL1_ID:
            *warn = PSU1_THERMAL1_HI_WARN;
            *error = PSU1_THERMAL1_HI_ERR;
            *shutdown = PSU1_THERMAL1_HI_SHUTDOWN;
            ret = get_sysnode_value(PSU1_THERMAL1, temp);
            if(ret < 0 || *temp == 0)
            {
                fail++;
            }
            break;
        case PSU1_THERMAL2_ID:
            *warn = PSU1_THERMAL2_HI_WARN;
            *error = PSU1_THERMAL2_HI_ERR;
            *shutdown = PSU1_THERMAL2_HI_SHUTDOWN;
            ret = get_sysnode_value(PSU1_THERMAL2, temp);
            if(ret < 0 || *temp == 0)
            {
                fail++;
            }
            break;
        case PSU2_THERMAL1_ID:
            *warn = PSU2_THERMAL1_HI_WARN;
            *error = PSU2_THERMAL1_HI_ERR;
            *shutdown = PSU2_THERMAL1_HI_SHUTDOWN;
            ret = get_sysnode_value(PSU2_THERMAL1, temp);
            if(ret < 0 || *temp == 0)
            {
                fail++;
            }
            break;
        case PSU2_THERMAL2_ID:
            *warn = PSU2_THERMAL2_HI_WARN;
            *error = PSU2_THERMAL2_HI_ERR;
            *shutdown = PSU2_THERMAL2_HI_SHUTDOWN;
            ret = get_sysnode_value(PSU2_THERMAL2, temp);
            if(ret < 0 || *temp == 0)
            {
                fail++;
            }
            break;
        default:
            break;
    }

    return fail;
}

int get_fan_speed(int id,int *per, int *rpm)
{
    int ret = 0, fail = 0;;

    *rpm = *per = 0;

    if((NULL == per) || (NULL == rpm))
    {
        printf("%s null pointer!\n", __FUNCTION__);
        return -1;
    }
    switch (id)
    {
        case CHASSIS_FAN1_ID:
            ret = get_sysnode_value(CHASSIS_FAN1_SPEED, rpm);
            if(ret < 0 || *rpm == 0)
            {
                fail++;
            }
            else
            {
                *per = *rpm * 100 / CHASSIS_FAN_MAX_RPM;
            }
            break;
        case CHASSIS_FAN2_ID:
            ret = get_sysnode_value(CHASSIS_FAN2_SPEED, rpm);
            if(ret < 0 || *rpm == 0)
            {
                fail++;
            }
            else
            {
                *per = *rpm * 100 / CHASSIS_FAN_MAX_RPM;
            }
            break;
        case CHASSIS_FAN3_ID:
            ret = get_sysnode_value(CHASSIS_FAN3_SPEED, rpm);
            if(ret < 0 || *rpm == 0)
            {
                fail++;
            }
            else
            {
                *per = *rpm * 100 / CHASSIS_FAN_MAX_RPM;
            }
            break;
        case PSU1_FAN_ID:
            ret = get_sysnode_value(PSU1_FAN, rpm);
            if(ret < 0 || *rpm == 0)
            {
                fail++;
            }
            else
            {
                *per = *rpm * 100 / PSU_FAN_MAX_RPM;
            }
            break;
        case PSU2_FAN_ID:
            ret = get_sysnode_value(PSU2_FAN, rpm);
            if(ret < 0 || *rpm == 0)
            {
                fail++;
            }
            else
            {
                *per = *rpm * 100 / PSU_FAN_MAX_RPM;
            }
            break;
        default:
            break;
    }

    return fail;
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
