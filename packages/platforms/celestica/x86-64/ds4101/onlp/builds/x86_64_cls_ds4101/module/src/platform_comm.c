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
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/fani.h>
#include "platform_comm.h"
#include "platform_wbmc.h"
#include "platform_wobmc.h"


extern uint8_t BMC_Status;

static const struct led_reg_mapper led_mapper[LED_COUNT + 1] = {
    {},
    {"LED_SYSTEM", LED_SYSTEM, LED_SYSTEM_REG},
    {"LED_ALARM",  LED_ALARM,  LED_ALARM_REG},
    {"LED_PSU",    LED_PSU,    LED_PSU_REG},
    {"LED_FAN",    LED_FAN,    LED_FAN_REG}
};

const struct psu_reg_bit_mapper psu_mapper [PSU_COUNT + 1] = {
    {},
    {PSU_STA_REG, 3, 5, 7, 1},
    {PSU_STA_REG, 2, 4, 6, 0},
};

char* trim (char *s)
{
    int i;

    while (isspace (*s)) s++;                               // skip left side white spaces
    for (i = strlen (s) - 1; (isspace (s[i])); i--) ;    // skip right side white spaces
    s[i + 1] = '\0';

    return s;
}

void array_trim(char *strIn, char *strOut)
{
    int i, j;

    i = 0;
    j = strlen(strIn) - 1;

    while(strIn[i] == ' ') ++i;
    while(strIn[j] == ' ') --j;

    /* in case all characters are spaces */
    if (-1 != j) {
        strncpy(strOut, strIn + i , j - i + 1);
        strOut[j - i + 1] = '\0';
    }
}

int exec_cmd(char *cmd, char *retd)
{
    int ret = 0;
    int i = 0;
    char c;
    FILE *pFd = NULL;

    pFd = popen(cmd, "r");
    if (pFd != NULL) {
        c = fgetc(pFd);
        while (c != EOF) {
            retd[i] = c;
            i++;
            c = fgetc(pFd);
        }
        pclose(pFd);
    }

    return ret;
}

uint8_t read_register(uint16_t dev_reg)
{
    char command[256];
    uint8_t value;
    int fd = 0;
    int ret = 0;

    sprintf(command, "%sdump", SYS_CPLD_PATH);
    fd = open(command, O_RDONLY);
    if (fd < 0) {
        printf("ERROR: Fail to open the file: %s \n", command);
        return -1;
    }
    lseek(fd, dev_reg, SEEK_SET);
    ret = read(fd, &value, sizeof(value));
    if (ret < 0) {
        printf("ERROR: Fail to read register: 0x%x \n", value);
        close(fd);
        return -1;
    }
    close(fd);

    return value;
}

int read_device_node_binary(char *filename, char *buffer, int buf_size, int data_len)
{
    int fd;
    int len;

    if ((buffer == NULL) || (buf_size < 0))
        return -1;

    if ((fd = open(filename, O_RDONLY)) == -1)
        return -1;

    if ((len = read(fd, buffer, buf_size)) < 0)
    {
        close(fd);
        return -1;
    }

    if ((close(fd) == -1))
        return -1;

    if ((len > buf_size) || (data_len != 0 && len != data_len))
        return -1;

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

int get_sysnode_value(const char *path, void *data)
{
    int fd;
    int ret = 0;
    FILE *fp = NULL;
    char new_path[80] = {0};
    char buffer[20] = {0};
    long *value = (long *)data;

    /* replace hwmon* with hwmon[0-9] */
    sprintf(new_path, "ls %s", path);
    fp = popen(new_path, "r");
    if (fp == NULL)
    {
        printf("call popen %s fail\n", new_path);
        return -1;
    }
    fscanf(fp, "%s", new_path);
    pclose(fp);

    fd = open(new_path, O_RDONLY);
    if (fd < 0) {
        printf("open %s fail\n", new_path);
        ret = -1;
    } else {
        read(fd, buffer, sizeof(buffer));
        *value = atol(buffer);
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
    if (fd < 0) {
        ret = -1;
    } else {
        sprintf(buffer, "%d", data);
        write(fd, buffer, strlen(buffer));
        close(fd);
    }

    return ret;
}

int syscpld_setting(uint32_t reg, uint8_t val)
{
    char data[30] = {0};
    char path[100] = {0};
    int fd, rc = 0;

    sprintf(path, "%s%s",SYS_CPLD_PATH, "setreg");
    sprintf(data, "0x%x 0x%x",reg, val);

    fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("Fail to open the file: %s \n", path);
        return ERR;
    }

    rc = write(fd, data, strlen(data));
    if (rc != strlen(data)) {
        printf("Write failed. count=%lu, rc=%d \n", strlen(data), rc);
        close(fd);
        return ERR;
    }

    close(fd);

    return OK;
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



int read_eeprom(char* path, int offset, char* data, unsigned int *count)
{
    int ret = 0;

    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        printf("Fail to open the file: %s \n", path);
        return -1;
    }
    if (offset > 0) {
        lseek(fd, offset, SEEK_SET);
    }

    size_t len = read(fd, data, *count);

    if (len == 0) {
        DEBUG_PRINT("The file(%s) is empty, count=%d. \n", path, *count);
    } else if (len < 0) {
        perror("Can't read eeprom");
        return ret;
    }

    *count = len;
    close(fd);

    return ret;
}


uint8_t get_led_status(int id)
{
    uint8_t ret = 0xFF;

    if (id > LED_COUNT || id < 0)
        return 0xFF;

    if (id <= LED_COUNT)
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

int get_psu_info(int id, int *mvin, int *mvout, int *mpin, int *mpout, int *miin, int *miout)
{
    int ret = 0;

    if (PRESENT == BMC_Status) {
        ret = get_psu_info_wbmc(id, mvin, mvout, mpin, mpout, miin, miout);
    } else {
        ret = get_psu_info_wobmc(id, mvin, mvout, mpin, mpout, miin, miout);
    }

    return ret;
}

int get_psu_model_sn(int id, char *model, char *serial_number)
{
    int ret = 0;

    if (PRESENT == BMC_Status) {
        ret = get_psu_model_sn_wbmc(id, model, serial_number);
    } else {
        ret = get_psu_model_sn_wobmc(id, model, serial_number);
    }

    return ret;
}

int get_fan_info(int id, char *model, char *serial, int *isfanb2f)
{
    int ret = 0;

    if (PRESENT == BMC_Status) {
        ret = get_fan_info_wbmc(id, model, serial, isfanb2f);
    } else {
        ret = get_fan_info_wobmc(id, model, serial, isfanb2f);
    }

    return ret;
}

int get_sensor_info(int id, int *temp, int *warn, int *error, int *shutdown)
{
    int ret = 0;

    if (PRESENT == BMC_Status) {
        ret = get_sensor_info_wbmc(id, temp, warn, error, shutdown);
    } else {
        ret = get_sensor_info_wobmc(id, temp, warn, error, shutdown);
    }

    return ret;
}

int get_fan_speed(int id,int *per, int *rpm)
{
    int ret = 0;

    if (PRESENT == BMC_Status) {
        ret = get_fan_speed_wbmc(id, per, rpm);
    } else {
        ret = get_fan_speed_wobmc(id, per, rpm);
    }

    return ret;
}
