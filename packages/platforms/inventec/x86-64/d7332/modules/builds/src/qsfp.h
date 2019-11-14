//port info
#ifndef __SFF_QSFP_H
#define __SFF_QSFP_H

int qsfp_vendor_info_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
int qsfp_temperature_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
int qsfp_voltage_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
int qsfp_lane_status_get(struct sff_obj_t *sff_obj, int type, u8 *value);
int qsfp_lane_monitor_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
int qsfp_lane_control_set(struct sff_obj_t *sff_obj, int type, u32 value);
int qsfp_lane_control_get(struct sff_obj_t *sff_obj, int type, u32 *value);
int qsfp_tx_disable_set(struct sff_obj_t *sff_obj, u8 value);
int qsfp_tx_disable_get(struct sff_obj_t *sff_obj, u8 *value);
int qsfp_id_get(struct sff_obj_t *sff_obj, u8 *id);
int sff_fsm_qsfp_task(struct sff_obj_t *sff_obj);
#endif /*__SFF_QSFP_H*/
