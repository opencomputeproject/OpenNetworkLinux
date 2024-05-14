import commands
from itertools import chain
from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as9817_64d_nb_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_64x800_2x25):
    PLATFORM='x86-64-accton-as9817-64d-nb-r0'
    MODEL="AS9817-64D-NB"
    SYS_OBJECT_ID=".9817.64"

    def modprobe(self, module, required=True, params={}):
        cmd = "modprobe %s" % module
        subprocess.check_call(cmd, shell=True)

    def baseconfig(self):
        self.modprobe('i2c-ismt')
        self.modprobe('optoe')
        self.modprobe('at24')

        # Initialize FPGA relay select register
        subprocess.call('i2cset -f -y 0 0x60 0x0f 0x03', shell=True)

        for m in [ 'i2c-ocores', 'fpga', 'cpld', 'mux', 'psu', 'fan', 'leds' ]:
            self.insmod("x86-64-accton-as9817-64-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('as9817_64_mux', 0x78, 0), # i2c 68-75
                ('as9817_64_mux', 0x70, 0), # i2c 76-83
                ('as9817_64_mux', 0x76, 76), # i2c 84-91

                # initiate FPGA/fan/psu/thermal
                ('as9817_64_fpga', 0x60, 0),
                ('as9817_64_fan', 0x33, 76),
                ('ps_2302_6l', 0x58, 77),
                ('ps_2302_6l', 0x59, 77),
                ('lm75', 0x48, 78),
                ('lm75', 0x49, 79),
                ('lm75', 0x4a, 78),
                ('lm75', 0x4b, 78),
                ('lm75', 0x4c, 78),
                ('lm75', 0x4d, 79),
                ('lm75', 0x4d, 84),  # FAN Board lm75
                ('lm75', 0x4e, 85),  # FAN Board lm75

                # initiate IDPROM
                ('24c02', 0x56, 68),
                ]
            )

        # initialize SFP devices
        for port in range(2, 66):
            subprocess.call('echo 0 > /sys/devices/platform/as9817_64_fpga/module_reset_%d' % (port-1), shell=True)

        sfp_bus = [
             2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
            18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
            34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
            50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
            66, 67
        ]

        for port in range(1, len(sfp_bus)+1):
            self.new_i2c_device('optoe3' if (port <= 64) else 'optoe2', 0x50, sfp_bus[port-1])
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, sfp_bus[port-1]), shell=True)

        for led in [ 'loc', 'diag', 'alarm' ]:
            subprocess.call('echo 0 > /sys/class/leds/as9817_64_led\:\:%s/brightness' % (led), shell=True)

        return True
