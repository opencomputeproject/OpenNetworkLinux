/*
 *  The Definition of the TlvInfo EEPROM format can be found at onie.org or
 *  github.com/onie
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <asm/byteorder.h>

#ifndef SYS_EEPROM_SIZE
#define SYS_EEPROM_SIZE         256
#endif


#define be16_to_cpu(x)  __be16_to_cpu(x)
#define cpu_to_be16(x)   __cpu_to_be16(x)

#ifdef FALSE
#undef FALSE
#endif
#define FALSE   0

#ifdef TRUE
#undef TRUE
#endif
#define TRUE    (!FALSE)
typedef unsigned char bool;

#define min(x, y)          ((x) > (y) ? (y) : (x))

/*
 * Tlvinf header: Layout of the header for the TlvInfo format
 *
 * See the end of this file for details of this eeprom format
 */
struct __attribute__ ((__packed__)) tlvinfo_header_s {
    char    signature[8];   /* 0x00 - 0x07 EEPROM Tag "TlvInfo" */
    uint8_t      version;  /* 0x08        Structure version */
    u_int16_t     totallen; /* 0x09 - 0x0A Length of all data which follows */
};
typedef struct tlvinfo_header_s tlvinfo_header_t;

// Header Field Constants
#define TLV_INFO_ID_STRING      "TlvInfo"
#define TLV_INFO_VERSION        0x01
#define TLV_TOTAL_LEN_MAX       (SYS_EEPROM_SIZE - sizeof(tlvinfo_header_t))

/*
 * TlvInfo TLV: Layout of a TLV field
 */
struct __attribute__ ((__packed__)) tlvinfo_tlv_s {
    uint8_t  type;
    uint8_t  length;
    uint8_t  value[0];
};
typedef struct tlvinfo_tlv_s tlvinfo_tlv_t;

/* Maximum length of a TLV value in bytes */
#define TLV_VALUE_MAX_LEN        255

/*
 *  Struct for displaying the TLV codes and names.
 */
struct tlv_code_desc {
    uint8_t m_code;
    char    *m_name;
    int      m_type;
};

#define TLV_CODE_CRC_32         0xFE
/*
 * List of TLV Type.
 */
#define TLV_TYPE_STRING		0x11
#define TLV_TYPE_MAC		0x12
#define TLV_TYPE_UINT8		0x13
#define TLV_TYPE_UINT16		0x14
#define TLV_TYPE_UINT32		0x15
#define TLV_TYPE_DATE		0x16

static inline char* tlv_code2name(struct tlv_code_desc *tlv_code_list, uint8_t type)
{
    char* name = "Unknown";
    int   i;

    for (i = 0; tlv_code_list[i].m_name ; i++) {
	if (tlv_code_list[i].m_code == type) {
	    name = tlv_code_list[i].m_name;
	    break;
	}
    }
    return name;
}
static inline int tlv_code2type(struct tlv_code_desc *tlv_code_list, uint8_t code)
{
    int type = -1;
    int   i;

    for (i = 0; tlv_code_list[i].m_name ; i++) {
	if (tlv_code_list[i].m_code == code) {
	    type = tlv_code_list[i].m_type;
	    break;
	}
    }
    return type;
}


/*
 * The max decode value is currently for the 'raw' type or the 'vendor
 * extension' type, both of which have the same decode format.  The
 * max decode string size is computed as follows:
 *
 *   strlen(" 0xFF") * TLV_VALUE_MAX_LEN + 1
 *
 */
#define TLV_DECODE_VALUE_MAX_LEN    ((5 * TLV_VALUE_MAX_LEN) + 1)


// Access functions to onie_tlvinfo
void tlvinfo_show_eeprom        (struct tlv_code_desc *tlv_code_list, uint8_t *eeprom);
void tlvinfo_update_eeprom_header(struct tlv_code_desc *tlv_code_list, uint8_t *eeprom);

bool tlvinfo_find_tlv           (struct tlv_code_desc *tlv_code_list, uint8_t *eeprom, uint8_t tcode, int *eeprom_index);
bool tlvinfo_delete_tlv         (struct tlv_code_desc *tlv_code_list, uint8_t *eeprom, uint8_t code);
bool tlvinfo_add_tlv            (struct tlv_code_desc *tlv_code_list, uint8_t *eeprom, int tcode, char * strval);
bool tlvinfo_decode_tlv         (struct tlv_code_desc *tlv_code_list, uint8_t *eeprom, uint8_t tcode, char* value);
void tlvinfo_show_tlv_code_list (struct tlv_code_desc *tlv_code_list);
bool tlvinfo_is_valid_tlvinfo_header(uint8_t *eeprom);

