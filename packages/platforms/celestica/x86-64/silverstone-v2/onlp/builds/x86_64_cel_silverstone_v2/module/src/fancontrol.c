#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include "fancontrol.h"
#include "platform_common.h"
#include "platform_nonbmc.h"

linear_t U17LINEAR_B2F = {
    23, 49, 102, 255, 3, -100, 0, 0
};

linear_t U16LINEAR_F2B = {
    37, 52, 97, 255, 3, -100, 0, 0
};


/* temperature unit is mC, so setpoint multiple 1000 to be the same level */
pid_algo_t CPU_PID = {
     -100, 0, 96, 1.8, 0.3, 0.0, 0
};

pid_algo_t SW_PID = {
    -100, 0, 100, 3, 0.3, 0.5, 0
};

extern const struct psu_reg_bit_mapper psu_mapper[PSU_COUNT + 1];



uint8_t get_temp(const char *path)
{
	uint8_t temp = 0;
	int t  = 0;
	int ret = 0;

	ret = get_sysnode_value(path, (void *)&t);
	if (ret) {
		printf("Can't get sysnode value\n");
		return ERR;
	}
	temp = t / TEMP_CONV_FACTOR;

	return temp;
}

int set_fan(const char *path, uint8_t pwm)
{
	int ret = 0;

	ret = set_sysnode_value(path, pwm);

	return ret;
}

int set_all_fans(uint8_t pwm)
{
	int ret = 0, i = 0;
	char  data[100] = {0};

	for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
		sprintf(data, "%spwm%d", CHASSIS_FAN_PATH, i);
		ret = set_fan(data, pwm);
		if (ret) {
			printf("Can't set fan%d pwm\n", i);
			return ERR;
		}
	}

	return 0;
}
int create_sysfs(char* sysfs_path, char* module,uint16_t addr)
{
	int rc = 0;
	char path[100] = {0},data[30] = {0};

	sprintf(path, "%s/delete_device",sysfs_path);
	int fd = open(path,O_WRONLY);
	if(fd < 0) {
		printf("Fila to open the file: %s \n", path);
		return ERR;
	}
	sprintf(data,"0x%x", addr);
	rc = write(fd,data,strlen(data));
	if (rc != strlen(data)){
		printf("Write failed.\n");
		close(fd);
		return ERR;
	}
	close(fd);
	sprintf(path,"%s/new_device", sysfs_path);
	sprintf(data,"%s 0x%x", module,addr);
	fd = open(path,O_WRONLY);
	if(fd < 0) {
		printf("Fila to open the file: %s \n", sysfs_path);
		return ERR;
	}
	rc = write(fd,data,strlen(data));
	if (rc != strlen(data)){
		printf("Write failed.\n");
		close(fd);
		return ERR;
	}
	close(fd);

	return 0;
}
int check_psu_status(void)
{
	uint8_t psu_status = 0, present_status = 0;

	psu_status = get_psu_status(PSU1_ID);
	present_status = (psu_status >> psu_mapper[PSU1_ID].bit_present) & 0x01;
	if (present_status == 1) /* absent */
		return FAULT;

	psu_status = get_psu_status(PSU2_ID);
	present_status = (psu_status >> psu_mapper[PSU2_ID].bit_present) & 0x01;
	if (present_status == 1) /* absent */
		return FAULT;

	return OK;
}

int check_psu_sysfs_status(int id)
{
	int status = 0,ret = 0;
	FILE *fp = NULL;
	char path[100] = {0};
	uint8_t psu_status = 0, present_status = 0;

	switch (id)
	{
		case PSU1_ID:
			sprintf(path,"ls %s",PSU1_THERMAL1);
			fp = popen(path, "r");
		    if (fp == NULL)
		    {
		        syslog(LOG_DEBUG, "No PSU%d sysfs\n",PSU1_ID);
		        status = -1;
		    }
			pclose(fp);
			if (status) {
				psu_status = get_psu_status(PSU1_ID);
				present_status = (psu_status >> psu_mapper[PSU1_ID].bit_present) & 0x01;
				if (present_status == 1) /* absent */
					ret++;
				/*if the psu is present, need to create the sysfs again to gain the temp*/
				else {
					if(create_sysfs(PSU1_SYSFS,PSU_MODULE,PSU1_I2C_ADDR))
						ret++;
					if(create_sysfs(PSU1_SYSFS,PSU_EEPROM_MODULE,PSU1_EEPROM_ADDR))
						ret++;
				}
			}

		case PSU2_ID:
			sprintf(path,"ls %s",PSU2_THERMAL1);
			fp = popen(path, "r");
		    if (fp == NULL)
		    {
		        syslog(LOG_DEBUG, "No PSU%d sysfs\n",PSU2_ID);
		        status = -1;
		    }
			pclose(fp);
			if (status) {
				psu_status = get_psu_status(PSU2_ID);
				present_status = (psu_status >> psu_mapper[PSU2_ID].bit_present) & 0x01;
				if (present_status == 1) /* absent */
					ret++;
				/*if the psu is present, need to create the sysfs again to gain the temp*/
				else {
					if(create_sysfs(PSU2_SYSFS,PSU_MODULE,PSU2_I2C_ADDR))
						ret++;
					if(create_sysfs(PSU2_SYSFS,PSU_EEPROM_MODULE,PSU2_EEPROM_ADDR))
						ret++;
				}
			}
	}

	return ret;
}

int check_fan_status(uint8_t *fault_num)
{
    int speed = 0, status = 0;
    int ret = 0, i = 0;
    char data[100] = {0};

    *fault_num = 0;

    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
		sprintf(data, "%sfan%d_present", CHASSIS_FAN_PATH, i);
		ret = get_sysnode_value(data, (void *)&status);
	if (ret) {
		printf("Can't get sysnode value\n");
		return ERR;
	}
	if (status == ABSENT)
		return ABSENT;

	sprintf(data, "%sfan%d_front_speed_rpm", CHASSIS_FAN_PATH, i);
	ret = get_sysnode_value(data, (void *)&speed);
	if (ret) {
		printf("Can't get sysnode value\n");
		return ERR;
	}
	if (speed < PWM_FAULT_DEF_FRONT) {
		syslog(LOG_DEBUG, "Fan%d Front speed is lower than %d!!!\n", i, PWM_FAULT_DEF_FRONT);
		(*fault_num)++;
	}
	sprintf(data, "%sfan%d_rear_speed_rpm", CHASSIS_FAN_PATH, i);
	ret = get_sysnode_value(data, (void *)&speed);
	if (ret) {
		printf("Can't get sysnode value\n");
		return ERR;
	}
	if (speed < PWM_FAULT_DEF_REAR) {
		syslog(LOG_DEBUG, "Fan%d Rear speed is lower than %d!!!\n", i, PWM_FAULT_DEF_REAR);
		(*fault_num)++;
	}
    }
    if (*fault_num > 0)
	    return FAULT;
    else
	    return OK;
}

short upper_bound(uint8_t temp, uint8_t min_temp, uint8_t max_temp, uint8_t min_pwm, uint8_t max_pwm, uint8_t hysteresis)
{
    short pwm = 0;

    pwm = (temp - min_temp) * (max_pwm - min_pwm) / (max_temp - min_temp - hysteresis) + min_pwm;
    //DEBUG_PRINT("%d, %d, %d, %d, %d, %d, %d\n", temp, min_temp, max_temp, min_pwm, max_pwm, hysteresis, pwm);

    return pwm > 0 ? pwm : 0;
}

short lower_bound(uint8_t temp, uint8_t min_temp, uint8_t max_temp, uint8_t min_pwm, uint8_t max_pwm, uint8_t hysteresis)
{
    short pwm = 0;

    pwm = (temp - min_temp - hysteresis) * (max_pwm - min_pwm) / (max_temp - min_temp - hysteresis) + min_pwm;
    //DEBUG_PRINT("%d, %d, %d, %d, %d, %d, %d\n", temp, min_temp, max_temp, min_pwm, max_pwm, hysteresis, pwm);

    return pwm > 0 ? pwm : 0;
}
int check_cpu_temp_valid(int max, int temp)
{
	int ret = 0;
	static int get_temp_flag = 0;
	int temp_total = 0;;
	static int last_temp = 0;
	int temp_retry[5];

	if(get_temp_flag == 0)
		last_temp = temp;
	if(temp == -1 || temp >= max || abs(last_temp-temp) >= TEMP_DIFF_RANGE){
		for(int count = 0;count < 5;count++){
			temp_retry[count] = get_temp(CPU_THERMAL);
			if(abs(last_temp-temp_retry[count]) < TEMP_DIFF_RANGE){
				temp = temp_retry[count];
				return temp;
			}else if(temp_retry[count] != -1 && temp_retry[count] < max){
				ret++;
				temp_total += temp_retry[count];
			}
		}
		if(ret == 5){
			syslog(LOG_DEBUG, "Cannot get temp of CPU \n");
			set_all_fans(PWM_MAX);
			return 0;
		}else
			temp = temp_total / ret;

		get_temp_flag = 1;
		last_temp = temp;
	}

	return temp;
}

int check_sw_temp_valid(int max, int temp)
{
	int ret = 0;
	int tmp1 = 0,tmp2 = 0;
	static int get_temp_flag = 0;
	int temp_total = 0;;
	static int last_temp = 0;
	int temp_retry[5];

	if(get_temp_flag == 0)
		last_temp = temp;
	if(temp == -1 || temp >= max || abs(last_temp-temp) >= TEMP_DIFF_RANGE){
		for(int count = 0;count < 5;count++){
			tmp1 = (int)read_register(TEMP_SW_Internal_LSB_FPGA_REG, SYS_FPGA_PATH);
			sleep(0.25);
			tmp2 = (int)read_register(TEMP_SW_Internal_MSB_FPGA_REG, SYS_FPGA_PATH);
			if(tmp1 <= 0 || tmp2 <= 0)
				temp_retry[count] = -1;
			else
				temp_retry[count] = ((434100 - ((12500000 / (tmp2 * 256 + tmp1) - 1) * 535)) + 3000) / TEMP_CONV_FACTOR;
			if(abs(last_temp-temp_retry[count]) < TEMP_DIFF_RANGE){
				temp = temp_retry[count];
				return temp;
			}else if(temp_retry[count] != -1 && temp_retry[count] < max){
				ret++;
				temp_total += temp_retry[count];
			}
		}
		if(ret == 5){
			syslog(LOG_DEBUG, "Cannot get temp of SW \n");
			set_all_fans(PWM_MAX);
			return 0;
		}else
			temp = temp_total / ret;

		get_temp_flag = 1;
		last_temp = temp;
	}

	return temp;
}
uint8_t linear_cal(uint8_t temp, linear_t *l)
{
	uint8_t pwm = 0;
	short ub = 0, lb = 0;

	ub = upper_bound(temp, l->min_temp, l->max_temp, l->min_pwm, l->max_pwm, l->hysteresis);
	lb = lower_bound(temp, l->min_temp, l->max_temp, l->min_pwm, l->max_pwm, l->hysteresis);

	if (-100 == l->lasttemp) {
		l->lasttemp = temp;
		l->lastpwm = lb;
		return lb;
	}
	if (temp >= l->max_temp) {
		l->lasttemp = temp;
		l->lastpwm = PWM_MAX;
		return PWM_MAX;
	} else if (temp < l->min_temp) {
		l->lasttemp = temp;
		l->lastpwm = l->min_pwm;
		return l->min_pwm;
	}

	DEBUG_PRINT("[start] linear temp %d, lasttemp %d\n", temp, l->lasttemp);
	if (temp > l->lasttemp) {
		DEBUG_PRINT("temperature rises from %d to %d\n", l->lasttemp, temp);
		if (lb < l->lastpwm)
			pwm = l->lastpwm;
		else
			pwm = lb;
	} else if (temp < l->lasttemp) {
		DEBUG_PRINT("temperature declines from %d to %d\n", l->lasttemp, temp);
		if (ub > l->lastpwm)
			pwm = l->lastpwm;
		else
			pwm = ub;
	} else {
		DEBUG_PRINT("temperature keeps to %d\n", temp);
		pwm = l->lastpwm;
	}
	if (pwm > l->max_pwm)
		pwm = l->max_pwm;
	else if (pwm < l->min_pwm)
		pwm = l->min_pwm;

	DEBUG_PRINT("[end] last pwm: %d, new pwm: %d\n", l->lastpwm, pwm);
	l->lasttemp = temp;
	l->lastpwm = pwm;

	return pwm;
}

uint8_t pid_cal(uint8_t temp, pid_algo_t *pid)
{
	int pweight = 0;
	int iweight = 0;
	int dweight = 0;
	uint8_t pwmpercent = 0;
	uint8_t pwm = 0;

	if (-100 == pid->lasttemp) {
		pid->lasttemp = temp;
		pid->lastlasttemp = temp;
		pid->lastpwmpercent = PWM_MIN * 100 / PWM_MAX;
		return PWM_MIN;
	}

	pweight = temp - pid->lasttemp;
	iweight = temp - pid->setpoint;
	dweight = temp - 2 * (pid->lasttemp) + pid->lastlasttemp;
	DEBUG_PRINT("[start] pid temp %d, lasttemp %d, lastlasttemp %d\n",
			temp, pid->lasttemp, pid->lastlasttemp);
	DEBUG_PRINT("P %f, I %f, D %f, pweight %d, iweight %d, dweight %d\n",
			pid->P, pid->I, pid->D, pweight, iweight, dweight);

	/* Add 0.5 pwm to support rounding */
	pwmpercent = (uint8_t)(pid->lastpwmpercent +
	pid->P * pweight + pid->I * iweight + pid->D * dweight + 0.5);
	DEBUG_PRINT("pid percent before cal %d\n", pwmpercent);
	pwmpercent = pwmpercent < (PWM_MIN * 100 / PWM_MAX) ? (PWM_MIN * 100 / PWM_MAX) : pwmpercent;
	pwmpercent = pwmpercent > 100 ? 100 : pwmpercent;
	DEBUG_PRINT("pid percent after cal %d\n", pwmpercent);
	pwm = pwmpercent * PWM_MAX / 100;
	pid->lastlasttemp = pid->lasttemp;
	pid->lasttemp = temp;
	pid->lastpwmpercent = pwmpercent;
	DEBUG_PRINT("[end] pid pwm %d\n", pwm);

	return pwm;
}


static uint8_t fan_status;
int update_fan(void)
{
	int ret = 0;
	int fan_direction = 0;
	int tmp1 = 0, tmp2 = 0;
	uint8_t pwm = 0, fan_full_speed_fault_num = 0, fault_num = 0;

	uint8_t fan_out_full_speed_enable = 0;
	uint8_t fan_in_duty_speed_enable = 0;
	uint8_t psu_absent_full_speed_enable = 0;
	uint8_t temper_fail_full_speed_enable = 0;
	uint8_t fan_fault_full_speed_enable = 0;
	uint8_t switch_high_critical_off_enable = 0;
	uint8_t over_temp_full_speed_warn_enable = 0;
	uint8_t fan_in_duty_speed_percent = 70;

	int cpu_temp = 0, sw_temp = 0;
	int xp3r3v_r_temp = 0, xp3r3v_l_temp = 0;
	int xp0r8v_temp = 0, vdd_core_temp = 0;
	int dimma0_temp = 0, dimmb0_temp = 0;
	int psu1_temp1 = 0, psu1_temp2 = 0, psu1_temp3 = 0;
	int psu2_temp1 = 0, psu2_temp2 = 0, psu2_temp3 = 0;
	uint8_t cpu_pwm = 0, sw_pwm = 0, tmp_pwm = 0;

	/* to avoid collect error logs all the time */
    static uint8_t swtemp_flt_flg = 0;
    static uint8_t cputemp_flt_flg = 0;
    static uint8_t xp3r3v_rtemp_flt_flg = 0;
    static uint8_t xp3r3v_ltemp_flt_flg = 0;
    static uint8_t xp0r8vtemp_flt_flg = 0;
    static uint8_t vdd_coretemp_flt_flg = 0;
    static uint8_t dimma0temp_flt_flg = 0;
    static uint8_t dimmb0temp_flt_flg = 0;
    static uint8_t psu1temp1_flt_flg = 0;
    static uint8_t psu1temp2_flt_flg = 0;
    static uint8_t psu1temp3_flt_flg = 0;
    static uint8_t psu2temp1_flt_flg = 0;
    static uint8_t psu2temp2_flt_flg = 0;
    static uint8_t psu2temp3_flt_flg = 0;
    static uint8_t u16temp_flt_flg = 0;
    static uint8_t u17temp_flt_flg = 0;

	fan_out_full_speed_enable = 1;
	fan_in_duty_speed_enable = 0;
	psu_absent_full_speed_enable = 1;
	temper_fail_full_speed_enable = 1;
	fan_fault_full_speed_enable = 1;
	switch_high_critical_off_enable = 1;
	over_temp_full_speed_warn_enable = 1;
	fan_full_speed_fault_num = 2;
	fan_in_duty_speed_percent = 70;

	/* get all temperatures */
	cpu_temp = get_temp(CPU_THERMAL);
	cpu_temp = check_cpu_temp_valid(TEMP_CPU_MAX_VALID,cpu_temp);
	if(cpu_temp == 0)
		return 0;

	// F2B used temperature
	int u16_temp = 0;//LINEAR
	uint8_t  u16_pwm = 0;

	u16_temp = get_temp(TEMP_SW_U16);

	// B2F used temperature
	int u17_temp = 0;//LINEAR
	uint8_t  u17_pwm = 0;
	u17_temp = get_temp(TEMP_FB_U17);

	xp3r3v_r_temp = get_temp(XP3R3V_R_Temp);
	xp3r3v_l_temp = get_temp(XP3R3V_L_Temp);
	xp0r8v_temp = get_temp(XP0R8V_Temp);
	vdd_core_temp = get_temp(VDD_CORE_Temp);
	dimma0_temp = get_temp(DIMMA0_THERMAL);
	dimmb0_temp = get_temp(DIMMB0_THERMAL);

	if(check_psu_sysfs_status(PSU1_ID) == OK){
		psu1_temp1 = get_temp(PSU1_THERMAL1);
		psu1_temp2 = get_temp(PSU1_THERMAL2);
		psu1_temp3 = get_temp(PSU1_THERMAL3);
	}
	if(check_psu_sysfs_status(PSU2_ID) == OK){
		psu2_temp1 = get_temp(PSU2_THERMAL1);
		psu2_temp2 = get_temp(PSU2_THERMAL2);
		psu2_temp3 = get_temp(PSU2_THERMAL3);
	}

	/* get fan1 direction as system fan direction */
	ret = get_sysnode_value(CHASSIS_FAN_DIRECTION(1), (void *)&fan_direction);
	if (ret) {
		printf("Can't get sysnode value\n");
		return ERR;
	}
	if (temper_fail_full_speed_enable) {
		if (ERR == cpu_temp || ERR == u16_temp || ERR == u17_temp) {
			DEBUG_PRINT("fail to get temperature, set full pwm: %d\n\n", PWM_MAX);
			set_all_fans(PWM_MAX);
			return 0;
		}
	}

	tmp1 = (int)read_register(TEMP_SW_Internal_LSB_FPGA_REG, SYS_FPGA_PATH);
	sleep(0.25);
	tmp2 = (int)read_register(TEMP_SW_Internal_MSB_FPGA_REG, SYS_FPGA_PATH);
	//Aligned with thermal and BMC team, need to add 3 Celsius
	if(tmp1 <= 0 || tmp2 <= 0)
		sw_temp = -1;
	else
		sw_temp = ((434100 - ((12500000 / (tmp2 * 256 + tmp1) - 1) * 535)) + 3000) / TEMP_CONV_FACTOR;
	sw_temp = check_sw_temp_valid(TEMP_SW_Internal_MAX_VALID,sw_temp);
	if(sw_temp == 0)
		return 0;

	DEBUG_PRINT("cpu_temp %d\nsw_temp %d\nu17_temp %d\nu16_temp %d\n", cpu_temp, sw_temp, u17_temp, u16_temp);

	/* soft shutdown */
	if (switch_high_critical_off_enable) {
		if (sw_temp >= TEMP_SW_Internal_HI_SHUTDOWN / TEMP_CONV_FACTOR) {
			syslog(LOG_DEBUG, "switch temperature %d is over than %d, soft shutdown !!!\n",
					sw_temp, TEMP_SW_Internal_HI_SHUTDOWN / TEMP_CONV_FACTOR);
			closelog();
			/* switch board power off */
			ret = syscpld_setting(LPC_SWITCH_PWCTRL_REG,SWITCH_OFF);
			if(ret)
				syslog(LOG_DEBUG, "switch board power off failed\n");
			ret = syscpld_setting(LPC_SWITCH_PWCTRL_REG,SWITCH_ON);
			sleep(1);
			if(ret)
				syslog(LOG_DEBUG, "switch board power on failed\n");
			/* Reboot the CPU*/
			syslog(LOG_DEBUG, "Now  reboot the COMe\n");
			system("reboot");
		}
	}

	/* warning and full speed if sensor value is higher than major alarm */
	if (over_temp_full_speed_warn_enable) {
		if (sw_temp >= TEMP_SW_Internal_HI_ERR / TEMP_CONV_FACTOR) {
            if (swtemp_flt_flg == 0)
				syslog(LOG_DEBUG, "SWITCH temperature %d is over than %d !!!\n",
						sw_temp, TEMP_SW_Internal_HI_ERR / TEMP_CONV_FACTOR);
            swtemp_flt_flg = 1;
		}else
        	swtemp_flt_flg = 0;

		if (cpu_temp >= CPU_THERMAL_HI_ERR / TEMP_CONV_FACTOR) {
			if (cputemp_flt_flg == 0)
				syslog(LOG_DEBUG, "CPU temperature %d is over than %d !!!\n",
						cpu_temp, CPU_THERMAL_HI_ERR / TEMP_CONV_FACTOR);
			cputemp_flt_flg = 1;
		}else
			cputemp_flt_flg = 0;
		if (xp3r3v_r_temp >= XP3R3V_R_Temp_HI_ERR / TEMP_CONV_FACTOR) {
			if (xp3r3v_rtemp_flt_flg == 0)
				syslog(LOG_DEBUG, "XP3R3V_R temperature %d is over than %d !!!\n",
						xp3r3v_r_temp, XP3R3V_R_Temp_HI_ERR / TEMP_CONV_FACTOR);
			xp3r3v_rtemp_flt_flg = 1;
		}else
			xp3r3v_rtemp_flt_flg = 0;
		if (xp3r3v_l_temp >= XP3R3V_L_Temp_HI_ERR / TEMP_CONV_FACTOR) {
			if (xp3r3v_ltemp_flt_flg == 0)
				syslog(LOG_DEBUG, "XP3R3V_L temperature %d is over than %d !!!\n",
						xp3r3v_l_temp, XP3R3V_L_Temp_HI_ERR / TEMP_CONV_FACTOR);
			xp3r3v_ltemp_flt_flg = 1;
		}else
			xp3r3v_ltemp_flt_flg = 0;
		if (xp0r8v_temp >= XP0R8V_Temp_HI_ERR / TEMP_CONV_FACTOR) {
			if (xp0r8vtemp_flt_flg == 0)
				syslog(LOG_DEBUG, "xp0r8v temperature %d is over than %d !!!\n",
						xp0r8v_temp, XP0R8V_Temp_HI_ERR / TEMP_CONV_FACTOR);
			xp0r8vtemp_flt_flg = 1;
		}else
			xp0r8vtemp_flt_flg = 0;
		if (vdd_core_temp >= VDD_CORE_Temp_HI_ERR / TEMP_CONV_FACTOR) {
			if (vdd_coretemp_flt_flg == 0)
				syslog(LOG_DEBUG, "VDD CORE temperature %d is over than %d !!!\n",
						vdd_core_temp, VDD_CORE_Temp_HI_ERR / TEMP_CONV_FACTOR);
			vdd_coretemp_flt_flg = 1;
		}else
			vdd_coretemp_flt_flg = 0;
		if (psu1_temp1 >= PSU1_THERMAL1_HI_ERR / TEMP_CONV_FACTOR) {
			if (psu1temp1_flt_flg == 0)
				syslog(LOG_DEBUG, "PSU1 temperature1 %d is over than %d !!!\n",
						psu1_temp1, PSU1_THERMAL1_HI_ERR / TEMP_CONV_FACTOR);
			psu1temp1_flt_flg = 1;
		}else
			psu1temp1_flt_flg = 0;
		if (psu1_temp2 >= PSU1_THERMAL2_HI_ERR / TEMP_CONV_FACTOR) {
			if (psu1temp2_flt_flg == 0)
				syslog(LOG_DEBUG, "PSU1 temperature2 %d is over than %d !!!\n",
						psu1_temp2, PSU1_THERMAL2_HI_ERR / TEMP_CONV_FACTOR);
			psu1temp2_flt_flg = 1;
		}else
			psu1temp2_flt_flg = 0;
		if (psu2_temp1 >= PSU2_THERMAL1_HI_ERR / TEMP_CONV_FACTOR) {
			if (psu2temp1_flt_flg == 0)
				syslog(LOG_DEBUG, "PSU2 temperature1 %d is over than %d !!!\n",
						psu2_temp1, PSU2_THERMAL1_HI_ERR / TEMP_CONV_FACTOR);
			psu2temp1_flt_flg = 1;
		}else
			psu2temp1_flt_flg = 0;
		if (psu2_temp2 >= PSU2_THERMAL2_HI_ERR / TEMP_CONV_FACTOR) {
			if (psu2temp2_flt_flg == 0)
				syslog(LOG_DEBUG, "PSU2 temperature2 %d is over than %d !!!\n",
						psu2_temp2, PSU2_THERMAL2_HI_ERR / TEMP_CONV_FACTOR);
			psu2temp2_flt_flg = 1;
		}else
			psu2temp2_flt_flg = 0;
		if (dimma0_temp >= DIMMA0_THERMAL_HI_ERR / TEMP_CONV_FACTOR) {
			if (dimma0temp_flt_flg == 0)
				syslog(LOG_DEBUG, "DIMMA0 temperature %d is over than %d !!!\n",
						dimma0_temp, DIMMB0_THERMAL_HI_ERR / TEMP_CONV_FACTOR);
			dimma0temp_flt_flg = 1;
		}else if(dimma0_temp == ERR){
				syslog(LOG_DEBUG, "No DIMMA0 temperature !!!Please check if the DIMMA0 is insertrd \n");
				dimma0temp_flt_flg = 1;
		}else
			dimma0temp_flt_flg = 0;
		if (dimmb0_temp >= DIMMB0_THERMAL_HI_ERR / TEMP_CONV_FACTOR) {
			if (dimmb0temp_flt_flg == 0)
				syslog(LOG_DEBUG, "DIMMB0 temperature %d is over than %d !!!\n",
						dimmb0_temp, DIMMA0_THERMAL_HI_ERR / TEMP_CONV_FACTOR);
			dimmb0temp_flt_flg = 1;
		}else if(dimmb0_temp == ERR){
				syslog(LOG_DEBUG, "No DIMMB0 temperature !!!Please check if the DIMMA0 is insertrd \n");
				dimma0temp_flt_flg = 1;
		}else
			dimmb0temp_flt_flg = 0;
		if (FAN_DIR_F2B == fan_direction) {
			if (psu1_temp3 >= PSU1_F2B_THERMAL3_HI_ERR / TEMP_CONV_FACTOR) {
				if (psu1temp3_flt_flg == 0)
					syslog(LOG_DEBUG, "PSU1 temperature3 %d is over than %d !!!\n",
							psu1_temp3, PSU1_F2B_THERMAL3_HI_ERR / TEMP_CONV_FACTOR);
				psu1temp3_flt_flg = 1;
			}else
				psu1temp3_flt_flg = 0;
			if (psu2_temp3 >= PSU2_F2B_THERMAL3_HI_ERR / TEMP_CONV_FACTOR) {
				if (psu1temp3_flt_flg == 0)
					syslog(LOG_DEBUG, "PSU2 temperature3 %d is over than %d !!!\n",
							psu2_temp3, PSU2_F2B_THERMAL3_HI_ERR / TEMP_CONV_FACTOR);
				psu1temp3_flt_flg = 1;
			}else
				psu1temp3_flt_flg = 0;
			if (u16_temp >= TEMP_SW_U16_HI_ERR / TEMP_CONV_FACTOR) {
				if (u16temp_flt_flg == 0)
					syslog(LOG_DEBUG, "SWU16 temperature3 %d is over than %d !!!\n",
							u16_temp, TEMP_SW_U16_HI_ERR / TEMP_CONV_FACTOR);
				u16temp_flt_flg = 1;
			}else
				u16temp_flt_flg = 0;
		} else if (fan_direction == FAN_DIR_B2F) {
			if (psu1_temp3 >= PSU1_B2F_THERMAL3_HI_ERR / TEMP_CONV_FACTOR) {
				if (psu1temp3_flt_flg == 0)
					syslog(LOG_DEBUG, "PSU1 temperature3 %d is over than %d !!!\n",
							psu1_temp3, PSU1_B2F_THERMAL3_HI_ERR / TEMP_CONV_FACTOR);
				psu1temp3_flt_flg = 1;
			}else
				psu1temp3_flt_flg = 0;
			if (psu2_temp3 >= PSU2_B2F_THERMAL3_HI_ERR / TEMP_CONV_FACTOR) {
				if (psu2temp3_flt_flg == 0)
					syslog(LOG_DEBUG, "PSU2 temperature3 %d is over than %d !!!\n",
							psu2_temp3, PSU2_B2F_THERMAL3_HI_ERR / TEMP_CONV_FACTOR);
				psu2temp3_flt_flg = 1;
			}else
				psu2temp3_flt_flg = 0;
			if (u17_temp >= TEMP_FB_U17_HI_ERR / TEMP_CONV_FACTOR) {
				if (u17temp_flt_flg == 0)
					syslog(LOG_DEBUG, "FBU17 temperature3 %d is over than %d !!!\n",
							u17_temp, TEMP_FB_U17_HI_ERR / TEMP_CONV_FACTOR);
				u17temp_flt_flg = 1;
			}else
				u17temp_flt_flg = 0;
		}
	}

	ret = check_psu_status();
	if (OK != ret && psu_absent_full_speed_enable) {
	    DEBUG_PRINT("psu absent, set full pwm: %d\n\n", PWM_MAX);
	    set_all_fans(PWM_MAX);
		return 0;
	}
	ret = check_fan_status(&fault_num);
	if (ABSENT == ret && fan_out_full_speed_enable) {
		DEBUG_PRINT("fan absent, set full pwm: %d\n\n", PWM_MAX);
		set_all_fans(PWM_MAX);
		fan_status = ABSENT;
	} else if (ABSENT != ret && ABSENT == fan_status && fan_in_duty_speed_enable) {
		DEBUG_PRINT("fan plug in, set duty pwm: %d\n\n", fan_in_duty_speed_percent * 255 / 100);
		set_all_fans(fan_in_duty_speed_percent * 255 / 100);
		fan_status = PRESENT;
	} else if (FAULT == ret && fan_full_speed_fault_num == fault_num && fan_fault_full_speed_enable) {
		DEBUG_PRINT("fan fault, set full pwm: %d\n\n", PWM_MAX);
		set_all_fans(PWM_MAX);
		return 0;
	} else {
		/* calculate all PWMs */
		cpu_pwm = pid_cal(cpu_temp, &CPU_PID);
		sw_pwm  = pid_cal(sw_temp, &SW_PID);
		if (FAN_DIR_F2B == fan_direction)
			tmp_pwm = u16_pwm = linear_cal(u16_temp, &U16LINEAR_F2B);
		else if (fan_direction == FAN_DIR_B2F)
			tmp_pwm = u17_pwm = linear_cal(u17_temp, &U17LINEAR_B2F);
		/* get max PWM */
		pwm = cpu_pwm > sw_pwm ? cpu_pwm : sw_pwm;
		pwm = pwm > tmp_pwm ? pwm : tmp_pwm;
		pwm = pwm > PWM_MAX ? PWM_MAX : pwm;
		/* set max PWM to all fans */
		DEBUG_PRINT("set normal pwm: %d\n\n", pwm);
		set_all_fans(pwm);
	}
	return 0;
}
