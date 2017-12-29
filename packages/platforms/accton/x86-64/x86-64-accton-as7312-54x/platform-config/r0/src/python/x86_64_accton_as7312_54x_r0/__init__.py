from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7312_54x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x25_6x100):

    PLATFORM='x86-64-accton-as7312-54x-r0'
    MODEL="AS7312-54X"
    SYS_OBJECT_ID=".7312.54"

    def baseconfig(self):
        self.insmod('ym2651y')
        for m in [ 'cpld', 'fan', 'psu', 'leds', 'sfp' ]:
            self.insmod("x86-64-accton-as7312-54x-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        # initialize multiplexer (PCA9548)
        self.new_i2c_device('pca9548', 0x76, 0)

        self.new_i2c_devices([
            # initiate chassis fan
            ('as7312_54x_fan', 0x66, 2),

            # inititate LM75
            ('lm75', 0x48, 3),
            ('lm75', 0x49, 3),
            ('lm75', 0x4a, 3),
            ('lm75', 0x4b, 3),
            ])


        self.new_i2c_devices([
            # initialize CPLD
            ('accton_i2c_cpld', 0x60, 4),
            ('accton_i2c_cpld', 0x62, 5),
            ('accton_i2c_cpld', 0x64, 6),
            ])

        self.new_i2c_device('pca9548', 0x71, 0)

        self.new_i2c_devices([
                # initiate PSU-1
                ('as7312_54x_psu1', 0x51, 11),
                ('ym2651', 0x59, 11),

                # initiate PSU-2
                ('as7312_54x_psu2', 0x50, 10),
                ('ym2651', 0x58, 10),
           ])



        ########### initialize I2C bus 1 ###########

        # initiate multiplexer (PCA9548)
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x72, 1),
                ('pca9548', 0x73, 1),
                ('pca9548', 0x74, 1),
                ('pca9548', 0x75, 1),
                ('pca9548', 0x76, 1),
                ('pca9548', 0x71, 1),
                ('pca9548', 0x70, 1),
                ]
            )

        # initialize QSFP port 1~54
        self.new_i2c_devices(
            [
                ('as7312_54x_port1', 0x50, 18),
                ('as7312_54x_port2', 0x50, 19),
                ('as7312_54x_port3', 0x50, 20),
                ('as7312_54x_port4', 0x50, 21),
                ('as7312_54x_port5', 0x50, 22),
                ('as7312_54x_port6', 0x50, 23),
                ('as7312_54x_port7', 0x50, 24),
                ('as7312_54x_port8', 0x50, 25),
                ('as7312_54x_port9', 0x50, 26),
                ('as7312_54x_port10', 0x50, 27),
                ('as7312_54x_port11', 0x50, 28),
                ('as7312_54x_port12', 0x50, 29),
                ('as7312_54x_port13', 0x50, 30),
                ('as7312_54x_port14', 0x50, 31),
                ('as7312_54x_port15', 0x50, 32),
                ('as7312_54x_port16', 0x50, 33),
                ('as7312_54x_port17', 0x50, 34),
                ('as7312_54x_port18', 0x50, 35),
                ('as7312_54x_port19', 0x50, 36),
                ('as7312_54x_port20', 0x50, 37),
                ('as7312_54x_port21', 0x50, 38),
                ('as7312_54x_port22', 0x50, 39),
                ('as7312_54x_port23', 0x50, 40),
                ('as7312_54x_port24', 0x50, 41),
                ('as7312_54x_port25', 0x50, 42),
                ('as7312_54x_port26', 0x50, 43),
                ('as7312_54x_port27', 0x50, 44),
                ('as7312_54x_port28', 0x50, 45),
                ('as7312_54x_port29', 0x50, 46),
                ('as7312_54x_port30', 0x50, 47),
                ('as7312_54x_port31', 0x50, 48),
                ('as7312_54x_port32', 0x50, 49),
                ('as7312_54x_port33', 0x50, 50),
                ('as7312_54x_port34', 0x50, 51),
                ('as7312_54x_port35', 0x50, 52),
                ('as7312_54x_port36', 0x50, 53),
                ('as7312_54x_port37', 0x50, 54),
                ('as7312_54x_port38', 0x50, 55),
                ('as7312_54x_port39', 0x50, 56),
                ('as7312_54x_port40', 0x50, 57),
                ('as7312_54x_port41', 0x50, 58),
                ('as7312_54x_port42', 0x50, 59),
                ('as7312_54x_port43', 0x50, 60),
                ('as7312_54x_port44', 0x50, 61),
                ('as7312_54x_port45', 0x50, 62),
                ('as7312_54x_port46', 0x50, 63),
                ('as7312_54x_port47', 0x50, 64),
                ('as7312_54x_port48', 0x50, 65),
                ('as7312_54x_port49', 0x50, 66),
                ('as7312_54x_port50', 0x50, 67),
                ('as7312_54x_port51', 0x50, 68),
                ('as7312_54x_port52', 0x50, 69),
                ('as7312_54x_port53', 0x50, 70),
                ('as7312_54x_port54', 0x50, 71),
            ]
        )
        self.new_i2c_device('24c02', 0x57, 1)
        return True
