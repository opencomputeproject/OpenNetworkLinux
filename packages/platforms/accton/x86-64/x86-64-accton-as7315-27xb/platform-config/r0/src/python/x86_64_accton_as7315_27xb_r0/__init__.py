from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7516_27xb_r0(OnlPlatformAccton,
                                               OnlPlatformPortConfig_20x10_4x25_3x100):
    PLATFORM='x86-64-accton-as7516-27xb-r0'
    MODEL="AS7516-27XB"
    SYS_OBJECT_ID=".7516.27"

    def baseconfig(self):
        self.insmod_platform()
        return True

