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
 * SFF Utility for eeprom files.
 *
 ***********************************************************/
#include <sff/sff_config.h>

#if SFF_CONFIG_INCLUDE_SFF_TOOL == 1

#include <sff/sff.h>
#include <unistd.h>
#include <BigList/biglist.h>
#include <limits.h>

int
sff_tool(int argc, char* argv[])
{
    int rv = 0;
    int i;
    int c;

    int s = 0;
    int e = 0;
    int help = 0;
    int n = 0;
    int v = 0;
    int sin = 0;

    biglist_t* fnames=NULL;


    while( (c = getopt(argc, argv, "sehnvi")) != -1) {
        switch(c)
            {
            case 's': s = 1; break;
            case 'e': e = 1; break;
            case 'h': help=1; rv=0; break;
            case 'n': n=1; break;
            case 'v': v=1; break;
            case 'i': sin=1; break;
            default: help=1; rv = 1; break;
            }
    }

    if(help) {
        printf("Usage: %s [OPTIONS] [FILES]\n", argv[0]);
        printf("  -i    Read eeprom data from stdin.\n");
        printf("  -s    Read filenames from stdin. \n");
        printf("  -n    Print the filename if successful.\n");
        printf("  -v    Print the filename always.\n");
        printf("  -e    Show the raw eeprom data.\n");
        printf("  -h    Help.\n");
        return rv;
    }

    if(sin) {
        uint8_t data[512];
        if(fread(data, 256, 1, stdin) == 1) {
            sff_info_t info;
            sff_info_init(&info, data);
            if(info.supported) {
                sff_info_show(&info, &aim_pvs_stdout);
                return 0;
            }
            else {
                aim_printf(&aim_pvs_stderr, "The eeprom data could not be parsed.\n");
            }
        }
        else {
            aim_printf(&aim_pvs_stderr, "Error reading eeprom data from stdin.\n");
        }
        return 1;
    }




    if(s) {
        /* Read filenames from stdin */
        char b[PATH_MAX];
        char* l;
        while((l = fgets(b, PATH_MAX, stdin))) {
            int len=SFF_STRLEN(l);
            if(len) {
                if(l[len-1] == '\n') {
                    l[len-1] = 0;
                }
                fnames = biglist_append(fnames, aim_strdup(l));
            }
        }
    }

    /* Read from command line. This can be used together with -s */
    for(i = optind; i < argc; i++) {
        fnames = biglist_append(fnames, aim_strdup(argv[i]));
    }

    biglist_t* ble;
    char* f;
    BIGLIST_FOREACH_DATA(ble, fnames, char*, f) {
        sff_info_t info;
        memset(&info, 0, sizeof(info));
        if( (rv = sff_info_init_file(&info, f)) >= 0) {
            if(n || v) {
                aim_printf(&aim_pvs_stdout, "%s\n", f);
            }
            if(e) {
                aim_printf(&aim_pvs_stdout, "eeprom:\n%{data}\n", info.eeprom, sizeof(info.eeprom));
            }
            sff_info_show(&info, &aim_pvs_stdout);
        }
        else if(v) {
            aim_printf(&aim_pvs_stdout, "%s: failed.\n", f);
        }
    }
    return 0;
}

#else
int __not_empty__;
#endif



