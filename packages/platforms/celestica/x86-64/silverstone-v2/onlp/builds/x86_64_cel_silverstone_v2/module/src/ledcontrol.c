#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <syslog.h> 

#include "platform_common.h"
#include "platform_nonbmc.h"
#include "ledcontrol.h"

extern const struct psu_reg_bit_mapper psu_mapper [PSU_COUNT + 1];

static uint8_t set_psu_led(uint8_t mode)
{
	int ret;
	
    ret = syscpld_setting(LPC_LED_PSU_REG, mode);
	if(ret)
		return ERR;
	
	return OK;
}

static uint8_t set_fan_led(uint8_t mode)
{
	int ret;
	
    ret = syscpld_setting(LPC_LED_FAN_REG, mode);
	if(ret)
		return ERR;
	
	return OK;

}

static int check_psu_status_led(void)
{
	uint8_t i = 0;
	uint8_t psu_status = 0;
    static uint8_t dc_flt_flg = 0;
    static uint8_t absent_flg = 0;

	int present_status=0,pow_status=0;

	for (i = 1; i <= PSU_COUNT; i++) {
		psu_status = get_psu_status(i);
		present_status = (psu_status >> psu_mapper[i].bit_present) & 0x01;
		pow_status = (psu_status >> psu_mapper[i].bit_pow_sta) & 0x01;

		if (0 == present_status) {
			if (0 == pow_status) {
                if (0 == (dc_flt_flg & (1 << i)))
					syslog(LOG_WARNING, "PSU%d DC power is detected failed !!!\n", i);
                dc_flt_flg |= (1 << i);
				return ERR;
			}else
				dc_flt_flg &= ~(1 << i);
			absent_flg &= ~(1 << i);
		} else {
            if (0 == (absent_flg & (1 << i)))
				syslog(LOG_WARNING, "PSU%d is absent !!!\n", i);
            absent_flg |= (1 << i);
			return ERR;
		}
	}

	return OK;
}


static int check_fan_status_led(int air_flow)
{
	int speed = 0, status = 0;
	int front_max = 0, rear_max = 0;
	int ret = OK, i = 0, cnt = 0, fail = 0;
	int pwm = 0;
	char data[100] = {0};
	int low_thr, high_thr = 0; 

	/* to avoid collect error logs all the time */
    static uint8_t frtovspeed_flg = 0;
    static uint8_t frtudspeed_flg = 0;
    static uint8_t rearovspeed_flg = 0;
    static uint8_t rearudspeed_flg = 0;
    static uint8_t absent_flg = 0;

	if (FAN_DIR_F2B == air_flow) {
		front_max = CHASSIS_FRONT_FAN_MAX_RPM_F2B;
		rear_max = CHASSIS_REAR_FAN_MAX_RPM_F2B;
	}
	else if(FAN_DIR_B2F == air_flow){
		front_max = CHASSIS_FRONT_FAN_MAX_RPM_B2F;
		rear_max = CHASSIS_REAR_FAN_MAX_RPM_B2F;
	}
	

	for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
		sprintf(data, "%sfan%d_present", CHASSIS_FAN_PATH, i);
		ret = get_sysnode_value(data, (void *)&status);
		if (ret) {
			printf("Can't get sysnode value\n");
			return ERR;
		}
		if (status == ABSENT) {
            if (0 == (absent_flg & (1 << i)))
			syslog(LOG_WARNING, "Fan%d is absent !!!\n", i);
            absent_flg |= (1 << i);
			cnt++;
			if (cnt == CHASSIS_FAN_COUNT)
				return ABSENT;
			fail++;
		}
        else
            absent_flg &= ~(1 << i);

		memset(data,'0',sizeof(data));
		sprintf(data, "%sfan%d_front_speed_rpm", CHASSIS_FAN_PATH, i);
		ret = get_sysnode_value(data, (void *)&speed);
		if (ret) {
			printf("Can't get sysnode value\n");
			return ERR;
		}
		if (speed < PWM_FAULT_DEF_FRONT)
			fail++;

	    /* if fan rpm duty is out of tolerant scope, set as fault */
		memset(data,'0',sizeof(data));
		sprintf(data, "%spwm%d", CHASSIS_FAN_PATH, i);
		ret = get_sysnode_value(data, (void *)&pwm);
		if (ret) {
			printf("Can't get sysnode value\n");
			return ERR;
		}

		low_thr = front_max * pwm * (100 - CHASIS_FAN_TOLERANT_PER) / 25500; 
		high_thr = front_max * pwm * (100 + CHASIS_FAN_TOLERANT_PER) / 25500; 
		if (speed < low_thr) {
            if (0 == (frtudspeed_flg & (1 << i)))
					syslog(LOG_WARNING, "Fan%d front rpm %d is less than %d at pwm %d!!!\n",
                               i, speed, low_thr, pwm); 
            frtudspeed_flg |= (1 << i);
			fail++;
		} else if (speed > high_thr) {
            if (0 == (frtovspeed_flg & (1 << i)))
					syslog(LOG_WARNING, "Fan%d front rpm %d is larger than %d at pwm %d!!!\n",
                               i, speed, high_thr, pwm); 
            frtovspeed_flg |= (1 << i);
			fail++;
        }else
        {
            frtovspeed_flg &= ~(1 << i);
            frtudspeed_flg &= ~(1 << i);
		}
		memset(data,'0',sizeof(data));
		sprintf(data, "%sfan%d_rear_speed_rpm", CHASSIS_FAN_PATH, i);
		ret = get_sysnode_value(data, (void *)&speed);
  		if (ret) {
			printf("Can't get sysnode value\n");
			return ERR;
		}
		if (speed < PWM_FAULT_DEF_REAR) 
	        fail++;

	    /* if fan rpm duty is out of tolerant scope, set as fault */
	    low_thr = rear_max * pwm * (100 - CHASIS_FAN_TOLERANT_PER) / 25500; 
	    high_thr = rear_max * pwm * (100 + CHASIS_FAN_TOLERANT_PER) / 25500; 
	    if (speed < low_thr) {
            if (0 == (rearudspeed_flg & (1 << i)))
				syslog(LOG_WARNING, "Fan%d rear rpm %d is less than %d at pwm %d!!!\n",
                               i, speed, low_thr, pwm); 
            rearudspeed_flg |= (1 << i);
			fail++;
		} else if (speed > high_thr) {
            if (0 == (rearovspeed_flg & (1 << i)))
				syslog(LOG_WARNING, "Fan%d rear rpm %d is larger than %d at pwm %d!!!\n",
                               i, speed, high_thr, pwm); 
            rearovspeed_flg |= (1 << i);
	        fail++;
		}else{
			rearovspeed_flg &= ~(1 << i);
			rearudspeed_flg &= ~(1 << i);
		}
	}

	return fail;
}


int update_led(void)
{
	uint8_t ret = 0;
	int air_flow = -1;

	/* get fan1 direction as system fan direction */
	ret = get_sysnode_value(CHASSIS_FAN_DIRECTION(1), (void *)&air_flow);
	if (ret) {
		printf("Can't get sysnode value\n");
		return ERR;
	}
 
	ret = check_psu_status_led();
	if (OK == ret) {
		/* set psu led as green */
		set_psu_led(LED_PSU_GRN);
	} else {
		/* set psu led as amber */
		set_psu_led(LED_PSU_AMB);
	}

	ret = check_fan_status_led(air_flow);
	if (OK == ret) {
		/* set fan led as green */
		set_fan_led(LED_FAN_GRN);
	} else if (ABSENT == ret) {
		/* set fan led as off */
		set_fan_led(LED_FAN_OFF);
	} else {
		/* set fan led as amber */
		set_fan_led(LED_FAN_AMB);
	}

	return 0;
}

