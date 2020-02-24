from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_agc7008s_r0(OnlPlatformDelta,
                                         OnlPlatformPortConfig_4x1_8x10):
    PLATFORM='x86-64-delta-agc7008s-r0'
    MODEL="agc7008s"
    SYS_OBJECT_ID=".7008.1"
    
    def baseconfig(self):

        #Insert i2c mosule
        self.insmod('delta_i2c_ismt')

        #Insert qsfp mosule
        self.insmod('optoe')

        #Insert platform module
        self.insmod('delta_agc7008s_platform')

        #Insert cpld module
        self.insmod('delta_agc7008s_cpld')

        #Insert swpld module
        self.insmod('delta_agc7008s_swpld')

        return True

