from onl.platform.base import *
from onl.platform.netberg import *

class OnlPlatform_x86_64_netberg_aurora_420_rangeley_r0(OnlPlatformNetberg,
                                                        OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-netberg-aurora-420-rangeley-r0'
    MODEL="AURORA420"
    SYS_OBJECT_ID=".420.1"

    def baseconfig(self):
        self.insmod("hardware_monitor")
        return True
