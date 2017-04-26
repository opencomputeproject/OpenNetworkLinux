from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_ag9032v1_r0(OnlPlatformDelta,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-delta-ag9032v1-r0'
    MODEL="AG9032V1"
    SYS_OBJECT_ID=".9032.1"


    def baseconfig(self):
        #PCA9547 modulize
        self.new_i2c_device('pca9547', 0x71, 1)
        
        #Insert swpld module on 0x31
        self.insmod('i2c_cpld')
        self.new_i2c_device('cpld', 0x31, 6)

        #IDEEPROM modulize
        self.new_i2c_device('24c02', 0x53, 2)
        
        #Insert psu module
        self.insmod('dni_ag9032v1_psu')
        self.new_i2c_device('dni_ag9032v1_psu', 0x58, 4)
        
        #insert fan module
        self.insmod('dni_emc2305')
        #Need to set 0x05 on bus 6 swpld 0x31 addr 0x21 to show Fan control on bus 3
        os.system("echo 0x21 > /sys/bus/i2c/devices/6-0031/addr")
        os.system("echo 0x05 > /sys/bus/i2c/devices/6-0031/data")
        self.new_i2c_device('emc2305', 0x2c, 3)
        self.new_i2c_device('emc2305', 0x2d, 3)

        #Insert temperature modules on bus 2 0x4d, bus 7 0x4c, 0x4d, 0x4e & bus 3 0x4f
        self.new_i2c_device('tmp75', 0x4d, 2)
        self.new_i2c_device('tmp75', 0x4c, 7)
        self.new_i2c_device('tmp75', 0x4d, 7)
        self.new_i2c_device('tmp75', 0x4e, 7)
        #Need to set 0x06 on bus 6 swpld 0x31 addr 0x21 to show device on bus 3
        os.system("echo 0x06 > /sys/bus/i2c/devices/6-0031/data")
        self.new_i2c_device('tmp75', 0x4f, 3)

        #Insert sfp module
	self.insmod('dni_ag9032v1_sfp')
	self.new_i2c_device('dni_ag9032v1_sfp', 0x50, 5)

        #set front panel sys light "GREEN"
        os.system("echo 0x1C > /sys/bus/i2c/devices/6-0031/addr")
        os.system("echo 0x04 > /sys/bus/i2c/devices/6-0031/data")

        return True


