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
 * Common SFP Utilities for Platform implementations.
 *
 ************************************************************/

#include <onlplib/sfp.h>
#include "onlplib_int.h"
#include "onlplib_log.h"
#include <onlp/onlp.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>


int
onlplib_sfp_is_present_file(const char* fname, const char* present,
                            const char* notpresent)
{
    char buf[64] = {0};

    int fd = open(fname, O_RDONLY);
    if (fd < 0) {
        return ONLP_STATUS_E_MISSING;
    }

    ssize_t nrd = read(fd, buf, sizeof(buf)-1);
    close(fd);

    if (nrd <= 0) {
        AIM_LOG_INTERNAL("Failed to read SFP presence file '%s'", fname);
        return ONLP_STATUS_E_INTERNAL;
    }

    if(!strcmp(buf, present)) {
        return 1;
    }

    if(!strcmp(buf, notpresent)) {
        return 0;
    }

    return ONLP_STATUS_E_INTERNAL;
}

int
onlplib_sfp_eeprom_read_file(const char* fname, uint8_t data[256])
{
    int fd = open(fname, O_RDONLY);
    if (fd < 0) {
        return ONLP_STATUS_E_MISSING;
    }

    int nrd = read(fd, data, 256);
    close(fd);

    if (nrd != 256) {
        AIM_LOG_INTERNAL("Failed to read EEPROM file '%s'", fname);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlplib_sfp_reset_file(const char* fname,
                       const char* first, int delay_ms, const char* second)
{
    int fd;
    ssize_t nwr;

    /* Open the file  */
    fd = open(fname, O_WRONLY);
    if (fd < 0) {
        return ONLP_STATUS_E_MISSING;
    }

    /* Write the 'first' string to the file. */
    nwr = write(fd, first, strlen(first));
    close(fd);

    if (nwr != strlen(first)) {
        AIM_LOG_INTERNAL("Failed to write to SFP reset file '%s'", fname);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* If there is a second write provided, wait delay and the write it. */
    if(second) {
        struct timespec delay;

        delay.tv_sec = delay_ms / 1000;
        delay.tv_nsec = 1000000L * (delay_ms % 1000);
        nanosleep(&delay, NULL);

        fd = open(fname, O_WRONLY);
        if (fd < 0) {
            AIM_LOG_INTERNAL("Failed to open SFP reset file '%s'", fname);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    nwr = write(fd, second, strlen(second));
    close(fd);

    if (nwr != strlen(second)) {
        AIM_LOG_INTERNAL("Failed to write to SFP reset file '%s'", fname);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
