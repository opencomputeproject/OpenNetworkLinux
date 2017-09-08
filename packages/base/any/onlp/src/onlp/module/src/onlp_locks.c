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
onlp_api_lock_denit(void)
{
    os_sem_destroy(api_sem__);
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

#include <onlplib/shlocks.h>

void
onlp_api_lock_init(void)
{
    onlp_shlock_global_init();
}

void
onlp_api_lock(const char* api)
{
    onlp_shlock_global_take();
}
void
onlp_api_unlock(void)
{
    onlp_shlock_global_give();
}

#endif


/*
 * This function will perform a sanity test on the API locking implementation.
 */
#include <onlplib/file.h>

static int
onlp_api_lock_test_locked__(void)
{
    static int counter__ = 1;
    int readback = 0;
    const char* fname = "/tmp/onlp_api_lock_test";

    fclose(fopen(fname, "w"));
    onlp_file_write_int(counter__, fname, NULL);
    onlp_file_read_int(&readback, fname, NULL);
    if(readback != counter__) {
        fprintf(stderr, "API LOCK TEST: write=%d read=%d", counter__, readback);
        return -1;
    }
    counter__++;
    return 0;
}
ONLP_LOCKED_API0(onlp_api_lock_test);

#else

int
onlp_api_lock_test(void)
{
    fprintf(stderr, "API Locking support not available in this build.\n");
    return 2;
}

#endif /* ONLP_CONFIG_INCLUDE_API_LOCK */
