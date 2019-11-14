//port info
#ifndef __SFF_QSFP_DD_H
#define __SFF_QSFP_DD_H

/*qsfp_dd function*/
//int qsfp_dd_channel_control_get(struct sff_obj_t *sff_obj, int type, u32 *value);
int qsfp_dd_lane_monitor_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
int qsfp_dd_tx_disable_set(struct sff_obj_t *sff_obj, u8 val);
int qsfp_dd_tx_disable_get(struct sff_obj_t *sff_obj, u8 *val);
int qsfp_dd_id_get(struct sff_obj_t *sff_obj, u8 *val);
int qsfp_dd_vendor_info_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
int qsfp_dd_module_st_get(struct sff_obj_t *sff_obj, u8 *st);
int qsfp_dd_temperature_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
int qsfp_dd_voltage_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
int sff_fsm_qsfp_dd_task(struct sff_obj_t *sff_obj);

#endif /*__SFF_QSFP_DD_H*/
