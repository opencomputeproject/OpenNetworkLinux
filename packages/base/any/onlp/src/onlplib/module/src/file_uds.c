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
 *
 *
 ***********************************************************/
#include <onlplib/file_uds.h>
#include "onlplib_log.h"

#include <BigList/biglist.h>
#include <BigList/biglist_locked.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

/**
 * Read/write an eventfd.
 */
static void
eventfd_write__(int fd)
{
    uint64_t val = 1;
    write(fd, &val, sizeof(val));
}
static void
eventfd_read__(int fd)
{
    uint64_t val;
    read(fd, &val, sizeof(val));
}


/**
 * This represents a single domain socket service.
 */
typedef struct onlp_file_uds_service_s {
    /** domain socket file path */
    const char* path;

    /** Listening descriptor */
    int lfd;

    /** client handler */
    onlp_file_uds_handler_t handler;
    void* cookie;

    /** service is active. */
    int active;

} onlp_file_uds_service_t;

/**
 * Destroy a file service.
 */
static void
onlp_file_uds_service_clear__(onlp_file_uds_service_t* p)
{
    if(p) {
        if(p->lfd > 0) {
            close(p->lfd);
        }
        if(p->path) {
            aim_free((char*)p->path);
        }
        memset(p, 0, sizeof(*p));
    }
}
static void
onlp_file_uds_service_destroy__(onlp_file_uds_service_t* p)
{
    if(p) {
        onlp_file_uds_service_clear__(p);
        aim_free(p);
    }
}

/**
 * Create a file service.
 */
static int
onlp_file_uds_service_create__(onlp_file_uds_service_t** rvp,
                               const char* path,
                               onlp_file_uds_handler_t handler, void* cookie)
{
    struct sockaddr_un addr;

    onlp_file_uds_service_t* rv = aim_zmalloc(sizeof(*rv));

    rv->path = aim_strdup(path);
    char* cmd = aim_fstrdup("mkdir -p `dirname %s`", path);
    if(system(cmd) != 0) {
        AIM_LOG_ERROR("Failed to create uds directory for %s", path);
        aim_free(cmd);
        goto failed;
    }
    aim_free(cmd);

    if ((rv->lfd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) {
        AIM_LOG_ERROR("socket: %{errno}", errno);
        goto failed;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
    unlink(path);

    if(bind(rv->lfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        AIM_LOG_ERROR("bind: %{errno}", errno);
        goto failed;
    }

    if (listen(rv->lfd, 1) == -1) {
        AIM_LOG_ERROR("listen: %{errno}", errno);
        goto failed;
    }

    rv->handler = handler;
    rv->cookie = cookie;
    *rvp = rv;

    return 0;

 failed:
    onlp_file_uds_service_destroy__(rv);
    return -1;
}

/**
 * This is the control object for a UDS service group.
 */
struct onlp_file_uds_s {

    /** Thread signal. Used to wake up the service thread when required. */
    int eventfd;

    /** Service worker thread */
    pthread_t thread;
    volatile int running;
    volatile int terminate;

    /** Service client list */
    biglist_locked_t* list;
};


/**
 * Add a descriptor to an epoll set.
 */
static int
epoll_add__(int epoll_fd, int add_fd, uint32_t events, void* data,
            int* counter, const char* name)
{
    struct epoll_event ev = {0};
    ev.data.ptr = data;
    ev.events = events;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, add_fd, &ev) != 0) {
        if(errno != EEXIST) {
            AIM_LOG_ERROR("epoll_ctl returned %{errno} for %s", errno, name);
            return -1;
        }
    }

    if(counter) {
        (*counter)++;
    }

    return 0;
}

/**
 * Service a connection.
 */
static void
accept__(onlp_file_uds_service_t* ufp)
{
    int fd;

    if((fd = accept(ufp->lfd, NULL, 0)) > 0) {
        ufp->handler(fd, ufp->cookie);
    }
    close(fd);
}

/**
 * This is the service list entry.
 */

/**
 * The service worker thread.
 *
 * All registered services are polled for incoming connections
 * and handled in series.
 *
 * These are designed for simple transactions and not
 * long-lived connections.
 */
static void*
uds_thread_worker__(void* p)
{
    volatile onlp_file_uds_t* control = (onlp_file_uds_t*)p;

    int epollfd;
    if((epollfd = epoll_create(256)) == -1) {
        AIM_LOG_ERROR("epoll_create(): %{errno}", errno);
        return NULL;
    }

    control->running = 1;
    for(;;) {

        biglist_t* ble;
        onlp_file_uds_service_t* ufp;
        int nfd = 0;

        if(control->terminate) {
            /** Request for termination. */
            break;
        }

        /** control->eventfd wakes us up */
        if(epoll_add__(epollfd, control->eventfd, EPOLLIN, NULL, &nfd, "eventfd") < 0) {
            return NULL;
        }

        /** Add all active descriptors. */
        biglist_lock(control->list);
        BIGLIST_FOREACH_DATA(ble, control->list->list, onlp_file_uds_service_t*, ufp) {
            switch(ufp->active)
                {
                case 1:
                    /* Service is active. Wait on it. */
                    epoll_add__(epollfd, ufp->lfd, EPOLLIN, ufp, &nfd, ufp->path);
                    break;
                case -1:
                    /* Service deletion request. */
                    AIM_LOG_MSG("Removing %s...", ufp->path);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, ufp->lfd, NULL);
                    onlp_file_uds_service_clear__(ufp);
                    break;
                case 0:
                    /* Service is inactive. */
                    break;
                }
        }

        biglist_unlock(control->list);

        struct epoll_event* events = aim_zmalloc(sizeof(*events)*nfd);
        int rv = epoll_wait(epollfd, events, nfd, -1);

        if(rv < 0) {
            if(errno != EINTR) {
                AIM_LOG_ERROR("epoll_wait() returned %{errno}", errno);
                break;
            }
        }
        else if(rv == 0) {
            /** Shouldn't happen with infinite timeout */
        }
        else {
            int i;
            for(i = 0; i < rv; i++) {
                if(events[i].events & EPOLLIN) {
                    onlp_file_uds_service_t* ufp = (onlp_file_uds_service_t*)events[i].data.ptr;
                    if(ufp == NULL) {
                        eventfd_read__(control->eventfd);
                    }
                    else {
                        if(ufp->active == 1) {
                            accept__(ufp);
                        }
                    }
                }
            }
        }
        aim_free(events);
    }
    control->running = 0;
    return NULL;
}

int
onlp_file_uds_create(onlp_file_uds_t** rvp)
{
    onlp_file_uds_t* rv = aim_zmalloc(sizeof(*rv));
    if((rv->eventfd = eventfd(0, 0)) == -1) {
        AIM_LOG_ERROR("eventfd: %{errno}", errno);
        goto failed;
    }
    if((rv->list = biglist_locked_create()) == NULL) {
        goto failed;
    }

    rv->running = 0;

    if(pthread_create(&rv->thread, NULL, uds_thread_worker__, rv) != 0) {
        AIM_LOG_ERROR("pthread_create failed: %{errno}", errno);
        goto failed;
    }

    *rvp = rv;
    return 0;

 failed:
    onlp_file_uds_destroy(rv);
    return -1;
}

void
onlp_file_uds_destroy(onlp_file_uds_t* p)
{
    if(p) {
        if(p->running == 1) {
            p->terminate = 1;
            eventfd_write__(p->eventfd);
            pthread_join(p->thread, NULL);
        }
        biglist_locked_free_all(p->list, (biglist_free_f)onlp_file_uds_service_destroy__);
        aim_free(p);
    }
}

static onlp_file_uds_service_t*
find_uds_locked__(biglist_t* list, const char* path)
{
    biglist_t* ble;
    onlp_file_uds_service_t* ufp;
    BIGLIST_FOREACH_DATA(ble, list, onlp_file_uds_service_t*, ufp) {
        if(ufp->path) {
            if(!strcmp(path, ufp->path)) {
                return ufp;
            }
        }
    }
    return NULL;
}

int
onlp_file_uds_add(onlp_file_uds_t* fuds, const char* path,
                  onlp_file_uds_handler_t handler, void* cookie)
{
    int rv = 0;
    biglist_lock(fuds->list);
    if(find_uds_locked__(fuds->list->list, path)) {
        AIM_LOG_ERROR("Cannot add duplicate UDS for %s", path);
        rv = -1;
    }
    else {
        onlp_file_uds_service_t* ufp;
        if(onlp_file_uds_service_create__(&ufp, path, handler, cookie) >= 0) {
            ufp->active = 1;
            fuds->list->list = biglist_append(fuds->list->list, ufp);
        }
    }
    biglist_unlock(fuds->list);
    eventfd_write__(fuds->eventfd);
    return rv;
}

void
onlp_file_uds_remove(onlp_file_uds_t* fuds, const char* path)
{
    biglist_lock(fuds->list);
    onlp_file_uds_service_t* ufp = find_uds_locked__(fuds->list->list, path);
    if(ufp) {
        /** Request deactivation */
        ufp->active = -1;
    }
    biglist_unlock(fuds->list);
}
