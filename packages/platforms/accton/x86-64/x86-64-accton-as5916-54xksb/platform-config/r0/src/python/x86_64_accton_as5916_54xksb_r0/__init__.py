from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5916_54xksb_r0(OnlPlatformAccton,
                                               OnlPlatformPortConfig_48x10_6x100):
    PLATFORM='x86-64-accton-as5916-54xksb-r0'
    MODEL="AS5916-54XKSB"
    SYS_OBJECT_ID=".5916.54"

    def baseconfig(self):
        for m in [ 'fan', 'psu', 'led', 'sfp', 'sys', 'thermal' ]:
            self.insmod("x86-64-accton-as5916-54xksb-%s.ko" % m)

        return True

