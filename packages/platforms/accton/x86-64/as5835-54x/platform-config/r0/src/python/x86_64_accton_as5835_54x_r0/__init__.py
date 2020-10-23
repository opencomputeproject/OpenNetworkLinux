from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5835_54x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-accton-as5835-54x-r0'
    MODEL="AS5835-54X"
    SYS_OBJECT_ID=".5835.54"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod("ym2651y")
        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as5835-54x-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 1),
                ('pca9548', 0x70, 2),
                ('pca9548', 0x71, 2),
                ('pca9548', 0x72, 2),

                # initialize CPLD
                ('as5835_54x_cpld1', 0x60, 3),
                ('as5835_54x_cpld2', 0x61, 3),
                ('as5835_54x_cpld3', 0x62, 3),
                ('as5835_54x_fan', 0x63, 3),

                # initiate PSU-1 AC Power
                ('as5835_54x_psu1', 0x50, 11),
                ('ym2401', 0x58, 11),

                # initiate PSU-2 AC Power
                ('as5835_54x_psu2', 0x53, 12),
                ('ym2401', 0x5b, 12),

                # inititate LM75
                ('lm75', 0x4b, 18),
                ('lm75', 0x4c, 19),
                ('lm75', 0x49, 20),
                ('lm75', 0x4a, 21),

                # initialize multiplexer (PCA9548)
                ('pca9548', 0x70, 3),
                ('pca9548', 0x71, 34),
                ('pca9548', 0x72, 35),
                ('pca9548', 0x73, 36),
                ('pca9548', 0x74, 37),
                ('pca9548', 0x75, 38),
                ('pca9548', 0x76, 39),

                # initiate IDPROM
                ('24c02', 0x57, 1),
                ]
            )

        # initialize SFP devices
        for port in range(1, 49):
            self.new_i2c_device('optoe2', 0x50, port+41)

        for port in range(1, 49):
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+41), shell=True)

        # initialize QSFP devices
        for port in range(49, 55):
            self.new_i2c_device('optoe1', 0x50, port-23)	
            subprocess.call('echo 0 > /sys/bus/i2c/devices/3-0062/module_reset_%d' % port, shell=True)

        sfp_map = [28,29,26,30,31,27]
        for i in range(0,len(sfp_map)):
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (i+49, sfp_map[i]), shell=True)

        #Set disable tx_disable to sfp port
        for port in range(1, 39):       
            subprocess.call('echo 0 > /sys/bus/i2c/devices/3-0061/module_tx_disable_%d' % port, shell=True)
        for port in range(39, 49): 
            subprocess.call('echo 0 > /sys/bus/i2c/devices/3-0062/module_tx_disable_%d' % port, shell=True)

        return True
