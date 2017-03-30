from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ly6_rangeley_r0(OnlPlatformQuanta,
                                                OnlPlatformPortConfig_32x40):
    PLATFORM='x86-64-quanta-ly6-rangeley-r0'
    MODEL='LY6'
    SYS_OBJECT_ID='.6.1'

    def baseconfig(self):
        self.insmod("quanta_hwmon_ly_series")
        self.insmod("quanta-ly6-i2c-mux")
        self.insmod("quanta_platform_ly6")

        # make ds1339 as default rtc
        os.system("ln -snf /dev/rtc1 /dev/rtc")
        os.system("hwclock --hctosys")

        return True
