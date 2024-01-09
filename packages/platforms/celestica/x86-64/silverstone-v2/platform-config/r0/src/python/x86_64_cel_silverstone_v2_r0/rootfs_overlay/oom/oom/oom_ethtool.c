/*
 * OOM Ethtool
 *
 * Copyright (c) 2016 Cumulus Networks, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <arpa/inet.h>

#include "oom_south.h"

#define MAX_PORTS 512

#define SFF_A0_BASE 0x0
#define SFF_A2_BASE 0x100
#define SFF_ID_ADDR (SFF_A0_BASE + 0x0)
#define SFF_ID_LEN  1

#define ERROR(...) \
        do { fprintf(oe->logfile ? oe->logfile : stderr, __VA_ARGS__); \
             fflush(oe->logfile); \
        } while(0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

typedef struct {
        oom_port_t portlist[MAX_PORTS];
        int portcount;
        FILE *logfile;
} oom_ethtool_t;

oom_ethtool_t oom_ethtool;
oom_ethtool_t *oe = &oom_ethtool;

int ethtool_get_socket(void)
{
        int fd;

        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) {
                ERROR("oom_ethtool: failed to open socket: %s", strerror(errno));
                return -1;
        }

        return fd;
}

int ethtool_get_port(int fd, const char *ifname, oom_port_t *port)
{
        struct ethtool_modinfo modinfo = { .cmd = ETHTOOL_GMODULEINFO };
        struct ifreq ifreq = { .ifr_data = (char *)&modinfo };
        unsigned int ifindex;
        int err;

        ifindex = if_nametoindex(ifname);
        if (ifindex == 0) {
                return -ENODEV;
        }

        strncpy(ifreq.ifr_name, ifname, sizeof(ifreq.ifr_name));

        err = ioctl(fd, SIOCETHTOOL, &ifreq);
        if (err) {
                return -EINVAL;
        }


        if (modinfo.type == 0) {
                // No module
                return -ENODEV;
        }

        if (port) {
                port->oom_class = OOM_PORT_CLASS_SFF;
                port->handle = (void *)(uintptr_t)ifindex;
                strncpy(port->name, ifname, sizeof port->name);
        }

        return 0;
}

int ethtool_update_portlist(int fd)
{
        int count = 0;
        int rv = 0;
        DIR *dir;
        struct dirent *dent;

        dir = opendir("/sys/class/net");
        if (dir < 0) {
                ERROR("oom_ethtool: failed to open /sys/class/net\n");
                return -EINVAL;
        }

        while ((dent = readdir(dir)) != NULL) {
                oom_port_t port = { 0 };

                if (strlen(dent->d_name) <= 0 || dent->d_name[0] == '.') {
                        continue;
                }

                // skip ports that fail an ethtool ifreq for module information
                if (ethtool_get_port(fd, dent->d_name, &port) != 0) {
                        continue;
                }

                if (count < ARRAY_SIZE(oe->portlist)) {
                        memcpy(&oe->portlist[count], &port, sizeof port);
                } else {
                        ERROR("oom_ethtool: internal portlist too small\n");
                        rv = -ENOMEM;
                        goto done;
                }

                count++;
        }

        oe->portcount = count;

done:
        closedir(dir);

        if (rv < 0) {
                memset(oe->portlist, 0, sizeof oe->portlist);
                oe->portcount = 0;
        }
        return rv;
}

/*
 * oom_get_portlist - get a list of ports
 *
 * Assumes portlist is oom_maxports() long.
 *
 * Returns the number of ports in the list or -1 on failure.
 */
int oom_get_portlist(oom_port_t portlist[], int listsize)
{
        bool changed = false;
        int err;
        int fd;

#ifdef DEBUG_LOGFILE
        oe->logfile = fopen("/tmp/oom_ethtool.log", "w");
#endif

        fd = ethtool_get_socket();
        if (fd < 0) {
                return -EINVAL;
        }

        err = ethtool_update_portlist(fd);
        close(fd);
        if (err < 0) {
                ERROR("oom_ethtool: failed to update portlist\n");
                return -EINVAL;
        }

        if (portlist == NULL) {
                return oe->portcount;
        }

        // return -ENOMEM when the portlist provided is too small
        if (oe->portcount > listsize) {
                return -ENOMEM;
        }

        // detect changes to the provided portlist
        if (memcmp(portlist, oe->portlist,
                   oe->portcount * sizeof portlist[0]) != 0) {
                changed = true;
        }
        memcpy(portlist, oe->portlist, oe->portcount * sizeof portlist[0]);

        // if the portlist changed in size or content, return the count
        if (changed || oe->portcount != listsize) {
                return oe->portcount;
        }

        // return 0 if the input portlist matched the updated portlist
        return 0;
}

int ethtool_get_memory(int fd, const char *ifname, int offset, int len, uint8_t *data)
{
        struct {
                struct ethtool_eeprom eepromreq;
                union {
                        uint8_t buf[1024];
                };
        } eeprom = { .eepromreq.cmd = ETHTOOL_GMODULEEEPROM };
        struct ifreq ifreq = { .ifr_data = (char *)&eeprom };
        int err;

        if (len > sizeof eeprom.buf) {
                ERROR("oom_ethtool: request too large: %u\n", len);
                return -1;
        }

        eeprom.eepromreq.offset = offset;
        eeprom.eepromreq.len = len;

        strncpy(ifreq.ifr_name, ifname, sizeof(ifreq.ifr_name));
        err = ioctl(fd, SIOCETHTOOL, &ifreq);
        if (err) {
                ERROR("oom_ethtool: %u ifreq failed: 0x%x\n", ifreq.ifr_ifindex, err);
                return err;
        }

        memcpy(data, eeprom.buf, len);

        return 0;
}

int oom_get_memory_sff(oom_port_t* port, int address, int page, int offset,
                       int len, uint8_t* data)

{
        char ifname[IF_NAMESIZE];
        unsigned int ifindex;
        int ethtool_offset;
        int ethtool_len;
        int fd;
        int err;

        ifindex = (unsigned int)(uintptr_t)port->handle;

        if (!if_indextoname(ifindex, ifname)) {
                ERROR("oom_ethtool: failed to translate index to name: %d\n", ifindex);
                return -ENODEV;
        }

        if (strcmp(ifname, port->name) != 0) {
                ERROR("oom_ethtool: port name for ifindex %d changed from %s to %s\n",
                      ifindex, port->name, ifname);
                return -ESTALE;
        }

        if (address == 0xa0) {
                ethtool_offset = SFF_A0_BASE;
        } else if (address == 0xa2) {
                ethtool_offset = SFF_A2_BASE;
        } else {
                ERROR("oom_ethtool: invalid address: 0x%02x\n", address);
                return -1;
        }
        ethtool_offset += page * 128;
        ethtool_offset += offset;
        ethtool_len = len;

        fd = ethtool_get_socket();
        if (fd < 0) {
                return -EINVAL;
        }

        err = ethtool_get_memory(fd, ifname, ethtool_offset, ethtool_len, data);
        close(fd);
        return err;
}

int oom_set_memory_sff(oom_port_t* port, int address, int page, int offset,
                       int len, uint8_t* data)

{
        // XXX - not implemented
        return -1;
}

int oom_set_function(oom_port_t* port, oom_functions_t function, int value)
{
        // XXX - not implemented
        return -1;
}

int oom_get_function(oom_port_t* port, oom_functions_t function, int* rv)
{
        // XXX - not implemented
        return -1;
}

int oom_set_memory_cfp(oom_port_t* port, int address, int len, uint16_t* data)
{
        // XXX - not implemented
        return -1;
}

int oom_get_memory_cfp(oom_port_t* port, int address, int len, uint16_t* data)
{
        // XXX - not implemented
        return -1;
}
