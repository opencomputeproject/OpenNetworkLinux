from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_d7054q28b_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-inventec-d7054q28b-r0'
    MODEL="X86-D7054Q28B"
    SYS_OBJECT_ID="7054.28"

    def baseconfig(self):
        os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko gpiobase=0")
        self.insmod('inv_platform')
        self.insmod('inv_psoc')
        self.insmod('inv_cpld')
        return True
