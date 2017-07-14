from onl.platform.base import *
from onl.platform.netberg import *

class OnlPlatform_x86_64_netberg_aurora_720_rangeley_r0(OnlPlatformNetberg,
                                                OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-netberg-aurora-720-rangeley-r0'
    MODEL="AURORA720"
    SYS_OBJECT_ID=".720.1"

    def baseconfig(self):
        self.insmod("hardware_monitor")
        return True
