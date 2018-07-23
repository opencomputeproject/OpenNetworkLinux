/**************************************************************************//**
 *
 *
 *
 *
 *****************************************************************************/
#ifndef __ONLP_PLATFORM_SIM__OIDS_H__
#define __ONLP_PLATFORM_SIM__OIDS_H__

#include <onlp/oids.h>
#include <onlp/chassis.h>
#include <onlp/psu.h>
#include <onlp/fan.h>
#include <onlp/thermal.h>
#include <onlp/led.h>
#include <onlp/sfp.h>

/**
 * @brief Initialize the OID simulation.
 * @param fname The OID JSON file.
 */
int onlp_platform_sim_oids_init(const char* fname);

/**
 * @brief Lookup a structure by OID.
 * @param oid The oid to search for.
 * @param[out] hdr Receives the pointer to the structure.
 * @note You would not normally use this function.
 * Instead use the instances provided below.
 */
onlp_oid_hdr_t*
onlp_platform_sim_oid_lookup(onlp_oid_t oid);

/**
 * @brief Get the chassis info structure for the given oid.
 * @param oid The chassis oid.
 */
int
onlp_platform_sim_chassis_get(onlp_oid_t oid,
                              onlp_oid_hdr_t* hdr,
                              onlp_chassis_info_t* info,
                              onlp_chassis_info_t** pinfo);

/**
 * @brief Get the psu info structure for the given oid.
 * @param oid The psu oid.
 */
int
onlp_platform_sim_psu_get(onlp_oid_t oid,
                               onlp_oid_hdr_t* hdr,
                               onlp_psu_info_t* info,
                               onlp_psu_info_t** pinfo);

/**
 * @brief Get the fan info structure for the given oid.
 * @param oid The fan oid.
 */
int
onlp_platform_sim_fan_get(onlp_oid_t oid,
                               onlp_oid_hdr_t* hdr,
                               onlp_fan_info_t* info,
                               onlp_fan_info_t** pinfo);

/**
 * @brief Get the thermal info structure for the given oid.
 * @param oid The thermal oid.
 */
int
onlp_platform_sim_thermal_get(onlp_oid_t oid,
                                   onlp_oid_hdr_t* hdr,
                                   onlp_thermal_info_t* info,
                                   onlp_thermal_info_t** pinfo);

/**
 * @brief Get the led info structure for the given oid.
 * @param oid The led oid.
 */
int
onlp_platform_sim_led_get(onlp_oid_t oid,
                               onlp_oid_hdr_t* hdr,
                               onlp_led_info_t* info,
                               onlp_led_info_t** pinfo);

/**
 * @brief Get the sfp info structure for the given oid.
 * @param oid The sfp oid.
 */
int
onlp_platform_sim_sfp_get(onlp_oid_t oid,
                               onlp_oid_hdr_t* hdr,
                               onlp_sfp_info_t* info,
                               onlp_sfp_info_t** pinfo);




#endif /* __ONLP_PLATFORM_SIM__OIDS_H__ */
