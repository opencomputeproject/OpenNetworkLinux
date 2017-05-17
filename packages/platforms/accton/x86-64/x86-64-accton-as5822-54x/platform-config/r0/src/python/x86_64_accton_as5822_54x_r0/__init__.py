from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5822_54x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-accton-as5822-54x-r0'
    MODEL="AS5822-54X"
    SYS_OBJECT_ID=".5822.54"

    def baseconfig(self):
        self.insmod("accton_i2c_cpld")
        self.insmod("ym2651y")
        for m in [ "sfp", "psu", "fan", "leds" ]:
            self.insmod("x86-64-accton-as5822-54x-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x70, 0),

                # initialize CPLD
                ('accton_i2c_cpld', 0x60, 6),

                # initiate PSU-1 AC Power
                ('as5822_54x_psu1', 0x50, 3),
                ('ym2401', 0x58, 3),

                # initiate PSU-2 AC Power
                ('as5822_54x_psu2', 0x51, 4),
                ('ym2401', 0x59, 4),
                
                # inititate LM75
                ('lm75', 0x48, 7),
                ('lm75', 0x49, 8),
                ('lm75', 0x4a, 9),
                ('lm75', 0x4b, 9),
                ]
            )

        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x70, 1),

                # initialize CPLD
                ('accton_i2c_cpld', 0x61, 10),
                ('accton_i2c_cpld', 0x62, 11),
                
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x71, 12),
                ('pca9548', 0x72, 13),
                ('pca9548', 0x73, 14),
                ('pca9548', 0x74, 15),
                ('pca9548', 0x75, 16),
                ('pca9548', 0x76, 17),
                ('pca9548', 0x77, 17),

                # initiate IDPROM
                ('24c02', 0x57, 1),
                ]
            )

        # initialize SFP devices
        for port in range(1, 49):
            self.new_i2c_device('as5822_54x_sfp%d' % port, 0x50, port+17)
            self.new_i2c_device('as5822_54x_sfp%d' % port, 0x51, port+17)

        # initialize QSFP devices
        for port in range(49, 55):
            self.new_i2c_device('as5822_54x_sfp%d' % port, 0x50, port+17)

        return True

