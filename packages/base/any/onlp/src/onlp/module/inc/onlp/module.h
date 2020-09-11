#ifndef __ONLP_MODULE_H__
#define __ONLP_MODULE_H__

#include <onlp/onlp.h>
#include <onlp/oids.h>

typedef struct onlp_module_info_t {
    /** OID Header */
    onlp_oid_hdr_t hdr;

    /* Status */
    uint32_t status;

} onlp_module_info_t;

/**
 * @brief Initialize the Module subsystem.
 */
int onlp_module_init(void);

/**
 * @brief Get the Module information.
 * @param id The Module OID.
 * @param rv [out] Receives the information structure.
 */
int onlp_module_info_get(onlp_oid_t id, onlp_module_info_t* rv);

/**
 * @brief Get the Module's operational status.
 * @param id The Module OID.
 * @param rv [out] Receives the operational status.
 */
int onlp_module_status_get(onlp_oid_t id, uint32_t* rv);

/**
 * @brief Get the Module's oid header.
 * @param id The Module OID.
 * @param rv [out] Receives the header.
 */
int onlp_module_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv);

/**
 * @brief Module OID debug dump
 * @param id The Module OID
 * @param pvs The output pvs
 * @param flags The output flags
 */
void onlp_module_dump(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags);


/**
 * @brief Show the given Module OID.
 * @param id The Module OID
 * @param pvs The output pvs
 * @param flags The output flags
 */
void onlp_module_show(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags);

#endif
