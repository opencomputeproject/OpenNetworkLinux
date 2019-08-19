from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7315_27xb_r0(OnlPlatformAccton,
                                               OnlPlatformPortConfig_20x10_4x25_3x100):
    PLATFORM='x86-64-accton-as7315-27xb-r0'
    MODEL="AS7315-27XB"
    SYS_OBJECT_ID=".7315.27"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        self.insmod('at24_as7315_27xb')
        self.insmod('accton_as7315_27xb_fan')
        for m in ['cpld', 'led', 'psu']:
            self.insmod("x86-64-accton-as7315-27xb-%s.ko" % m)

        # initiate multiplexer (PCA9548)
        self.new_i2c_devices(
            [
                ('pca9548', 0x76, 1),
                ('pca9548', 0x74, 3),
                ('pca9548', 0x72, 0),
                ('pca9548', 0x70, 18),
                ('pca9548', 0x71, 19),
                ('pca9548', 0x73, 20),
            ]
        )

        # initiate cplds
        self.new_i2c_devices(
            [
                ('as7315_cpld1', 0x63, 8),
                ('as7315_cpld2', 0x64, 7),  #Also gen 4 more i2c buses
            ]
        )

        self.new_i2c_devices(
            [
                ('as7315_fan', 0x66, 50),

                # inititate LM75
                ('lm75', 0x49, 51),
                ('lm75', 0x4a, 52),
                ('lm75', 0x4c, 53),

                # initiate PSU-2
                ('as7315_27xb_psu2', 0x50, 12),
                ('ym2401',  0x58, 12),

                # initiate PSU-1
                ('as7315_27xb_psu1', 0x53, 13),
                ('ym2401',  0x5b, 13),
            ]
        )
        # System EEPROM, due to only support byte-read, mount just 512KB size.
        self.new_i2c_device('24cxb04', 0x57, 4)

        # initialize SFP devices
        for port in range(1, 25):
            bus = port+25
            self.new_i2c_device('optoe2', 0x50, bus)
            cmd = 'echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, bus)
            subprocess.call(cmd, shell=True)

        # Initialize QSFP devices
        for port in range(25, 28):
            bus = port - 25 + 21
            self.new_i2c_device('optoe1', 0x50, bus)
            cmd = 'echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, bus)
            subprocess.call(cmd, shell=True)

        return True

