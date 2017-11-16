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
#include <onlp/oids.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <AIM/aim.h>
#include <onlp/onlp.h>
#include <onlplib/shlocks.h>

/**
 * Base functionality unit tests.
 */
#define __TRY(_prefix, _expr, _suffix)                          \
    do {                                                        \
        int _rv;                                                \
        fprintf(stderr, "%s%s...%s", _prefix, #_expr, _suffix); \
        fflush(stderr);                                         \
        _rv = _expr ;                                           \
        fprintf(stderr, "%s%s...%d\n", _prefix, #_expr, _rv);   \
        fflush(stderr);                                         \
        if(_rv < 0) {                                           \
            AIM_DIE("%s%s: failed: %d", #_expr, _rv);           \
        }                                                       \
    } while(0)

#define __TRYNR(_prefix, _expr, _suffix)                        \
    do {                                                        \
        fprintf(stderr, "%s%s...%s", _prefix, #_expr, _suffix); \
        fflush(stderr);                                         \
        _expr ;                                                 \
        fprintf(stderr, "%s%s...Done\n", _prefix, #_expr);      \
        fflush(stderr);                                         \
    } while(0)

#define TRY(_expr) __TRY("  ", _expr, "\r")
#define TRYNR(_expr) ___TRYNR("  ", _expr, "\r")
#define TEST(_expr) __TRYNR("", _expr, "\n");

/**
 * Test Shared Locks
 */
void
shlock_test(void)
{
    onlp_shlock_t* lock = NULL;

    TRY(onlp_shlock_create(0xEEEF, &lock, "utest-lock:%d", 1));
    TRY(onlp_shlock_take(lock));
    TRY(onlp_shlock_give(lock));
    TRY(onlp_shlock_take(lock));
    TRY(onlp_shlock_give(lock));
    TRY(onlp_shlock_global_take());
    TRY(onlp_shlock_global_give());
    TRY(onlp_shlock_global_take());
    TRY(onlp_shlock_global_give());
    if(strcmp("utest-lock:1", onlp_shlock_name(lock))) {
        AIM_DIE("lock name does not match (%s)", onlp_shlock_name(lock));
    }
}

/**
 * Test ONIE parsing
 */
void
onie_test(void)
{
    /* TODO */
}

int
iter__(onlp_oid_t oid, void* cookie)
{
    onlp_oid_hdr_t hdr;
    onlp_oid_hdr_get(oid, &hdr);
    printf("OID: 0x%x, D='%s'\n", oid, hdr.description);
    return 0;
}

#include <onlp/fan.h>
#include <onlp/thermal.h>
#include <onlp/oids.h>
#include <onlp/sys.h>

int
aim_main(int argc, char* argv[])
{
    //    TEST(shlock_test());

    /* Example Platform Dump */
    onlp_init();
    onlp_platform_dump(&aim_pvs_stdout, ONLP_OID_DUMP_RECURSE);
    onlp_oid_iterate(0, 0, iter__, NULL);
    onlp_platform_show(&aim_pvs_stdout, ONLP_OID_SHOW_RECURSE|ONLP_OID_SHOW_EXTENDED);

    if(argv[1] && !strcmp("manage", argv[1])) {
        onlp_sys_platform_manage_start();
        printf("Sleeping...\n");
        sleep(10);
        printf("Stopping...\n");
        onlp_sys_platform_manage_stop();
        printf("Stopped.\n");
    }
    return 0;
}
