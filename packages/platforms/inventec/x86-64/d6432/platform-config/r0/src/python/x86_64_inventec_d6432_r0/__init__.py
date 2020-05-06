from onl.platform.base import *
from onl.platform.inventec import *
import logging

class OnlPlatform_x86_64_inventec_d6432_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_32x400):
    PLATFORM='x86-64-inventec-d6432-r0'
    MODEL="D6432"
    SYS_OBJECT_ID=".6432.1"
    CHASSIS_FAN_NUM=6
    FAN_VPD_CHANNEL=3
    FAN_VPD_ADDR_BASE=0x52
    EEPROM_PATH="/sys/bus/i2c/devices/2-0055/eeprom"

    _path_prefix_list=[
        "/sys/bus/i2c/devices/2-0058/hwmon/",
        "/sys/bus/i2c/devices/2-0059/hwmon/",
        "/sys/devices/platform/coretemp.0/hwmon/",
        "/sys/bus/i2c/devices/3-0018/hwmon/",
        "/sys/bus/i2c/devices/3-0048/hwmon/",
        "/sys/bus/i2c/devices/3-0049/hwmon/",
        "/sys/bus/i2c/devices/3-004a/hwmon/",
        "/sys/bus/i2c/devices/3-004d/hwmon/",
        "/sys/bus/i2c/devices/3-004e/hwmon/"
    ]
    _path_dst_list=[
        "/var/psu1",
        "/var/psu2",
        "/var/coretemp",
        "/var/board_thermal_1",
        "/var/board_thermal_2",
        "/var/board_thermal_3",
        "/var/board_thermal_4",
        "/var/board_thermal_5",
        "/var/board_thermal_6",
    ]

    def baseconfig(self):
        psu1_addr_old="2-0058"
        psu2_addr_old="2-0059"
        psu1_addr_new="2-005a"
        psu2_addr_new="2-005b"

        os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko gpiobase=0")
        self.insmod('i2c-gpio')
        self.insmod('inv_ucd90160')
        self.insmod('inv-i2c-mux-pca9641')
        self.insmod('inv_psu')
        self.insmod('inv_cpld')
        self.insmod('inv_platform')
        self.insmod('inv_eeprom')
        self.new_i2c_device('inv_eeprom', 0x55, 2)

        self.insmod('inv_sff')
        self.insmod('vpd')
        
        self.insmod('optoe')
        for ch  in range(0,32):
            self.new_i2c_device('optoe1', 0x50, 14 + ch )

        status=self.check_version()
        if (status>=0): #if detect the machine is new version, then run the follow script
            self._path_prefix_list[0]=self._path_prefix_list[0].replace(psu1_addr_old,psu1_addr_new)
            self._path_prefix_list[1]=self._path_prefix_list[1].replace(psu2_addr_old,psu2_addr_new)
        else: #if detect the machine is old version, then run the following script
            for addr_offset in range(0,self.CHASSIS_FAN_NUM):
                self.new_i2c_device('inv_eeprom',self.FAN_VPD_ADDR_BASE+addr_offset,self.FAN_VPD_CHANNEL)

        for i in range(0,len(self._path_prefix_list)):
            if( os.path.islink(self._path_dst_list[i]) ):
                os.unlink(self._path_dst_list[i])
                logging.warning("Path %s exists, remove before link again" % self._path_dst_list[i] )
            self.link_dir(self._path_prefix_list[i],self._path_dst_list[i])

        return True

    def link_dir(self,prefix,dst):
        ret=os.path.isdir(prefix)
        if ret==True:
            dirs=os.listdir(prefix)
            ret=False
            for i in range(0,len(dirs)):
                if 'hwmon' in dirs[i]:
                    src=prefix+dirs[i]
                    os.symlink(src,dst)
                    ret=True
                    break
            if ret==False:
                logging.warning("Can't find proper dir to link under %s" % prefix)            
        else:
            logging.warning("Path %s is not a dir" % prefix)

    def check_version(self):
        with open(self.EEPROM_PATH, 'rb') as f:
            hex_content = f.read()
        result = hex_content.find(b'\xfd')
        return result
