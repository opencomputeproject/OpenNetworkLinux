from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7512_32x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-accton-as7512-32x-r0'
    MODEL="AS7512-32X"
    SYS_OBJECT_ID=".7512.32"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod("ym2651y")
        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as7512-32x-%s" % m)

        ########### initialize I2C bus 0 ###########
        # initialize multiplexer (PCA9548)
        self.new_i2c_device('pca9548', 0x76, 0)

        # initiate chassis fan
        self.new_i2c_device('as7512_32x_fan', 0x66, 2)

        # inititate LM75
        self.new_i2c_devices(
            [
                ('lm75', 0x48, 3),
                ('lm75', 0x49, 3),
                ('lm75', 0x4a, 3),
                ]
            )
        # initialize CPLD
        self.new_i2c_devices(
            [
                ('as7512_32x_cpld1', 0x60, 4),
                ('as7512_32x_cpld2', 0x62, 5),
                ('as7512_32x_cpld3', 0x64, 6),
                ]
            )
        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initiate system eeprom
                ('24c02', 0x57, 1),

                # initiate multiplexer (PCA9548)
                ('pca9548', 0x71, 1),

                # initiate PSU-1
                ('as7512_32x_psu', 0x50, 10),
                ('ym2651', 0x58, 10),

                # initiate PSU-2
                ('as7512_32x_psu', 0x53, 11),
                ('ym2651', 0x5b, 11),

                #initiate max6657 thermal sensor
                ('max6657', 0x4c, 15),

                # initiate multiplexer (PCA9548)
                ('pca9548', 0x72, 1),
                ('pca9548', 0x73, 1),
                ('pca9548', 0x74, 1),
                ('pca9548', 0x75, 1),
                ]
            )

        # initialize QSFP port 1~32
        # initialize QSFP devices
        for port in range(1, 33):
            self.new_i2c_device('optoe1', 0x50, port+17)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+17), shell=True)

        return True
