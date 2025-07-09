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

int
onlp_file_read_hex(int* value, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    uint8_t  data[16]={0};
    int len;
    rv = onlp_file_vread(data, 16, &len, fmt, vargs);
    va_end(vargs);
    *value = strtol((const char * )data, NULL, 0);// Base 0 automatically detects hex
    return rv;
}

int
onlp_file_vwrite_hex(int value, const char* fmt, va_list vargs)
{
    int rv;
    char* s = aim_fstrdup("0x%x", value);
    rv = onlp_file_vwrite_str(s, fmt, vargs);
    aim_free(s);
    return rv;
}


int
onlp_file_write_hex(int value, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = onlp_file_vwrite_hex(value, fmt, vargs);
    va_end(vargs);
    return rv;
}

int 
onlp_file_readb(int* value,int offset, const char* fmt, ...)
{
    int fd;
    va_list vargs;
    uint8_t data;
    va_start(vargs, fmt);
    fd = onlp_file_vopen(O_RDONLY, 0, fmt, vargs);
    va_end(vargs);
    if (fd<0)
        return ONLP_STATUS_E_INTERNAL;
    lseek(fd, offset, SEEK_SET);
    if(read(fd, &data, 1) <=0){
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }
    close(fd);
    *value=data;
    return ONLP_STATUS_OK;
}

int 
onlp_file_readw(int* value,int offset, const char* fmt, ...)
{
    int fd;
    va_list vargs;
    uint8_t data[2];
    va_start(vargs, fmt);
    fd = onlp_file_vopen(O_RDONLY, 0, fmt, vargs);
    va_end(vargs);
    if (fd<0)
        return ONLP_STATUS_E_INTERNAL;
    lseek(fd, offset, SEEK_SET);
    if(read(fd, &data, 2) <=0){
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }
    close(fd);
    *value=data[0]<<8|data[1];
    return ONLP_STATUS_OK;
}

int 
onlp_file_writeb(int value,int offset, const char* fmt, ...)
{
    int fd;
    va_list vargs;
    uint8_t data=value;
    va_start(vargs, fmt);
    fd = onlp_file_vopen(O_WRONLY, 0, fmt, vargs);
    va_end(vargs);
    if (fd<0)
        return ONLP_STATUS_E_INTERNAL;
    lseek(fd, offset, SEEK_SET);
    if(write(fd, &data, 1) <=0){
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }
    close(fd);
    return ONLP_STATUS_OK;
}

int 
onlp_file_writew(int value,int offset, const char* fmt, ...)
{
    int fd;
    va_list vargs;
    uint8_t data[2];
    data[1]=value&0xff;
    data[0]=(value >> 8 )&0xff;
    va_start(vargs, fmt);
    fd = onlp_file_vopen(O_WRONLY, 0, fmt, vargs);
    va_end(vargs);
    if (fd<0)
        return ONLP_STATUS_E_INTERNAL;
    lseek(fd, offset, SEEK_SET);
    if(write(fd, &data, 2) <=0){
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }
    close(fd);
    return ONLP_STATUS_OK;
}