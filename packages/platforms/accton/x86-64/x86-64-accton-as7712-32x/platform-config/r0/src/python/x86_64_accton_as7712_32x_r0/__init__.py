from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7712_32x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-accton-as7712-32x-r0'
    MODEL="AS7712-32X"
    SYS_OBJECT_ID=".7712.32"

    def baseconfig(self):
        self.insmod('ym2651y')
        self.insmod('accton_i2c_cpld')
        for m in [ 'fan', 'psu', 'leds', 'sfp' ]:
            self.insmod("x86-64-accton-as7712-32x-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([

            # initialize multiplexer (PCA9548)
            ('pca9548', 0x76, 0),

            # initiate chassis fan
            ('as7712_32x_fan', 0x66, 2),

            # inititate LM75
            ('lm75', 0x48, 3),
            ('lm75', 0x49, 3),
            ('lm75', 0x4a, 3),
            ('lm75', 0x4b, 3),

            ('accton_i2c_cpld', 0x60, 4),
            ('accton_i2c_cpld', 0x62, 5),
            ('accton_i2c_cpld', 0x64, 6),
            ])

        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x71, 1),

                # initiate PSU-1
                ('as7712_32x_psu1', 0x53, 11),
                ('ym2651', 0x5b, 11),

                # initiate PSU-2
                ('as7712_32x_psu2', 0x50, 10),
                ('ym2651', 0x58, 10),

                # initiate multiplexer (PCA9548)
                ('pca9548', 0x72, 1),
                ('pca9548', 0x73, 1),
                ('pca9548', 0x74, 1),
                ('pca9548', 0x75, 1),
                ]
            )

        # initialize QSFP port 1~32
        self.new_i2c_devices([
                ('as7712_32x_port9', 0x50, 18),
                ('as7712_32x_port10', 0x50, 19),
                ('as7712_32x_port11', 0x50, 20),
                ('as7712_32x_port12', 0x50, 21),
                ('as7712_32x_port1', 0x50, 22),
                ('as7712_32x_port2', 0x50, 23),
                ('as7712_32x_port3', 0x50, 24),
                ('as7712_32x_port4', 0x50, 25),
                ('as7712_32x_port6', 0x50, 26),
                ('as7712_32x_port5', 0x50, 27),
                ('as7712_32x_port8', 0x50, 28),
                ('as7712_32x_port7', 0x50, 29),
                ('as7712_32x_port13', 0x50, 30),
                ('as7712_32x_port14', 0x50, 31),
                ('as7712_32x_port15', 0x50, 32),
                ('as7712_32x_port16', 0x50, 33),
                ('as7712_32x_port17', 0x50, 34),
                ('as7712_32x_port18', 0x50, 35),
                ('as7712_32x_port19', 0x50, 36),
                ('as7712_32x_port20', 0x50, 37),
                ('as7712_32x_port25', 0x50, 38),
                ('as7712_32x_port26', 0x50, 39),
                ('as7712_32x_port27', 0x50, 40),
                ('as7712_32x_port28', 0x50, 41),
                ('as7712_32x_port29', 0x50, 42),
                ('as7712_32x_port30', 0x50, 43),
                ('as7712_32x_port31', 0x50, 44),
                ('as7712_32x_port32', 0x50, 45),
                ('as7712_32x_port21', 0x50, 46),
                ('as7712_32x_port22', 0x50, 47),
                ('as7712_32x_port23', 0x50, 48),
                ('as7712_32x_port24', 0x50, 49),
                ])

        self.new_i2c_device('24c02', 0x57, 1)
        return True
