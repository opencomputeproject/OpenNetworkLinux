/************************************************************
 * <bsn.cl fy=2017 v=onl>
 *
 *        Copyright 2017 Big Switch Networks, Inc.
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
 * This module provides a domain socket registration and handling
 * service.
 *
 * Clients register a filesystem path they wish to publish
 * as a domain socket their handlers are called when
 * the domain socket is accessed.
 *
 * Some ONLP data can only be retreived by accessing another process
 * Which cannot be part of the ONLP layer itself for various reasons.
 *
 * The ONLP File APIs support unix domain sockets for all operations
 * as if they were regular files. This module provides a common
 * framework for clients to implement the server side of the
 * domain socket as well.
 *
 * Some examples of how this is used:
 *   - Reporting the switch internal thermal temperature
 *     - This can only be accessed by the code managing the switch.
 *     - In this case the switch management agent exports a domain socket
 *       that reports the temperature when the socket is read and the
 *       thermali implementation uses that domain socket to satisfy
 *       the request for the OID.
 *
 *   - SFP Access through the switch
 *     - Some platforms implement SFP I2C access through a bus connected
 *       to the switch itself.
 *     - Only the agent running the switch can access the SFP eeproms.
 *     - In this case the strategy is for the switch agent to export domain
 *       sockets for each SFP which can be used to read the SFP status/eeprom
 *       etc. The SFPI interface then reads these domain sockets to get the
 *       required information.
 *
 * Standardizing on this method allows all system ONLP clients to access
 * all data, even if that data is present only in seperate processes.
 *
 *
 ***********************************************************/
#ifndef __ONLPLIB_FILE_UDS_H__
#define __ONLPLIB_FILE_UDS_H__

#include <onlplib/onlplib_config.h>

/**
 * @brief This is the handle for the service object.
 */
typedef struct onlp_file_uds_s onlp_file_uds_t;


/**
 * @brief Create a domain socket service manager.
 * @param fuds Receives the service object pointer.
 */
int onlp_file_uds_create(onlp_file_uds_t** fuds);

/**
 * @brief This is the prototype for your service handler function.
 * @param fd The client file descriptor. This is the descriptor accepted
 * on your behalf by the service manager when someone attempts to open your domain socket.
 * @param cookie Private callback pointer.
 */
typedef int (*onlp_file_uds_handler_t)(int fd, void* cookie);

/**
 * @brief Add a domain socket service path to an existing service manager.
 * @param fuds The service manager
 * @param path The domain socket filesystem path you would like to register.
 * @param handler The connection handler for the domain socket.
 * @param cookie Cookie for you connection handler.
 */
int onlp_file_uds_add(onlp_file_uds_t* fuds,
                      const char* path,
                      onlp_file_uds_handler_t handler, void* cookie);

/**
 * @brief Remove a domain socket service path from an existing service manager.
 * @param fuds The service manager.
 * @param path The domain socket service path to remove.
 */
void onlp_file_uds_remove(onlp_file_uds_t* fuds, const char* path);

/**
 * @brief Destroy a service manager object.
 * @param fuds The object pointer.
 * @notes All registered services will be destroyed.
 */
void onlp_file_uds_destroy(onlp_file_uds_t* fuds);

#endif /* __ONLPLIB_FILE_UDS_H__ */
