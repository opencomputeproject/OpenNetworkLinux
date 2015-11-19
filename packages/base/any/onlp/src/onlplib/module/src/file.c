/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
#include <onlplib/onlplib_config.h>
#include <onlplib/file.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include "onlplib_log.h"
#include <onlp/onlp.h>
#include <errno.h>

int
onlp_file_vread(uint8_t* data, int max, int* len, const char* fmt, va_list vargs)
{
    int fd;
    char fname[PATH_MAX];
    int rv;

    ONLPLIB_VSNPRINTF(fname, sizeof(fname)-1, fmt, vargs);

    if ((fd = open(fname, O_RDONLY)) == -1) {
        rv = ONLP_STATUS_E_MISSING;
    }
    else {
        if ((*len = read(fd, data, max)) <= 0) {
            AIM_LOG_ERROR("Failed to read input file '%s'", fname);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        else {
            rv = ONLP_STATUS_OK;
        }
        close(fd);
    }
    return rv;
}

int
onlp_file_read(uint8_t* data, int max, int* len, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = onlp_file_vread(data, max, len, fmt, vargs);
    va_end(vargs);
    return rv;
}

int
onlp_file_vread_int(int* value, const char* fmt, va_list vargs)
{
    int rv;
    uint8_t data[32];
    int len;
    rv = onlp_file_vread(data, sizeof(data), &len, fmt, vargs);
    if(rv < 0) {
        return rv;
    }
    *value = ONLPLIB_ATOI((char*)data);
    return 0;

}

int
onlp_file_read_int(int* value, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = onlp_file_vread_int(value, fmt, vargs);
    va_end(vargs);
    return rv;
}

int
onlp_file_read_int_max(int* value, char** files)
{
    char** s = NULL;
    int max = 0;

    if(value == NULL || files == NULL || *files == NULL) {
        return ONLP_STATUS_E_PARAM;
    }

    *value = 0;

    for(s = files; *s; s++) {
        int value = 0;
        int rv = onlp_file_read_int(&value, *s);
        if(rv < 0) {
            return rv;
        }
        if(max < value) {
            max = value;
        }
    }

    *value = max;
    return 0;
}

int
onlp_file_vwrite(uint8_t* data, int len, const char* fmt, va_list vargs)
{
    int fd;
    char fname[PATH_MAX];
    int rv;
    int wlen;

    ONLPLIB_VSNPRINTF(fname, sizeof(fname)-1, fmt, vargs);

    if ((fd = open(fname, O_WRONLY)) == -1) {
        rv = ONLP_STATUS_E_MISSING;
    }
    else {
        if ((wlen = write(fd, data, len)) != len) {
            AIM_LOG_ERROR("Failed to write output file '%s'", fname);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        else {
            rv = ONLP_STATUS_OK;
        }
        close(fd);
    }
    return rv;
}

int
onlp_file_write(uint8_t* data, int len, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = onlp_file_vwrite(data, len, fmt, vargs);
    va_end(vargs);
    return rv;
}

int
onlp_file_vwrite_str(const char* str, const char* fmt, va_list vargs)
{
    return onlp_file_vwrite((uint8_t*)str, strlen(str)+1, fmt, vargs);
}

int
onlp_file_write_str(const char* str, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = onlp_file_vwrite_str(str, fmt, vargs);
    va_end(vargs);
    return rv;
}

int
onlp_file_vwrite_int(int value, const char* fmt, va_list vargs)
{
    int rv;
    char* s = aim_fstrdup("%d", value);
    rv = onlp_file_vwrite_str(s, fmt, vargs);
    aim_free(s);
    return rv;
}

int
onlp_file_write_int(int value, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = onlp_file_vwrite_int(value, fmt, vargs);
    va_end(vargs);
    return rv;
}

int
onlp_file_open(int flags, int log, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = onlp_file_vopen(flags, log, fmt, vargs);
    va_end(vargs);
    return rv;
}


int
onlp_file_vopen(int flags, int log, const char* fmt, va_list vargs)
{
    int rv;
    char fname[PATH_MAX];
    ONLPLIB_VSNPRINTF(fname, sizeof(fname)-1, fmt, vargs);
    rv = open(fname, flags);
    if(rv < 0 && log) {
        AIM_LOG_ERROR("failed to open file %s (0x%x): %{errno}", fname, flags, errno);
    }
    return rv;
}
