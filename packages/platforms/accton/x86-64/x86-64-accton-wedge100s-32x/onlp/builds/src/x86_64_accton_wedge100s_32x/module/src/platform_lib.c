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
#include <unistd.h>
#include <fcntl.h>
#include <onlplib/file.h>
#include <onlp/onlp.h>
#include "platform_lib.h"

#define TTY_DEVICE                      "/dev/ttyACM0"
#define TTY_PROMPT                      "@bmc:"
#define TTY_I2C_TIMEOUT                 60000
#define TTY_BMC_LOGIN_TIMEOUT			1000000
#define TTY_RETRY						10
#define MAXIMUM_TTY_BUFFER_LENGTH       1024
#define MAXIMUM_TTY_STRING_LENGTH       (MAXIMUM_TTY_BUFFER_LENGTH - 1)

static int  tty_fd = -1;
static char tty_buf[MAXIMUM_TTY_BUFFER_LENGTH] = {0};

static int tty_open(void)
{
    int i = 20;
    struct termios attr;

    if (tty_fd > -1) {
        return 0;
    }

    do {
        if ((tty_fd = open(TTY_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY)) > -1) {
            tcgetattr(tty_fd, &attr);
            attr.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
            attr.c_iflag = IGNPAR;
            attr.c_oflag = 0;
            attr.c_lflag = 0;
            attr.c_cc[VMIN] = (unsigned char)
                ((MAXIMUM_TTY_STRING_LENGTH > 0xFF) ?  0xFF : MAXIMUM_TTY_STRING_LENGTH);
            attr.c_cc[VTIME] = 0;
            cfsetospeed(&attr, B57600);
            cfsetispeed(&attr, B57600);
            tcsetattr(tty_fd, TCSANOW, &attr);
            return 0;
        }

        i--;
        usleep(100000);
    } while (i > 0);

    return -1;
}

static int tty_close(void)
{
    close(tty_fd);
    tty_fd = -1;
    return 0;
}

static int tty_exec_buf(unsigned long udelay, const char *str)
{
    if (tty_fd < 0)
        return -1;

    write(tty_fd, tty_buf, strlen(tty_buf)+1);
    usleep(udelay);
    read(tty_fd, tty_buf, MAXIMUM_TTY_BUFFER_LENGTH);
    return (strstr(tty_buf, str) != NULL) ? 0 : -1;
}

static int tty_login(void)
{
    int i = 10;

	for (i = 1; i <= TTY_RETRY; i++) {
        snprintf(tty_buf, MAXIMUM_TTY_BUFFER_LENGTH, "\r\r");
        if (!tty_exec_buf(0, TTY_PROMPT)) {
            return 0;
        }

        if (strstr(tty_buf, "bmc login:") != NULL)
        {
            snprintf(tty_buf, MAXIMUM_TTY_BUFFER_LENGTH, "root\r");
			
            if (!tty_exec_buf(TTY_BMC_LOGIN_TIMEOUT, "Password:")) {
                snprintf(tty_buf, MAXIMUM_TTY_BUFFER_LENGTH, "0penBmc\r");
                if (!tty_exec_buf(TTY_BMC_LOGIN_TIMEOUT, TTY_PROMPT)) {
                    return 0;
                }

            }
        }
        usleep(50000);
	}

    return -1;
}

int bmc_send_command(char *cmd)
{
	int i, ret = 0;
	
	for (i = 1; i <= TTY_RETRY; i++) {
	    if (tty_open() != 0) {
	        printf("ERROR: Cannot open TTY device\n");
	        continue;
	    }
	    if (tty_login() != 0) {
	        //printf("ERROR: Cannot login TTY device\n");
	        tty_close();
	        continue;
	    }

		snprintf(tty_buf, MAXIMUM_TTY_BUFFER_LENGTH, "%s", cmd);
	    ret = tty_exec_buf(TTY_I2C_TIMEOUT * i, TTY_PROMPT);
	    tty_close();
	    if (ret != 0) {
	        printf("ERROR: bmc_send_command timed out\n");
	        continue;
	    }

		return 0;
	}

	AIM_LOG_ERROR("Unable to send command to bmc(%s)\r\n", cmd);
	return -1;
}

int
bmc_command_read_int(int* value, char *cmd, int base)
{
    int len;
    int i;
    char *prev_str = NULL;
    char *current_str= NULL;
    if (bmc_send_command(cmd) < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    len = (int)strlen(cmd);
    prev_str = strstr(tty_buf, cmd);
    if (prev_str == NULL) {
        return -1;
    }
    for (i = 1; i <= TTY_RETRY; i++) {
        current_str = strstr(prev_str + len, cmd);
        if(current_str == NULL) {
            *value = strtoul(prev_str + len, NULL, base);
            break;
        }else {
            prev_str = current_str;
            continue;
        }
    }
    return 0;
}

int
bmc_file_read_int(int* value, char *file, int base)
{
	char cmd[64] = {0};
	snprintf(cmd, sizeof(cmd), "cat %s\r\n", file);
	return bmc_command_read_int(value, cmd, base);
}

int
bmc_i2c_readb(uint8_t bus, uint8_t devaddr, uint8_t addr)
{
	int ret = 0, value;
	char cmd[64] = {0};

	snprintf(cmd, sizeof(cmd), "i2cget -f -y %d 0x%x 0x%02x\r\n", bus, devaddr, addr);
	ret = bmc_command_read_int(&value, cmd, 16);
	return (ret < 0) ? ret : value;
}

int
bmc_i2c_writeb(uint8_t bus, uint8_t devaddr, uint8_t addr, uint8_t value)
{
	char cmd[64] = {0};
	snprintf(cmd, sizeof(cmd), "i2cset -f -y %d 0x%x 0x%02x 0x%x\r\n", bus, devaddr, addr, value);
	return bmc_send_command(cmd);
}

int
bmc_i2c_readw(uint8_t bus, uint8_t devaddr, uint8_t addr)
{
	int ret = 0, value;
	char cmd[64] = {0};

	snprintf(cmd, sizeof(cmd), "i2cget -f -y %d 0x%x 0x%02x w\r\n", bus, devaddr, addr);
	ret = bmc_command_read_int(&value, cmd, 16);
	return (ret < 0) ? ret : value;
}

int
bmc_i2c_readraw(uint8_t bus, uint8_t devaddr, uint8_t addr, char* data, int data_size)
{
	int data_len, i = 0;
	char cmd[64] = {0};
	char *str = NULL;
	snprintf(cmd, sizeof(cmd), "i2craw -w 0x%x -r 0 %d 0x%02x\r\n", addr, bus, devaddr);

    if (bmc_send_command(cmd) < 0) {
        AIM_LOG_ERROR("Unable to send command to bmc(%s)\r\n", cmd);
        return ONLP_STATUS_E_INTERNAL;
    }

	str = strstr(tty_buf, "Received:\r\n  ");
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

