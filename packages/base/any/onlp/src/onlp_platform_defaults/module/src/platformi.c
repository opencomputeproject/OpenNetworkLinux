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
#include <onlp/platformi/platformi.h>
#include "onlp_platform_defaults_int.h"
#include "onlp_platform_defaults_log.h"

const char* __ONLP_DEFAULTI
onlp_platformi_get(void)
{
    /*
     * This function should never be called.
     *
     * If we get here its because the platform libraries are
     * not written, initialized, or setup properly before we execute.
     */
    AIM_LOG_ERROR("The default implementation of onlp_platformi_get() has been called.");
    AIM_LOG_ERROR("This can happen for the following reasons, all fatal:");
    AIM_LOG_ERROR("* The ONLP build configuration is incorrect.");
    AIM_LOG_ERROR("* The ONLP platform library for this platform does not contain the onlp_platformi_get() symbol.");
    AIM_LOG_ERROR("* The ONLP platform shared libraries are not setup properly before we executed.");
    AIM_LOG_ERROR("* The platform cannot continue until this issue is resolved.");
    abort();
    return NULL;
}

int __ONLP_DEFAULTI
onlp_platformi_debug(aim_pvs_t* pvs, int argc, char* argv[])
{
    aim_printf(pvs, "This platform does not support debug features.\n");
    return -1;
}

__ONLP_DEFAULTI_IMPLEMENTATION(onlp_platformi_sw_init(void));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_platformi_hw_init(uint32_t flags));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_platformi_sw_denit(void));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_platformi_set(const char* p));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_platformi_manage_init(void));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_platformi_manage_fans(void));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_platformi_manage_leds(void));
