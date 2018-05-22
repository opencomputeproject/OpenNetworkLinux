from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ix8_rglbmc_r0(OnlPlatformQuanta,
                                                OnlPlatformPortConfig_48x25_8x100):
    PLATFORM='x86-64-quanta-ix8-rglbmc-r0'
    MODEL="IX8"
    """ Define Quanta SYS_OBJECT_ID rule.

    SYS_OBJECT_ID = .xxxx.ABCC
    "xxxx" define QCT device mark. For example, LB9->1048, LY2->3048
    "A" define QCT switch series name: LB define 1, LY define 2, IX define 3
    "B" define QCT switch series number 1: For example, LB9->9, LY2->2
    "CC" define QCT switch series number 2: For example, LY2->00, LY4R->18(R is 18th english letter)
    """
    SYS_OBJECT_ID=".4048.3800"

    def baseconfig(self):
        self.insmod("qci_cpld_sfp28")
        self.insmod("qci_cpld_led")
        self.insmod("qci_platform_ix8")

        return True
