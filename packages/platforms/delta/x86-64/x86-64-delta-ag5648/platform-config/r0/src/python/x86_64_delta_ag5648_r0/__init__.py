from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_ag5648_r0(OnlPlatformDelta,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-delta-ag5648-r0'
    MODEL="AG5648"
    SYS_OBJECT_ID=".5648"


    def baseconfig(self):
        #PCA9548 modulize
        self.new_i2c_device('pca9548', 0x70, 1)
        
        #Insert cpld module 
        self.insmod('i2c_cpld')
        self.new_i2c_device('cpld', 0x31, 2)
        self.new_i2c_device('cpld', 0x35, 2)
        self.new_i2c_device('cpld', 0x39, 2)

        #IDEEPROM modulize
        self.new_i2c_device('24c02', 0x53, 2)
        
        #Insert psu module
        self.insmod('dni_ag5648_psu')
        self.new_i2c_device('dni_ag5648_psu', 0x58, 6)
        self.new_i2c_device('dni_ag5648_psu', 0x59, 6)
        
        #insert fan module
        self.insmod('dni_emc2305')
        self.new_i2c_device('emc2305', 0x4d, 3)
        self.new_i2c_device('emc2305', 0x4d, 5)

        #Insert temperature modules
        self.new_i2c_device('tmp75', 0x4d, 2)
        self.new_i2c_device('tmp75', 0x49, 3)
        self.new_i2c_device('tmp75', 0x4b, 3)
        self.new_i2c_device('tmp75', 0x4c, 3)
        self.new_i2c_device('tmp75', 0x4e, 3)
        self.new_i2c_device('tmp75', 0x4f, 3)

        #Insert sfp module
        self.insmod('dni_ag5648_sfp')
        os.system("echo 0x18 > /sys/bus/i2c/devices/2-0035/data")
        self.new_i2c_device('dni_ag5648_sfp', 0x50, 4)

        #set front panel sys light
        os.system("echo 0x04 > /sys/bus/i2c/devices/2-0039/addr")
        os.system("echo 0x10 > /sys/bus/i2c/devices/2-0039/data")

        #set thermal Thigh & Tlow
        os.system("echo 80000 > /sys/class/hwmon/hwmon5/temp1_max")
        os.system("echo 70000 > /sys/class/hwmon/hwmon6/temp1_max")
        os.system("echo 60000 > /sys/class/hwmon/hwmon7/temp1_max")
        os.system("echo 85000 > /sys/class/hwmon/hwmon8/temp1_max")
        os.system("echo 65000 > /sys/class/hwmon/hwmon9/temp1_max")
        os.system("echo 60000 > /sys/class/hwmon/hwmon10/temp1_max")
 
        os.system("echo 75000 > /sys/class/hwmon/hwmon5/temp1_max_hyst")
        os.system("echo 65000 > /sys/class/hwmon/hwmon6/temp1_max_hyst")
        os.system("echo 55000 > /sys/class/hwmon/hwmon7/temp1_max_hyst")
        os.system("echo 80000 > /sys/class/hwmon/hwmon8/temp1_max_hyst")
        os.system("echo 60000 > /sys/class/hwmon/hwmon9/temp1_max_hyst")
        os.system("echo 55000 > /sys/class/hwmon/hwmon10/temp1_max_hyst")

        return True


