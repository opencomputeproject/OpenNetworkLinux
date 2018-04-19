from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_ag8032_r0(OnlPlatformDelta,OnlPlatformPortConfig_32x40):

    PLATFORM='x86-64-delta-ag8032-r0'
    MODEL="AG8032"
    SYS_OBJECT_ID=".8032.1"

    def baseconfig(self):

        self.insmod('x86-64-delta-ag8032-i2c-mux-setting.ko')
        self.insmod('x86-64-delta-ag8032-i2c-mux-cpld.ko')
        self.insmod('delta-at24.ko')

        
        self.new_i2c_devices(
            [
                ('tmp75', 0x48, 0),
                ('tmp75', 0x4a, 2),
                ('tmp75', 0x4c, 2),
                ('tmp75', 0x4d, 2),
                ('delta-24c02', 0x51, 2),
                ('delta-24c02', 0x52, 2),
                ('delta-24c02', 0x53, 2),
                ('delta-24c08', 0x54, 5),
                ('max6620', 0x29, 2),
                ('max6620', 0x2a, 2),
                ('delta-at24c02', 0x50, 4),
            ]
            )

    
        return True
