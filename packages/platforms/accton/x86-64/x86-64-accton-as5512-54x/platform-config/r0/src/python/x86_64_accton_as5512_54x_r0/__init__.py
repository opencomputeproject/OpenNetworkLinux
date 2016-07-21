from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5512_54x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-accton-as5512-54x-r0'
    MODEL="AS5512-54X"
    SYS_OBJECT_ID=".5512.54.1"

    def baseconfig(self):
        ########### initialize I2C bus 0 ###########

        # initialize multiplexer (PCA9548)
        self.new_i2c_devices(
            [
                ('pca9548', 0x71, 0),
                ('pca9548', 0x72, 0),
                ('pca9548', 0x73, 0),
                ('pca9548', 0x74, 0),
                ('pca9548', 0x75, 0),
                ('pca9548', 0x76, 0),
                ('pca9548', 0x77, 0),
                ]
            )
        # initialize CPLDs
        self.new_i2c_devices(
            [
                ('accton_i2c_cpld', 0x60, 0),
                ('accton_i2c_cpld', 0x61, 0),
                ('accton_i2c_cpld', 0x62, 0),
                ]
            )
        # initialize SFP devices
        for port in range(1, 49):
            self.new_i2c_device('sfp%d' % port, 0x50, port+1)
            self.new_i2c_device('sfp%d' % port, 0x51, port+1)

        # Initialize QSFP devices
        self.new_i2c_device('sfp51', 0x50, 50)
        self.new_i2c_device('sfp54', 0x50, 51)
        self.new_i2c_device('sfp50', 0x50, 52)
        self.new_i2c_device('sfp53', 0x50, 53)
        self.new_i2c_device('sfp49', 0x50, 54)
        self.new_i2c_device('sfp52', 0x50, 55)

        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x70, 1),

                # initiate PSU-1 AC Power
                ('as5512_54x_psu', 0x38, 59),
                ('cpr_4011_4mxx',  0x3c, 59),

                # initiate PSU-2 AC Power
                ('as5512_54x_psu', 0x3b, 60),
                ('cpr_4011_4mxx',  0x3f, 60),

                # initiate lm75
                ('lm75', 0x48, 63),
                ('lm75', 0x49, 64),
                ('lm75', 0x4a, 65),

                # initiate IDPROM
                ('24c02', 0x57, 1),
                ]
            )
        return True
