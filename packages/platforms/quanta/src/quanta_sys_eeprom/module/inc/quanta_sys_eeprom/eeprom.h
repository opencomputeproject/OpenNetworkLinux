/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#ifndef __QUANTA_SYS_EEPROM_EEPROM_H__
#define __QUANTA_SYS_EEPROM_EEPROM_H__

#include <quanta_sys_eeprom/quanta_sys_eeprom_config.h>
#include <onlplib/onie.h>
#include <AIM/aim_pvs.h>

typedef struct quanta_sys_eeprom_data_s {

#define EEPROM_STR_MAX 32

    char product_name[EEPROM_STR_MAX];
    char model_name[EEPROM_STR_MAX];
    char part_number[EEPROM_STR_MAX];
    char serial_number[EEPROM_STR_MAX];
    char manufacture_date[EEPROM_STR_MAX];
    char label_version[EEPROM_STR_MAX];
    uint8_t mac_address[6];
    uint32_t hardware_version;
    uint32_t software_version;
    uint32_t card_type;
    uint32_t crc;

} quanta_sys_eeprom_t;

/**
 * @brief Parse the given data in EEPROM format.
 * @param data The eeprom data.
 * @param size The length of the data.
 * @param rv Receives the eeprom information.
 */
int quanta_sys_eeprom_parse_data(const uint8_t* data, int size,
                                 quanta_sys_eeprom_t* rv);

/**
 * @brief Parse the given file in EEPROM format.
 * @param file The eeprom file.
 * @param rv Receives the eeprom information.
 */
int quanta_sys_eeprom_parse_file(const char* file, quanta_sys_eeprom_t* rv);


/**
 * @brief Convert The sys-eeprom to the onie-eeprom format.
 * @param src Source structure.
 * @param dst Destination structure.
 */
int quanta_sys_eeprom_to_onie(const quanta_sys_eeprom_t* src,
                              onlp_onie_info_t* dst);

/**
 * @brief Show the contents of the given eeprom data;
 * @param pvs The output pvs.
 * @param e The eeprom structure.
 */

int quanta_sys_eeprom_show(aim_pvs_t* pvs, quanta_sys_eeprom_t* e);

#endif /* __QUANTA_SYS_EEPROM_EEPROM_H__ */
