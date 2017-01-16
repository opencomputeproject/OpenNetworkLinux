from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ly8_rangeley_r0(OnlPlatformQuanta,
                                                OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-quanta-ly8-rangeley-r0'
    MODEL="LY8"
    SYS_OBJECT_ID=".8.1"

    def baseconfig(self):
        self.insmod("emerson700")
        self.insmod("quanta_hwmon")
        self.insmod("quanta_switch", params=dict(platform="x86-64-quanta-ly8-rangeley"))

        # make ds1339 as default rtc
        os.system("ln -snf /dev/rtc1 /dev/rtc")
        os.system("hwclock --hctosys")

        # set system led to green
        os.system("echo 74 > /sys/class/gpio/export")
        os.system("echo out > /sys/class/gpio/gpio74/direction")
        os.system("echo 0 > /sys/class/gpio/gpio74/value")
        os.system("echo 74 > /sys/class/gpio/unexport")
        os.system("echo 75 > /sys/class/gpio/export")
        os.system("echo out > /sys/class/gpio/gpio75/direction")
        os.system("echo 1 > /sys/class/gpio/gpio75/value")
        os.system("echo 75 > /sys/class/gpio/unexport")

        return True
