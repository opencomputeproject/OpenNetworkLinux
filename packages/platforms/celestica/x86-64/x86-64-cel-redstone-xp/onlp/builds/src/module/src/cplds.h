#ifndef __CPLDS_H__
#define __CPLDS_H__

int cpld_io_init(void);
int cpld_read(int addr);
void cpld_write(int addr, uint8_t value);
void cpld_modify(int addr, uint8_t andmask, uint8_t ormask);
int cpld_dump(aim_pvs_t* pvs, int cpldid);

#endif /* __CPLDS_H__ */


