from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ix2_rangeley_r0(OnlPlatformQuanta,
                                                OnlPlatformPortConfig_48x25_8x100):
    PLATFORM='x86-64-quanta-ix2-rangeley-r0'
    MODEL="IX2"
    SYS_OBJECT_ID=".8.1"

    def baseconfig(self):
        # Expose PSOC that behind PCA9641
        os.system("i2cset -y 0 0x8 0x5 0xfb")
        os.system("i2cset -y 0 0x8 0x1 0x5")

        self.insmod("qci_pmbus")
        self.insmod("qci_cpld_sfp28")
        self.insmod("quanta_hwmon_ix_series")
        self.insmod("quanta_platform_ix2")

        return True
