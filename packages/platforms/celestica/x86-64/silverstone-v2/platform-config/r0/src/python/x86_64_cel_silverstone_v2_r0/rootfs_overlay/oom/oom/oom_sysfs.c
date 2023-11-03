/*
 * Southbound API shim for OOM, based on EEPROM files in /sys (sysfs)
 * Assumes that the files exist, available for read/write access
 * Tries to parameterize the location and format of those
 * files so it is easy to add support for other platforms with small
 * variations.
 *
 * As of September 2016, this shim has been verified on:
 *     Cumulus 3.0 (reads through page 3, writes)
 *     ONL (reads only page 0, does not write, both due to ONL limitations
 *
 * Copyright 2016  Finisar Inc.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include "oom_south.h"

#define MAXPORTS 512

/*
 * Based on the style of system, use the appropriate device directory
 * in sysfs, the right device naming convention, the right memory
 * mapping, etc.
 */

typedef enum sysfs_style_e {
	STYLE_CUMULUS = 0,
	STYLE_ONLP = 1,
	STYLE_UNKNOWN = -1
} sysfs_style_t;

typedef struct fsparms_s {
	sysfs_style_t style;
	char * eeprom_dir;
	char * devname;
} fsparms_t;

#define NUMSTYLES 2
fsparms_t fsp[NUMSTYLES] = {
	{STYLE_ONLP, "/sys/bus/i2c/devices", "sfp_eeprom"},
	{STYLE_CUMULUS, "/sys/class/eeprom_dev", "device/eeprom"},
};

fsparms_t parms = {STYLE_UNKNOWN, "", ""};


/* shimstate can be:
 *   0 - not initialized (port_array has not been filled)
 *   1 - initialized, port_array is filled, but not copied to user
 *          (oom_get_portlist(0,0) called, but not oom_get_portlist(list,len)
 *   2 - oom_get_portlist(list,len) has been returned
 */
int shimstate = 0;
oom_port_t port_array[MAXPORTS];
char port_filename[MAXPORTS][32];
int portcount = 0;

/*
 * setpackagpath is an undocumented southbound entry point
 * It doesn't have to do anything, or even exist, but the stub is here...
 */
void setpackagepath(char* packagedirparm) {
	return;
}


#define LABELEN 32
int fill_port_array()
{ 
	DIR *dir;
	struct dirent *dent;
	FILE *fd;
	oom_port_t *port;
	char labelpath[256];;
	char label[LABELEN];
	int i, len, nameend, portnum;

	if (parms.style == STYLE_UNKNOWN) {  /* figure out the style */
		for (i = 0; i < NUMSTYLES; i++) {
			parms = fsp[i];      /* note: parms is persistent */
			dir = opendir(parms.eeprom_dir);
			if (dir == NULL) continue;  /* not our style */
			if (parms.style == STYLE_ONLP) { /* 1 more test */
				bool foundit = false;
				while ((dent = readdir(dir)) != NULL) {
					/* ONLP style has sfp_eeprom files */
					sprintf(labelpath, 
						"%s/%s/sfp_eeprom", 
						parms.eeprom_dir, 
						dent->d_name);
					fd = fopen(labelpath, "r");
					if (fd != NULL) {  /* our style */
						rewinddir(dir);
						foundit = true;
						break;
					}
				}
				if (foundit == false) continue;  /* not it */
			}
			if (dir != NULL) {    /* success, parms is our style */
				break;
			}
		}
		if (i == NUMSTYLES) {  /* failed to open dev dir */
			printf("Unsupported platform for OOM sysfs shim\n");
			return (-1);
		}
	} else {
		dir = opendir(parms.eeprom_dir);
	}
	
	/* cycle through the directory, finding EEPROMs, making ports */
	/* note the directory was opened in the phrase above */
	portcount = 0;
	while ((dent = readdir(dir)) != NULL) {
		if ((strlen(dent->d_name) <= 0) || 
		    (dent->d_name[0] == '.')) continue;
		switch (parms.style) {

		    case STYLE_CUMULUS:
			/* 
			 * for eeproms, the label contains 'port<num>'
			 * where num is the port number
			 */
			sprintf(labelpath, "%s/%s/label", 
				parms.eeprom_dir, dent->d_name);
			fd = fopen(labelpath, "r");
			if (fd == NULL) continue;
			len = fread(label, 1, LABELEN, fd);
			if (len < 5) continue;  /* looking for 'port<num>' */
			if (strncmp(label, "port", 4) != 0) continue;

			/* this is the eeprom device we are looking for */
			port = &port_array[portcount];
			port->oom_class = OOM_PORT_CLASS_SFF;
			port->handle = (void *) (uintptr_t) portcount;
			for (i = 0; i < LABELEN; i++) {
				if (label[i] == 0xA) {
					label[i] = '\0';
				}
			}
			strcpy(port->name, label);
			break;

		    case STYLE_ONLP:
			/*
			 * device names look like '<num>-00<addr>', 
			 * eg 54-0050.  addr is the i2c address of 
			 * the EEPROM, eg 0xA0 or 0xA2, right shifted
			 * by one bit.  Hence 0xA0 = 50, 0xA2 = 51
			 * We just want one entry for each EEPROM
			 * They all use 0xA0, not all use 0xA2
			 * so we are looking for '<num>--50'
			 */
			nameend = strlen(dent->d_name);
			if ((dent->d_name[nameend - 2] != '5') ||
			    (dent->d_name[nameend - 1] != '0')) continue;
			sprintf(labelpath, "%s/%s/sfp_port_number", 
				           parms.eeprom_dir, 
					   dent->d_name);
			fd = fopen(labelpath, "r");
			if (fd == NULL) continue;
			len = fread(label, 1, LABELEN, fd);
			/* port number is terminated by newline (0xA) */
			for (i = 0; i < LABELEN; i++) {
				if (label[i] == 0xA) {
					label[i] = '\0';
				}
			}
			portnum = atoi(label); /* returns 0 if not number */
			if (portnum == 0) continue;

			/* this is the eeprom device we are looking for */
			port = &port_array[portcount];
			port->oom_class = OOM_PORT_CLASS_SFF;
			port->handle = (void *) (uintptr_t) portcount;
			for (i = 0; i < LABELEN; i++) {
				if (label[i] == 0xA) {
					label[i] = '\0';
				}
			}
			sprintf(port->name, "port%s", label);
			break;

	    	    default:
			printf("Unsupported platform for OOM sysfs shim\n");
			return (-1);
		}
		/* save the name of this port for later use */
		strcpy(port_filename[portcount], dent->d_name);
		portcount++;
	}
	return 0;
}


/* oom_get_portlist - figure out how many ports, what they are */

int oom_get_portlist(oom_port_t portlist[], int listsize)
{
	int err;
	int retval;

	if ((portlist == NULL) && (listsize == 0)) {  /* asking # of ports */
		err = fill_port_array();
		if (err < 0) return err;
		shimstate = 1;
		return(portcount);
	}
	if (shimstate != 1) {   /* if uninitialized or already returned */
		err = fill_port_array();
		if (err < 0) return err;
	}
	if (listsize < portcount) {   /* not enough room */
		return(-ENOMEM);
	}

	/* if portlist has changed, need to return count, else 0 */
	retval = 0;
	if ((listsize != portcount) ||
	    (memcmp(port_array, portlist, 
		    portcount * sizeof(oom_port_t)) != 0)) {
		retval = portcount;
		memcpy(portlist, port_array, portcount * sizeof(oom_port_t));
		shimstate = 2;
	}
	return(retval);   
}


int oom_get_memory_sff(oom_port_t* port, int address, int page, 
		       int offset, int len, uint8_t* data)
{
	int fd;
	char fpath[256];
	char devname[LABELEN];
	int offs, loc, retval;

	offs = offset;
	switch (parms.style) {
	    case STYLE_CUMULUS :
		/* generate the name of the actual eeprom file */
		sprintf(fpath, "%s/%s/device/eeprom", parms.eeprom_dir, 
			port_filename[(intptr_t)port->handle]);

		/*
		 * Cumulus EEPROM mem map puts 256 bytes of A0, 
		 * followed by all of A2 pages 
		 * it also puts all of the A0 pages, 128 bytes each, 
		 * consecutively.  Thus, byte 256 is EITHER, 
		 * A0 page 1 (eg QSFP+), or A2 base (eg SFP+)
		 */
		if (address == 0xA2) offs += 256;
		break;

	    case STYLE_ONLP:
		/* generate the name of the actuall eeprom file */
		strcpy(devname, port_filename[(intptr_t)port->handle]);
		/* devname is for 0xA0, if necessary tweak it for 0xA2 */
		/* recall that <num>--0050 is 0xA0, <num--0051> is 0xA2 */
		if (address == 0xA2) devname[strlen(devname)-1] = '1';
		sprintf(fpath, "%s/%s/sfp_eeprom", parms.eeprom_dir, devname);
		break;

	    default:
		printf("Unsupported platform for OOM sysfs shim\n");
		return (-1);
	}

	if ((fd = open(fpath, O_RDONLY)) == -1 ) return -errno;
	offs += (page * 128);
	loc = lseek(fd, offs, SEEK_SET);
	if (loc != offs) {
		close(fd);
		return -errno;
	}
	retval = read(fd, data, len);
	if (retval == -1) retval = -errno;
	close(fd);
	return retval;
}

int oom_set_memory_sff(oom_port_t* port, int address, int page, 
		       int offset, int len, uint8_t* data)
{
	int fd;
	char fpath[256];
	char devname[LABELEN];
	int offs, loc, retval;

	offs = offset;
	switch (parms.style) {
	    case STYLE_CUMULUS :
		/* generate the name of the actuall eeprom file */
		sprintf(fpath, "%s/%s/device/eeprom", parms.eeprom_dir, 
			port_filename[(intptr_t)port->handle]);

		/*
		 * Cumulus EEPROM mem map puts 256 bytes of A0, 
		 * followed by all of A2 pages 
		 * it also puts all of the A0 pages, 128 bytes each, 
		 * consecutively.  Thus, byte 256 is EITHER, 
		 * A0 page 1 (eg QSFP+), or A2 base (eg SFP+)
		 */
		if (address == 0xA2) offs += 256;
		break;

	    case STYLE_ONLP:
		/* generate the name of the actuall eeprom file */
		strcpy(devname, port_filename[(intptr_t)port->handle]);
		/* devname is for 0xA0, if necessary tweak it for 0xA2 */
		/* recall that <num>--0050 is 0xA0, <num--0051> is 0xA2 */
		if (address == 0xA2) devname[strlen(devname)-1] = '1';
		sprintf(fpath, "%s/%s/sfp_eeprom", parms.eeprom_dir, devname);
		break;

	    default:
		printf("Unsupported platform for OOM sysfs shim\n");
		return (-1);
	}

	if ((fd = open(fpath, O_RDWR)) == -1 ) return -errno;
	offs += page * 128;
	loc = lseek(fd, offs, SEEK_SET);
	if (loc != offs) {
		close(fd);
		return -errno;
	}
	retval = write(fd, data, len);
	if (retval == -1) retval = -errno;
	close(fd);
	return retval;
}


int oom_get_memory_cfp(oom_port_t* port, int address, int len, uint16_t* data)
{
	return -1;
}

int oom_set_memory_cfp(oom_port_t* port, int address, int len, uint16_t* data)
{
	return -1;
}
