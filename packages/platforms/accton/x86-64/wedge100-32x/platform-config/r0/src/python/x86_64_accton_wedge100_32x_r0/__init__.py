from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_wedge100_32x_r0(OnlPlatformAccton,
                                                OnlPlatformPortConfig_32x100):
    MODEL="Wedge-100-32X"
    PLATFORM="x86-64-accton-wedge100-32x-r0"
    SYS_OBJECT_ID=".100.32.1"

    def baseconfig(self):
        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x70, 0),
                ('pca9548', 0x71, 0),
                ('pca9548', 0x72, 0),
                ('pca9548', 0x73, 0),
                ('pca9548', 0x74, 0),

                ('24c64', 0x50, 39),
                ])

        return True
