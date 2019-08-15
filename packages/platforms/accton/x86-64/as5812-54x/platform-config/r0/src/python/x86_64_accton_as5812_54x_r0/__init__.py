from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5812_54x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x10_6x40):

    PLATFORM='x86-64-accton-as5812-54x-r0'
    MODEL="AS5812-54X"
    SYS_OBJECT_ID=".5812.54.1"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('cpr_4011_4mxx')
        self.insmod("ym2651y")
        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as5812-54x-%s.ko" % m)

        ########### initialize I2C bus 0 ###########

        # initialize CPLDs
        self.new_i2c_devices(
            [
                ('as5812_54x_cpld1', 0x60, 0),
                ('as5812_54x_cpld2', 0x61, 0),
                ('as5812_54x_cpld3', 0x62, 0),
                ]
            )
        # initialize SFP devices
        for port in range(1, 49):
            self.new_i2c_device('optoe2', 0x50, port+1)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+1), shell=True)

        # Initialize QSFP devices
        for port in range(49, 55):
            self.new_i2c_device('optoe1', 0x50, port+1)
        subprocess.call('echo port49 > /sys/bus/i2c/devices/50-0050/port_name', shell=True)
        subprocess.call('echo port52 > /sys/bus/i2c/devices/51-0050/port_name', shell=True)
        subprocess.call('echo port50 > /sys/bus/i2c/devices/52-0050/port_name', shell=True)
        subprocess.call('echo port53 > /sys/bus/i2c/devices/53-0050/port_name', shell=True)
        subprocess.call('echo port51 > /sys/bus/i2c/devices/54-0050/port_name', shell=True)
        subprocess.call('echo port54 > /sys/bus/i2c/devices/55-0050/port_name', shell=True)

        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x70, 1),

                # initiate PSU-1 AC Power
                ('as5812_54x_psu1', 0x38, 57),
                ('cpr_4011_4mxx',  0x3c, 57),
                ('as5812_54x_psu1', 0x50, 57),
                ('ym2401',  0x58, 57),

                # initiate PSU-2 AC Power
                ('as5812_54x_psu2', 0x3b, 58),
                ('cpr_4011_4mxx',  0x3f, 58),
                ('as5812_54x_psu2', 0x53, 58),
                ('ym2401',  0x5b, 58),

                # initiate lm75
                ('lm75', 0x48, 61),
                ('lm75', 0x49, 62),
                ('lm75', 0x4a, 63),

                # System EEPROM
                ('24c02', 0x57, 1),
                ]
            )
        return True
