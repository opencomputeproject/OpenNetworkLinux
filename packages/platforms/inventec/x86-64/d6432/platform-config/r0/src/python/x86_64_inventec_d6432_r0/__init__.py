from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_d6432_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_32x400):
    PLATFORM='x86-64-inventec-d6432-r0'
    MODEL="D6432"
    SYS_OBJECT_ID=".6432.1"
    CHASSIS_FAN_NUM=6
    FAN_VPD_CHANNEL=3
    FAN_VPD_ADDR_BASE=0x52

    def baseconfig(self):
        os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko gpiobase=0")
        self.insmod('i2c-gpio')
        self.insmod('inv_ucd90160')
        self.insmod('inv-i2c-mux-pca9641')
        self.insmod('inv_psu')
        self.insmod('inv_cpld')
        self.insmod('inv_platform')
        self.insmod('inv_eeprom')
        self.new_i2c_device('inv_eeprom', 0x55, 2)

        for addr_offset in range(0,self.CHASSIS_FAN_NUM):
            self.new_i2c_device('inv_eeprom',self.FAN_VPD_ADDR_BASE+addr_offset,self.FAN_VPD_CHANNEL)

        self.insmod('inv_sff')
        self.insmod('vpd')
        self.insmod('optoe')
        for ch  in range(0,32):
           self.new_i2c_device('optoe1', 0x50, 14 + ch )        

        return True
