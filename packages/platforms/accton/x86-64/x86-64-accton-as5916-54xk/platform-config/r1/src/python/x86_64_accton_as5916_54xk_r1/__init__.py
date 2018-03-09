from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5916_54xk_r1(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-accton-as5916-54xk-r1'
    MODEL="AS5916-54XK"
    SYS_OBJECT_ID=".5916.54"

    def baseconfig(self):
        self.insmod("accton_i2c_cpld")
        self.insmod("ym2651y")
        for m in [ "sfp", "psu", "fan", "leds" ]:
            self.insmod("x86-64-accton-as5916-54xk-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0),
                ('pca9548', 0x76, 1),

                # initiate chassis fan
                ('as5916_54xk_fan', 0x66, 9),

                # inititate LM75
                ('lm75', 0x48, 10),
                ('lm75', 0x49, 10),
                ('lm75', 0x4a, 10),
                ('lm75', 0x4b, 10),

                # initialize CPLDs
                ('accton_i2c_cpld', 0x60, 11),
                ('accton_i2c_cpld', 0x62, 12),

                # initialize multiplexer (PCA9548)
                ('pca9548', 0x74, 2),

                # initiate PSU-1 AC Power
                ('as5916_54xk_psu1', 0x53, 18),
                ('ym2651', 0x5b, 18),

                # initiate PSU-2 AC Power
                ('as5916_54xk_psu2', 0x50, 17),
                ('ym2651', 0x58, 17),

                # initialize multiplexer (PCA9548)
                ('pca9548', 0x72, 2),
                ('pca9548', 0x75, 25),
                ('pca9548', 0x75, 26),
                ('pca9548', 0x75, 27),
                ('pca9548', 0x75, 28),
                ('pca9548', 0x75, 29),
                ('pca9548', 0x75, 30),
                ('pca9548', 0x75, 31),

                # initiate IDPROM
                ('24c02', 0x56, 0),
                ]
            )

        # initialize SFP devices
        for port in range(1, 49):
            self.new_i2c_device('as5916_54xk_sfp%d' % port, 0x50, port+32)
            self.new_i2c_device('as5916_54xk_sfp%d' % port, 0x51, port+32)

        # initialize QSFP devices
        for port in range(49, 55):
            self.new_i2c_device('as5916_54xk_sfp%d' % port, 0x50, port+32)

        return True

