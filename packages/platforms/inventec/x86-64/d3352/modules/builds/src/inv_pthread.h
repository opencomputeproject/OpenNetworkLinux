#ifndef INV_PTHREAD_H
#define INV_PTHREAD_H

#define CPLD_DEV_LED_GRN_INDEX	(0)
#define CPLD_DEV_LED_RED_INDEX	(1)

/*
 * onlp_led_id should be same as in onlp/platform_lib.h
 */
enum onlp_led_id {
    LED_RESERVED = 0,
    LED_STK,
    LED_FAN,
    LED_PWR,
    LED_SYS,
    LED_FAN1,
    LED_FAN2,
    LED_MAX
};

typedef struct cpld_led_map_s {
	char	*name;
	int	bit_shift;
	u8	bit_mask;
	u8	led_off;
	u8	led_on;
	u8	led_blink;
	u8	led_blink_slow;
} cpld_led_map_t;

ssize_t cpld_show_psu(char *buf);
ssize_t cpld_show_led(char *buf, size_t count);

ssize_t cpld_set_ctl(const char *buf, size_t count);
ssize_t cpld_set_sys_led(const char *buf, size_t count);
ssize_t cpld_set_pwr_led(const char *buf, size_t count);
ssize_t cpld_set_fan_led(const char *buf, size_t count);
ssize_t cpld_set_stk_led(const char *buf, size_t count);

ssize_t psoc_show_psu_state(char *buf, int index);
ssize_t psoc_show_fan_input(char *buf, int index);
ssize_t psoc_show_fan_state(char *buf);
ssize_t psoc_show_psu_vin(char *buf, int index);
ssize_t psoc_show_diag(char *buf);
ssize_t psoc_set_diag(const char *buf, size_t count);

#endif /* INV_PTHREAD_H */
