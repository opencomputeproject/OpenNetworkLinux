import subprocess
import time
from onl.platform.base import *
from onl.platform.celestica import *

class OnlPlatform_x86_64_cel_belgite_r0(OnlPlatformCelestica,
                                            OnlPlatformPortConfig_48x1_8x10):
    PLATFORM='x86-64-cel-belgite-r0'
    MODEL="Belgite"
    SYS_OBJECT_ID=".2060.1"

    def run_command(self, cmd):
        status = True
        result = ""
        try:
            p = subprocess.Popen(
                cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            raw_data, err = p.communicate()
            if err == '':
                result = raw_data.strip()
        except:
            status = False
        return status, result

    def register_hwdevice_multi_times(self, driver, bus, addr, times, node):
        """
        Register the hwmon device multiple times to fix loading issue
        Param:
            string: driver name like "fsp550"
            int:    bus, decimal format like 11
            hex:    addr, hex format like 0x59
            string: node like 'fan1_input'
        Returns:
            bool:   true for success , false for fail
        """
        count = 0
        while count < times:
            self.new_i2c_device(driver, addr, bus)
            ret = os.system("ls /sys/bus/i2c/devices/i2c-%d/%d-%4.4x/hwmon/hwmon*/ | grep %s > /dev/null" % (bus, bus, addr, node))
            if ret == 0:
                return True
            os.system("echo 0x%4.4x > /sys/bus/i2c/devices/i2c-%d/delete_device" % (addr, bus))
            count = count + 1
        return False

    def baseconfig(self):
        sfp_quantity = 8
        sfp_i2c_start_bus = 10

        #Celestica Blacklist file
        blacklist_file_path="/etc/modprobe.d/celestica-blacklist.conf"
        #Blacklist the unuse module.
        if os.path.exists(blacklist_file_path):
            os.system("rm {0}".format(blacklist_file_path))
       
        os.system("touch {0}".format(blacklist_file_path))
        cel_paths = "/lib/modules/{0}/onl/celestica/".format(os.uname()[2])
        cel_dirs = os.listdir(cel_paths)
        for dir in cel_dirs:
            full_cel_path=cel_paths+dir
            if os.path.isdir(full_cel_path):
                modules=os.listdir(full_cel_path)
                for module in modules:
                    os.system("echo 'blacklist {0}' >> {1}".format(module[0:-3],blacklist_file_path))

        print("Initialize and Install the driver here")
        self.insmod("platform_cpld.ko")
        self.insmod("platform_psu.ko")
        self.insmod("platform_fan.ko")

        # Add drivers
        os.system("modprobe i2c-ismt")
        os.system("modprobe optoe") 
        self.new_i2c_device('24c64', 0x52, 1)
        # os.system("i2cset -y 1 0x70 0x10 0x00 0x01 i") # reset pca9548
        self.new_i2c_device('pca9548', 0x70, 1)
        self.new_i2c_device('fan', 0x32, 2)
        self.new_i2c_device('24c64', 0x53, 2)
        self.new_i2c_device('24c64', 0x50, 3)
        self.new_i2c_device('24c02', 0x50, 4)
        self.new_i2c_device('24c02', 0x51, 4)
        # reinstall fsp550 because it may fail to create fanx_input node if there's error 0x80 in 0x7e register when driver probes
        ret = self.register_hwdevice_multi_times('fsp550', 4, 0x58, 6, 'fan1_input')
        if ret is False:
            print("*** # Fail to register fsp550 on 4-0058, please check...")
        ret = self.register_hwdevice_multi_times('fsp550', 4, 0x59, 6, 'fan1_input')
        if ret is False:
            print("*** # Fail to register fsp550 on 4-0059, please check...")
        self.new_i2c_device('lm75b', 0x48, 5)
        self.new_i2c_device('lm75b', 0x49, 5)
        self.new_i2c_device('lm75b', 0x49, 6)
        self.new_i2c_device('lm75b', 0x4a, 6)        
        self.new_i2c_device('pca9548', 0x71, 9)
        
        # initialize SFP devices name
        for actual_i2c_port in range(sfp_i2c_start_bus, sfp_i2c_start_bus+sfp_quantity):
            self.new_i2c_device('optoe2', 0x50, actual_i2c_port)
            port_number = actual_i2c_port - (sfp_i2c_start_bus-1)
            os.system("echo 'SFP{1}' > /sys/bus/i2c/devices/i2c-{0}/{0}-0050/port_name".format(actual_i2c_port,port_number))
        
        # get fan direction from FRU eeprom
        os.system("i2cset -y 1 0x52 0x00 0xb0")
        status,fan_direction = self.run_command("i2cget -y 0x1 0x52")
        # set fan direction, B2F/F2B
        if  fan_direction.strip() == "0xbf":
            os.system("i2cset -y -f 2 0x32 0x88 0x7")
        else:
            os.system("i2cset -y -f 2 0x32 0x88 0x0")
            
        return True

