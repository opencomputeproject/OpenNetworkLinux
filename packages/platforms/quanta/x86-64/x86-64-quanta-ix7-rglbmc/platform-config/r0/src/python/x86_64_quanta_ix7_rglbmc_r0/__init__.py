from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ix7_rglbmc_r0(OnlPlatformQuanta,
                                                OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-quanta-ix7-rglbmc-r0'
    MODEL="IX7"
    """ Define Quanta SYS_OBJECT_ID rule.

    SYS_OBJECT_ID = .xxxx.ABCC
    "xxxx" define QCT device mark. For example, LB9->1048, LY2->3048
    "A" define QCT switch series name: LB define 1, LY define 2, IX define 3
    "B" define QCT switch series number 1: For example, LB9->9, LY2->2
    "CC" define QCT switch series number 2: For example, LY2->00, LY4R->18(R is 18th english letter)
    """
    SYS_OBJECT_ID=".7032.3700"

    def baseconfig(self):
        self.insmod("qci_cpld")
        self.insmod("qci_cpld_led")
        self.insmod("quanta_platform_ix7")

        return True
