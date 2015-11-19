/**************************************************************************//**
 * 
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
 *
 *****************************************************************************/
#ifndef __FAULTD_H__
#define __FAULTD_H__

#include <faultd/faultd_config.h>
#include <AIM/aim_pvs.h>

/**
 * This structure contains the full fault information. 
 * 
 * This structure will be filled out by the fault handler
 * 
 */
typedef struct faultd_info_s { 
    /** Service pipe name - Server side only */ 
    char* pipename; 

    /** The full name and path of the process binary */
    char binary[FAULTD_CONFIG_BINARY_SIZE]; 

    /** The pid of the process */
    pid_t pid; 
    /** The kernel thread id of the process */
    pid_t tid; 
    
    /** The signal number that caused the fault. */
    int  signal;
    /** The signal code for the fault. */
    int  signal_code; 
    /** Offending address, if applicable. */
    void* fault_address; 

    /** The last value of errno at the time of the fault */
    int last_errno; 

    /** The size of this backtrace */ 
    int backtrace_size; 
    /** The backtrace */
    void* backtrace[FAULTD_CONFIG_BACKTRACE_SIZE_MAX]; 

    /** 
     * This will store the output from backtrace_symbols_fd(). 
     *
     * When writing the message to the pipe, set it to non-zero. 
     * The reader will then read additional bytes until the null 
     * terminating byte is found. 
     *
     * The pointer will then be replaced on the receiving side. 
     *
     */
    char* backtrace_symbols;

} faultd_info_t; 
    


/**************************************************************************//**
 *
 * FaultD Servers
 *
 *
 *****************************************************************************/
typedef struct faultd_server_s faultd_server_t; 

typedef int faultd_sid_t; 

/**
 * @brief Create a faultd server object. 
 * @param rfso Receives the faultd_server object. 
 */
int faultd_server_create(faultd_server_t** rfso); 

/**
 * @brief Destroy a server object. 
 * @param fco The faultd server object. 
 */
void faultd_server_destroy(faultd_server_t* fco); 

/**
 * @brief Add a named pipe service to the server. 
 * @param fso The faultd server object. 
 * @param pipename The name of the pipe. 
 * @returns The service id. 
 * @note FAULTD_CONFIG_PIPE_NAME_DEFAULT will be used if pipename is NULL. 
 */
faultd_sid_t faultd_server_add(faultd_server_t* fso, char* pipename); 

/**
 * @brief Remove a named pipe service. 
 * @param fso The faultd server object. 
 * @param pipename The name of the pipe. 
 * @param sid The service id. 
 *
 * @note You can remove either by pipename or servicename. 
 */
int faultd_server_remove(faultd_server_t* fso, char* pipename, 
                         faultd_sid_t sid); 

/**
 * @brief Read a fault message from any service pipe. 
 * @param fso The faultd server object. 
 * @param info The fault information. 
  * @note if sid == -1, all services will be polled. 
 * @note else the given service will be polled. 
 * @returns The sid from which the message was received. 
 */
int faultd_server_read(faultd_server_t* fso, faultd_info_t* info, 
                       faultd_sid_t sid); 


/**
 * @brief Read and report all messages on all service pipes. 
 * @param fso The fault descriptor. 
 * @param sid The fault service. 
 * @param count Process at most 'count' messages. 
 * @param pvs The output pvs passed to fauld_info_show()
 * @param decode Passed to faultd_info_show()
 * @note if count is -1, process forever. 
 */
int faultd_server_process(faultd_server_t* fso, faultd_sid_t sid, 
                          int count, aim_pvs_t* pvs, int decode);


/**************************************************************************//**
 *
 * faultd Clients
 *
 *
 *****************************************************************************/
typedef struct faultd_client_s faultd_client_t; 

/**
 * @brief Create a faultd client object. 
 * @param fco Receives the faultd client object. 
 * @param pipename The named pipe filename. 
 * @note FAULTD_CONFIG_PIPE_NAME_DEFAULT will be used if pipename is NULL. 
 */
int faultd_client_create(faultd_client_t** fco, const char* pipename); 

/**
 * @brief Send a fault message to the server. 
 * @param fco The faultd client object. 
 * @param info The fault information. 
 * @note If backtrace_symbols is not NULL, the
 * backtrace_symbols_fd() will be called on the backtrace 
 * and included in the report. 
 */
int faultd_client_write(faultd_client_t* fco, faultd_info_t* info); 

/**
 * @brief Destroy a client object. 
 * @param fco The faultd client object. 
 */
void faultd_client_destroy(faultd_client_t* fco); 


/**************************************************************************//**
 *
 * faultd Hander
 *
 *
 *****************************************************************************/
int faultd_handler_register(int localfd, 
                            const char* pipename, 
                            const char* binaryname); 


/**************************************************************************//**
 *
 * Common 
 *
 *
 *****************************************************************************/

/**
 * @brief Output the fault message information to the given PVS. 
 * @param info The fault message. 
 * @param pvs The output pvs. 
 * @param decode If set, the backtrace will be processing through addr2line
 */
int faultd_info_show(faultd_info_t* info, aim_pvs_t* pvs, int decode); 


#endif /* __FAULTD_H__ */
