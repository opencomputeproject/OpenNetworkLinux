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
 *
 ***********************************************************/
#include <onlp/platformi/sysi.h>
#include "onlp_platform_defaults_int.h"
#include "onlp_platform_defaults_log.h"

const char* __ONLP_DEFAULTI
onlp_sysi_platform_get(void)
{
    /*
     * This function should never be called.
     *
     * If we get here its because the platform libraries are
     * not written, initialized, or setup properly before we execute.
     */
    AIM_LOG_ERROR("The default implementation of onlp_sysi_platform_get() has been called.");
    AIM_LOG_ERROR("This can happen for the following reasons, all fatal:");
    AIM_LOG_ERROR("* The ONLP build configuration is incorrect.");
    AIM_LOG_ERROR("* The ONLP platform library for this platform does not contain the onlp_sysi_platform_get() symbol.");
    AIM_LOG_ERROR("* The ONLP platform shared libraries are not setup properly before we executed.");

#if ONLP_CONFIG_INCLUDE_PLATFORM_ERROR_CHECK == 1
    AIM_LOG_ERROR("* The platform cannot continue until this issue is resolved.");
    abort();
#endif

    return ONLP_SYSI_PLATFORM_NAME_DEFAULT;
}

int __ONLP_DEFAULTI
onlp_sysi_debug(aim_pvs_t* pvs, int argc, char* argv[])
{
    aim_printf(pvs, "This platform does not support debug features.\n");
    return -1;
}

__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sysi_platform_set(const char* p));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sysi_init(void));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sysi_onie_data_phys_addr_get(void** physaddr));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sysi_onie_data_get(uint8_t** data, int* size));
__ONLP_DEFAULTI_VIMPLEMENTATION(onlp_sysi_onie_data_free(uint8_t* data));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sysi_onie_info_get(onlp_onie_info_t* onie));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sysi_oids_get(onlp_oid_t* table, int max));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sysi_platform_info_get(onlp_platform_info_t* pi));
__ONLP_DEFAULTI_VIMPLEMENTATION(onlp_sysi_platform_info_free(onlp_platform_info_t* pi));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sysi_ioctl(int id, va_list vargs));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sysi_platform_manage_init(void));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sysi_platform_manage_fans(void));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sysi_platform_manage_leds(void));

