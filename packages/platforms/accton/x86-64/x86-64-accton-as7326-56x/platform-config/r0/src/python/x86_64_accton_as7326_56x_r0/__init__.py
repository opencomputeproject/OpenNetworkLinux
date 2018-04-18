from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7326_56x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x25_8x100):

    PLATFORM='x86-64-accton-as7326-56x-r0'
    MODEL="AS7326-56X"
    SYS_OBJECT_ID=".7326.56"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as7326-56x-%s.ko" % m)

        self.new_i2c_device('pca9548', 0x77, 0)
        ########### initialize I2C bus 1 ###########
        # initialize multiplexer (PCA9548)
        self.new_i2c_device('pca9548', 0x70, 1)
        self.new_i2c_device('pca9548', 0x71, 1)
        self.new_i2c_device('pca9548', 0x72, 24)

        self.new_i2c_devices([
            # initiate chassis fan
            ('as7326_56x_fan', 0x66, 11),

            # inititate LM75
            ('lm75', 0x48, 15),
            ('lm75', 0x49, 15),
            ('lm75', 0x4a, 15),
            ('lm75', 0x4b, 15),
            ])

        self.new_i2c_devices([
            # initialize CPLD
            ('as7326_56x_cpld1', 0x60, 18),
            ('as7326_56x_cpld2', 0x62, 12),
            ('as7326_56x_cpld3', 0x64, 19),
            ])

        self.new_i2c_devices([
                # initiate PSU-1
                ('as7326_56x_psu1', 0x51, 17),
                ('ym2651', 0x59, 17),

                # initiate PSU-2
                ('as7326_56x_psu2', 0x53, 13),
                ('ym2651', 0x5b, 13),
           ])
        ########### initialize I2C bus 1 ###########

        # initiate multiplexer (PCA9548)
        self.new_i2c_devices(
            [
                ('pca9548', 0x70, 2),
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x71, 33),
                ('pca9548', 0x72, 34),
                ('pca9548', 0x73, 35),
                ('pca9548', 0x74, 36),
                ('pca9548', 0x75, 37),
                ('pca9548', 0x76, 38),
                ]
            )

        sfp_map =  [
        42,41,44,43,47,45,46,50,
        48,49,52,51,53,56,55,54,
        58,57,60,59,61,63,62,64,
        66,68,65,67,69,71,72,70,
        74,73,76,75,77,79,78,80,
        81,82,84,85,83,87,88,86,    #port 41~48
        25,26,27,28,29,30,31,32,    #port 49~56 QSFP
        22,23]                      #port 57~58 SFP+ from CPU NIF.

        # initialize SFP+ port 1~56 and 57+58.
        for port in range(1, 49):
            bus = sfp_map[port-1]
            self.new_i2c_device('optoe2', 0x50, bus)
    
        self.new_i2c_device('optoe2', 0x50, sfp_map[57-1])
        self.new_i2c_device('optoe2', 0x50, sfp_map[58-1])

        # initialize QSFP port 49~56
        for port in range(49, 57):
            bus = sfp_map[port-1]
            self.new_i2c_device('optoe1', 0x50, bus)

        for port in range(1, len(sfp_map)):
            bus = sfp_map[port-1]
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, bus), shell=True)

        self.new_i2c_device('24c04', 0x56, 0)
        return True
