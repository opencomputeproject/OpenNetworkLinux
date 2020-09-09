from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as9926_24d_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_24x400_2x10):
    PLATFORM='x86-64-accton-as9926-24d-r0'
    MODEL="AS9926-24D"
    SYS_OBJECT_ID=".9926.24"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod("dps850")
        for m in [ 'cpld', 'fan', 'psu', 'leds', 'fpga' ]:
            self.insmod("x86-64-accton-as9926-24d-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0),
                ('pca9548', 0x72, 1),
                ('pca9548', 0x76, 1),
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 2),
                ('pca9548', 0x74, 2),
                ('pca9548', 0x76, 2),

                # initialize CPLD
                ('as9926_24d_cpld1', 0x68, 19),
                ('as9926_24d_cpld2', 0x61, 20),
                ('as9926_24d_cpld3', 0x62, 21),
                ('as9926_24d_fan', 0x66, 17),

                # initiate PSU-1 AC Power
                ('as9926_24d_psu1', 0x51, 10),
                ('dps850', 0x59, 10),

                # initiate PSU-2 AC Power
                ('as9926_24d_psu2', 0x50, 9),
                ('dps850', 0x58, 9),

                # inititate LM75
                ('lm75', 0x48, 18),
                ('lm75', 0x49, 18),
                ('lm75', 0x4a, 18),
                ('lm75', 0x4b, 18),
                ('lm75', 0x4d, 18),
                ('lm75', 0x4e, 18),
                ('lm75', 0x4f, 18),
                ('tmp432', 0x4c, 18),

                # initiate IDPROM
                ('24c02', 0x57, 0),
                ]
            )

        # initialize QSFP devices
        for port in range(1, 25):
            self.new_i2c_device('optoe1', 0x50, port+24)	

        # initialize SFP devices
        for port in range(25, 27):
            self.new_i2c_device('optoe2', 0x50, port+24)

        # Bring QSFP out of reset
        for port in range(1, 17):
            subprocess.call('echo 0 > /sys/bus/i2c/devices/20-0061/module_reset_%d' % port, shell=True)

        for port in range(17, 25):
            subprocess.call('echo 0 > /sys/bus/i2c/devices/21-0062/module_reset_%d' % port, shell=True)

        # set port name
        for port in range(1, 27):
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+24), shell=True)

        return True
