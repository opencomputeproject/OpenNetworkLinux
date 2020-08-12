from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_d6356_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_48x25_8x100):
    PLATFORM='x86-64-inventec-d6356-r0'
    MODEL="D6356"
    SYS_OBJECT_ID=".6356.1"

    def baseconfig(self):
	os.system("rmmod gpio_ich")
	self.insmod('i2c-gpio')
	os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko gpiobase=0")
	self.insmod('ucd9000')
	self.insmod('inv-i2c-mux-pca9641')
	self.insmod('inv_platform')
	self.insmod('inv_cpld')
	self.insmod('swps')
	self.new_i2c_device('inv_eeprom', 0x55, 2)
	os.system("insmod /lib/modules/`uname -r`/onl/inventec/common/inv_eeprom.ko")

	return True
