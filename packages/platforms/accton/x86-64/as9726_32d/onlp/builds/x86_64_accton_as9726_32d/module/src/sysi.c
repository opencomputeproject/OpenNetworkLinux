/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include <onlp/platformi/sfpi.h>
#include "platform_lib.h"
#include "x86_64_accton_as9726_32d_int.h"
#include "x86_64_accton_as9726_32d_log.h"

#define NUM_OF_FAN_ON_MAIN_BROAD    6
#define PREFIX_PATH_ON_CPLD_DEV    "/sys/bus/i2c/devices/"
#define NUM_OF_CPLD		   3
#define FAN_DUTY_CYCLE_MAX         (100)
#define FAN_DUTY_CYCLE_DEFAULT     (FAN_DUTY_CYCLE_MAX)
/* Number of sensor points considered by the fan/thermal policy */
#define NUM_THERMAL_POLICY_SENSORS   7

static char arr_cplddev_name[NUM_OF_CPLD][10] =
{
	"1-0060",
	"10-0061",
	"10-0062"
};

const char* onlp_sysi_platform_get(void)
{
	return "x86-64-accton-as9726-32d-r0";
}

int onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
	uint8_t* rdata = aim_zmalloc(256);

	if (onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) {
		if (*size == 256) {
			*data = rdata;
			return ONLP_STATUS_OK;
		}
	}

	aim_free(rdata);
	*size = 0;
	return ONLP_STATUS_E_INTERNAL;
}

int onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
	int i;
	onlp_oid_t* e = table;
	memset(table, 0, max*sizeof(onlp_oid_t));

	/* 7 Thermal sensors on the chassis */
	for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++)
		*e++ = ONLP_THERMAL_ID_CREATE(i);

	/* 5 LEDs on the chassis */
	for (i = 1; i <= CHASSIS_LED_COUNT; i++)
		*e++ = ONLP_LED_ID_CREATE(i);

	/* 2 PSUs on the chassis */
	for (i = 1; i <= CHASSIS_PSU_COUNT; i++)
		*e++ = ONLP_PSU_ID_CREATE(i);

	/* 6 Fans on the chassis */
	for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
		*e++ = ONLP_FAN_ID_CREATE(i);

	return 0;
}

int onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
	int   i, v[NUM_OF_CPLD]={0};

	for (i = 0; i < NUM_OF_CPLD; i++) {
		v[i] = 0;

		if (onlp_file_read_int(v+i, "%s%s/version", 
				      PREFIX_PATH_ON_CPLD_DEV,
				      arr_cplddev_name[i]) < 0) {
			return ONLP_STATUS_E_INTERNAL;
		}
	}
	pi->cpld_versions = aim_fstrdup("%d.%d.%d", v[0], v[1], v[2]);

	return 0;
}

void onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
	aim_free(pi->cpld_versions);
}

int onlp_sysi_platform_manage_leds(void)
{
	return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 Thermal policy:
 a.Defaut fan duty_cycle=100%
 b.One fan fail, set to fan duty_cycle=100%
 1.For AFI:
   Default fan duty_cycle will be 100%(fan_policy_state=LEVEL_FAN_MAX).
   If all below case meet with, set to 75%(LEVEL_FAN_MID).
   MB board
   (MB board)
   LM75-1(0X48)>=49.5
   LM75-2(0X49)>=42.9
   LM75-3(0X4A)>=46.3
   LM75-4(0X4C)>=40.1
   LM75-6(0X4F)>=39.4
   (CPU board)
   Core>=46
   LM75-1(0X4B)>=34.8

   When fan_policy_state=LEVEL_FAN_MID, meet with below case,  Fan duty_cycle will be 100%(LEVEL_FAN_DAX)
   (MB board)
   LM75-1(0X48)>=55.9
   LM75-2(0X49)>=48.8
   LM75-3(0X4A)>=51.5
   LM75-4(0X4C)>=45.3
   LM75-6(0X4F)>=43.4
   (CPU board)
   Core>=50
   LM75-1(0X4B)>=43.4
   Transceiver >=65

   Yellow Alarm
   MB board
   LM75-1(0X48)>=57.9
   LM75-2(0X49)>=51.9
   LM75-3(0X4A)>=48.9
   LM75-4(0X4C)>=55.9
   LM75-6(0X4F)>=48.5
   CPU Board
   Core>=52
   LM75-1(0X4B)>=41.8
   Transceiver >=73

   Red Alarm
   MB board
   LM75-1(0X48)>=62.9
   LM75-2(0X49)>=56.9
   LM75-3(0X4A)>=53.9
   LM75-4(0X4C)>=58.9
   LM75-6(0X4F)>=53.5
   CPU Board
   Core>=57
   LM75-1(0X4B)>=46.8
   Transceiver >=75

   Shutdown
   MB board
   LM75-1(0X48)>=67.9
   LM75-2(0X49)>=61.9
   LM75-3(0X4A)>=58.9
   LM75-4(0X4C)>=63.9
   LM75-6(0X4F)>=58.5
   CPU Board
   Core>=62
   LM75-1(0X4B)>=51.8
   Transceiver >=77

 2. For AFO:
   At default, FAN duty_cycle was 100%(LEVEL_FAN_MAX). If all below case meet with, set to 75%(LEVEL_FAN_MID).
   (MB board)
   LM75-1(0X48)<=56
   LM75-2(0X49)<=53.5
   LM75-3(0X4A)<=52.5
   LM75-4(0X4C)<=52
   LM75-6(0X4F)<=52.8
   (CPU board)
   Core<=62
   LM75-1(0X4B)<=45.8

   When FAN duty_cycle was 75%(LEVEL_FAN_MID). If all below case meet with, set to 50%.
   (MB board)
   LM75-1(0X48)<=50
   LM75-2(0X49)<=47.3
   LM75-3(0X4A)<=46.4
   LM75-4(0X4C)<=44.6
   LM75-6(0X4F)<=47
   (CPU board)
   Core<=56
   LM75-1(0X4B)<=38.8

   When fan_speed 50%.
   Meet with below case, Fan duty_cycle will be 75%(LEVEL_FAN_MID)
   (MB board)
   LM75-1(0X48)>=63
   LM75-2(0X49)>=60.5
   LM75-3(0X4A)>=60
   LM75-4(0X4C)>=60
   LM75-6(0X4F)>=61
   (CPU board)
   Core>=72
   LM75-1(0X4B)>=50
   Transceiver >=55

   When FAN duty_cycle was 75%(LEVEL_FAN_MID). If all below case meet with, set to 100%(LEVEL_FAN_MAX).
   (MB board)
   LM75-1(0X48)>=63
   LM75-2(0X49)>=60
   LM75-3(0X4A)>=60
   LM75-4(0X4C)>=59
   LM75-6(0X4F)>=60

   (CPU board)
   Core >=69
   LM75-1(0X4B)>=51.5
   Transceiver >=65

   Yellow Alarm
   MB board
   LM75-1(0X48)>=67
   LM75-2(0X49)>=65
   LM75-3(0X4A)>=64
   LM75-4(0X4C)>=62
   LM75-6(0X4F)>=64
   CPU Board
   Core>=73
   LM75-1(0X4B)>=67
   Transceiver >=73

   Red Alarm
   MB board
   LM75-1(0X48)>=72
   LM75-2(0X49)>=70
   LM75-3(0X4A)>=69
   LM75-4(0X4C)>=67
   LM75-6(0X4F)>=69
   CPU Board
   Core>=78
   LM75-1(0X4B)>=72
   Transceiver >=75

   Shutdown
   MB board
   LM75-1(0X48)>=77
   LM75-2(0X49)>=75
   LM75-3(0X4A)>=74
   LM75-4(0X4C)>=72
   LM75-6(0X4F)>=74
   CPU Board
   Core>=83
   LM75-1(0X4B)>=77
   Transceiver >=77
*/

/**
 * Fan direction enum
 * AFI: Air Flow In
 * AFO: Air Flow Out
 */
typedef enum {
	FAN_DIR_AFI     = 0,
	FAN_DIR_AFO     = 1,
	FAN_DIR_MAX     = 2
} fan_direction_t;

/* Fan Policy States */
typedef enum {
	LEVEL_FAN_INIT = 0,
	LEVEL_FAN_MIN = 1,
	LEVEL_FAN_MID = 2,
	LEVEL_FAN_MAX = 3,
	LEVEL_FAN_YELLOW_ALARM = 4,
	LEVEL_FAN_RED_ALARM = 5,
	LEVEL_FAN_SHUTDOWN = 6
} fan_policy_level_t;

/* Sensor Types */
typedef enum {
	TYPE_SENSOR = 0,
	TYPE_TRANSCEIVER = 1,
	TYPE_MAX = 2
} sensor_type_t;

/* Monitor ports for ZR/ZR+ transceivers */
static int monitor_ports[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31};

/* Fan Policy Configuration */
typedef struct {
	int duty_cycle;
	int pwm_value;
} fan_policy_config_t;

static fan_policy_config_t fan_policy_f2b[] = {  /* AFO */
	[LEVEL_FAN_MIN] = {50, 0x7},
	[LEVEL_FAN_MID] = {75, 0xb},
	[LEVEL_FAN_MAX] = {100, 0xf}
};

static fan_policy_config_t fan_policy_b2f[] = {  /* AFI */
	[LEVEL_FAN_MID] = {75, 0xb},
	[LEVEL_FAN_MAX] = {100, 0xf}
};

/* Thermal Specification Structure */
typedef struct {
	int threshold;
} thermal_threshold_t;

/* Complete Thermal Specification for each sensor/transceiver */
typedef struct {
	onlp_oid_t id;
	thermal_threshold_t min_to_mid;		/* For AFO only */
	thermal_threshold_t mid_to_max;
	thermal_threshold_t max_to_mid;
	thermal_threshold_t mid_to_min;		/* For AFO only */
	thermal_threshold_t max_to_yellow;
	thermal_threshold_t yellow_to_red;
	thermal_threshold_t red_to_shutdown;
} thermal_spec_t;


static thermal_spec_t thermal_specs[FAN_DIR_MAX][TYPE_MAX][NUM_THERMAL_POLICY_SENSORS] = {
	/* AFI */
	[FAN_DIR_AFI] = {
		[TYPE_SENSOR] = {
			/* LM75-1 (0x48) - MB board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD),
				.min_to_mid = {0},      /* AFI doesn't use min state */
				.mid_to_max = {55900},
				.max_to_mid = {49500},
				.mid_to_min = {0},      /* AFI doesn't use min state */
				.max_to_yellow = {57900},
				.yellow_to_red = {62900},
				.red_to_shutdown = {67900}
			},
			/* LM75-2 (0x49) - MB board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD),
				.min_to_mid = {0},
				.mid_to_max = {48800},
				.max_to_mid = {42900},
				.mid_to_min = {0},
				.max_to_yellow = {51900},
				.yellow_to_red = {56900},
				.red_to_shutdown = {61900}
			},
			/* LM75-3 (0x4A) - MB board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD),
				.min_to_mid = {0},
				.mid_to_max = {51500},
				.max_to_mid = {46300},
				.mid_to_min = {0},
				.max_to_yellow = {48900},
				.yellow_to_red = {53900},
				.red_to_shutdown = {58900}
			},
			/* LM75-4 (0x4C) - MB board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD),
				.min_to_mid = {0},
				.mid_to_max = {45300},
				.max_to_mid = {40100},
				.mid_to_min = {0},
				.max_to_yellow = {55900},
				.yellow_to_red = {58900},
				.red_to_shutdown = {63900}
			},
			/* LM75-6 (0x4F) - MB board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD),
				.min_to_mid = {0},
				.mid_to_max = {43400},
				.max_to_mid = {39400},
				.mid_to_min = {0},
				.max_to_yellow = {48500},
				.yellow_to_red = {53500},
				.red_to_shutdown = {58500}
			},
			/* Core - CPU board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE),
				.min_to_mid = {0},
				.mid_to_max = {50000},
				.max_to_mid = {46000},
				.mid_to_min = {0},
				.max_to_yellow = {52000},
				.yellow_to_red = {57000},
				.red_to_shutdown = {62000}
			},
			/* LM75-1 (0x4B) - CPU board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD),
				.min_to_mid = {0},
				.mid_to_max = {43400},
				.max_to_mid = {34800},
				.mid_to_min = {0},
				.max_to_yellow = {41800},
				.yellow_to_red = {46800},
				.red_to_shutdown = {51800}
			}
		},
		[TYPE_TRANSCEIVER] = {
			{
				.id = 0,
				.min_to_mid = {0},
				.mid_to_max = {65000},
				.max_to_mid = {64000},
				.mid_to_min = {0},
				.max_to_yellow = {73000},
				.yellow_to_red = {75000},
				.red_to_shutdown = {77000}
			}
		}
	},
	/* AFO */
	[FAN_DIR_AFO] = {
		[TYPE_SENSOR] = {
			/* LM75-1 (0x48) - MB board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD),
				.min_to_mid = {63000},
				.mid_to_max = {63000},
				.max_to_mid = {56000},
				.mid_to_min = {50000},
				.max_to_yellow = {67000},
				.yellow_to_red = {72000},
				.red_to_shutdown = {77000}
			},
			/* LM75-2 (0x49) - MB board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD),
				.min_to_mid = {60500},
				.mid_to_max = {60000},
				.max_to_mid = {53500},
				.mid_to_min = {47300},
				.max_to_yellow = {65000},
				.yellow_to_red = {70000},
				.red_to_shutdown = {75000}
			},
			/* LM75-3 (0x4A) - MB board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD),
				.min_to_mid = {60000},
				.mid_to_max = {60000},
				.max_to_mid = {52500},
				.mid_to_min = {46400},
				.max_to_yellow = {64000},
				.yellow_to_red = {69000},
				.red_to_shutdown = {74000}
			},
			/* LM75-4 (0x4C) - MB board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD),
				.min_to_mid = {60000},
				.mid_to_max = {59000},
				.max_to_mid = {52000},
				.mid_to_min = {44600},
				.max_to_yellow = {62000},
				.yellow_to_red = {67000},
				.red_to_shutdown = {72000}
			},
			/* LM75-6 (0x4F) - MB board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD),
				.min_to_mid = {61000},
				.mid_to_max = {60000},
				.max_to_mid = {52800},
				.mid_to_min = {47000},
				.max_to_yellow = {64000},
				.yellow_to_red = {69000},
				.red_to_shutdown = {74000}
			},
			/* Core - CPU board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE),
				.min_to_mid = {72000},
				.mid_to_max = {69000},
				.max_to_mid = {62000},
				.mid_to_min = {56000},
				.max_to_yellow = {73000},
				.yellow_to_red = {78000},
				.red_to_shutdown = {83000}
			},
			/* LM75-1 (0x4B) - CPU board */
			{
				.id = ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD),
				.min_to_mid = {50000},
				.mid_to_max = {51500},
				.max_to_mid = {45800},
				.mid_to_min = {38800},
				.max_to_yellow = {67000},
				.yellow_to_red = {72000},
				.red_to_shutdown = {77000}
			}
		},
		[TYPE_TRANSCEIVER] = {
			{
				.id = 0,
				.min_to_mid = {55000},
				.mid_to_max = {65000},
				.max_to_mid = {64000},
				.mid_to_min = {54000},
				.max_to_yellow = {73000},
				.yellow_to_red = {75000},
				.red_to_shutdown = {77000}
			}
		}
	}
};

/* State machine API for thermal management */
typedef struct {
	sensor_type_t type;
	int current_temp;
	int port;
	thermal_spec_t *spec;
	char sensor_name[ONLP_OID_DESC_SIZE];
} thermal_sensor_data_t;

typedef struct {
	int ori_state;
	int new_state;
	int max_to_mid_count;
	int mid_to_min_count;
	int alarm;
} thermal_state_result_t;

typedef struct thermal_sensor_ops {
	const char* name;
	int (*get_temp)(int index, fan_direction_t fan_dir, thermal_sensor_data_t *data);
	int (*get_count)(fan_direction_t dir);
	bool (*should_process)(int index);
	bool (*is_critical_error)(int error_code);  /* Determine if error is critical */
} thermal_sensor_ops_t;

typedef struct {
	thermal_sensor_ops_t *ops;
	sensor_type_t type;
} sensor_handler_t;

void onlp_sysi_over_temp_protector(void)
{
	int fpga_ver = 0;
	int i;

	for (i = 0; i < AIM_ARRAYSIZE(monitor_ports); i++) {
		onlp_sfpi_control_set(monitor_ports[i] - 1, ONLP_SFP_CONTROL_RESET, 1);
	}

	if (onlp_file_read_int(&fpga_ver, "%s%s/version",
				PREFIX_PATH_ON_CPLD_DEV, "1-0060") < 0) {
		AIM_LOG_ERROR("Can not get the FPGA version. (%s%s/version)", 
				PREFIX_PATH_ON_CPLD_DEV, "1-0060");
	}

	system("sync;sync;sync;");
	system("/sbin/fstrim -av");
	if (fpga_ver >= 9) {
			/* Power off main board & Power off cpu */
			system("i2cset -y -f 1 0x60 0x60 0x11 & i2cset -y -f 1 0x65 0x07 0x2c");
	} else {
			/* Power-cycle dut */
			system("i2cset -y -f 1 0x60 0x60 0x10");
	}
}

/* Get fan direction: 1 for B2F (AFI), 2 for F2B (AFO) */
static fan_direction_t onlp_sysi_get_fan_direction(void)
{
	onlp_fan_info_t fan_info;
	int b2f_count = 0, f2b_count = 0;
	int i, rv;

	for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BROAD; i++) {
		rv = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info);
		if (rv < 0) {
			continue;
		}

		if (!(fan_info.status & ONLP_FAN_STATUS_PRESENT)) {
			continue;
		}

		if (fan_info.status & ONLP_FAN_STATUS_B2F) {
			b2f_count++;
		} else {
			f2b_count++;
		}
	}

	return (b2f_count >= f2b_count) ? FAN_DIR_AFI : FAN_DIR_AFO;
}

int onlp_sysi_get_fan_status(void)
{
	int i, ret;
	onlp_fan_info_t fi[CHASSIS_FAN_COUNT];
	memset(fi, 0, sizeof(fi));

	for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
		ret = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i+1), &fi[i]);
		if (ret != ONLP_STATUS_OK) {
			AIM_LOG_ERROR("Unable to get '%s' status", fi[i].hdr.description);
			return ONLP_STATUS_E_INTERNAL;
		}

		if (!(fi[i].status & ONLP_FAN_STATUS_PRESENT)) {
			AIM_LOG_ERROR("'%s' is NOT present", fi[i].hdr.description);
			return ONLP_STATUS_E_INTERNAL;
		}

		if (fi[i].status & ONLP_FAN_STATUS_FAILED) {
			AIM_LOG_ERROR("'%s' is NOT operational", fi[i].hdr.description);
			return ONLP_STATUS_E_INTERNAL;
		}
	}

	return ONLP_STATUS_OK;
}

int onlp_sysi_set_fan_duty_all(int duty)
{
	int fid, ret = ONLP_STATUS_OK;

	for (fid = 1; fid <= CHASSIS_FAN_COUNT; fid++) {
		if (ONLP_STATUS_OK != onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(fid), duty)) {
			ret = ONLP_STATUS_E_INTERNAL;
		}
	}

	return ret;
}

int onlp_sysi_get_sff8436_temp(int port, int *temp)
{
	int value;
	int16_t port_temp;

	/* Read memory model */
	value = onlp_sfpi_dev_readb(port, 0x50, 0x2);
	if (value & 0x04) {
		*temp = ONLP_STATUS_E_MISSING;
		return ONLP_STATUS_OK;
	}

	value = onlp_sfpi_dev_readb(port, 0x50, 22);
	if (value < 0) {
		*temp = ONLP_STATUS_E_MISSING;
		return ONLP_STATUS_OK;
	}
	port_temp = (int16_t)((value & 0xFF) << 8);

	value = onlp_sfpi_dev_readb(port, 0x50, 23);
	if (value < 0) {
		*temp = ONLP_STATUS_E_MISSING;
		return ONLP_STATUS_OK;
	}
	port_temp = (port_temp | (int16_t)(value & 0xFF));

	*temp = (int)port_temp * 1000 / 256;
	return ONLP_STATUS_OK;
}

int onlp_sysi_get_cmis_temp(int port, int *temp)
{
	int value;
	int16_t port_temp;

	/* Read memory model */
	value = onlp_sfpi_dev_readb(port, 0x50, 0x2);
	if (value & 0x80) {
		*temp = ONLP_STATUS_E_MISSING;
		return ONLP_STATUS_OK;
	}

	value = onlp_sfpi_dev_readb(port, 0x50, 14);
	if (value < 0) {
		*temp = ONLP_STATUS_E_MISSING;
		return ONLP_STATUS_OK;
	}
	port_temp = (int16_t)((value & 0xFF) << 8);

	value = onlp_sfpi_dev_readb(port, 0x50, 15);
	if (value < 0) {
		*temp = ONLP_STATUS_E_MISSING;
		return ONLP_STATUS_OK;
	}
	port_temp = (port_temp | (int16_t)(value & 0xFF));

	*temp = (int)port_temp * 1000 / 256;
	return ONLP_STATUS_OK;
}

int onlp_sysi_get_xcvr_temp(int port)
{
	int ret = ONLP_STATUS_OK;
	int current_temp = 0;
	int value;

	/* Skip if transceiver not present */
	if (onlp_sfpi_is_present(port) != 1) {
		return ONLP_STATUS_E_MISSING;
	}

	value = onlp_sfpi_dev_readb(port, 0x50, 0);
	if (value < 0) {
		AIM_LOG_ERROR("Unable to get read 'Port %02d' eeprom", port);
		return ONLP_STATUS_E_MISSING;
	}

	if (value == 0x18 || value == 0x19 || value == 0x1E) {
		ret = onlp_sysi_get_cmis_temp(port, &current_temp);
		if (ret == ONLP_STATUS_OK) {
			return current_temp;
		}
	}
	else if (value == 0x0C || value == 0x0D || value == 0x11 || value ==  0xE1) {
		ret = onlp_sysi_get_sff8436_temp(port, &current_temp);
		if (ret == ONLP_STATUS_OK) {
			return current_temp;
		}
	}

	return ONLP_STATUS_E_MISSING;
}

static int sensor_get_temp(int index, fan_direction_t fan_dir, thermal_sensor_data_t *data)
{
	int ret;
	onlp_thermal_info_t thermal_info;

	data->spec = &thermal_specs[fan_dir][TYPE_SENSOR][index];
	ret = onlp_thermali_info_get(data->spec->id, &thermal_info);

	if (ret != ONLP_STATUS_OK) {
		AIM_SYSLOG_WARN("Temperature warning", "Temperature warning", 
				"Failed to read temperature from sensor '%s' (error: %d)", 
				thermal_info.hdr.description, ret);
		return ret;
	}

	data->type = TYPE_SENSOR;
	data->current_temp = thermal_info.mcelsius;
	snprintf(data->sensor_name, sizeof(data->sensor_name), "%s", thermal_info.hdr.description);

	return ONLP_STATUS_OK;
}

static int sensor_get_count(fan_direction_t dir)
{
	return AIM_ARRAYSIZE(thermal_specs[dir][TYPE_SENSOR]);
}

static bool sensor_should_process(int index)
{
	return true; /* Process all sensors */
}

/* Sensor critical error check: any error is critical */
static bool sensor_is_critical_error(int error_code)
{
	return (error_code != ONLP_STATUS_OK);
}

/* TYPE_TRANSCEIVER implementation */
static int transceiver_get_temp(int index, fan_direction_t fan_dir, thermal_sensor_data_t *data)
{
	int current_temp;
	int port;

	port = monitor_ports[index] - 1;
	current_temp = onlp_sysi_get_xcvr_temp(port);
	if (current_temp == ONLP_STATUS_E_MISSING) {
		return ONLP_STATUS_E_MISSING;
	}

	data->type = TYPE_TRANSCEIVER;
	data->spec = &thermal_specs[fan_dir][TYPE_TRANSCEIVER][0];
	data->port = port;
	data->current_temp = current_temp;
	snprintf(data->sensor_name, sizeof(data->sensor_name), "Port %02d", port);

	return ONLP_STATUS_OK;
}

static int transceiver_get_count(fan_direction_t dir)
{
	return AIM_ARRAYSIZE(monitor_ports);
}

static bool transceiver_should_process(int index)
{
	return true;
}

/* Transceiver critical error check: only non-E_MISSING errors are critical */
static bool transceiver_is_critical_error(int error_code)
{
	/* E_MISSING (not present or temporary issue) is not critical */
	/* Other errors (e.g., E_INTERNAL) are critical */
	return (error_code != ONLP_STATUS_OK && error_code != ONLP_STATUS_E_MISSING);
}

/* Sensor operation structures */
static thermal_sensor_ops_t sensor_ops = {
	.name = "Thermal Sensor",
	.get_temp = sensor_get_temp,
	.get_count = sensor_get_count,
	.should_process = sensor_should_process,
	.is_critical_error = sensor_is_critical_error
};

static thermal_sensor_ops_t transceiver_ops = {
	.name = "Transceiver",
	.get_temp = transceiver_get_temp,
	.get_count = transceiver_get_count,
	.should_process = transceiver_should_process,
	.is_critical_error = transceiver_is_critical_error
};

/* Registered handlers */
static sensor_handler_t sensor_handlers[] = {
	{ .ops = &sensor_ops, .type = TYPE_SENSOR },
	{ .ops = &transceiver_ops, .type = TYPE_TRANSCEIVER }
};

/* State Machine API */
void process_thermal_state(thermal_sensor_data_t *sensor_data, thermal_state_result_t *result)
{
	int current_temp = 0;
	thermal_spec_t *spec = NULL;
	const char *sensor_name = NULL;

	if (!sensor_data || !result) {
		return;
	}

	current_temp = sensor_data->current_temp;
	spec = sensor_data->spec;
	sensor_name = sensor_data->sensor_name;

	if (current_temp == 0) return;  /* Skip invalid readings */

	/* State machine logic */
	if (result->ori_state == LEVEL_FAN_MIN) {
		if (current_temp >= spec->min_to_mid.threshold) {
			result->new_state = LEVEL_FAN_MID;
		}
	} else if (result->ori_state == LEVEL_FAN_MID) {
		if (current_temp <= spec->mid_to_min.threshold) {
			result->mid_to_min_count += 1;
		} else if (current_temp >= spec->mid_to_max.threshold) {
			result->new_state = LEVEL_FAN_MAX;
			AIM_SYSLOG_WARN("Temperature warning", "Temperature warning", 
					"%s: %.1f°C exceeds mid->max threshold %.1f°C", 
					sensor_name, current_temp/1000.0, spec->mid_to_max.threshold/1000.0);
		}
	} else {  /* LEVEL_FAN_MAX */
		/* Check for temperature reduction */
		if (current_temp <= spec->max_to_mid.threshold) {
			result->max_to_mid_count += 1;
		}

		/* Check alarm conditions */
		if (result->alarm == 0) {
			if (current_temp >= spec->max_to_yellow.threshold) {
				result->alarm = LEVEL_FAN_YELLOW_ALARM;
				AIM_SYSLOG_WARN("Temperature warning", "Temperature warning", 
						"Yellow alarm - %s: %.1f°C exceeds %.1f°C", 
						sensor_name, current_temp/1000.0, spec->max_to_yellow.threshold/1000.0);
			}
		} else if (result->alarm == LEVEL_FAN_YELLOW_ALARM) {
			if (current_temp >= spec->yellow_to_red.threshold) {
				result->alarm = LEVEL_FAN_RED_ALARM;
				AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical",
						"Red alarm - %s: %.1f°C exceeds %.1f°C", 
						sensor_name, current_temp/1000.0, spec->yellow_to_red.threshold/1000.0);
			}
		} else if (result->alarm == LEVEL_FAN_RED_ALARM) {
			if (current_temp >= spec->red_to_shutdown.threshold) {
				result->alarm = LEVEL_FAN_SHUTDOWN;
				if (sensor_data->type == TYPE_SENSOR) {
					AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical", 
							"SHUTDOWN - %s: %.1f°C exceeds %.1f°C - powering off", 
							sensor_name, current_temp/1000.0, 
							spec->red_to_shutdown.threshold/1000.0);
					onlp_sysi_over_temp_protector();
				} else if (sensor_data->type == TYPE_TRANSCEIVER) {
					AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical", 
							"SHUTDOWN - %s: %.1f°C exceeds %.1f°C - reset", 
							sensor_name, current_temp/1000.0, 
							spec->red_to_shutdown.threshold/1000.0);
					onlp_sfpi_control_set(sensor_data->port, ONLP_SFP_CONTROL_RESET, 1);
					result->alarm = 0;
				}
			}
		}
	}
}

/* Generic sensor processing function */
static int process_sensor_type(sensor_handler_t *handler, 
				fan_direction_t fan_dir,
				thermal_state_result_t *result,
				int *processed_count)
{
	int i, count;
	thermal_sensor_ops_t *ops = handler->ops;

	count = ops->get_count(fan_dir);
	*processed_count = 0;

	for (i = 0; i < count; i++) {
		thermal_sensor_data_t sensor_data = {0};
		int ret;

		if (!ops->should_process(i)) {
			continue;
		}

		ret = ops->get_temp(i, fan_dir, &sensor_data);

		if (ret != ONLP_STATUS_OK) {
			if (ops->is_critical_error(ret)) {
				return ret;
			} else {
				continue;
			}
		}

		(*processed_count)++;

		/* Process temperature state */
		process_thermal_state(&sensor_data, result);

		if (result->new_state != result->ori_state) {
			return ONLP_STATUS_OK;
		}
	}

	return ONLP_STATUS_OK;
}

/* Simplified main function */
int onlp_sysi_platform_manage_fans(void)
{
	static int current_state = LEVEL_FAN_MAX;
	static bool initialized = false;
	static int persistent_alarm = 0;
	fan_direction_t fan_dir;
	thermal_state_result_t result = {0};
	int i, total_processed = 0;

	/* Check fan status */
	if (onlp_sysi_get_fan_status() != ONLP_STATUS_OK) {
		current_state = LEVEL_FAN_MAX;
		onlp_sysi_set_fan_duty_all(FAN_DUTY_CYCLE_MAX);
		AIM_SYSLOG_WARN("Temperature warning", "Temperature warning", 
				"Fan failure detected, setting maximum speed");
		return ONLP_STATUS_OK;
	}

	/* Initialize */
	if (!initialized) {
		onlp_sysi_set_fan_duty_all(FAN_DUTY_CYCLE_MAX);
		AIM_SYSLOG_INFO("Temperature info", "Temperature info", 
				"Initializing fan to %d%%", FAN_DUTY_CYCLE_MAX);
		initialized = true;
	}

	/* Get fan direction */
	fan_dir = onlp_sysi_get_fan_direction();

	/* Initialize result */
	result.ori_state = current_state;
	result.new_state = current_state;
	result.alarm = persistent_alarm;

	/* Process all sensor types */
	for (i = 0; i < AIM_ARRAYSIZE(sensor_handlers); i++) {
		int processed;
		int ret;

		ret = process_sensor_type(&sensor_handlers[i], fan_dir, &result, &processed);
		if (ret != ONLP_STATUS_OK) {
			current_state = LEVEL_FAN_MAX;
			onlp_sysi_set_fan_duty_all(FAN_DUTY_CYCLE_MAX);
			return ONLP_STATUS_OK;
		}

		total_processed += processed;

		if (result.new_state != current_state) {
			break;
		}
	}

	/* Check state transitions */
	if (result.max_to_mid_count == total_processed && current_state == LEVEL_FAN_MAX) {
		result.new_state = LEVEL_FAN_MID;
		if (result.alarm != 0) {
			result.alarm = 0;
			AIM_SYSLOG_INFO("Temperature info", "Temperature info", "Temperature alarms cleared");
		}
	}

	if (fan_dir == FAN_DIR_AFO) {
		if (result.mid_to_min_count == total_processed && current_state == LEVEL_FAN_MID) {
			result.new_state = LEVEL_FAN_MIN;
			AIM_SYSLOG_INFO("Temperature info", "Temperature info", 
					"All temperatures low - transitioning to MIN");
		}
	}

	persistent_alarm = result.alarm;

	/* Apply fan policy changes */
	if (current_state != result.new_state) {
		fan_policy_config_t *fan_policy;

		fan_policy = (fan_dir == FAN_DIR_AFI) ? fan_policy_b2f : fan_policy_f2b;

		if (result.new_state > result.ori_state) {
			AIM_SYSLOG_WARN("Temperature warning", "Temperature warning", 
					"Increasing fan duty cycle from %d%% to %d%%", 
					fan_policy[result.ori_state].duty_cycle, 
					fan_policy[result.new_state].duty_cycle);
		} else {
			AIM_SYSLOG_INFO("Temperature info", "Temperature info", 
					"Decreasing fan duty cycle from %d%% to %d%%", 
					fan_policy[result.ori_state].duty_cycle, 
					fan_policy[result.new_state].duty_cycle);
		}
		onlp_sysi_set_fan_duty_all(fan_policy[result.new_state].duty_cycle);

		current_state = result.new_state;
	}

	return ONLP_STATUS_OK;
}
