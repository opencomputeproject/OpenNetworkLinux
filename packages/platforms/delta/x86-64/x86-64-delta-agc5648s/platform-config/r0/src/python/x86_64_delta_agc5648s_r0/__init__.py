from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_agc5648s_r0(OnlPlatformDelta,
                                           OnlPlatformPortConfig_48x1_4x10):
    PLATFORM='x86-64-delta-agc5648s-r0'
    MODEL="agc5648s"
    SYS_OBJECT_ID=".5658.1"

    def baseconfig(self):
        
        # initiate eeprom
        self.new_i2c_device('24c02', 0x53, 0)
        
        return True

