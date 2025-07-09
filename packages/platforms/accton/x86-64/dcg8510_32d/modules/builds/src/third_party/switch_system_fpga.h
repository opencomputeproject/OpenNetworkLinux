#ifndef FPGA_DRIVER_H
#define FPGA_DRIVER_H

enum fpga_type {
    fpga1 = 0,
    fpga2,
    max_type
};

int fpga_pcie_write(u8 device_id, u32 reg, u32 value);
int fpga_pcie_read(u8 device_id ,u32 reg);


#endif /* FPGA_DRIVER_H */


