/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/utsname.h>
#include <linux/version.h>
#include <AIM/aim.h>
#include <onlplib/file.h>
#include <sys/mman.h>
#include "platform_lib.h"

int
onlp_get_kernel_ver()
{
    struct utsname buff;
    char ver[4];
    char *p;
    int i = 0;

    if (uname(&buff) != 0)
        return ONLP_STATUS_E_INTERNAL;

    p = buff.release;

    while (*p) {
        if (isdigit(*p)) {
            ver[i] = strtol(p, &p, 10);
            i++;
            if (i >= 3)
                break;
        } else {
            p++;
        }
    }

    return KERNEL_VERSION(ver[0], ver[1], ver[2]);
}
