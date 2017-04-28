from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ly4r_r0(OnlPlatformQuanta,
                                                OnlPlatformPortConfig_48x1_4x10):
    PLATFORM='x86-64-quanta-ly4r-r0'
    MODEL="LY4R"
    SYS_OBJECT_ID=".8.1"

    def baseconfig(self):
        self.insmod("quanta_platform_ly4r")

        return True
