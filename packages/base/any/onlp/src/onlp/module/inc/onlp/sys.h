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
 * Platform System Information
 *
 ***********************************************************/

#ifndef __ONLP_SYS_H__
#define __ONLP_SYS_H__

#include <onlp/onlp_config.h>
#include <onlp/onlp.h>
#include <onlplib/onie.h>
#include <onlplib/pi.h>
#include <onlp/oids.h>


typedef struct onlp_sys_info_s {
    /** OID Header */
    onlp_oid_hdr_t hdr;

    /* ONIE System Information */
    onlp_onie_info_t onie_info;

    /* Platform Information */
    onlp_platform_info_t platform_info;

} onlp_sys_info_t;


/**
 * @brief Initialize the System API
 */
int onlp_sys_init(void);

/**
 * @brief Get the system information structure.
 * @param rv [out] Receives the system information.
 */
int onlp_sys_info_get(onlp_sys_info_t* rv);

/**
 * @brief Free a system information structure.
 */
void onlp_sys_info_free(onlp_sys_info_t* info);

/**
 * @brief Get the system header.
 */
int onlp_sys_hdr_get(onlp_oid_hdr_t* hdr);

/**
 * @brief SYS OID debug dump.
 * @param id The SYS OID.
 * @param pvs The output pvs.
 * @param flags The output flags.
 * @note This output is designed for debugging.
 */
void onlp_sys_dump(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags);

/**
 * @brief Show the status of the given OID.
 * @param id the SYS OID.
 * @param pvs The output pvs.
 * @param flags The output flags
 */
void onlp_sys_show(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags);

/**
 * @brief SYS Ioctl
 * @param code The ioctl code.
 * @param ... Arguments
 */
int onlp_sys_ioctl(int code, ...);

/**
 * @brief SYS Ioctl
 * @param code The ioctl code.
 * @param vargs arguments.
 */
int onlp_sys_vioctl(int code, va_list vargs);

/**
 * @brief Start the platform management thread.
 */
int onlp_sys_platform_manage_start(int block);

/**
 * @brief Stop the platform management thread.
 */
int onlp_sys_platform_manage_stop(int block);

/**
 * @brief Join the platform management thread.
 */
int onlp_sys_platform_manage_join(void);

/**
 * @brief Perform any pending platform management activities.
 * @note  A call to this function will perform any pending
 * platform management activities. It is not intended to block
 * for an extended period of time.
 */

void onlp_sys_platform_manage_now(void);

int onlp_sys_debug(aim_pvs_t* pvs, int argc, char** argv);

#endif /* __ONLP_SYS_H_ */
