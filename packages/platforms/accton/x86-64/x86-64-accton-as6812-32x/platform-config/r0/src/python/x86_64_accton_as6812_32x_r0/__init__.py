from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as6812_32x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_32x40):
    PLATFORM='x86-64-accton-as6812-32x-r0'
    MODEL="AS6812-32X"
    SYS_OBJECT_ID=".6812.32"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('cpr_4011_4mxx')
        self.insmod("ym2651y")
        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as6812-32x-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        # initialize CPLD
        self.new_i2c_devices(
            [
                ('as6812_32x_cpld1', 0x60, 0),
                ('as6812_32x_cpld2', 0x62, 0),
                ('as6812_32x_cpld3', 0x64, 0),
                ]
            )

        # initialize QSFP port 1~32
        for port in range(1, 33):
            self.new_i2c_device('optoe1', 0x50, port+1)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+1), shell=True)

        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x70, 1),

                # initiate PSU-1
                ('as6812_32x_psu1', 0x38, 35),
                ('cpr_4011_4mxx',  0x3C, 35),
                ('as6812_32x_psu1', 0x50, 35),
                ('ym2401',  0x58, 35),

                # initiate PSU-2
                ('as6812_32x_psu2', 0x3b, 36),
                ('cpr_4011_4mxx',  0x3F, 36),
                ('as6812_32x_psu2', 0x53, 36),
                ('ym2401',  0x5b, 36),

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
