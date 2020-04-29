from onl.platform.base import *
from onl.platform.inventec import *
import subprocess
import logging

class OnlPlatform_x86_64_inventec_d6332_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-inventec-d6332-r0'
    MODEL="D6332"
    SYS_OBJECT_ID=".6332.1"
    CHASSIS_FAN_NUM=5
    FAN_VPD_CHANNEL=1
    FAN_VPD_ADDR_BASE=0x52

    _path_prefix_list=[
        "/sys/bus/i2c/devices/2-005a/hwmon/",
        "/sys/bus/i2c/devices/2-005b/hwmon/",
        "/sys/devices/platform/coretemp.0/hwmon/",
        "/sys/bus/i2c/devices/3-0048/hwmon/",
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
    ]

    def baseconfig(self):
        addr_ten_digit=self.FAN_VPD_ADDR_BASE-self.FAN_VPD_ADDR_BASE%10
        cmd=None
        result=None
        addr=None
        line_idx=None

        os.system("rmmod gpio_ich")
        self.insmod('i2c-gpio')
        os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko gpiobase=0")
        self.insmod('ucd9000')
        self.insmod('inv_platform')
        self.insmod('inv_cpld')
        self.insmod('swps')
        self.insmod('inv_eeprom')
        self.new_i2c_device('inv_eeprom', 0x55, 0)

        for addr_offset in range(0,self.CHASSIS_FAN_NUM): 
            addr=self.FAN_VPD_ADDR_BASE+addr_offset
            cmd = "i2cdetect -y "+str(self.FAN_VPD_CHANNEL)+" "+str(addr)+" "+str(addr)+" | grep "+str(hex(addr)).replace('0x','')
            result=os.system(cmd)
            if( result==0 ):
                self.new_i2c_device('inv_eeprom',addr,self.FAN_VPD_CHANNEL)

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