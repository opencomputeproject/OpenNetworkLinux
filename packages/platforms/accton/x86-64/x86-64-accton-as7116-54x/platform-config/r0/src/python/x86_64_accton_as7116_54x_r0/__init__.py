from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7116_54x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x25_6x100):
    PLATFORM='x86-64-accton-as7116-54x-r0'
    MODEL="AS7116-54X"
    SYS_OBJECT_ID=".7116.54"

    def baseconfig(self):
        self.insmod('ym2651y')
        for m in [ 'cpld', 'fan', 'psu', 'sfp', 'led' ]:
            self.insmod("x86-64-accton-as7116-54x-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
            # initialize multiplexer (PCA9548)
            ('pca9548', 0x77, 0),

            ('pca9548', 0x70, 1),
            ('pca9545', 0x71, 1),
            ('pca9548', 0x72, 1),
            ('as7116_54x_fan', 0x63, 1),

            # initiate lm75
            ('lm75', 0x4b, 17),
            ('lm75', 0x49, 19),
            ('lm75', 0x4a, 20),

            # initiate cpld
            ('as7116_54x_cpld1', 0x60, 2),
            ('as7116_54x_cpld2', 0x61, 2),            
            ('as7116_54x_cpld3', 0x62, 2),

            # initiate psu
            ('as7116_54x_psu1', 0x50, 10),
            ('ym2401', 0x58, 10),            
            ('as7116_54x_psu2', 0x53, 11),
            ('ym2401', 0x5b, 11),

            # initiate 9548
            ('pca9548', 0x70, 2),
            ('pca9548', 0x71, 29),
            ('pca9548', 0x72, 30),
            ('pca9548', 0x73, 31),
            ('pca9548', 0x74, 32),
            ('pca9548', 0x75, 33),
            ('pca9548', 0x76, 34),

            # System EEPROM
            ('24c02', 0x56, 0),
        ])

        # initialize SFP devices, start from 37
        for port in range(1, 49):
            self.new_i2c_device('as7116_54x_sfp%d' % port, 0x50, port-1+37)

        # Initialize QSFP devices, start from 21
        for port in range(49, 55):
            self.new_i2c_device('as7116_54x_sfp%d' % port, 0x50, port-49+21)
            
        return True
