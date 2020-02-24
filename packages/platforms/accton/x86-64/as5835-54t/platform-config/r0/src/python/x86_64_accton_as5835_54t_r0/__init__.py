from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5835_54t_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-accton-as5835-54t-r0'
    MODEL="AS5835-54T"
    SYS_OBJECT_ID=".5835.54"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod("ym2651y")
        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as5835-54t-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 1),
                ('pca9548', 0x70, 2),
                ('pca9548', 0x71, 2),
                ('pca9548', 0x72, 2),

                # initialize CPLD
                ('as5835_54t_cpld1', 0x60, 3),
                ('as5835_54t_cpld2', 0x61, 3),
                ('as5835_54t_cpld3', 0x62, 3),
                ('as5835_54t_fan', 0x63, 3),

                # initiate PSU-1 AC Power
                ('as5835_54t_psu1', 0x50, 11),
                ('ym2401', 0x58, 11),

                # initiate PSU-2 AC Power
                ('as5835_54t_psu2', 0x53, 12),
                ('ym2401', 0x5b, 12),

                # inititate LM75
                ('lm75', 0x4b, 18),
                ('lm75', 0x4c, 19),
                ('lm75', 0x49, 20),
                ('lm75', 0x4a, 21),

                # initiate IDPROM
                ('24c02', 0x57, 1),
                ]
            )

        # initialize QSFP devices
        for port in range(49, 55):
            self.new_i2c_device('optoe1', 0x50, port-23)	

        sfp_map = [28,29,26,30,31,27]
        for i in range(0,len(sfp_map)):
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (i+49, sfp_map[i]), shell=True)

        return True
