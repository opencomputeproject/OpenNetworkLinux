/************************************************************
 * <bsn.cl v=2014 v=onl>
 * 
 *           Copyright 2015 Big Switch Networks, Inc.          
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
 * IPC Shared Memory Locks
 *
 ***********************************************************/
#include <onlplib/shlocks.h>
#include "onlplib_log.h"
#include <sys/ipc.h>
#include <errno.h>

static int
shared_pthread_mutex_init__(pthread_mutex_t* mutex)
{
    int rv;
    pthread_mutexattr_t ma;

    pthread_mutexattr_init(&ma);

    /* default to failed */
    rv = -1;
    if(pthread_mutexattr_setrobust(&ma, PTHREAD_MUTEX_ROBUST) != 0) {
        AIM_LOG_ERROR("setrobust() failed: %{errno}", errno);
    }
    else if(pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED) != 0) {
        AIM_LOG_ERROR("setpshared() failed: %{errno}", errno);
    }
    else if(pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_ERRORCHECK) != 0) {
        AIM_LOG_ERROR("settype() failed: %{errno}", errno);
    }
    else if(pthread_mutex_init(mutex, &ma) != 0) {
        AIM_LOG_ERROR("mutex_init() failed: %{errno}", errno);
    }
    else {
        /* Success */
        rv = 0;
    }
    pthread_mutexattr_destroy(&ma);
    return rv;
}

int
onlp_shmem_create(key_t key, uint32_t size, void** rvmem)
{
    int rv = 0;
    int shmid;

    if(rvmem == NULL) {
        return -1;
    }
    *rvmem = NULL;

#define SHARED_MODE_FLAGS 0


    shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | SHARED_MODE_FLAGS) ;
    if(shmid == -1) {
        if(errno == EEXIST) {
            shmid = shmget(key, size, IPC_CREAT | SHARED_MODE_FLAGS);
            if(shmid == -1) {
                /* Exists, but could not be accessed */
                AIM_LOG_ERROR("shmget failed on existing segment: %{errno}", errno);
                return -1;
            }
            else {
                /* Already Exists */
                rv = 0;
            }
        }
    }
    else {
        /* Newly created */
        rv = 1;
    }

    *rvmem = shmat(shmid, 0, 0);
    if(*rvmem == ( (void*) -1 )) {
        AIM_LOG_ERROR("shmat failed on segment: %{errno}", errno);
        rv = -1;
    }
    return rv;
}


struct onlp_shlock_s {
    uint32_t magic;
    char name[64];

    pthread_mutex_t mutex;
};

#define SHLOCK_MAGIC 0xDEADBEEF

static void
onlp_shlock_init__(onlp_shlock_t* l, const char* fmt, va_list vargs)
{
    if(l->magic != SHLOCK_MAGIC) {
        if(shared_pthread_mutex_init__(&l->mutex) != 0) {
            /* There is no useful recovery from this */
            AIM_DIE("shlock_init(): mutex_init failed\n");
        }
        char* s = aim_vfstrdup(fmt, vargs);
        aim_strlcpy(l->name, s, sizeof(l->name));
        l->magic = SHLOCK_MAGIC;
    }
}


int
onlp_shlock_create(key_t id, onlp_shlock_t** rvl, const char* fmt, ...)
{

    onlp_shlock_t* l = NULL;
    int rv = onlp_shmem_create(id, sizeof(onlp_shlock_t), (void**)&l);

    if(rv >= 0) {
        va_list vargs;
        va_start(vargs, fmt);
        /* Initialize if necessary */
        onlp_shlock_init__(l, fmt, vargs);
        va_end(vargs);
        *rvl = l;
    }
    else {
        AIM_DIE("shlock_create(): shmem_create failed\n");
        rv = -1;
    }
    return rv;
}

int
onlp_shlock_destroy(onlp_shlock_t* shlock)
{
    /* Nothing at the moment. */
    return 0;
}

int
onlp_shlock_take(onlp_shlock_t* shlock)
{
    int rv;

    if(shlock == NULL) {
        AIM_DIE("shlock_take(): lock is NULL");
    }

    rv = pthread_mutex_lock(&shlock->mutex);
    if(rv == 0) {
        /* All good. */
        return 0;
    }
    if(rv == EOWNERDEAD) {
        /*
         * We got the lock, but someone else aborted while holding it.
         * No explicit recovery actions at this point.
         */
        AIM_LOG_WARN("Detected EOWNERDEAD on take.");
        pthread_mutex_consistent(&shlock->mutex);
        return 0;
    }

    /*
     * No other runtime conditions are allowed.
     * abort to make that obvious during debugging and development.
     */
    AIM_DIE("mutex_lock failed: %{errno}", errno);
    return -1;
}


/**
 * @brief Give a shared memory lock.
 * @param shlock The shared lock.
 */
int
onlp_shlock_give(onlp_shlock_t* shlock)
{
    if(shlock == NULL) {
        AIM_DIE("shlock_give(): lock is NULL");
    }

    if(pthread_mutex_unlock(&shlock->mutex) != 0) {
        AIM_DIE("mutex_unlock() failed: %{errno}", errno);
        return -1;
    }
    return 0;
}

const char*
onlp_shlock_name(onlp_shlock_t* lock)
{
    return lock->name;
}


static onlp_shlock_t* global_lock__ = NULL;


void
onlp_shlock_global_init(void)
{
    if(global_lock__ == NULL) {
        if(onlp_shlock_create(ONLP_SHLOCK_GLOBAL_KEY, &global_lock__,
                              "onlp-global-lock") < 0) {
            AIM_DIE("Global lock created failed.");
        }
    }
}

int
onlp_shlock_global_take(void)
{
#if ONLP_CONFIG_INCLUDE_SHLOCK_GLOBAL_INIT == 0
    /* Always attempt initialization first */
    onlp_shlock_global_init();
#endif

    return onlp_shlock_take(global_lock__);
}

int
onlp_shlock_global_give(void)
{
    return onlp_shlock_give(global_lock__);
}
