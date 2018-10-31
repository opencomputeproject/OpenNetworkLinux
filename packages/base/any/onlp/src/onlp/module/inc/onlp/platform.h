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
 ********************************************************//**
 *
 * @file
 * @brief Platform Management and Initialization
 * @addtogroup platform
 * @{
 *
 ***********************************************************/
#ifndef __ONLP_PLATFORM_H__
#define __ONLP_PLATFORM_H__

#include <onlp/onlp_config.h>
#include <onlp/onlp.h>

#include <AIM/aim_pvs.h>

/**
 * @brief Get the current ONL platform name.
 */
char* onlp_platform_name_get(void);

/**
 * @brief Platform software init.
 */
int onlp_platform_sw_init(const char* platform);

/**
 * @brief Platform Hardware init.
 * @param flags The init flags.
 */
int onlp_platform_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the chassis software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_platform_sw_denit(void);

/**
 * @brief Start the platform management thread.
 * @param block Whether the call should block on completion.
 */
int onlp_platform_manager_start(int block);

/**
 * @brief Stop the platform management thread.
 * @param block Block on termination.
 */
int onlp_platform_manager_stop(int block);

/**
 * @brief Join the platform management thread.
 */
int onlp_platform_manager_join(void);

/**
 * @brief Perform any pending platform management activities.
 * @note  A call to this function will perform any pending
 * platform management activities. It is not intended to block
 * for an extended period of time.
 */
void onlp_platform_manager_manage(void);

/**
 * @brief Run in platform manager dameon mode.
 */
void
onlp_platform_manager_daemon(const char* name, const char* logfile,
                             const char* pidfile, char** argv);
/**
 * @brief Call the platform debug hook.
 */
int onlp_platform_debug(aim_pvs_t* pvs, int argc, char** argv);

#endif /* __ONLP_PLATFORM_H__ */
/* @} */
