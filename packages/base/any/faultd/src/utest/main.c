/**************************************************************************//**
 * <bsn.cl fy=2013 v=onl>
 * 
 *        Copyright 2013, 2014 BigSwitch Networks, Inc.        
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
 *****************************************************************************/
#include <faultd/faultd_config.h>
#include <faultd/faultd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AIM/aim.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <execinfo.h>

#define LOCALNAME "faultd.pipe"

int
client_main(int argc, char* argv[])
{
    /* Report a dummy message */        
    faultd_info_t info; 
    faultd_client_t* clientfd; 
    int count = 1; 
    int i; 
    void* backtrace_symbols = NULL; 
    char* service = LOCALNAME; 

    if(argc >= 1) { 
        count = atoi(argv[0]); 
    }
    if(argc >= 2) { 
        intptr_t bts = atoi(argv[1]); 
        backtrace_symbols = (void*)bts; 
    }
    if(argc >= 3) { 
        service = argv[2]; 
    }
    for(i = 0; i < count || count == -1; i++) { 
        if(faultd_client_create(&clientfd, service) < 0) { 
            perror("client_create: "); 
            abort(); 
        }
        strcpy(info.binary, "client_main"); 
        info.pid = getpid(); 
        info.tid = 100;
        info.signal = SIGSEGV; 
        info.signal_code = i;
        info.fault_address = (void*)(0xDEAD); 
        info.last_errno = -42; 
        info.backtrace_size = backtrace(info.backtrace, AIM_ARRAYSIZE(info.backtrace)); 
        info.backtrace_symbols = backtrace_symbols;
        printf("writing msg...\n"); 
        faultd_client_write(clientfd, &info); 
        faultd_client_destroy(clientfd);
    }
    return 0;

}

int
server_main(int argc, char* argv[])
{
    int count = -1; 
    int decode = 0; 
    faultd_server_t* fso; 

    if(argc >= 1) { 
        count = atoi(argv[0]); 
    }
    if(argc >= 2) { 
        decode = atoi(argv[1]); 
    }
            
    if(faultd_server_create(&fso)) { 
        abort(); 
    }
    
    if(argc >= 3) { 
        int i; 
        for(i = 2; i < argc; i++) { 
            if(faultd_server_add(fso, argv[i]) < 0) { 
                abort(); 
            }
        }
    }
    else {
        if(faultd_server_add(fso, LOCALNAME) < 0) { 
            abort(); 
        }
    }
    faultd_server_process(fso, -1, count, &aim_pvs_stdout, decode); 
    faultd_server_destroy(fso); 
    return 0; 
}

int
utest_main(int argc, char* argv[])
{
    fprintf(stderr, "No standalone unit test implemented.\n"); 
    return 0; 
}

int
local_main(int argc, char* argv[])
{
    int i = 0; 
    /** Just test local descriptor fault processing */
    if(faultd_handler_register(0, LOCALNAME, "utest") < 0) { 
        fprintf(stderr, "faultd_handler_register() failed."); 
        abort(); 
    }
    for(;;) { 
        sleep(1); 
        printf("count = %d\n", i++); 
        if(i % 5 == 0) {
            * ((int*)(0)) = 0; 
        }
    }
    return 0; 
}

int 
aim_main(int argc, char* argv[])
{
    if(argc == 1) { 
        /* Run in unit test mode */
        return utest_main(argc, argv); 
    }
    else { 
        argc--; 
        argv++; 
        if(!strcmp(argv[0], "-c")) { 
            return client_main(--argc, ++argv); 
        }
        if(!strcmp(argv[0], "-s")) { 
            return server_main(--argc, ++argv); 
        }
        if(!strcmp(argv[0], "-l")) { 
            return local_main(--argc, ++argv); 
        }
        if(!strcmp(argv[0], "-u")) { 
            return utest_main(--argc, ++argv); 
        }
        fprintf(stderr, "Unknown option '%s'\n", argv[0]); 
        return 1; 
    }
}

