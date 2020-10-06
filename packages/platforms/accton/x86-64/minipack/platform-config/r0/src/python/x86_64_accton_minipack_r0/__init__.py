from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_minipack_r0(OnlPlatformAccton,
                                                OnlPlatformPortConfig_128x100):
    MODEL="Minipack"
    PLATFORM="x86-64-accton-minipack-r0"
    SYS_OBJECT_ID=".7628.3.168"

    def baseconfig(self):
        self.insmod('optoe')

        ########### load driver with defined password if any ###########
        try:
            with open('/etc/bmcpwd', "r") as f:
                pw = f.readline(128)
                pms = {'passwd':pw}
                self.insmod("minipack_psensor", params=pms)
        except IOError:
                self.insmod('minipack_psensor')

        ########### initialize I2C bus 1 ###########
        # initialize level 1 multiplexer (PCA9548)
        self.new_i2c_devices([
                ('pca9548', 0x70, 1),
                ('24c64', 0x57, 1),
                ])

        self.new_i2c_devices([
                ('optoe1', 0x50, 1),
                ])

        return True
