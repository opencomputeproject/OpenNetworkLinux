from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7316_26xb_r0(OnlPlatformAccton,
                                               OnlPlatformPortConfig_16x10_8x25_2x100):
    PLATFORM='x86-64-accton-as7316-26xb-r0'
    MODEL="AS7316-26XB"
    SYS_OBJECT_ID=".7316.26"

    def baseconfig(self):
        self.insmod_platform()
        return True

