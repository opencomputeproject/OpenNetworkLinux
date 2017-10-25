/**
 * These parts must be special-cased by vendor and model
 * due to nonstandard eeprom contents.
 */
#include <sff/sff.h>

typedef struct sff_ns_entry_s {
    const char* vendor;
    const char* model;
    sff_module_type_t mt;
    int len;
} sff_ns_entry_t;

static sff_ns_entry_t nonstandard_modules__[] =
    {
        { "CISCO-OEM       ", "QSFP-4SFP+-CU2M ", SFF_MODULE_TYPE_40G_BASE_CR4, 2 },
        { "CISCO-OEM       ", "QSFP-4SFP+-CU3M ", SFF_MODULE_TYPE_40G_BASE_CR4, 3 },
        { "CISCO-OEM       ", "QSFP-4SFP+-CU5M ", SFF_MODULE_TYPE_40G_BASE_CR4, 5 },
        { "Mellanox        ", "MC2206130-001   ", SFF_MODULE_TYPE_40G_BASE_CR4, 1 },
        { "OEM             ", "F4M-QSSFP-C-2-30", SFF_MODULE_TYPE_40G_BASE_CR4, 2 },
        {},
    };


int
sff_nonstandard_lookup(sff_info_t* info)
{
    sff_ns_entry_t* p;
    for(p = nonstandard_modules__; p->vendor; p++) {
        if(!strcmp(info->vendor, p->vendor) && !strcmp(info->model, p->model)) {
            sff_info_from_module_type(info, info->sfp_type, p->mt);
            info->length = p->len;
            return 0;
        }
    }
    return -1;
}
