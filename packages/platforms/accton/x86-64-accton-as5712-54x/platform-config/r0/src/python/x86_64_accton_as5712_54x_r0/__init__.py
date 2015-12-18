from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5712_54x_r0(OnlPlatformAccton):

    def model(self):
        return "AS5712-54X"

    def platform(self):
        return "x86-64-accton-as5712-54x-r0"

    def sys_oid_platform(self):
        return ".5712.54"

    def baseconfig(self):
        ########### initialize I2C bus 0 ###########
        # initialize CPLDs
        self.new_i2c_devices(
            [
                ('as5712_54x_cpld1', 0x60, 0),
                ('as5712_54x_cpld2', 0x61, 0),
                ('as5712_54x_cpld3', 0x62, 0),
                ]
            )
        # initialize SFP devices
        for port in range(1, 49):
            self.new_i2c_device('as5712_54x_sfp%d' % port, 0x50, port+1)
            self.new_i2c_device('as5712_54x_sfp%d' % port, 0x51, port+1)

        # Initialize QSFP devices
        self.new_i2c_device('as5712_54x_sfp49', 0x50, 50)
        self.new_i2c_device('as5712_54x_sfp52', 0x50, 51)
        self.new_i2c_device('as5712_54x_sfp50', 0x50, 52)
        self.new_i2c_device('as5712_54x_sfp53', 0x50, 53)
        self.new_i2c_device('as5712_54x_sfp51', 0x50, 54)
        self.new_i2c_device('as5712_54x_sfp54', 0x50, 55)

        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x70, 1),

                # initiate PSU-1 AC Power
                ('as5712_54x_psu', 0x38, 57),
                ('cpr_4011_4mxx',  0x3c, 57),

                # initiate PSU-2 AC Power
                ('as5712_54x_psu', 0x3b, 58),
                ('cpr_4011_4mxx',  0x3f, 58),

		# initiate PSU-1 DC Power
		('as5712_54x_psu', 0x50, 57)

                # initiate PSU-1 DC Power
                ('as5712_54x_psu', 0x53, 58)

                # initiate lm75
                ('lm75', 0x48, 61),
                ('lm75', 0x49, 62),
                ('lm75', 0x4a, 63),
                ]
            )

        # ONIE System EEPROM
        self.new_device('24c02', 0x57, '/sys/devices/pci0000:00/0000:00:13.0/i2c-1', '1-0057')
        return True
