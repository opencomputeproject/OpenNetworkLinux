#include <onlp/platformi/chassisi.h>
#include "platform_lib.h"

/**
 * @brief Software initializaiton of the Chassis module.
 */
int onlp_chassisi_sw_init(void) {
  return ONLP_STATUS_OK;
}

/**
 * @brief Hardware initializaiton of the Chassis module.
 * @param flags The hardware initialization flags.
 */
int onlp_chassisi_hw_init(uint32_t flags) {
  return ONLP_STATUS_OK;
}

/**
 * @brief Deinitialize the chassis software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_chassisi_sw_denit(void) {
  return ONLP_STATUS_OK;
}


/**
 * @brief Get the chassis hdr structure.
 * @param id The Chassis OID.
 * @param[out] hdr Receives the header.
 */
int onlp_chassisi_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
  int i;
  onlp_oid_t* e = hdr->coids;

  ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
  ONLP_OID_STATUS_FLAG_SET(hdr, OPERATIONAL);
 
  /* 8 Thermal sensors on the chassis */
  for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++)
  {
    *e++ = ONLP_THERMAL_ID_CREATE(i);
  }

  /* 2 LEDs on the chassis */
  for (i = 1; i <= CHASSIS_LED_COUNT; i++)
  {
    *e++ = ONLP_LED_ID_CREATE(i);
  }

  /* 2 PSUs on the chassis */
  for (i = 1; i <= CHASSIS_PSU_COUNT; i++)
  {
    *e++ = ONLP_PSU_ID_CREATE(i);
  }

  /* 5 Fans on the chassis */
  for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
  {
    *e++ = ONLP_FAN_ID_CREATE(i);
  }

  /* 32 QSFPs */
  for(i = 1; i <= 32; i++) {
    *e++ = ONLP_SFP_ID_CREATE(i);
  }
  
  return ONLP_STATUS_OK;
}

/**
 * @brief Get the chassis info structure.
 * @param id The Chassis OID.
 * @param[out] info Receives the chassis information.
 */
int onlp_chassisi_info_get(onlp_oid_id_t id, onlp_chassis_info_t* info) {
  onlp_chassisi_hdr_get(id, &info->hdr);
  return ONLP_STATUS_OK;
}