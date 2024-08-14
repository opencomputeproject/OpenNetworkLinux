/******************************
*
*  Southbound API definition for
*  Open Optical Monitoring (OOM) initiative under the 
*  umbrella of OCP.
*
*  Copyright 2015  Finisar Inc.
*
*  LIKELY TO CHANGE, no promises of compatibility with future 
*  versions is made or implied
*
*  Version: 0.4, January 28, 2016 (added oom_get_port(n))
*  Author: Don Bollinger
*
*******************************/

#include <stdint.h>

/*  Discovery definitions */

/* 
 * list of valid port types
 * Values are from SFF-8436 spec (rev 4.8, page 74)
 * note OOM_PORT_TYPE_NOT_PRESENT to indicate no 
 * module is present in this port
 */

typedef enum oom_port_class_e {
	OOM_PORT_CLASS_UNKNOWN = 0x00,
	OOM_PORT_CLASS_SFF = 0x01,
	OOM_PORT_CLASS_CFP = 0x02,
} oom_port_class_t;


/* Define the elements of a port
 * Note: seq_num is an implementation defined magic 
 * number to detect that the module
 * on this port has been removed/inserted since this 
 * port was last accessed
 * port_flags is implementation dependent, for use 
 * by the underlying NOS and switch
 * port_flags should NOT be used or modified by the 
 * decode layer or above
 */
typedef struct oom_port_s {
	void *handle;     /* opaque handle for this port */
	oom_port_class_t oom_class;  /* class is SFF or CFP */
	char name[32];  /* 32 bytes for a human readable name */ 
} oom_port_t;


/*
 * get the list of available ports, as an array 
 * of oom_port_t structs.  'listsize' indicates the
 * number of ports that will fit in the portlist[] array.
 *
 * If there are listsize or fewer ports, and the portlist[]
 * array is new or changed, then the shim populates
 * the portlist structure, and returns the number of valid
 * ports in the portlist (which may be less than listsize).
 *
 * If the portlist structure is already current, then the 
 * shim does not modify portlist[], and returns 0 (no change).
 * This is intended as a way to poll for changes to the ports 
 * (eg modules added, deleted, replaced, etc)
 *
 * If there are more than listize ports, the shim
 * returns -ENOMEM, to indicate that a larger portlist[]
 * array is required.
 *
 * If called as oom_get_portlist(NULL, 0), then the shim will
 * return the number of ports, ie the minimum value of listsize.  Note
 * that some implementations can dynamically add ports, so the return
 * value is a very good hint, but not a guarantee as to the required
 * size of the portlist[] structure.
 *
 * Thus:  a positive return value indicates the number of valid ports
 *        a zero return value indicates no change to the portlist
 *        a negative return value indicates an error
 */
int oom_get_portlist(oom_port_t portlist[], int listsize);


/* Read/write control "PINS" function definitions */

/* note that these 'control pins' functions are in limbo right 
 * now, and may not be implemented until later, if at all.
 * Developers of "southbound shims" may choose to delay 
 * implementation of oom_{get, set}_function()
 */

/* list of functions that can be controlled */
typedef enum oom_functions_e {
	OOM_FUNCTIONS_TX_FAULT,
	OOM_FUNCTIONS_TX_DISABLE,
	OOM_FUNCTIONS_MODULE_ABSENT,
	OOM_FUNCTIONS_RS0,
	OOM_FUNCTIONS_RS1,
     /* more control functions to be defined here */
	OOM_FUNCTIONS_RXLOSS_OF_SIG,
	OOM_FUNCTIONS_LAST = OOM_FUNCTIONS_RXLOSS_OF_SIG,
        OOM_FUNCTIONS_COUNT,
	OOM_FUNCTIONS_INVALID = -1,
} oom_functions_t;

/* 
 * read a function
 * rv will be 1 for asserted or enabled, 0 if not
 * returns 0 on success, or negative error code
 */
int oom_get_function(oom_port_t* port, oom_functions_t function, int* rv);

/* 
 * write a function
 * value should be 1 for asserted or enabled, 0 if not
 * returns 0 on success, or negative error code
 */
int oom_set_function(oom_port_t* port, oom_functions_t function, int value);


/* Read/write EEPROM definitions */

/* 
 * read EEPROM
 * address: the 2-wire (i2c) address, per the SFF specs, 
 *   eg A0h, A2h, A8h, etc.  (160, 162, 168 respectively.)
 *   Note that this address points to 256 bytes of data.
 *   Bytes 0-127 are intrinsic to this address space.
 *   Bytes 128-255 are the contents of the "page" selected
 *   in byte 127.  (The page select byte will be written by
 *   the driver as part of this call.  DO NOT explicitly 
 *   set byte 127 to select a page.)
 *   Thus there are 256 pages (based on one
 *   byte of page select in byte 127).  ONE of these pages 
 *   can be accessed in an EEPROM read, and the content
 *   of that page will be at offset 128-255 of the address 
 *   space.  Page contents starts at offset=128. It is NOT
 *   necessary to read the first 128 bytes of the address 
 *   space to access the page contents in the second 128 
 *   bytes.  See the specs for a further description of
 *   this access architecture, and the content of each page.
 *   Note:  Bytes 0-127 do not depend on the value of page.
 *   Reading 256 bytes, starting at offset 0, will read both
 *   the lower half of the address space, AND the page selected.
 * page: page of EEPROM to read from
 * offset: byte location within the address space to begin 
 *   transferring data from.  Remember, offset 0-127 reference
 *   the first 128 bytes of "address" range.  Their contents
 *   do not depend on the value of page.  Page content begins
 *   at offset=128.
 * len: the number of bytes to be read
 *   note that (offset + len) must be no more than 256, as 
 *   there are only 256 bytes of data available at A0, A2, etc 
 * data: receives the memory contents
 * returns: the number of bytes read, or a 
 *   negative error code
 */
int oom_get_memory_sff(oom_port_t* port, int address, int page, int offset, int len, uint8_t* data);


/* 
 * write EEPROM
 * address: the 2-wire (i2c) address, per the SFF specs, 
 *   eg A0h, A2h, A8h, etc.  (160, 162, 168 respectively.)
 *   Note that this address points to 256 bytes of data.
 *   Bytes 0-127 are intrinsic to this address space.
 *   Bytes 128-255 are the contents of the "page" selected
 *   in byte 127.  (The page select byte will be written by
 *   the driver as part of this call.  DO NOT explicitly 
 *   set byte 127 to select a page.)
 *   Thus there are 256 pages (based on one
 *   byte of page select in byte 127).  ONE of these pages 
 *   can be accessed in an EEPROM write, and data written 
 *   to that page must be written to offset 128-255 of the
 *   address space.
 *   Page contents starts at offset=128. It is NOT
 *   necessary to write the first 128 bytes of the address 
 *   space to access the page contents in the second 128 
 *   bytes.  See the specs for a further description of
 *   this access architecture, and the content of each page.
 *   Note:  Writes to bytes 0-127 will go to the lower half
 *   of the address space, and do not depend on the 
 *   value of page.  Writing 256 bytes, starting at offset 0,
 *   will write both the lower half of the address space AND
 *   the page selected.
 * page: page of EEPROM to write to
 * offset: byte location within the address space to begin 
 *   transferring data from.  Remember, offset 0-127 reference
 *   the first 128 bytes of "address" range.  Their contents
 *   do not depend on the value of page.  Page content begins
 *   at offset=128.
 * len: the number of bytes to be written
 *   note that (offset + len) must be no more than 256, as
 *   there are only 256 bytes of address space at A0, A2, etc 
 * data: data to be written to memory
 * returns: the number of bytes written, or a 
 *   negative error code
 */
int oom_set_memory_sff(oom_port_t* port, int address, int page, int offset, int len, uint8_t* data);

/* 
 * read 16 bit oriented EEPROM
 * CFP modules (for example) do not use 2-wire (i2c) addresses
 * nor do they use the page table scheme of SFP, XFP, QSFP, etc
 * their EEPROM is addressed in 16 bit words, in a 32K word
 * linear address space from 8000hex to FFFFhex 
 * (0000-7FFF are reserved for IEEE 802.3 use)
 * the interface below is for these types of modules
 * port: an OOM port structure
 * address: address between 0x8000 and 0xFFFF to begin read
 *   Each consecutive address is a 16 bit "register"
 *   (not an 8 bit byte)
 * len: number of 16 bit "registers" to read
 * data: receives the memory contents
 * returns the number of words read, or a negative error code
 */
int oom_get_memory_cfp(oom_port_t* port, int address, int len, uint16_t* data);

/* 
 * write 16 bit oriented EEPROM
 * CFP modules (for example) do not use 2-wire (i2c) addresses
 * nor do they use the page table scheme of SFP, XFP, QSFP, etc
 * their EEPROM is addressed in 16 bit words, in a 32K word
 * linear address space from 8000hex to FFFFhex 
 * (0000-7FFF are reserved for IEEE 802.3 use)
 * the interface below is for these types of modules
 * port: an OOM port structure
 * address: address between 0x8000 and 0xFFFF to begin write
 * Each consecutive address is a 16 bit "register"
 * (not an 8 bit byte)
 * len: number of 16 bit "registers" to write
 * data: data to be written to memory
 * returns the number of words written, or a negative error code
 */
int oom_set_memory_cfp(oom_port_t* port, int address, int len, uint16_t* data);
