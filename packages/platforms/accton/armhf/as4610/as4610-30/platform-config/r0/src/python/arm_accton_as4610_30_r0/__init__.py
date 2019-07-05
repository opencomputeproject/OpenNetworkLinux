from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_arm_accton_as4610_30_r0(OnlPlatformAccton,
                                          OnlPlatformPortConfig_24x1_4x10):
    PLATFORM='arm-accton-as4610-30-r0'
    MODEL="AS4610-30"
    SYS_OBJECT_ID=".4610.30"

    def baseconfig(self):
        self.insmod("accton_as4610_cpld")
        self.insmod("accton_as4610_psu")
        self.insmod("accton_as4610_fan")
        self.insmod("accton_as4610_leds")
        self.insmod("ym2651y")
        self.insmod("optoe")

        subprocess.call('echo port25 > /sys/bus/i2c/devices/2-0050/port_name', shell=True)
        subprocess.call('echo port26 > /sys/bus/i2c/devices/3-0050/port_name', shell=True)
        subprocess.call('echo port27 > /sys/bus/i2c/devices/4-0050/port_name', shell=True)
        subprocess.call('echo port28 > /sys/bus/i2c/devices/5-0050/port_name', shell=True)
        subprocess.call('echo port29 > /sys/bus/i2c/devices/6-0050/port_name', shell=True)
        subprocess.call('echo port30 > /sys/bus/i2c/devices/7-0050/port_name', shell=True)

        return True
