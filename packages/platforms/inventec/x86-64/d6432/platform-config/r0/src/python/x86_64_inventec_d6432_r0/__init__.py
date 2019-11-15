from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_d6432_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_48x25_6x100):
    PLATFORM='x86-64-inventec-d6432-r0'
    MODEL="D6432"
    SYS_OBJECT_ID=".6432.1"

    def baseconfig(self):
        os.system("insmod /lib/modules/`uname -r`/onl/inventec/x86-64-inventec-d6432/gpio-ich.ko gpiobase=0")
        self.insmod('i2c-gpio')
        self.insmod('inv_ucd90160')
        self.insmod('inv-i2c-mux-pca9641')
        self.insmod('inv_psu')
        self.insmod('inv_cpld')
        self.insmod('inv_platform')
        self.insmod('inv_sff')
        self.insmod('vpd')
        
        return True
