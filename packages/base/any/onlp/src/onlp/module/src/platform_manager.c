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
 * This file implements the Platform Management infrastructure.
 *
 ***********************************************************/
#include <onlp/psu.h>
#include <onlp/fan.h>
#include <onlp/platformi/platformi.h>
#include <onlplib/mmap.h>
#include <timer_wheel/timer_wheel.h>
#include <OS/os_time.h>
#include <OS/os_thread.h>
#include <AIM/aim.h>
#include "onlp_log.h"
#include "onlp_int.h"
#include <sys/eventfd.h>
#include <errno.h>
#include <pthread.h>

/**
 * Timer wheel callback entry.
 */
typedef struct management_entry_s {
    /** Timer wheel for this entry */
    timer_wheel_entry_t twe;

    /** This is the callback for this timer */
    int (*manage)(void);

    /** This is the callback rate in microseconds */
    uint64_t rate;

    /** The name of this callback (for debugging) */
    const char* name;

    /** The number of times this has been called. */
    int calls;

} management_entry_t;

/**
 * Platform management control structure.
 */
typedef struct management_ctrl_s {
    timer_wheel_t* tw;

    int eventfd;
    pthread_t thread;

} management_ctrl_t;

/* This is the global control state */
static management_ctrl_t control__ = { NULL };


/*
 * Internal notification handler for PSU
 * status changes (all platforms)
 */
static int platform_psus_notify__(void);


/*
 * Internal notification handler for FAN
 * status changes (all platforms)
 */
static int platform_fans_notify__(void);



/*
 * First Version : Static callback rates.
 * TODO: Allow individual platform callbacks to reregister
 * themselves at whatever rate they want.
 */
static management_entry_t management_entries[] =
    {
        {
            { },
            onlp_platformi_manage_fans,
            /* Every 10 seconds */
            10*1000*1000,
            "Fans",
        },
        {
            { },
            onlp_platformi_manage_leds,
            /* Every 2 seconds */
            2*1000*1000,
            "LEDs",

        },
        {
            { },
            platform_psus_notify__,
            /* Every second */
            1*1000*1000,
            "PSUs",
        },
        {
            { },
            platform_fans_notify__,
            /* Every second */
            1*1000*1000,
            "Fans",
        }
    };


void
onlp_sys_platform_manage_init(void)
{
    if(control__.tw == NULL) {
        int i;
        uint64_t now = os_time_monotonic();

        onlp_platformi_manage_init();
        control__.tw = timer_wheel_create(4, 512, now);

        for(i = 0; i < AIM_ARRAYSIZE(management_entries); i++) {
            management_entry_t* e = management_entries+i;
            timer_wheel_insert(control__.tw,  &e->twe, now + e->rate);
        }
    }
}


void
onlp_sys_platform_manage_now(void)
{
    management_entry_t* e;

    onlp_sys_platform_manage_init();

    while( (e = (management_entry_t*) timer_wheel_next(control__.tw,
                                                       os_time_monotonic())) ) {
        if(e->manage) {
            e->manage();
        }
        e->calls++;
        timer_wheel_insert(control__.tw, &e->twe, os_time_monotonic() + e->rate);
    }
}

static void*
onlp_sys_platform_manage_thread__(void* vctrl)
{
    volatile management_ctrl_t* ctrl = (volatile management_ctrl_t*)(vctrl);

    os_thread_name_set("onlp.sys.pm");

    /*
     * Wait on the eventfd for the specified timeout period.
     */
    for(;;) {

        fd_set fds;
        uint64_t now;
        struct timeval tv;
        timer_wheel_entry_t* twe;

        FD_ZERO(&fds);
        FD_SET(ctrl->eventfd, &fds);

        /*
         * Ask the timer wheel if there is an expiration in the next 2 seconds.
         */
        now = os_time_monotonic();
        twe = timer_wheel_peek(ctrl->tw, now + 20000000);

        if(twe == NULL) {
            /* Nothing in the next two seconds. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;
        }
        else {
            if(twe->deadline > now) {
                /* Sleep until next deadline */
                tv.tv_sec = (twe->deadline - now) / 1000000;
                tv.tv_usec = (twe->deadline - now) % 1000000;
            }
            else {
                /* We have surpassed the current deadline */
                tv.tv_sec = 0;
                tv.tv_usec = 0;
            }
        }

        int rv = select(ctrl->eventfd+1, &fds, NULL, NULL, &tv);
        if(rv == 1 && FD_ISSET(ctrl->eventfd, &fds)) {
            /* We've been asked to terminate. */
            AIM_LOG_MSG("Terminating.");
            /* Also signifies that we have exit */
            close(ctrl->eventfd);
            ctrl->eventfd = -1;
            return NULL;
        }
        if(rv < 0) {
            AIM_LOG_ERROR("select() returned %d (%{errno})", rv, errno);
            /* Sleep 1 second, but continue to run */
            sleep(1);
        }

        /*
         * We don't bother to check the result of select() here.
         */
        onlp_sys_platform_manage_now();
    }
}

int
onlp_platform_manager_start(int block)
{
    onlp_sys_platform_manage_init();

    if(control__.eventfd > 0) {
        /* Already running */
        return 0;
    }

    if( (control__.eventfd = eventfd(0, EFD_SEMAPHORE)) < 0) {
        AIM_LOG_ERROR("eventfd create failed: %{errno}", errno);
        return -1;
    }

    if( (pthread_create(&control__.thread, NULL, onlp_sys_platform_manage_thread__,
                        &control__)) != 0) {
        AIM_LOG_ERROR("pthread create failed.");
        close(control__.eventfd);
        control__.eventfd = -1;
        return -1;
    }

    if(block) {
        onlp_platform_manager_join();
    }

    return 0;
}

int
onlp_platform_manager_stop(int block)
{
    if(control__.eventfd > 0) {
        uint64_t zero = 1;
        /* Tell the thread to exit */
        write(control__.eventfd, &zero, sizeof(zero));

        if(block) {
            onlp_platform_manager_join();
        }
    }
    return 0;
}

int
onlp_platform_manager_join(void)
{
    if(control__.eventfd > 0) {
        /* Wait for the thread to terminate */
        pthread_join(control__.thread, NULL);
        close(control__.eventfd);
        control__.eventfd = -1;
    }
    return 0;
}

static onlp_oid_hdr_t*
oid_hdr_entry_find__(biglist_t* list, onlp_oid_t oid)
{
    onlp_oid_hdr_t* hdr;
    biglist_t* ble;

    BIGLIST_FOREACH_DATA(ble, list, onlp_oid_hdr_t*, hdr) {
        if(hdr->id == oid) {
            return hdr;
        }
    }
    return NULL;
}

static int
platform_psus_notify__(void)
{
    int rv;
    static biglist_t* previous = NULL;
    static biglist_t* current = NULL;

    biglist_t* ble;
    onlp_oid_hdr_t* hdr;

    if(ONLP_FAILURE(rv = onlp_oid_hdr_get_all(ONLP_OID_CHASSIS,
                                              ONLP_OID_TYPE_FLAG_PSU, 0x0,
                                              &current))) {
        return rv;
    }

    if(previous == NULL) {
        /** Log initial states. */
        BIGLIST_FOREACH_DATA(ble, current, onlp_oid_hdr_t*, hdr) {
            int pid = ONLP_OID_ID_GET(hdr->id);

            if(ONLP_OID_PRESENT(hdr)) {
                AIM_SYSLOG_INFO("PSU <id> is present.",
                                "The given PSU is present.",
                                "PSU %d is present.", pid);

                if(ONLP_OID_FAILED(hdr)) {
                    AIM_SYSLOG_CRIT("PSU <id> has failed.",
                                    "The given PSU has failed.",
                                    "PSU %d has failed.", pid);
                }
                else if(ONLP_OID_STATUS_FLAG_IS_SET(hdr, UNPLUGGED)) {
                    AIM_SYSLOG_WARN("PSU <id> is unplugged.",
                                    "The given PSU is unplugged.",
                                    "PSU %d is unplugged.", pid);
                }
            }
            else {
                AIM_SYSLOG_INFO("PSU <id> is not present.",
                                "The given PSU is not present.",
                                "PSU %d is not present.", pid);
            }
        }
        previous = current;
        current = NULL;
        return 0;
    }


    BIGLIST_FOREACH_DATA(ble, current, onlp_oid_hdr_t*, hdr) {

        int pid = ONLP_OID_ID_GET(hdr->id);
        onlp_oid_hdr_t* phdr = oid_hdr_entry_find__(previous, hdr->id);

        if(!phdr) {
            /* A new PSU has popped into existance. Unlikely. */
            AIM_SYSLOG_INFO("PSU <id> has been discovered.",
                            "A new PSU has been discovered.",
                            "PSU %d has been discovered.", pid);
            continue;
        }

        uint32_t xor = hdr->status ^ phdr->status;

        if(xor & ONLP_OID_STATUS_FLAG_PRESENT) {
            if(ONLP_OID_PRESENT(hdr)) {
                AIM_SYSLOG_INFO("PSU <id> has been inserted.",
                                "A PSU has been inserted in the given slot.",
                                "PSU %d has been inserted.", pid);
            }
            else {
                AIM_SYSLOG_WARN("PSU <id> has been removed.",
                                "A PSU has been removed from the given slot.",
                                "PSU %d has been removed.", pid);
                /* The remaining bits are only relevant if the PSU is present. */
                continue;
            }
        }
        if(xor & ONLP_OID_STATUS_FLAG_FAILED) {
            if(ONLP_OID_FAILED(hdr)) {
                AIM_SYSLOG_CRIT("PSU <id> has failed.",
                                "The given PSU has failed.",
                                "PSU %d has failed.", pid);
            }
            else {
                AIM_SYSLOG_INFO("PSU <id> has recovered.",
                                "The given PSU has recovered from a failure.",
                                "PSU %d has recovered.", pid);
            }
        }
        if(xor & ONLP_OID_STATUS_FLAG_UNPLUGGED) {
            if(ONLP_OID_STATUS_FLAG_IS_SET(hdr, UNPLUGGED)) {
                /* PSU has been unplugged. */
                AIM_SYSLOG_WARN("PSU <id> has been unplugged.",
                                "The given PSU has been unplugged.",
                                "PSU %d has been unplugged.", pid);
            }
            else {
                /* PSU has been plugged in */
                AIM_SYSLOG_INFO("PSU <id> has been plugged in.",
                                "The given PSU has been plugged in.",
                                "PSU %d has been plugged in.", pid);
            }
        }
    }

    BIGLIST_FOREACH_DATA(ble, previous, onlp_oid_hdr_t*, hdr) {
        onlp_oid_hdr_t* chdr = oid_hdr_entry_find__(current, hdr->id);
        if(!chdr) {
            /* A PSU has disappeared. */
            AIM_SYSLOG_INFO("PSU <id> has disappeared.",
                            "A PSU has disappeared.",
                            "PSU %d has disappeared.", ONLP_OID_ID_GET(hdr->id));
        }
    }


    /* The previous list is deleted and the current list becomes the previous */
    onlp_oid_get_all_free(previous);
    previous = current;
    current = NULL;
    return 0;
}

static int
platform_fans_notify__(void)
{
    int rv;
    static biglist_t* previous = NULL;
    static biglist_t* current = NULL;

    biglist_t* ble;
    onlp_oid_hdr_t* hdr;

    if(ONLP_FAILURE(rv = onlp_oid_hdr_get_all(ONLP_OID_CHASSIS,
                                              ONLP_OID_TYPE_FLAG_FAN, 0x0,
                                              &current))) {
        return rv;
    }

    if(previous == NULL) {
        /** Log initial states. */
        BIGLIST_FOREACH_DATA(ble, current, onlp_oid_hdr_t*, hdr) {
            int fid = ONLP_OID_ID_GET(hdr->id);

            if(ONLP_OID_PRESENT(hdr)) {
                AIM_SYSLOG_INFO("Fan <id> is present.",
                                "The given fan is present.",
                                "Fan %d is present.", fid);

                if(ONLP_OID_FAILED(hdr)) {
                    AIM_SYSLOG_INFO("Fan <id> has failed.",
                                    "The given fan has failed.",
                                    "Fan %d has failed.", fid);
                }
            }
            else {
                AIM_SYSLOG_INFO("Fan <id> is not present.",
                                "The given fan is not present.",
                                "Fan %d is not present.", fid);
            }
        }
        previous = current;
        current = NULL;
        return 0;
    }


    BIGLIST_FOREACH_DATA(ble, current, onlp_oid_hdr_t*, hdr) {

        int fid = ONLP_OID_ID_GET(hdr->id);
        onlp_oid_hdr_t* phdr = oid_hdr_entry_find__(previous, hdr->id);

        if(!phdr) {
            /* A new Fan has popped into existance. Unlikely. */
            AIM_SYSLOG_INFO("Fan <id> has been discovered.",
                            "A new fan has been discovered.",
                            "Fan %d has been discovered.", fid);
            continue;
        }

        uint32_t xor = hdr->status ^ phdr->status;

        if(xor & ONLP_OID_STATUS_FLAG_PRESENT) {
            if(ONLP_OID_PRESENT(hdr)) {
                AIM_SYSLOG_INFO("Fan <id> has been inserted.",
                                "A fan has been inserted.",
                                "Fan %d has been inserted.", fid);
            }
            else {
                AIM_SYSLOG_WARN("Fan <id> has been removed.",
                                "A fan has been removed.",
                                "Fan %d has been removed.", fid);
                /* The remaining bits are only relevant if the Fan is present. */
                continue;
            }
        }
        if(xor & ONLP_OID_STATUS_FLAG_FAILED) {
            if(ONLP_OID_FAILED(hdr)) {
                AIM_SYSLOG_CRIT("Fan <id> has failed.",
                                "The given fan has failed.",
                                "Fan %d has failed.", fid);
            }
            else {
                AIM_SYSLOG_INFO("Fan <id> has recovered.",
                                "The given fan has recovered from a failure.",
                                "Fan %d has recovered.", fid);
            }
        }
    }

    BIGLIST_FOREACH_DATA(ble, previous, onlp_oid_hdr_t*, hdr) {
        onlp_oid_hdr_t* chdr = oid_hdr_entry_find__(current, hdr->id);
        if(!chdr) {
            /* A Fan has disappeared. */
            AIM_SYSLOG_INFO("Fan <id> has disappeared.",
                            "A fan has disappeared.",
                            "Fan %d has disappeared.", ONLP_OID_ID_GET(hdr->id));
        }
    }


    /* The previous list is deleted and the current list becomes the previous */
    onlp_oid_get_all_free(previous);
    previous = current;
    current = NULL;
    return 0;
}
