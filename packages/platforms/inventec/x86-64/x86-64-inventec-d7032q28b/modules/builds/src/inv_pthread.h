#ifndef INV_PTHREAD_H
#define INV_PTHREAD_H

#define CPLD_DEV_LED_GRN_INDEX	(0)
#define CPLD_DEV_LED_RED_INDEX	(1)

ssize_t cpld_show_led(char *buf, int index);
ssize_t cpld_set_led(const char *buf, size_t count, int index);
ssize_t cpld_show_ctl(char *buf);
ssize_t cpld_set_ctl(const char *buf, size_t count);

ssize_t psoc_show_psu_state(char *buf, int index);
ssize_t psoc_show_fan_input(char *buf, int index);
ssize_t psoc_show_fan_state(char *buf);
ssize_t psoc_show_psu_vin(char *buf, int index);
ssize_t psoc_show_diag(char *buf);
ssize_t psoc_set_diag(const char *buf, size_t count);

#endif /* INV_PTHREAD_H */
