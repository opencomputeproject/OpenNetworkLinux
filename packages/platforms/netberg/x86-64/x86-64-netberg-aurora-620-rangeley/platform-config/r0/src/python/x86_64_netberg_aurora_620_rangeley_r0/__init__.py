from onl.platform.base import *
from onl.platform.netberg import *

class OnlPlatform_x86_64_netberg_aurora_620_rangeley_r0(OnlPlatformNetberg,
                                                OnlPlatformPortConfig_48x25_6x100):
    PLATFORM='x86-64-netberg-aurora-620-rangeley-r0'
    MODEL="AURORA620"
    SYS_OBJECT_ID=".8.1"

    def baseconfig(self):
        self.insmod("hardware_monitor")

        # make ds1339 as default rtc
        os.system("ln -snf /dev/rtc1 /dev/rtc")
        os.system("hwclock --hctosys")

        return True
