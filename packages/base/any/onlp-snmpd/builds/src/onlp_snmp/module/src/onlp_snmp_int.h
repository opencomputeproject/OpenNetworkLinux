/**************************************************************************//**
 *
 * onlp_snmp Internal Header
 *
 *****************************************************************************/
#ifndef __ONLP_SNMP_INT_H__
#define __ONLP_SNMP_INT_H__

#include <onlp_snmp/onlp_snmp_config.h>

int onlp_snmp_sensors_init(void);
int onlp_snmp_sensor_update_start(void);
int onlp_snmp_platform_init(void);
int onlp_snmp_platform_update_start(void);

#endif /* __ONLP_SNMP_INT_H__ */
