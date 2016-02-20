/************************************************************
 * <bsn.cl fy=2016 v=onl>
 *
 *        Copyright 2016 Big Switch Networks, Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <onlp/onlp.h>
#include <onlp/sfp.h>
#include <sff/sff.h>
#include <oom-shim/oom-shim.h>
#include <oom-shim/oom_south.h>

int Initialized = 0;

/*Initializing the SFP and ONLP modules*/
void __oom_shim_module_init__(void) {
    onlp_init();
    Initialized = 1;
}

/*Gets the portlist of the SFP ports on the switch*/
int oom_get_portlist(oom_port_t portlist[], int listsize){
    
    int port,i=0;
    oom_port_t* pptr;
    
    if(Initialized == 0){
         __oom_shim_module_init__();
    }

    onlp_sfp_bitmap_t bitmap;
    onlp_sfp_bitmap_t_init(&bitmap);
    onlp_sfp_bitmap_get(&bitmap);
   
    if ((portlist == NULL) && (listsize == 0)){ /* asking # of ports */
        if(AIM_BITMAP_COUNT(&bitmap) == 0){
            printf("No SFPs on this platform.\n");
            return 0;
        }
        else
            return AIM_BITMAP_COUNT(&bitmap);
    }

    AIM_BITMAP_ITER(&bitmap, port){
        int rv;
        uint8_t* data;
        
        pptr = &portlist[i];
        pptr->handle = (void *)(uintptr_t)port;
        pptr->oom_class = OOM_PORT_CLASS_SFF;
        sprintf(pptr->name, "port%d", port);
        i++;
        
        rv = onlp_sfp_is_present(port);
        if(rv == 0){
            printf("module %d is not present\n", port);
            continue;
        }

        if(rv < 0){
            printf("%4d  Error\n", port);
            continue;
        }

        rv = onlp_sfp_eeprom_read(port, &data);

        if(rv < 0){
            printf("%4d  Error}\n", port);
            continue;
        }
     
        sff_info_t sff;
        sff_info_init(&sff, data);
        if(!sff.supported) {
            printf("%13d  UNK\n", port);
            continue;
        }
        printf("The sfp_type is : %-14s",sff.sfp_type_name);
    }
    return 0;
}
