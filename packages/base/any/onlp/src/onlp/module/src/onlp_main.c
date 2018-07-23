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
#include <uCli/ucli.h>
#include <AIM/aim_thread.h>
#include <unistd.h>
#include <onlp/onlp.h>
#include "onlp_log.h"

/*
 * Some CLI commands require the original argv vector
 * to implement things like daemonized restart.
 */
extern char** onlp_ucli_argv;

int
onlpdump_main(int argc, char* argv[])
{

    ucli_t* ucli = NULL;
    char** argv_strings;
    char** opt;

    aim_thread_name_set("onlpd.main");
    onlp_ucli_argv = argv;

    extern ucli_node_t* onlp_ucli_node_create(void);
    char hostname[128] = {0};
    gethostname(hostname, sizeof(hostname)-1);
    const char* prompt = aim_fstrdup("%s.onlp", hostname);

    AIM_TRY_OR_DIE(onlp_sw_init(NULL));

    AIM_TRY_OR_DIE(ucli_init());
    ucli = ucli_create(prompt, NULL, onlp_ucli_node_create());
    argv_strings = ucli_argv_to_strings(argv+1);
    if(argv_strings) {
        for(opt = argv_strings; *opt; opt++) {
            if(ucli_dispatch_string(ucli, &aim_pvs_stdout, *opt) < 0) {
                return 1;
            }
        }
        ucli_argv_to_strings_free(argv_strings);
    }
    if(!argv_strings || !argv_strings[0]) {
        ucli_run(ucli, prompt);
    }
    aim_free((char*)prompt);
    ucli_destroy(ucli);

    AIM_TRY_OR_DIE(onlp_sw_denit());
    return 0;
}

#if TO_BE_CONVERTED_TO_UCLI

    int show = 0;
    uint32_t showflags = 0;
    int help = 0;
    int c;
    int rv = -1;
    int j = 0;
    int o = 0;
    int m = 0;
    int i = 0;
    int p = 0;
    int x = 0;
    int S = 0;
    int l = 0;
    int M = 0;
    int b = 0;
    char* pidfile = NULL;
    const char* O = NULL;
    const char* t = NULL;
    const char* J = NULL;

    /**
     * debug trap
     */
    if(argc > 1 && (!strcmp(argv[1], "debug"))) {
        onlp_sw_init(NULL);
        return onlp_debug(&aim_pvs_stdout, argc-2, argv+2);
    }

    while( (c = getopt(argc, argv, "srehdojmyM:ipxlSt:O:bJ:")) != -1) {
        switch(c)
            {
            case 's': show=1; break;
            case 'r': show=1; showflags |= ONLP_OID_SHOW_RECURSE; break;
            case 'e': show=1; showflags |= ONLP_OID_SHOW_EXTENDED; break;
            case 'd': show=0; break;
            case 'h': help=1; rv = 0; break;
            case 'j': j=1; break;
            case 'o': o=1; break;
            case 'x': x=1; break;
            case 'm': m=1; break;
            case 'M': M=1; pidfile = optarg; break;
            case 'i': i=1; break;
            case 'p': p=1; show=-1; break;
            case 't': t = optarg; break;
            case 'O': O = optarg; break;
            case 'S': S=1; break;
            case 'l': l=1; break;
            case 'b': b=1; break;
            case 'J': J = optarg; break;
            case 'y': show=1; showflags |= ONLP_OID_SHOW_YAML; break;
            default: help=1; rv = 1; break;
            }
    }

    if(help) {
        printf("Usage: %s [OPTIONS]\n", argv[0]);
        printf("  -d   Use dump(). This is the default.\n");
        printf("  -s   Use show() instead of dump().\n");
        printf("  -r   Recursive show(). Implies -s\n");
        printf("  -e   Extended show(). Implies -s\n");
        printf("  -y   Yaml show(). Implies -s\n");
        printf("  -o   Dump ONIE data only.\n");
        printf("  -x   Dump Platform Info only.\n");
        printf("  -j   Dump ONIE data in JSON format.\n");
        printf("  -m   Run platform manager.\n");
        printf("  -M   Run as platform manager daemon.\n");
        printf("  -i   Iterate OIDs.\n");
        printf("  -p   Show SFP presence.\n");
        printf("  -t   <file>  Decode TlvInfo data.\n");
        printf("  -O   <oid> Dump OID.\n");
        printf("  -S   Decode SFP Inventory\n");
        printf("  -b   Decode SFP Inventory into SFF database entries.\n");
        printf("  -l   API Lock test.\n");
        printf("  -J   Decode ONIE JSON data.\n");
        return rv;
    }

    AIM_REFERENCE(j);

    if(J) {
        int rv;
        onlp_onie_info_t onie;
        rv = onlp_onie_read_json(&onie, J);
        if(rv < 0) {
            fprintf(stderr, "onie read json failed: %d\n", rv);
            return 1;
        }
        else {
            onlp_onie_show(&onie, &aim_pvs_stdout);
            onlp_onie_info_free(&onie);
            return 0;
        }
    }

    if(t) {
        int rv;
        onlp_onie_info_t onie;
        rv = onlp_onie_decode_file(&onie, t);
        if(rv >= 0) {
            onlp_onie_show(&onie, &aim_pvs_stdout);
            onlp_onie_info_free(&onie);
            return 0;
        }
        else {
            aim_printf(&aim_pvs_stdout, "Decode failed.");
            return 1;
        }
    }

    onlp_sw_init(NULL);

    if(M) {
        platform_manager_daemon__(pidfile, argv);
        exit(0);
    }

    if(l) {
        extern int onlp_api_lock_test(void);
        int i;
        for(i = 1; i; i++) {
            if(onlp_api_lock_test() != 0) {
                return 1;
            }
            aim_printf(&aim_pvs_stdout, "%d\r", i);
        }
    }

    if(S) {
        show_inventory__(&aim_pvs_stdout, b);
        return 0;
    }

    if(O) {
        int oid;
        if(sscanf(O, "0x%x", &oid) == 1) {
            onlp_oid_dump(oid, &aim_pvs_stdout,
                          ONLP_OID_DUMP_RECURSE |
                          ONLP_OID_DUMP_EVEN_IF_ABSENT);
        }
        return 0;
    }

    if(i) {
        iterate_oids__();
        return 0;
    }

    if(o || x) {
        onlp_sys_info_t si;
        if(onlp_sys_info_get(&si) < 0) {
            fprintf(stderr, "onlp_sys_info_get() failed.");
            return 1;
        }

        if(o) {
            if(j) {
                onlp_onie_show_json(&si.onie_info, &aim_pvs_stdout);
            }
            else {
                onlp_onie_show(&si.onie_info, &aim_pvs_stdout);
            }
        }

        if(x) {
            if(j) {
                onlp_platform_info_show_json(&si.platform_info, &aim_pvs_stdout);
            }
            else {
                onlp_platform_info_show(&si.platform_info, &aim_pvs_stdout);
            }
        }

        onlp_sys_info_free(&si);
        return 0;
    }
    if(show >= 0) {
        if(show == 0) {
            /* Default to full dump */
            onlp_platform_dump(&aim_pvs_stdout,
                               ONLP_OID_DUMP_RECURSE | ONLP_OID_DUMP_EVEN_IF_ABSENT);
        }
        else {
            onlp_platform_show(&aim_pvs_stdout,
                               showflags);
        }
    }


    if(m) {
        printf("Running the platform manager for 600 seconds...\n");
        onlp_platform_manager_start(0);
        sleep(600);
        printf("Stopping the platform manager.\n");
        onlp_platform_manager_stop(1);
    }

    if(p) {
        onlp_sfp_bitmap_t presence;
        onlp_sfp_bitmap_t_init(&presence);
        int rv = onlp_sfp_presence_bitmap_get(&presence);
        aim_printf(&aim_pvs_stdout, "Presence: ");
        if(rv < 0) {
            aim_printf(&aim_pvs_stdout, "Error %{onlp_status}\n", rv);
        }
        else {
            aim_printf(&aim_pvs_stdout, "%{aim_bitmap}\n", &presence);
        }
    }
    return 0;
}

#include <AIM/aim_daemon.h>
#include <AIM/aim_pvs_syslog.h>
#include <signal.h>
#include <errno.h>

void
sighandler__(int signal)
{
    onlp_platform_manager_stop(0);
}

static void
platform_manager_daemon__(const char* pidfile, char** argv)
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
    aim_log_handler_basic_init_all("onlpd",
                               "/var/log/onlpd.log",
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
    signal(SIGTERM, sighandler__);

    /** Start and block in platform manager. */
    onlp_platform_manager_start(1);

    /** Terminated via signal. Cleanup and exit. */
    onlp_platform_manager_stop(1);

    aim_log_handler_basic_denit_all();
    exit(0);
}
#endif
