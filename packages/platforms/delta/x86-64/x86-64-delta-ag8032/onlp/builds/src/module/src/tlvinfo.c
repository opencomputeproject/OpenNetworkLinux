/*
 *  The Definition of the TlvInfo EEPROM format can be found at onie.org or
 *  github.com/onie
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "tlvinfo.h"

static uint32_t *global_crc32_table;

static uint32_t* crc32_filltable(uint32_t *crc_table, int endian)
{
	uint32_t polynomial = endian ? 0x04c11db7 : 0xedb88320;
	uint32_t c;
	int i, j;

	if (!crc_table)
		crc_table = malloc(256 * sizeof(uint32_t));

	for (i = 0; i < 256; i++) {
		c = endian ? (i << 24) : i;
		for (j = 8; j; j--) {
			if (endian)
				c = (c&0x80000000) ? ((c << 1) ^ polynomial) : (c << 1);
			else
				c = (c&1) ? ((c >> 1) ^ polynomial) : (c >> 1);
		}
		*crc_table++ = c;
	}

	return crc_table - 256;
}

static uint32_t crc32_block_endian0(uint32_t val, const void *buf, unsigned len, uint32_t *crc_table)
{
	const void *end = (uint8_t*)buf + len;

	while (buf != end) {
		val = crc_table[(uint8_t)val ^ *(uint8_t*)buf] ^ (val >> 8);
		buf = (uint8_t*)buf + 1;
	}
	return val;
}

static unsigned long crc32 (unsigned long crc, const unsigned char *buf, unsigned len)
{
    if (!global_crc32_table) {
        global_crc32_table = crc32_filltable(NULL, 0);
    }
    return crc32_block_endian0( crc ^ 0xffffffffL, buf, len, global_crc32_table) ^ 0xffffffffL;
}


static int os_memcpy (void *pdst, void *psrc, int len)
{
	int i;
	char *dst = (char *)pdst;
	char *src = (char *)psrc;

	for (i = 0 ; i < len ; i ++)
		dst[i] = src[i];
	return len;
}

static inline int is_multicast_ether_addr(const u_int8_t *addr)
{
    return 0x01 & addr[0];
}

static inline int is_zero_ether_addr(const u_int8_t *addr)
{
    return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

static inline int is_valid_ether_addr(const u_int8_t *addr)
{
    return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
}

static int set_bytes(char *buf, const char *string, int * converted_accum)
{
    char *p = (char *) string;
    int   i;
    uint  byte;

    if (!p) {
	printf("ERROR: NULL string passed in.\n");
	return -1;
    }
    /* Convert string to bytes */
    for (i = 0, p = (char *)string; (i < TLV_VALUE_MAX_LEN) && (*p != 0);
	 i++) {
	while ((*p == ' ') || (*p == '\t') || (*p == ',') ||
	       (*p == ';')) {
	    p++;
	}
	if (*p != 0) {
	    if (!isdigit(*p)) {
		printf("ERROR: Non-digit found in byte string: (%s)\n", string);
		return -1;
	    }
	    byte = strtoul(p, &p, 0);
	    if (byte >= 256) {
		printf("ERROR: The value specified is greater than 255: (%u) " \
		       "in string: %s\n", byte, string);
		return -1;
	    }
	    buf[i] = byte & 0xFF;
	}
    }
    if ((i == TLV_VALUE_MAX_LEN) && (*p != 0)) {
	printf("ERROR: Trying to assign too many bytes "
	       "(max: %d) in string: %s\n", TLV_VALUE_MAX_LEN, string);
	return -1;
    }
    *converted_accum = i;
    return 0;
}

/*
 *  set_date
 *
 *  Validates the format of the data string
 *
 *  This function takes a pointer to a date string (i.e. MM/DD/YYYY hh:mm:ss)
 *  and validates that the format is correct. If so the string is copied
 *  to the supplied buffer.
 */
static int set_date(char *buf, const char *string)
{
    int i;

    if (!string) {
	printf("ERROR: NULL date string passed in.\n");
	return -1;
    }
    if (strlen(string) != 19) {
	printf("ERROR: Date strlen() != 19 -- %d\n", (int)strlen(string));
	printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n", string);
	return -1;
    }
    for (i = 0; string[i] != 0; i++) {
	switch (i) {
	case 2:
	case 5:
	    if (string[i] != '/') {
		printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n",
		       string);
		return -1;
	    }
	    break;
	case 10:
	    if (string[i] != ' ') {
		printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n",
		       string);
		return -1;
	    }
	    break;
	case 13:
	case 16:
	    if (string[i] != ':') {
		printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n",
		       string);
		return -1;
	    }
	    break;
	default:
	    if (!isdigit(string[i])) {
		printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n",
		       string);
		return -1;
	    }
	    break;
	}
    }
    strcpy(buf, string);
    return 0;
}

/*
 *  is_valid_tlv
 *
 *  Perform basic sanity checks on a TLV field. The TLV is pointed to
 *  by the parameter provided.
 *      1. The type code is not reserved (0x00 or 0xFF)
 */
static inline bool is_valid_tlv(tlvinfo_tlv_t *tlv)
{
    //return((tlv->type != 0x00) && (tlv->type != 0xFF));
    return((tlv->type != 0xFF));
}

/*
 *  is_hex
 *
 *  Tests if character is an ASCII hex digit
 */
static inline char is_hex(char p)
{
    return (((p >= '0') && (p <= '9')) ||
	    ((p >= 'A') && (p <= 'F')) ||
	    ((p >= 'a') && (p <= 'f')));
}

/*
 *  set_mac
 *
 *  Converts a string MAC address into a binary buffer.
 *
 *  This function takes a pointer to a MAC address string
 *  (i.e."XX:XX:XX:XX:XX:XX", where "XX" is a two-digit hex number).
 *  The string format is verified and then converted to binary and
 *  stored in a buffer.
 */
static int set_mac(char *buf, const char *string)
{
    char *p = (char *) string;
    int   i;
    int   err = 0;
    char *end;

    if (!p) {
	printf("ERROR: NULL mac addr string passed in.\n");
	return -1;
    }
    if (strlen(p) != 17) {
	printf("ERROR: MAC address strlen() != 17 -- %d\n", (int)strlen(p));
	printf("ERROR: Bad MAC address format: %s\n", string);
	return -1;
    }
    for (i = 0; i < 17; i++) {
	if ((i % 3) == 2) {
	    if (p[i] != ':') {
		err++;
		printf("ERROR: mac: p[%i] != :, found: `%c'\n",
		       i, p[i]);
		break;
	    }
	    continue;
	} else if (!is_hex(p[i])) {
	    err++;
	    printf("ERROR: mac: p[%i] != hex digit, found: `%c'\n",
		   i, p[i]);
	    break;
	}
    }
    if (err != 0) {
	printf("ERROR: Bad MAC address format: %s\n", string);
	return -1;
    }
    /* Convert string to binary */
    for (i = 0, p = (char *)string; i < 6; i++) {
	buf[i] = p ? strtoul(p, &end, 16) : 0;
	if (p) {
	    p = (*end) ? end + 1 : end;
	}
    }
    if (!is_valid_ether_addr((u_int8_t *)buf)) {
	printf("ERROR: MAC address must not be 00:00:00:00:00:00, "
	       "a multicast address or FF:FF:FF:FF:FF:FF.\n");
	printf("ERROR: Bad MAC address format: %s\n", string);
	return -1;
    }
    return 0;
}

/*
 *  is_valid_tlvinfo_header
 *
 *  Perform sanity checks on the first 11 bytes of the TlvInfo EEPROM
 *  data pointed to by the parameter:
 *      1. First 8 bytes contain null-terminated ASCII string "TlvInfo"
 *      2. Version byte is 1
 *      3. Total length bytes contain value which is less than or equal
 *         to the allowed maximum (2048-11)
 *
 */
static inline bool is_valid_tlvinfo_header(tlvinfo_header_t *hdr)
{
    int max_size = TLV_TOTAL_LEN_MAX;
    return((strcmp(hdr->signature, TLV_INFO_ID_STRING) == 0) &&
	   (hdr->version == TLV_INFO_VERSION) &&
	   (be16_to_cpu(hdr->totallen) <= max_size) );
}

bool tlvinfo_is_valid_tlvinfo_header(u_int8_t *eeprom)
{
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	return is_valid_tlvinfo_header (eeprom_hdr);
}

/*
 *  decode_tlv_value
 *
 *  Decode a single TLV value into a string.

 *  The validity of EEPROM contents and the TLV field have been verified
 *  prior to calling this function.
 */
#define DECODE_NAME_MAX     20

static void decode_tlv_value(struct tlv_code_desc *tlv_code_list, tlvinfo_tlv_t * tlv, char* value)
{
    int i;

    switch (tlv_code2type(tlv_code_list, tlv->type)) {
    case TLV_TYPE_DATE:
    case TLV_TYPE_STRING:
	os_memcpy(value, tlv->value, tlv->length);
	value[tlv->length] = 0;
	break;
    case TLV_TYPE_MAC:
	sprintf(value, "%02X:%02X:%02X:%02X:%02X:%02X",
		tlv->value[0], tlv->value[1], tlv->value[2],
		tlv->value[3], tlv->value[4], tlv->value[5]);
	break;
    case TLV_TYPE_UINT8:
	sprintf(value, "%u", tlv->value[0]);
	break;
    case TLV_TYPE_UINT16:
	sprintf(value, "%u", (tlv->value[0] << 8) | tlv->value[1]);
	break;
    case TLV_TYPE_UINT32:
	sprintf(value, "0x%02X%02X%02X%02X",
		tlv->value[0], tlv->value[1], tlv->value[2],
		tlv->value[3]);
	break;
    default:
	value[0] = 0;
	for (i = 0; (i < (TLV_DECODE_VALUE_MAX_LEN/5)) && (i < tlv->length);
	     i++) {
	    sprintf(value, "%s 0x%02X", value, tlv->value[i]);
	}
	break;
    }

}

/*
 *  decode_tlv
 *
 *  Print a string representing the contents of the TLV field. The format of
 *  the string is:
 *      1. The name of the field left justified in 20 characters
 *      2. The type code in hex right justified in 5 characters
 *      3. The length in decimal right justified in 4 characters
 *      4. The value, left justified in however many characters it takes
 *  The validity of EEPROM contents and the TLV field have been verified
 *  prior to calling this function.
 */
#define DECODE_NAME_MAX     20

static void decode_tlv(struct tlv_code_desc *tlv_code_list, tlvinfo_tlv_t * tlv)
{
    char name[DECODE_NAME_MAX];
    char value[TLV_DECODE_VALUE_MAX_LEN];

    decode_tlv_value(tlv_code_list, tlv, value);

    strncpy(name, tlv_code2name(tlv_code_list, tlv->type), DECODE_NAME_MAX);
    name[DECODE_NAME_MAX-1] = 0;

    printf("%-20s 0x%02X %3d %s\n", name, tlv->type, tlv->length, value);
}

/*
 *  is_checksum_valid
 *
 *  Validate the checksum in the provided TlvInfo EEPROM data. First,
 *  verify that the TlvInfo header is valid, then make sure the last
 *  TLV is a CRC-32 TLV. Then calculate the CRC over the EEPROM data
 *  and compare it to the value stored in the EEPROM CRC-32 TLV.
 */
static bool is_checksum_valid(u_int8_t *eeprom)
{
    tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t    * eeprom_crc;
    unsigned int       calc_crc;
    unsigned int       stored_crc;

    // Is the eeprom header valid?
    if (!is_valid_tlvinfo_header(eeprom_hdr)) {
	return(FALSE);
    }

    // Is the last TLV a CRC?
    eeprom_crc = (tlvinfo_tlv_t *) &eeprom[sizeof(tlvinfo_header_t) +
					   be16_to_cpu(eeprom_hdr->totallen) -
					   (sizeof(tlvinfo_tlv_t) + 4)];
    if ((eeprom_crc->type != TLV_CODE_CRC_32) || (eeprom_crc->length != 4)) {
	return(FALSE);
    }

    // Calculate the checksum
    calc_crc = crc32(0, (void *)eeprom, sizeof(tlvinfo_header_t) +
		     be16_to_cpu(eeprom_hdr->totallen) - 4);
    stored_crc = ((eeprom_crc->value[0] << 24) | (eeprom_crc->value[1] << 16) |
		  (eeprom_crc->value[2] <<  8) | eeprom_crc->value[3]);
    return(calc_crc == stored_crc);
}

/*
 *  update_crc
 *
 *  This function updates the CRC-32 TLV. If there is no CRC-32 TLV, then
 *  one is added. This function should be called after each update to the
 *  EEPROM structure, to make sure the CRC is always correct.
 */
static void update_crc(u_int8_t *eeprom)
{
    tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t    * eeprom_crc;
    unsigned int       calc_crc;

    // Is the eeprom header valid?
    if (!is_valid_tlvinfo_header(eeprom_hdr)) {
	return;
    }
    // Is the last TLV a CRC?
    eeprom_crc = (tlvinfo_tlv_t *) &eeprom[sizeof(tlvinfo_header_t) +
					   be16_to_cpu(eeprom_hdr->totallen) -
					   (sizeof(tlvinfo_tlv_t) + 4)];
    if (eeprom_crc->type != TLV_CODE_CRC_32) {
	if ((be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + 4) >
	    TLV_TOTAL_LEN_MAX) {
	    return;
	}
	eeprom_crc = (tlvinfo_tlv_t *) &eeprom[sizeof(tlvinfo_header_t) +
					       be16_to_cpu(
						   eeprom_hdr->totallen)];
	eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) +
					   sizeof(tlvinfo_tlv_t) + 4);
	eeprom_crc->type = TLV_CODE_CRC_32;
    }
    eeprom_crc->length = 4;

    // Calculate the checksum
    calc_crc = crc32(0, (void *)eeprom,
		     sizeof(tlvinfo_header_t) +
		     be16_to_cpu(eeprom_hdr->totallen) - 4);
    eeprom_crc->value[0] = (calc_crc >> 24) & 0xFF;
    eeprom_crc->value[1] = (calc_crc >> 16) & 0xFF;
    eeprom_crc->value[2] = (calc_crc >>  8) & 0xFF;
    eeprom_crc->value[3] = (calc_crc >>  0) & 0xFF;
    return;
}

/*
 *  show_eeprom
 *
 *  Display the contents of the EEPROM
 */
void tlvinfo_show_eeprom (struct tlv_code_desc *tlv_code_list, u_int8_t *eeprom)
{
    int tlv_end;
    int curr_tlv;
    tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t    * eeprom_tlv;

    if ( !is_valid_tlvinfo_header(eeprom_hdr) ) {
	printf("EEPROM does not contain data in a valid TlvInfo format.\n");
	return;
    }

    printf("TlvInfo Header:\n");
    printf("   Id String:    %s\n", eeprom_hdr->signature);
    printf("   Version:      %d\n", eeprom_hdr->version);
    printf("   Total Length: %d\n", be16_to_cpu(eeprom_hdr->totallen));
    printf("TLV Name             Code Len Value\n");
    printf("-------------------- ---- --- -----\n");
    curr_tlv = sizeof(tlvinfo_header_t);
    tlv_end  = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
    while (curr_tlv < tlv_end) {
	eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[curr_tlv];
	if (!is_valid_tlv(eeprom_tlv)) {
	    printf("Invalid TLV field starting at EEPROM offset %d\n",
		   curr_tlv);
	    return;
	}
	decode_tlv(tlv_code_list, eeprom_tlv);
	curr_tlv += sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
    }
    printf("Checksum is %s.\n", is_checksum_valid(eeprom) ? "valid" :
	   "invalid");

#ifdef DEBUG
    printf("EEPROM dump: (0x%x bytes)", SYS_EEPROM_SIZE);
    for (i = 0; i < SYS_EEPROM_SIZE; i++) {
	if ((i % 16) == 0)
	    printf("\n%02X: ", i);
	printf("%02X ", eeprom[i]);
    }
    printf("\n");
#endif
    return;
}

#if 0
/*
 *  read_eeprom
 *
 *  Read the EEPROM into memory, if it hasn't already been read.
 */
int read_eeprom(u_int8_t *eeprom)
{
    int ret;
    tlvinfo_header_t *eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t *eeprom_tlv = (tlvinfo_tlv_t *)&eeprom[
	sizeof(tlvinfo_header_t)];

    if (has_been_read)
	return 0;

    /* Read the header */
    ret = read_sys_eeprom((void *)eeprom_hdr, 0, sizeof(tlvinfo_header_t));
    /* If the header was successfully read, read the TLVs */
    if ((ret == 0) && is_valid_tlvinfo_header(eeprom_hdr)) {
	ret = read_sys_eeprom((void *)eeprom_tlv, sizeof(tlvinfo_header_t),
			      be16_to_cpu(eeprom_hdr->totallen));
    }
    // If the contents are invalid, start over with default contents
    if (!is_valid_tlvinfo_header(eeprom_hdr))
	fprintf(stderr,
                "Notice:  Invalid TLV header found.  Using default contents.\n");
    if (!is_checksum_valid(eeprom))
	fprintf(stderr,
                "Notice:  Invalid TLV checksum found.  Using default contents.\n");
    if ( !is_valid_tlvinfo_header(eeprom_hdr) || !is_checksum_valid(eeprom) ){
	strcpy(eeprom_hdr->signature, TLV_INFO_ID_STRING);
	eeprom_hdr->version = TLV_INFO_VERSION;
	eeprom_hdr->totallen = cpu_to_be16(0);
	update_crc(eeprom);
	/* Note that the contents of the hardware is not valid */
	hw_eeprom_valid = 0;
    }
    has_been_read = 1;

#ifdef DEBUG
    show_eeprom(eeprom);
#endif
    return ret;
}

/*
 *  prog_eeprom
 *  Write the EEPROM data from CPU memory to the hardware.
 */
int prog_eeprom(u_int8_t * eeprom)
{
    int ret = 0;
    tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
    int eeprom_len;

    eeprom_len = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
    ret = write_sys_eeprom(eeprom, eeprom_len);
    if (ret) {
	printf("Programming failed.\n");
	return -1;
    }

    /* After writing the HW contents are valid */
    hw_eeprom_valid = 1;

    printf("Programming passed.\n");
    return 0;
}
#endif

/*
 *  tlvinfo_find_tlv
 *
 *  This function finds the TLV with the supplied code in the EERPOM.
 *  An offset from the beginning of the EEPROM is returned in the
 *  eeprom_index parameter if the TLV is found.
 */

bool tlvinfo_find_tlv (struct tlv_code_desc *tlv_code_list, u_int8_t *eeprom, u_int8_t tcode, int *eeprom_index)
{
    tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t    * eeprom_tlv;
    int eeprom_end;

    // Make sure the EEPROM contents are valid
    if (!is_valid_tlvinfo_header(eeprom_hdr) || !is_checksum_valid(eeprom)) {
	return(FALSE);
    }
    // Search through the TLVs, looking for the first one which matches the
    // supplied type code.
    *eeprom_index = sizeof(tlvinfo_header_t);
    eeprom_end = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
    while (*eeprom_index < eeprom_end) {
	eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[*eeprom_index];
	if (!is_valid_tlv(eeprom_tlv)) {
	    return(FALSE);
	}
	if (eeprom_tlv->type == tcode) {
	    return(TRUE);
	}
	*eeprom_index += sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
    }
    return(FALSE);
}

/*
 *  tlvinfo_decode_tlv
 *
 *  This function finds the TLV with the supplied code in the EERPOM
 *  and decodes the value into the buffer provided.
 */
bool tlvinfo_decode_tlv (struct tlv_code_desc *tlv_code_list, u_int8_t *eeprom, u_int8_t tcode, char* value)
{
    int eeprom_index;
    tlvinfo_tlv_t * eeprom_tlv;

    // Find the TLV and then decode it
    if (tlvinfo_find_tlv(tlv_code_list, eeprom, tcode, &eeprom_index)) {
        eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
        decode_tlv_value(tlv_code_list, eeprom_tlv, value);
        return TRUE;
    }

    return FALSE;
}

/*
 *  tlvinfo_delete_tlv
 *
 *  This function deletes the TLV with the specified type code from the
 *  EEPROM.
 */
bool tlvinfo_delete_tlv (struct tlv_code_desc *tlv_code_list, u_int8_t *eeprom, u_int8_t code)
{
    int eeprom_index;
    int tlength;
    tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t * eeprom_tlv;

    if ( !is_valid_tlvinfo_header(eeprom_hdr) || !is_checksum_valid(eeprom) ){
	strcpy(eeprom_hdr->signature, TLV_INFO_ID_STRING);
	eeprom_hdr->version = TLV_INFO_VERSION;
	eeprom_hdr->totallen = cpu_to_be16(0);
	update_crc(eeprom);
    }

    // Find the TLV and then move all following TLVs "forward"
    if (tlvinfo_find_tlv(tlv_code_list, eeprom, code, &eeprom_index)) {
	eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
	tlength = sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
	os_memcpy(&eeprom[eeprom_index], &eeprom[eeprom_index+tlength],
	       sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen) -
	       eeprom_index - tlength);
	eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) -
					   tlength);
	update_crc(eeprom);
	return(TRUE);
    }
    return(FALSE);
}

/*
 *  tlvinfo_add_tlv
 *
 *  This function adds a TLV to the EEPROM, converting the value (a string) to
 *  the format in which it will be stored in the EEPROM.
 */
#define MAX_TLV_VALUE_LEN   256
bool tlvinfo_add_tlv (struct tlv_code_desc *tlv_code_list, u_int8_t *eeprom, int tcode, char * strval)
{
    tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t * eeprom_tlv;
    int new_tlv_len = 0;
    u_int32_t value;
    char data[MAX_TLV_VALUE_LEN];
    int eeprom_index;
    int max_size = TLV_TOTAL_LEN_MAX;

    if ( !is_valid_tlvinfo_header(eeprom_hdr) || !is_checksum_valid(eeprom) ){
	strcpy(eeprom_hdr->signature, TLV_INFO_ID_STRING);
	eeprom_hdr->version = TLV_INFO_VERSION;
	eeprom_hdr->totallen = cpu_to_be16(0);
	update_crc(eeprom);
    }

    // Encode each TLV type into the format to be stored in the EERPOM
    switch (tlv_code2type(tlv_code_list, tcode)) {
    case TLV_TYPE_STRING:
	strncpy(data, strval, MAX_TLV_VALUE_LEN);
	new_tlv_len = min(MAX_TLV_VALUE_LEN, strlen(strval));
	break;
    case TLV_TYPE_UINT8:
	value = strtoul(strval, NULL, 0);
	if (value >= 256) {
	    printf("ERROR: must be 255 or less. Value " \
		   "supplied: %u", value);
	    return(FALSE);
	}
	data[0] = value & 0xFF;
	new_tlv_len = 1;
	break;
    case TLV_TYPE_UINT16:
	value = strtoul(strval, NULL, 0);
	if (value >= 65536) {
	    printf("ERROR: must be 65535 or less. Value " \
		   "supplied: %u", value);
	    return(FALSE);
	}
	data[0] = (value >> 8) & 0xFF;
	data[1] = value & 0xFF;
	new_tlv_len = 2;
	break;

    case TLV_TYPE_UINT32:
	value = strtoul(strval, NULL, 0);
	data[0] = (value >> 24) & 0xFF;
	data[1] = (value >> 16) & 0xFF;
	data[2] = (value >> 8) & 0xFF;
	data[3] = value & 0xFF;
	new_tlv_len = 4;
	break;

    case TLV_TYPE_DATE:
	if (set_date(data, strval) != 0) {
	    return(FALSE);
	}
	new_tlv_len = 19;
	break;
    case TLV_TYPE_MAC:
	if (set_mac(data, strval) != 0) {
	    return(FALSE);
	}
	new_tlv_len = 6;
	break;
    default:
	if (set_bytes(data, strval, &new_tlv_len) != 0 ) {
	    return(FALSE);
	}
	break;
    }

    // Is there room for this TLV?
    if ((be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) +
	 new_tlv_len) > max_size) {
	printf("ERROR: There is not enough room in the EERPOM to save data.\n");
	return(FALSE);
    }

    // Add TLV at the end, overwriting CRC TLV if it exists
    if (tlvinfo_find_tlv(tlv_code_list, eeprom, TLV_CODE_CRC_32, &eeprom_index)) {
	eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen)
					   - sizeof(tlvinfo_tlv_t) - 4);
    } else {
	eeprom_index = sizeof(tlvinfo_header_t) +
	    be16_to_cpu(eeprom_hdr->totallen);
    }
    eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
    eeprom_tlv->type = tcode;
    eeprom_tlv->length = new_tlv_len;
    os_memcpy(eeprom_tlv->value, data, new_tlv_len);

    // Update the total length and calculate (add) a new CRC-32 TLV
    eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) +
				       sizeof(tlvinfo_tlv_t) + new_tlv_len);
    update_crc(eeprom);

    return(TRUE);
}

void tlvinfo_update_eeprom_header(struct tlv_code_desc *tlv_code_list, u_int8_t *eeprom)
{
    tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;

    strcpy(eeprom_hdr->signature, TLV_INFO_ID_STRING);
    eeprom_hdr->version = TLV_INFO_VERSION;
    eeprom_hdr->totallen = cpu_to_be16(0);
    update_crc(eeprom);
}

/*
 *  show_tlv_code_list - Display the list of TLV codes and names
 */
void tlvinfo_show_tlv_code_list (struct tlv_code_desc *tlv_code_list)
{
    int i;

    printf("TLV Code    TLV Name\n");
    printf("========    =================\n");
    for (i = 0; tlv_code_list[i].m_name ; i++) {
	printf("0x%02x        %s\n", tlv_code_list[i].m_code, tlv_code_list[i].m_name);
    }
}

