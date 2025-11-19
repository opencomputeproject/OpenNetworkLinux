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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
	do {                                        \
		if(!ONLP_OID_IS_THERMAL(_id)) {         \
			return ONLP_STATUS_E_INVALID;       \
		}                                       \
	} while(0)

static char* devfiles__[] = { /* must map with onlp_thermal_id */
	NULL,
	NULL,                  /* CPU_CORE files */
	"/sys/devices/platform/as9737_32db_thermal*temp1_input",
	"/sys/devices/platform/as9737_32db_thermal*temp2_input",
	"/sys/devices/platform/as9737_32db_thermal*temp3_input",
	"/sys/devices/platform/as9737_32db_thermal*temp4_input",
	"/sys/devices/platform/as9737_32db_thermal*temp5_input",
	"/sys/devices/platform/as9737_32db_psu*psu1_temp1_input",
	"/sys/devices/platform/as9737_32db_psu*psu1_temp2_input",
	"/sys/devices/platform/as9737_32db_psu*psu1_temp3_input",
	"/sys/devices/platform/as9737_32db_psu*psu2_temp1_input",
	"/sys/devices/platform/as9737_32db_psu*psu2_temp2_input",
	"/sys/devices/platform/as9737_32db_psu*psu2_temp3_input"
};

static char* cpu_coretemp_files[] = {
	"/sys/devices/platform/coretemp.0*temp2_input",
	"/sys/devices/platform/coretemp.0*temp3_input",
	"/sys/devices/platform/coretemp.0*temp4_input",
	"/sys/devices/platform/coretemp.0*temp5_input",
	NULL,
};

#define AS9737_THERMAL_CAPS (ONLP_THERMAL_CAPS_GET_TEMPERATURE | \
				 ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD | \
				 ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD | \
				 ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD)

/* Static values */
static onlp_thermal_info_t tinfo[] = {
	{ }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0}, 
		ONLP_THERMAL_STATUS_PRESENT,
		AS9737_THERMAL_CAPS, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "MB_FrontCenter_temp(0x48)", 0},
		ONLP_THERMAL_STATUS_PRESENT,
		AS9737_THERMAL_CAPS, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "MB_FrontRight_temp(0x49)", 0},
		ONLP_THERMAL_STATUS_PRESENT,
		AS9737_THERMAL_CAPS, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "MB_RearCenter_temp(0x4A)", 0},
		ONLP_THERMAL_STATUS_PRESENT,
		AS9737_THERMAL_CAPS, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "MB_RearLeft_temp(0x4C)", 0},
		ONLP_THERMAL_STATUS_PRESENT,
		AS9737_THERMAL_CAPS, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "MB_RearCenter_temp(0x4F)", 0},
		ONLP_THERMAL_STATUS_PRESENT,
		AS9737_THERMAL_CAPS, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", 
		ONLP_PSU_ID_CREATE(PSU1_ID)}, 
		ONLP_THERMAL_STATUS_PRESENT,
		ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU1), "PSU-1 Thermal Sensor 2", 
		ONLP_PSU_ID_CREATE(PSU1_ID)}, 
		ONLP_THERMAL_STATUS_PRESENT,
		ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_PSU1), "PSU-1 Thermal Sensor 3", 
		ONLP_PSU_ID_CREATE(PSU1_ID)}, 
		ONLP_THERMAL_STATUS_PRESENT,
		ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", 
		ONLP_PSU_ID_CREATE(PSU2_ID)}, 
		ONLP_THERMAL_STATUS_PRESENT,
		ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU2), "PSU-2 Thermal Sensor 2", 
		ONLP_PSU_ID_CREATE(PSU2_ID)}, 
		ONLP_THERMAL_STATUS_PRESENT,
		ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_PSU2), "PSU-2 Thermal Sensor 3", 
		ONLP_PSU_ID_CREATE(PSU2_ID)}, 
		ONLP_THERMAL_STATUS_PRESENT,
		ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	}
};

typedef struct threshold_t {
	int warning;
	int error;
	int shutdown;
} threshold_t;

threshold_t threshold[FAN_DIR_COUNT][THERMAL_COUNT-(CHASSIS_PSU_COUNT*NUM_OF_THERMAL_PER_PSU)] = {
	[FAN_DIR_F2B][THERMAL_CPU_CORE].warning  = 85000,
	[FAN_DIR_F2B][THERMAL_CPU_CORE].error    = 95000,
	[FAN_DIR_F2B][THERMAL_CPU_CORE].shutdown = 100000,
	[FAN_DIR_F2B][THERMAL_1_ON_MAIN_BROAD].warning  = 65000,
	[FAN_DIR_F2B][THERMAL_1_ON_MAIN_BROAD].error    = 70000,
	[FAN_DIR_F2B][THERMAL_1_ON_MAIN_BROAD].shutdown = 85000,
	[FAN_DIR_F2B][THERMAL_2_ON_MAIN_BROAD].warning  = 65000,
	[FAN_DIR_F2B][THERMAL_2_ON_MAIN_BROAD].error    = 70000,
	[FAN_DIR_F2B][THERMAL_2_ON_MAIN_BROAD].shutdown = 85000,
	[FAN_DIR_F2B][THERMAL_3_ON_MAIN_BROAD].warning  = 65000,
	[FAN_DIR_F2B][THERMAL_3_ON_MAIN_BROAD].error    = 70000,
	[FAN_DIR_F2B][THERMAL_3_ON_MAIN_BROAD].shutdown = 85000,
	[FAN_DIR_F2B][THERMAL_4_ON_MAIN_BROAD].warning  = 65000,
	[FAN_DIR_F2B][THERMAL_4_ON_MAIN_BROAD].error    = 70000,
	[FAN_DIR_F2B][THERMAL_4_ON_MAIN_BROAD].shutdown = 85000,
	[FAN_DIR_F2B][THERMAL_5_ON_MAIN_BROAD].warning  = 65000,
	[FAN_DIR_F2B][THERMAL_5_ON_MAIN_BROAD].error    = 70000,
	[FAN_DIR_F2B][THERMAL_5_ON_MAIN_BROAD].shutdown = 85000,

	[FAN_DIR_B2F][THERMAL_CPU_CORE].warning  = 85000,
	[FAN_DIR_B2F][THERMAL_CPU_CORE].error    = 95000,
	[FAN_DIR_B2F][THERMAL_CPU_CORE].shutdown = 100000,
	[FAN_DIR_B2F][THERMAL_1_ON_MAIN_BROAD].warning  = 55000,
	[FAN_DIR_B2F][THERMAL_1_ON_MAIN_BROAD].error    = 60000,
	[FAN_DIR_B2F][THERMAL_1_ON_MAIN_BROAD].shutdown = 75000,
	[FAN_DIR_B2F][THERMAL_2_ON_MAIN_BROAD].warning  = 55000,
	[FAN_DIR_B2F][THERMAL_2_ON_MAIN_BROAD].error    = 60000,
	[FAN_DIR_B2F][THERMAL_2_ON_MAIN_BROAD].shutdown = 75000,
	[FAN_DIR_B2F][THERMAL_3_ON_MAIN_BROAD].warning  = 55000,
	[FAN_DIR_B2F][THERMAL_3_ON_MAIN_BROAD].error    = 60000,
	[FAN_DIR_B2F][THERMAL_3_ON_MAIN_BROAD].shutdown = 75000,
	[FAN_DIR_B2F][THERMAL_4_ON_MAIN_BROAD].warning  = 55000,
	[FAN_DIR_B2F][THERMAL_4_ON_MAIN_BROAD].error    = 60000,
	[FAN_DIR_B2F][THERMAL_4_ON_MAIN_BROAD].shutdown = 75000,
	[FAN_DIR_B2F][THERMAL_5_ON_MAIN_BROAD].warning  = 55000,
	[FAN_DIR_B2F][THERMAL_5_ON_MAIN_BROAD].error    = 60000,
	[FAN_DIR_B2F][THERMAL_5_ON_MAIN_BROAD].shutdown = 75000,
};

threshold_t psu_threshold[NUM_OF_PSU_TYPE][NUM_OF_THERMAL_PER_PSU] = {
	[PSU_TYPE_AC_FSJ001_610G_F2B][0].warning  = 72000,
	[PSU_TYPE_AC_FSJ001_610G_F2B][0].error    = 77000,
	[PSU_TYPE_AC_FSJ001_610G_F2B][0].shutdown = 80000,
	[PSU_TYPE_AC_FSJ001_610G_F2B][1].warning  = 105000,
	[PSU_TYPE_AC_FSJ001_610G_F2B][1].error    = 110000,
	[PSU_TYPE_AC_FSJ001_610G_F2B][1].shutdown = 113000,
	[PSU_TYPE_AC_FSJ001_610G_F2B][2].warning  = 109000,
	[PSU_TYPE_AC_FSJ001_610G_F2B][2].error    = 114000,
	[PSU_TYPE_AC_FSJ001_610G_F2B][2].shutdown = 117000,
	[PSU_TYPE_AC_FSJ001_612G_F2B][0].warning  = 72000,
	[PSU_TYPE_AC_FSJ001_612G_F2B][0].error    = 77000,
	[PSU_TYPE_AC_FSJ001_612G_F2B][0].shutdown = 80000,
	[PSU_TYPE_AC_FSJ001_612G_F2B][1].warning  = 105000,
	[PSU_TYPE_AC_FSJ001_612G_F2B][1].error    = 110000,
	[PSU_TYPE_AC_FSJ001_612G_F2B][1].shutdown = 113000,
	[PSU_TYPE_AC_FSJ001_612G_F2B][2].warning  = 109000,
	[PSU_TYPE_AC_FSJ001_612G_F2B][2].error    = 114000,
	[PSU_TYPE_AC_FSJ001_612G_F2B][2].shutdown = 117000,
	[PSU_TYPE_AC_FSJ004_612G_B2F][0].warning  = 72000,
	[PSU_TYPE_AC_FSJ004_612G_B2F][0].error    = 77000,
	[PSU_TYPE_AC_FSJ004_612G_B2F][0].shutdown = 80000,
	[PSU_TYPE_AC_FSJ004_612G_B2F][1].warning  = 105000,
	[PSU_TYPE_AC_FSJ004_612G_B2F][1].error    = 110000,
	[PSU_TYPE_AC_FSJ004_612G_B2F][1].shutdown = 113000,
	[PSU_TYPE_AC_FSJ004_612G_B2F][2].warning  = 109000,
	[PSU_TYPE_AC_FSJ004_612G_B2F][2].error    = 114000,
	[PSU_TYPE_AC_FSJ004_612G_B2F][2].shutdown = 117000,
	[PSU_TYPE_DC48_FSJ035_610G_F2B][0].warning  = 65000,
	[PSU_TYPE_DC48_FSJ035_610G_F2B][0].error    = 70000,
	[PSU_TYPE_DC48_FSJ035_610G_F2B][0].shutdown = 75000,
	[PSU_TYPE_DC48_FSJ035_610G_F2B][1].warning  = 112000,
	[PSU_TYPE_DC48_FSJ035_610G_F2B][1].error    = 117000,
	[PSU_TYPE_DC48_FSJ035_610G_F2B][1].shutdown = 120000,
	[PSU_TYPE_DC48_FSJ035_610G_F2B][2].warning  = 95000,
	[PSU_TYPE_DC48_FSJ035_610G_F2B][2].error    = 100000,
	[PSU_TYPE_DC48_FSJ035_610G_F2B][2].shutdown = 110000,
	[PSU_TYPE_DC48_FSJ036_610G_B2F][0].warning  = 65000,
	[PSU_TYPE_DC48_FSJ036_610G_B2F][0].error    = 70000,
	[PSU_TYPE_DC48_FSJ036_610G_B2F][0].shutdown = 75000,
	[PSU_TYPE_DC48_FSJ036_610G_B2F][1].warning  = 112000,
	[PSU_TYPE_DC48_FSJ036_610G_B2F][1].error    = 117000,
	[PSU_TYPE_DC48_FSJ036_610G_B2F][1].shutdown = 120000,
	[PSU_TYPE_DC48_FSJ036_610G_B2F][2].warning  = 95000,
	[PSU_TYPE_DC48_FSJ036_610G_B2F][2].error    = 100000,
	[PSU_TYPE_DC48_FSJ036_610G_B2F][2].shutdown = 110000,
};

static int
onlp_thermali_psu_threshold_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
	int ret = ONLP_STATUS_OK;
	int thermal_id;
	int psu_id;
	psu_type_t psu_type;
	VALIDATE(id);

	thermal_id = ONLP_OID_ID_GET(id);
	psu_id = ((thermal_id - THERMAL_1_ON_PSU1) / NUM_OF_THERMAL_PER_PSU) + 1;

	/* Get PSU type
	 */
	psu_type = get_psu_type(psu_id, NULL, 0);

	switch (psu_type) {
	case PSU_TYPE_AC_FSJ001_610G_F2B:
	case PSU_TYPE_AC_FSJ001_612G_F2B:
	case PSU_TYPE_AC_FSJ004_612G_B2F:
	case PSU_TYPE_DC48_FSJ035_610G_F2B:
	case PSU_TYPE_DC48_FSJ036_610G_B2F:
		info->thresholds.warning  = psu_threshold[psu_type][(thermal_id - THERMAL_1_ON_PSU1) % NUM_OF_THERMAL_PER_PSU].warning;
		info->thresholds.error    = psu_threshold[psu_type][(thermal_id - THERMAL_1_ON_PSU1) % NUM_OF_THERMAL_PER_PSU].error;
		info->thresholds.shutdown = psu_threshold[psu_type][(thermal_id - THERMAL_1_ON_PSU1) % NUM_OF_THERMAL_PER_PSU].shutdown;
		break;
	default:
		ret = ONLP_STATUS_E_UNSUPPORTED;
		break;
	}

	return ret;
}

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
	return ONLP_STATUS_OK;
}

/*
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */
int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
	int tid;
	int psu_id, psu_tid_start = 0;
	int val = 0;
	int ret = 0;
	int i;
	enum onlp_fan_dir dir = FAN_DIR_F2B;;
	
	VALIDATE(id);
	tid = ONLP_OID_ID_GET(id);

	for (i = 1; i < CHASSIS_FAN_COUNT+1; i++) {
		dir = onlp_get_fan_dir(i);

		if (dir == FAN_DIR_B2F)
			break;
	}

	if( (tid >= THERMAL_CPU_CORE) && (tid <= THERMAL_5_ON_MAIN_BROAD)) {
		/* Set the onlp_oid_hdr_t and capabilities */
		*info = tinfo[tid];
		info->thresholds.warning  = threshold[dir][tid].warning;
		info->thresholds.error    = threshold[dir][tid].error;
		info->thresholds.shutdown = threshold[dir][tid].shutdown;
	} else if ((tid >= THERMAL_1_ON_PSU1) && (tid <= THERMAL_3_ON_PSU2)) {
		*info = tinfo[tid];
		onlp_thermali_psu_threshold_get(id, info);
	}

	if (tid == THERMAL_CPU_CORE)
		return onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);

	psu_tid_start = CHASSIS_THERMAL_COUNT + 1;

	if (tid >= psu_tid_start) {
		psu_id = ( tid <= THERMAL_3_ON_PSU1 ) ? PSU1_ID : PSU2_ID;
		/* Get power good status */
		ret = onlp_file_read_int(&val, PSU_SYSFS_FORMAT, psu_id, "power_good");
		if (ret < 0) {
			AIM_LOG_ERROR("Unable to read status from (PSU_SYSFS_FORMAT)\r\n", psu_id, "power_good");
			return ONLP_STATUS_E_INTERNAL;
		}

		if(val != PSU_STATUS_POWER_GOOD) {
			info->status |= ONLP_THERMAL_STATUS_FAILED;
		}
	}

	return onlp_file_read_int(&info->mcelsius, devfiles__[tid]);
}
