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
#include <onlp/platform.h>
#include <onlp/platformi/platformi.h>

#include <AIM/aim.h>
#include <AIM/aim_daemon.h>

#include "onlp_log.h"
#include "onlp_int.h"
#include "onlp_locks.h"


#include <unistd.h>
#include <signal.h>
#include <errno.h>

/**
 * @brief Determine the platform name from the filesystem.
 */
static char*
platform_detect_fs__(void)
{
    /*
     * Check the filesystem for the platform identifier.
     */
    char* rv = NULL;
    if(ONLP_CONFIG_PLATFORM_FILENAME) {
        FILE* fp;
        if((fp=fopen(ONLP_CONFIG_PLATFORM_FILENAME, "r"))) {
            char platform[256];
            if(fgets(platform, sizeof(platform), fp) == platform) {
                if(platform[0]) {
                    if(platform[ONLP_STRLEN(platform)-1] == '\n') {
                        platform[ONLP_STRLEN(platform)-1] = 0;
                    }
                    rv = aim_strdup(platform);
                }
            }
            fclose(fp);
        }
        else {
            AIM_LOG_ERROR("could not open platform filename '%s'", ONLP_CONFIG_PLATFORM_FILENAME);
        }
    }
    return rv;
}

static char*
onlp_platform_name_get__(const char* override)
{
    if(override) {
        return aim_strdup(override);
    }

    if(getenv("ONLP_CONFIG_PLATFORM_NAME")) {
        return aim_strdup(getenv("ONLP_CONFIG_PLATFORM_NAME"));
    }

    if(ONLP_CONFIG_PLATFORM_NAME) {
        /** Set at compile time. */
        return aim_strdup(ONLP_CONFIG_PLATFORM_NAME);
    }
    return platform_detect_fs__();
}

char*
onlp_platform_name_get(void)
{
    return onlp_platform_name_get__(NULL);
}

int
onlp_platform_sw_init_locked__(const char* platform)
{
    int rv;
    platform = onlp_platform_name_get__(platform);
    if(platform == NULL) {
        AIM_DIE("Could not determine the current platform.");
    }

    const char* driver = onlp_platformi_get();

    if( (driver == NULL) ||
        strcmp(driver, platform) ) {
        /**
         * The platform name and the driver name do not match.
         * Request the current platform explicitly.
         */
        if(ONLP_FAILURE(rv = onlp_platformi_set(platform))) {
            if(ONLP_UNSUPPORTED(rv)) {
                AIM_LOG_ERROR("The current platform interface (%s) does not support the current platform (%s). This is fatal.",
                              driver, platform);
            }
            else {
                AIM_LOG_ERROR("onlp_platformi_set(%s) failed: %{onlp_status}",
                              platform, rv);
            }
            aim_free((void*)platform);
            return rv;
        }
    }

    /* If we get here, its all good */
    aim_free((void*)platform);

    rv = onlp_platformi_sw_init();
    return rv;
}
ONLP_LOCKED_API1(onlp_platform_sw_init, const char*, platform);


int
onlp_platform_hw_init_locked__(uint32_t flags)
{
    return onlp_platformi_hw_init(flags);
}
ONLP_LOCKED_API1(onlp_platform_hw_init, uint32_t, flags);

int
onlp_platform_sw_denit_locked__(void)
{
    return 0;
}
ONLP_LOCKED_API0(onlp_platform_sw_denit);

static void
daemon_sighandler__(int signal)
{
    onlp_platform_manager_stop(0);
}


void
onlp_platform_manager_daemon(const char* name,
                             const char* logfile,
                             const char* pidfile,
                             char** argv)
{
    aim_pvs_t* aim_pvs_syslog = NULL;
    aim_daemon_restart_config_t rconfig;
    aim_daemon_config_t config;

    memset(&config, 0, sizeof(config));
    aim_daemon_restart_config_init(&rconfig, 1, 1, argv);
    AIM_BITMAP_CLR(&rconfig.signal_restarts, SIGTERM);
    AIM_BITMAP_CLR(&rconfig.exit_restarts, 0);
    rconfig.maximum_restarts=50;
    rconfig.pvs = NULL;
    config.wd = "/";

    aim_daemonize(&config, &rconfig);
    aim_log_handler_basic_init_all(name,
                                   logfile,
                                   1024*1024,
                                   99);
    if(pidfile) {
        FILE* fp = fopen(pidfile, "w");
        if(fp == NULL) {
            int e = errno;
            aim_printf(aim_pvs_syslog, "fatal: open(%s): %s\n",
                       pidfile, strerror(e));
            aim_printf(&aim_pvs_stderr, "fatal: open(%s): %s\n",
                       pidfile, strerror(e));

            /* Don't attempt restart */
            raise(SIGTERM);
        }
        fprintf(fp, "%d\n", getpid());
        fclose(fp);
    }

    /** Signal handler for terminating the platform manager */
    signal(SIGTERM, daemon_sighandler__);

    /** Start and block in platform manager. */
    onlp_platform_manager_start(1);

    /** Terminated via signal. Cleanup and exit. */
    onlp_platform_manager_stop(1);

    aim_log_handler_basic_denit_all();
    exit(0);
}
