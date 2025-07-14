import commands
from itertools import chain
from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7515_24x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_4x100_20x25):
    PLATFORM='x86-64-accton-as7515-24x-r0'
    MODEL="AS7515-24X"
    SYS_OBJECT_ID=".7515.24"

    def modprobe(self, module, required=True, params={}):
        cmd = "modprobe %s" % module
        subprocess.check_call(cmd, shell=True)

    def baseconfig(self):
        self.modprobe('optoe')
        self.modprobe('at24')
        self.modprobe('ym2651y')

        for m in [ 'fpga', 'cpld', 'fan', 'leds', 'mux', 'psu', 'sfp' ]:
            self.insmod("x86-64-accton-as7515-24x-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
                # initialize FPGA
                ('as7515_fpga_mux', 0x77, 0), # i2c 1-13

                # initialize CPLD
                ('as7515_24x_cpld', 0x61, 2),
                ('as7515_cpld_mux', 0x74, 2), # i2c 14-37

                # initialize Thermal Sensor
                ('tmp431', 0x4C, 1),
                ('lm75', 0x49, 13),
                ('lm75', 0x4A, 13),
                ('lm75', 0x4C, 13),
                ('lm75', 0x4D, 13),

                # initiate PSU
                ('as7515_24x_psu1', 0x52, 6),
                ('ym2401', 0x5A, 6),
                ('as7515_24x_psu2', 0x50, 7),
                ('ym2401', 0x58, 7),

                # initiate FAN
                ('as7515_24x_fan', 0x66, 8),

                # initiate sys-eeprom
                ('24c64', 0x56, 4),
                ])

        subprocess.call('echo 0 > /sys/devices/platform/as7515_24x_sfp/module_reset_all', shell=True)

        # initialize SFP/QSFP
        sfp_bus = [
            14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
            26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37
        ]
        for port in range(0, len(sfp_bus)):
            self.new_i2c_device('optoe1' if (port < 4) else 'optoe2', 0x50, sfp_bus[port])
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, sfp_bus[port]), shell=True)

        # initialize LED
        subprocess.call('echo 0 > /sys/devices/platform/as7515_24x_led/led_loc', shell=True)
        subprocess.call('echo 16 > /sys/devices/platform/as7515_24x_led/led_diag', shell=True)

        return True
