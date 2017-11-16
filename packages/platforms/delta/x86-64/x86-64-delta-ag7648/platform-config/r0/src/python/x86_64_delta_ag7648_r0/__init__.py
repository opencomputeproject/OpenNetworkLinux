from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_ag7648_r0(OnlPlatformDelta,OnlPlatformPortConfig_48x10_6x40):

    PLATFORM='x86-64-delta-ag7648-r0'
    MODEL="AG7648"
    SYS_OBJECT_ID=".7648.1"

    def baseconfig(self):
        self.new_i2c_device('pca9547', 0x70, 1);

        self.insmod('x86-64-delta-ag7648-cpld-mux-1.ko')
        self.insmod('x86-64-delta-ag7648-cpld-mux-2.ko')
        ########### initialize I2C bus 0 ###########

        
        self.new_i2c_devices(
            [
                ('clock_gen', 0x69, 0),
                ('tmp75', 0x4d, 2),
                ('tmp75', 0x4c, 3),
                ('tmp75', 0x4d, 3),
                ('tmp75', 0x4e, 3),
            ]
            )

    
        return True
