#ifndef __ONLP_MODULEI_H__
#define __ONLP_MODULEI_H__

#include <onlp/module.h>

/**
 * @brief Initialize the Module subsystem.
 */
int onlp_modulei_init(void);

/**
 * @brief Get the information structure for the given Module
 * @param id The Module OID
 * @param rv [out] Receives the Module information.
 */
int onlp_modulei_info_get(onlp_oid_t id, onlp_module_info_t* rv);

/**
 * @brief Get the Module's operational status.
 * @param id The Module OID.
 * @param rv [out] Receives the operational status.
 */
int onlp_modulei_status_get(onlp_oid_t id, uint32_t* rv);

/**
 * @brief Get the Module's oid header.
 * @param id The Module OID.
 * @param rv [out] Receives the header.
 */
int onlp_modulei_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv);

#endif /* __ONLP_MODULEI_H__ */
