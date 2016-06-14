from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5812_54t_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-accton-as5812-54t-r0'
    MODEL="AS5812-54T"
    SYS_OBJECT_ID=".5812.54.2"

    def baseconfig(self):
        ########### initialize I2C bus 0 ###########

        # initialize CPLDs
        self.new_i2c_device('accton_i2c_cpld', 0x60, 0)

        # initiate multiplexer (PCA9548)
        self.new_i2c_device('pca9548', 0x71, 0)

        # Initialize QSFP devices
        self.new_i2c_device('as5812_54t_qsfp49', 0x50, 4)
        self.new_i2c_device('as5812_54t_qsfp50', 0x50, 6)
        self.new_i2c_device('as5812_54t_qsfp51', 0x50, 3)
        self.new_i2c_device('as5812_54t_qsfp52', 0x50, 5)
        self.new_i2c_device('as5812_54t_qsfp53', 0x50, 7)
        self.new_i2c_device('as5812_54t_qsfp54', 0x50, 2)

        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x70, 1),

                # initiate PSU-1 AC Power
                ('as5812_54t_psu', 0x38, 11),
                ('cpr_4011_4mxx',  0x3c, 11),

                # initiate PSU-2 AC Power
                ('as5812_54t_psu', 0x3b, 12),
                ('cpr_4011_4mxx',  0x3f, 12),

                # initiate lm75
                ('lm75', 0x48, 15),
                ('lm75', 0x49, 16),
                ('lm75', 0x4a, 17),

                # System EEPROM
                ('24c02', 0x57, 1),
                ]
            )
        return True
