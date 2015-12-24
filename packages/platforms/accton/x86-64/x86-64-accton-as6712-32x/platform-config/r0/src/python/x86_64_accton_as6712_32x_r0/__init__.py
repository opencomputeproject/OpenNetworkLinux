from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as6712_32x_r0(OnlPlatformAccton):

    def model(self):
        return "AS6712-32X"

    def platform(self):
        return "x86-64-accton-as6712-32x-r0"

    def sys_oid_platform(self):
        return ".6712.32"


    def baseconfig(self):

        ########### initialize I2C bus 0 ###########
        # initialize CPLD
        self.new_i2c_devices(
            [
                ('as6712_32x_cpld1', 0x60, 0),
                ('as6712_32x_cpld2', 0x62, 0),
                ('as6712_32x_cpld3', 0x64, 0),
                ]
            )

        # initialize QSFP port 1~32
        for port in range(1, 33):
            self.new_i2c_device('as6712_32x_sfp%d' % port,
                                0x50,
                                port+1)

        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x70, 1),

                # initiate PSU-1 AC Power
                ('as6712_32x_psu', 0x38, 35),
                ('cpr_4011_4mxx',  0x3C, 35),

                # initiate PSU-2 AC Power
                ('as6712_32x_psu', 0x3b, 36),
                ('cpr_4011_4mxx',  0x3F, 36),

                # initiate PSU-1 DC Power
                ('as6712_32x_psu', 0x50, 35),

                # initiate PSU-2 DC Power
                ('as6712_32x_psu', 0x53, 36),

                # initiate lm75
                ('lm75', 0x48, 38),
                ('lm75', 0x49, 39),
                ('lm75', 0x4a, 40),
                ('lm75', 0x4b, 41),

                # System eeprom.
                ('24c02', 0x57, 1),
                ]
            )

        return True
