from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7512_32x_r0(OnlPlatformAccton):

    def model(self):
        return "AS7512-32X"

    def platform(self):
        return "x86-64-accton-as7512-32x-r0"

    def sys_oid_platform(self):
        return ".7512.32"

    def baseconfig(self):
        ########### initialize I2C bus 0 ###########
        # initialize multiplexer (PCA9548)
        self.new_i2c_device(
            [
                ('pca9458', 0x76, 0),
                ]
            )

        # initiate chassis fan
        self.new_i2c_device(
            [
                ('as7512_32x_fan', 0x66, 2),
                ]
            )

        # inititate LM75
        self.new_i2c_devices(
            [
                ('lm75', 0x48, 3),
                ('lm75', 0x49, 3),
                ('lm75', 0x4a, 3),
                ]
            )
        # initialize CPLD
        self.new_i2c_devices(
            [
                ('accton_i2c_cpld', 0x60, 4),
                ('accton_i2c_cpld', 0x62, 5),
                ('accton_i2c_cpld', 0x64, 6),
                ]
            )
        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initiate system eeprom
                ('24c02', 057, 1),

                # initiate multiplexer (PCA9548)
                ('pca9548', 0x71, 1),

                # initiate PSU-1
                ('as7512_32x_psu1', 0x50, 10),
                ('ym2651', 0x58, 11),

                # initiate PSU-2
                ('as7512_32x_psu2', 0x53, 11),
                ('ym2651', 0x5b, 10),

                #initiate max6657 thermal sensor
                ('max6657', 0x4c, 15),

                # initiate multiplexer (PCA9548)
                ('pca9548', 0x72, 1),
                ('pca9548', 0x73, 1),
                ('pca9548', 0x74, 1),
                ('pca9548', 0x75, 1),
                ]
            )

        # initialize QSFP port 1~32

        self.new_i2c_device('as7512_54x_sfp1', 0x50, 18)
        self.new_i2c_device('as7512_54x_sfp2', 0x50, 19)
        self.new_i2c_device('as7512_54x_sfp3', 0x50, 20)
        self.new_i2c_device('as7512_54x_sfp4', 0x50, 21)
        self.new_i2c_device('as7512_54x_sfp5', 0x50, 22)
        self.new_i2c_device('as7512_54x_sfp6', 0x50, 23)
        self.new_i2c_device('as7512_54x_sfp7', 0x50, 24)
        self.new_i2c_device('as7512_54x_sfp8', 0x50, 25)
        self.new_i2c_device('as7512_54x_sfp9', 0x50, 26)
        self.new_i2c_device('as7512_54x_sfp10, 0x50, 27)
        self.new_i2c_device('as7512_54x_sfp11, 0x50, 28)
        self.new_i2c_device('as7512_54x_sfp12, 0x50, 29)
        self.new_i2c_device('as7512_54x_sfp13, 0x50, 30)
        self.new_i2c_device('as7512_54x_sfp14, 0x50, 31)
        self.new_i2c_device('as7512_54x_sfp15, 0x50, 32)
        self.new_i2c_device('as7512_54x_sfp16, 0x50, 33)
        self.new_i2c_device('as7512_54x_sfp17, 0x50, 34)
        self.new_i2c_device('as7512_54x_sfp18, 0x50, 35)
        self.new_i2c_device('as7512_54x_sfp19, 0x50, 36)
        self.new_i2c_device('as7512_54x_sfp20, 0x50, 37)
        self.new_i2c_device('as7512_54x_sfp21, 0x50, 38)
        self.new_i2c_device('as7512_54x_sfp22, 0x50, 39)
        self.new_i2c_device('as7512_54x_sfp23, 0x50, 40)
        self.new_i2c_device('as7512_54x_sfp24, 0x50, 41)
        self.new_i2c_device('as7512_54x_sfp25, 0x50, 42)
        self.new_i2c_device('as7512_54x_sfp26, 0x50, 43)
        self.new_i2c_device('as7512_54x_sfp27, 0x50, 44)
        self.new_i2c_device('as7512_54x_sfp28, 0x50, 45)
        self.new_i2c_device('as7512_54x_sfp29, 0x50, 46)
        self.new_i2c_device('as7512_54x_sfp30, 0x50, 47)
        self.new_i2c_device('as7512_54x_sfp31, 0x50, 48)
        self.new_i2c_device('as7512_54x_sfp32, 0x50, 49)
                ]
            )

        return True
