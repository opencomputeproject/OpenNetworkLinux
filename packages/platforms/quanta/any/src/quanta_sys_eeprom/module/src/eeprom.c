/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <quanta_sys_eeprom/quanta_sys_eeprom_config.h>
#include <quanta_sys_eeprom/eeprom.h>
#include <time.h>
#include "quanta_sys_eeprom_log.h"
#include <onlplib/file.h>

int
quanta_sys_eeprom_parse_data(const uint8_t* data, int size,
                             quanta_sys_eeprom_t* rv)
{
    /*
     * EEPROM Magic: 0xFF 0x01 0xE0
     */
    const uint8_t* p = data;
    if(!rv || size < 3 || *p++ != 0xFF || *p++ != 0x01 || *p++ != 0xE0) {
        return -1;
    }

    memset(rv, 0, sizeof(*rv));

    while(p < (data+size)) {
        uint8_t code = *p++;
        int clen = *p++;
        if(clen < 1) {
            break;
        }
        switch(code)
            {
#define EEPROM_STRCPY(_field) strncpy(rv->_field, (char*)p, clen)
#define EEPROM_LONG(_field) rv->_field = (p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3])

            case 0x1:
                /* Product Name */
                EEPROM_STRCPY(product_name);
                break;
            case 0x2:
                /* Part Number */
                EEPROM_STRCPY(part_number);
                break;
            case 0x3:
                /* Serial Number */
                EEPROM_STRCPY(serial_number);
                break;
            case 0x4:
                /* MAC */
                memcpy(rv->mac_address, p, 6);
                break;

            case 0x5:
                /* Manufacture Date */
                {
                    struct tm time_tm;
                    time_t time_time;
                    memset(&time_tm, 0, sizeof(time_tm));

                    time_tm.tm_year = (p[0] << 8 | p[1]) - 1900;
                    time_tm.tm_mon = p[2] - 1;
                    time_tm.tm_mday = p[3];
                    time_time = mktime(&time_tm);
                    struct tm* gtm = gmtime(&time_time);
                    strftime(rv->manufacture_date,
                             sizeof(rv->manufacture_date),
                             "%m/%d/%Y %H:%M:%S",
                             gtm);
                    break;
                }

            case 0x6:
                /* Card Type */
                EEPROM_LONG(card_type);
                break;
            case 0x7:
                /* Hardware Version */
                EEPROM_LONG(hardware_version);
                break;
            case 0x8:
                /* Label Version */
                EEPROM_STRCPY(label_version);
                break;
            case 0x9:
                /* Model Name */
                EEPROM_STRCPY(model_name);
                break;
            case 0xA:
                /* Software Version */
                EEPROM_LONG(software_version);
                break;

            case 0x00:
                /* CRC */
                rv->crc = p[0] << 8 | p[1];
                return 0;
            }
        p+=clen;
    }
    return 0;
}

int
quanta_sys_eeprom_parse_file(const char* file, quanta_sys_eeprom_t* e)
{
    int rv;
    uint8_t data[256];
    int len;

    rv = onlp_file_read(data, sizeof(data), &len, (char*)file);
    if(rv >= 0) {
        rv = quanta_sys_eeprom_parse_data(data, sizeof(data), e);
    }
    return rv;
}


int
quanta_sys_eeprom_to_onie(const quanta_sys_eeprom_t* src,
                          onlp_onie_info_t* dst)
{
    if(src == NULL || dst == NULL) {
        return -1;
    }

    memset(dst, 0, sizeof(*dst));
    list_init(&dst->vx_list);
    dst->product_name = aim_strdup(src->product_name);
    dst->part_number = aim_strdup(src->part_number);
    dst->serial_number = aim_strdup(src->serial_number);
    memcpy(dst->mac, src->mac_address, 6);
    dst->manufacture_date = aim_strdup(src->manufacture_date);
    dst->label_revision = aim_strdup(src->label_version);
    dst->mac_range = 1;
    dst->manufacturer = aim_strdup("Quanta");
    dst->vendor = aim_strdup("QuantaMesh");
    return 0;
}

int
quanta_sys_eeprom_show(aim_pvs_t* pvs, quanta_sys_eeprom_t* e)
{
    aim_printf(pvs, "Product Name: %s\n", e->product_name);
    aim_printf(pvs, "Model Name: %s\n", e->model_name);
    aim_printf(pvs, "Part Number:  %s\n", e->part_number);
    aim_printf(pvs, "Serial Number: %s\n", e->serial_number);
    aim_printf(pvs, "Manufacture Date: %s\n", e->manufacture_date);
    aim_printf(pvs, "Label Version: %s\n", e->label_version);
    aim_printf(pvs, "MAC: %{mac}\n", e->mac_address);
    aim_printf(pvs, "Hardware Version: 0x%x (%d)\n", e->hardware_version, e->hardware_version);
    aim_printf(pvs, "Software Version: 0x%x (%d)\n", e->software_version, e->software_version);
    aim_printf(pvs, "Card Type: 0x%x (%d)\n", e->card_type, e->card_type);
    aim_printf(pvs, "CRC: 0x%.2x\n", e->crc);
    return 0;
}

int
quanta_onie_sys_eeprom_custom_format(onlp_onie_info_t* onie)
{
    char buf[512];

    if(onie == NULL) {
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d.%d.%d.%d (0x%02x%02x)",
        ((onie->diag_version[0] & 0xf0) >> 4),
        (onie->diag_version[0] & 0x0f),
        ((onie->diag_version[1] & 0xf0) >> 4),
        (onie->diag_version[1] & 0x0f),
        (onie->diag_version[2] & 0xff),
        (onie->diag_version[3] & 0xff));
    aim_free((void*) onie->diag_version);
    onie->diag_version = aim_zmalloc(strlen(buf) + 1);
    memcpy((void*) onie->diag_version, buf, strlen(buf));

    return 0;
}
