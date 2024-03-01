from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7535_28xb_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_2x100_2x400_24x25):
    PLATFORM='x86-64-accton-as7535-28xb-r0'
    MODEL="AS7535-28XB"
    SYS_OBJECT_ID=".7535.28"

    def baseconfig(self):
        self.insmod('optoe')
        for m in [ 'sys', 'cpld', 'fan', 'psu', 'thermal', 'leds', 'fpga']:
            self.insmod("x86-64-accton-as7535-28xb-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0), # i2c 1-8
                ('pca9548', 0x76, 1), # i2c 9-16
                ('pca9548', 0x73, 9), # i2c 17-24
                ('pca9548', 0x74, 17), # i2c 25-32
                ('pca9548', 0x74, 18), # i2c 33-40
                ('pca9548', 0x74, 19), # i2c 41-48

                # initialize CPLD
                ('as7535_28xb_cpld', 0x61, 12),
                # initialize FPGA
                ('as7535_28xb_fpga', 0x60, 11),
                ]
            )

        # initialize SFP devices
        for port in range(1, 5):
            subprocess.call('echo 0 > /sys/bus/i2c/devices/12-0061/module_reset_%d' % (port), shell=True)

        sfp_bus = [23, 21, 24, 22, 25, 26, 27, 28, 29, 30,
                   31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                   41, 42, 43, 44, 45, 46, 47, 48]

        # initialize QSFP devices
        for port in range(1, len(sfp_bus)+1):
            self.new_i2c_device('optoe1' if (port<=4) else 'optoe2', 0x50, sfp_bus[port-1])
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, sfp_bus[port-1]), shell=True)

        return True
