from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_ak7448_r0(OnlPlatformDelta,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-delta-ak7448-r0'
    MODEL="AK7448"
    SYS_OBJECT_ID=".7448"

    def baseconfig(self):
        #PCA9548 modulize
        self.new_i2c_device('pca9547', 0x71, 1)
        
        #Insert cpld module 
        self.insmod('i2c_cpld')
        self.new_i2c_device('cpld', 0x31, 2)
        self.new_i2c_device('cpld', 0x33, 5)
        self.new_i2c_device('cpld', 0x32, 5)

        #IDEEPROM modulize
        self.new_i2c_device('24c02', 0x53, 2)
        
        #Insert psu module
        self.insmod('dni_ak7448_psu')
        os.system("echo 0x04 > /sys/bus/i2c/devices/5-0032/addr")
        os.system("echo 0x02 > /sys/bus/i2c/devices/5-0032/data")
        self.new_i2c_device('dni_ak7448_psu', 0x58, 4)
        
        #Insert fan module
        self.insmod('dni_emc2305')
        os.system("echo 0x0a > /sys/bus/i2c/devices/5-0032/addr")
        os.system("echo 0x05 > /sys/bus/i2c/devices/5-0032/data")
        self.new_i2c_device('emc2305', 0x2c, 7)
        self.new_i2c_device('emc2305', 0x2d, 7)

        #Insert temperature modules
        self.new_i2c_device('tmp75', 0x4d, 2)
        self.new_i2c_device('tmp75', 0x4c, 6)
        self.new_i2c_device('tmp75', 0x4d, 6)
        self.new_i2c_device('tmp75', 0x4e, 6)
        os.system("echo 0x0a > /sys/bus/i2c/devices/5-0032/addr")
        os.system("echo 0x06 > /sys/bus/i2c/devices/5-0032/data")
        self.new_i2c_device('tmp75', 0x4f, 7)

        #Insert sfp module
        self.insmod('dni_ak7448_sfp')
        self.new_i2c_device('dni_ak7448_sfp', 0x50, 3)
        self.new_i2c_device('dni_ak7448_sfp', 0x50, 8)

        #Set front panel sys light
        os.system("echo 0x09 > /sys/bus/i2c/devices/5-0032/addr")
        os.system("echo 0x03 > /sys/bus/i2c/devices/5-0032/data")

        #Set thermal Thigh & Tlow
        os.system("echo 80000 > /sys/class/hwmon/hwmon6/temp1_max")
        os.system("echo 75000 > /sys/class/hwmon/hwmon6/temp1_max_hyst")

        return True


