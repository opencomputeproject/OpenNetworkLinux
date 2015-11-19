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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <execinfo.h>
#include "faultd_log.h"


typedef struct faultd_service_s {
    /** The filename of the named pipe */
    char* pipename; 

    /**
     * The open pipe descriptor. 
     * 
     * This is opened RDONLY for the server.
     * This is opened WRONLY for the client. 
     */
    int pipefd;     

    /**
     * Server's open write descriptor. 
     *
     * The server-side always opens the named pipe for reading. 
     * There is not necessarily a writer for the pipe at all times, as
     * this depends on whether any clients are currently connected. 
     *
     * The server wants to use select() on the named pipe to wait for
     * any client connections, but this only works properly if
     * there is a writer connected to the pipe from which we are reading. 
     *
     * The server always opens a write connection to the named pipe at
     * startup time to make sure there is always at least one writer connected. 
     * We never write anything to it. 
     *
     * Note -- empirically, it seems possible to open the pipe descriptor in 
     * O_RDWR to accomplish this behavior (instead of opening a separate
     * descriptor), but this is technically undefined behavior. 
     */
    int writefd;         

} faultd_service_t; 



/**
 * faultd Server Object
 */
struct faultd_server_s { 
    /** All services */
    faultd_service_t services[FAULTD_CONFIG_SERVICE_PIPES_MAX]; 
    /** The last service from which we read a message */
    int sid_last;
}; /* faultd_server_t */


int 
faultd_server_create(faultd_server_t** rfso)
{
    faultd_server_t* fso; 

    if(rfso == NULL) {
        return -1; 
    }

    fso = aim_zmalloc(sizeof(*fso)); 

    *rfso = fso; 
    return 0;
}

void
faultd_server_destroy(faultd_server_t* fso)
{
    int i; 
    if(fso) { 
        for(i = 0; i < AIM_ARRAYSIZE(fso->services); i++) { 
            faultd_server_remove(fso, NULL, i); 
        }
        AIM_FREE(fso); 
    }
}

static void
faultd_service_destroy__(faultd_service_t* sp)
{
    if(sp) { 
        if(sp->pipename) { 
            AIM_FREE(sp->pipename);
        }
        if(sp->pipefd) { 
            close(sp->pipefd); 
        }
        if(sp->writefd) { 
            close(sp->writefd); 
        }
        AIM_MEMSET(sp, 0, sizeof(*sp)); 
    }
}

int 
faultd_server_add(faultd_server_t* fso, char* pipename)
{
    int i; 
    if(fso == NULL) { 
        return -1; 
    }
    if(pipename == NULL) { 
        pipename = FAULTD_CONFIG_PIPE_NAME_DEFAULT; 
    }

    /* Find a free slot */
    for(i = 0; i < AIM_ARRAYSIZE(fso->services); i++) { 
        if(fso->services[i].pipename == NULL) { 
            int rv; 
            faultd_service_t* sp = fso->services+i; 

            sp->pipename = aim_strdup(pipename); 

            /**
             * Create the fifo if it doesn't already exist. 
             */
            if( mkfifo(sp->pipename, 0644) < 0) { 
                if(errno != EEXIST) { 
                    goto server_add_failed; 
                }
            }

            /** 
             * Open the fifo. 
             */
            rv = open(sp->pipename, O_RDONLY | O_NONBLOCK); 
            if(rv < 0) { 
                AIM_LOG_ERROR("open(pipe): %s", strerror(errno)); 
                goto server_add_failed;
            }
            sp->pipefd = rv; 

            /** 
             * Open our write connection. 
             */ 
            rv = open(sp->pipename, O_WRONLY | O_NONBLOCK); 
            if(rv < 0) { 
                AIM_LOG_ERROR("open(writefd): %s", strerror(errno)); 
                goto server_add_failed;
            }
            sp->writefd = rv; 
    
            /**
             * We opened the pipe originally in non-blocking mode. 
             * Otherwise, the open() would have blocked waiting for
             * a client a client to open() it for writing. 
             *
             * We now want all reads on the pipe to be blocking so 
             * we can use select() to pend on clients. 
             *
             * Reset the pipe to blocking here:
             */
            if( (rv = fcntl(sp->pipefd, F_GETFL, 0)) < 0) { 
                goto server_add_failed; 
            }
            rv &= ~O_NONBLOCK; 
            if(fcntl(sp->pipefd, F_SETFL, rv) < 0) { 
                goto server_add_failed;
            }

            /* Good to go. 'i' is the service id.  */
            return i;
        }
    }
    /* All services full */
    return -1;

 server_add_failed:
    faultd_server_remove(fso, NULL, i); 
    return -1; 
}

int 
faultd_server_remove(faultd_server_t* fso, char* pipename, 
                     faultd_sid_t sid)
{
    if(fso == NULL) {
        return -1; 
    }
    else if(sid < 0 || sid >= AIM_ARRAYSIZE(fso->services)) {
        return -1; 
    }
    else {
        faultd_service_destroy__(fso->services + sid); 
        return 0; 
    }
}

int 
faultd_server_process(faultd_server_t* fdo, faultd_sid_t sid,
                      int count, aim_pvs_t* pvs, int decode)
{
    int c; 
    faultd_info_t fault_info; 
    
    for(c = 0; c < count || count == -1; c++) {         
        FAULTD_MEMSET(&fault_info, 0, sizeof(fault_info)); 
        faultd_server_read(fdo, &fault_info, sid); 
        faultd_info_show(&fault_info, pvs, decode);
        if(fault_info.backtrace_symbols) { 
            AIM_FREE(fault_info.backtrace_symbols); 
        }
    }           
    return 0; 
}

struct faultd_client_s { 
    faultd_service_t s;
}; /* faultd_client_t */

int
faultd_client_create(faultd_client_t** rfco, const char* pipename)
{
    int rv; 
    faultd_client_t* fco; 

    if(rfco == NULL) {    
        return -1; 
    }
    
    if(pipename == NULL) { 
        pipename = FAULTD_CONFIG_PIPE_NAME_DEFAULT; 
    }

    fco = aim_zmalloc(sizeof(*fco)); 
    fco->s.pipename = aim_strdup(pipename); 
    
    /** 
     * Open the fifo. 
     */
    rv = open(fco->s.pipename, O_WRONLY | O_NONBLOCK); 
    if(rv < 0) { 
        goto client_create_failed; 
    }
    fco->s.pipefd = rv; 

    *rfco = fco; 
    return 0; 
    
 client_create_failed:
    faultd_client_destroy(fco); 
    return rv; 
}

void 
faultd_client_destroy(faultd_client_t* fco)
{
    if(fco) { 
        faultd_service_destroy__(&fco->s); 
        AIM_FREE(fco);
    }
}

static int
read_until__(int fd, char terminator, char* dst, int size)
{
    int rv; 
    int count = 0;
    char c; 

    do { 
        rv = read(fd, &c, 1); 
        if(rv < 0) { 
            if(errno == EINTR) { 
                continue; 
            }
            else {
                return rv; 
            }
        }
        if(count < size) { 
            dst[count] = c; 
        }
        if(c == terminator) { 
            return count; 
        }
        count++; 
    
        /* Fixme - need a timeout */
    } while(1);
}



static int
read_size__(int fd, char* dst, int size)
{
    int rv; 
    int remaining = size;       
    char* p = dst; 

    do { 
        rv = read(fd, p, remaining); 
        if(rv < 0) { 
            if(errno == EINTR) { 
                /* Keep trying */
                continue; 
            }
            else {
                /* Read failed. Probably not good. */
                /* Do something here */
                AIM_LOG_MSG("read failed errno=%d %s\n", errno, strerror(errno)); 
                return -1; 
            }
        }
        if(rv == 0 && remaining) {  
            /* What to do here? */
            AIM_LOG_MSG("read() returned zero but we didn't get everything."); 
            return -1; 
        }
        p += rv; 
        remaining -= rv; 
    } while(remaining); 

    return size; 
}


static int
write_size__(int fd, char* src, int size)
{
    int rv; 
    int remaining = size; 
    char* p = src; 

    do { 
        rv = write(fd, p, remaining); 
        if(rv < 0) { 
            if(errno == EINTR) { 
                /* Keep trying */
                continue; 
            }
            else {
                /*
                 * Write failed. Probably not good. 
                 * The faultd server may have died, or doesn't
                 * exist, or something else extra bad in our process. 
                 * Given why we're here, maybe not too surpising. 
                 *
                 * Theres really nothing else to do. 
                 */
                return -1; 
            }
        }
        p += rv; 
        remaining -= rv; 
    } while(remaining); 

    return size; 
}

int 
faultd_wait_services__(faultd_server_t* fso, int sid, fd_set* rfds)
{
    int rv; 
    int maxfd; 

    FD_ZERO(rfds); 

    if(sid < 0 && sid >= AIM_ARRAYSIZE(fso->services) && sid != -1) { 
        /* invalid sid */
        return -1; 
    }
    
    if(sid == -1) { 
        /* All services */
        int i; 
        for(i = 0, maxfd = 0; i < AIM_ARRAYSIZE(fso->services); i++) { 
            if(fso->services[i].pipefd) { 
                FD_SET(fso->services[i].pipefd, rfds); 
                if(fso->services[i].pipefd > maxfd) { 
                    maxfd = fso->services[i].pipefd; 
                }
            }
        }
    }
    else { 
        /* Single service */
        if(fso->services[sid].pipefd == 0) { 
            /* Invalid sid */
            return -1;
        }
        else {
            FD_SET(fso->services[sid].pipefd, rfds); 
            maxfd = fso->services[sid].pipefd; 
        }
    }

    /* Wait on configured services */
    do { 
        rv = select(maxfd+1, rfds, NULL, NULL, NULL); 
    } while(rv == -1 && errno == EINTR); 

    return rv; 
}


int 
faultd_server_read(faultd_server_t* fso, faultd_info_t* info, int sid)
{
    int i;      
    int rv; 
    fd_set rfds;
    int count; 

    rv = faultd_wait_services__(fso, sid, &rfds); 
    
    if(rv < 0) { 
        /* Error on select or sid */
        return rv; 
    }

    /** 
     * Read message on a ready descriptor.
     *
     * If we're polling all services, we start looking for the 
     * next sid after the last sid we've received a message on. 
     * This avoids starvation if multiple services are producing
     * messages. This is unlikely to be a problem under normal
     * circumstances and use cases, but can be avoided easily nonetheless. 
     */
    for(i = fso->sid_last+1, count = 0; 
        count < AIM_ARRAYSIZE(fso->services); 
        i++, count++) { 
        int s = i % AIM_ARRAYSIZE(fso->services); 
        if(FD_ISSET(fso->services[s].pipefd, &rfds)) { 
            rv = read_size__(fso->services[s].pipefd, (char*)info, sizeof(*info)); 
    
            if(rv < 0) { 
                /* Do something here, like restare the pipe */
                AIM_LOG_ERROR("truncated read on pipe."); 
                continue; 
            }
            
            /**
             * Backtrace symbols information available? 
             */
            if(info->backtrace_symbols) { 
                /*
                 * The backtrace symbol information is of variable length. 
                 */
                info->backtrace_symbols = aim_zmalloc(FAULTD_CONFIG_BACKTRACE_SYMBOLS_SIZE); 
                /* Backtrace symbols are terminated with a null character. */
                read_until__(fso->services[s].pipefd, 0, info->backtrace_symbols, 
                             FAULTD_CONFIG_BACKTRACE_SYMBOLS_SIZE); 
            }

            info->pipename = fso->services[s].pipename; 
            fso->sid_last = s; 
            return s; 
        }
    }
    /* Error while reading on the only ready descriptor */
    return -1; 
}
        
 
int
faultd_client_write(faultd_client_t* fco, faultd_info_t* info)
{
    int rv = write_size__(fco->s.pipefd, (char*)info, sizeof(*info)); 
    
    if(rv < 0) { 
        return rv; 
    }

    if(info->backtrace_symbols) { 
        char c = 0; 
        backtrace_symbols_fd(info->backtrace, info->backtrace_size, 
                             fco->s.pipefd); 
        /* Terminate backtrace symbols with a null character. */
        write_size__(fco->s.pipefd, &c, 1); 
    }
    return 0; 
}

int
faultd_info_show(faultd_info_t* info, aim_pvs_t* pvs, int decode)
{
    int i = 0;
    aim_printf(pvs, "service = %s\n", info->pipename); 
    aim_printf(pvs, "binary = %s\n", info->binary); 
    aim_printf(pvs, "pid = %d\n", info->pid); 
    aim_printf(pvs, "tid = %d\n", info->tid); 
    aim_printf(pvs, "signal = %d (%s)\n", info->signal, strsignal(info->signal)); 
    aim_printf(pvs, "code = %d\n", info->signal_code); 
    aim_printf(pvs, "fa = %p\n", info->fault_address); 
    aim_printf(pvs, "errno = %d\n", info->last_errno); 
    aim_printf(pvs, "backtrace_size=%d\n", info->backtrace_size); 
    for(i = 0; i < info->backtrace_size; i++) { 
        aim_printf(pvs, "    %p\n", info->backtrace[i]); 
    }
    aim_printf(pvs, "backtrace_symbols: %p\n", info->backtrace_symbols); 
    if(info->backtrace_symbols) {
        aim_printf(pvs, "%s\n", info->backtrace_symbols); 
    }
    return 0; 
}       
        



        
    
    
    
    
