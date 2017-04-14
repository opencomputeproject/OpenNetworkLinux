from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ix1_rangeley_r0(OnlPlatformQuanta,
                                                OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-quanta-ix1-rangeley-r0'
    MODEL="IX1"
    SYS_OBJECT_ID=".8.1"

    def baseconfig(self):
        self.insmod("qci_pmbus")
        self.insmod("qci_cpld")
        self.insmod("quanta_hwmon_ix_series")
        self.insmod("quanta_platform_ix1")

        return True
