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
#include "onlplib_log.h"
#include <onlplib/file.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include "onlplib_log.h"
#include <onlp/onlp.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

/**
 * @brief Connects to a unix domain socket.
 * @param path The socket path.
 */
static int
ds_connect__(const char* path)
{
    int fd;
    struct sockaddr_un addr;

    if( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        AIM_LOG_ERROR("socket: %{errno}", errno);
        return -1;
    }

    /*
     * UDS connects must be non-blocking.
     */
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);

    if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {

        /*
         * Set blocking with a 1 second timeout on all domain socket read/write operations.
         */
        struct timeval tv;

        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_NONBLOCK);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        return fd;
    }
    else {
        return ONLP_STATUS_E_MISSING;
    }
}

/**
 * @brief Open a file or domain socket.
 * @param dst Receives the full filename (for logging purposes).
 * @param flags The open flags.
 * @param fmt Format specifier.
 * @param vargs Format specifier arguments.
 */
static int
vopen__(char** dst, int flags, const char* fmt, va_list vargs)
{
    int fd;
    struct stat sb;
    char fname[PATH_MAX];
    char* asterisk;

    ONLPLIB_VSNPRINTF(fname, sizeof(fname)-1, fmt, vargs);

    /**
     * An asterisk in the filename separates a search root
     * directory from a filename.
     */
    if( (asterisk = strchr(fname, '*')) ) {
        char* root = fname;
        char* rpath = NULL;
        *asterisk = 0;
        if(onlp_file_find(root, asterisk+1, &rpath) < 0) {
            return ONLP_STATUS_E_MISSING;
        }
        strcpy(fname, rpath);
        aim_free(rpath);
    }

    if(dst) {
        *dst = aim_strdup(fname);
    }

    if(stat(fname, &sb) == -1) {
        return ONLP_STATUS_E_MISSING;
    }

    if(S_ISSOCK(sb.st_mode)) {
        fd = ds_connect__(fname);
    }
    else {
        fd = open(fname, flags);
    }

    return (fd > 0) ? fd : ONLP_STATUS_E_MISSING;
}

int
onlp_file_vsize(const char* fmt, va_list vargs)
{
    int rv;
    struct stat sb;

    char* fname = aim_vfstrdup(fmt, vargs);
    if(stat(fname, &sb) != -1) {
        rv = sb.st_size;
    }
    else {
        rv = ONLP_STATUS_E_MISSING;
    }
    aim_free(fname);
    return rv;
}

int
onlp_file_size(const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = onlp_file_vsize(fmt, vargs);
    va_end(vargs);
    return rv;
}


int
onlp_file_vread(uint8_t* data, int max, int* len, const char* fmt, va_list vargs)
{
    int fd;
    char* fname = NULL;
    int rv;

    if ((fd = vopen__(&fname, O_RDONLY, fmt, vargs)) < 0) {
        rv = fd;
    }
    else {
        memset(data, 0, max);
        if ((*len = read(fd, data, max)) <= 0) {
            AIM_LOG_ERROR("Failed to read input file '%s'", fname);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        else {
            rv = ONLP_STATUS_OK;
        }
        close(fd);
    }
    aim_free(fname);
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
onlp_file_vread_all(uint8_t** data, const char* fmt, va_list vargs)
{
    int rv;
    uint8_t* contents = NULL;
    char * fname = NULL;
    int fsize, rsize;

    if(data == NULL || fmt == NULL) {
        return ONLP_STATUS_E_PARAM;
    }

    fname = aim_vdfstrdup(fmt, vargs);

    *data = NULL;

    if((fsize = onlp_file_size(fname)) > 0) {
        contents = aim_zmalloc(fsize);
        if((rv = onlp_file_read(contents, fsize,  &rsize, fname)) >= 0) {
            *data = contents;
            rv = rsize;
        }
    }
    else {
        rv = fsize;
    }
    aim_free(fname);
    return rv;
}

int
onlp_file_read_all(uint8_t** data, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = onlp_file_vread_all(data, fmt, vargs);
    va_end(vargs);
    return rv;
}

int
onlp_file_vread_str(char** str, const char* fmt, va_list vargs)
{
    int rv = onlp_file_vread_all((uint8_t**)str, fmt, vargs);
    if(rv > 0) {
        while(rv && ( (*str)[rv-1] == '\n' || (*str)[rv-1] == '\r')) {
            (*str)[rv-1] = 0;
            rv--;
        }
    }
    return rv;

}

int
onlp_file_read_str(char** str, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = onlp_file_vread_str(str, fmt, vargs);
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
    char* fname = NULL;
    int rv;
    int wlen;

    if ((fd = vopen__(&fname, O_WRONLY, fmt, vargs)) < 0) {
        rv = fd;
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
    aim_free(fname);
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
    char* fname;

    rv = vopen__(&fname, flags, fmt, vargs);
    if(rv < 0 && log) {
        AIM_LOG_ERROR("failed to open file %s (0x%x): %{errno}", fname, flags, errno);
    }
    aim_free(fname);
    return rv;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <fts.h>

int
onlp_file_find(char* root, char* fname, char** rpath)
{
    FTS *fs;
    FTSENT *ent;
    char* argv[] = { NULL, NULL };
    argv[0] = root;

    if ((fs = fts_open(argv, FTS_PHYSICAL | FTS_NOCHDIR | FTS_COMFOLLOW,
                       NULL)) == NULL) {
        AIM_LOG_ERROR("fts_open(%s): %{errno}", argv[0], errno);
        return ONLP_STATUS_E_INTERNAL;
    }

    while ((ent = fts_read(fs)) != NULL) {
        switch (ent->fts_info)
            {
            case FTS_F:
                {
                    if(!strcmp(fname, ent->fts_name)) {
                        *rpath = realpath(ent->fts_path, NULL);
                        fts_close(fs);
                        return ONLP_STATUS_OK;
                    }
                }
                break;
            }
    }
    fts_close(fs);
    return ONLP_STATUS_E_MISSING;
}
