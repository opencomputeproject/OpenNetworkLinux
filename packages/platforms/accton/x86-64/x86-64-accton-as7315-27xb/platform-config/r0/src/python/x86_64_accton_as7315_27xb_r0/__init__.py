from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7315_27xb_r0(OnlPlatformAccton,
                                               OnlPlatformPortConfig_20x10_4x25_3x100):
    PLATFORM='x86-64-accton-as7315-27xb-r0'
    MODEL="AS7315-27XB"
    SYS_OBJECT_ID=".7315.27"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod("ym2651y")
        self.insmod("accton_i2c_cpld")
        self.insmod_platform()

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

        self.new_i2c_devices(
            [
                # initiate PSU-1
                ('as7315_27xb_psu1', 0x50, 12),
                ('ym2401',  0x58, 12),
                # initiate PSU-1
                ('as7315_27xb_psu2', 0x53, 13),
                ('ym2401',  0x5b, 13),
            ]
        )
        # System EEPROM
        #self.new_i2c_device('24c02', 0x57, 4)

        # initialize SFP devices
        for port in range(1, 25):
            bus = port+25
            self.new_i2c_device('optoe2', 0x50, bus)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, bus))

        # Initialize QSFP devices
        for port in range(25, 28):
            bus = port - 25 + 21
            self.new_i2c_device('optoe1', 0x50, bus)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, bus))

        return True

