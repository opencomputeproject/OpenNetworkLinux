from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_ag9064_r0(OnlPlatformDelta,
                                         OnlPlatformPortConfig_64x100):
    PLATFORM='x86-64-delta-ag9064-r0'
    MODEL="ag9064"
    SYS_OBJECT_ID=".9064.1"
    
    def baseconfig(self):
        
        # initiate eeprom
        self.new_i2c_device('24c02', 0x56, 0)
        
        return True

