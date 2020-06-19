from onl.platform.base import *
from onl.platform.accton import *
import time

class OnlPlatform_x86_64_accton_as7716_24sc_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-accton-as7716-24sc-r0'
    MODEL="AS7716-24sc"
    SYS_OBJECT_ID=".7716.24"

    def baseconfig(self):

        self.insmod('accton_i2c_cpld')
        self.insmod('dps850')
        self.insmod_platform()

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x70, 1),

                # initiate chassis fan
                ('as7716_24sc_fan', 0x66, 9),

                # inititate LM75
                ('lm75', 0x48, 10),
                ('lm75', 0x49, 10),
                ('lm75', 0x4a, 10),
                ('lm75', 0x4b, 10),
                ('lm75', 0x4c, 10),
                ('lm75', 0x4d, 10),

                #initiate CPLD
                ('accton_i2c_cpld', 0x60, 11),
                ('accton_i2c_cpld', 0x62, 12),
                ('accton_i2c_cpld', 0x64, 13),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x71, 2),

                # initiate PSU-1
                ('as7716_24sc_psu1', 0x50, 18),
                ('dps850', 0x58, 18),

                # initiate PSU-2
                ('as7716_24sc_psu2', 0x51, 17),
                ('dps850', 0x59, 17),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 2),
                ('pca9548', 0x74, 2),
                ('pca9548', 0x75, 2),

                # initialize QSFP port 1-32
                ('as7716_24sc_port9',  0x50, 25),
                ('as7716_24sc_port10', 0x50, 26),
                ('as7716_24sc_port11', 0x50, 27),
                ('as7716_24sc_port12', 0x50, 28),
                ('as7716_24sc_port1',  0x50, 29),
                ('as7716_24sc_port2',  0x50, 30),
                ('as7716_24sc_port3',  0x50, 31),
                ('as7716_24sc_port4',  0x50, 32),
                ('as7716_24sc_port6',  0x50, 33),
                ('as7716_24sc_port5',  0x50, 34),
                ('as7716_24sc_port8',  0x50, 35),
                ('as7716_24sc_port7',  0x50, 36),
                ('as7716_24sc_port13', 0x50, 37),
                ('as7716_24sc_port14', 0x50, 38),
                ('as7716_24sc_port15', 0x50, 39),
                ('as7716_24sc_port16', 0x50, 40),
                ('as7716_24sc_port18', 0x50, 41),
                ('as7716_24sc_port17', 0x50, 42),
                ('as7716_24sc_port20', 0x50, 43),
                ('as7716_24sc_port19', 0x50, 44),
                ('as7716_24sc_port22', 0x50, 45),
                ('as7716_24sc_port21', 0x50, 46),
                ('as7716_24sc_port24', 0x50, 47),
                ('as7716_24sc_port23', 0x50, 48),
                ('as7716_24sc_port26', 0x50, 49),
                ('as7716_24sc_port25', 0x50, 50),
                ('as7716_24sc_port28', 0x50, 51),
                ('as7716_24sc_port27', 0x50, 52),
                ('as7716_24sc_port30', 0x50, 53),
                ('as7716_24sc_port29', 0x50, 54),
                ('as7716_24sc_port32', 0x50, 55),
                ('as7716_24sc_port31', 0x50, 56),

                # initialize expansion card
                ('as7716_24sc_excard1', 0x6a, 41),
                ('as7716_24sc_excard2', 0x6a, 43),
                ('as7716_24sc_excard3', 0x6a, 45),
                ('as7716_24sc_excard4', 0x6a, 47),
                ('as7716_24sc_excard5', 0x6a, 49),
                ('as7716_24sc_excard6', 0x6a, 51),
                ('as7716_24sc_excard7', 0x6a, 53),
                ('as7716_24sc_excard8', 0x6a, 55),
                ])

        # Linux 5.4
        # https://github.com/torvalds/linux/commit/f1fb64b04bf414ab04e31ac107bb28137105c5fd
        for bus in ['0-0077', '1-0070', '2-0071', '2-0072', '2-0073', '2-0074', '2-0075']:
            with open('/sys/bus/i2c/devices/{}/idle_state'.format(bus), 'w') as f:
                f.write('-2')
            time.sleep(.5)

        return True
