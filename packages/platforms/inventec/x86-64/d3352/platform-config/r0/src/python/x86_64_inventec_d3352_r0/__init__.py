from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_d3352_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_48x1_4x10):
    PLATFORM='x86-64-inventec-d3352-r0'
    MODEL="X86-D3352"
    SYS_OBJECT_ID=".3352.1"

    def baseconfig(self):
        os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko")
        self.insmod('i2c-gpio')
        self.insmod('inv_platform')
        self.insmod('inv_psoc')
        self.insmod('inv_cpld')
	self.new_i2c_device('inv_eeprom', 0x53, 0)
        self.insmod('inv_eeprom')
	self.new_i2c_device('inv_psoc', 0x66, 1)
	self.new_i2c_device('inv_cpld', 0x55, 1)
	self.insmod('swps')
	self.insmod('inv_pthread')
        #self.insmod('vpd')
        return True
