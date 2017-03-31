from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_agc7648a_r0(OnlPlatformDelta,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-delta-agc7648a-r0'
    MODEL="AGC7648A"
    SYS_OBJECT_ID=".7648"

    def baseconfig(self):
        # Insert kernel module
        self.insmod('i2c_cpld')
        self.insmod('agc7648a_emc2305')
        self.insmod('agc7648a_dps800ab')
        self.insmod('agc7648a_sfp')

        # Register multuplexer 
        self.new_i2c_device('pca9547', 0x71, 1)

        # Register cpld
        self.new_i2c_device('cpld', 0x31, 2)
        self.new_i2c_device('cpld', 0x30, 5)
        self.new_i2c_device('cpld', 0x31, 5)
        self.new_i2c_device('cpld', 0x32, 5)

        # Register eeprom
        self.new_i2c_device('24c02', 0x53, 2)

        # Register fan control
        os.system("echo 0x67 > /sys/bus/i2c/devices/5-0030/addr")
        os.system("echo 0x05 > /sys/bus/i2c/devices/5-0030/data")
        self.new_i2c_device('agc7648a_emc2305', 0x2c, 3)
        self.new_i2c_device('agc7648a_emc2305', 0x2d, 3)

        # Register thermal
        self.new_i2c_device('tmp75', 0x4d, 2)
        os.system("echo 0x06 > /sys/bus/i2c/devices/5-0030/data")
        self.new_i2c_device('tmp75', 0x4f, 3)
        self.new_i2c_device('tmp75', 0x4e, 6)
        self.new_i2c_device('adt7461', 0x4c, 6)
        self.new_i2c_device('tmp75', 0x4f, 6)
        self.new_i2c_device('tmp423', 0x4c, 7)

        # Register PSU
        os.system("echo 0x1f > /sys/bus/i2c/devices/5-0030/addr")
        os.system("echo 0x00 > /sys/bus/i2c/devices/5-0030/data")
        self.new_i2c_device('agc7648a_dps800ab', 0x58, 4)

        # Register SFP
        self.new_i2c_device('agc7648a_sfp', 0x50, 8)

        # Set front panel green light of sys led
        os.system("echo 0x30 > /sys/bus/i2c/devices/5-0030/addr")
        os.system("echo 0x10 > /sys/bus/i2c/devices/5-0030/data")
        return True
