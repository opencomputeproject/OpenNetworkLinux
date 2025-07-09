#ifndef SYSFS_IPMI_H
#define SYSFS_IPMI_H

#include "switch_sensor_driver.h"

//unsigned int bmc_get_temp(unsigned int index, long *temp_input);
int drv_get_sensor_temp_input_from_bmc(unsigned int index, long *value);
int drv_get_sensor_vol_input_from_bmc(vr_sensor_node_t node, long *value);
int drv_get_sensor_curr_input_from_bmc(vr_sensor_node_t node, long *value);
ssize_t drv_get_wind_from_bmc(unsigned int fan_index, unsigned int *wind);
ssize_t drv_get_hw_version_from_bmc(unsigned int fan_index, char *buf);
ssize_t drv_get_model_name_from_bmc(unsigned int fan_index, char *buf);
ssize_t drv_get_sn_from_bmc(unsigned int fan_index, char *buf);
ssize_t drv_get_vendor_from_bmc(unsigned int fan_index, char *buf);
ssize_t drv_get_part_number_from_bmc(unsigned int fan_index, char *buf);
ssize_t drv_get_speed_from_bmc(unsigned int slot_id, unsigned int fan_id, unsigned int *speed);
ssize_t drv_get_pwm_from_bmc(unsigned int fan_id, int *pwm);
ssize_t drv_set_pwm_from_bmc(unsigned int fan_id, int pwm);
ssize_t drv_get_led_status_from_bmc(unsigned int fan_index, unsigned int *led);
ssize_t drv_set_led_status_from_bmc(unsigned int fan_index, unsigned int led);
int drv_get_mfr_info_from_bmc(unsigned int psu_index, u8 pmbus_command, char *buf);
bool ipmi_bmc_is_ok(void);
ssize_t drv_fmea_get_work_status_from_bmc(unsigned int index, char *buf, char *plt);
ssize_t drv_fmea_get_current_status_from_bmc(unsigned int index, char *buf, char *plt);
ssize_t drv_fmea_get_pmbus_status_from_bmc(unsigned int index, char *buf, char *plt);
ssize_t drv_fmea_get_mfr_id_from_bmc(unsigned int index, int *value);
int drv_cpld_write_from_bmc(unsigned char index, unsigned char reg, unsigned char value);
int drv_cpld_read_from_bmc(unsigned char index, unsigned char reg, unsigned char *value);

#endif /*SYSFS_IPMI_H*/
