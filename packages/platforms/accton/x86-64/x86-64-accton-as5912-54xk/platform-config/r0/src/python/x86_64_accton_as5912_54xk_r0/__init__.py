from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5912_54xk_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-accton-as5912-54xk-r0'
    MODEL="AS5912-54xk"
    SYS_OBJECT_ID=".5912.54"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod("ym2651y")
        for m in [ "cpld", "psu", "fan", "leds" ]:
            self.insmod("x86-64-accton-as5912-54xk-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x76, 0),
                
                # initiate chassis fan
                ('as5912_54xk_fan', 0x66, 2),

                # inititate LM75
                ('lm75', 0x48, 3),
                ('lm75', 0x49, 3),
                ('lm75', 0x4a, 3),
                ('lm75', 0x4b, 3),

                # initialize CPLDs
                ('as5912_54xk_cpld1', 0x60, 4),
                ('as5912_54xk_cpld2', 0x62, 5),
                ]
            )

        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x74, 1),
                
                # initiate PSU-1 AC Power
                ('as5912_54xk_psu1', 0x53, 11),
                ('ym2651', 0x5b, 11),

                # initiate PSU-2 AC Power
                ('as5912_54xk_psu2', 0x50, 10),
                ('ym2651', 0x58, 10),

                # initiate IDPROM
                ('24c02', 0x57, 1),
                ]
            )

        # initialize multiplexer (PCA9548) for SFP ports
        self.new_i2c_devices(
            [
                ('pca9548', 0x72, 1),
                ('pca9548', 0x77, 18),
                ('pca9548', 0x77, 19),
                ('pca9548', 0x77, 20),
                ('pca9548', 0x77, 21),
                ('pca9548', 0x77, 22),
                ('pca9548', 0x77, 23),
                ('pca9548', 0x77, 24),
                ]
            )

        # initialize SFP devices
        for port in range(1, 49):
            self.new_i2c_device('optoe2', 0x50, port+25)

        # initialize QSFP devices
        for port in range(49, 55):
            self.new_i2c_device('optoe1', 0x50, port+25)

        for port in range(1, 55):
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+25), shell=True)

        return True

