from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5916_26xb_r0(OnlPlatformAccton,
                                               OnlPlatformPortConfig_24x10_2x100):
    PLATFORM='x86-64-accton-as5916-26xb-r0'
    MODEL="AS5916-26XB"
    SYS_OBJECT_ID=".5916.26"

    def baseconfig(self):
        self.insmod_platform()
        return True

