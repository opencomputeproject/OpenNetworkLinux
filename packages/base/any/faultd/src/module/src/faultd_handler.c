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

#define __USE_GNU /* Needed to get REG_EIP from ucontext.h */

#include <faultd/faultd.h>
#include <AIM/aim.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <errno.h>
#include <ucontext.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#define _XOPEN_SOURCE 600
#include <sys/select.h>


static pthread_spinlock_t thread_lock__;
static faultd_client_t* faultd_client__ = NULL; 
static faultd_info_t faultd_info__;
static int localfd__ = -1; 

inline int signal_backtrace__(void** buffer, int size, ucontext_t* context,
                              int distance)
{
#define IP_STACK_FRAME_NUMBER 3
    
    int rv; 
    rv = backtrace(buffer, size);
    
    if(context) { 
        distance += IP_STACK_FRAME_NUMBER; 
        
#ifdef __i386__
        buffer[distance] = (void*)(context->uc_mcontext.gregs[REG_EIP]); 
#endif
#ifdef __PPC__
        buffer[distance] = (void*)(context->uc_mcontext.regs->nip); 
#endif
        /* Note -- 64bit does not require require modifications */
    }   
    return rv; 
}    
  #include <AIM/aim_pvs.h>      
static void 
faultd_signal_handler__(int signal, siginfo_t* siginfo, void* context)
{
    int rv; 

    /*
     * Make sure we syncronize properly with other threads that
     * may *also* be faulting
     */
    rv = pthread_spin_trylock(&thread_lock__); 

    if (rv == EBUSY) { 
        sigset_t mask; 
        sigemptyset(&mask); 
        pselect(0, NULL, NULL, NULL, NULL, &mask); 
    }
    
    /*
     * Generate our fault information. 
     */ 
    faultd_info__.pid = getpid(); 
    faultd_info__.tid = 0; 
    faultd_info__.signal = signal; 
    faultd_info__.signal_code = siginfo->si_code; 
    faultd_info__.fault_address = siginfo->si_addr; 
    faultd_info__.last_errno = errno; 

    faultd_info__.backtrace_size = signal_backtrace__(faultd_info__.backtrace, 
                                                      AIM_ARRAYSIZE(faultd_info__.backtrace),
                                                      context, 0); 
    faultd_info__.backtrace_symbols = (void*)1; 
    if(faultd_client__) { 
        faultd_client_write(faultd_client__, &faultd_info__); 
    }
    if(localfd__ >= 0) { 
        char* signame = strsignal(faultd_info__.signal); 
        char* nl = "\n"; 
        write(localfd__, signame, strlen(signame)+1); 
        write(localfd__, nl, 2); 
        backtrace_symbols_fd(faultd_info__.backtrace, 
                             faultd_info__.backtrace_size, 
                             localfd__); 
    }

    /* 
     * Unlock spinlock, in case this signal wasn't fatal
     */
    pthread_spin_unlock(&thread_lock__); 
}


int
faultd_handler_register(int localfd, 
                        const char* pipename, 
                        const char* binaryname)
{
    int rv; 
    struct sigaction saction; 
    void* dummy_backtrace[1]; 
    int dummy_backtrace_size; 
    int fd; 

    if ( (rv = pthread_spin_init(&thread_lock__, 0)) ) { 
        return rv; 
    }

    /* 
     * These calls to backtrace are to assure that 
     * backtrace() and backtrace_symbols_fd() have actually 
     * been loaded into our process -- its possible they 
     * come from a dynamic library, and we don't want them
     * to get loaded at fault-time.
     */
    dummy_backtrace_size = backtrace(dummy_backtrace, 1); 
    
    /** Note - we could just pass an invalid descriptor here, but it 
     * it flags errors in valgrind. 
     */
    fd = open("/dev/null", O_WRONLY); 
    backtrace_symbols_fd(dummy_backtrace, dummy_backtrace_size, fd); 
    close(fd); 

    AIM_MEMSET(&faultd_info__, 0, sizeof(faultd_info__)); 
    if(!binaryname) { 
        binaryname = "Not specified."; 
    }
    aim_strlcpy(faultd_info__.binary, binaryname, sizeof(faultd_info__.binary)); 
                

    if(pipename) { 
        faultd_client_create(&faultd_client__, pipename); 
    }

    AIM_MEMSET(&saction, 0, sizeof(saction)); 
    saction.sa_sigaction = faultd_signal_handler__; 

    sigfillset(&saction.sa_mask); 
    saction.sa_flags = SA_SIGINFO | SA_RESETHAND; 
    
    rv = sigaction (SIGSEGV, &saction, NULL); 
    rv |= sigaction (SIGILL, &saction, NULL);
    rv |= sigaction (SIGFPE, &saction, NULL);  
    rv |= sigaction (SIGBUS, &saction, NULL);
    rv |= sigaction (SIGQUIT, &saction, NULL);
    rv |= sigaction (SIGALRM, &saction, NULL);

    /*
     * SIGUSR2 can be used to request a backtrace explicitly. 
     * In this case, we don't want to reset the handler. 
     */
    saction.sa_flags = SA_SIGINFO; 
    rv |= sigaction (SIGUSR2, &saction, NULL);  

    /*
     * The local fault handler will attempt to write a subset of
     * the fault information (signal type and backtrace) 
     * to the localfd descriptor if specified. 
     */
    localfd__ = localfd; 

    return rv;
}






