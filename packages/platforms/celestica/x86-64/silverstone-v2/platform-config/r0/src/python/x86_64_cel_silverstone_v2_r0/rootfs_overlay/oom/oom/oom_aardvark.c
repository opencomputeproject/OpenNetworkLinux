/*
 * Aardvark Southbound shim for OOM
 * Based on Aardvark C API
 *
 * Uses Finisar "eep" files to provide data
 * Place an eep file in the "./module_data/<n>" to
 * provide simulation data for port <n>
 * Currently limited to 6 ports (implementation hack for simplicity)
 */

/** AARDVARK Code **/

//=========================================================================
// INCLUDES
//=========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oom_south.h"
#include <errno.h>

#include "aardvark.h"
#include "oom_internal.h"

//=========================================================================
// oom_get_portlist
//=========================================================================
int oom_get_portlist(oom_port_t portlist[], int listsize) 
{
    /* Aardvark data structures */
    u16 ports[16];
    u32 unique_ids[16];
    int nelem = 16;

    // Find all the attached devices
    int count = aa_find_devices_ext(nelem, ports, nelem, unique_ids);
    int i;

    if ((portlist == NULL) && (listsize == 0)) {  /* asking # of ports */
        return(count);
    }

    /* Aardvark code, OK to have more devices than room for them */
    if (count > nelem)  count = nelem; 

    /* OOM bounces with -ENOMEM if there are more ports than space */
    if (listsize < count) {   /* not enough room */
	return(-ENOMEM);
    }

    printf("%d device(s) found:\n", count);

    // Print the information on each device
    // And build an OOM port, put it in the portlist
    for (i = 0; i < count; ++i) {
        // Determine if the device is in-use
        const char *status = "(avail) ";
        if (ports[i] & AA_PORT_NOT_FREE) {
            ports[i] &= ~AA_PORT_NOT_FREE;
            status = "(in-use)";
        }

        // Display device port number, in-use status, and serial number
        printf(" Aardvark port=%-3d %s (%04d-%06d)\n",
               ports[i], status,
               unique_ids[i] / 1000000,
               unique_ids[i] % 1000000);
	portlist[i].handle = (void *)(uintptr_t) i;
	portlist[i].oom_class = OOM_PORT_CLASS_SFF;
	sprintf(portlist[i].name, "%04d-%06d", unique_ids[i]/1000000,
					       unique_ids[i]%1000000);
    }
    return count;
}


//=========================================================================
// CONSTANTS
//=========================================================================
#define PAGE_SIZE   8
#define BUS_TIMEOUT 150  // ms


//=========================================================================
// FUNCTION
//=========================================================================
int _writeMemory (Aardvark handle, u08 device, u08 addr, u16 length,
                          uint8_t* data)
{
    u16 i;
    u08 data_out[1+length];
    int retval;

    // printf("aa_writeMemory, handle: %d, device: %x, addr: %d,
    //           length: %d, data[0]: %x\n",
    //           handle, device, addr, length, data[0]);

    // Write to the I2C EEPROM
    // Set the offset (addr) as the first byte
    data_out[0] = addr;

    // copy the data into the rest of the packet
    for (i = 0; i < length; i++) {
        data_out[i+1] = data[i];
    }

    // Write the address and data
    retval = aa_i2c_write(handle, device, AA_I2C_NO_FLAGS, length+1, data_out);
    aa_sleep_ms(10);
    return(retval);
}


int _readMemory (Aardvark handle, u08 device, u08 addr, u16 length, uint8_t* data)
{
    int count;

    // Write the address
    aa_i2c_write(handle, device, AA_I2C_NO_STOP, 1, &addr);

    // read the data
    count = aa_i2c_read(handle, device, AA_I2C_NO_FLAGS, length, data);
    if (count < 0) {
        printf("error: %s\n", aa_status_string(count));
        return (count);
    }
    if (count == 0) {
        printf("error: no bytes read\n");
        printf("...check cabling, power to the eval board\n");
        return (count);
    }
    else if (count != length) {
        printf("error: read %d bytes (expected %d)\n", count, length);
    }

    return(count);
}


//=========================================================================
// read/write routine (common setup between the two calls)
//=========================================================================

#define READ 0
#define WRITE 1

int aa_read_write(oom_port_t* oomport, int rw, int oomaddr, \
		int offset, int oomlength, uint8_t* data)
{
    Aardvark handle;
    int port;
    int bitrate;
    u08 aa_device;
    u08 aa_addr;
    u16 aa_length;
    // int bus_timeout;
    int retval;

    port    = (int)(uintptr_t)oomport->handle;
    bitrate = 150;
    /* note we shift the i2c address right one bit to match SFF spec
     * with Aardvark addressing (lsb in SFF is included but not used */
    aa_device  = (u08)oomaddr / 2;
    aa_addr    = (u08)offset;
    aa_length  = (u16) oomlength;

    // Open the aa_device
    // printf ("opening port: %d\n", port);
    handle = aa_open(port);
    if (handle <= 0) {
        printf("Unable to open Aardvark device on port %d\n", port);
        printf("Error code = %d\n", handle);
        return -1;
    }

    // Ensure that the I2C subsystem is enabled
    aa_configure(handle,  AA_CONFIG_SPI_I2C);

    // Enable the I2C bus pullup resistors (2.2k resistors).
    // This command is only effective on v2.0 hardware or greater.
    // The pullup resistors on the v1.02 hardware are enabled by default.
    aa_i2c_pullup(handle, AA_I2C_PULLUP_BOTH);

    // Power the EEPROM using the Aardvark adapter's power supply.
    // This command is only effective on v2.0 hardware or greater.
    // The power pins on the v1.02 hardware are not enabled by default.
    aa_target_power(handle, AA_TARGET_POWER_BOTH);

    // Set the bitrate
    aa_i2c_bitrate(handle, bitrate);
    // printf("Bitrate set to %d kHz\n", bitrate);

    // Set the bus lock timeout
    aa_i2c_bus_timeout(handle, BUS_TIMEOUT);
    // bus_timeout = aa_i2c_bus_timeout(handle, BUS_TIMEOUT);
    // printf("Bus lock timeout set to %d ms\n", bus_timeout);

    // Perform the operation
    if (rw == WRITE) { 
        retval = _writeMemory(handle, aa_device, aa_addr, aa_length, data);
        // printf("Wrote to EEPROM\n");
    }
    else if (rw == READ) {   /* 0 is for read */
        retval = _readMemory(handle, aa_device, aa_addr, aa_length, data);
    }

    //Aardvark routines add an extra byte for the address, 
    //remove it from the return value (number of bytes  read/written)
    retval --;   
    
    // Close the device and exit
    aa_close(handle);
    return (retval);
}

/** AARDVARK Code **/

/* intercept memcpy, print out parameters (uncomment the printf) */
void pmemcpy(void *dest, const void *src, size_t n)
{
/*	printf("pmemcpy - dest: %d, src: %d, size: %d\n", dest, src, n); */
	memcpy(dest, src, n);
}


int oom_set_memory_sff(oom_port_t* port, int address, int page, int offset, int len, uint8_t* data)
{
	int retval;

	// set the page register if leaving lower 128 bytes
	/* */
	if ((offset + len) > 128) {
		uint8_t pagebyte[1];
		pagebyte[0] = (uint8_t) page;
		retval = aa_read_write(port, WRITE, address, 127, 1, pagebyte);
		if (retval != 1) {
			return (retval);
		}
	}
	/* */

	retval = aa_read_write(port, WRITE, address, offset, len, data);
	return(retval);
}


int oom_get_memory_sff(oom_port_t* port, int address, int page, int offset, int len, uint8_t* data)
{
	int retval;

	// set the page register if leaving lower 128 bytes
	/* */
	if ((offset + len) > 128) {
		uint8_t pagebyte[1];
		pagebyte[0] = (uint8_t) page;
		retval = aa_read_write(port, WRITE, address, 127, 1, pagebyte);
		if (retval != 1) {
			return (retval);
		}
	}
	/* */

	retval = aa_read_write(port, READ, address, offset, len, data);
	return(retval);
}

/* Aardvark is an i2c device, doesn't do CFP */
int oom_set_memory_cfp(oom_port_t* port, int address, int len, uint16_t* data)
{
	return (-1);
}

int oom_get_memory_cfp(oom_port_t* port, int address, int len, uint16_t* data)
{
	return (-1);
}
