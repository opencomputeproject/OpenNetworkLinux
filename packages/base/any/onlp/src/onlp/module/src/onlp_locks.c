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
#include "onlp_locks.h"

#if ONLP_CONFIG_INCLUDE_API_LOCK == 1

#if ONLP_CONFIG_API_LOCK_GLOBAL_SHARED == 0

#include <OS/os_sem.h>

/**
 * The API Lock is a simple semaphore locking within
 * the current process only.
 */
static os_sem_t api_sem__;
static const char* owner__ = NULL;

void
onlp_api_lock_init(void)
{
    api_sem__ = os_sem_create_flags(1, OS_SEM_CREATE_F_TRUE_RELATIVE_TIMEOUTS);
}

void
onlp_api_lock(const char* api)
{
    if(os_sem_take_timeout(api_sem__, ONLP_CONFIG_API_LOCK_TIMEOUT) != 0) {
        AIM_DIE("The ONLP API lock in %s could not be acquired after %d microseconds. It appears to be currently owned by call to %s. This is considered fatal.",
                api, ONLP_CONFIG_API_LOCK_TIMEOUT, owner__ ? owner__ : "(none)");
    }
    owner__ = api;
}

void
onlp_api_unlock(void)
{
    os_sem_give(api_sem__);
}

#else
#error GLOBAL_SHARED API Lock support is not yet implemented.
#endif

#endif /* ONLP_CONFIG_INCLUDE_API_LOCK */


