from onl.platform.base import *
from onl.platform.accton import *
import subprocess

class OnlPlatform_x86_64_accton_as5812_54t_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-accton-as5812-54t-r0'
    MODEL="AS5812-54T"
    SYS_OBJECT_ID=".5812.54.2"

    def baseconfig(self):
        ########### initialize I2C bus 0 ###########
        self.insmod("accton_i2c_cpld")
        self.insmod("cpr_4011_4mxx")
        self.insmod("ym2651y")
        for m in [ "sfp", "psu", "fan", "leds" ]:
            self.insmod("x86-64-accton-as5812-54t-%s" % m)

        # initialize CPLDs
        self.new_i2c_device('accton_i2c_cpld', 0x60, 0)

        # initiate multiplexer (PCA9548)
        self.new_i2c_device('pca9548', 0x71, 0)

        # Initialize QSFP devices
        self.new_i2c_device('as5812_54t_port49', 0x50, 4)
        self.new_i2c_device('as5812_54t_port50', 0x50, 6)
        self.new_i2c_device('as5812_54t_port51', 0x50, 3)
        self.new_i2c_device('as5812_54t_port52', 0x50, 5)
        self.new_i2c_device('as5812_54t_port53', 0x50, 7)
        self.new_i2c_device('as5812_54t_port54', 0x50, 2)

        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x70, 1),

                # initiate PSU-1 AC Power
                ('as5812_54t_psu1', 0x38, 11),
                ('cpr_4011_4mxx',  0x3c, 11),
                ('as5812_54t_psu1', 0x50, 11),
                ('ym2401',  0x58, 11),

                # initiate PSU-2 AC Power
                ('as5812_54t_psu2', 0x3b, 12),
                ('cpr_4011_4mxx',  0x3f, 12),
                ('as5812_54t_psu2', 0x53, 12),
                ('ym2401',  0x5b, 12),

                # initiate lm75
                ('lm75', 0x48, 15),
                ('lm75', 0x49, 16),
                ('lm75', 0x4a, 17),

                # System EEPROM
                ('24c02', 0x57, 1),
                ]
            )


        # Fixme - bring 10G phys out of reset.
        subprocess.check_call("i2cset -y -f 0 0x60 0x05 0x0f b", shell=True)
        subprocess.check_call("i2cset -y -f 0 0x60 0x6  0xff b", shell=True)

        return True
