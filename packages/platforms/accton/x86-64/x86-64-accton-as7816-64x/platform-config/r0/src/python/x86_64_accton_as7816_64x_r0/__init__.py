from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7816_64x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_64x100):
    PLATFORM='x86-64-accton-as7816-64x-r0'
    MODEL="AS7816-64x"
    SYS_OBJECT_ID=".7816.64"

    def baseconfig(self):

        self.insmod("ym2651y")
        self.insmod('accton_i2c_cpld')
        self.insmod_platform()

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x71, 1),
                ('pca9548', 0x76, 1),
                ('pca9548', 0x73, 1),

                # initiate PSU-1
                ('as7816_64x_psu1', 0x53, 10),
                ('ym2851', 0x5b, 10),

                # initiate PSU-2
                ('as7816_64x_psu2', 0x50, 9),
                ('ym2851', 0x58, 9),

                # initiate chassis fan
                ('as7816_64x_fan', 0x68, 17),

                # inititate LM75
                ('lm75', 0x48, 18),
                ('lm75', 0x49, 18),
                ('lm75', 0x4a, 18),
                ('lm75', 0x4b, 18),
                ('lm75', 0x4d, 17),
                ('lm75', 0x4e, 17),

                #initiate CPLD
                ('accton_i2c_cpld', 0x60, 19),
                ('accton_i2c_cpld', 0x62, 20),
                ('accton_i2c_cpld', 0x64, 21),
                ('accton_i2c_cpld', 0x66, 22),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x70, 2),
                ('pca9548', 0x71, 2),
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 2),
                ('pca9548', 0x74, 2),
                ('pca9548', 0x75, 2),
                ('pca9548', 0x76, 2),

                # initialize QSFP port 1-64
                ('as7816_64x_port61', 0x50, 25),
                ('as7816_64x_port62', 0x50, 26),
                ('as7816_64x_port63', 0x50, 27),
                ('as7816_64x_port64', 0x50, 28),
                ('as7816_64x_port55', 0x50, 29),
                ('as7816_64x_port56', 0x50, 30),
                ('as7816_64x_port53', 0x50, 31),
                ('as7816_64x_port54', 0x50, 32),
                ('as7816_64x_port9',  0x50, 33),
                ('as7816_64x_port10', 0x50, 34),
                ('as7816_64x_port11', 0x50, 35),
                ('as7816_64x_port12', 0x50, 36),
                ('as7816_64x_port1',  0x50, 37),
                ('as7816_64x_port2',  0x50, 38),
                ('as7816_64x_port3',  0x50, 39),
                ('as7816_64x_port4',  0x50, 40),
                ('as7816_64x_port6',  0x50, 41),
                ('as7816_64x_port5',  0x50, 42),
                ('as7816_64x_port8',  0x50, 43),
                ('as7816_64x_port7',  0x50, 44),
                ('as7816_64x_port13', 0x50, 45),
                ('as7816_64x_port14', 0x50, 46),
                ('as7816_64x_port15', 0x50, 47),
                ('as7816_64x_port16', 0x50, 48),
                ('as7816_64x_port17', 0x50, 49),
                ('as7816_64x_port18', 0x50, 50),
                ('as7816_64x_port19', 0x50, 51),
                ('as7816_64x_port20', 0x50, 52),
                ('as7816_64x_port25', 0x50, 53),
                ('as7816_64x_port26', 0x50, 54),
                ('as7816_64x_port27', 0x50, 55),
                ('as7816_64x_port28', 0x50, 56),
                ('as7816_64x_port29', 0x50, 57),
                ('as7816_64x_port30', 0x50, 58),
                ('as7816_64x_port31', 0x50, 59),
                ('as7816_64x_port32', 0x50, 60),
                ('as7816_64x_port21', 0x50, 61),
                ('as7816_64x_port22', 0x50, 62),
                ('as7816_64x_port23', 0x50, 63),
                ('as7816_64x_port24', 0x50, 64),
                ('as7816_64x_port41', 0x50, 65),
                ('as7816_64x_port42', 0x50, 66),
                ('as7816_64x_port43', 0x50, 67),
                ('as7816_64x_port44', 0x50, 68),
                ('as7816_64x_port33', 0x50, 69),
                ('as7816_64x_port34', 0x50, 70),
                ('as7816_64x_port35', 0x50, 71),
                ('as7816_64x_port36', 0x50, 72),
                ('as7816_64x_port45', 0x50, 73),
                ('as7816_64x_port46', 0x50, 74),
                ('as7816_64x_port47', 0x50, 75),
                ('as7816_64x_port48', 0x50, 76),
                ('as7816_64x_port37', 0x50, 77),
                ('as7816_64x_port38', 0x50, 78),
                ('as7816_64x_port39', 0x50, 79),
                ('as7816_64x_port40', 0x50, 80),
                ('as7816_64x_port57', 0x50, 81),
                ('as7816_64x_port58', 0x50, 82),
                ('as7816_64x_port59', 0x50, 83),
                ('as7816_64x_port60', 0x50, 84),
                ('as7816_64x_port49', 0x50, 85),
                ('as7816_64x_port50', 0x50, 86),
                ('as7816_64x_port51', 0x50, 87),
                ('as7816_64x_port52', 0x50, 88),

                ('24c02', 0x56, 0),
                ])

        return True
