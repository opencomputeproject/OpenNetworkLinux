from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ly9_rangeley_r0(OnlPlatformQuanta,
                                                OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-quanta-ly9-rangeley-r0'
    MODEL="LY9"
    SYS_OBJECT_ID=".9.1"

    def baseconfig(self):
        self.insmod("emerson700")
        self.insmod("quanta_hwmon_ly_series")
        self.insmod("qci_cpld")
        self.insmod("quanta_platform_ly9")

        # make ds1339 as default rtc
        os.system("ln -snf /dev/rtc1 /dev/rtc")
        os.system("hwclock --hctosys")

        return True
