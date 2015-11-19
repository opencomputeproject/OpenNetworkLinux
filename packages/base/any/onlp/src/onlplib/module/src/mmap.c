/************************************************************
 * <bsn.cl v=2014 v=onl>
 * 
 *           Copyright 2015 Big Switch Networks, Inc.          
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
#include <onlplib/mmap.h>
#include "onlplib_log.h"
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

void*
onlp_mmap(off_t pa, uint32_t size, const char* name)
{
    int msize = 0;
    int psize = getpagesize();

    msize = (((size / psize) + 1) * psize);

    int fd = open("/dev/mem", O_RDWR | O_SYNC);

    if(fd <= 0) {
        AIM_LOG_ERROR("open(/dev/mem) failed: %{errno}", errno);
        return NULL;
    }

    uint8_t* memory = mmap(NULL,
                           msize,
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED,
                           fd,
                           pa);

    close(fd);

    if(memory == MAP_FAILED) {
        AIM_LOG_ERROR("mmap() pa=0x%llx size=%d name=%s failed: %{errno}",
                      pa, size, errno);
        return NULL;
    }
    return memory;
}

