from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_wb2448_r0(OnlPlatformDelta,
							OnlPlatformPortConfig_48x1_4x10):
    PLATFORM='x86-64-delta-wb2448-r0'
    MODEL="wb2448"
    SYS_OBJECT_ID=".2448.1"

    def baseconfig(self):
        
        # initiate eeprom
        self.new_i2c_device('24c02', 0x53, 0)
        
        return True

