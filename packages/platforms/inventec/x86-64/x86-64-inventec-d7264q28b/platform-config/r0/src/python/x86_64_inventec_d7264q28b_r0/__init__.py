from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_d7264q28b_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_64x100):
    PLATFORM='x86-64-inventec-d7264q28b-r0'
    MODEL="X86-D7264Q28B"
    SYS_OBJECT_ID=".7264.1"

    def baseconfig(self):
	os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko")
        self.insmod('inv_platform')
        self.insmod('inv_psoc')
        self.insmod('inv_cpld')
	self.new_i2c_device('inv_eeprom', 0x53, 0)
        self.insmod('inv_eeprom')
        self.insmod('swps')
        #self.insmod('vpd')
        self.insmod('inv_pthread')
        return True
