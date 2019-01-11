from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_wedge100bf_65x_r0(OnlPlatformAccton,
                                                OnlPlatformPortConfig_64x100):
    MODEL="Wedge-100bf-65x"
    PLATFORM="x86-64-accton-wedge100bf-65x-r0"
    SYS_OBJECT_ID=".100.65.1"

    def baseconfig(self):
        ########### initialize I2C bus 1 & bus 2 ###########
        self.new_i2c_devices([
         # initialize multiplexer (PCA9548)
                ('pca9548', 0x70, 1),
                ('pca9548', 0x71, 1),
                ('pca9548', 0x72, 1),
                ('pca9548', 0x73, 1),
                ('pca9548', 0x74, 1),


                # initialize multiplexer (PCA9548)
                ('pca9548', 0x70, 2),
                ('pca9548', 0x71, 2),
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 2),
                ('pca9548', 0x74, 2),

                ('24c64', 0x50, 41),
                ])

        return True
