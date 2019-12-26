#ifndef __LC_DEV_H
#define __LC_DEV_H


/*function declaration*/
int lc_dev_lc_cpld_init(int lc_id);
int lc_dev_power_set(int lc_id, bool on);
int lc_dev_power_ready(int lc_id, bool *ready);
void lc_dev_phy_ready(unsigned long bitmap, bool *ready);
int lc_dev_prs_get(unsigned long *bitmap);
int lc_dev_temp_get_text(int lc_id, char *buf, int size);
int lc_dev_reset_set(int lc_id, u8 lv);
int lc_dev_reset_get(int lc_id, u8 *lv);
int lc_dev_type_get(int lc_id, lc_type_t *type);
int lc_dev_type_get_text(int lc_id, u8 *buf, int size);
int lc_dev_mux_l1_reset(int lc_id, u8 lv);
bool lc_dev_mux_l1_is_alive(int lc_id);
int lc_dev_mux_l2_reset(int lc_id, int mux_l2_id, u8 lv);
int lc_dev_led_set(int lc_id, lc_led_ctrl_t ctrl);
int lc_dev_led_st_get(int lc_id, lc_led_ctrl_t *ctrl);
int lc_dev_init(int platform_id, int io_no_init);
void lc_dev_deinit(void);
int lc_dev_handler(void);
struct sff_io_driver_t *sff_io_drv_get_lcdev(void);
int lc_dev_over_temp_asserted(int lc_id, bool *asserted);
int lc_dev_over_temp_deasserted(int lc_id, bool *deasserted);
int lc_dev_temp_get_text(int lc_id, char *buf, int size);
int lc_dev_temp_th_get_text(int lc_id, char *buf, int size);
#endif /*__LC_DEV_H*/
