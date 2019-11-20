from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_d7332_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_48x25_6x100):
    PLATFORM='x86-64-inventec-d7332-r0'
    MODEL="D7332"
    SYS_OBJECT_ID=".7332.1"

    def baseconfig(self):
        os.system("insmod /lib/modules/`uname -r`/onl/inventec/x86-64-inventec-d7332/gpio-ich.ko gpiobase=0")
        self.insmod('i2c-gpio')
        self.insmod('inv_ucd90160')
        self.insmod('inv-i2c-mux-pca9641')
        self.insmod('inv_psu')
        self.insmod('inv_cpld')
        self.insmod('inv_platform')
        self.insmod('inv_eeprom')
        self.new_i2c_device('inv_eeprom', 0x55, 2)
        self.insmod('inv_sff')
        self.insmod('vpd')
        
        self.insmod('optoe')
        for ch  in range(0,32):
            self.new_i2c_device('optoe1', 0x50, 14 + ch )
        return True
