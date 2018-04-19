from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7312_54xs_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x25_6x100):

    PLATFORM='x86-64-accton-as7312-54xs-r0'
    MODEL="AS7312-54XS"
    SYS_OBJECT_ID=".7312.54.1"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as7312-54xs-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        # initialize multiplexer (PCA9548)
        self.new_i2c_device('pca9548', 0x76, 0)

        self.new_i2c_devices([
            # initiate chassis fan
            ('as7312_54xs_fan', 0x66, 2),

            # inititate LM75
            ('lm75', 0x48, 3),
            ('lm75', 0x49, 3),
            ('lm75', 0x4a, 3),
            ('lm75', 0x4b, 3),
            ])


        self.new_i2c_devices([
            # initialize CPLD
            ('as7312_54xs_cpld1', 0x60, 4),
            ('as7312_54xs_cpld2', 0x62, 5),
            ('as7312_54xs_cpld3', 0x64, 6),
            ])

        self.new_i2c_device('pca9548', 0x71, 0)

        self.new_i2c_devices([
                # initiate PSU-1
                ('as7312_54xs_psu1', 0x51, 11),
                ('ym2651', 0x59, 11),

                # initiate PSU-2
                ('as7312_54xs_psu2', 0x50, 10),
                ('ym2651', 0x58, 10),
           ])



        ########### initialize I2C bus 1 ###########

        # initiate multiplexer (PCA9548)
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x72, 1),
                ('pca9548', 0x73, 1),
                ('pca9548', 0x74, 1),
                ('pca9548', 0x75, 1),
                ('pca9548', 0x76, 1),
                ('pca9548', 0x71, 1),
                ('pca9548', 0x70, 1),
                ]
            )

        # initialize QSFP port 1~54
        for port in range(1, 49):
            self.new_i2c_device('optoe2', 0x50, port+17)

        for port in range(49, 55):
            self.new_i2c_device('optoe1', 0x50, port+17)

        for port in range(1, 55):
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+17), shell=True)

        self.new_i2c_device('24c02', 0x57, 1)
        return True
