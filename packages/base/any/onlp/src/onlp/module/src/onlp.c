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
 *
 ***********************************************************/
#include <onlp/onlp_config.h>
#include <onlp/onlp.h>

#include <onlp/platform.h>

#include <onlp/chassis.h>
#include <onlp/fan.h>
#include <onlp/generic.h>
#include <onlp/led.h>
#include <onlp/module.h>
#include <onlp/psu.h>
#include <onlp/sfp.h>
#include <onlp/thermal.h>

#include "onlp_int.h"
#include "onlp_json.h"
#include "onlp_locks.h"

int
onlp_sw_init(const char* platform)
{
    extern void __onlp_module_init__(void);
    __onlp_module_init__();

#if ONLP_CONFIG_INCLUDE_API_LOCK == 1
    onlp_api_lock_init();
#endif

    ONLP_TRY(onlp_platform_sw_init(platform));

#define ONLP_OID_TYPE_ENTRY(_name, _id, _upper, _lower) \
    ONLP_TRY(onlp_##_lower##_sw_init());
#include <onlp/onlp.x>

    return 0;
}

int
onlp_hw_init(uint32_t flags)
{
    return 0;
}

int
onlp_sw_denit(void)
{

#define ONLP_OID_TYPE_ENTRY(_name, _id, _upper, _lower) \
    ONLP_TRY(onlp_##_lower##_sw_denit());
#include <onlp/onlp.x>

    ONLP_TRY(onlp_platform_sw_denit());

#if ONLP_CONFIG_INCLUDE_API_LOCK == 1
    onlp_api_lock_denit();
#endif


    return 0;
}
