from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_ag9032v2_r0(OnlPlatformDelta,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-delta-ag9032v2-r0'
    MODEL="AG9032V2"
    SYS_OBJECT_ID=".9032.2"


    def baseconfig(self):

        #IDEEPROM modulize
        self.new_i2c_device('24c02', 0x53, 1)

        return True


