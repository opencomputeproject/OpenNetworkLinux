#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <syslog.h>

#include "platform_comm.h"
#include "platform_wobmc.h"
#include "ledcontrol.h"

extern struct fan Fan[FAN_COUNT + 1];
extern struct temp Temp_B2F[THERMAL_COUNT + 1];
extern struct temp Temp_F2B[THERMAL_COUNT + 1];
extern struct vol_curr_pwr Vol_Curr_Pwr[POWER_COUNT + 1];
extern struct psu Psu[PSU_COUNT + 1];
extern uint8_t Sys_Airflow;

static uint8_t set_panel_psu_led(uint8_t mode)
{
    int ret = 0;

    ret = syscpld_setting(LPC_LED_PSU_REG, mode);
    if (ret) {
        perror("Can't set PSU LED");
        return ERR;
    }

    return OK;
}

static uint8_t set_panel_fan_led(uint8_t mode)
{
    int ret = 0;

    ret = syscpld_setting(LPC_LED_FAN_REG, mode);
    if (ret) {
        perror("Can't set FAN LED");
        return ERR;
    }

    return OK;
}

static uint8_t set_panel_alarm_led(uint8_t mode)
{
    int ret = 0;

    ret = syscpld_setting(LPC_LED_ALARM_REG, mode);
    if (ret) {
        perror("Can't set alarm LED");
        return ERR;
    }

    return OK;
}

int update_led(void)
{
    int alarm_led = 0;
    int ret = 0;
    int i = 0, fault = 0;
    int num_fan_absent = 0;
    int num_psu_absent = 0;
    int num_fan_fault = 0;
    int num_psu_fault = 0;
    int num_fan_airflow_fault = 0;
    int num_psu_airflow_fault = 0;
    FILE *fp = NULL;
    char buffer[200] = {0};
    struct temp *temp_array;

    num_psu_absent = check_psu_absent();
    num_fan_absent = check_fan_absent();
    num_psu_fault = check_psu_fault();
    num_fan_fault = check_fan_fault();
    num_fan_airflow_fault = check_fan_airflow_fault(Sys_Airflow);
    num_psu_airflow_fault = check_psu_airflow_fault(Sys_Airflow);

    /* Set panel PSU LED */
    if (0 == num_psu_absent && 0 == num_psu_fault) {
        set_panel_psu_led(LED_PSU_GRN);
    }
    else {
        set_panel_psu_led(LED_PSU_AMB);
    }

    /* Set panel fan LED */
    if (CHASSIS_FAN_COUNT == num_fan_absent) {
        set_panel_fan_led(LED_FAN_OFF);
    }
    else if (0 != num_fan_fault || num_fan_absent > 0) {
        set_panel_fan_led(LED_FAN_AMB);
    } else {
        set_panel_fan_led(LED_FAN_GRN);
    }

    /* Set panel alarm LED */
    fault = num_psu_absent + num_fan_absent + num_psu_fault + \
            num_fan_fault + num_fan_airflow_fault + num_psu_airflow_fault;
    if (0 == fault) {
        alarm_led = led_grn;
    }
    else if (1 == fault) {
        alarm_led = led_amb_1hz;
    }
    else if (fault > 1) {
        alarm_led = led_amb;
    }

    /* temperature fault flags are set in fancontrol service */
    if (FAN_F2B == Sys_Airflow) {
        temp_array = Temp_F2B;
    } else {
        temp_array = Temp_B2F;
    }
    for (i = 1; i <= THERMAL_COUNT; i++) {
        if (temp_array[i].is_ovshutdown_fault) {
            alarm_led += led_amb;
            continue;
        } else if (temp_array[i].is_overr_fault) {
            alarm_led += led_amb_4hz;
            continue;
        }
        else if (temp_array[i].is_ovwarn_fault) {
            alarm_led += led_amb_1hz;
        } else {
            alarm_led += led_grn;
        }
    }
    /* check power fault flags */
    for (i = 1; i <= POWER_COUNT; i++) {
        if (NULL != strstr(Vol_Curr_Pwr[i].descr, "PSU1")) {
           if (Psu[1].is_absent || Psu[1].is_ac_fault ||
               Psu[1].is_power_fault || Psu[1].is_alert) {
               continue;
           }
        }
        if (NULL != strstr(Vol_Curr_Pwr[i].descr, "PSU2")) {
           if (Psu[2].is_absent || Psu[2].is_ac_fault ||
               Psu[2].is_power_fault || Psu[2].is_alert) {
               continue;
           }
        }
        ret = get_sysnode_value(Vol_Curr_Pwr[i].path, &Vol_Curr_Pwr[i].vol_curr_pwr);
        if (ret < 0)
            continue;

        if (NOT_DEFINE != Vol_Curr_Pwr[i].hi_shutdown &&
               Vol_Curr_Pwr[i].vol_curr_pwr >= Vol_Curr_Pwr[i].hi_shutdown) {
            if (0 == Vol_Curr_Pwr[i].is_ovshutdown_fault)
                syslog(LOG_WARNING, "%s voltage %ld is over than %ld !!!\n",
                        Vol_Curr_Pwr[i].descr, Vol_Curr_Pwr[i].vol_curr_pwr,
                                               Vol_Curr_Pwr[i].hi_shutdown);

            Vol_Curr_Pwr[i].is_ovshutdown_fault = 1;
            /**
             * Need to count the number of errors to set the alarm LED,
             * so here clear over warn error
             */
            Vol_Curr_Pwr[i].is_overr_fault = 0;
            Vol_Curr_Pwr[i].is_ovwarn_fault = 0;
            Vol_Curr_Pwr[i].is_udshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_uderr_fault = 0;
            Vol_Curr_Pwr[i].is_udwarn_fault = 0;
            alarm_led += led_amb;
        } else if (NOT_DEFINE != Vol_Curr_Pwr[i].hi_err &&
                     Vol_Curr_Pwr[i].vol_curr_pwr >= Vol_Curr_Pwr[i].hi_err) {
            if (0 == Vol_Curr_Pwr[i].is_overr_fault)
                syslog(LOG_WARNING, "%s voltage %ld is over than %ld !!!\n",
                        Vol_Curr_Pwr[i].descr, Vol_Curr_Pwr[i].vol_curr_pwr,
                                               Vol_Curr_Pwr[i].hi_err);

            Vol_Curr_Pwr[i].is_ovshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_overr_fault = 1;
            Vol_Curr_Pwr[i].is_ovwarn_fault = 0;
            Vol_Curr_Pwr[i].is_udshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_uderr_fault = 0;
            Vol_Curr_Pwr[i].is_udwarn_fault = 0;
            alarm_led += led_amb_4hz;
        } else if (NOT_DEFINE != Vol_Curr_Pwr[i].hi_warn &&
                     Vol_Curr_Pwr[i].vol_curr_pwr >= Vol_Curr_Pwr[i].hi_warn) {
            if (0 == Vol_Curr_Pwr[i].is_ovwarn_fault)
                syslog(LOG_WARNING, "%s voltage %ld is over than %ld !!!\n",
                        Vol_Curr_Pwr[i].descr, Vol_Curr_Pwr[i].vol_curr_pwr,
                                               Vol_Curr_Pwr[i].hi_warn);

            Vol_Curr_Pwr[i].is_ovshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_overr_fault = 0;
            Vol_Curr_Pwr[i].is_ovwarn_fault = 1;
            Vol_Curr_Pwr[i].is_udshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_uderr_fault = 0;
            Vol_Curr_Pwr[i].is_udwarn_fault = 0;
            alarm_led += led_amb_1hz;
        } else if (NOT_DEFINE != Vol_Curr_Pwr[i].lo_shutdown &&
                     Vol_Curr_Pwr[i].vol_curr_pwr <= Vol_Curr_Pwr[i].lo_shutdown) {
            if (0 == Vol_Curr_Pwr[i].is_udshutdown_fault)
                syslog(LOG_WARNING, "%s voltage %ld is less than %ld !!!\n",
                        Vol_Curr_Pwr[i].descr, Vol_Curr_Pwr[i].vol_curr_pwr,
                                               Vol_Curr_Pwr[i].lo_shutdown);

            Vol_Curr_Pwr[i].is_ovshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_overr_fault = 0;
            Vol_Curr_Pwr[i].is_ovwarn_fault = 0;
            Vol_Curr_Pwr[i].is_udshutdown_fault = 1;
            Vol_Curr_Pwr[i].is_uderr_fault = 0;
            Vol_Curr_Pwr[i].is_udwarn_fault = 0;
            alarm_led += led_amb;
        } else if (NOT_DEFINE != Vol_Curr_Pwr[i].lo_err &&
                     Vol_Curr_Pwr[i].vol_curr_pwr <= Vol_Curr_Pwr[i].lo_err) {
            if (0 == Vol_Curr_Pwr[i].is_uderr_fault)
                syslog(LOG_WARNING, "%s voltage %ld is less than %ld !!!\n",
                        Vol_Curr_Pwr[i].descr, Vol_Curr_Pwr[i].vol_curr_pwr,
                                               Vol_Curr_Pwr[i].lo_err);

            Vol_Curr_Pwr[i].is_ovshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_overr_fault = 0;
            Vol_Curr_Pwr[i].is_ovwarn_fault = 0;
            Vol_Curr_Pwr[i].is_udshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_uderr_fault = 1;
            Vol_Curr_Pwr[i].is_udwarn_fault = 0;
            alarm_led += led_amb_4hz;
        } else if (NOT_DEFINE != Vol_Curr_Pwr[i].lo_warn &&
                     Vol_Curr_Pwr[i].vol_curr_pwr <= Vol_Curr_Pwr[i].lo_warn) {
            if (0 == Vol_Curr_Pwr[i].is_udwarn_fault)
                syslog(LOG_WARNING, "%s voltage %ld is less than %ld !!!\n",
                        Vol_Curr_Pwr[i].descr, Vol_Curr_Pwr[i].vol_curr_pwr,
                                               Vol_Curr_Pwr[i].lo_warn);

            Vol_Curr_Pwr[i].is_ovshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_overr_fault = 0;
            Vol_Curr_Pwr[i].is_ovwarn_fault = 0;
            Vol_Curr_Pwr[i].is_udshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_uderr_fault = 0;
            Vol_Curr_Pwr[i].is_udwarn_fault = 1;
            alarm_led += led_amb_1hz;
        } else {
            Vol_Curr_Pwr[i].is_ovshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_overr_fault = 0;
            Vol_Curr_Pwr[i].is_ovwarn_fault = 0;
            Vol_Curr_Pwr[i].is_udshutdown_fault = 0;
            Vol_Curr_Pwr[i].is_uderr_fault = 0;
            Vol_Curr_Pwr[i].is_udwarn_fault = 0;
            alarm_led += led_grn;
       }
   }

    /* Check mce error */
    fp = popen("dmesg | grep 'Machine check events logged'", "r");
    if (fp == NULL) {
        printf("call popen %s fail\n", "dmesg");
        return -1;
    }
    fscanf(fp, "%s", buffer);
    if (0 != strlen(buffer)) {
        alarm_led += led_amb;
        syslog(LOG_WARNING, "MCE event is logged !!!\n");
    } else {
        alarm_led += led_grn;
    }
    pclose(fp);

    if (alarm_led >= led_amb) {
        set_panel_alarm_led(LED_ALARM_AMB);
    } else if (alarm_led == led_amb_4hz) {
        set_panel_alarm_led(LED_ALARM_AMB_4HZ);
    } else if (alarm_led == led_amb_1hz) {
        set_panel_alarm_led(LED_ALARM_AMB_1HZ);
    } else {
        set_panel_alarm_led(LED_ALARM_GRN);
    }

    return 0;
}
