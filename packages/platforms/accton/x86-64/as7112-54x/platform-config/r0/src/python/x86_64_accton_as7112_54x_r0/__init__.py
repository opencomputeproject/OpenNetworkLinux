from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7112_54x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x25_6x100):
    PLATFORM='x86-64-accton-as7112-54x-r0'
    MODEL="AS7112-54X"
    SYS_OBJECT_ID=".7112.54"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        for m in [ 'cpld', 'fan', 'psu', 'led' ]:
            self.insmod("x86-64-accton-as7112-54x-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([

            ('pca9548', 0x70, 0),
            ('pca9545', 0x71, 0),
            ('pca9548', 0x72, 0),
            ('as7112_54x_fan', 0x63, 0),
            
            # initiate psu
            ('as7112_54x_psu1', 0x50, 3),
            ('ym2401', 0x58, 3),            
            ('as7112_54x_psu2', 0x53, 4),
            ('ym2401', 0x5b, 4),
            
            # initiate lm75
            ('lm75', 0x4b, 10),
            ('lm75', 0x4c, 11),
            ('lm75', 0x49, 12),
            ('lm75', 0x4a, 13),

            # initiate cpld
            ('as7112_54x_cpld1', 0x60, 1),
            ('as7112_54x_cpld2', 0x61, 1),            
            ('as7112_54x_cpld3', 0x62, 1),


            # initiate 9548
            ('pca9548', 0x70, 1),
            ('pca9548', 0x71, 22),
            ('pca9548', 0x72, 23),
            ('pca9548', 0x73, 24),
            ('pca9548', 0x74, 25),
            ('pca9548', 0x75, 26),
            ('pca9548', 0x76, 27),

            # System EEPROM
            ('24c02', 0x57, 1),
        ])

        # initialize QSFP port 1~54
        for port in range(1, 49):
            self.new_i2c_device('optoe2', 0x50, port-1+30)

        for port in range(49, 55):
            self.new_i2c_device('optoe1', 0x50, port-49+14)

        for port in range(1, 49):
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port-1+30), shell=True)

        for port in range(49, 55):
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port-49+14), shell=True)
           
        return True
