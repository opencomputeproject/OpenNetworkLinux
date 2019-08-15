from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ly6_rangeley_r0(OnlPlatformQuanta,
                                                OnlPlatformPortConfig_32x40):
    PLATFORM='x86-64-quanta-ly6-rangeley-r0'
    MODEL='LY6'
    """ Define Quanta SYS_OBJECT_ID rule.

    SYS_OBJECT_ID = .xxxx.ABCC
    "xxxx" define QCT device mark. For example, LB9->1048, LY2->3048
    "A" define QCT switch series name: LB define 1, LY define 2, IX define 3
    "B" define QCT switch series number 1: For example, LB9->9, LY2->2
    "CC" define QCT switch series number 2: For example, LY2->00, LY4R->18(R is 18th english letter)
    """
    SYS_OBJECT_ID='.5032.2600'

    def baseconfig(self):
        self.insmod("quanta_hwmon_ly_series")
        self.insmod("quanta-ly6-i2c-mux")
        self.insmod("quanta_platform_ly6")

        # make ds1339 as default rtc
        os.system("ln -snf /dev/rtc1 /dev/rtc")
        os.system("hwclock --hctosys")

        return True
