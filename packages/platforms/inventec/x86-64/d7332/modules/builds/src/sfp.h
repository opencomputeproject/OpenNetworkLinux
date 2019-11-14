//port info
#ifndef __SFF_SFP_H
#define __SFF_SFP_H

int sfp_vendor_info_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
int sfp_temperature_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
int sfp_voltage_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
int sfp_lane_status_get(struct sff_obj_t *sff_obj, int type, u8 *value);
int sfp_lane_monitor_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
int sfp_lane_control_set(struct sff_obj_t *sff_obj, int type, u32 value);
int sfp_lane_control_get(struct sff_obj_t *sff_obj, int type, u32 *value);
int sfp_id_get(struct sff_obj_t *sff_obj, u8 *id);
int sff_fsm_sfp_task(struct sff_obj_t *sff_obj);
#endif /*__SFF_SFP_H*/
