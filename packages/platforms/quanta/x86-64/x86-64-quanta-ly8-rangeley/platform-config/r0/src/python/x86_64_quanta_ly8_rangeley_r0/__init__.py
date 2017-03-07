from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ly8_rangeley_r0(OnlPlatformQuanta,
                                                OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-quanta-ly8-rangeley-r0'
    MODEL="LY8"
    SYS_OBJECT_ID=".8.1"

    def baseconfig(self):
        self.insmod("emerson700")
        self.insmod("quanta_hwmon_ly_series")
        self.insmod("quanta_platform_ly8")

        # make ds1339 as default rtc
        os.system("ln -snf /dev/rtc1 /dev/rtc")
        os.system("hwclock --hctosys")

        return True
