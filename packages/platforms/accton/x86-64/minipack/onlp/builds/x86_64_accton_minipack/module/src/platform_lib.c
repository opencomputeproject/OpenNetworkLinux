/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2017 Accton Technology Corporation.
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
#include <termios.h>
#include <sys/file.h>
#include <fcntl.h>
#include <onlplib/file.h>
#include <onlp/onlp.h>
#include "platform_lib.h"

#define TTY_DEVICE                      "/dev/ttyACM0"
#define TTY_USER                        "root"
#define TTY_PROMPT                      TTY_USER"@"
#define TTY_I2C_WAIT_REPLY              200000
#define TTY_BMC_LOGIN_TIMEOUT           1000000
#define TTY_BMC_LOGIN_INTERVAL          50000
#define TTY_LOGIN_RETRY                 (20)
#define TTY_RETRY_INTERVAL              100000
#define TTY_RETRY                       PLATFOTM_H_TTY_RETRY

static bool global_logged_in = false;

static int tty_open(int *fd)
{
    int ret;
    int i = 20;
    struct termios attr;

    if (*fd > -1) {
        return ONLP_STATUS_OK;
    }

    do {
        if ((*fd = open(TTY_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY)) > -1) {
            ret = flock(*fd, LOCK_EX | LOCK_NB);
            if (ret == -1 && errno != EWOULDBLOCK) {
                AIM_LOG_ERROR("ERROR: Cannot flock TTY device\n");
                return ONLP_STATUS_E_INTERNAL;
            } else {
                tcgetattr(*fd, &attr);
                attr.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
                attr.c_iflag = IGNPAR | IGNCR;
                attr.c_oflag = 0;
                attr.c_lflag = 0;
                attr.c_cc[VMIN] = (unsigned char)
                                  ((MAXIMUM_TTY_STRING_LENGTH > 0xFF) ?  0xFF : MAXIMUM_TTY_STRING_LENGTH);
                attr.c_cc[VTIME] = 0;
                cfsetospeed(&attr, B57600);
                cfsetispeed(&attr, B57600);
                tcsetattr(*fd, TCSANOW, &attr);
                return ONLP_STATUS_OK;
            }
        }
        i--;
    } while (i > 0);
    return ONLP_STATUS_E_GENERIC;
}

static int tty_close(int *fd)
{
    if (flock(*fd, LOCK_UN) == -1) {
        AIM_LOG_ERROR("ERROR: Cannot unlock TTY device file\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    close(*fd);
    *fd = -1;
    return 0;
}

static int tty_clear_rxbuf(int fd, char* buf, int max_size) {
    int ret, i;

    if (!buf || fd < 0) {
        return ONLP_STATUS_E_PARAM;
    }
    do {
        ret = read(fd, buf, max_size);
        memset(buf, 0, max_size);
        i++;
    } while (ret > 0 && i < TTY_RETRY);
    return ret;
}

static int tty_write_and_read( int fd, const char *cmd,
                               uint32_t udelay, char *buf, int buf_size)
{
    int ret, len, retry;

    if (fd < 0 || !cmd) {
        return ONLP_STATUS_E_PARAM;
    }
    tty_clear_rxbuf(fd, buf, buf_size);
    len = strlen(cmd) + 1;
    retry = 0;
    do {
        ret = write(fd, cmd, len);
        retry++ ;
    } while(ret != len && retry < TTY_RETRY && usleep(TTY_RETRY_INTERVAL*retry));

    DEBUG_PRINT("sent cmd:%s\n",cmd);
    usleep(udelay);
    memset(buf, 0, buf_size);
    ret = read(fd, buf, buf_size);
    DEBUG_PRINT("Read %d bytes, \"%s\", \n", ret, buf);
    return ret;
}

static int tty_access_and_match( int fd, const char *cmd,
                                 uint32_t udelay, const char *keywd)
{
    int num;
    char resp[MAX_TTY_CMD_LENGTH] = {0};

    num = tty_write_and_read(fd, cmd, udelay, resp, sizeof(resp));
    if (num <= 0) {
        return ONLP_STATUS_E_GENERIC;
    }
    return (strstr(resp, keywd) != NULL) ?
           ONLP_STATUS_OK : ONLP_STATUS_E_GENERIC;
}

static bool is_logged_in(int fd, char *resp, int max_size)
{
    int num;
    char *p;

    if (global_logged_in == true) {
        return true;
    }

    num = tty_write_and_read(fd, "\r\r", TTY_BMC_LOGIN_TIMEOUT, resp, max_size);
    if (num <= 0) {
        return false;
    }
    p = strstr(resp, TTY_PROMPT);
    if (p != NULL ) {
        global_logged_in = true;
        return true;
    } else {
        return false;
    }
}

static int get_passwd(char *buf, int buf_size)
{
    char *file_pw = "/etc/bmcpwd";

    /*check if file exists */
    if( access( file_pw, F_OK ) != -1 ){
        char *pw = NULL;
        int len = onlp_file_read_str(&pw, file_pw);
        if (!pw || len <= 0) {
            aim_free(pw);
            return ONLP_STATUS_E_INTERNAL;
        }
        AIM_MEMCPY(buf, pw, buf_size);
        buf[len] = '\0';
        aim_free(pw);
    } else {
        char *pw = "0penBmc";
        int len = strlen(pw);
        AIM_MEMCPY(buf, pw, buf_size);
        buf[len] = '\0';
    }
    return ONLP_STATUS_OK;
}

static int tty_login(int fd, char *buf, int buf_size)
{
    int i;

    for (i = 0; i < TTY_LOGIN_RETRY; i++) {
        if (is_logged_in(fd, buf, buf_size)) {
            DEBUG_PRINT("Been logged in!\n");
            return ONLP_STATUS_OK;
        }

        DEBUG_PRINT("Try to login, @%d!\n", i);
        if (strstr(buf, " login:") != NULL)
        {
            char pw[128+2];
            get_passwd(pw, 128);
            AIM_STRCAT(pw, "\r");
            if (!tty_access_and_match(fd, TTY_USER"\r",TTY_BMC_LOGIN_TIMEOUT, "Password:")) {
                if (!tty_access_and_match(fd, pw, TTY_BMC_LOGIN_TIMEOUT, TTY_PROMPT)) {
                    return ONLP_STATUS_OK;
                }

            }
        }
        usleep(TTY_BMC_LOGIN_INTERVAL*i);
    }

    return ONLP_STATUS_E_GENERIC;
}

static int
tty_transaction(const char *cmd, uint32_t udelay, char *resp, int max_size)
{
    char *buf;
    int  tty_fd = -1;
    int  num, ret = ONLP_STATUS_OK;
    int  buf_size = MAXIMUM_TTY_BUFFER_LENGTH;

    if (!cmd || !resp || !max_size)
        return ONLP_STATUS_E_PARAM;

    if (tty_open(&tty_fd) != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("ERROR: Cannot open TTY device\n");
        return ONLP_STATUS_E_GENERIC;
    }

    buf = (char *)calloc(buf_size, sizeof(char));
    if (buf == NULL)
    {
        AIM_LOG_ERROR("ERROR: Cannot allocate memory\n");
        goto exit;
    }
    ret = tty_login(tty_fd, buf, buf_size);
    if (ret != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("ERROR: Cannot login TTY device\n");
        goto exit;
    }
    num = tty_write_and_read(tty_fd, cmd, udelay, buf, buf_size);
    if (num <= 0)
    {
        AIM_LOG_ERROR("ERROR: Cannot read/write TTY device\n");
        goto exit;
    }
    aim_strlcpy(resp, buf, max_size);
exit:
    free(buf);
    tty_close(&tty_fd);
    return ret;
}

static int chk_numeric_char(char *data, int base)
{
    int len, i, orig = 0;

    if( *data == '\0' ) {
        return 0;
    }
    len = (int) strlen(data);
    if( base == 10 ) {
        for( i=0; i<len; i++ ) {
            if( i && ( *(data+i) == 0xd ) ) {
                break;
            }
            if( i && ( *(data+i) == 0xa ) ) {
                break;
            }
            if( *(data+i) < '0' ) {
                return 0;
            }
            if( *(data+i) > '9' ) {
                return 0;
            }
        }
        return 1;
    }
    else if( base == 16 ) {
        if( !memcmp(data, "0x", 2) ) {
            if( len <= 2 ) {
                return 0;
            }
            orig = 2;
        }
        else if( !memcmp(data, "0X", 2) ) {
            if( len <= 2 ) {
                return 0;
            }
            orig = 2;
        }
        for( i=orig; i<len; i++ ) {
            if( (i > orig) && ( *(data+i) == 0xd ) ) {
                break;
            }
            if( (i > orig) && ( *(data+i) == 0xa ) ) {
                break;
            }
            if( !( ( (*(data+i) >= '0') && (*(data+i) <= '9') ) ||
                    ( (*(data+i) >= 'A') && (*(data+i) <= 'F') ) ||
                    ( (*(data+i) >= 'a') && (*(data+i) <= 'f') )
                 )
              ) {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

static int strip_off_prompt(char *buf, int max_size)
{
    char *p;

    p = strstr(buf, TTY_PROMPT);
    if (p != NULL && p < (buf+max_size)) {
        *p = '\0';
    }
    return  ONLP_STATUS_OK;
}

static char * deblank(char *str)
{
    char *out = str, *put = str;

    for(; *str != '\0'; ++str)
    {
        if (*str != ' '  && *str != '\r' && *str != '\n')
            *put++ = *str;
    }
    *put = '\0';

    return out;
}

static bool _strInStr(const char *str1, const char *str2) {
    char *p1, *p2;
    char o1[128];
    char o2[128];

    strcpy(o1, str1);
    strcpy(o2, str2);
    p1 = deblank(o1);
    p2 = deblank(o2);

    return !!strstr(p1, p2);
}


int bmc_reply_pure(char *cmd, uint32_t udelay, char *resp, int max_size)
{
    int i, ret = 0;
    char cmdr[MAX_TTY_CMD_LENGTH];

    /*In case, caller forgets put the "enter" at the very end of cmd.*/
    snprintf(cmdr, sizeof(cmdr), "%s%s", cmd, "\r");

    for (i = 1; i <= TTY_RETRY; i++) {
        ret = tty_transaction(cmdr, udelay, resp, max_size);
        if (ret != ONLP_STATUS_OK) {
            usleep(100000*i);
            continue;
        }
        strip_off_prompt(resp, max_size);
        /*Find if cmd is inside the response.*/
        if (_strInStr(resp, cmd)) {
            memcpy(resp, &resp[strlen(cmdr)], max_size);
            return ONLP_STATUS_OK;
        }
        DEBUG_PRINT("Resp: [%s]\n", resp);
    }
    AIM_LOG_ERROR("Unable to send command to bmc(%s)\r\n", cmd);
    return ONLP_STATUS_E_GENERIC;
}

int bmc_reply(char *cmd, char *resp, int max_size)
{
    int i, ret = 0;

    for (i = 1; i <= TTY_RETRY; i++) {
        ret = tty_transaction(cmd, TTY_I2C_WAIT_REPLY, resp, max_size);
        if (ret != ONLP_STATUS_OK) {
            continue;
        }
        if (strstr(resp, TTY_PROMPT) == NULL) {
            continue;
        }
        return ONLP_STATUS_OK;
    }
    DEBUG_PRINT("Unable to send command to bmc(%s)\r\n", cmd);
    return ONLP_STATUS_E_GENERIC;
}

int
bmc_command_read_int(int *value, char *cmd, int base)
{
    char resp[MAX_TTY_CMD_LENGTH];
    char *prev_str = NULL;

    if (bmc_reply(cmd, resp, sizeof(resp)) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }
    if (! _strInStr(resp, cmd)) {
        return ONLP_STATUS_E_INTERNAL;
    }
    prev_str = &resp[strlen(cmd)-1];
    if (base == 16) {
        if (sscanf(prev_str, "%x", value)!= 1) {
            return ONLP_STATUS_E_INTERNAL;
        }
    } else {
        if( !chk_numeric_char(prev_str, base) ) {
            return ONLP_STATUS_E_INTERNAL;
        }
        *value = strtoul(prev_str, NULL, base);
    }
    return 0;
}


int
bmc_file_read_int(int* value, char *file, int base)
{
    char cmd[MAX_TTY_CMD_LENGTH] = {0};
    snprintf(cmd, sizeof(cmd), "cat %s\r\n", file);
    return bmc_command_read_int(value, cmd, base);
}

int
bmc_i2c_readb(uint8_t bus, uint8_t devaddr, uint8_t addr)
{
    int ret = 0, value;
    char cmd[MAX_TTY_CMD_LENGTH] = {0};

    snprintf(cmd, sizeof(cmd), "i2cget -f -y %d 0x%x 0x%02x\r\n", bus, devaddr, addr);
    ret = bmc_command_read_int(&value, cmd, 16);
    return (ret < 0) ? ret : value;
}

int
bmc_i2c_writeb(uint8_t bus, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    char cmd[MAX_TTY_CMD_LENGTH] = {0};
    char resp[MAX_TTY_CMD_LENGTH];
    snprintf(cmd, sizeof(cmd), "i2cset -f -y %d 0x%x 0x%02x 0x%x\r\n", bus, devaddr, addr, value);
    return bmc_reply(cmd, resp, sizeof(resp));
}

int
bmc_i2c_readw(uint8_t bus, uint8_t devaddr, uint8_t addr, uint16_t *data)
{
    int ret = 0, value;
    char cmd[MAX_TTY_CMD_LENGTH] = {0};

    snprintf(cmd, sizeof(cmd), "i2cget -f -y %d 0x%x 0x%02x w\r\n", bus, devaddr, addr);
    ret = bmc_command_read_int(&value, cmd, 16);
    *data = value;
    return ret;
}

int
bmc_i2c_readraw(uint8_t bus, uint8_t devaddr, uint8_t addr, char* data, int data_size)
{
    int data_len, i = 0;
    char cmd[MAX_TTY_CMD_LENGTH] = {0};
    char resp[MAX_TTY_CMD_LENGTH];
    char *str = NULL;
    snprintf(cmd, sizeof(cmd), "i2craw -w 0x%x -r 0 %d 0x%02x\r\n", addr, bus, devaddr);

    if (bmc_reply(cmd, resp, sizeof(resp)) < 0) {
        AIM_LOG_ERROR("Unable to send command to bmc(%s)\r\n", cmd);
        return ONLP_STATUS_E_INTERNAL;
    }

    str = strstr(resp, "Received:\r\n  ");
    if (str == NULL) {
        return -1;
    }

    /* first byte is data length */
    str += strlen("Received:\r\n  ");;
    data_len = strtoul(str, NULL, 16);
    if (data_size < data_len) {
        data_len = data_size;
    }

    for (i = 0; (i < data_len) && (str != NULL); i++) {
        str = strstr(str, " ") + 1; /* Jump to next token */
        data[i] = strtoul(str, NULL, 16);
    }

    data[i] = 0;
    return 0;
}

uint32_t pltfm_create_sem (sem_t *mutex)
{
    int         rc;

    rc = sem_init(mutex, 1, 1);
    if (rc != 0) {
        AIM_DIE("%s failed, errno %d.", __func__, errno);
        return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}

