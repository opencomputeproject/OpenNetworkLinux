from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_ag9064_r0(OnlPlatformDelta,
                                         OnlPlatformPortConfig_64x100):
    PLATFORM='x86-64-delta-ag9064-r0'
    MODEL="ag9064"
    SYS_OBJECT_ID=".9064.1"
    
    def baseconfig(self):

        #Insert MEI-I2C module
        self.insmod('i2c-mei')

        #Insert qsfp mosule
        self.insmod('optoe')

        #Insert platform module
        self.insmod('delta_ag9064_platform')

        #Insert cpld module
        self.insmod('delta_ag9064_cpld')

        #Insert swpld module
        self.insmod('delta_ag9064_swpld')

        return True

