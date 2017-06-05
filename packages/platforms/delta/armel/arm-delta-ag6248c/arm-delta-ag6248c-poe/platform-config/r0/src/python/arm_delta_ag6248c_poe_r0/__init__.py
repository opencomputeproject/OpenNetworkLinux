from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_arm_delta_ag6248c_poe_r0(OnlPlatformDelta):
    PLATFORM='arm-delta-ag6248c-poe-r0'
    MODEL="AG6248C-POE"
    SYS_OBJECT_ID=".6248.1"
    PORT_COUNT=50
    PORT_CONFIG="48x1 + 2x10"

    def baseconfig(self):
        self.insmod('arm-delta-ag6248c-poe-cpld-mux-1.ko')
        self.insmod('arm-delta-ag6248c-poe-cpld-mux-2.ko')
		
        self.new_i2c_devices(
            [
                # initiate lm75
                ('tmp75', 0x49, 0),
                ('tmp75', 0x4a, 0),

            ]
        )
        return True
