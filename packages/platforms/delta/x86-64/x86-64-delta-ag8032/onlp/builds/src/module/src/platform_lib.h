/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 (C) Delta Networks, Inc.
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

#ifndef __PLAT_LIB_H__
#define __PLAT_LIB_H__

#include <onlplib/i2c.h>

#define ONIE_EEPROM_LOCATION	"/sys/bus/i2c/devices/i2c-5/5-0054/eeprom"

typedef int (*hook_present)(void *e);
typedef int (*hook_event  )(void *e, int ev);

extern int present_on_board_always (void *e);

////////////////////////////////////////////////////////////////
// PLAT DEV ROUTINE
typedef enum plat_dev_id {
	PLAT_DEV_ID_INVALID = 0,
	PLAT_DEV_ID_1,
	PLAT_DEV_ID_2,
	PLAT_DEV_ID_3,
	PLAT_DEV_ID_4,
	PLAT_DEV_ID_5,
	// ....
	PLAT_DEV_ID_MAX = 128,
} plat_dev_id_t;

typedef struct plat_dev_desc {
	char *name;
	// i2c dev
	int bus;
	uint8_t addr;
} plat_dev_desc_t;

extern int plat_dev_id_is_valid (plat_dev_id_t id);
extern int plat_dev_get_byte (plat_dev_id_t id, uint8_t reg);
extern int plat_dev_set_byte (plat_dev_id_t id, uint8_t reg, uint8_t val);

////////////////////////////////////////////////////////////////
// CPLD PLAT ROUTINE
typedef enum plat_cpld_id {
	PLAT_CPLD_ID_INVALID = 0,
	PLAT_CPLD_ID_1,
	PLAT_CPLD_ID_2,
	PLAT_CPLD_ID_3,
	PLAT_CPLD_ID_4,
	PLAT_CPLD_ID_MAX,
} plat_cpld_id_t ;

typedef struct plat_cpld {
	plat_dev_id_t id;
	uint8_t cached[256];
	uint8_t cache[256];
} plat_cpld_t;

extern int cpld_id_is_valid (plat_cpld_id_t id);
extern int cpld_get (plat_cpld_id_t id, uint8_t reg);
extern int cpld_set (plat_cpld_id_t id, uint8_t reg, uint8_t val);
extern int cpld_field_get (plat_cpld_id_t id, uint8_t reg, uint8_t field, uint8_t len);
extern int cpld_field_set (plat_cpld_id_t id, uint8_t reg, uint8_t field, uint8_t len, uint8_t val);

#define CPLD_CPUPLD		PLAT_CPLD_ID_1
#define CPLD_SWPLD		PLAT_CPLD_ID_2

////////////////////////////////////////////////////////////////
// CPLD REG PLAT ROUTINE
typedef struct cpld_reg {
	plat_cpld_id_t id;
	uint8_t reg;
	uint8_t field;
	uint8_t len;
	char valid;
} cpld_reg_t;

#define CPLD_REG(i,r,f,l)	{.valid=1, .id=i,.reg=r,.field=f,.len=l,}
extern int cpld_reg_is_valid (cpld_reg_t *r);
extern int cpld_reg_get (cpld_reg_t *r);
extern int cpld_reg_set (cpld_reg_t *r, uint8_t val);

////////////////////////////////////////////////////////////////
// THERMAL PLAT ROUTINE
typedef enum plat_thermal_id {
	PLAT_THERMAL_ID_INVALID,
	PLAT_THERMAL_ID_1 = 1,
	PLAT_THERMAL_ID_2,
	PLAT_THERMAL_ID_3,
	PLAT_THERMAL_ID_4,
	PLAT_THERMAL_ID_5,
	PLAT_THERMAL_ID_6,
	PLAT_THERMAL_ID_7,
	PLAT_THERMAL_ID_MAX
} plat_thermal_id_t;

typedef struct plat_thermal {
	char	*desc;

	hook_present present;
	void *present_data;

	char	*temp_get_path;

	char	*warnning_set_path;
	int		def_warnning;

	char	*critical_set_path;
	int		def_critical;

	char	*shutdown_set_path;
	int		def_shutdown;

} plat_thermal_t ;

////////////////////////////////////////////////////////////////
// LED PLAT ROUTINE
typedef enum plat_led_id {
	PLAT_LED_ID_INVALID = 0,
	PLAT_LED_ID_1,
	PLAT_LED_ID_2,
	PLAT_LED_ID_3,
	PLAT_LED_ID_4,
	PLAT_LED_ID_5,
	PLAT_LED_ID_6,
	PLAT_LED_ID_7,
	PLAT_LED_ID_MAX
} plat_led_id_t ;

typedef struct led_mode {
	int	onlp_val;
	int	hw_val;
} led_mode_t;
#define PLAT_LED_MODE_MAX	16
#define PLAT_LED_MODE(o,h)	{ .onlp_val = o, .hw_val = h, }
#define PLAT_LED_MODE_END	{ .onlp_val = -1, .hw_val = -1, }

#define PLAT_LED_INTERNAL_DEF \
	.hw_val_run = -1


typedef struct plat_led {
	char *name;
	hook_present present;
	void *present_data;
	cpld_reg_t hw;
	int hw_val_run;
	led_mode_t mode[PLAT_LED_MODE_MAX];
} plat_led_t;

////////////////////////////////////////////////////////////////
// FAN PLAT ROUTINE
typedef enum plat_fan_id {
	PLAT_FAN_ID_INVALID = 0,
	PLAT_FAN_ID_1,
	PLAT_FAN_ID_2,
	PLAT_FAN_ID_3,
	PLAT_FAN_ID_4,
	PLAT_FAN_ID_5,
	PLAT_FAN_ID_6,
	PLAT_FAN_ID_7,
	PLAT_FAN_ID_8,
	PLAT_FAN_ID_MAX,
} plat_fan_id_t ;

typedef enum plat_fan_state {
	PLAT_FAN_STATE_UNPRESENT = 0,
	PLAT_FAN_STATE_PRESENT,
} plat_fan_state_t;

typedef enum plat_fan_event {
	PLAT_FAN_EVENT_UNPLUG = 0,
	PLAT_FAN_EVENT_PLUGIN,
} plat_fan_event_t ;

typedef struct plat_fan {
	char *name;

	hook_present present;
	void *present_data;

	char *rpm_get_path;
	char *rpm_set_path;
	int   def_rpm;
	char *per_get_path;
	char *per_set_path;
	int   def_per;

	char *eeprom_path;

	uint32_t caps;

	// internal use
	int rpm_set_value;
	int per_set_value;

	plat_fan_state_t state;
	hook_event event_callback;

	uint8_t eeprom[256];

} plat_fan_t;

#define PLAT_FAN_INTERNAL_DEF \
	.rpm_set_value = -1,\
	.per_set_value = -1,\
	.state = PLAT_FAN_STATE_UNPRESENT

extern int plat_fan_state_update (plat_fan_t *fan);

////////////////////////////////////////////////////////////////
// SFP PLAT ROUTINE
typedef enum plat_sff_id {
	PLAT_SFF_ID_MIN = 1,
	PLAT_SFF_ID_MAX = 32,
} plat_sff_id_t;

typedef int (*hook_sff_control)(void *e, int sval, int *gval, int *sup);

typedef struct plat_sff {
	char valid;

	hook_present present;
	cpld_reg_t present_cpld_reg;

	hook_sff_control reset;
	cpld_reg_t reset_cpld_reg;

	hook_sff_control lpmode;
	cpld_reg_t lpmode_cpld_reg;

	hook_sff_control rxlos;
	cpld_reg_t rxlos_cpld_reg;

	hook_sff_control txfault;
	cpld_reg_t txfault_cpld_reg;

	hook_sff_control txdisable;
	cpld_reg_t txdisable_cpld_reg;


	int bus;
} plat_sff_t;

////////////////////////////////////////////////////////////////
// PSU PLAT ROUTINE
typedef enum plat_psu_id {
	PLAT_PSU_ID_INVALID = 0,
	PLAT_PSU_ID_1,
	PLAT_PSU_ID_2,
	PLAT_PSU_ID_MAX
} plat_psu_id_t;

typedef enum plat_psu_state {
	PLAT_PSU_STATE_UNPRESENT = 0,
	PLAT_PSU_STATE_PRESENT,
	PLAT_PSU_STATE_PMBUS_READY,
	PLAT_PSU_STATE_MAX
} plat_psu_state_t;

typedef enum plat_psu_event {
	PLAT_PSU_EVENT_UNPLUG = 0,
	PLAT_PSU_EVENT_PLUGIN,
	PLAT_PSU_PMBUS_CONNECT,
	PLAT_PSU_PMBUS_DISCONNECT,
LAT_PSU_EVENT_
} plat_psu_event_t;

typedef struct plat_psu {

	char *name;

	hook_present present;
	cpld_reg_t present_cpld_reg;

	char *vin_path;
	char *vout_path;
	char *iin_path;
	char *iout_path;
	char *pin_path;
	char *pout_path;

	char *vin_max_path;
	char *vin_min_path;

	hook_event event_callback;

	char eeprom_bus;
	char eeprom_addr;

	// use for probe and insmod
	plat_psu_state_t state;
	char *pmbus_insert_cmd;
	char *pmbus_remove_cmd;
	char *pmbus_ready_path;
	uint8_t pmbus_bus;
	uint8_t pmbus_addr;

	uint8_t eeprom[256];

} plat_psu_t;

extern int plat_psu_state_update (plat_psu_t *psu);

////////////////////////////////////////////////////////////////
// OS HELP ROUTINE
extern int plat_os_file_is_existed (char *path);
extern int plat_os_file_read (uint8_t *data, int max, int *len, char *path, ...);
extern int plat_os_file_read_int (int *val, char *path, ...);
extern int plat_os_file_write_int(int  val, char *path, ...);

#endif // __PLAT_LIB_H__

