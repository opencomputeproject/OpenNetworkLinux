/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include "platform_lib.h"
#include <sys/types.h>
#include <dirent.h>


int platform_hwmon_diag_enable_read(int *enable)
{
    int rv = ONLP_STATUS_OK;
    char* info_path=hwmon_path(INV_HWMON_BASE);
    rv = onlp_file_read_int((int*)enable, "%sdiag", info_path );
    return rv;
}


int platform_hwmon_diag_enable_write(int enable)
{
    int rv = ONLP_STATUS_OK;
    char* info_path=hwmon_path(INV_HWMON_BASE);
    rv = onlp_file_write_int(enable, "%sdiag", info_path);
    return rv;
}

char* hwmon_path( char* parent_dir)
{
    DIR * dir;
    struct dirent * ptr;
    dir = opendir(parent_dir);
    char* buf=NULL;
    int fail=1;

    while( (ptr = readdir(dir))!=NULL ) {
        buf=ptr->d_name;
        if( strncmp(buf,"hwmon",5)==0 ) {
            fail=0;
            break;
        }
    }
    closedir(dir);
    if(fail==1) {
        printf("[ERROR] Can't find valid path\n");
        return NULL;
    }

    static char result[ONLP_CONFIG_INFO_STR_MAX];
    snprintf(result, ONLP_CONFIG_INFO_STR_MAX, "%s%s/", parent_dir , buf);

    return result;
}