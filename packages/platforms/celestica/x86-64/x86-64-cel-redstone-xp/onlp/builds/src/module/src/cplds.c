#include <errno.h>
#include <unistd.h>
#include <sys/io.h>

#include <x86_64_cel_redstone_xp/x86_64_cel_redstone_xp_config.h>
#include "x86_64_cel_redstone_xp_log.h"
#include "x86_64_cel_redstone_xp_int.h"
#include "cplds.h"

int
cpld_io_init(void)
{
    /* Initialize LPC access ports */
    if(ioperm(0x100, 0x2FF, 1) == -1) {
        AIM_LOG_ERROR("ioperm() failed: %{errno}", errno);
        return -1;
    }
    return 0;
}

int
cpld_read(int addr)
{
    return inb(addr);
}

void
cpld_write(int addr, uint8_t value)
{
    outb(value, addr);
}

int read_cpld(int reg, unsigned char *value)
{
    *value = inb(reg);
    return 0;
}
int write_cpld(int reg, unsigned char value)
{
    outb(value, reg);
    return 0;
}

void
cpld_modify(int addr, uint8_t andmask, uint8_t ormask)
{
    int v;
    read_cpld(addr, (unsigned char *)&v);
    v &= andmask;
    v |= ormask;
    cpld_write(addr, v);
}

int
cpld_dump(aim_pvs_t* pvs, int cpldid)
{
    int data;

    aim_map_si_t* si;
    aim_map_si_t* maps[] = {
        cpld1_reg_map,
        cpld2_reg_map,
        cpld3_reg_map,
        cpld4_reg_map,
        cpld5_reg_map,
    };
    if(cpldid < 1 || cpldid > 5) {
        aim_printf(pvs, "Invalid CPLDID %d\n", cpldid);
        return -1;
    }
    else {
        for(si = maps[cpldid-1]; si->s; si++) {
            read_cpld(si->i, (unsigned char *)&data);
            aim_printf(pvs, "  %32.32s [0x%.2x] = 0x%.2x %{8bits}\n", si->s, si->i, data, data);
        }
    }
    return 0;
}


// EXPERIMENT
#define PORT_BANK1_START 1
#define PORT_BANK1_END 18
#define PORT_BANK2_START 19
#define PORT_BANK2_END 36
#define PORT_BANK3_START 37
#define PORT_BANK3_END 48
#define PORT_BANK4_START 49
#define PORT_BANK4_END 54
int
read_sfp(int portID, char devAddr, char reg, char *data, int len)
{
    int count;
    char byte;
    short temp;
    short portid, opcode, devaddr, cmdbyte0, ssrr, writedata = 0, readdata;
    int ioBase = 0;

    cpld_io_init();

    if ((reg + len) > 256)
        return -1;

    if ((portID >= PORT_BANK1_START) && (portID <= PORT_BANK1_END)) {
        portid = 0x210;
        opcode = 0x211;
        devaddr = 0x212;
        cmdbyte0 = 0x213;
        ssrr = 0x216;
        writedata = 0x220;
        readdata = 0x230;

        while ((inb(ioBase + ssrr) & 0x40));

        if ((inb(ioBase + ssrr) & 0x80) == 0x80) {
            outb(0x00, ioBase + ssrr);
            usleep(3000);
            outb(0x01, ioBase + ssrr);
            return -1;
        }
    } else if ((portID >= PORT_BANK2_START) && (portID <= PORT_BANK2_END)) {
        portid = 0x290;
        opcode = 0x291;
        devaddr = 0x292;
        cmdbyte0 = 0x293;
        ssrr = 0x296;
        writedata = 0x2A0;
        readdata = 0x2B0;

        while ((inb(ioBase + ssrr) & 0x40));

        if ((inb(ioBase + ssrr) & 0x80) == 0x80) {
            outb(0x00, ioBase + ssrr);
            usleep(3000);
            outb(0x01, ioBase + ssrr);
            return -1;
        }
    } else if ((portID >= PORT_BANK3_START) && (portID <= PORT_BANK3_END)) {
        portid = 0x390;
        opcode = 0x391;
        devaddr = 0x392;
        cmdbyte0 = 0x393;
        ssrr = 0x396;
        writedata = 0x3A0;
        readdata = 0x3B0;

        while ((inb(ioBase + ssrr) & 0x40));

        if ((inb(ioBase + ssrr) & 0x80) == 0x80) {
            outb(0x00, ioBase + ssrr);
            usleep(3000);
            outb(0x01, ioBase + ssrr);
            return -1;
        }
    } else if ((portID >= PORT_BANK4_START) && (portID <= PORT_BANK4_END)) {
        portid = 0x310;
        opcode = 0x311;
        devaddr = 0x312;
        cmdbyte0 = 0x313;
        ssrr = 0x316;
        writedata = 0x320;
        readdata = 0x330;

        while ((inb(ioBase + ssrr) & 0x40));

        if ((inb(ioBase + ssrr) & 0x80) == 0x80) {
            outb(0x00, ioBase + ssrr);
            usleep(3000);
            outb(0x01, ioBase + ssrr);
            return -1;
        }
    } else {
        return -1;
    }

    byte = 0x40 + portID;
    outb(byte, ioBase + portid);
    outb(reg,ioBase + cmdbyte0);

    while (len > 0) {
        count = (len >= 8) ? 8 : len;
        len -= count;
        byte = count * 16 + 1;
        outb(byte, ioBase + opcode);
        devAddr |= 0x01;
        outb(devAddr, ioBase + devaddr);

        while ((inb(ioBase + ssrr) & 0x40))
            usleep(100);

        if ((inb(ioBase + ssrr) & 0x80) == 0x80) {
            outb(0x00, ioBase + ssrr);
            usleep(3000);
            outb(0x01, ioBase + ssrr);
            return -1;
        }

        temp = ioBase + readdata;

        while (count-- > 0) {
            char read_byte;
            read_byte = inb(temp);
            *(data++) = read_byte;
            temp++;
        }

        if (len > 0) {
            reg += 0x08;
            outb(reg, ioBase + cmdbyte0);
        }
    }
// FIXME
    return writedata * 0;
//    return 0;
}

int
write_sfp(int portID, char devAddr, char reg, char *data, int len)
{
	int count;
	char byte;
	short temp;
	short portid, opcode, devaddr, cmdbyte0, ssrr, writedata, readdata;
    int ioBase = 0;

	if ((reg + len) > 256)
        return -1;

    if ((portID >= PORT_BANK1_START) && (portID <= PORT_BANK1_END)) {
		portid = 0x210;
		opcode = 0x211;
		devaddr = 0x212;
		cmdbyte0 = 0x213;
		ssrr = 0x216;
		writedata = 0x220;
		readdata = 0x230;
		while ((inb(ioBase + ssrr) & 0x40));
		if ((inb(ioBase + ssrr) & 0x80) == 0x80) {
			outb(0x00, ioBase + ssrr);
			usleep(3000);
			outb(0x01, ioBase + ssrr);
			return -1;
		}
	} else if ((portID >= PORT_BANK2_START) && (portID <= PORT_BANK2_END)) {
		portid = 0x290;
		opcode = 0x291;
		devaddr = 0x292;
		cmdbyte0 = 0x293;
		ssrr = 0x296;
		writedata = 0x2A0;
		readdata = 0x2B0;
		while ((inb(ioBase + ssrr) & 0x40));
		if ((inb(ioBase + ssrr) & 0x80) == 0x80) {
			outb(0x00, ioBase + ssrr);
			usleep(3000);
			outb(0x01, ioBase + ssrr);
			return -1;
		}
	} else if ((portID >= PORT_BANK3_START) && (portID <= PORT_BANK3_END)) {
		portid = 0x390;
		opcode = 0x391;
		devaddr = 0x392;
		cmdbyte0 = 0x393;
		ssrr = 0x396;
		writedata = 0x3A0;
		readdata = 0x3B0;
		while ((inb(ioBase + ssrr) & 0x40));
		if ((inb(ioBase + ssrr) & 0x80) == 0x80) {
			outb(0x00, ioBase + ssrr);
			usleep(3000);
			outb(0x01, ioBase + ssrr);
			return -1;
		}
	} else if ((portID >= PORT_BANK4_START) && (portID <= PORT_BANK4_END)) {
		portid = 0x310;
		opcode = 0x311;
		devaddr = 0x312;
		cmdbyte0 = 0x313;
		ssrr = 0x316;
		writedata = 0x320;
		readdata = 0x330;
		while ((inb(ioBase + ssrr) & 0x40));
		if ((inb(ioBase + ssrr) & 0x80) == 0x80) {
			outb(0x00, ioBase + ssrr);
			usleep(3000);
			outb(0x01, ioBase + ssrr);
			return -1;
		}
	} else {
		return -1;
	}

	byte = 0x40 + portID;
	outb(byte, ioBase + portid);
	outb(reg, ioBase + cmdbyte0);
	while (len > 0) {
		count = (len >= 8) ? 8 : len;
		len -= count;
		byte = (count << 4) + 1;
		outb(byte, ioBase + opcode);
		temp = ioBase + writedata;
		while (count-- > 0) {
			outb(*(data++), temp);
			temp += 0x01;
		}
		devAddr &= 0xfe;
		outb(devAddr, ioBase + devaddr);
		while ((inb(ioBase + ssrr) & 0x40))
		{
			usleep(100);
		}
		if ((inb(ioBase + ssrr) & 0x80) == 0x80) {
			outb(0x00, ioBase + ssrr);
			usleep(3000);
			outb(0x01, ioBase + ssrr);
			return -1;
		}
		if (len > 0) {
			reg += 0x08;
			outb(ioBase + cmdbyte0, reg);
		}
	}
// FIXME
    return writedata * readdata * 0;
//    return 0;
}

